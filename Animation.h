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
	float m_tickPerSecond = 0;

	float m_duration = 0;

	std::unordered_map<std::string, BoneTransformTimeline> m_animation;
};



Animation loadAnimation(const std::string& filepath, SkeletonMesh mesh, std::string rootBoneName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	assert(scene->HasAnimations());

	Skeleton animationSkeleton = extractSkeletonFromAnimFile(scene, rootBoneName);

	if (!(mesh.m_skeleton == animationSkeleton))
	{
		std::cerr << "Animation skeleton does not match skeleton of the mesh!" << std::endl;
		throw std::runtime_error("");
	}

	Animation animation;
	aiAnimation* panim = scene->mAnimations[0];
	animation.m_duration = (float)panim->mDuration;
	animation.m_tickPerSecond = (float)panim->mTicksPerSecond;
	for (unsigned int i = 0; i < panim->mNumChannels; i++)
	{
		aiNodeAnim* currentBoneAnim = panim->mChannels[i];

		std::string currentBoneName = currentBoneAnim->mNodeName.C_Str();

		BoneTransformTimeline currentBoneTimeline;

		currentBoneTimeline.m_boneName = currentBoneName;

		for (unsigned int j = 0; j < currentBoneAnim->mNumPositionKeys; j++)
		{
			TranslationKeyFrame curTransltionKeyFrame;
			curTransltionKeyFrame.m_timeStamp = (float)currentBoneAnim->mPositionKeys[j].mTime;
			curTransltionKeyFrame.m_boneTranslation = { currentBoneAnim->mPositionKeys[j].mValue.x, currentBoneAnim->mPositionKeys[j].mValue.y, currentBoneAnim->mPositionKeys[j].mValue.z };
			currentBoneTimeline.m_translationKeys.insert(curTransltionKeyFrame);
		}

		for (unsigned int j = 0; j < currentBoneAnim->mNumScalingKeys; j++)
		{
			ScaleKeyFrame curScaleKeyFrame;
			curScaleKeyFrame.m_timeStamp = (float)currentBoneAnim->mScalingKeys[j].mTime;
			curScaleKeyFrame.m_boneScale = { currentBoneAnim->mScalingKeys[j].mValue.x,currentBoneAnim->mScalingKeys[j].mValue.y,currentBoneAnim->mScalingKeys[j].mValue.z };
			currentBoneTimeline.m_scaleKeys.insert(curScaleKeyFrame);
		}

		for (unsigned int j = 0; j < currentBoneAnim->mNumRotationKeys; j++)
		{
			RotationKeyFrame rotationKeyFrame;
			rotationKeyFrame.m_timeStamp = (float)currentBoneAnim->mRotationKeys[j].mTime;

			rotationKeyFrame.m_boneRotation = { currentBoneAnim->mRotationKeys[j].mValue.w, currentBoneAnim->mRotationKeys[j].mValue.x, currentBoneAnim->mRotationKeys[j].mValue.y, currentBoneAnim->mRotationKeys[j].mValue.z };
			currentBoneTimeline.m_rotationKeys.insert(rotationKeyFrame);
		}

		animation.m_animation.insert(std::pair(currentBoneName, currentBoneTimeline));
	}



	return animation;



}



//assume the three timeline are sync
glm::mat4 interpolateTransform(const BoneTransformTimeline& timeline, float t)
{
	glm::mat4 scale(1.0);
	glm::mat4 rotation(1.0);
	glm::mat4 translation(1.0);

	for (const auto& it : timeline.m_scaleKeys)
	{
		if (t < it.m_timeStamp)
		{
			scale = glm::scale(glm::mat4(1.0), it.m_boneScale);
			break;
		}

	}

	for (const auto& it : timeline.m_rotationKeys)
	{
		if (t < it.m_timeStamp)
		{
			rotation = glm::toMat4(it.m_boneRotation);
			break;
		}

	}

	for (const auto& it : timeline.m_translationKeys)
	{
		if (t < it.m_timeStamp)
		{
			translation = glm::translate(glm::mat4(1.0), it.m_boneTranslation);
			break;
		}

	}

	return translation * rotation * scale;
}

std::vector<glm::mat4> getBonesTransformation(const Skeleton& skeleton, const Animation& anim, float t)
{
	std::unordered_map<std::string, glm::mat4> boneTransformations;

	for (const auto& boneTimelinePair : anim.m_animation)
	{
		boneTransformations.insert(std::pair(boneTimelinePair.first, interpolateTransform(boneTimelinePair.second, t)));
	}


	std::vector<glm::mat4> boneTransAfterProp;
	boneTransAfterProp.resize(skeleton.m_bones.size());
	for (auto& bone : boneTransAfterProp)
	{
		bone = glm::mat4(1.0);
	}

	for (const auto& bone : boneTransformations)
	{
		unsigned int boneID = skeleton.m_bones.at(bone.first).m_boneId;


		const Bone* currentBone = &skeleton.m_bones.at(bone.first);
		glm::mat4 finalTrans = skeleton.m_bones.at(bone.first).m_offset;


		while (currentBone->m_parent != "")
		{
			finalTrans = boneTransformations.at(currentBone->m_name) * finalTrans;
			currentBone = &skeleton.m_bones.at(currentBone->m_parent);
		}

		boneTransAfterProp[boneID] = finalTrans;
	}


	return boneTransAfterProp;

}

//solve the root bone problem
void cleanAnimation(Animation* anim, std::string rootName)
{
	BoneTransformTimeline timeline;
	timeline.m_boneName = rootName;
	std::string translationName = "";
	std::string rotationName = "";
	std::string scaleName = "";
	for (auto bone : anim->m_animation)
	{
		if (bone.first.find("root") != std::string::npos && bone.first.find("Translation") != std::string::npos)
		{
			timeline.m_translationKeys = bone.second.m_translationKeys;
			translationName = bone.first;
			break;
		}
	}

	for (auto bone : anim->m_animation)
	{
		if (bone.first.find("root") != std::string::npos && bone.first.find("Rotation") != std::string::npos)
		{
			timeline.m_rotationKeys = bone.second.m_rotationKeys;
			rotationName = bone.first;
			break;
		}
	}

	for (auto bone : anim->m_animation)
	{
		if (bone.first.find("root") != std::string::npos && bone.first.find("Scaling") != std::string::npos)
		{
			timeline.m_scaleKeys = bone.second.m_scaleKeys;
			scaleName = bone.first;
			break;
		}
	}

	if (translationName != "" && rotationName != "" && scaleName != "")
	{
		anim->m_animation.erase(translationName);
		anim->m_animation.erase(rotationName);
		anim->m_animation.erase(scaleName);
		anim->m_animation.insert(std::pair(timeline.m_boneName, timeline));
	}


}



#endif