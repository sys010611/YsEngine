#include "HierarchyPanel.h"

#include "imgui.h"
#include "Entity.h"

#include <string>

HierarchyPanel::HierarchyPanel(std::vector<Entity*>& entityList)
	: entityList(entityList)
{}

void HierarchyPanel::Update()
{
	ImGui::Begin("Hierarchy");

	for (auto& entity : entityList)
	{
		std::string name = entity->GetName();
		ImGui::Text(name.c_str());
	}

	ImGui::End();
}

void HierarchyPanel::UpdateEntityList(std::vector<Entity*>& entityList)
{
	this->entityList = entityList;
}
