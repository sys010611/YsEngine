#pragma once
#include "Light.h"
class DirectionalLight : public Light
{
public:
	DirectionalLight(GLfloat ambient, GLfloat diffuse, glm::vec4 color, glm::vec3 direction) :
		Light(ambient, diffuse, color),
		direction(direction)
	{}

	glm::vec3 direction;
};

