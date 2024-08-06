#pragma once

#include "GL/glew.h"
#include "CommonValues.h"

class Mesh;
class Shader;

class Terrain
{
public:	
	Terrain();

	void LoadTerrain(const char* fileName);
	void CreateTerrain();
	void DrawTerrain(glm::mat4 viewMat, glm::mat4 projMat);

	~Terrain();

private:
	Shader* terrainShader;
	GLuint loc_PVM;

	int width, height, nChannels;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	GLuint VAO, VBO, IBO;

	unsigned int NUM_STRIPS;
	unsigned int NUM_VERTS_PER_STRIP;
	unsigned int rez;
};

