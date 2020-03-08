



/*
void skeletonMeshExport(const SkeletonMesh& sk)
{
	std::string vertexInfo;
	for (auto vertex : sk.m_vertices)
	{
		vertexInfo.append(glm::to_string(vertex.position));
		vertexInfo.append("@");
		vertexInfo.append(glm::to_string(vertex.boneWeights));
		vertexInfo.append("@");
		vertexInfo.append(glm::to_string(vertex.affectedBonesID));
		vertexInfo.append("\n");
	}

	std::string indicesInfo;
	for (auto index : sk.m_indices)
	{
		indicesInfo.append(std::to_string(index));
		indicesInfo.append("\n");
	}

	std::ofstream myfile;
	myfile.open("hornetVertices.txt");
	myfile << vertexInfo;
	myfile.close();


	myfile.open("hornetIndices.txt");
	myfile << indicesInfo;
	myfile.close();



	std::cout << "Export finish" << std::endl;
}
*/
/*
#include "Skeleton.h"
#include "Animation.h"
#include <iostream>
#include <fstream>

void animationExport(const Skeleton& sk, const Animation& anim)
{
	std::string animInfo = "";
	float step = anim.m_duration / 250;
	float time = 0;
	for (int i = 0; i < 245; i++)
	{
		animInfo.append(std::to_string(time));
		auto boneT = getBonesTransformation(sk, anim, time);


		for (auto transformation : boneT)
		{
			animInfo.append("@");
			animInfo.append(glm::to_string(transformation));
		}
		animInfo.append("\n");
		time += step;
	}

	std::ofstream myfile;
	myfile.open("hornetNormalAttackDef.txt");
	myfile << animInfo;
	myfile.close();

	std::cout << "Export finish" << std::endl;
}
*/