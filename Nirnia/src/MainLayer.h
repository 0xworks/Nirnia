#pragma once

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
	void InitGroundTiles();
	void InitPlayer();
	void InitCamera();
	void InitMap();
	void GenerateMapChunk(const int i, const int j);
	bool OnWindowResize(Hazel::WindowResizeEvent& e);

	void UpdatePlayer(Hazel::Timestep ts);

private:
	Random m_Random;
	FastNoise m_TerrainSampler;
	FastNoise m_GrassSampler;
	FastNoise m_TreeSampler;

	Hazel::Scope<Hazel::OrthographicCamera> m_Camera;
	uint32_t m_ViewportWidth;
	uint32_t m_ViewportHeight;

	Hazel::Ref<Hazel::Texture2D> m_BackgroundSheet;
	Hazel::Ref<Hazel::Texture2D> m_PlayerSheet;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_GroundTiles;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_TreeTiles;
	Hazel::Ref<Hazel::SubTexture2D> m_TreeShadow;

	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_PlayerSprites;
	std::vector<std::vector<uint8_t>> m_PlayerAnimations;

	uint32_t m_ChunkWidth;
	uint32_t m_ChunkHeight;
	std::unordered_map<std::pair<int, int>, std::vector<uint8_t>> m_GroundType;
	std::unordered_map<std::pair<int, int>, std::vector<uint8_t>> m_TreeType;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec3>> m_TreeCoords;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec2>> m_TreeScale;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec3>> m_TreeShadowCoords;
	std::unordered_map<std::pair<int, int>, std::vector<glm::vec2>> m_TreeShadowSize;

	glm::vec2 m_PlayerPos;
	glm::vec2 m_PlayerSize;
	PlayerState m_PlayerState;
	uint32_t m_PlayerFrame;

	float m_AspectRatio = 1.0f;
	float m_Zoom = 4.0f;
	float m_AnimationAccumulator = 0.0f;

};
