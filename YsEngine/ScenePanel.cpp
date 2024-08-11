#include "ScenePanel.h"

#include <iostream>

#include "imgui.h"
#include "ImGuizmo.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "FrameBuffer.h"
#include "Model.h"
#include "CameraBase.h"
#include "PlayerCamera.h"
#include "FreeCamera.h"
#include "Window.h"

ScenePanel::ScenePanel(FrameBuffer* fb, CameraBase* cam, Window* win) :
	sceneBuffer(fb), camera(cam), mainWindow(win), selectedEntity(nullptr)
{
	currOperation = ImGuizmo::OPERATION::TRANSLATE;

	width = -1;
	height = -1;
}

void ScenePanel::Update()
{
	// Render ImGui
	ImGui::Begin("Viewport", NULL);

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

	if(selectedEntity)
		DrawGizmo(pos);

	HandleInput();
	ImGui::End();
}

void ScenePanel::HandleInput()
{
	if (ImGui::IsWindowFocused())
	{
		PlayerCamera* pc = dynamic_cast<PlayerCamera*>(camera);
		if (pc)
			pc->SetCanMove(true);

		if (ImGui::IsKeyDown(ImGuiKey_MouseRight))
		{
			FreeCamera* fc = dynamic_cast<FreeCamera*>(camera);
			if(fc)
				fc->SetCanMove(true);
		}
		else
		{
			FreeCamera* fc = dynamic_cast<FreeCamera*>(camera);
			if (fc)
				fc->SetCanMove(false);

			if (ImGui::IsKeyPressed(ImGuiKey_W))
				currOperation = ImGuizmo::TRANSLATE;

			if (ImGui::IsKeyPressed(ImGuiKey_E))
				currOperation = ImGuizmo::ROTATE;

			if (ImGui::IsKeyPressed(ImGuiKey_R))
				currOperation = ImGuizmo::SCALE;
		}
	}
	else
	{
		camera->SetCanMove(false);
	}
}

void ScenePanel::SetEntity(Entity* e)
{
	selectedEntity = e;
}

void ScenePanel::DrawGizmo(ImVec2 pos)
{
	// Gizmos
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(pos.x, pos.y, width, height);

	glm::mat4 model = selectedEntity->GetModelMat();
	glm::mat4 view = camera->GetViewMatrix();
	const glm::mat4& projection = camera->GetProjectionMatrix(width, height);

	ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
		(ImGuizmo::OPERATION)currOperation, ImGuizmo::LOCAL, glm::value_ptr(model));

	if (ImGuizmo::IsUsing())
		selectedEntity->UpdateTransform(model);
}

ScenePanel::~ScenePanel()
{
}