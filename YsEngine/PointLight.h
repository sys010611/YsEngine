#pragma once
#include "Light.h"
class PointLight : public Light
{
public:
	PointLight(GLfloat ambient, GLfloat diffuse, glm::vec4 color, glm::vec3 position,
		GLfloat constant, GLfloat linear, GLfloat exponent):
		Light(ambient, diffuse, color),
		position(position),
		constant(constant),
		linear(linear),
		exponent(exponent)
	{}

	glm::vec3 position;

	// attenuation
	GLfloat constant;
	GLfloat linear;
	GLfloat exponent;
};

