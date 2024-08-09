#pragma once

#include "GL/glew.h"
#include "CommonValues.h"

class Shader;
class Texture;

class Terrain
{
public:	
	Terrain() = default;

	void LoadTerrain(const char* fileName);
	void DrawTerrain(glm::mat4 viewMat, glm::mat4 projMat);
	Shader* GetShader() { return terrainShader; }
	void UseShader();

	~Terrain();

private:
	Shader* terrainShader;
	GLuint loc_PVM;

	int width, height, nChannels;

	std::vector<Vertex> vertices;

	GLuint VAO, VBO;
	unsigned int rez;

	Texture* heightMap;
	Texture* diffuseMap;
};

