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
#include "HierarchyPanel.h"
#include "Entity.h"
#include "Terrain.h"
#include "Player.h"
#include "FreeCamera.h"
#include "PlayerCamera.h"

#define WIDTH 1600
#define HEIGHT 900

Window* mainWindow;

CameraBase* currCamera;
FreeCamera* freeCamera;
PlayerCamera* playerCamera;

GLfloat deltaTime = 0.f;
GLfloat lastTime = 0.f;

// Vertex Shader
static const char* vShaderPath = "Shaders/vertex.glsl";
// Fragment Shader
static const char* fShaderPath = "Shaders/fragment.glsl";

std::vector<Shader*> shaderList;
std::vector<Entity*> entityList;

Model* mainModel;
Model* currModel;

Player* player;

Animator* animator;

Animation* idleAnim;
Animation* runAnim;
Animation* danceAnim;

DirectionalLight* directionalLight;

PointLight* pointLights[MAX_POINT_LIGHTS];
unsigned int pointLightCount = 0;

Skybox* skybox;
Terrain* terrain;

ScenePanel* scenePanel;
HierarchyPanel* hierarchyPanel;

// 쉐이더 변수 핸들
GLuint loc_modelMat = 0;
GLuint loc_PVM = 0;
GLuint loc_diffuseSampler = 0;
GLuint loc_normalSampler = 0;
GLuint loc_normalMat = 0;
GLuint loc_eyePos = 0;
GLuint loc_finalBonesMatrices = 0;

void CreateShader();
void GetShaderHandles();
glm::mat3 GetNormalMat(glm::mat4& modelMat);
void MoveCamera();

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

	// Directional Light
	directionalLight = new DirectionalLight
		(0.5f, 1.f,
		glm::vec4(1.f, 1.f, 1.f, 1.f), 
		glm::vec3(0.f, 1.f, 1.f));
	entityList.push_back(directionalLight);

	// Point Light
	pointLights[0] = new PointLight
		(0.f, 0.5f,
		glm::vec4(1.f, 1.f, 1.f, 1.f),
		glm::vec3(0.f, 1.5f, 0.2f),
		1.0f, 0.22f, 0.20f);
	pointLightCount++;
	pointLights[1] = new PointLight
		(0.0f, 0.5f,
		glm::vec4(1.f, 1.f, 1.f, 1.f),
		glm::vec3(-2.0f, 2.0f, -1.f),
		1.0, 0.045f, 0.0075f);
	pointLightCount++;
	for(int i=0;i<pointLightCount;i++)
		entityList.push_back(pointLights[i]);

	// Skybox
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/px.png");
	skyboxFaces.push_back("Textures/Skybox/nx.png");
	skyboxFaces.push_back("Textures/Skybox/py.png");
	skyboxFaces.push_back("Textures/Skybox/ny.png");
	skyboxFaces.push_back("Textures/Skybox/pz.png");
	skyboxFaces.push_back("Textures/Skybox/nz.png");
	skybox = new Skybox(skyboxFaces);

	// Terrain
	terrain = new Terrain();
	terrain->LoadTerrain("Heightmaps/iceland_heightmap.png");
	entityList.push_back(terrain);

	// Model
	mainModel = new Model();
	std::string modelPath = "devola_-_nier_automata/untitled.fbx";
	mainModel->LoadModel(modelPath);
	entityList.push_back(mainModel);
	currModel = mainModel;

	// Player
	player = new Player(mainModel);

	// Camera
	freeCamera = new FreeCamera(glm::vec3(0.f, 12.f, 5.f), 10.f, 0.3f);
	playerCamera = new PlayerCamera(player);
	currCamera = playerCamera;

	// Animation
	//idleAnim = new Animation("Models/devola_-_nier_automata/Idle.fbx", currModel);

	// Animator
	//animator = new Animator(nullptr);

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
	scenePanel = new ScenePanel(&sceneBuffer, currCamera, mainWindow);
	hierarchyPanel = new HierarchyPanel(entityList, scenePanel);

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

		if (currCamera->CanMove())
			MoveCamera();
		currCamera->Update();

		player->HandleInput(mainWindow->GetKeys(), deltaTime);

		player->HandleInput(mainWindow->GetKeys(), deltaTime);
		player->Move(deltaTime, terrain);

		//animator->UpdateAnimation(deltaTime);
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

		glm::mat4 viewMat = currCamera->GetViewMatrix();
		glm::mat4 projMat = currCamera->GetProjectionMatrix(scenePanel->GetWidth(), scenePanel->GetHeight());

		glm::vec3 camPos = currCamera->GetPosition();

		skybox->DrawSkybox(viewMat, projMat);

		terrain->UseShader();
		terrain->GetShader()->UseDirectionalLight(directionalLight);
		terrain->GetShader()->UsePointLights(pointLights, pointLightCount);
		terrain->GetShader()->UseEyePos(camPos);
		terrain->DrawTerrain(viewMat, projMat);

		shaderList[0]->UseShader();
		{
			GetShaderHandles();

			Model* currModel = mainModel;

			glm::mat4 modelMat = currModel->GetModelMat();
			glm::mat4 PVM = projMat * viewMat * modelMat;
			glm::mat3 normalMat = GetNormalMat(modelMat);
			glUniformMatrix4fv(loc_modelMat, 1, GL_FALSE, glm::value_ptr(modelMat));
			glUniformMatrix4fv(loc_PVM, 1, GL_FALSE, glm::value_ptr(PVM));
			glUniformMatrix3fv(loc_normalMat, 1, GL_FALSE, glm::value_ptr(normalMat));

			shaderList[0]->UseEyePos(camPos);
			shaderList[0]->UseDirectionalLight(directionalLight);
			shaderList[0]->UsePointLights(pointLights, pointLightCount);

			shaderList[0]->UseMaterial(mainModel->GetMaterial());

			//const auto& transforms = animator->GetFinalBoneMatrices();
			//shaderList[0]->UseFinalBoneMatrices(transforms);

			glUniform1i(loc_diffuseSampler, 0);
			glUniform1i(loc_normalSampler, 1);

			mainModel->RenderModel();
		}
		glUseProgram(0);

		// --------------------------------------------------------------------------------
		sceneBuffer.Unbind();
		// --------------------------------------------------------------------------------

		scenePanel->Update();
		hierarchyPanel->UpdateEntityList(entityList);
		hierarchyPanel->Update();

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
	loc_diffuseSampler = shaderList[0]->GetColorSamplerLoc();
	loc_normalSampler = shaderList[0]->GetNormalSamplerLoc();
}

glm::mat3 GetNormalMat(glm::mat4& modelMat)
{
	return glm::mat3(glm::transpose(glm::inverse(modelMat)));
}

void MoveCamera()
{
	currCamera->KeyControl(mainWindow->GetKeys(), deltaTime);
	currCamera->MouseControl(mainWindow->getXChange(), mainWindow->getYChange());
	currCamera->ScrollControl(mainWindow->GetScrollYChange());
}