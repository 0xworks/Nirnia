#pragma once

#include <Hazel/Core/Layer.h>
#include <Hazel/Renderer/OrthographicCameraController.h>
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

private:
	FastNoise m_TerrainSampler;
	FastNoise m_GrassSampler;
	FastNoise m_TreeSampler;

	Hazel::Scope<Hazel::OrthographicCameraController> m_CameraController;

	Hazel::Ref<Hazel::Texture2D> m_BackgroundSheet;
	Hazel::Ref<Hazel::Texture2D> m_PlayerSheet;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_GroundTiles;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_TreeTiles;
	std::vector<Hazel::Ref<Hazel::SubTexture2D>> m_PlayerSprites;
	std::vector<std::vector<uint8_t>> m_PlayerAnimations;
	std::uint32_t m_PlayerAnimation = 0;
	std::uint32_t m_PlayerFrame = 0;

	Hazel::Ref<Hazel::SubTexture2D> m_TreeShadow;

	float m_AspectRatio = 1.0f;
	float m_AnimationAccumulator = 0.0f;
};
