#pragma once

#include <vector>
#include "Panel.h"

class Entity;

class HierarchyPanel : Panel
{
public:
	HierarchyPanel(std::vector<Entity*>& entityList);
	
	virtual void Update() override;

	void UpdateEntityList(std::vector<Entity*>& entityList);
private:
	std::vector<Entity*>& entityList;
};

