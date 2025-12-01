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

#include "imgui.h"
#include "AssimpGLMHelpers.h"
#include "ImGuizmo.h"

Model::Model()
{
	translate = glm::vec3(0.f, 0.f, 0.f);
	rotate = glm::vec3(-90.f, 0.f, 0.f);
	scale = glm::vec3(1.f, 1.f, 1.f);

	modelMat = GetModelMat();

	material = nullptr;
}

std::string Model::GetName()
{
	return "Model";
}

void Model::ShowProperties()
{
	// Transform
	ImGui::Text("Transform");

	ImGui::InputFloat3("Translate", GetTranslate());
	ImGui::InputFloat3("Rotate", GetRotate());
	ImGui::InputFloat3("Scale", GetScale());

	// Material
	Material* currMaterial = GetMaterial();
	ImGui::Text("Material");

	ImGui::SliderFloat("Specular", &currMaterial->specular, 0.f, 5.f);
	ImGui::SliderFloat("Shininess", &currMaterial->shininess, 0.f, 512.f);
}

void Model::LoadModel(const std::string& fileName)
{
	// モデル名を抽出
	int firstSlashIdx = fileName.find('/', 0);
	modelName = fileName.substr(0, firstSlashIdx);

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile("Models/" + fileName,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices |
		aiProcess_CalcTangentSpace);

	if (!scene)
	{
		std::cout << fileName << " Failed to load model : " << importer.GetErrorString() << std::endl;
		return;
	}

	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
}

void Model::RenderModel()
{
	std::vector<std::pair<Mesh*, unsigned int>> solidMeshList;
	std::vector<std::pair<Mesh*, unsigned int>> transparentMeshList;

	// LoadMesh 関数で埋めておいた meshList を走査してメッシュを分類する。
	for (size_t i = 0; i < meshList.size(); i++)
	{
		unsigned int materialIndex = meshToTex[i];

		// メッシュ分類
		if (meshList[i]->GetName().find("Hair") != std::string::npos ||
			meshList[i]->GetName().find("facial_serious10") != std::string::npos ||
			meshList[i]->GetName().find("facial_normal9") != std::string::npos)
			transparentMeshList.push_back({ meshList[i], materialIndex });
		else
			solidMeshList.push_back({ meshList[i], materialIndex });
	}

	// 不透明メッシュのレンダリング
	for (auto& item : solidMeshList)
	{
		int materialIndex = item.second;
		Mesh* mesh = item.first;

		if (materialIndex < diffuseMaps.size() && diffuseMaps[materialIndex])
			diffuseMaps[materialIndex]->UseTexture(GL_TEXTURE0);
		if (materialIndex < normalMaps.size() && normalMaps[materialIndex])
			normalMaps[materialIndex]->UseTexture(GL_TEXTURE1);

		mesh->RenderMesh();
	}

	// 透明メッシュのレンダリング
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (auto& item : transparentMeshList)
	{
		int materialIndex = item.second;
		Mesh* mesh = item.first;

		if (materialIndex < diffuseMaps.size() && diffuseMaps[materialIndex])
			diffuseMaps[materialIndex]->UseTexture(GL_TEXTURE0);
		if (materialIndex < normalMaps.size() && normalMaps[materialIndex])
			normalMaps[materialIndex]->UseTexture(GL_TEXTURE1);

		mesh->RenderMesh();
	}
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
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

	for (size_t i = 0; i < diffuseMaps.size(); i++)
	{
		if (diffuseMaps[i])
		{
			delete diffuseMaps[i];
			diffuseMaps[i] = nullptr;
		}
	}

	delete(material);
}

void Model::LoadNode(aiNode* node, const aiScene* scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		// node->mMeshes[i] はメッシュ自体ではなく、メッシュの ID を意味する。
		// 実際のメッシュは scene に保存されているため、このように参照する。
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	// 子ノードを再帰呼び出しで巡回しながらメッシュを順にロードする。
	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

// VBO と IBO に入れる情報を構成してから送る。
void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	// vertices を埋める
	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		InitVertexBoneData(vertex);

		// position
		vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);

		// texture
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = (mesh->mTextureCoords[0][i].x);
			vec.y = (mesh->mTextureCoords[0][i].y);
			vertex.TexCoords = vec;
		}
		else // 存在しない場合はそのまま 0 を入れる
		{
			vertex.TexCoords = glm::vec2(0.f, 0.f);
		}

		// normal 
		vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

		// tangent, bitangent
		vertex.Tangent = AssimpGLMHelpers::GetGLMVec(mesh->mTangents[i]);
		vertex.Bitangent = AssimpGLMHelpers::GetGLMVec(mesh->mBitangents[i]);

		vertices.push_back(vertex);
	}

	// indices を埋める
	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	ExtractBoneWeightForVertices(vertices, mesh, scene);

	Mesh* newMesh = new Mesh();
	newMesh->CreateMesh(vertices, indices, mesh->mName.C_Str()); // GPU の VBO と IBO に頂点情報を送る。
	meshList.push_back(newMesh);

	// meshList に mesh を追加すると同時に、meshToTex にはその mesh の materialIndex を入れる。
	// このように meshList と meshToTex を並行して埋めることで、mesh に対応する material を簡単に見つけられる。
	meshToTex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene* scene)
{
	diffuseMaps.resize(scene->mNumMaterials);
	normalMaps.resize(scene->mNumMaterials);
	for (size_t i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];

		diffuseMaps[i] = nullptr;
		normalMaps[i] = nullptr;

		LoadDiffuseMaps(material, i);
		LoadNormalMaps(material, i);
	}

	material = new Material(0.3f, 64.f);
}

void Model::LoadDiffuseMaps(aiMaterial* material, const size_t& i)
{
	// Diffuse テクスチャが存在するか先に確認する
	if (material->GetTextureCount(aiTextureType_DIFFUSE))
	{
		aiString texturePath;
		// テクスチャパスの取得に成功した場合
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
		{
			// テクスチャパスが絶対パスになっている場合の処理
			int idx = std::string(texturePath.data).rfind("/");
			std::string textureName = std::string(texturePath.data).substr(idx + 1);
			idx = std::string(textureName).rfind("\\");
			textureName = std::string(textureName).substr(idx + 1);

			std::string texPath = "Models/" + modelName + "/textures/" + textureName;

			diffuseMaps[i] = new Texture(texPath.c_str());
			std::cout << "Loading Diffuse : " << texPath << std::endl;

			// テクスチャをディスクからメモリにロードし、GPU に送る。
			if (!diffuseMaps[i]->LoadTexture(4))
			{ // 失敗時
				std::cout << "Failed to load texture : " << texPath << std::endl;
				delete diffuseMaps[i];
				diffuseMaps[i] = nullptr;
			}
		}
	}
}

void Model::LoadNormalMaps(aiMaterial* material, const size_t& i)
{
	// normal map が存在するかどうか
	if (material->GetTextureCount(aiTextureType_NORMALS) || material->GetTextureCount(aiTextureType_HEIGHT))
	{
		aiString texturePath;
		// テクスチャパスの取得に成功した場合
		if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == aiReturn_SUCCESS ||
			material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == aiReturn_SUCCESS)
		{
			// テクスチャパスが絶対パスになっている場合の処理
			int idx = std::string(texturePath.data).rfind("/");
			std::string textureName = std::string(texturePath.data).substr(idx + 1);
			idx = std::string(textureName).rfind("\\");
			textureName = std::string(textureName).substr(idx + 1);

			std::string texPath = "Models/" + modelName + "/textures/" + textureName;

			normalMaps[i] = new Texture(texPath.c_str());
			std::cout << "Loading Normal : " << texPath << std::endl;

			// テクスチャをディスクからメモリにロードし、GPU に送る。
			if (!normalMaps[i]->LoadTexture(3))
			{ // 失敗時
				std::cout << "Failed to load texture : " << texPath << std::endl;
				delete normalMaps[i];
				normalMaps[i] = nullptr;
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
			// 一つだけ埋めて抜ける
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
			auto offsetMat = mesh->mBones[boneIndex]->mOffsetMatrix;
			boneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
				mesh->mBones[boneIndex]->mOffsetMatrix
			);

			boneInfoMap[boneName] = boneInfo;
			boneID = boneCounter;
			boneCounter++;
		}
		else
		{
			boneID = boneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
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
	// model 行列を構成する
	glm::mat4 T = glm::translate(glm::mat4(1.f), glm::vec3(translate[0], translate[1], translate[2]));
	glm::mat4 R = glm::mat4_cast(glm::quat(glm::vec3(glm::radians(rotate[0]), glm::radians(rotate[1]), glm::radians(rotate[2]))));
	glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(scale[0], scale[1], scale[2]));
	return modelMat = T * R * S;
}

void Model::UpdateTransform(glm::mat4 newModelMat)
{
	glm::vec3 translation, rotation, scale;
	ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(newModelMat), &translation[0], &rotation[0], &scale[0]);

	SetTranslate(translation);
	SetRotate(rotation);
	SetScale(scale);
}

Model::~Model()
{
	ClearModel();
}
