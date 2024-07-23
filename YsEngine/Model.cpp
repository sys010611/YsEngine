#include <iostream>
#include <vector>

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
	std::vector<std::pair<Mesh*, unsigned int>> solidMeshList;
	std::vector<std::pair<Mesh*, unsigned int>> hairMeshList;

	// LoadMesh 함수에서 채워놓은 meshList를 순회하며 메시들을 렌더링한다.
	for (size_t i = 0; i < meshList.size(); i++)
	{
		unsigned int materialIndex = meshToTex[i];
		
		// 메시 분류
		if (meshList[i]->GetName().find("Hair") != std::string::npos)
			hairMeshList.push_back({meshList[i], materialIndex});
		else
			solidMeshList.push_back({meshList[i], materialIndex});
	}

	for (auto& item : solidMeshList)
	{
		int materialIndex = item.second;
		Mesh* mesh = item.first;

		if (materialIndex < textureList.size() && textureList[materialIndex])
			textureList[materialIndex]->UseTexture();
		if(normalMapList[materialIndex])
			normalMapList[materialIndex]->UseNormal();

		mesh->RenderMesh();
	}

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (auto& item : hairMeshList)
	{
		int materialIndex = item.second;
		Mesh* mesh = item.first;

		if (materialIndex < textureList.size() && textureList[materialIndex])
			textureList[materialIndex]->UseTexture();
		if (normalMapList[materialIndex])
			normalMapList[materialIndex]->UseNormal();

		mesh->RenderMesh();
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	for (auto& item : solidMeshList)
	{
		int materialIndex = item.second;
		Mesh* mesh = item.first;

		if (materialIndex < textureList.size() && textureList[materialIndex])
			textureList[materialIndex]->UseTexture();
		if (normalMapList[materialIndex])
			normalMapList[materialIndex]->UseNormal();

		mesh->RenderMesh();
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

	delete(material);
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

// VBO, IBO에 담을 정보들을 구성한 뒤 쏴준다.
void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		InitVertexBoneData(vertex);

		// position
		vertex.Position.x = (mesh->mVertices[i].x);
		vertex.Position.y = (mesh->mVertices[i].y);
		vertex.Position.z = (mesh->mVertices[i].z);

		// texture
		if (mesh->mTextureCoords[0])
		{
			vertex.TexCoords.s = (mesh->mTextureCoords[0][i].x);
			vertex.TexCoords.t = (mesh->mTextureCoords[0][i].y);
		}
		else // 존재하지 않을 경우 그냥 0을 넣어주기
		{
			vertex.TexCoords.s = (0.f);
			vertex.TexCoords.t = (0.f);
		}

		// normal (aiProcess_GenSmoothNormals를 적용했기 때문에 없을 수가 없다.)
		vertex.Normal.x = (mesh->mNormals[i].x);
		vertex.Normal.y = (mesh->mNormals[i].y);
		vertex.Normal.z = (mesh->mNormals[i].z);

		vertices.push_back(vertex);
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
	newMesh->CreateMesh(vertices, indices, mesh->mName.C_Str()); // GPU의 VBO, IBO로 버텍스 정보를 쏴준다.
	meshList.push_back(newMesh);

	// meshList에 mesh를 채워줌과 동시에, meshToTex에는 그 mesh의 materialIndex를 채워준다.
	// 이렇게 meshList와 meshToTex를 나란히 채워줌으로써 mesh와 맞는 material을 손쉽게 찾을 수 있다.
	meshToTex.push_back(mesh->mMaterialIndex);

	ExtractBoneWeightForVertices(vertices, mesh, scene);
}

void Model::LoadMaterials(const aiScene* scene)
{
	textureList.resize(scene->mNumMaterials);
	normalMapList.resize(scene->mNumMaterials);
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		textureList[i] = nullptr;

		LoadDiffuseTexture(material, i);
		LoadNormalMap(material, i);
	}

	material = new Material(0.3f, 64.f);
}

void Model::LoadDiffuseTexture(aiMaterial* material, const size_t& i)
{
	// Diffuse 텍스쳐가 존재하는 지 먼저 확인
	if (material->GetTextureCount(aiTextureType_DIFFUSE))
	{
		aiString texturePath;
		// 텍스쳐 경로를 가져오는 데 성공했다면
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			// 혹시나 텍스쳐 경로가 절대 경로로 되어있다면 그에 대한 처리
			int idx = std::string(texturePath.data).rfind("/");
			std::string textureName = std::string(texturePath.data).substr(idx + 1);
			idx = std::string(textureName).rfind("\\");
			textureName = std::string(textureName).substr(idx + 1);

			std::string texPath = "Models/" + modelName + "/textures/" + textureName;

			textureList[i] = new Texture(texPath.c_str());
			std::cout << "텍스쳐 로딩 : " << texPath << std::endl;

			// 텍스쳐를 디스크에서 메모리로 로드, GPU로 쏴준다.
			if (!textureList[i]->LoadTexture())
			{ // 실패 시
				std::cout << "텍스쳐 로드 실패 : " << texPath << std::endl;
				delete textureList[i];
				textureList[i] = nullptr;
			}
		}
	}
}

void Model::LoadNormalMap(aiMaterial* material, const size_t& i)
{
	// normal map이
	if (material->GetTextureCount(aiTextureType_NORMALS))
	{
		aiString texturePath;
		// 텍스쳐 경로를 가져오는 데 성공했다면
		if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == aiReturn_SUCCESS)
		{
			// 혹시나 텍스쳐 경로가 절대 경로로 되어있다면 그에 대한 처리
			int idx = std::string(texturePath.data).rfind("/");
			std::string textureName = std::string(texturePath.data).substr(idx + 1);
			idx = std::string(textureName).rfind("\\");
			textureName = std::string(textureName).substr(idx + 1);

			std::string texPath = "Models/" + modelName + "/textures/" + textureName;

			normalMapList[i] = new Texture(texPath.c_str());
			std::cout << "텍스쳐 로딩 : " << texPath << std::endl;

			// 텍스쳐를 디스크에서 메모리로 로드, GPU로 쏴준다.
			if (!normalMapList[i]->LoadTexture())
			{ // 실패 시
				std::cout << "텍스쳐 로드 실패 : " << texPath << std::endl;
				delete normalMapList[i];
				normalMapList[i] = nullptr;
			}
		}
	}
}

void Model::InitVertexBoneData(Vertex& vertex)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		vertex.m_BoneIDs[i] = -1;
		vertex.m_Weights[i] = 0.f;
	}
}

void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		if (vertex.m_BoneIDs[i] < 0)
		{
			// 하나만 채우고 도망가기
			vertex.m_Weights[i] = weight;
			vertex.m_BoneIDs[i] = boneID;
			break;
		}
	}
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
{
	for (int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			BoneInfo boneInfo;
			boneInfo.id = boneCounter;
			aiMatrix4x4& offsetMat = mesh->mBones[boneIndex]->mOffsetMatrix;
			std::memcpy(glm::value_ptr(boneInfo.offset), &offsetMat, sizeof(offsetMat));

			boneInfoMap[boneName] = boneInfo;

			boneID = boneCounter;
			boneCounter++;
		}
		else
		{
			boneID = boneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; weightIndex++)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
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