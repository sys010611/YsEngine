#include "PlayerCamera.h"

#include "Player.h"
#include "Model.h"

PlayerCamera::PlayerCamera(Player* player) : CameraBase(glm::vec3(0.f,0.f,0.f), 20.f, 0.f)
{
	this->player = player;
	canMove = false;
}

void PlayerCamera::MouseControl(GLfloat xChange, GLfloat yChange)
{
}
