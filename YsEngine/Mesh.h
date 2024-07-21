#pragma once

#include "CommonValues.h"

#include <GL\glew.h>
#include <glm/glm.hpp>

class Mesh
{
public:
	Mesh();

	void CreateMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
	void RenderMesh();
	void ClearMesh();

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};

