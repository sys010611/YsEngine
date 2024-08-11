#pragma once

#include "CameraBase.h"
#include "Entity.h"

class Window;

class FreeCamera : public CameraBase
{
public:
	FreeCamera(glm::vec3 startPosition, GLfloat moveSpeed, GLfloat turnSpeed, 
		GLfloat startPitch = 0.f, GLfloat startYaw= -90.f);

	void KeyControl(bool* keys, GLfloat deltaTime);
	virtual void MouseControl(GLfloat xChange, GLfloat yChange) override;
	void SetSpeed(GLfloat scrollY);

	virtual void SetCanMove(bool flag) override { canMove = flag; }
	virtual bool CanMove() override { return canMove; }

	~FreeCamera();

private:
	GLfloat moveSpeed;
	GLfloat turnSpeed;

	bool canMove;
};

