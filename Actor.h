
#ifndef ACTOR_H
#define ACTOR_H


#include "Math.h"
#include <vector>

#include "ActorComponent.h"

struct LocalFrame
{
	glm::vec4 X = { 1,0,0,0 };
	glm::vec4 Y = { 0,1,0,0 };
	glm::vec4 Z = { 0,0,1,0 };
	glm::vec4 O = { 0,0,0,1 };

	glm::mat4 getLocalFrame();

	void translate(glm::vec3 direction, float amount);
	void rotate(glm::vec3 axis, float degree);
	void rotate(glm::vec4 axis, float degree);
	void scale(glm::vec3 scale);

};


class SkeletonMesh;
struct Socket;

class Actor
{
public:
	Actor(glm::mat4 rootTransform = glm::mat4(1.0));

	LocalFrame m_localFrame;

	void setSkeletonMesh(SkeletonMesh* mesh);

	SkeletonMesh* getSkeletonMesh();


	void setRootTransform(glm::mat4 t);

	glm::mat4 getModelTransformation()
	{
		return m_localFrame.getLocalFrame() * m_rootTransform;
	}

	~Actor();

private:
	SkeletonMesh* m_mesh;

	Socket* m_attchedSocket;

	std::vector<ActorComponent> m_components;

	std::string name;

	glm::mat4 m_rootTransform;
};

#endif