#pragma once

#include "NavMesh2D.h"
#include "PlayerState.h"
#include "Random.h"

#include <Hazel/Core/Layer.h>
#include <Hazel/Renderer/OrthographicCamera.h>
#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/SubTexture2D.h>
#include <Hazel/Renderer/VertexArray.h>

// HACK: (see comments in OnWindowResize)
#include <Hazel/Events/ApplicationEvent.h>

#include "FastNoise.h"

#include <glm/glm.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>


namespace std {

	template<typename T>
	void
		hash_combine(size_t& seed, T const& key) {
		std::hash<T> hasher;
		seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}


	template<typename T1, typename T2>
	struct hash<pair<T1, T2>> {
		size_t operator()(pair<T1, T2> const& p) const {
			size_t seed(0);
			hash_combine(seed, p.first);
			hash_combine(seed, p.second);
			return seed;
		}
	};

}


class MainLayer : public Hazel::Layer
{
public:
	MainLayer();
	virtual ~MainLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Hazel::Timestep ts) override;

	virtual void OnImGuiRender() override;

	void OnEvent(Hazel::Event& e) override;

private:
	struct TreeProperties {
		glm::vec3 Position;
		glm::vec3 ShadowPosition;
		glm::vec2 Size;
		glm::vec2 ShadowSize;
		uint8_t Type;
	};

private:
	void InitGroundTextures();
	void InitPlayer();
	void InitCamera();
	void InitMap();
	
	// Submits work to the chunk generator and returns immediately
	void GenerateMapChunk(const int i, const int j);

	TreeProperties GenerateTree(uint8_t tree, const int x, const int y, const int chunkTop, Random& treeRandomizer);

	// Generates the map chunks (on a worker thread)
	void ChunkGenerator();

	// Submits work to the chunk eraser and returns immediately
	void EraseMapChunk(const int i, const int j);

	// Erases map chunks (on a worker thread)
	void ChunkEraser();

	bool OnWindowResize(Hazel::WindowResizeEvent& e);

	void UpdatePlayer(Hazel::Timestep ts, const std::pair<int, int> chunk);

	bool IsGrass(uint8_t groundTile) { return (groundTile == 40) || (groundTile == 81) || (groundTile == 82); }
	bool IsDirt(uint8_t groundTile) { return (groundTile >= 41) && (groundTile < 81); }
	bool IsWater(uint8_t groundTile) { return groundTile < 40; }

private:
	Random m_Random;
	FastNoise m_TerrainSampler;
	FastNoise m_GrassSampler;
	FastNoise m_TreeSampler;

	Hazel::Scope<Hazel::OrthographicCamera> m_Camera;
	int m_ViewportWidth;
	int m_ViewportHeight;

	Hazel::Ref<Hazel::Texture2D> m_BackgroundSheet;
	Hazel::Ref<Hazel::Texture2D> m_PlayerSheet;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_GroundTextures;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_TreeTextures;
	Hazel::Ref<Hazel::SubTexture2D> m_TreeShadowTexture;
	std::vector<float> m_TreeBottomYOffsets; // distance from bottom of tree to centre of tile.  Used for positioning tree within ground tiles, and for computing tree "z"
	std::vector<float> m_TreeYScale;         // scale tree depending on tree type
	std::vector<float> m_TreeShadowScale;    // scale shadow depending on tree type
	constexpr static float m_TreeShadowYOffset = 39.0f / 128.0f;  // distance from centre of shadow in asset to centre of tile.  Used to correctly position the shadow texture within the ground tiles
	constexpr static float m_PlayerYOffset = 44.0f / 128.0f;      // distance from centre of player sprite to players feet.  Player position is the position of the feet.

	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_PlayerSprites;
	std::vector<std::vector<uint8_t>> m_PlayerAnimations;

	bool m_StopThreads;                                           // Setting this to true will terminate helper threads (e.g. the Chunk Generator thread)
	std::thread m_ChunkGenerator;                                 // Thread is started in OnAttach(), and runs until m_StopThreads is true.  Need to store this thread handle so that OnDetach() can wait for exit.
	std::thread m_ChunkEraser;                                    // Thread is started in OnAttach(), and runs until m_StopThreads is true.  Need to store this thread handle so that OnDetach() can wait for exit.
	HZ_PROFILE_LOCK(std::mutex, m_ChunkMutex, "Chunk Mutex");     // Synch access to chunk data
	std::condition_variable_any m_ChunkGeneratorCV;               // Notified when there are some chunks that require generation
	std::condition_variable_any m_ChunkEraserCV;                  // Notified when there are some chunks that require erasure
	std::unordered_set<std::pair<int, int>> m_ChunksToGenerate;   // "queue" of chunks to generate. (implemented as a set.  It doesn't matter what order we do them in, and unordered_set makes it easy and efficient to prevent adding same chunk more than once)
	std::unordered_set<std::pair<int, int>> m_ChunksToErase;      // "queue" of chunks to erase. (implemented as a set.  It doesn't matter what order we do them in, and unordered_set makes it easy and efficient to prevent adding same chunk more than once)

	int m_ChunkWidth;
	int m_ChunkHeight;
	std::unordered_map<std::pair<int, int>, std::vector<uint8_t>> m_GroundType;
	std::unordered_map<std::pair<int, int>, std::vector<uint8_t>> m_TreeType;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec3>> m_TreePositions;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec2>> m_TreeSize;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec3>> m_TreeShadowPositions;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec2>> m_TreeShadowSize;
	std::unordered_map<std::pair<int, int>, NavMesh2D> m_NavMesh;

	glm::vec2 m_PlayerPos;
	PlayerState m_PlayerState;
	uint32_t m_PlayerFrame;
	glm::vec2 m_PlayerSize;
	constexpr static float m_PlayerMovementSpeed = 1.5f;

	std::pair<int, int> m_PrevChunk;

	float m_AspectRatio = 1.0f;
	float m_Zoom = 4.0f;
	float m_AnimationAccumulator = 0.0f;

};
