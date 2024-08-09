#include "PointLight.h"

#include "imgui.h"
#include "ImGuizmo.h"

void PointLight::ShowProperties()
{
	// Transform
	ImGui::Text("Transform");

	ImGui::InputFloat3("Position", &position[0]);
	ImGui::InputFloat3("Color", &color[0]);
	ImGui::SliderFloat("Ambient", GetAmbientIntensity(), 0.f, 5.f);
	ImGui::SliderFloat("Diffuse", GetDiffuseIntensity(), 0.f, 5.f);

	// Range
	ImGui::Text("Decreasement");
	ImGui::SliderFloat("Constant", &constant, 0.f, 1.f);
	ImGui::SliderFloat("Linear", &linear, 0.f, 1.f);
	ImGui::SliderFloat("Exponent", &exponent, 0.f, 1.f);
}

glm::mat4 PointLight::GetModelMat()
{
	return modelMat = glm::translate(glm::mat4(1.f), position);
}

void PointLight::UpdateTransform(glm::mat4 newModelMat)
{
	glm::vec3 translation, rotation, scale;
	ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(newModelMat), &translation[0], &rotation[0], &scale[0]);

	position = translation;
}
