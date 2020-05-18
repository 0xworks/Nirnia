#include "GroundLayer.h"
#include "Random.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <random>

GroundLayer::GroundLayer()
: Layer("Map")
{
	//
	// loads of options here
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


void GroundLayer::OnAttach() {
	HZ_PROFILE_FUNCTION();
	m_BackgroundSheet = Hazel::Texture2D::Create("assets/textures/RPGpack_sheet_2X.png");
	m_PlayerSheet = Hazel::Texture2D::Create("assets/textures/player_tilesheet.png");

	m_GroundTiles.resize(83);                                                                        //  TL    TR    BL    BR
	m_GroundTiles[ 0] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {11, 11}, {128, 128});  // water water water water
	m_GroundTiles[ 1] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {13, 12}, {128, 128});  // water water water grass
	m_GroundTiles[ 2] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water water water dirt  => X
	m_GroundTiles[ 3] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {14, 12}, {128, 128});  // water water grass water
	m_GroundTiles[ 4] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {11, 10}, {128, 128});  // water water grass grass
	m_GroundTiles[ 5] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water water grass dirt  => X
	m_GroundTiles[ 6] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water water dirt water  => X
	m_GroundTiles[ 7] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water water dirt grass  => X
	m_GroundTiles[ 8] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water water dirt dirt   => X
	m_GroundTiles[ 9] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {13, 11}, {128, 128});  // water grass water water
	m_GroundTiles[10] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {12, 11}, {128, 128});  // water grass water grass
	m_GroundTiles[11] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water grass water dirt  => X
	m_GroundTiles[12] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {13, 10}, {128, 128});  // water grass grass water
	m_GroundTiles[13] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {12, 10}, {128, 128});  // water grass grass grass
	m_GroundTiles[14] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water grass grass dirt  => X
	m_GroundTiles[15] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water grass dirt water  => X
	m_GroundTiles[16] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water grass dirt grass  => X
	m_GroundTiles[17] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water grass dirt dirt   => X
	m_GroundTiles[18] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt water water  => X
	m_GroundTiles[19] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt water grass  => X
	m_GroundTiles[20] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt water dirt   => X
	m_GroundTiles[21] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt grass water  => X
	m_GroundTiles[22] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt grass grass  => X
	m_GroundTiles[23] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt grass dirt   => X
	m_GroundTiles[24] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt dirt water   => X
	m_GroundTiles[25] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt dirt grass   => X
	m_GroundTiles[26] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // water dirt dirt dirt    => X
	m_GroundTiles[27] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {14, 11}, {128, 128});  // grass water water water
	m_GroundTiles[28] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {14, 10}, {128, 128});  // grass water water grass
	m_GroundTiles[29] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass water water dirt  => X
	m_GroundTiles[30] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {10, 11}, {128, 128});  // grass water grass water
	m_GroundTiles[31] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {10, 10}, {128, 128});  // grass water grass grass
	m_GroundTiles[32] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass water grass dirt  => X
	m_GroundTiles[33] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass water dirt water  => X
	m_GroundTiles[34] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass water dirt grass  => X
	m_GroundTiles[35] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass water dirt dirt   => X
	m_GroundTiles[36] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {11, 12}, {128, 128});  // grass grass water water
	m_GroundTiles[37] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {12, 12}, {128, 128});  // grass grass water grass
	m_GroundTiles[38] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass grass water dirt  => X
	m_GroundTiles[39] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {10, 12}, {128, 128});  // grass grass grass water
	m_GroundTiles[40] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 1, 11}, {128, 128});  // grass grass grass grass
	m_GroundTiles[41] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 5, 12}, {128, 128});  // grass grass grass dirt
	m_GroundTiles[42] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass grass dirt water  => X
	m_GroundTiles[43] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 7, 12}, {128, 128});  // grass grass dirt grass
	m_GroundTiles[44] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 6, 12}, {128, 128});  // grass grass dirt dirt
	m_GroundTiles[45] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass dirt water water  => X
	m_GroundTiles[46] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass dirt water grass  => X
	m_GroundTiles[47] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass dirt water dirt   => X
	m_GroundTiles[48] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass dirt grass water  => X
	m_GroundTiles[49] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 5, 10}, {128, 128});  // grass dirt grass grass
	m_GroundTiles[50] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 5, 11}, {128, 128});  // grass dirt grass dirt
	m_GroundTiles[51] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // grass dirt dirt water   => X
	m_GroundTiles[52] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 9, 10}, {128, 128});  // grass dirt dirt grass
	m_GroundTiles[53] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 9, 11}, {128, 128});  // grass dirt dirt dirt
	m_GroundTiles[54] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water water water  => X
	m_GroundTiles[55] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water water grass  => X
	m_GroundTiles[56] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water water dirt   => X
	m_GroundTiles[57] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water grass water  => X
	m_GroundTiles[58] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water grass grass  => X
	m_GroundTiles[59] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water grass dirt   => X
	m_GroundTiles[60] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water dirt water   => X
	m_GroundTiles[61] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water dirt grass   => X
	m_GroundTiles[62] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt water dirt dirt    => X
	m_GroundTiles[63] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt grass water water  => X
	m_GroundTiles[64] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt grass water grass  => X
	m_GroundTiles[65] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt grass water dirt   => X
	m_GroundTiles[66] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt grass grass water  => X
	m_GroundTiles[67] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 7, 10}, {128, 128});  // dirt grass grass grass
	m_GroundTiles[68] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 8, 10}, {128, 128});  // dirt grass grass dirt
	m_GroundTiles[69] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt grass dirt water  => X
	m_GroundTiles[70] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 7, 11}, {128, 128});  // dirt grass dirt grass
	m_GroundTiles[71] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 8, 11}, {128, 128});  // dirt grass dirt dirt
	m_GroundTiles[72] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt dirt water water  => X
	m_GroundTiles[73] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt dirt water grass  => X
	m_GroundTiles[74] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt dirt water dirt   => X
	m_GroundTiles[75] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt dirt grass water  => X
	m_GroundTiles[76] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 6, 10}, {128, 128});  // dirt dirt grass grass
	m_GroundTiles[77] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 9, 12}, {128, 128});  // dirt dirt grass dirt
	m_GroundTiles[78] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 0,  0}, {128, 128});  // dirt dirt dirt water   => X
	m_GroundTiles[79] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 8, 12}, {128, 128});  // dirt dirt dirt grass
	m_GroundTiles[80] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 6, 11}, {128, 128});  // dirt dirt dirt dirt

	// There's a couple of other "grass" tiles
	m_GroundTiles[81] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 3, 10}, {128, 128});  // grass grass grass grass
	m_GroundTiles[82] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, { 4, 10}, {128, 128});  // grass grass grass grass

	// Trees
	m_TreeTiles.resize(12);
	m_TreeTiles[ 0] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0, 1}, {128, 128}, {1, 2}); // large light green tree
	m_TreeTiles[ 1] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {1, 1}, {128, 128}, {1, 2}); // small light green tree
	m_TreeTiles[ 2] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {2, 1}, {128, 128}, {1, 2}); // large orange tree
	m_TreeTiles[ 3] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {3, 1}, {128, 128}, {1, 2}); // small orange tree
	m_TreeTiles[ 4] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {4, 1}, {128, 128}, {1, 2}); // large dark green tree
	m_TreeTiles[ 5] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {5, 1}, {128, 128}, {1, 2}); // small dark green tree
	m_TreeTiles[ 6] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {0, 3}, {128, 128}, {1, 1}); // large light green shrub
	m_TreeTiles[ 7] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {1, 3}, {128, 128}, {1, 1}); // small light green shrub
	m_TreeTiles[ 8] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {2, 3}, {128, 128}, {1, 1}); // large orange shrub
	m_TreeTiles[ 9] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {3, 3}, {128, 128}, {1, 1}); // small orange shrub
	m_TreeTiles[10] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {4, 3}, {128, 128}, {1, 1}); // large dark green shrub
	m_TreeTiles[11] = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {5, 3}, {128, 128}, {1, 1}); // small dark green shrub

	m_TreeShadow = Hazel::SubTexture2D::CreateFromCoords(m_BackgroundSheet, {15, 11}, {128, 128}, {1, 1});

	// Player sprites
	m_PlayerSprites.resize(24);
	m_PlayerSprites[ 0] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {0, 2}, {80, 110}, {1, 1}); // idle 1
	m_PlayerSprites[ 1] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {1, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 2] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {2, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 3] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {3, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 4] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {4, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 5] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {5, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 6] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {6, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 7] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {7, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 8] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {8, 2}, {80, 110}, {1, 1});
	m_PlayerSprites[ 9] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {0, 1}, {80, 110}, {1, 1}); // walk
	m_PlayerSprites[10] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {1, 1}, {80, 110}, {1, 1}); // walk
	m_PlayerSprites[11] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {2, 1}, {80, 110}, {1, 1});
	m_PlayerSprites[12] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {3, 1}, {80, 110}, {1, 1});
	m_PlayerSprites[13] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {4, 1}, {80, 110}, {1, 1});
	m_PlayerSprites[14] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {5, 1}, {80, 110}, {1, 1});
	m_PlayerSprites[15] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {6, 1}, {80, 110}, {1, 1});
	m_PlayerSprites[16] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {7, 1}, {80, 110}, {1, 1});
	m_PlayerSprites[17] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {8, 1}, {80, 110}, {1, 1});
	m_PlayerSprites[18] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {0, 0}, {80, 110}, {1, 1});
	m_PlayerSprites[19] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {1, 0}, {80, 110}, {1, 1});
	m_PlayerSprites[20] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {2, 0}, {80, 110}, {1, 1});
	m_PlayerSprites[21] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {3, 0}, {80, 110}, {1, 1}); // down
	m_PlayerSprites[22] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {4, 0}, {80, 110}, {1, 1}); // up
	m_PlayerSprites[23] = Hazel::SubTexture2D::CreateFromCoords(m_PlayerSheet, {5, 0}, {80, 110}, {1, 1}); // idle 2

	m_PlayerSize = glm::vec2{80, 110} / glm::vec2{128, 128};

	m_PlayerAnimations.resize(static_cast<int>(PlayerState::NumStates));

	m_PlayerAnimations[static_cast<int>(PlayerState::Idle)] = {0, 0, 0, 0, 0, 23, 23, 23, 23, 23};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkLeft)] = {9, 9, 9, 9, 10, 10, 10, 10};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkRight)] = {9, 9, 9, 9, 10, 10, 10, 10};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkUp)] = {22};
	m_PlayerAnimations[static_cast<int>(PlayerState::WalkDown)] = {21};

	m_AspectRatio = static_cast<float>(Hazel::Application::Get().GetWindow().GetWidth()) / static_cast<float>(Hazel::Application::Get().GetWindow().GetHeight());
	m_Camera = Hazel::CreateScope<Hazel::OrthographicCamera>(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
}


void GroundLayer::OnDetach() {
	HZ_PROFILE_FUNCTION();
}


void GroundLayer::OnUpdate(Hazel::Timestep ts) {
	HZ_PROFILE_FUNCTION();

	UpdatePlayer(ts);

	glm::vec3 position = {m_PlayerPos, 0.0f};

	m_Camera->SetPosition(position);

	// TODO: what if overflow an int?  (unlikely, but still...)
	auto left = static_cast<int>(std::floor((-m_AspectRatio * m_Zoom) + position.x - 1.0f));
	auto right = static_cast<int>(std::ceil(m_AspectRatio * m_Zoom + position.x + 1.0f));
	auto bottom = static_cast<int>(std::floor(-m_Zoom + position.y - 2.0f));
	auto top = static_cast<int>(std::ceil(m_Zoom + position.y + 1.0f));

	int width = (right - left);
	int height = (top - bottom);

	std::vector<uint8_t> groundType;
	std::vector<uint8_t> treeType;
	std::vector<glm::vec3> treeCoords;
	std::vector<glm::vec2> treeScale;
	std::vector<glm::vec3> treeShadowCoords;
	std::vector<glm::vec2> treeShadowSize;
	groundType.reserve(width * height);
	treeType.reserve(width * height);
	treeCoords.reserve(width * height);
	treeScale.reserve(width * height);
	treeShadowCoords.reserve(width * height);
	treeShadowSize.reserve(width * height);
	{
		HZ_PROFILE_SCOPE("Generate map");

		// TODO: don't generate the whole thing every time, just add and remove bits as the "viewport" moves around

		// TODO: switch to "FasterNoise"  (aka. FastNoiseSIMD)

		// Ground
		for (int y = bottom; y < top; ++y) {
			for (int x = left; x < right; ++x) {
				float terrainValue = m_TerrainSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
				if (terrainValue < -0.1f) {
					groundType.emplace_back(0);          // water
				} else if (terrainValue < 0.4f) {
					groundType.emplace_back(1);          // grass
				} else {
					groundType.emplace_back(2);          // dirt
				}
			}
		}

		// Trees
		// The result is underwhelming.  Some sort of poisson disk sampling, with noise-dependent radius might be better
		// (with pre-generated level of fixed size)
		for (int y = bottom + 1; y < top; ++y) {
			for (int x = left + 1; x < right; ++x) {
				uint32_t index = ((y - bottom) * width) + (x - left);
				uint32_t groundTile = (27 * groundType[index - 1]) + (9 * groundType[index]) + (3 * groundType[index - width - 1]) + groundType[index - width];

				// trees don't grow on water tiles (any tile <= 39)
				if (groundTile == 40) {
					float treeValue = m_TreeSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
					Random treeRandomizer({x, y});
					if (treeValue > 0.45f) {
						// big tree
						if (treeRandomizer.Uniform0_1() < 0.3f) {
							float xOffset = treeRandomizer.Uniform(0.2f, 0.8f);
							float yOffset = treeRandomizer.Uniform(0.2f, 0.8f);
							float scale = treeRandomizer.Uniform(0.8f, 1.2f);
							treeType.emplace_back(0);
							treeCoords.emplace_back(x - xOffset, y - yOffset + scale, ((top - (y - yOffset)) / height / 10.0f) - 0.8f);
							treeScale.emplace_back(scale, 2.0f * scale);
							treeShadowCoords.emplace_back(x - xOffset, y - yOffset + 0.36 * scale, ((top - (y - yOffset)) / height / 10.0f) - 0.9f);
							treeShadowSize.emplace_back(1.2f * scale, 1.2f * scale);
						}
					} else if (treeValue > 0.0f) {
						// small tree
						if (treeRandomizer.Uniform0_1() < 0.2f) {
							float xOffset = treeRandomizer.Uniform(0.2f, 0.8f);
							float yOffset = treeRandomizer.Uniform(0.2f, 0.8f);
							float scale = treeRandomizer.Uniform(0.8f, 1.2f);
							treeType.emplace_back(1);
							treeCoords.emplace_back(x - xOffset, y - yOffset + scale, ((top - (y - yOffset)) / height / 10.0f) - 0.8f);
							treeScale.emplace_back(scale, 2.0f * scale);
							treeShadowCoords.emplace_back(x - xOffset, y - yOffset + 0.3f * scale, ((top - (y - yOffset)) / height / 10.0f) - 0.9f);
							treeShadowSize.emplace_back(scale, scale);
						}
					}
				} else if (groundTile > 40) {
					float treeValue = m_TreeSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
					Random treeRandomizer({x, y});
					if (treeValue > 0.7f) {
						// small orange shrub
						if (treeRandomizer.Uniform0_1() < 0.6f) {
							float xOffset = treeRandomizer.Uniform0_1();
							float yOffset = treeRandomizer.Uniform0_1();
							float scale = 1.0f;
							treeType.emplace_back(9);
							treeCoords.emplace_back(x - xOffset, y - yOffset, ((top - (y - yOffset)) / height / 10.0f) - 0.8f);
							treeScale.emplace_back(scale, scale);
							treeShadowCoords.emplace_back(x - xOffset, y - yOffset, ((top - (y - yOffset)) / height / 10.0f) - 0.9f);
							treeShadowSize.emplace_back(scale, scale);
						}
					} else if (treeValue > 0.0f) {
						// orange shrubs
						if (treeRandomizer.Uniform0_1() < 0.5f) {
							float xOffset = treeRandomizer.Uniform0_1();
							float yOffset = treeRandomizer.Uniform0_1();
							float scale = treeRandomizer.Uniform(0.8f, 1.2f);
							treeType.emplace_back(8);
							treeCoords.emplace_back(x - xOffset, y - yOffset, ((top - (y - yOffset)) / height / 10.0f) - 0.8f);
							treeScale.emplace_back(scale, scale);
							treeShadowCoords.emplace_back(x - xOffset, y - yOffset - 0.1f * scale, ((top - (y - yOffset)) / height / 10.0f) - 0.9f);
							treeShadowSize.emplace_back(scale, scale);
							if (treeRandomizer.Uniform0_1() < 0.5f) {
								float xOffset = treeRandomizer.Uniform0_1();
								float yOffset = treeRandomizer.Uniform0_1();
								float scale = treeRandomizer.Uniform(0.8f, 1.2f);
								treeType.emplace_back(9);
								treeCoords.emplace_back(x - xOffset, y - yOffset, ((top - (y - yOffset)) / height / 10.0f) - 0.8f);
								treeScale.emplace_back(scale, scale);
								treeShadowCoords.emplace_back(x - xOffset, y - yOffset - 0.21f * scale, ((top - (y - yOffset)) / height / 10.0f) - 0.9f);
								treeShadowSize.emplace_back(0.7f * scale, 0.7f * scale);
							}
						}
					}
				}
			}
		}
	}

	// Animate
	{
		//HZ_PROFILE_SCOPE("Animate");
		m_AnimationAccumulator += ts;
		if (m_AnimationAccumulator > 0.1f) {
			m_AnimationAccumulator = 0.0f;
			m_PlayerFrame = (++m_PlayerFrame) % m_PlayerAnimations[static_cast<int>(m_PlayerState)].size();
		}
	}

	// Render
	Hazel::Renderer2D::ResetStats();
	Hazel::Renderer2D::StatsBeginFrame();

	{
		HZ_PROFILE_SCOPE("Renderer Draw");

		Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
		Hazel::RenderCommand::Clear();

		Hazel::Renderer2D::BeginScene(*m_Camera);

		// Ground
		for (int y = 1; y < height; ++y) {
			for (int x = 1; x < width; ++x) {
				uint32_t index = (y * width) + x;
				uint32_t groundTile = (27 * groundType[index - 1]) + (9 * groundType[index]) + (3 * groundType[index - width - 1]) + groundType[index - width];
				if (groundTile == 40) {
					float grassValue = m_GrassSampler.GetNoise(static_cast<float>(x + left), static_cast<float>(y + bottom));
					if (grassValue < -0.2f) {
						groundTile = 40;
					} else if (grassValue < 0.2f) {
						groundTile = 81;
					} else {
						groundTile = 82;
					}
				}
				Hazel::Renderer2D::DrawQuad({x + left - 0.5f, y + bottom - 0.5f, -0.99f}, {1, 1}, m_GroundTiles[groundTile]);
			}
		}

		// Tree shadows
		for (size_t t = 0; t < treeCoords.size(); ++t) {
			Hazel::Renderer2D::DrawQuad(treeShadowCoords[t], treeShadowSize[t], m_TreeShadow);
		}

		// Trees
		for(size_t t = 0; t < treeCoords.size(); ++t) {
			Hazel::Renderer2D::DrawQuad(treeCoords[t], treeScale[t], m_TreeTiles[treeType[t]]);
		}

		// Player
		glm::vec3 playerPos = {m_PlayerPos, ((top - m_PlayerPos.y - 0.5f) / height / 10.0f) - 0.8f};
		Hazel::Renderer2D::DrawQuad(playerPos, m_PlayerSize, m_PlayerSprites[m_PlayerAnimations[static_cast<int>(m_PlayerState)][m_PlayerFrame]]);

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


void GroundLayer::UpdatePlayer(Hazel::Timestep ts) {
	HZ_PROFILE_FUNCTION();

	constexpr float moveSpeed = 1.5f;

	PlayerState lastState = m_PlayerState;
	m_PlayerState = PlayerState::Idle;
	if (Hazel::Input::IsKeyPressed(HZ_KEY_A)) {
		m_PlayerPos.x -= ts * moveSpeed;
		m_PlayerState = PlayerState::WalkLeft;
		m_PlayerSize = {-80.0f / 128.0f, 110.0f / 128.0f};
	} else if (Hazel::Input::IsKeyPressed(HZ_KEY_D)) {
		m_PlayerPos.x += ts * moveSpeed;
		m_PlayerState = PlayerState::WalkRight;
		m_PlayerSize = {80.0f / 128.0f, 110.0f / 128.0f};
	}

	if (Hazel::Input::IsKeyPressed(HZ_KEY_W)) {
		m_PlayerPos.y += ts * moveSpeed;
		m_PlayerState = PlayerState::WalkUp;
	} else if (Hazel::Input::IsKeyPressed(HZ_KEY_S)) {
		m_PlayerPos.y -= ts * moveSpeed;
		m_PlayerState = PlayerState::WalkDown;
	}

	if (m_PlayerState != lastState) {
		m_PlayerFrame = 0;
	}
}


void GroundLayer::OnImGuiRender() {
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


void GroundLayer::OnEvent(Hazel::Event& e) {

	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(GroundLayer::OnWindowResize));
}


bool GroundLayer::OnWindowResize(Hazel::WindowResizeEvent& e) {
	m_AspectRatio = static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight());
	m_Camera->SetProjection(-m_AspectRatio * m_Zoom, m_AspectRatio * m_Zoom, -m_Zoom, m_Zoom);
	return false;
}
