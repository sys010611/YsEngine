#pragma once
#include "Panel.h"

class FrameBuffer;
class Model;
class Window;
class Camera;

class ScenePanel : Panel
{
public:
	ScenePanel(FrameBuffer* fb, Model* md, Camera* cam, Window* win) :
		sceneBuffer(fb), currModel(md), camera(cam), mainWindow(win) 
	{}

	virtual void Render() override;

	~ScenePanel();

private:
	FrameBuffer* sceneBuffer;
	Model* currModel;
	Camera* camera;
	Window* mainWindow;
};

