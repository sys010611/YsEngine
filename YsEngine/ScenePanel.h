#pragma once
#include "Panel.h"

#include "GL/glew.h"

class FrameBuffer;
class Model;
class Window;
class Camera;
class Entity;
struct ImVec2;

class ScenePanel : Panel
{
public:
	ScenePanel(FrameBuffer* fb, Camera* cam, Window* win);

	virtual void Update() override;
	void HandleInput();
	void SetEntity(Entity* e);

	GLfloat GetWidth() { return width; }
	GLfloat GetHeight() { return height; }

	~ScenePanel();

private:
	void DrawGizmo(ImVec2 pos);

	FrameBuffer* sceneBuffer;
	Entity* selectedEntity;
	Camera* camera;
	Window* mainWindow;

	int currOperation;

	GLfloat width, height;
};

