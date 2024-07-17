#include "ScenePanel.h"

#include <iostream>

#include "imgui.h"
#include "ImGuizmo.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "FrameBuffer.h"
#include "Model.h"
#include "Camera.h"
#include "Window.h"

ScenePanel::ScenePanel(FrameBuffer* fb, Model* md, Camera* cam, Window* win) :
	sceneBuffer(fb), currModel(md), camera(cam), mainWindow(win)
{
	currOperation = ImGuizmo::OPERATION::TRANSLATE;

	width = -1;
	height = -1;
}

void ScenePanel::Update()
{
	// Render ImGui
	ImGui::Begin("Scene", NULL);

	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float window_width = ImGui::GetContentRegionAvail().x;
	const float window_height = ImGui::GetContentRegionAvail().y;

	width = window_width;
	height = window_height;

	// 프레임버퍼 텍스처를 ImGui 윈도우에 렌더링
	ImGui::GetWindowDrawList()->AddImage(
		(void*)(intptr_t)sceneBuffer->getFrameTexture(),
		ImVec2(pos.x, pos.y),
		ImVec2(pos.x + window_width, pos.y + window_height),
		ImVec2(0, 1),
		ImVec2(1, 0)
	);
	{
		// Gizmos
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(pos.x, pos.y, window_width, window_height);

		glm::mat4 model = currModel->GetModelMat();
		glm::mat4 view = camera->GetViewMatrix();
		const glm::mat4& projection = camera->GetProjectionMatrix(window_width, window_height);

		ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
			(ImGuizmo::OPERATION)currOperation, ImGuizmo::LOCAL, glm::value_ptr(model));

		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation, rotation, scale;
			ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), &translation[0], &rotation[0], &scale[0]);

			currModel->SetTranslate(translation);
			currModel->SetRotate(rotation);
			currModel->SetScale(scale);
		}
	}

	HandleInput();
	ImGui::End();
}

void ScenePanel::HandleInput()
{
	if (ImGui::IsWindowFocused())
	{
		if (ImGui::IsKeyDown(ImGuiKey_MouseRight))
		{
			camera->SetCanMove(true);
		}
		else
		{
			camera->SetCanMove(false);

			if (ImGui::IsKeyPressed(ImGuiKey_W))
				currOperation = ImGuizmo::TRANSLATE;

			if (ImGui::IsKeyPressed(ImGuiKey_E))
				currOperation = ImGuizmo::ROTATE;

			if (ImGui::IsKeyPressed(ImGuiKey_R))
				currOperation = ImGuizmo::SCALE;
		}
	}
}

ScenePanel::~ScenePanel()
{

}
