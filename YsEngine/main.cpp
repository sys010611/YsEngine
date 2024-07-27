#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <assimp/Importer.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"

#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "Model.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Texture.h"
#include "Material.h"
#include "FrameBuffer.h"
#include "ScenePanel.h"
#include "InspectorPanel.h"
#include "Skybox.h"
#include "Animation.h"
#include "Animator.h"

#define WIDTH 1600
#define HEIGHT 900

Window* mainWindow;
Camera* camera;

GLfloat deltaTime = 0.f;
GLfloat lastTime = 0.f;

// Vertex Shader
static const char* vShaderPath = "Shaders/vertex.glsl";

// Fragment Shader
static const char* fShaderPath = "Shaders/fragment.glsl";

std::vector<Shader*> shaderList;

Model* mainModel;
Model* currModel;

Animator* animator;

Animation* idleAnim;
Animation* runAnim;
Animation* danceAnim;

DirectionalLight* directionalLight;

PointLight* pointLights[MAX_POINT_LIGHTS];
unsigned int pointLightCount = 0;

Skybox* skybox;

ScenePanel* scenePanel;
InspectorPanel* inspectorPanel;

// 쉐이더 변수 핸들
GLuint loc_modelMat = 0;
GLuint loc_PVM = 0;
GLuint loc_diffuseSampler = 0;
GLuint loc_normalSampler = 0;
GLuint loc_normalMat = 0;
GLuint loc_eyePos = 0;
GLuint loc_finalBonesMatrices = 0;

// 쉐이더 컴파일
void CreateShader()
{
	Shader* shader = new Shader;
	shader->CreateFromFiles(vShaderPath, fShaderPath);
	shaderList.push_back(shader);
}

void GetShaderHandles()
{
	// 핸들 얻어오기
	loc_modelMat = shaderList[0]->GetModelMatLoc();
	loc_PVM = shaderList[0]->GetPVMLoc();
	loc_normalMat = shaderList[0]->GetNormalMatLoc();
	loc_eyePos = shaderList[0]->GetEyePosLoc();
	loc_finalBonesMatrices = shaderList[0]->GetFinalBonesMatricesLoc();
}

glm::mat3 GetNormalMat(glm::mat4& modelMat)
{
	return glm::mat3(glm::transpose(glm::inverse(modelMat)));
}

void MoveCamera()
{
	camera->keyControl(mainWindow->GetKeys(), deltaTime);
	camera->mouseControl(mainWindow->getXChange(), mainWindow->getYChange());
}

int main()
{
    // GLFW 초기화
    if (!glfwInit())
    {
        printf("GLFW 초기화 실패\n");
        glfwTerminate();
        return 1;
    }

    mainWindow = new Window(WIDTH, HEIGHT);
    mainWindow->Initialize();

	CreateShader();

	// 카메라
	GLfloat initialPitch = 0.f;
	GLfloat initialYaw = -90.f; // 카메라가 -z축을 보고 있도록
	camera = new Camera(glm::vec3(0.f, 1.f, 5.f), glm::vec3(0.f, 1.f, 0.f), initialYaw, initialPitch, 10.f, 0.3f);
	
	// Directional Light
	directionalLight = new DirectionalLight
		(1.f, 0.5f,
		glm::vec4(1.f, 1.f, 1.f, 1.f), 
		glm::vec3(1.f, 1.5f, -1.f));

	// Point Light
	pointLights[0] = new PointLight
		(0.0f, 1.0f,
		glm::vec4(1.f, 0.f, 0.f, 1.f),
		glm::vec3(0.3f, 0.2f, 0.1f),
		1.0f, 0.22f, 0.20f);
	pointLightCount++;
	pointLights[1] = new PointLight
		(0.0f, 1.0f,
		glm::vec4(0.f, 1.f, 0.f, 1.f),
		glm::vec3(-4.0f, 2.0f, 0.0f),
		1.0, 0.045f, 0.0075f);
	pointLightCount++;

	// Skybox
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/px.png");
	skyboxFaces.push_back("Textures/Skybox/nx.png");
	skyboxFaces.push_back("Textures/Skybox/py.png");
	skyboxFaces.push_back("Textures/Skybox/ny.png");
	skyboxFaces.push_back("Textures/Skybox/pz.png");
	skyboxFaces.push_back("Textures/Skybox/nz.png");
	//skybox = new Skybox(skyboxFaces);

	// Model
	mainModel = new Model();
	std::string modelPath = "devola_-_nier_automata/devola.fbx";
	mainModel->LoadModel(modelPath);

	currModel = mainModel;


	// Animation
	idleAnim = new Animation("Models/devola_-_nier_automata/Idle.fbx", currModel);
	runAnim = new Animation("Models/devola_-_nier_automata/Slow_Run.fbx", currModel);
	danceAnim = new Animation("Models/devola_-_nier_automata/dance.fbx", currModel);

	// Animator
	animator = new Animator(danceAnim);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(mainWindow->GetGLFWwindow(), true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones. 
	ImGui_ImplOpenGL3_Init();

	// Frame Buffer 생성
	FrameBuffer sceneBuffer(mainWindow->getBufferWidth(), mainWindow->getBufferHeight());
	mainWindow->SetSceneBuffer(&sceneBuffer);

	// Panel 생성
	scenePanel = new ScenePanel(&sceneBuffer, currModel, camera, mainWindow);
	inspectorPanel = new InspectorPanel(currModel, directionalLight);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    ///////////////////////////////////////////////////////////////////////////
    /// main loop
    //////////////////////////////////////////////////////////////////////////
	while (!mainWindow->GetShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// ---------------------------------------
		glfwPollEvents();

		if (camera->CanMove())
			MoveCamera();

		animator->UpdateAnimation(deltaTime);
		// ----------------------------------------

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport();

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --------------------------------------------------------------------------------
		// Clear the framebuffer
		sceneBuffer.Bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// --------------------------------------------------------------------------------

		glm::mat4 viewMat = camera->GetViewMatrix();
		glm::mat4 projMat = camera->GetProjectionMatrix(scenePanel->GetWidth(), scenePanel->GetHeight());

		glm::mat4 identityMat(1.f);
		//skybox->DrawSkybox(viewMat, projMat);

		shaderList[0]->UseShader();
		GetShaderHandles();

		glUniform1i(loc_diffuseSampler, 0);
		glUniform1i(loc_normalSampler, 0);
		Model* currModel = mainModel;

		glm::mat4 modelMat = currModel->GetModelMat();
		glm::mat4 PVM = projMat * viewMat * modelMat;
		glm::mat4 normalMat = GetNormalMat(modelMat);
		glUniformMatrix4fv(loc_modelMat, 1, GL_FALSE, glm::value_ptr(modelMat));
		glUniformMatrix4fv(loc_PVM, 1, GL_FALSE, glm::value_ptr(PVM));
		glUniformMatrix3fv(loc_normalMat, 1, GL_FALSE, glm::value_ptr(normalMat));

		shaderList[0]->UseDirectionalLight(directionalLight);
		shaderList[0]->UsePointLights(pointLights, pointLightCount);

		glm::vec4 camPos = glm::vec4(camera->GetPosition(), 1.f);
		glm::vec3 camPos_wc = glm::vec3(modelMat * camPos);
		glUniform3f(loc_eyePos, camPos_wc.x, camPos_wc.y, camPos_wc.z);

		shaderList[0]->UseMaterial(mainModel->GetMaterial());

		const auto& transforms = animator->GetFinalBoneMatrices();
		shaderList[0]->UseFinalBoneMatrices(transforms);
		
		mainModel->RenderModel();

		glUseProgram(0);

		// --------------------------------------------------------------------------------
		sceneBuffer.Unbind();
		// --------------------------------------------------------------------------------

		scenePanel->Update();
		inspectorPanel->Update();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		mainWindow->swapBuffers();
	}

    return 0;
}