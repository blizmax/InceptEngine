#include "Hornet.h"
#include <string>
#include "SkeletonMesh.h"
#include "Utils.h"
#include "GameWorld.h"
#include "Skeleton.h"
#include "Global.h"
#include "Animation.h"

Hornet::Hornet(GameWorld* world):
	Player(glm::mat4(1.0), world)
{
	m_world = world;
	std::string modelPath = "D:\\Inception\\Content\\Models\\HornetGL.FBX";
	std::string texturePath = "D:\\Inception\\Content\\Textures\\Hornet.HDR";
	ShaderPath shaderpath = { "D:\\Inception\\Content\\Shaders\\spv\\vertex.spv", "D:\\Inception\\Content\\Shaders\\spv\\fragment.spv" };
	SkeletonMesh* hornetMesh = SkeletonMesh::loadSkeletonMesh(m_world->m_renderer.get(), modelPath, shaderpath, texturePath, "root", false);

	setSkeletonMesh(hornetMesh);
	m_boneT = std::vector<glm::mat4>(70, glm::mat4(1.0));
	getSkeletonMesh()->initializeUniformBuffer(m_world->m_renderer.get(), m_boneT, m_world->m_light.get());
	getSkeletonMesh()->getSkeleton()->addSocket("hand_r", "swordSocket", swordSocket);
	std::vector<std::string> anims =
	{
		"D:\\Inception\\Content\\Models\\HornetBattleIdle.FBX",
		"D:\\Inception\\Content\\Models\\ThirdPersonRun.FBX",
		"D:\\Inception\\Content\\Models\\HornetNormalAttackOne.FBX",
	};
	getSkeletonMesh()->loadAnimation(anims, "root");
	getSkeletonMesh()->m_animations[2]->m_rootMotion = true;
}


Hornet::~Hornet()
{
}

void Hornet::update()
{

	m_boneT[1] = getActorTransformation();

	if (m_speed == 1 && !m_isRunning)
	{
		m_isRunning = true;
		playAnimation(getSkeletonMesh(), 1, &m_boneT, true);
	}
	else if(m_speed == 0 && m_isRunning)
	{
		m_isRunning = false;
		playAnimation(getSkeletonMesh(), 0, &m_boneT, true);
	}

	getSkeletonMesh()->m_boneTLock.lock();
	getSkeletonMesh()->updateUniformBuffer(m_world->m_renderer.get(), m_boneT, m_world->m_light.get());
	getSkeletonMesh()->m_boneTLock.unlock();
}
