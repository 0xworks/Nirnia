#include "MainLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Core/EntryPoint.h"

#include <GLFW/glfw3.h>

class NirniaApp : public Hazel::Application
{
public:
	NirniaApp() : Hazel::Application(Hazel::WindowProps{"Nirnia", 1280, 720, 640, 640})
	{
		PushLayer(new MainLayer());
	}

};

Hazel::Application* Hazel::CreateApplication()
{
	return new NirniaApp();
}
