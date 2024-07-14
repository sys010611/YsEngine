#pragma once

#include "stdio.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

class Window
{
public:
	Window();

	Window(GLint windowWidth, GLint windowHeight);

	int Initialise();

	GLint getBufferWidth() { return bufferWidth; }
	GLint getBufferHeight() { return bufferHeight; }

	bool GetShouldClose() { return glfwWindowShouldClose(mainWindow); }

	bool* GetKeys() { return keys; }
	GLfloat getXChange();
	GLfloat getYChange();

	GLFWwindow* GetGLFWwindow() { return mainWindow; }

	void swapBuffers() { glfwSwapBuffers(mainWindow); }

	~Window();

private:
	GLFWwindow* mainWindow;

	GLint width, height;
	GLint bufferWidth, bufferHeight;

	bool keys[1024];

	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseButton[8];
	bool mouseClicked;

	void createCallbacks();
	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void handleMouseButton(GLFWwindow* window, int button, int action, int mods);
	static void handleMousePos(GLFWwindow* window, double xPos, double yPos);
};