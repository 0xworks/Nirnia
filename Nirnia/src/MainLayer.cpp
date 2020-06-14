#include "MainLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <random>

#define TEST_MODE 0

MainLayer::MainLayer()
: Layer("Map")
{
	//
	// loads of options here
	m_TerrainSampler.SetSeed(1337);
	m_TerrainSampler.SetFrequency(0.02f);                      // Default 0.1
	m_TerrainSampler.SetInterp(FastNoise::Quintic);            // Default: Quintic
	m_TerrainSampler.SetNoiseType(FastNoise::SimplexFractal);  // Default: Simplex

	// Fractal noise only
	m_TerrainSampler.SetFractalOctaves(4);                     // Default 3
	m_TerrainSampler.SetFractalLacunarity(2.0);                // Default 2.0
	m_TerrainSampler.SetFractalGain(0.5);                      // Default 0.5  (otherwise known as "persistence")
	m_TerrainSampler.SetFractalType(FastNoise::FBM);           // Default: FBM

	// + others for Cellular noise...

	m_GrassSampler.SetSeed(2345);
	m_GrassSampler.SetNoiseType(FastNoise::SimplexFractal);
	m_GrassSampler.SetFrequency(0.1f);

	m_TreeSampler.SetSeed(5433);
	m_TreeSampler.SetNoiseType(FastNoise::SimplexFractal);
	m_TreeSampler.SetFrequency(0.02f);

	// note: defer creation of camera until OnAttach(), so we know the correct window size.
}


void MainLayer::OnAttach() {
	HZ_PROFILE_FUNCTION();

	m_StopThreads = false;
	m_ChunkGenerator = std::thread(&MainLayer::ChunkGenerator, this);
	m_ChunkEraser = std::thread(&MainLayer::ChunkEraser, this);

	InitGroundTextures();
	InitPlayer();
	InitCamera();
	InitMap();
}


void MainLayer::OnDetach() {
	HZ_PROFILE_FUNCTION();
	if (m_ChunkGenerator.joinable()) {
		{
			std::lock_guard lock(m_ChunkMutex);
			HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
			m_StopThreads = true;
		}
		m_ChunkGeneratorCV.notify_one();
		m_ChunkGenerator.join();
	}
	if (m_ChunkEraser.joinable()) {
		{
			std::lock_guard lock(m_ChunkMutex);
			HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
			m_StopThreads = true;
		}
		m_ChunkEraserCV.notify_one();
		m_ChunkEraser.join();
	}
}


void MainLayer::InitGroundTextures() {
	HZ_PROFILE_FUNCTION();

	m_BackgroundSheet = Hazel::Texture2D::Create("assets/textures/RPGpack_sheet_2X.png");

	m_GroundTextures.resize(83);                                                                        //  TL    TR    BL    BR
	m_GroundTextures[0] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {11, 11}, {128, 128});  // water water water water
	m_GroundTextures[1] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {13, 12}, {128, 128});  // water water water grass
	m_GroundTextures[2] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water water water dirt  => X
	m_GroundTextures[3] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {14, 12}, {128, 128});  // water water grass water
	m_GroundTextures[4] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {11, 10}, {128, 128});  // water water grass grass
	m_GroundTextures[5] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water water grass dirt  => X
	m_GroundTextures[6] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water water dirt water  => X
	m_GroundTextures[7] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water water dirt grass  => X
	m_GroundTextures[8] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water water dirt dirt   => X
	m_GroundTextures[9] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {13, 11}, {128, 128});  // water grass water water
	m_GroundTextures[10] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {12, 11}, {128, 128});  // water grass water grass
	m_GroundTextures[11] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water grass water dirt  => X
	m_GroundTextures[12] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {13, 10}, {128, 128});  // water grass grass water
	m_GroundTextures[13] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {12, 10}, {128, 128});  // water grass grass grass
	m_GroundTextures[14] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water grass grass dirt  => X
	m_GroundTextures[15] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water grass dirt water  => X
	m_GroundTextures[16] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water grass dirt grass  => X
	m_GroundTextures[17] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water grass dirt dirt   => X
	m_GroundTextures[18] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt water water  => X
	m_GroundTextures[19] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt water grass  => X
	m_GroundTextures[20] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt water dirt   => X
	m_GroundTextures[21] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt grass water  => X
	m_GroundTextures[22] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt grass grass  => X
	m_GroundTextures[23] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt grass dirt   => X
	m_GroundTextures[24] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt dirt water   => X
	m_GroundTextures[25] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt dirt grass   => X
	m_GroundTextures[26] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // water dirt dirt dirt    => X
	m_GroundTextures[27] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {14, 11}, {128, 128});  // grass water water water
	m_GroundTextures[28] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {14, 10}, {128, 128});  // grass water water grass
	m_GroundTextures[29] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass water water dirt  => X
	m_GroundTextures[30] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {10, 11}, {128, 128});  // grass water grass water
	m_GroundTextures[31] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {10, 10}, {128, 128});  // grass water grass grass
	m_GroundTextures[32] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass water grass dirt  => X
	m_GroundTextures[33] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass water dirt water  => X
	m_GroundTextures[34] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass water dirt grass  => X
	m_GroundTextures[35] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass water dirt dirt   => X
	m_GroundTextures[36] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {11, 12}, {128, 128});  // grass grass water water
	m_GroundTextures[37] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {12, 12}, {128, 128});  // grass grass water grass
	m_GroundTextures[38] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass grass water dirt  => X
	m_GroundTextures[39] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {10, 12}, {128, 128});  // grass grass grass water
	m_GroundTextures[40] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {1, 11}, {128, 128});  // grass grass grass grass
	m_GroundTextures[41] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {5, 12}, {128, 128});  // grass grass grass dirt
	m_GroundTextures[42] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass grass dirt water  => X
	m_GroundTextures[43] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {7, 12}, {128, 128});  // grass grass dirt grass
	m_GroundTextures[44] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {6, 12}, {128, 128});  // grass grass dirt dirt
	m_GroundTextures[45] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass dirt water water  => X
	m_GroundTextures[46] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass dirt water grass  => X
	m_GroundTextures[47] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass dirt water dirt   => X
	m_GroundTextures[48] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass dirt grass water  => X
	m_GroundTextures[49] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {5, 10}, {128, 128});  // grass dirt grass grass
	m_GroundTextures[50] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {5, 11}, {128, 128});  // grass dirt grass dirt
	m_GroundTextures[51] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // grass dirt dirt water   => X
	m_GroundTextures[52] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {9, 10}, {128, 128});  // grass dirt dirt grass
	m_GroundTextures[53] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {9, 11}, {128, 128});  // grass dirt dirt dirt
	m_GroundTextures[54] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water water water  => X
	m_GroundTextures[55] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water water grass  => X
	m_GroundTextures[56] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water water dirt   => X
	m_GroundTextures[57] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water grass water  => X
	m_GroundTextures[58] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water grass grass  => X
	m_GroundTextures[59] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water grass dirt   => X
	m_GroundTextures[60] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water dirt water   => X
	m_GroundTextures[61] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water dirt grass   => X
	m_GroundTextures[62] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt water dirt dirt    => X
	m_GroundTextures[63] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt grass water water  => X
	m_GroundTextures[64] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt grass water grass  => X
	m_GroundTextures[65] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt grass water dirt   => X
	m_GroundTextures[66] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt grass grass water  => X
	m_GroundTextures[67] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {7, 10}, {128, 128});  // dirt grass grass grass
	m_GroundTextures[68] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {8, 10}, {128, 128});  // dirt grass grass dirt
	m_GroundTextures[69] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt grass dirt water  => X
	m_GroundTextures[70] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {7, 11}, {128, 128});  // dirt grass dirt grass
	m_GroundTextures[71] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {8, 11}, {128, 128});  // dirt grass dirt dirt
	m_GroundTextures[72] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt dirt water water  => X
	m_GroundTextures[73] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt dirt water grass  => X
	m_GroundTextures[74] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt dirt water dirt   => X
	m_GroundTextures[75] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt dirt grass water  => X
	m_GroundTextures[76] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {6, 10}, {128, 128});  // dirt dirt grass grass
	m_GroundTextures[77] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {9, 12}, {128, 128});  // dirt dirt grass dirt
	m_GroundTextures[78] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0,  0}, {128, 128});  // dirt dirt dirt water   => X
	m_GroundTextures[79] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {8, 12}, {128, 128});  // dirt dirt dirt grass
	m_GroundTextures[80] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {6, 11}, {128, 128});  // dirt dirt dirt dirt

	// There's a couple of other "grass" tiles
	m_GroundTextures[81] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {3, 10}, {128, 128});  // grass grass grass grass
	m_GroundTextures[82] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {4, 10}, {128, 128});  // grass grass grass grass

	// Trees
	m_TreeTextures.resize(12);
	m_TreeTextures[0] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0, 1}, {128, 128}, {1, 2}); // large light green tree
	m_TreeTextures[1] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {1, 1}, {128, 128}, {1, 2}); // small light green tree
	m_TreeTextures[2] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {2, 1}, {128, 128}, {1, 2}); // large orange tree
	m_TreeTextures[3] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {3, 1}, {128, 128}, {1, 2}); // small orange tree
	m_TreeTextures[4] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {4, 1}, {128, 128}, {1, 2}); // large dark green tree
	m_TreeTextures[5] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {5, 1}, {128, 128}, {1, 2}); // small dark green tree
	m_TreeTextures[6] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0, 3}, {128, 128}, {1, 1}); // large light green shrub
	m_TreeTextures[7] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {1, 3}, {128, 128}, {1, 1}); // small light green shrub
	m_TreeTextures[8] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {2, 3}, {128, 128}, {1, 1}); // large orange shrub
	m_TreeTextures[9] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {3, 3}, {128, 128}, {1, 1}); // small orange shrub
	m_TreeTextures[10] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {4, 3}, {128, 128}, {1, 1}); // large dark green shrub
	m_TreeTextures[11] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {5, 3}, {128, 128}, {1, 1}); // small dark green shrub

	m_TreeShadowTexture = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {15, 11}, {128, 128}, {1, 1});

	m_TreeBottomYOffsets.resize(12);
	m_TreeBottomYOffsets[0] = 62 / 128.0f;
	m_TreeBottomYOffsets[1] = 62 / 128.0f;
	m_TreeBottomYOffsets[2] = 62 / 128.0f;
	m_TreeBottomYOffsets[3] = 62 / 128.0f;
	m_TreeBottomYOffsets[4] = 62 / 128.0f;
	m_TreeBottomYOffsets[5] = 62 / 128.0f;
	m_TreeBottomYOffsets[6] = 58.0f / 128.0f;//
	m_TreeBottomYOffsets[7] = 60.0f / 128.0f;
	m_TreeBottomYOffsets[8] = 58.0f / 128.0f;//
	m_TreeBottomYOffsets[9] = 60.0f / 128.0f;
	m_TreeBottomYOffsets[10] = 58.0f / 128.0f;//
	m_TreeBottomYOffsets[11] = 60.0f / 128.0f;

	m_TreeYScale.resize(12);
	m_TreeYScale[0] = 2.0f;
	m_TreeYScale[1] = 2.0f;
	m_TreeYScale[2] = 2.0f;
	m_TreeYScale[3] = 2.0f;
	m_TreeYScale[4] = 2.0f;
	m_TreeYScale[5] = 2.0f;
	m_TreeYScale[6] = 1.0f;
	m_TreeYScale[7] = 1.0f;
	m_TreeYScale[8] = 1.0f;
	m_TreeYScale[9] = 1.0f;
	m_TreeYScale[10] = 1.0f;
	m_TreeYScale[11] = 1.0f;

	m_TreeShadowScale.resize(12);
	m_TreeShadowScale[0] = 1.0f;
	m_TreeShadowScale[1] = 1.0f;
	m_TreeShadowScale[2] = 1.0f;
	m_TreeShadowScale[3] = 1.0f;
	m_TreeShadowScale[4] = 1.0f;
	m_TreeShadowScale[5] = 1.0f;
	m_TreeShadowScale[6] = 1.0f;
	m_TreeShadowScale[7] = 0.6f;
	m_TreeShadowScale[8] = 1.0f;
	m_TreeShadowScale[9] = 0.6f;
	m_TreeShadowScale[10] = 1.0f;
	m_TreeShadowScale[11] = 0.6f;

}


void MainLayer::InitPlayer() {
	HZ_PROFILE_FUNCTION();

	m_PlayerSheet = Hazel::Texture2D::Create("assets/textures/player_sheet.png");

	m_PlayerSprites.resize(32);
	m_PlayerSprites[0] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {0, 3}, {128, 128}, {1, 1});
	m_PlayerSprites[1] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {1, 3}, {128, 128}, {1, 1});
	m_PlayerSprites[2] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {2, 3}, {128, 128}, {1, 1});
	m_PlayerSprites[3] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {3, 3}, {128, 128}, {1, 1});
	m_PlayerSprites[4] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {4, 3}, {128, 128}, {1, 1});
	m_PlayerSprites[5] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {5, 3}, {128, 128}, {1, 1});
	m_PlayerSprites[6] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {6, 3}, {128, 128}, {1, 1});
	m_PlayerSprites[7] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {7, 3}, {128, 128}, {1, 1});

	m_PlayerSprites[8] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {0, 2}, {128, 128}, {1, 1});
	m_PlayerSprites[9] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {1, 2}, {128, 128}, {1, 1});
	m_PlayerSprites[10] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {2, 2}, {128, 128}, {1, 1});
	m_PlayerSprites[11] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {3, 2}, {128, 128}, {1, 1});
	m_PlayerSprites[12] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {4, 2}, {128, 128}, {1, 1});
	m_PlayerSprites[13] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {5, 2}, {128, 128}, {1, 1});
	m_PlayerSprites[14] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {6, 2}, {128, 128}, {1, 1});
	m_PlayerSprites[15] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {7, 2}, {128, 128}, {1, 1});

	m_PlayerSprites[16] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {0, 1}, {128, 128}, {1, 1});
	m_PlayerSprites[17] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {1, 1}, {128, 128}, {1, 1});
	m_PlayerSprites[18] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {2, 1}, {128, 128}, {1, 1});
	m_PlayerSprites[19] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {3, 1}, {128, 128}, {1, 1});
	m_PlayerSprites[20] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {4, 1}, {128, 128}, {1, 1});
	m_PlayerSprites[21] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {5, 1}, {128, 128}, {1, 1});
	m_PlayerSprites[22] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {6, 1}, {128, 128}, {1, 1});
	m_PlayerSprites[23] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {7, 1}, {128, 128}, {1, 1});

	m_PlayerSprites[24] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {0, 0}, {128, 128}, {1, 1});
	m_PlayerSprites[25] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {1, 0}, {128, 128}, {1, 1});
	m_PlayerSprites[26] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {2, 0}, {128, 128}, {1, 1});
	m_PlayerSprites[27] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {3, 0}, {128, 128}, {1, 1});
	m_PlayerSprites[28] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {4, 0}, {128, 128}, {1, 1});
	m_PlayerSprites[29] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {5, 0}, {128, 128}, {1, 1});
	m_PlayerSprites[30] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {6, 0}, {128, 128}, {1, 1});
	m_PlayerSprites[31] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {7, 0}, {128, 128}, {1, 1});

	m_PlayerAnimations.resize(static_cast<int>(PlayerState::NumStates));

	m_PlayerAnimations[static_cast<int>(PlayerState::Idle0)] = {8, 8, 8, 8, 8, 8, 8, 8};
	m_PlayerAnimations[static_cast<int>(PlayerState::Idle1)] = {8, 9, 10, 9, 8, 8, 8, 8};
	m_PlayerAnimations[static_cast<int>(PlayerState::Idle2)] = {12, 12, 13, 13, 12, 12, 13, 13};
	m_PlayerAnimations[static_cast<int>(PlayerState::Idle3)] = {12, 14, 15, 11, 12, 12, 13, 13};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkLeft)] = {0, 1, 2, 3, 4, 5, 6, 7};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkRight)] = {0, 1, 2, 3, 4, 5, 6, 7};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkUp)] = {24, 25, 26, 27, 28, 29, 30, 31};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkDown)] = {16, 17, 18, 19, 20, 21, 22, 23};

	m_PlayerPos = {0, 0};
	m_PlayerSize = {1, 1};
	m_PlayerState = PlayerState::Idle0;
	m_PlayerFrame = 0;
}


void MainLayer::InitCamera() {
	HZ_PROFILE_FUNCTION();
	m_AspectRatio = static_cast<float>(Hazel::Application::Get().GetWindow().GetWidth()) / static_cast<float>(Hazel::Application::Get().GetWindow().GetHeight());
	m_Camera = Hazel::CreateScope<Hazel::OrthographicCamera>(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	m_ViewportWidth = (2 * static_cast<int>(std::ceil(m_AspectRatio * m_Zoom))) + 1;
	m_ViewportHeight = (2 * static_cast<int>(m_Zoom)) + 2;  // + 2 to avoid tree pop
}


void MainLayer::InitMap() {
	using namespace std::chrono_literals;
	HZ_PROFILE_FUNCTION();

	// Erase existing map chunks (if any)
	std::unordered_set<std::pair<int, int>> chunksToErase;
	for (const auto& [chunk, groundType] : m_GroundType) {
		chunksToErase.emplace(chunk);
	}
	for (const auto [i, j] : chunksToErase) {
		EraseMapChunk(i, j);
	}

	// Wait until the chunks are erased before starting to generate new ones
	// Because we are going to change chunk size, which means that the chunk key values
	// now have different meanings.
	// (e.g. chunk(0,0) after resize is a different chunk to chunk (0,0) before resize)
	bool doneErasing = false;
	while (!doneErasing) {
		std::this_thread::sleep_for(25ms);
		std::lock_guard lock(m_ChunkMutex);
		HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
		doneErasing = m_ChunksToErase.empty();
	}

	m_ChunkWidth = 2 * m_ViewportWidth;
	m_ChunkHeight = 2 * m_ViewportHeight;

	glm::vec2 position = {m_PlayerPos.x, m_PlayerPos.y + m_PlayerYOffset};

	auto left = static_cast<int>(std::floor((-m_AspectRatio * m_Zoom) + position.x));
	auto bottom = static_cast<int>(std::floor(-m_Zoom + position.y));
	auto chunkX = static_cast<int>(std::floor((left - 1.0f) / (m_ChunkWidth - m_ViewportWidth))) + 1;
	auto chunkY = static_cast<int>(std::floor((bottom - 1.0f) / (m_ChunkHeight - m_ViewportHeight))) + 1;

	// Submit 9 chunks for generation
	for (auto j = chunkY - 1; j <= chunkY + 1; ++j) {
		for (auto i = chunkX - 1; i <= chunkX + 1; ++i) {
			GenerateMapChunk(i, j);
		}
	}

	// Wait until the chunks are generated
	// Because we don't want to start drawing the world until we know it's been generated
	bool doneGenerating = false;
	while (!doneGenerating) {
		std::this_thread::sleep_for(25ms);
		std::lock_guard lock(m_ChunkMutex);
		HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
		doneGenerating = m_ChunksToGenerate.empty();
	}
}


void MainLayer::GenerateMapChunk(const int i, const int j) {
	{
		std::lock_guard lock(m_ChunkMutex);
		HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
		m_ChunksToGenerate.emplace(i, j);
	}
	m_ChunkGeneratorCV.notify_one();
}


MainLayer::TreeProperties MainLayer::GenerateTree(uint8_t tree, const int x, const int y, const int chunkTop, Random& treeRandomizer) {
	TreeProperties props;
#if TEST_MODE
	float xOffset = 0.5f;
	float yOffset = 0.5f;
	float scale = 1.0f;
#else
	float xOffset = treeRandomizer.Uniform(0.2f, 0.8f);
	float yOffset = treeRandomizer.Uniform(0.2f, 0.8f);
	float scale = treeRandomizer.Uniform(0.8f, 1.2f);
#endif
	props.Type = tree;
	props.Position = {x + xOffset, y + yOffset + ((((m_TreeYScale[tree] - 1.0f) / 2.0f) + m_TreeBottomYOffsets[tree]) * scale), ((chunkTop - (y + yOffset)) / m_ChunkHeight / 10.0f) - 0.8f};
	props.Size = {scale, m_TreeYScale[tree] * scale};
	props.ShadowPosition = {x + xOffset, y + yOffset + (m_TreeShadowYOffset * m_TreeShadowScale[tree] * scale), ((chunkTop - (y + yOffset)) / m_ChunkHeight / 10.0f) - 0.9f};
	props.ShadowSize = {m_TreeShadowScale[tree] * scale, m_TreeShadowScale[tree] * scale};
	return props;
}


void MainLayer::ChunkGenerator() {
	for (;;) {
		bool isWorkToDo = false;
		std::pair<int, int> chunk;
		{
			// suspend thread until we're told to stop, or there is a chunk to generate
			std::unique_lock lock(m_ChunkMutex);
			m_ChunkGeneratorCV.wait(lock, [&] { return m_StopThreads || !m_ChunksToGenerate.empty(); });
			HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
			//
			// Here, we have lock on m_ChunkMutex. This blocks anyone from modifying thread shared data (such as m_StopThread, and the queue)
			// until we've finished with it.
			if (m_StopThreads) {
				break;
			}

			isWorkToDo = !m_ChunksToGenerate.empty();
			if (isWorkToDo) {
				chunk = *m_ChunksToGenerate.begin();
			}
		} // release lock

		while (isWorkToDo) {
			HZ_PROFILE_SCOPE("Generate Map Chunk");
			HZ_INFO("Generating chunk ({0}, {1})", chunk.first, chunk.second);

			int chunkLeft = chunk.first * (m_ChunkWidth - m_ViewportWidth) - (m_ChunkWidth / 2);
			int chunkRight = chunkLeft + m_ChunkWidth;
			int chunkBottom = chunk.second * (m_ChunkHeight - m_ViewportHeight) - (m_ChunkHeight / 2);
			int chunkTop = chunkBottom + m_ChunkHeight;

			std::vector<uint8_t> groundCorners;
			std::vector<uint8_t> groundType;
			std::vector<uint8_t> treeType;
			std::vector<glm::vec3> treeCoords;
			std::vector<glm::vec2> treeScale;
			std::vector<glm::vec3> treeShadowCoords;
			std::vector<glm::vec2> treeShadowSize;
			size_t cornerCount = (m_ChunkWidth + 1) * (m_ChunkHeight + 1);
			size_t tileCount = m_ChunkHeight * m_ChunkWidth;
			groundCorners.reserve(cornerCount);
			groundType.resize(tileCount);
			treeType.reserve(tileCount);
			treeCoords.reserve(tileCount);
			treeScale.reserve(tileCount);
			treeShadowCoords.reserve(tileCount);
			treeShadowSize.reserve(tileCount);

			// TODO: switch to "FasterNoise"  (aka. FastNoiseSIMD)

			// Ground corners
			for (int y = chunkBottom; y <= chunkTop; ++y) {
				for (int x = chunkLeft; x <= chunkRight; ++x) {
#if TEST_MODE
					float terrainValue = 0.0f;
					if (x == 0 && y == 0) {
						terrainValue = 1.0;
					}
#else
					float terrainValue = m_TerrainSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
#endif
					if (terrainValue < -0.1f) {
						groundCorners.emplace_back(0);          // water
					} else if (terrainValue < 0.4f) {
						groundCorners.emplace_back(1);          // grass
					} else {
						groundCorners.emplace_back(2);          // dirt
					}
				}
			}

			// Ground tiles
			for (int y = chunkBottom; y < chunkTop; ++y) {
				for (int x = chunkLeft; x < chunkRight; ++x) {
					int index = ((y - chunkBottom) * m_ChunkWidth) + (x - chunkLeft);
					int cornerBL = ((y - chunkBottom) * (m_ChunkWidth + 1)) + (x - chunkLeft);
					int cornerBR = cornerBL + 1;
					int cornerTL = cornerBL + (m_ChunkWidth + 1);
					int cornerTR = cornerBL + (m_ChunkWidth + 1) + 1;
					uint8_t groundTile = (27 * groundCorners[cornerTL]) + (9 * groundCorners[cornerTR]) + (3 * groundCorners[cornerBL]) + groundCorners[cornerBR];
					if (groundTile == 40) {
						float grassValue = m_GrassSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
						if (grassValue < -0.2f) {
							groundTile = 40;
						} else if (grassValue < 0.2f) {
							groundTile = 81;
						} else {
							groundTile = 82;
						}
					}
					groundType[index] = groundTile;
				}
			}

			// Trees
			// The result is underwhelming.  Some sort of poisson disk sampling, with noise-dependent radius might be better
			// (with pre-generated level of fixed size)
#if TEST_MODE
			for (int y = chunkBottom; y < chunkTop; ++y) {
				for (int x = chunkLeft; x < chunkRight; ++x) {
					if (y == 0 && x == 0) {
						TreeProperties props = GenerateTree(8, x, y, chunkTop, Random({x, y}));
						treeType.emplace_back(props.Type);
						treeCoords.emplace_back(props.Position);
						treeScale.emplace_back(props.Size);
						treeShadowCoords.emplace_back(props.ShadowPosition);
						treeShadowSize.emplace_back(props.ShadowSize);
					} else if (y == 1 && x == 1) {
						TreeProperties props = GenerateTree(0, x, y, chunkTop, Random({x, y}));
						treeType.emplace_back(props.Type);
						treeCoords.emplace_back(props.Position);
						treeScale.emplace_back(props.Size);
						treeShadowCoords.emplace_back(props.ShadowPosition);
						treeShadowSize.emplace_back(props.ShadowSize);
					}
				}
			}
#else
			for (int y = chunkBottom; y < chunkTop; ++y) {
				for (int x = chunkLeft; x < chunkRight; ++x) {
					uint32_t index = ((y - chunkBottom) * m_ChunkWidth) + (x - chunkLeft);
					uint8_t groundTile = groundType[index];
					if (IsGrass(groundTile)) {
						float treeValue = m_TreeSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
						Random treeRandomizer({x, y});
						if (treeValue > 0.45f) {
							// big tree
							if (treeRandomizer.Uniform0_1() < 0.3f) {
								TreeProperties props = GenerateTree(0, x, y, chunkTop, treeRandomizer);
								treeType.emplace_back(props.Type);
								treeCoords.emplace_back(props.Position);
								treeScale.emplace_back(props.Size);
								treeShadowCoords.emplace_back(props.ShadowPosition);
								treeShadowSize.emplace_back(props.ShadowSize);
							}
						} else if (treeValue > 0.0f) {
							// small tree
							if (treeRandomizer.Uniform0_1() < 0.2f) {
								TreeProperties props = GenerateTree(1, x, y, chunkTop, treeRandomizer);
								treeType.emplace_back(props.Type);
								treeCoords.emplace_back(props.Position);
								treeScale.emplace_back(props.Size);
								treeShadowCoords.emplace_back(props.ShadowPosition);
								treeShadowSize.emplace_back(props.ShadowSize);
							}
						}
					} else if (IsDirt(groundTile)) {
						float treeValue = m_TreeSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
						Random treeRandomizer({x, y});
						if (treeValue > 0.7f) {
							// small orange shrub
							if (treeRandomizer.Uniform0_1() < 0.6f) {
								TreeProperties props = GenerateTree(9, x, y, chunkTop, treeRandomizer);
								treeType.emplace_back(props.Type);
								treeCoords.emplace_back(props.Position);
								treeScale.emplace_back(props.Size);
								treeShadowCoords.emplace_back(props.ShadowPosition);
								treeShadowSize.emplace_back(props.ShadowSize);
							}
						} else if (treeValue > 0.0f) {
							// orange shrubs
							if (treeRandomizer.Uniform0_1() < 0.5f) {
								TreeProperties props = GenerateTree(8, x, y, chunkTop, treeRandomizer);
								treeType.emplace_back(props.Type);
								treeCoords.emplace_back(props.Position);
								treeScale.emplace_back(props.Size);
								treeShadowCoords.emplace_back(props.ShadowPosition);
								treeShadowSize.emplace_back(props.ShadowSize);
								if (treeRandomizer.Uniform0_1() < 0.5f) {
									TreeProperties props = GenerateTree(9, x, y, chunkTop, treeRandomizer);
									treeType.emplace_back(props.Type);
									treeCoords.emplace_back(props.Position);
									treeScale.emplace_back(props.Size);
									treeShadowCoords.emplace_back(props.ShadowPosition);
									treeShadowSize.emplace_back(props.ShadowSize);
								}
							}
						}
					}
				}
			}
#endif
			{
				std::lock_guard lock(m_ChunkMutex);
				HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
				m_GroundType.insert(std::make_pair(chunk, std::move(groundType)));
				m_TreeType.insert(std::make_pair(chunk, std::move(treeType)));
				m_TreePositions.insert(std::make_pair(chunk, std::move(treeCoords)));
				m_TreeSize.insert(std::make_pair(chunk, std::move(treeScale)));
				m_TreeShadowPositions.insert(std::make_pair(chunk, std::move(treeShadowCoords)));
				m_TreeShadowSize.insert(std::make_pair(chunk, std::move(treeShadowSize)));
				m_ChunksToGenerate.erase(chunk);
				isWorkToDo = !m_ChunksToGenerate.empty();
				if (isWorkToDo) {
					chunk = *m_ChunksToGenerate.begin();
				}
			}

		}
	}
}


void MainLayer::EraseMapChunk(const int i, const int j) {
	{
		std::lock_guard lock(m_ChunkMutex);
		HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
		m_ChunksToErase.emplace(i, j);
	}
	m_ChunkEraserCV.notify_one();
}


void MainLayer::ChunkEraser() {
	for (;;) {
		bool isWorkToDo = false;
		std::pair<int, int> chunk;
		{
			// suspend thread until we're told to stop, or there is a chunk to erase
			std::unique_lock lock(m_ChunkMutex);
			m_ChunkEraserCV.wait(lock, [&] { return m_StopThreads || !m_ChunksToErase.empty(); });
			HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
			//
			// Here, we have lock on m_ChunkMutex. This blocks anyone from modifying thread shared data (such as m_StopThread, and the queue)
			// until we've finished with it.
			if (m_StopThreads) {
				break;
			}

			isWorkToDo = !m_ChunksToErase.empty();
			if (isWorkToDo) {
				chunk = *m_ChunksToErase.begin();
			}
		} // release lock

		while (isWorkToDo) {
			HZ_PROFILE_SCOPE("Erase Map Chunk");
			HZ_INFO("Erasing chunk ({0}, {1})", chunk.first, chunk.second);
			{
				std::lock_guard lock(m_ChunkMutex);
				HZ_PROFILE_LOCKMARKER(m_ChunkMutex);
				m_GroundType.erase(chunk);
				m_TreeType.erase(chunk);
				m_TreePositions.erase(chunk);
				m_TreeSize.erase(chunk);
				m_TreeShadowPositions.erase(chunk);
				m_TreeShadowSize.erase(chunk);
				m_ChunksToErase.erase(chunk);
				isWorkToDo = !m_ChunksToErase.empty();
				if (isWorkToDo) {
					chunk = *m_ChunksToErase.begin();
				}
			}
		}
	}
}


void MainLayer::OnUpdate(Hazel::Timestep ts) {
	HZ_PROFILE_FRAMEMARKER();
	HZ_PROFILE_FUNCTION();

	Hazel::Renderer2D::ResetStats();
	Hazel::Renderer2D::StatsBeginFrame();

	UpdatePlayer(ts);

	glm::vec3 position = {m_PlayerPos.x, m_PlayerPos.y + m_PlayerYOffset, 0.0f};

	auto left = static_cast<int>(std::floor((-m_AspectRatio * m_Zoom) + position.x));
	auto bottom = static_cast<int>(std::floor(-m_Zoom + position.y)) - 1; // -1 to avoid tree pop
	auto i = static_cast<int>(std::floor((left - 1.0f) / (m_ChunkWidth - m_ViewportWidth))) + 1;
	auto j = static_cast<int>(std::floor((bottom - 1.0f) / (m_ChunkHeight - m_ViewportHeight))) + 1;

	int chunkLeft = i * (m_ChunkWidth - m_ViewportWidth) - m_ChunkWidth / 2;
	int chunkBottom = j * (m_ChunkHeight - m_ViewportHeight) - m_ChunkHeight / 2;
	int chunkTop = chunkBottom + m_ChunkHeight;

	std::pair chunk = {i, j};
	if (chunk.first != m_PrevChunk.first) {
		GenerateMapChunk(chunk.first + (chunk.first - m_PrevChunk.first), chunk.second - 1);
		GenerateMapChunk(chunk.first + (chunk.first - m_PrevChunk.first), chunk.second);
		GenerateMapChunk(chunk.first + (chunk.first - m_PrevChunk.first), chunk.second + 1);
		EraseMapChunk(m_PrevChunk.first - (chunk.first - m_PrevChunk.first), m_PrevChunk.second - 1);
		EraseMapChunk(m_PrevChunk.first - (chunk.first - m_PrevChunk.first), m_PrevChunk.second);
		EraseMapChunk(m_PrevChunk.first - (chunk.first - m_PrevChunk.first), m_PrevChunk.second + 1);
	}
	if (chunk.second != m_PrevChunk.second) {
		GenerateMapChunk(chunk.first - 1, chunk.second + (chunk.second - m_PrevChunk.second));
		GenerateMapChunk(chunk.first, chunk.second + (chunk.second - m_PrevChunk.second));
		GenerateMapChunk(chunk.first + 1, chunk.second + (chunk.second - m_PrevChunk.second));
		EraseMapChunk(m_PrevChunk.first - 1, m_PrevChunk.second - (chunk.second - m_PrevChunk.second));
		EraseMapChunk(m_PrevChunk.first, m_PrevChunk.second - (chunk.second - m_PrevChunk.second));
		EraseMapChunk(m_PrevChunk.first + 1, m_PrevChunk.second - (chunk.second - m_PrevChunk.second));
	}
	m_PrevChunk = chunk;

	// Animate
	{
		//HZ_PROFILE_SCOPE("Animate");
		m_AnimationAccumulator += ts;
		if (m_AnimationAccumulator > 0.1f) {
			m_AnimationAccumulator = 0.0f;
			m_PlayerFrame = ++m_PlayerFrame % m_PlayerAnimations[static_cast<int>(m_PlayerState)].size();
			if (IsIdle(m_PlayerState) && (m_PlayerFrame == 0)) {
				if (m_Random.Uniform0_1() < 0.25f) {
					m_PlayerState = SetBlinkState(m_PlayerState);
				} else {
					m_PlayerState = ClearBlinkState(m_PlayerState);
				}
				if (m_Random.Uniform0_1() < 1.0f / 8.0f) {
					m_PlayerState = SwapFootTapState(m_PlayerState);
				}
			}
		}
	}

	// Render
	{
		HZ_PROFILE_SCOPE("Renderer Draw");

		Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
		Hazel::RenderCommand::Clear();

		m_Camera->SetPosition(position);

		Hazel::Renderer2D::BeginScene(*m_Camera);

		// Ground
		const std::vector<uint8_t>& groundType = m_GroundType[chunk];
		for (int y = bottom; y < bottom + m_ViewportHeight; ++y) {
			for (int x = left; x < left + m_ViewportWidth; ++x) {
				int index = ((y - chunkBottom) * m_ChunkWidth) + (x - chunkLeft);
#if TEST_MODE
				if (x == 0 && y == 0) {
					Hazel::Renderer2D::DrawQuad({x + 0.5, y + 0.5, -0.99f}, {1, 1}, m_GroundTextures[groundType[index]], 1.0f, {1.0f, 0.0f, 0.0f, 1.0f});
				} else {
					Hazel::Renderer2D::DrawQuad({x + 0.5, y + 0.5, -0.99f}, {1, 1}, m_GroundTextures[groundType[index]]);
				}
#else
				Hazel::Renderer2D::DrawQuad({x + 0.5f, y + 0.5f, -0.99f}, {1, 1}, m_GroundTextures[groundType[index]]);
#endif
			}
		}

		// Tree shadows
		const std::vector<glm::vec3>& treeShadowCoords = m_TreeShadowPositions[chunk];
		const std::vector<glm::vec2>& treeShadowSize = m_TreeShadowSize[chunk];
		for (size_t t = 0; t < treeShadowCoords.size(); ++t) {
			Hazel::Renderer2D::DrawQuad(treeShadowCoords[t], treeShadowSize[t], m_TreeShadowTexture);
		}

		// Trees
		const std::vector<glm::vec3>& treeCoords = m_TreePositions[chunk];
		const std::vector<glm::vec2>& treeSize = m_TreeSize[chunk];
		const std::vector<uint8_t>& treeType = m_TreeType[chunk];
		for(size_t t = 0; t < treeCoords.size(); ++t) {
			Hazel::Renderer2D::DrawQuad(treeCoords[t], treeSize[t], m_TreeTextures[treeType[t]]);
		}

		// Player
		glm::vec3 playerPosition = {position.x, position.y, ((chunkTop - (m_PlayerPos.y)) / m_ChunkHeight / 10.0f) - 0.8f};
		Hazel::Renderer2D::DrawQuad(playerPosition, m_PlayerSize, m_PlayerSprites[m_PlayerAnimations[static_cast<int>(m_PlayerState)][m_PlayerFrame]]);

		Hazel::Renderer2D::EndScene();
	}

	Hazel::Renderer2D::StatsEndFrame();

	auto stats = Hazel::Renderer2D::GetStats();
	float averageRenderTime = stats.TotalFrameRenderTime / stats.FrameRenderTime.size(); // nb: wont be accurate until we have gathered at least stats.FrameRenderTime().size() results
	float averageFPS = 1.0f / averageRenderTime;
	char buffer[64];
	sprintf_s(buffer, 64, "Average frame render time: %8.5f (%5.0f fps)", averageRenderTime, averageFPS);
	glfwSetWindowTitle((GLFWwindow*)Hazel::Application::Get().GetWindow().GetNativeWindow(), buffer);
}


void MainLayer::UpdatePlayer(Hazel::Timestep ts) {
	HZ_PROFILE_FUNCTION();

	PlayerState newState = PlayerState::Idle0;
	if (Hazel::Input::IsKeyPressed(HZ_KEY_A)) {
		m_PlayerPos.x -= ts * m_PlayerMovementSpeed;
		m_PlayerSize = {-1, 1};
		newState = PlayerState::WalkLeft;
	} else if (Hazel::Input::IsKeyPressed(HZ_KEY_D)) {
		m_PlayerPos.x += ts * m_PlayerMovementSpeed;
		m_PlayerSize = {1, 1};
		newState = PlayerState::WalkRight;
	}

	if (Hazel::Input::IsKeyPressed(HZ_KEY_W)) {
		m_PlayerPos.y += ts * m_PlayerMovementSpeed;
		newState = PlayerState::WalkUp;
	} else if (Hazel::Input::IsKeyPressed(HZ_KEY_S)) {
		m_PlayerPos.y -= ts * m_PlayerMovementSpeed;
		newState = PlayerState::WalkDown;
	}

	if (newState != m_PlayerState) {
		if (!IsIdle(newState) || !IsIdle(m_PlayerState)) {
			m_PlayerState = newState;
			m_PlayerFrame = 0;
		}
	}
}


void MainLayer::OnImGuiRender() {
	HZ_PROFILE_FUNCTION();

	ImGui::Begin("Stats");
	auto stats = Hazel::Renderer2D::GetStats();
	ImGui::Text("Renderer2D Stats:");
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
	ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
	ImGui::End();
}


void MainLayer::OnEvent(Hazel::Event& e) {

	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(MainLayer::OnWindowResize));
}


bool MainLayer::OnWindowResize(Hazel::WindowResizeEvent& e) {
	InitCamera();
	InitMap();
	return false;
}
