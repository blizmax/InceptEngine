#ifndef ANIMATION_H
#define ANIMATION_H

#include "Skeleton.h"
#include "SkeletonMesh.h"
#include <chrono>

class ScaleKeyFrame
{
public:
	float m_timeStamp = 0;

	glm::vec3 m_boneScale;

	friend bool operator< (const ScaleKeyFrame& frame1, const ScaleKeyFrame& frame2)
	{
		return frame1.m_timeStamp < frame2.m_timeStamp;
	}
};


class RotationKeyFrame
{
public:
	float m_timeStamp = 0;

	glm::quat m_boneRotation;

	friend bool operator< (const RotationKeyFrame& frame1, const RotationKeyFrame& frame2)
	{
		return frame1.m_timeStamp < frame2.m_timeStamp;
	}
};




class TranslationKeyFrame
{
public:
	float m_timeStamp = 0;

	glm::vec3 m_boneTranslation;

	friend bool operator< (const TranslationKeyFrame& frame1, const TranslationKeyFrame& frame2)
	{
		return frame1.m_timeStamp < frame2.m_timeStamp;
	}
};


class BoneTransformTimeline
{
public:
	std::string m_boneName = "";

	std::set<ScaleKeyFrame> m_scaleKeys;

	std::set<RotationKeyFrame> m_rotationKeys;

	std::set<TranslationKeyFrame> m_translationKeys;

};



class Animation
{
public:
	Animation();

	float m_tickPerSecond = 0;

	float m_duration = 0;

	glm::mat4 m_rootTransform;

	std::unordered_map<std::string, BoneTransformTimeline> m_animation;
};



Animation* loadAnimation(const std::string& filepath, SkeletonMesh* mesh, std::string rootBoneName);



//assume the three timeline are sync
glm::mat4 interpolateTransform(const BoneTransformTimeline& timeline, float t);

std::vector<glm::mat4> getBonesTransformation(const Skeleton& skeleton, const Animation& anim, float t);

//solve the root bone problem
void cleanAnimation(Animation* anim, std::string rootName);



#endif