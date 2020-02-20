
#include "Actor.h"

#include "Skeleton.h"

Actor::Actor(glm::mat4 rootTransform)
{
	m_attchedSocket = nullptr;
	m_components = {};
	m_mesh = nullptr;
	name = "";
	m_rootTransform = rootTransform;
}


void Actor::setSkeletonMesh(SkeletonMesh* mesh)
{
	m_mesh = mesh;
}

SkeletonMesh* Actor::getSkeletonMesh()
{
	return m_mesh;
}

Actor::~Actor()
{
	if (m_attchedSocket != nullptr)
	{
		delete m_attchedSocket;
	}
}
