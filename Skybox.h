#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
class GameWorld;
struct CubeMap;

#include "Actor.h"
#include <vector>

class Skybox : public Actor
{
public:
	Skybox(GameWorld* world, std::string texturePath[6]);
	virtual void update();
	~Skybox();


private:
	std::vector<glm::mat4> m_skyboxBoneT;
};


#endif
