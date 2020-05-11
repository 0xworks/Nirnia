#include "GroundLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>


GroundLayer::GroundLayer()
: Layer("Map")
{
	//
	// loads of options here
	m_NoiseSampler.SetFrequency(0.01);                        // Default 0.1
	m_NoiseSampler.SetInterp(FastNoise::Quintic);            // Default: Quintic
	m_NoiseSampler.SetNoiseType(FastNoise::Simplex);         // Default: Simplex  (SimplexFractal is good for height-maps, but that's not really what we're after here)

	// Fractal noise only
	m_NoiseSampler.SetFractalOctaves(3);                     // Default 3
	m_NoiseSampler.SetFractalLacunarity(2.0);                // Default 2.0
	m_NoiseSampler.SetFractalGain(0.5);                      // Default 0.5  (otherwise known as "persistence")
	m_NoiseSampler.SetFractalType(FastNoise::FBM);           // Default: FBM

	// + others for Cellular noise...


	// note: defer creation of camera controller until OnAttach(), so we know the correct window size.

}


void GroundLayer::OnAttach() {
	HZ_PROFILE_FUNCTION();
	m_SpriteSheet = Hazel::Texture2D::Create("assets/textures/RPGpack_sheet_2X.png");
	m_Dirt = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {6,11}, {128,128});
	m_Grass = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {1,11}, {128,128});
	m_Water = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11,11}, {128,128});

	// HACK: we need to remember width and height because there is currently no way to retrieve
	//       these from the camera controller later.
	m_AspectRatio = static_cast<float>(Hazel::Application::Get().GetWindow().GetWidth()) / static_cast<float>(Hazel::Application::Get().GetWindow().GetHeight());
	m_CameraController = Hazel::CreateScope<Hazel::OrthographicCameraController>(m_AspectRatio);

	float zoom = 4.0f;
	m_CameraController->SetZoomLevel(zoom);
	m_CameraController->GetCamera().SetProjection(-m_AspectRatio * zoom, m_AspectRatio * zoom, -zoom, zoom); // BUG: camera controller does not do this when you SetZoomLevel()
}


void GroundLayer::OnDetach() {
	HZ_PROFILE_FUNCTION();
}


void GroundLayer::OnUpdate(Hazel::Timestep ts) {
	HZ_PROFILE_FUNCTION();

	// Update
	m_CameraController->OnUpdate(ts);

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
		Hazel::Renderer2D::BeginScene(m_CameraController->GetCamera());

		float zoom = m_CameraController->GetZoomLevel();
		glm::vec3 position = m_CameraController->GetCamera().GetPosition();
		
		float left = std::floor((-m_AspectRatio * zoom) + position.x) + 0.5f;
		float right = std::ceil(m_AspectRatio * zoom + position.x) + 0.5f;
		float bottom = std::floor(-zoom + position.y) + 0.5f;
		float top = std::ceil(zoom + position.y) + 0.5f;


		for (float y = top; y >= bottom; --y) {
			for (float x = left; x < right; ++x) {
				float noiseValue = m_NoiseSampler.GetNoise(x, y);
				Hazel::Ref<Hazel::SubTexture2D> subTexture = m_Grass;
				if (noiseValue < 0.005) {
					subTexture = m_Water;
				} else if (noiseValue < 0.2) {
					subTexture = m_Dirt;
				}
				Hazel::Renderer2D::DrawQuad({x,y}, {1, 1}, subTexture);
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

	Hazel::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(GroundLayer::OnWindowResize));

	m_CameraController->OnEvent(e);
}


// HACK: we need this because there is currently no way to retrieve width/height from the camera controller
bool GroundLayer::OnWindowResize(Hazel::WindowResizeEvent& e) {
	m_AspectRatio = static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight());
	return false;
}
