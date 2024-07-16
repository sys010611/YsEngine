#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <assimp/Importer.hpp>

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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuizmo.h"

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

Model* model_2B;
Model* currModel;

DirectionalLight* directionalLight;

ScenePanel* scenePanel;
InspectorPanel* inspectorPanel;

// 쉐이더 변수 핸들
GLuint loc_modelMat = 0;
GLuint loc_PVM = 0;
GLuint loc_sampler = 0;
GLuint loc_normalMat = 0;
GLuint loc_eyePos = 0;

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
}

glm::mat4 GetPVM(glm :: mat4& modelMat)
{
	// PVM 구성
	glm::mat4 view = camera->GetViewMatrix();
	glm::mat4 projection = camera->GetProjectionMatrix(scenePanel->GetWidth(), scenePanel->GetHeight());
	glm::mat4 PVM = projection * view * modelMat;

	return PVM;
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
	camera = new Camera(glm::vec3(0.f, 0.f, 20.f), glm::vec3(0.f, 1.f, 0.f), initialYaw, initialPitch, 10.f, 0.3f);
	
	// Directional Light
	directionalLight = new DirectionalLight
		(0.5f, 0.5f,
		glm::vec4(1.f, 1.f, 1.f, 1.f), 
		glm::vec3(1.f, 1.5f, -1.f));

	// 모델
	model_2B = new Model();
	std::string modelPath = "2b_nier_automata/scene.gltf";
	model_2B->LoadModel(modelPath);

	currModel = model_2B;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(mainWindow->GetGLFWwindow(), true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones. 
	ImGui_ImplOpenGL3_Init();

	// Frame Buffer 생성
	FrameBuffer sceneBuffer(mainWindow->getBufferWidth(), mainWindow->getBufferHeight());
	mainWindow->SetSceneBuffer(&sceneBuffer);

	// Panel 생성
	scenePanel = new ScenePanel(&sceneBuffer, currModel, camera, mainWindow);
	inspectorPanel = new InspectorPanel(currModel, directionalLight);

    ///////////////////////////////////////////////////////////////////////////
    /// main loop
    //////////////////////////////////////////////////////////////////////////
	while (!mainWindow->GetShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();

		if (camera->CanMove())
		{
			MoveCamera();
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// --------------------------------------------------------------------------------
		sceneBuffer.Bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// --------------------------------------------------------------------------------

		shaderList[0]->UseShader();
		GetShaderHandles();

		glUniform1i(loc_sampler, 0);
		Model* currModel = model_2B;

		glm::mat4 modelMat = currModel->GetModelMat();
		glm::mat4 PVM = GetPVM(modelMat);
		glm::mat4 normalMat = GetNormalMat(modelMat);
		glUniformMatrix4fv(loc_modelMat, 1, GL_FALSE, glm::value_ptr(modelMat));
		glUniformMatrix4fv(loc_PVM, 1, GL_FALSE, glm::value_ptr(PVM));
		glUniformMatrix3fv(loc_normalMat, 1, GL_FALSE, glm::value_ptr(normalMat));

		shaderList[0]->UseDirectionalLight(directionalLight);

		glm::vec4 camPos = glm::vec4(camera->GetPosition(), 1.f);
		glm::vec3 camPos_wc = glm::vec3(modelMat * camPos);
		glUniform3f(loc_eyePos, camPos_wc.x, camPos_wc.y, camPos_wc.z);

		shaderList[0]->UseMaterial(model_2B->GetMaterial());
		model_2B->RenderModel();

		glUseProgram(0);

		// --------------------------------------------------------------------------------
		sceneBuffer.Unbind();
		// --------------------------------------------------------------------------------

		scenePanel->Update();
		inspectorPanel->Update();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		mainWindow->swapBuffers();
	}

    return 0;
}