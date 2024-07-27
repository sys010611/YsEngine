#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <GL\glew.h>

#include "CommonValues.h"

class DirectionalLight;
class PointLight;
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
	void UsePointLights(PointLight** pointLights, unsigned int count);
	void UseMaterial(Material* material);
	void UseFinalBoneMatrices(const std::vector<glm::mat4>& transforms);

	GLuint GetID() { return shaderID; }

	GLuint GetModelMatLoc() { return modelMatLoc; }
	GLuint GetPVMLoc() { return PVMLoc; }
	GLuint GetColorSamplerLoc() { return colorSamplerLoc; }
	GLuint GetNormalSamplerLoc() { return normalSamplerLoc; }
	GLuint GetNormalMatLoc() { return normalMatLoc; }
	GLuint GetEyePosLoc() { return eyePosLoc; }
	GLuint GetFinalBonesMatricesLoc() { return finalBonesMatricesLoc; }

	void Validate();

	~Shader();

private:
	// handles
	GLuint shaderID,
		modelMatLoc, PVMLoc, 
		colorSamplerLoc, normalSamplerLoc,
		normalMatLoc,
		eyePosLoc, finalBonesMatricesLoc,
		pointLightCountLoc;

	struct
	{
		GLuint ambientIntensityLoc;
		GLuint diffuseIntensityLoc;
		GLuint colorLoc;

		GLuint directionLoc;
	} directionalLightLoc;

	struct
	{
		GLuint ambientIntensityLoc;
		GLuint diffuseIntensityLoc;
		GLuint colorLoc;

		GLuint positionLoc;
		GLuint constantLoc;
		GLuint linearLoc;
		GLuint exponentLoc;
	} pointLightLoc[MAX_POINT_LIGHTS];

	struct
	{
		GLuint specularIntensityLoc;
		GLuint shininessLoc;
	} materialLoc;

	void CompileShader(const char* vertexCode, const char* fragmentCode);
	void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);
};

