

#include "Renderer.h"
#include "WindowEventCallback.h"
#include <stdio.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <stack>
#include <chrono>


glm::mat4 mat4_cast(const aiMatrix4x4& ai)
{
	glm::mat4 mat;

	mat[0][0] = ai.a1; mat[1][0] = ai.a2; mat[2][0] = ai.a3; mat[3][0] = ai.a4;
	mat[0][1] = ai.b1; mat[1][1] = ai.b2; mat[2][1] = ai.b3; mat[3][1] = ai.b4;
	mat[0][2] = ai.c1; mat[1][2] = ai.c2; mat[2][2] = ai.c3; mat[3][2] = ai.c4;
	mat[0][3] = ai.d1; mat[1][3] = ai.d2; mat[2][3] = ai.d3; mat[3][3] = ai.d4;

	return mat;
}

struct Bone
{
	Bone()
	{

	}
	Bone(std::string boneName, std::string parent, glm::mat4 transformation)
	{
		m_name = boneName;
		m_parent = parent;
		m_transformToParent = transformation;
	}
	std::string m_name;
	std::string m_parent;
	unsigned int m_boneId;
	glm::mat4 m_offset;
	glm::mat4 m_transformToParent;
};


class Skeleton
{
public:
	std::unordered_map<std::string, Bone> m_bones;

	bool operator == (const Skeleton& other)
	{
		std::set<std::string> othersBoneNames;
		for (auto bone : other.m_bones)
		{
			othersBoneNames.insert(bone.first);
		}
		for (auto bone : this->m_bones)
		{
			if (othersBoneNames.count(bone.first) == 0)
			{
				std::cout << bone.first << std::endl;
				return false;
			}
			othersBoneNames.erase(bone.first);
		}
		return othersBoneNames.empty();
	}

};


//not support LOD for now
class SkeletonMesh
{
public:
	SkeletonMesh()
	{
		m_vertices.resize(0);
		m_indices.resize(0);
	}
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	Skeleton m_skeleton;
	void addBoneToVertex(unsigned int boneID, unsigned int vertexID, float weights)
	{
		float min = 1.0f;
		unsigned int minIdx = 0;
		for (unsigned int i = 0; i < MAX_BONE_PER_VERTEX; i++)
		{
			if (m_vertices[vertexID].boneWeights[i] < min)
			{
				min = m_vertices[vertexID].boneWeights[i];
				minIdx = i;
			}
		}
		
		m_vertices[vertexID].boneWeights[minIdx] = weights;
		m_vertices[vertexID].affectedBonesID[minIdx] = boneID;
	}

private:

};


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

void traceRootBone(aiNode* curNode, aiNode*& pRootNode, const std::string& rootBoneName)
{
	if (strcmp(curNode->mName.C_Str(), rootBoneName.c_str()) == 0)
	{
		
		pRootNode = curNode;
		return;
	}

	if (curNode->mNumChildren == 0)
	{
		return;
	}

	for (unsigned int i = 0; i < curNode->mNumChildren; i++)
	{
		traceRootBone(curNode->mChildren[i], pRootNode, rootBoneName);
	}
}

Skeleton extractSkeletonFromAnimFile(const aiScene* scene, const std::string& rootBoneName)
{
	Skeleton skeleton;
	std::stack<aiNode*> nodes;
	
	aiNode* pRootBone = nullptr;

	traceRootBone(scene->mRootNode, pRootBone, rootBoneName);


	if (pRootBone == nullptr)
	{
		throw std::runtime_error("");
	}

	unsigned int currentID = 0;
	Bone rootBone(pRootBone->mName.C_Str(), "", glm::mat4(1.0));
	rootBone.m_boneId = currentID;
	currentID++;
	skeleton.m_bones.insert(std::pair(pRootBone->mName.C_Str(), rootBone));

	nodes.push(pRootBone);

	while (! nodes.empty())
	{
		aiNode* currentNode = nodes.top();
		nodes.pop();
		for (unsigned int i = 0; i < currentNode->mNumChildren; i++)
		{
			aiNode* child = currentNode->mChildren[i];
			Bone bone(child->mName.C_Str(), currentNode->mName.C_Str(), mat4_cast(child->mTransformation));
			bone.m_boneId = currentID;
			currentID++;
			skeleton.m_bones.insert(std::pair(child->mName.C_Str(), bone));
			nodes.push(child);
		}
	}
	
	for (unsigned int i = 0; i < scene->mMeshes[0]->mNumBones; i++)
	{
		aiBone* curBone = scene->mMeshes[0]->mBones[i];
		std::string boneName = curBone->mName.C_Str();
		skeleton.m_bones.at(boneName).m_offset = mat4_cast(curBone->mOffsetMatrix);
	}
	


	return skeleton;

}


Animation loadAnimation(const std::string& filepath, SkeletonMesh mesh, std::string rootBoneName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	assert(scene->HasAnimations());
	
	if (!(mesh.m_skeleton == extractSkeletonFromAnimFile(scene, rootBoneName)))
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



void printModelStats(const aiScene* scene)
{
	std::cout << "The model has " << scene->mNumMeshes << " meshes" << std::endl;
	std::cout << "The model has " << scene->mNumAnimations << " anims" << std::endl;
	std::cout << "The model has " << scene->mNumCameras << " cameras" << std::endl;
	std::cout << "The model has " << scene->mNumLights << " lights" << std::endl;
	std::cout << "The model has " << scene->mNumMaterials << " materials" << std::endl;
	std::cout << "The model has " << scene->mNumTextures << " textures" << std::endl;
	
}

SkeletonMesh loadSkeletonMesh(const std::string& filepath, const std::string& rootBoneName)
{

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	
	if (!scene)
	{
		throw std::runtime_error("");
	}

	printModelStats(scene);

	
	if (!scene->HasMeshes())
	{
		throw std::runtime_error("");
	}


	SkeletonMesh skMesh;
	aiMesh* mesh = scene->mMeshes[0];
	
	


	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex v;
		v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1 };
		if (mesh->HasVertexColors(i))
		{
			v.color = { mesh->mColors[i][0].r, mesh->mColors[i][0].g, mesh->mColors[i][0].b, 1.0 };
			std::cout << i << std::endl;
		}
		else
		{
			v.color = { 0.0, 0.0, 0.0, 1.0 };
		}
		
		skMesh.m_vertices.push_back(v);
	}
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace* face = &mesh->mFaces[i];
		assert(face->mNumIndices == 3);
		skMesh.m_indices.push_back((uint32_t)face->mIndices[0]);
		skMesh.m_indices.push_back((uint32_t)face->mIndices[1]);
		skMesh.m_indices.push_back((uint32_t)face->mIndices[2]);
	}
	assert(skMesh.m_indices.size() % 3 == 0);

	skMesh.m_skeleton = extractSkeletonFromAnimFile(scene, rootBoneName);

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		if (skMesh.m_skeleton.m_bones.count(mesh->mBones[i]->mName.C_Str()) == 0)
		{
			throw std::runtime_error("");
		}
	}

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		std::string curBoneName = mesh->mBones[i]->mName.C_Str();
		unsigned int curBoneID = skMesh.m_skeleton.m_bones.at(curBoneName).m_boneId;
		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
		{
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			unsigned int affectedVertexId = mesh->mBones[i]->mWeights[j].mVertexId;
			skMesh.addBoneToVertex(curBoneID, affectedVertexId, weight);
		}
	}


	return skMesh;
}

void printSkeletonBones(Skeleton sk)
{
	for (auto bone : sk.m_bones)
	{
		std::cout << bone.first << std::endl;
	}
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
		if(t < it.m_timeStamp)
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
		boneTransformations.insert(std::pair(boneTimelinePair.first ,interpolateTransform(boneTimelinePair.second, t)));
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
		glm::mat4 finalTrans =  skeleton.m_bones.at(bone.first).m_offset;

		
		while (currentBone->m_parent != "")
		{
			finalTrans = boneTransformations.at(currentBone->m_name) * finalTrans;
			currentBone = &skeleton.m_bones.at(currentBone->m_parent);
		}

		boneTransAfterProp[boneID] = finalTrans;
	}
 
	
	return boneTransAfterProp;

}

int main()
{
	SkeletonMesh mesh = loadSkeletonMesh("D:\\Inception\\Content\\Models\\SK_Hornet.FBX", "root");

	Animation anim = loadAnimation("D:\\Inception\\Content\\Models\\HornetNormalAttackOne.FBX", mesh, "root");

	std::cout << "load finish" << std::endl;


	Renderer renderer;
	renderer.setVertices(mesh.m_vertices, mesh.m_indices);
	renderer.init();


	auto m_window = renderer.getWindow();

	glfwSetInputMode(*m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	unsigned char pixels[16 * 16 * 4];
	memset(pixels, 0xff, sizeof(pixels));
	GLFWimage image;
	image.width = 16;
	image.height = 16;
	image.pixels = pixels;
	GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
	glfwSetCursor(*m_window, cursor);

	glfwSetWindowUserPointer(*m_window, &renderer);
	glfwSetFramebufferSizeCallback(*renderer.getWindow(), framebufferResizeCallback);
	glfwSetKeyCallback(*renderer.getWindow(), key_callback);

	auto startTime = std::chrono::high_resolution_clock::now(); 
	
	float time = 0.0f;

	while (!glfwWindowShouldClose(*m_window))
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		if (time > 1.8f)
		{
			startTime = std::chrono::high_resolution_clock::now();
		}

		auto boneT = getBonesTransformation(mesh.m_skeleton, anim, time);

		glfwPollEvents();

		renderer.drawTriangle(boneT);
	
	}
	
	
}



/*
		for (unsigned int i = 0; i < mesh.m_vertices.size(); i++)
		{
			auto vertex = verticesCopy[i];
			glm::mat4 vertexTrans = glm::mat4(0.0);
			for (unsigned int i = 0; i < MAX_BONE_PER_VERTEX; i++)
			{
				vertexTrans += vertex.boneWeights[i] * boneT[vertex.affectedBonesID[i]];
			}

			mesh.m_vertices[i].position = vertexTrans * vertex.position;
		}

		renderer.setVertices(mesh.m_vertices, mesh.m_indices);
		*/
