#pragma once

#include "Actor.h"
class SkeletonMesh;
class Renderer;
class GameWorld;

class Sword : public Actor
{
public:
	Sword(GameWorld* world);
	~Sword();
	virtual void update();
private:
	std::vector<glm::mat4> m_swordBoneT;
	
};

