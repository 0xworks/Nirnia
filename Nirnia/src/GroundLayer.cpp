#include "GroundLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

GroundLayer::GroundLayer()
: Layer("Map")
, m_CameraController(1280.0f / 720.0f)
{}


void GroundLayer::OnAttach() {
	HZ_PROFILE_FUNCTION();
	m_SpriteSheet = Hazel::Texture2D::Create("assets/textures/RPGpack_sheet_2X.png");
	m_Grass = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {3,10}, {128,128});

	m_Noise = GenerateNoise(m_LevelWidth, m_LevelHeight, 0.15f);
}


void GroundLayer::OnDetach() {
	HZ_PROFILE_FUNCTION();
}


void GroundLayer::OnUpdate(Hazel::Timestep ts) {
	HZ_PROFILE_FUNCTION();

	// Update
	m_CameraController.OnUpdate(ts);

	// Render
	Hazel::Renderer2D::ResetStats();
	Hazel::Renderer2D::StatsBeginFrame();
	{
		HZ_PROFILE_SCOPE("Renderer Prep");
		Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
		Hazel::RenderCommand::Clear();
	}

	{
		HZ_PROFILE_SCOPE("Renderer Draw");
		Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

		for (uint32_t y = 0; y < m_LevelHeight; ++y) {
		 	for (uint32_t x = 0; x < m_LevelWidth; ++x) {
				float noiseValue = m_Noise[(y * m_LevelWidth) + x];
				Hazel::Renderer2D::DrawQuad({x,y}, {1, 1}, glm::vec4 {noiseValue, noiseValue, noiseValue, 1.0f});
		 	}
		}
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
	m_CameraController.OnEvent(e);
}


std::vector<float> GroundLayer::GenerateNoise(uint32_t width, uint32_t height, float scale) {
	std::vector<float> noise(width * height, 0.0f);

	if (scale <= 0.0f) {
		scale = 0.00001f;
	}

	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			float sampleX = static_cast<float>(x) / scale;
			float sampleY = static_cast<float>(y) / scale;
			noise[(y * width) + x] = m_NoiseSampler.GetSimplex(sampleX, sampleY);
		}
	}

	return noise;
}
