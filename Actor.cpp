
#include "Actor.h"
#include "SkeletonMesh.h"
#include "Skeleton.h"

Actor::Actor(glm::mat4 startTransformation)
{
	m_attchedSocket = nullptr;
	m_components = {};
	m_mesh = nullptr;
	name = "";
	m_localFrame.X = startTransformation[0];
	m_localFrame.Y = startTransformation[1];
	m_localFrame.Z = startTransformation[2];
	m_localFrame.O = startTransformation[3];
}


void Actor::setSkeletonMesh(SkeletonMesh* mesh)
{
	m_mesh = mesh;
}

SkeletonMesh* Actor::getSkeletonMesh()
{
	return m_mesh;
}

void Actor::setActorTransformation(glm::mat4 t)
{
	m_localFrame.X = t[0];
	m_localFrame.Y = t[1];
	m_localFrame.Z = t[2];
	m_localFrame.O = t[3];
}

//const glm::mat4 swordSocketTransformation = glm::translation([0, 2, 0]).times(Mat4.rotation(-0.2, Vec.of(0, 1, 0))).times(Mat4.rotation(0.2, Vec.of(1, 0, 0))).times(Mat4.translation([-5, 5, 0])).times(Mat4.rotation(0.8, Vec.of(0, 0, 1))).times(Mat4.translation([0, 0, -5])).times(Mat4.translation([10, 0, 0])).times(Mat4.translation([0, -5, 0])).times(Mat4.translation([0, 0, -10])).times(Mat4.translation([-20, 0, 0]));


glm::mat4 Actor::getActorTransformation()
{
	return m_localFrame.getLocalFrame();
}

void Actor::translate(glm::vec3 direction, float amount)
{
	m_localFrame.O += amount * glm::vec4(direction, 0);
}

void Actor::rotate(glm::vec3 axis, float degree)
{
	auto rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(degree), axis);
	m_localFrame.X = rotationMatrix * m_localFrame.X;
	m_localFrame.Y = rotationMatrix * m_localFrame.Y;
	m_localFrame.Z = rotationMatrix * m_localFrame.Z;
}

void Actor::rotate(glm::vec4 axis, float degree)
{
	auto rotationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(degree), glm::vec3(axis.x, axis.y, axis.z));
	m_localFrame.X = rotationMatrix * m_localFrame.X;
	m_localFrame.Y = rotationMatrix * m_localFrame.Y;
	m_localFrame.Z = rotationMatrix * m_localFrame.Z;
}

void Actor::scale(glm::vec3 scale)
{
	auto scaleMatrix = glm::scale(scale);
	m_localFrame.X = scaleMatrix * m_localFrame.X;
	m_localFrame.Y = scaleMatrix * m_localFrame.Y;
	m_localFrame.Z = scaleMatrix * m_localFrame.Z;
}

void Actor::setActorLocation(glm::vec4 location)
{
	m_localFrame.O = location;
}

void Actor::setActorHeight(float height)
{
	m_localFrame.O.y = height;
}

glm::vec4 Actor::getForwardVector()
{
	return -m_localFrame.Z;
}

glm::vec4 Actor::getRightWardVector()
{
	return m_localFrame.X;
}

glm::vec4 Actor::getUpWardVector()
{
	return m_localFrame.Y;
}

glm::vec4 Actor::getActorLocation()
{
	return m_localFrame.O;
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
