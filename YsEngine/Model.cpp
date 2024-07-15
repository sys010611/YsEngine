#include <iostream>

#include "Model.h"

#include "Texture.h"
#include "Mesh.h"
#include "Material.h"

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

Model::Model()
{
	translate = glm::vec3(0.f, 0.f, 0.f);
	rotate = glm::vec3(0.f, 0.f, 0.f);
	scale = glm::vec3(1.f, 1.f, 1.f);

	modelMat = GetModelMat();

	material = nullptr;
}

void Model::LoadModel(const std::string& fileName)
{
	// 모델 이름 추출
	int firstSlashIdx = fileName.find('/', 0);
	modelName = fileName.substr(0, firstSlashIdx);

	Assimp::Importer importer;

	// 2번째 인자는 타입이 unsigned int인데, 비트 or 연산을 사용한다. 자세한 내용은 후술
	const aiScene* scene = importer.ReadFile("Models/" + fileName,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		std::cout << fileName << " Model 로드 실패 : " << importer.GetErrorString() << std::endl;
		return;
	}

	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
}

void Model::RenderModel()
{
	// LoadMesh 함수에서 채워놓은 meshList를 순회하며 메시들을 렌더링한다.
	for (size_t i = 0; i < meshList.size(); i++)
	{
		// 메시에 해당하는 머티리얼을 통해 텍스쳐를 가져와 사용한다.
		unsigned int materialIndex = meshToTex[i];
		if (materialIndex < textureList.size() && textureList[materialIndex])
			textureList[materialIndex]->UseTexture();

		meshList[i]->RenderMesh();
	}
}

void Model::ClearModel()
{
	for (size_t i = 0; i < meshList.size(); i++)
	{
		if (meshList[i])
		{
			delete meshList[i];
			meshList[i] = nullptr;
		}
	}

	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (textureList[i])
		{
			delete textureList[i];
			textureList[i] = nullptr;
		}
	}
}

void Model::LoadNode(aiNode* node, const aiScene* scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		// node->mMeshes[i] : 메시 자체가 아니고, 메시의 ID를 의미한다.
		// 실제 메시는 scene에 저장되어있기 때문에 이렇게 참조하게 된다.
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	// 자식 노드들을 재귀호출을 통해 순회하며 메시를 쭉 로드한다.
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

// 실제로 VBO, IBO로 쏴줄 정보들을 구성한다.
void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		// position
		vertices.push_back(mesh->mVertices[i].x);
		vertices.push_back(mesh->mVertices[i].y);
		vertices.push_back(mesh->mVertices[i].z);

		// texture
		if (mesh->mTextureCoords[0])
		{
			vertices.push_back(mesh->mTextureCoords[0][i].x);
			vertices.push_back(mesh->mTextureCoords[0][i].y);
		}
		else // 존재하지 않을 경우 그냥 0을 넣어주기
		{
			vertices.push_back(0.f);
			vertices.push_back(0.f);
		}

		// normal (aiProcess_GenSmoothNormals를 적용했기 때문에 없을 수가 없다.)
		vertices.push_back(mesh->mNormals[i].x);
		vertices.push_back(mesh->mNormals[i].y);
		vertices.push_back(mesh->mNormals[i].z);
	}

	// indices 채워주기
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	Mesh* newMesh = new Mesh();
	newMesh->CreateMesh(vertices, indices); // GPU의 VBO, IBO로 버텍스 정보를 쏴준다.
	meshList.push_back(newMesh);

	// meshList에 mesh를 채워줌과 동시에, meshToTex에는 그 mesh의 materialIndex를 채워준다.
	// 이렇게 meshList와 meshToTex를 나란히 채워줌으로써 mesh와 맞는 material을 손쉽게 찾을 수 있다.
	meshToTex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene* scene)
{
	textureList.resize(scene->mNumMaterials);
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		textureList[i] = nullptr;

		// 텍스쳐가 존재하는 지 먼저 확인
		if (material->GetTextureCount(aiTextureType_BASE_COLOR))
		{
			aiString texturePath;
			// 텍스쳐 경로를 가져오는 데 성공했다면
			if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) == aiReturn_SUCCESS)
			{
				// 혹시나 텍스쳐 경로가 절대 경로로 되어있다면 그에 대한 처리
				int idx = std::string(texturePath.data).rfind("/");
				std::string textureName = std::string(texturePath.data).substr(idx + 1);

				std::string texPath = "Models/" + modelName + "/textures/" + textureName;

				textureList[i] = new Texture(texPath.c_str());

				// 텍스쳐를 디스크에서 메모리로 로드, GPU로 쏴준다.
				if (!textureList[i]->LoadTexture())
				{ // 실패 시
					std::cout << "텍스쳐 로드 실패 : " << texPath << std::endl;
					delete textureList[i];
					textureList[i] = nullptr;
				}
			}
		}

		// textureList에 텍스쳐를 담는데 실패했다면
		if (!textureList[i])
		{
			textureList[i] = new Texture("plain.png"); // 흰색 텍스쳐로 대체
		}
	}

	// specularMap이 존재하지 않을 경우
	material = new Material(4.f, 64.f);
}

glm::mat4 Model::GetModelMat()
{
	// model Matrix 구성
	glm::mat4 T = glm::translate(glm::mat4(1.f), glm::vec3(translate[0], translate[1], translate[2]));
	glm::mat4 R = glm::mat4_cast(glm::quat(glm::vec3(glm::radians(rotate[0]), glm::radians(rotate[1]), glm::radians(rotate[2]))));
	glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(scale[0], scale[1], scale[2]));
	glm::mat4 modelMat = T * R * S;

	return modelMat;
}

Model::~Model()
{
	ClearModel();
}