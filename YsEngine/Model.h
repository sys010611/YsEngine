#pragma once

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>

#include <glm/glm.hpp>

class Mesh;
class Texture;
class Material;

class Model
{
public:
	Model();

	void LoadModel(const std::string& fileName);
	void RenderModel();
	void ClearModel();

	GLfloat* GetTranslate() { return &translate[0]; }
	GLfloat* GetRotate() { return &rotate[0]; }
	GLfloat* GetScale() { return &scale[0]; }

	void SetTranslate(glm::vec3 translate) { this->translate = translate; }
	void SetRotate(glm::vec3 rotate) { this->rotate = rotate; }
	void SetScale(glm::vec3 scale) { this->scale = scale; }

	/// <summary>
	/// model matrix를 계산 후 리턴
	/// </summary>
	glm::mat4 GetModelMat();

	Material* GetMaterial() { return material; }

	~Model();

private:
	void LoadNode(aiNode* node, const aiScene* scene);
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);

	std::vector<Mesh*> meshList;
	std::vector<Texture*> textureList;
	std::vector<unsigned int> meshToTex;

	std::string modelName;

	// model transformation
	glm::vec3   translate;  // T
	glm::vec3   rotate;	 // R
	glm::vec3   scale;      // S

	glm::mat4	modelMat;

	Material* material;
};

