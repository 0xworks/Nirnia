#pragma once

#include <Hazel/Core/Layer.h>
#include <Hazel/Renderer/OrthographicCameraController.h>
#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Texture.h>
#include <Hazel/Renderer/VertexArray.h>

#include <glm/glm.hpp>

class MapLayer : public Hazel::Layer
{
public:
	MapLayer();
	virtual ~MapLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Hazel::Timestep ts) override;

	virtual void OnImGuiRender() override;

	void OnEvent(Hazel::Event& e) override;

private:
	Hazel::OrthographicCameraController m_CameraController;

	// Temp
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;
	Hazel::Ref<Hazel::Shader> m_FlatColorShader;

	Hazel::Ref<Hazel::Texture2D> m_CheckerboardTexture;

	glm::vec4 m_SquareColor = {0.2f, 0.3f, 0.8f, 1.0f};

};
