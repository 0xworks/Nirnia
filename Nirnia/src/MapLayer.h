#pragma once

#include "Hazel/Core/Layer.h"

class MapLayer : public Hazel::Layer
{
public:
	MapLayer();
	virtual ~MapLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Hazel::Timestep ts) override;

};
