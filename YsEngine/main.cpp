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

glm::mat4 GetPVM(glm :: mat4& modelMat)
{
	// PVM 구성
	glm::mat4 view = camera->GetViewMatrix();
	glm::mat4 projection = camera->GetProjectionMatrix(mainWindow);
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

void compose_imgui_frame(Model* currModel, FrameBuffer* sceneBuffer)
{
	// control window
	{
		// Model
		ImGui::Begin("Model");

		ImGui::InputFloat3("Translate", currModel->GetTranslate());
		ImGui::InputFloat3("Rotate", currModel->GetRotate());
		ImGui::InputFloat3("Scale", currModel->GetScale());


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

	// 카메라
	GLfloat initialPitch = 0.f;
	GLfloat initialYaw = -90.f; // 카메라가 -z축을 보고 있도록
	camera = new Camera(glm::vec3(0.f, 0.f, 20.f), glm::vec3(0.f, 1.f, 0.f), initialYaw, initialPitch, 10.f, 0.3f);
	
	// Directional Light
	dLight_ambient = 0.5f;
	dLight_diffuse = 0.5f;
	directionalLight = new DirectionalLight
		(dLight_ambient, dLight_diffuse,
		glm::vec4(1.f, 1.f, 1.f, 1.f), 
		glm::vec3(1.f, 1.5f, -1.f));

	// 모델
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

	FrameBuffer sceneBuffer(mainWindow->getBufferWidth(), mainWindow->getBufferHeight());

	mainWindow->SetSceneBuffer(&sceneBuffer);
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

		if (mainWindow->GetMouseButton()[GLFW_MOUSE_BUTTON_2])
			MoveCamera();

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

		shaderList[0]->UseDirectionalLight(directionalLight, dLight_ambient, dLight_diffuse);

		glm::vec4 camPos = glm::vec4(camera->GetPosition(), 1.f);
		glm::vec3 camPos_wc = glm::vec3(modelMat * camPos);
		glUniform3f(loc_eyePos, camPos_wc.x, camPos_wc.y, camPos_wc.z);

		shaderList[0]->UseMaterial(model_2B->GetMaterial());
		model_2B->RenderModel();

		glUseProgram(0);

		// --------------------------------------------------------------------------------
		sceneBuffer.Unbind();
		// --------------------------------------------------------------------------------

		// Render ImGui
		ImGui::Begin("Scene", NULL, ImGuiWindowFlags_NoMove);

		ImVec2 pos = ImGui::GetCursorScreenPos();
		const float window_width = ImGui::GetContentRegionAvail().x;
		const float window_height = ImGui::GetContentRegionAvail().y;

		// 프레임버퍼 텍스처를 ImGui 윈도우에 렌더링
		ImGui::GetWindowDrawList()->AddImage(
			(void*)(intptr_t)sceneBuffer.getFrameTexture(),
			ImVec2(pos.x, pos.y),
			ImVec2(pos.x + window_width, pos.y + window_height),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);
		{
			// Gizmos
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			//ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
			ImGuizmo::SetRect(pos.x, pos.y, window_width, window_height);

			glm::mat4 model = currModel->GetModelMat();
			glm::mat4 view = camera->GetViewMatrix();
			const glm::mat4& projection = camera->GetProjectionMatrix(mainWindow);

			ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
				ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::LOCAL, glm::value_ptr(model));

			if (ImGuizmo::IsUsing())
			{
				std::cout << "isusing" << std::endl;
				glm::vec3 translation, rotation, scale;
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), &translation[0], &rotation[0], &scale[0]);

				currModel->SetTranslate(translation);
				currModel->SetRotate(rotation);
				currModel->SetScale(scale);
			}
		}
		ImGui::End();

		compose_imgui_frame(currModel, &sceneBuffer);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		mainWindow->swapBuffers();
	}

    return 0;
}