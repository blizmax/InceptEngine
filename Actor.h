
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
	//void setLocalFrame();


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
	Actor(glm::mat4 startTransformation);

	void setSkeletonMesh(SkeletonMesh* mesh);

	SkeletonMesh* getSkeletonMesh();

	void setActorTransformation(glm::mat4 t);

	glm::mat4 getActorTransformation();

	void translate(glm::vec3 direction, float amount);
	void rotate(glm::vec3 axis, float degree);
	void rotate(glm::vec4 axis, float degree);
	void scale(glm::vec3 scale);
	void setActorLocation(glm::vec4 location);
	void setActorHeight(float height);


	glm::vec4 getForwardVector();

	glm::vec4 getRightWardVector();

	glm::vec4 getUpWardVector();

	glm::vec4 getActorLocation();

	~Actor();



private:
	SkeletonMesh* m_mesh;

	Socket* m_attchedSocket;

	std::vector<ActorComponent*> m_components;

	std::string name;

	LocalFrame m_localFrame;
};

#endif