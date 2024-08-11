#pragma once
#include "CameraBase.h"

class Player;

class PlayerCamera : public CameraBase
{
public:
	PlayerCamera(Player* player);

	virtual void MouseControl(GLfloat xChange, GLfloat yChange) override;
	virtual void SetCanMove(bool flag) override { canMove = flag; }
	virtual bool CanMove() override { return canMove; }

private:
	Player* player;
	float distanceFromPlayer = 50.f;
	float angleAroundPlayer = 0.f;
	bool canMove;
};

