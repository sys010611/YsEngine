#ifndef COMMONVALS
#define COMMONVALS

#include "stb_image.h"
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

const int MAX_POINT_LIGHTS = 3;

const int MAX_BONE_INFLUENCE = 4;
const int MAX_BONE_COUNT = 100;

struct Vertex
{
	Vertex() {}

	Vertex(float px, float py, float pz)
	{
		Position.x = px; Position.y = py; Position.z = pz;
		TexCoords.s = 0.f; TexCoords.t = 0.f;
		Normal.x = 0.f; Normal.y = 0.f; Normal.z = 0.f;
	}

	glm::vec3 Position;
	glm::vec2 TexCoords;
	glm::vec3 Normal;

	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;

	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct BoneInfo
{
	int id; // finalBoneMatrices의 index
	glm::mat4 offset;
};

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childcount;
	std::vector<AssimpNodeData> children;
};

#endif