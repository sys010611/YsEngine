#include "Shader.h"

#include "DirectionalLight.h"
#include "Material.h"

Shader::Shader()
{
	shaderID = 0;

	modelMatLoc=0;
	PVMLoc = 0;
	colorSamplerLoc = 0;
	normalSamplerLoc = 0;
	normalMatLoc = 0;
}

/// <summary>
/// 쉐이더 파일을 불러와서 컴파일
/// </summary>
void Shader::CreateFromFiles(const char* vertexLocation, const char* fragmentLocation)
{
	std::string vertexString = ReadFile(vertexLocation);
	std::string fragmentString = ReadFile(fragmentLocation);
	const char* vertexCode = vertexString.c_str();
	const char* fragmentCode = fragmentString.c_str();

	CompileShader(vertexCode, fragmentCode);
}

std::string Shader::ReadFile(const char* fileLocation)
{
	std::string content;
	std::ifstream fileStream(fileLocation, std::ios::in);

	if (!fileStream.is_open())
	{
		printf("Failed to read %s! File doesn't exist.", fileLocation);
		return "";
	}

	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

void Shader::CompileShader(const char* vertexCode, const char* fragmentCode)
{
	shaderID = glCreateProgram();

	if (!shaderID)
	{
		printf("Error creating shader program!\n");
		return;
	}

	AddShader(shaderID, vertexCode, GL_VERTEX_SHADER);
	AddShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error linking program: '%s'\n", eLog);
		return;
	}

	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}

	// 쉐이더 컴파일 성공, 쉐이더 내부 변수들 위치 가져오기
	GetVariableLocations();
}

void Shader::UseShader()
{
	glUseProgram(shaderID);
}

void Shader::ClearShader()
{
	if (shaderID != 0)
	{
		glDeleteProgram(shaderID);
		shaderID = 0;
	}
}

void Shader::GetVariableLocations()
{
	modelMatLoc = glGetUniformLocation(shaderID, "modelMat");
	PVMLoc = glGetUniformLocation(shaderID, "PVM");
	colorSamplerLoc = glGetUniformLocation(shaderID, "colorSampler");
	normalSamplerLoc = glGetUniformLocation(shaderID, "normalSampler");
	normalMatLoc = glGetUniformLocation(shaderID, "normalMat");

	directionalLightLoc.ambientLoc = glGetUniformLocation(shaderID, "directionalLight.base.ambient");
	directionalLightLoc.diffuseLoc = glGetUniformLocation(shaderID, "directionalLight.base.diffuse");
	directionalLightLoc.colorLoc = glGetUniformLocation(shaderID, "directionalLight.base.color");
	directionalLightLoc.directionLoc = glGetUniformLocation(shaderID, "directionalLight.direction");

	materialLoc.specularLoc = glGetUniformLocation(shaderID, "material.specular");
	materialLoc.shininessLoc = glGetUniformLocation(shaderID, "material.shininess");

	eyePosLoc = glGetUniformLocation(shaderID, "eyePosition");
	finalBonesMatricesLoc = glGetUniformLocation(shaderID, "finalBonesMatrices");
}

void Shader::UseDirectionalLight(DirectionalLight* light)
{
	glUniform1f(directionalLightLoc.ambientLoc, light->ambient);
	glUniform1f(directionalLightLoc.diffuseLoc, light->diffuse);
	glUniform4f(directionalLightLoc.colorLoc, 
		light->color.r,
		light->color.g,
		light->color.b,
		light->color.a);
	glUniform3f(directionalLightLoc.directionLoc, 
		light->direction.x,
		light->direction.y,
		light->direction.z);
}

void Shader::UseMaterial(Material* material)
{
	glUniform1f(materialLoc.specularLoc, material->specular);
	glUniform1f(materialLoc.shininessLoc, material->shininess);
}

void Shader::AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);

	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}

	glAttachShader(theProgram, theShader);
}

void Shader::Validate()
{
	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}
}

Shader::~Shader()
{
	ClearShader();
}
