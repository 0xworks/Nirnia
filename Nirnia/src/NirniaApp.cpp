#include "GroundLayer.h"

#include "Hazel/Core/Application.h"
#include "Hazel/Core/EntryPoint.h"

class NirniaApp : public Hazel::Application
{
public:
	NirniaApp()
	{
		PushLayer(new GroundLayer());
	}

};

Hazel::Application* Hazel::CreateApplication()
{
	return new NirniaApp();
}
