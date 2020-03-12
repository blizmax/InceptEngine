#include "Sword.h"
#include "SkeletonMesh.h"
#include "Utils.h"
#include "GameWorld.h"
#include "Skeleton.h"
#include "Player.h"
#include "Hornet.h"


Sword::Sword(GameWorld* world)
	:Actor(glm::mat4(1.0), world)
{
	std::string swordPath = "D:\\Inception\\Content\\Models\\ElementSword.obj";
	std::string swordTexturePath = "D:\\Inception\\Content\\Textures\\Firesword.HDR";
	ShaderPath shaderpath = { "D:\\Inception\\Content\\Shaders\\spv\\vertex.spv", "D:\\Inception\\Content\\Shaders\\spv\\fragment.spv" };

	SkeletonMesh* mesh = SkeletonMesh::loadSkeletonMesh(m_world->m_renderer.get(), swordPath, shaderpath, swordTexturePath, "", true);
	setSkeletonMesh(mesh);
	m_swordBoneT = std::vector<glm::mat4>(2, glm::mat4(1.0));
	m_attchedSocket = m_world->m_player->getSkeletonMesh()->getSkeleton()->getSocket("swordSocket");
	update();
}

Sword::~Sword()
{
}

void Sword::update()
{
	m_swordBoneT[1] = m_world->m_player->getActorTransformation() * m_world->m_player->getSkeletonMesh()->getSkeleton()->getSocketLocation(m_attchedSocket->m_socketName, (dynamic_cast<Hornet*>(m_world->m_player.get()))->m_boneT);
	getSkeletonMesh()->updateUniformBuffer(m_world->m_renderer.get(), m_swordBoneT, m_world->m_light.get());
}
