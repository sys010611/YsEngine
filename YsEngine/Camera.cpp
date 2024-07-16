#include "Camera.h"

#include "Window.h"
#include "glm/glm.hpp"

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat moveSpeed, GLfloat turnSpeed)
{
	position = startPosition;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;

	this->moveSpeed = moveSpeed;
	this->turnSpeed = turnSpeed;

	nearClippingPlane = 0.1f;
	farClippingPlane = 100.f;

	canMove = false;

	update();
}


void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = moveSpeed * deltaTime;

	if (keys[GLFW_KEY_W])
		position += front * velocity;

	if (keys[GLFW_KEY_S])
		position -= front * velocity;

	if (keys[GLFW_KEY_A])
		position -= right * velocity;

	if (keys[GLFW_KEY_D])
		position += right * velocity;

	if (keys[GLFW_KEY_Q])
		position -= up * velocity;

	if (keys[GLFW_KEY_E])
		position += up * velocity;
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	update();
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix(GLfloat width, GLfloat height)
{
	return glm::perspective(glm::radians(45.0f),
		width / height, nearClippingPlane, farClippingPlane);
}

glm::vec3 Camera::GetPosition()
{
	return position;
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}


Camera::~Camera()
{
}
