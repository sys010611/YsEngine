#pragma once
#include "Panel.h"

#include "GL/glew.h"

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

	GLfloat GetWidth() { return width; }
	GLfloat GetHeight() { return height; }

	~ScenePanel();

private:
	FrameBuffer* sceneBuffer;
	Model* currModel;
	Camera* camera;
	Window* mainWindow;

	int currOperation;

	GLfloat width, height;
};

