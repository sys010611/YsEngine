#pragma once
#include "Panel.h"


class FrameBuffer;
class Model;
class Window;
class Camera;

class ScenePanel : Panel
{
public:
	ScenePanel(FrameBuffer* fb, Model* md, Camera* cam, Window* win);

	virtual void Update() override;
	void HandleInput();

	~ScenePanel();

private:
	FrameBuffer* sceneBuffer;
	Model* currModel;
	Camera* camera;
	Window* mainWindow;

	int currOperation;
};

