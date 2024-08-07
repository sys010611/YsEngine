#pragma once

#include "GL/glew.h"
#include "CommonValues.h"

class Shader;

class Terrain
{
public:	
	Terrain() = default;

	void LoadTerrain(const char* fileName);
	void DrawTerrain(glm::mat4 viewMat, glm::mat4 projMat);

	~Terrain();

private:
	Shader* terrainShader;
	GLuint loc_PVM;

	int width, height, nChannels;

	std::vector<Vertex> vertices;

	GLuint VAO, VBO;
	unsigned int rez;
	unsigned int textureID;
};

