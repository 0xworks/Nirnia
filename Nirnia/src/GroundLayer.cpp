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
	m_TerrainSampler.SetFrequency(0.02f);                      // Default 0.1
	m_TerrainSampler.SetInterp(FastNoise::Quintic);            // Default: Quintic
	m_TerrainSampler.SetNoiseType(FastNoise::SimplexFractal);  // Default: Simplex

	// Fractal noise only
	m_TerrainSampler.SetFractalOctaves(3);                     // Default 3
	m_TerrainSampler.SetFractalLacunarity(2.0);                // Default 2.0
	m_TerrainSampler.SetFractalGain(0.5);                      // Default 0.5  (otherwise known as "persistence")
	m_TerrainSampler.SetFractalType(FastNoise::FBM);           // Default: FBM

	// + others for Cellular noise...

	m_GrassTypeSampler.SetNoiseType(FastNoise::WhiteNoise);

	// note: defer creation of camera controller until OnAttach(), so we know the correct window size.

}


void GroundLayer::OnAttach() {
	HZ_PROFILE_FUNCTION();
	m_SpriteSheet = Hazel::Texture2D::Create("assets/textures/RPGpack_sheet_2X.png");

	m_SubTextures.resize(83);                                                                        //  TL    TR    BL    BR
	m_SubTextures[ 0] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11, 11}, {128, 128});  // water water water water
	m_SubTextures[ 1] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {13, 12}, {128, 128});  // water water water grass
	m_SubTextures[ 2] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water water water dirt  => X
	m_SubTextures[ 3] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {14, 12}, {128, 128});  // water water grass water
	m_SubTextures[ 4] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11, 10}, {128, 128});  // water water grass grass
	m_SubTextures[ 5] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water water grass dirt  => X
	m_SubTextures[ 6] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water water dirt water  => X
	m_SubTextures[ 7] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water water dirt grass  => X
	m_SubTextures[ 8] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water water dirt dirt   => X
	m_SubTextures[ 9] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {13, 11}, {128, 128});  // water grass water water
	m_SubTextures[10] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {12, 11}, {128, 128});  // water grass water grass
	m_SubTextures[11] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water grass water dirt  => X
	m_SubTextures[12] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water grass grass water => X
	m_SubTextures[13] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {12, 10}, {128, 128});  // water grass grass grass
	m_SubTextures[14] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water grass grass dirt  => X
	m_SubTextures[15] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water grass dirt water  => X
	m_SubTextures[16] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water grass dirt grass  => X
	m_SubTextures[17] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water grass dirt dirt   => X
	m_SubTextures[18] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt water water  => X
	m_SubTextures[19] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt water grass  => X
	m_SubTextures[20] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt water dirt   => X
	m_SubTextures[21] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt grass water  => X
	m_SubTextures[22] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt grass grass  => X
	m_SubTextures[23] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt grass dirt   => X
	m_SubTextures[24] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt dirt water   => X
	m_SubTextures[25] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt dirt grass   => X
	m_SubTextures[26] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // water dirt dirt dirt    => X
	m_SubTextures[27] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {14, 11}, {128, 128});  // grass water water water
	m_SubTextures[28] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass water water grass => X
	m_SubTextures[29] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass water water dirt  => X
	m_SubTextures[30] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {10, 11}, {128, 128});  // grass water grass water
	m_SubTextures[31] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {10, 10}, {128, 128});  // grass water grass grass
	m_SubTextures[32] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass water grass dirt  => X
	m_SubTextures[33] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass water dirt water  => X
	m_SubTextures[34] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass water dirt grass  => X
	m_SubTextures[35] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass water dirt dirt   => X
	m_SubTextures[36] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {11, 12}, {128, 128});  // grass grass water water
	m_SubTextures[37] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {12, 12}, {128, 128});  // grass grass water grass
	m_SubTextures[38] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass grass water dirt  => X
	m_SubTextures[39] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {10, 12}, {128, 128});  // grass grass grass water
	m_SubTextures[40] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 1, 11}, {128, 128});  // grass grass grass grass
	m_SubTextures[41] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 5, 12}, {128, 128});  // grass grass grass dirt
	m_SubTextures[42] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass grass dirt water  => X
	m_SubTextures[43] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 12}, {128, 128});  // grass grass dirt grass
	m_SubTextures[44] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 12}, {128, 128});  // grass grass dirt dirt
	m_SubTextures[45] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass dirt water water  => X
	m_SubTextures[46] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass dirt water grass  => X
	m_SubTextures[47] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass dirt water dirt   => X
	m_SubTextures[48] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass dirt grass water  => X
	m_SubTextures[49] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 5, 10}, {128, 128});  // grass dirt grass grass
	m_SubTextures[50] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 5, 11}, {128, 128});  // grass dirt grass dirt
	m_SubTextures[51] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass dirt dirt water   => X
	m_SubTextures[52] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // grass dirt dirt grass   => X
	m_SubTextures[53] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 9, 11}, {128, 128});  // grass dirt dirt dirt
	m_SubTextures[54] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water water water  => X
	m_SubTextures[55] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water water grass  => X
	m_SubTextures[56] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water water dirt   => X
	m_SubTextures[57] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water grass water  => X
	m_SubTextures[58] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water grass grass  => X
	m_SubTextures[59] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water grass dirt   => X
	m_SubTextures[60] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water dirt water   => X
	m_SubTextures[61] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water dirt grass   => X
	m_SubTextures[62] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt water dirt dirt    => X
	m_SubTextures[63] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt grass water water  => X
	m_SubTextures[64] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt grass water grass  => X
	m_SubTextures[65] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt grass water dirt   => X
	m_SubTextures[66] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt grass grass water  => X
	m_SubTextures[67] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 10}, {128, 128});  // dirt grass grass grass
	m_SubTextures[68] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt grass grass dirt  => X
	m_SubTextures[69] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt grass dirt water  => X
	m_SubTextures[70] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 7, 11}, {128, 128});  // dirt grass dirt grass
	m_SubTextures[71] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8, 11}, {128, 128});  // dirt grass dirt dirt
	m_SubTextures[72] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt dirt water water  => X
	m_SubTextures[73] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt dirt water grass  => X
	m_SubTextures[74] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt dirt water dirt   => X
	m_SubTextures[75] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt dirt grass water  => X
	m_SubTextures[76] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 10}, {128, 128});  // dirt dirt grass grass
	m_SubTextures[77] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 9, 12}, {128, 128});  // dirt dirt grass dirt
	m_SubTextures[78] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 0,  0}, {128, 128});  // dirt dirt dirt water   => X
	m_SubTextures[79] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 8, 12}, {128, 128});  // dirt dirt dirt grass
	m_SubTextures[80] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, { 6, 11}, {128, 128});  // dirt dirt dirt dirt

	// There's a couple of other "grass" tiles
	m_SubTextures[81] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {3, 10}, {128, 128});  // grass grass grass grass
	m_SubTextures[82] = Hazel::SubTexture2D::CreateFromCoords(m_SpriteSheet, {4, 10}, {128, 128});  // grass grass grass grass


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

	float zoom = m_CameraController->GetZoomLevel();
	glm::vec3 position = m_CameraController->GetCamera().GetPosition();

	// TODO: what if overflow an int?  (unlikely, but still...)
	int left = static_cast<int>(std::floor((-m_AspectRatio * zoom) + position.x - 1.0f));
	int right = static_cast<int>(std::ceil(m_AspectRatio * zoom + position.x + 1.0f));
	int bottom = static_cast<int>(std::floor(-zoom + position.y - 1.0f));
	int top = static_cast<int>(std::ceil(zoom + position.y + 1.0f));

	int width = (right - left);
	int height = (top - bottom);

	std::vector<float> tiles;
	tiles.reserve(width * height);
	{
		HZ_PROFILE_SCOPE("Generate map");

		// TODO: don't generate the whole thing every time, just add and remove bits as the "viewport" moves around

		// generate from raw noise
		for (int y = top - 1; y >= bottom; --y) {
			for (int x = left; x < right; ++x) {
				float terrainValue = m_TerrainSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
				if (terrainValue < 0.005) {
					tiles.emplace_back(0.0f);       // water
				} else if (terrainValue < 0.45) {
					float grassValue = m_GrassTypeSampler.GetNoise(static_cast<float>(x), static_cast<float>(y));
					if (grassValue < 0.33) {
						tiles.emplace_back(1.0f);       // grass
					} else if (grassValue < 0.66) {
						tiles.emplace_back(1.1f);     // also grass
					} else {
						tiles.emplace_back(1.25f);    // also grass
					}
				} else {
					tiles.emplace_back(2.0f);       // dirt
				}
			}
		}
	}

	// Render
	Hazel::Renderer2D::ResetStats();
	Hazel::Renderer2D::StatsBeginFrame();

	{
		HZ_PROFILE_SCOPE("Renderer Draw");

		Hazel::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
		Hazel::RenderCommand::Clear();

		Hazel::Renderer2D::BeginScene(m_CameraController->GetCamera());

		for (int y = height - 1; y > 0; --y) {
			for (int x = 1; x < width; ++x) {
				uint32_t index = ((height - y) * width) + x;
				uint32_t tile = (27 * static_cast<uint32_t>(tiles[index - width - 1])) + (9 * static_cast<uint32_t>(tiles[index - width])) + (3 * static_cast<uint32_t>(tiles[index - 1])) + static_cast<uint32_t>(tiles[index]);
				if (tile == 40) {
					float alternativeGrass = tiles[index - width - 1] + tiles[index - width] + tiles[index - 1] + tiles[index];
					if (alternativeGrass < 4.2) {
						tile = 40;
					} else if (alternativeGrass < 4.5) {
						tile = 81;
					} else {
						tile = 82;
					}
				}
				Hazel::Renderer2D::DrawQuad({x + left + 0.5, y + bottom + 0.5}, {1, 1}, m_SubTextures[tile]);
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
