#pragma once

#include "PlayerState.h"

#include <Hazel/Core/Layer.h>
#include <Hazel/Renderer/OrthographicCamera.h>
#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/SubTexture2D.h>
#include <Hazel/Renderer/VertexArray.h>

// HACK: (see comments in OnWindowResize)
#include <Hazel/Events/ApplicationEvent.h>

#include "FastNoise.h"

#include <glm/glm.hpp>

#include <vector>

class GroundLayer : public Hazel::Layer
{
public:
	GroundLayer();
	virtual ~GroundLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Hazel::Timestep ts) override;

	virtual void OnImGuiRender() override;

	void OnEvent(Hazel::Event& e) override;

private:
	bool OnWindowResize(Hazel::WindowResizeEvent& e);

	void UpdatePlayer(Hazel::Timestep ts);

private:
	FastNoise m_TerrainSampler;
	FastNoise m_GrassSampler;
	FastNoise m_TreeSampler;

	Hazel::Scope<Hazel::OrthographicCamera> m_Camera;

	Hazel::Ref<Hazel::Texture2D> m_BackgroundSheet;
	Hazel::Ref<Hazel::Texture2D> m_PlayerSheet;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_GroundTiles;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_TreeTiles;
	Hazel::Ref<Hazel::SubTexture2D> m_TreeShadow;

	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_PlayerSprites;
	std::vector<std::vector<uint8_t>> m_PlayerAnimations;
	glm::vec2 m_PlayerPos = {0.0f, 0.0f};
	glm::vec2 m_PlayerSize = {1.0f, 1.0f};
	PlayerState m_PlayerState;
	std::uint32_t m_PlayerFrame = 0;

	float m_AspectRatio = 1.0f;
	float m_Zoom = 4.0f;
	float m_AnimationAccumulator = 0.0f;
};
