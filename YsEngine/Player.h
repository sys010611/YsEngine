#pragma once

#include "glm/glm.hpp"
class Model;

class Player
{
public:
	Player(Model* model);
	void HandleInput(bool* keys, float deltaTime);
	void Move(float deltaTime);
	float GetRotY();
	
	Model* GetModel() { return model; }

private:
	void Jump();

	Model* model;

	const float MOVE_SPEED;
	const float TURN_SPEED;
	const float GRAVITY;
	const float JUMP_POWER;

	float currMoveSpeed;
	float currTurnSpeed;
	float upwardSpeed;
	float groundHeight;

	bool isJumping;
};

