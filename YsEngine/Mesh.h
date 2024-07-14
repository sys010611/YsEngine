#pragma once

#include <GL\glew.h>

class Mesh
{
public:
	Mesh();

	void CreateMesh(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices);
	void RenderMesh();
	void ClearMesh();

	~Mesh();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};

