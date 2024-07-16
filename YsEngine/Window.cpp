#include "Window.h"

#include <iostream>
#include "FrameBuffer.h"

Window::Window(GLint windowWidth, GLint windowHeight)
{
	width = windowWidth;
	height = windowHeight;

	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = 0;
	}
	for (size_t i = 0; i < 8; i++)
	{
		mouseButton[i] = 0;
	}

	xChange = 0.0f;
	yChange = 0.0f;

	mouseRightButtonClicked = false;
}

int Window::Initialize()
{
	if (!glfwInit())
	{
		printf("Error Initialising GLFW");
		glfwTerminate();
		return 1;
	}

	// GLFW 윈도우 속성 셋업
	// OpenGL 버전
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // 버전 3.3
	// Core profile = 이전 버전 호환성 없음
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// 앞으로의 호환성을 허용
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	// Create the window
	mainWindow = glfwCreateWindow(width, height, "Test Window", NULL, NULL);
	if (!mainWindow)
	{
		printf("Error creating GLFW window!");
		glfwTerminate();
		return 1;
	}

	// Get buffer size information
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set the current context
	glfwMakeContextCurrent(mainWindow);

	// Handle Key + Mouse Input
	createCallbacks();
	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Allow modern extension access
	glewExperimental = GL_TRUE;

	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		printf("Error: %s", glewGetErrorString(error));
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);

	// Create Viewport
	glViewport(0, 0, bufferWidth, bufferHeight);

	glfwSetWindowUserPointer(mainWindow, this);
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, handleKeys);
	glfwSetMouseButtonCallback(mainWindow, handleMouseButton);
	glfwSetCursorPosCallback(mainWindow, handleMousePos);
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	//{
	//	glfwSetWindowShouldClose(window, GL_TRUE);
	//}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			myWindow->keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			myWindow->keys[key] = false;
		}
	}
}

void Window::handleMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	Window* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (action == GLFW_PRESS)
		myWindow->mouseButton[button] = true;
	else
		myWindow->mouseButton[button] = false;

}

void Window::handleMousePos(GLFWwindow* window, double xPos, double yPos)
{
	Window* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (myWindow->mouseButton[GLFW_MOUSE_BUTTON_2])
	{
		if (!myWindow->mouseRightButtonClicked)
		{
			myWindow->mouseRightButtonClicked = true;
		}
		else
		{
			myWindow->xChange = xPos - myWindow->lastX;
			myWindow->yChange = myWindow->lastY - yPos;
		}

		myWindow->lastX = xPos;
		myWindow->lastY = yPos;
	}
	else
	{
		myWindow->mouseRightButtonClicked = false;
	}
}

void Window::SetSceneBuffer(FrameBuffer* sceneBuffer)
{
	this->sceneBuffer = sceneBuffer;
}

Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
