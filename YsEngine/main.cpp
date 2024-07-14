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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define WIDTH 1280
#define HEIGHT 720

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

DirectionalLight* directionalLight;

// DirectionalLight Intensity
GLfloat dLight_ambient;
GLfloat dLight_diffuse;

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

void compose_imgui_frame(Model* currModel)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// control window
	{
		// Model
		ImGui::Begin("Model");

		ImGui::SliderFloat3("Translate", currModel->GetTranslate(), -100.f, 100.f);
		ImGui::InputFloat3("Rotate", currModel->GetRotate());
		ImGui::SliderFloat3("Scale", currModel->GetScale(), -10.f, 10.f);

		ImGui::End();


		// Directional Light
		ImGui::Begin("DirectionalLight");

		ImGui::SliderFloat("Ambient", &dLight_ambient, 0.f, 5.f);
		ImGui::SliderFloat("Diffuse", &dLight_diffuse, 0.f, 5.f);

		ImGui::End();

		
		// Material
		Material* currMaterial = currModel->GetMaterial();
		ImGui::Begin("Material");

		ImGui::SliderFloat("Specular", &currMaterial->specular, 0.f, 5.f);
		ImGui::SliderFloat("Shininess", &currMaterial->shininess, 0.f, 512.f);

		ImGui::End();
	}
}

glm::mat4 GetModelMat(Model* currModel)
{
	GLfloat* translate = currModel->GetTranslate();
	GLfloat* rotate = currModel->GetRotate();
	GLfloat* scale = currModel->GetScale();

	// model Matrix 구성
	glm::mat4 T = glm::translate(glm::mat4(1.f), glm::vec3(translate[0], translate[1], translate[2]));
	glm::mat4 R = glm::mat4_cast(glm::quat(glm::vec3(rotate[0], rotate[1], rotate[2])));
	glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(scale[0], scale[1], scale[2]));
	glm::mat4 modelMat = T * R * S;

	return modelMat;
}

glm::mat4 GetPVM(glm :: mat4& modelMat)
{
	// PVM 구성
	glm::mat4 view = camera->calculateViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f),
		(GLfloat)mainWindow->getBufferWidth() / mainWindow->getBufferHeight(), 0.1f, 100.0f);
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
    mainWindow->Initialise();

	CreateShader();

	GLfloat initialPitch = 0.f;
	GLfloat initialYaw = -90.f; // 카메라가 -z축을 보고 있도록
	camera = new Camera(glm::vec3(0.f, 0.f, 20.f), glm::vec3(0.f, 1.f, 0.f), initialYaw, initialPitch, 10.f, 0.3f);
	
	dLight_ambient = 0.5f;
	dLight_diffuse = 0.5f;
	directionalLight = new DirectionalLight
		(dLight_ambient, dLight_diffuse,
		glm::vec4(1.f, 1.f, 1.f, 1.f), 
		glm::vec3(1.f, 1.5f, -1.f));

	model_2B = new Model();
	std::string modelPath = "2b_nier_automata/scene.gltf";
	model_2B->LoadModel(modelPath);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(mainWindow->GetGLFWwindow(), true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones. 
	ImGui_ImplOpenGL3_Init();

    /////////////////////////
    /// while loop
    ////////////////////////
    while (!mainWindow->GetShouldClose())
    {
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();

		const auto& io = ImGui::GetIO();
		if (!io.WantCaptureMouse && !io.WantCaptureKeyboard)
			MoveCamera();

		// Clear the window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// -------------------------------------------------------------------

		shaderList[0]->UseShader();

		// 쉐이더 내부 변수 위치들 가지고오기
		GetShaderHandles();
		glUniform1i(loc_sampler, 0); // sampler를 0번 텍스쳐 유닛과 연결

		Model* currModel = model_2B;

		// imgui 창 그리기
		compose_imgui_frame(currModel);

		glm::mat4 modelMat = GetModelMat(currModel);
		glm::mat4 PVM = GetPVM(modelMat);
		glm::mat4 normalMat = GetNormalMat(modelMat);
		glUniformMatrix4fv(loc_modelMat, 1, GL_FALSE, glm::value_ptr(modelMat));
		glUniformMatrix4fv(loc_PVM, 1, GL_FALSE, glm::value_ptr(PVM));
		glUniformMatrix3fv(loc_normalMat, 1, GL_FALSE, glm::value_ptr(normalMat));

		shaderList[0]->UseDirectionalLight(directionalLight, dLight_ambient, dLight_diffuse);

		glm::vec4 camPos = glm::vec4(camera->GetPosition(), 1.f);
		glm::vec3 camPos_wc = modelMat * camPos;
		glUniform3f(loc_eyePos, camPos_wc.x, camPos_wc.y, camPos_wc.z);

		shaderList[0]->UseMaterial(model_2B->GetMaterial());

		model_2B->RenderModel();

		//-------------------------------------------------------------------

		glUseProgram(0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		mainWindow->swapBuffers();
    }
    return 0;
}