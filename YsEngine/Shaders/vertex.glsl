#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 normal;

out vec3 FragPos; // 월드 좌표계
out vec2 TexCoord;
out vec3 FragNormal;

uniform mat4 modelMat;
uniform mat4 PVM;
uniform mat3 normalMat;

void main()
{
	gl_Position = PVM * vec4(pos, 1.0);

	FragPos = (modelMat * vec4(pos, 1.0)).xyz; 
	TexCoord = tex;
	FragNormal = normalMat * normal;
}