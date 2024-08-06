#pragma once

#include <GL\glew.h>

class Texture
{
public:
	Texture(const char* fileLoc);

	/// <summary>
	/// 텍스쳐 메모리로 로드, GPU로 쏴주기
	/// </summary>
	bool LoadDiffuse();
	bool LoadNormal();
	void UseTexture(GLenum textureUnit);
	void ClearTexture(); // 메모리에서 텍스쳐 내리기

	~Texture();

private:
	GLuint textureID;
	int width, height, bitDepth;

	const char* fileLocation;
};

