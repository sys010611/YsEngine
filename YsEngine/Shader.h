#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <GL\glew.h>

class DirectionalLight;
class Material;

class Shader
{
public:
	Shader();

	void CreateFromFiles(const char* vertexLocation, const char* fragmentLocation);

	std::string ReadFile(const char* fileLocation);

	void UseShader();
	void ClearShader();

	void GetVariableLocations();

	void UseDirectionalLight(DirectionalLight* light);
	void UseMaterial(Material* material);

	GLuint GetModelMatLoc() { return modelMatLoc; }
	GLuint GetPVMLoc() { return PVMLoc; }
	GLuint GetSamplerLoc() { return samplerLoc; }
	GLuint GetNormalMatLoc() { return normalMatLoc; }
	GLuint GetEyePosLoc() { return eyePosLoc; }

	~Shader();

private:
	// handles
	GLuint shaderID,
		modelMatLoc, PVMLoc, samplerLoc, normalMatLoc,
		eyePosLoc;

	struct
	{
		GLuint ambientLoc;
		GLuint diffuseLoc;
		GLuint colorLoc;
		GLuint directionLoc;
	} directionalLightLoc;

	struct
	{
		GLuint specularLoc;
		GLuint shininessLoc;
	} materialLoc;

	void CompileShader(const char* vertexCode, const char* fragmentCode);
	void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};

