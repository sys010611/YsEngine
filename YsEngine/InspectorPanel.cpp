#include "InspectorPanel.h"

#include "imgui.h"

#include "Model.h"
#include "DirectionalLight.h"
#include "Material.h"

void InspectorPanel::Update()
{
	ImGui::Begin("Inspector");

	// Model
	ImGui::Text("Model");

	ImGui::InputFloat3("Translate", currModel->GetTranslate());
	ImGui::InputFloat3("Rotate", currModel->GetRotate());
	ImGui::InputFloat3("Scale", currModel->GetScale());

	// Directional Light
	ImGui::Text("DirectionalLight");

	ImGui::SliderFloat("Ambient", dirLight->GetAmbientIntensity(), 0.f, 5.f);
	ImGui::SliderFloat("Diffuse", dirLight->GetDiffuseIntensity(), 0.f, 5.f);

	// Material
	Material* currMaterial = currModel->GetMaterial();
	ImGui::Text("Material");

	ImGui::SliderFloat("Specular", &currMaterial->specular, 0.f, 5.f);
	ImGui::SliderFloat("Shininess", &currMaterial->shininess, 0.f, 512.f);

	ImGui::End();
}

InspectorPanel::~InspectorPanel()
{
}
