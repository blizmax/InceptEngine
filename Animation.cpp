#include "Animation.h"
#include "Global.h"
Animation* Animation::loadAnimation(const std::string& filepath, SkeletonMesh* mesh, std::string rootBoneName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	assert(scene->HasAnimations());

	Skeleton* animationSkeleton = Skeleton::extractSkeletonFromAnimFile(scene, rootBoneName);

	if (!(*mesh->getSkeleton() == *animationSkeleton))
	{
		std::cerr << "Animation skeleton does not match skeleton of the mesh!" << std::endl;
		throw std::runtime_error("");
	}

	delete animationSkeleton;

	Animation* animation = new Animation();
	aiAnimation* panim = scene->mAnimations[0];
	animation->m_duration = (float)panim->mDuration;
	animation->m_tickPerSecond = (float)panim->mTicksPerSecond;
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

		animation->m_animation.insert(std::pair(currentBoneName, currentBoneTimeline));
	}


	cleanAnimation(animation, rootBoneName);

	return animation;

}

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

void Animation::setBonesTransformation(const Skeleton& skeleton, const Animation& anim, std::vector<glm::mat4>* boneT, float t)
{
	std::unordered_map<std::string, glm::mat4> boneTransformations;

	for (const auto& boneTimelinePair : anim.m_animation)
	{
		boneTransformations.insert(std::pair(boneTimelinePair.first, interpolateTransform(boneTimelinePair.second, t)));
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

		(*boneT)[boneID] = anim.m_rootTransform * finalTrans;
	}
}

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

Animation::Animation()
{
	m_rootTransform = FBX_Import_Mesh_Root_Transformation;
}
