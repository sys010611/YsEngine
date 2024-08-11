#include "Player.h"

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Model.h"

Player::Player(Model* model) : MOVE_SPEED(10.f), TURN_SPEED(200.f), GRAVITY(0.2f), JUMP_POWER(0.05f)
{
	this->model = model;
	groundHeight = 10;
	upwardSpeed = 0;

	isJumping = true;
}

void Player::HandleInput(bool* keys, float deltaTime)
{
	if (keys[GLFW_KEY_W])
		currMoveSpeed = MOVE_SPEED;
	else if (keys[GLFW_KEY_S])
		currMoveSpeed = -MOVE_SPEED;
	else
		currMoveSpeed = 0;

	if (keys[GLFW_KEY_A])
		currTurnSpeed = TURN_SPEED;
	else if (keys[GLFW_KEY_D])
		currTurnSpeed = -TURN_SPEED;
	else
		currTurnSpeed = 0;

	if (keys[GLFW_KEY_SPACE])
		Jump();

	Move(deltaTime);
}

void Player::Move(float deltaTime)
{
	// 회전
	GLfloat* currRot = model->GetRotate();

	float rotation = currTurnSpeed * deltaTime;

	float newRotY = currRot[1] + rotation; // new rotY
	if(newRotY > 180)
		newRotY -= 360.f;
	if (newRotY < -180.f)
		newRotY += 360.f;

	glm::vec3 newRot(currRot[0], newRotY, currRot[2]);

	model->SetRotate(newRot);

	// 이동
	GLfloat* currPos = model->GetTranslate();
	float distance = currMoveSpeed * deltaTime;

	float dx = distance * sinf(DegreeToRadian(newRotY));
	float dz = distance * cosf(DegreeToRadian(newRotY));

	upwardSpeed -= GRAVITY * deltaTime;

	glm::vec3 newPos(currPos[0]+dx, currPos[1] + upwardSpeed, currPos[2]+dz);

	if (newPos[1] <= groundHeight) // 땅에 닿았다면
	{
		upwardSpeed = 0;
		newPos[1] = groundHeight;
		isJumping = false;
	}

	model->SetTranslate(newPos);
}

void Player::Jump()
{
	if(isJumping)
		return;
	
	upwardSpeed = JUMP_POWER;
	isJumping = true;
}

float Player::DegreeToRadian(float degree)
{
	float pi = 3.14159f;
	return (degree * (pi / 180.f));
}