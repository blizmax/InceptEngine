
#include "Actor.h"
#include "SkeletonMesh.h"
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

void Actor::setRootTransform(glm::mat4 t)
{
	m_rootTransform = t;
}

Actor::~Actor()
{
	if (m_attchedSocket != nullptr)
	{
		delete m_attchedSocket;
	}

	if (m_mesh != nullptr)
	{
		delete m_mesh;
	}

	for (auto component : m_components)
	{
		if (component != nullptr)
		{
			delete component;
		}
	}
}

glm::mat4 LocalFrame::getLocalFrame()
{
	return glm::mat4(X, Y, Z, O);
}

void LocalFrame::translate(glm::vec3 direction, float amount)
{
	O += amount * glm::vec4(direction, 0);
}

void LocalFrame::rotate(glm::vec3 axis, float degree)
{
	auto rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(degree), axis);
	X = rotationMatrix * X;
	Y = rotationMatrix * Y;
	Z = rotationMatrix * Z;
}

void LocalFrame::rotate(glm::vec4 axis, float degree)
{
	auto rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(degree), glm::vec3(axis.x, axis.y, axis.z));
	X = rotationMatrix * X;
	Y = rotationMatrix * Y;
	Z = rotationMatrix * Z;


}

void LocalFrame::scale(glm::vec3 scale)
{
	auto scaleMatrix = glm::scale(scale);
	X = scaleMatrix * X;
	Y = scaleMatrix * Y;
	Z = scaleMatrix * Z;
}
