#pragma once

#include <Hazel/Core/Layer.h>
#include <Hazel/Renderer/OrthographicCameraController.h>
#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/SubTexture2D.h>
#include <Hazel/Renderer/VertexArray.h>

#include <glm/glm.hpp>

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
	Hazel::OrthographicCameraController m_CameraController;

	Hazel::Ref<Hazel::Texture2D> m_SpriteSheet;
	Hazel::Ref<Hazel::SubTexture2D> m_Grass;

};
