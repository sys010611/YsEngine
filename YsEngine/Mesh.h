#pragma once

#include "CommonValues.h"

#include <GL\glew.h>
#include <glm/glm.hpp>

#include <string>

class Mesh
{
public:
	Mesh();

	void CreateMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::string name = "");
	void RenderMesh();
	void ClearMesh();

	std::string& GetName() { return name; }

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
	std::string name;
};

