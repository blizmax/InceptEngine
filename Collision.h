#ifndef COLLISION_H
#define COLLISION_H

#include "Math.h"

class Actor;


struct CapsuleCollision
{
	CapsuleCollision(Actor* owner, glm::vec4 A = glm::vec4(0,0,0,1), glm::vec4 B = glm::vec4(0,50,0,1), float r = 5);
	~CapsuleCollision();

	//equal mean two capsule are intersecting
	bool operator == (const CapsuleCollision& other);

	glm::vec4 m_A;
	glm::vec4 m_B;
	float m_R;
	Actor* m_owner = nullptr;
};

#endif
