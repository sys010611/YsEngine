#pragma once
#include "Panel.h"

class Model;
class DirectionalLight;

class InspectorPanel: public Panel
{
public:
	InspectorPanel(Model* md, DirectionalLight* dl) : 
		currModel(md), dirLight(dl)
	{}

	virtual void Update() override;

	~InspectorPanel();

private:
	Model* currModel;
	DirectionalLight* dirLight;
};

