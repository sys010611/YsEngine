#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

class CameraBase
{
public: 
	CameraBase(glm::vec3 startPosition, GLfloat startPitch = 0.f, GLfloat startYaw = -90.f);

	virtual void MouseControl(GLfloat xChange, GLfloat yChange) = 0;
	virtual bool CanMove() = 0;
	virtual void SetCanMove(bool flag) = 0;

	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix(GLfloat width, GLfloat height);
	glm::vec3 GetPosition();


protected:
	void Update();

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat nearClippingPlane;
	GLfloat farClippingPlane;
};

