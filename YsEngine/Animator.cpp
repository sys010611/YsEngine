#include "Animator.h"

#include "Animation.h"
#include "Bone.h"

Animator::Animator(Animation* animation)
{
	currentTime = 0.f;
	currentAnimation = animation;

	finalBoneMatrices.reserve(100);

	for (int i = 0; i < 100; i++)
	{
		finalBoneMatrices.push_back(glm::mat4(1.f));
	}
}

void Animator::UpdateAnimation(float deltaTime)
{
	this->deltaTime = deltaTime;
	if (currentAnimation)
	{
		currentTime += currentAnimation->GetTicksPerSecond() * deltaTime;
		currentTime += fmod(currentTime, currentAnimation->GetDuration());
		CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.f));
	}
}

void Animator::PlayAnimation(Animation* pAnimation)
{
	currentAnimation = pAnimation;
	currentTime = 0.f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
	std::string nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	Bone* bone = currentAnimation->FindBone(nodeName);

	if (bone)
	{
		bone->Update(currentTime);
		nodeTransform = bone->GetLocalTransform();
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = currentAnimation->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;
		finalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childcount; i++)
	{
		CalculateBoneTransform(&node->children[i], globalTransformation);
	}
}

std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
{
	return finalBoneMatrices;
}