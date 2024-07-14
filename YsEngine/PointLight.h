#pragma once
#include "Light.h"
class PointLight : public Light
{
public:
	PointLight(GLfloat ambient, GLfloat diffuse, glm::vec4 color, glm::vec3 position):
		Light(ambient, diffuse, color),
		position(position)
	{}

	glm::vec3 position;
};

