#include "MapLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Renderer/RenderCommand.h"
#include "Hazel/Renderer/Renderer2D.h"

#include <GLFW/glfw3.h>

MapLayer::MapLayer()
: Layer("Map")
{}


void MapLayer::OnAttach() {
}

void MapLayer::OnDetach() {
}

void MapLayer::OnUpdate(Hazel::Timestep ts) {


	Hazel::Renderer2D::ResetStats();
	Hazel::Renderer2D::StatsBeginFrame();

	// Update (TODO)

	// Animate (TODO)

	// Render (TODO)
	Hazel::RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 1});
	Hazel::RenderCommand::Clear();

	//Hazel::Renderer2D::BeginScene(/* camera here */);

// 	for (size_t row = minRow; row < maxRow; ++row) {
// 		for (size_t col = minCol; col < maxCol; ++col) {
// 			GameObject& object = m_Level.GetGameObject(row, col);
// 			glm::vec3 position(col - m_ViewPort.GetLeft() + 0.5f, row - m_ViewPort.GetBottom() + 0.5f, 0.0f);
// 			Hazel::Renderer2D::DrawQuad(position, m_TileSize, m_Tiles[(int)object.GetTile()]);
// 			if (object.IsPlayer()) {
// 				m_ViewPort.SetPlayerPosition((float)col, (float)row);
// 				if ((row == m_Level.GetExitRow()) && (col == m_Level.GetExitCol())) {
// 					m_WonLevel = true;
// 				}
// 			}
// 		}
// 	}

	//Hazel::Renderer2D::EndScene();
	Hazel::Renderer2D::StatsEndFrame();

	auto stats = Hazel::Renderer2D::GetStats();
	float averageRenderTime = stats.TotalFrameRenderTime / stats.FrameRenderTime.size(); // nb: wont be accurate until we have gathered at least stats.FrameRenderTime().size() results
	float averageFPS = 1.0f / averageRenderTime;
	char buffer[64];
	sprintf_s(buffer, 64, "Average frame render time: %8.5f (%5.0f fps)", averageRenderTime, averageFPS);
	glfwSetWindowTitle((GLFWwindow*)Hazel::Application::Get().GetWindow().GetNativeWindow(), buffer);
}
