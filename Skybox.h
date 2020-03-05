#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
class Renderer;
struct CubeMap;

#include "Actor.h"

class Skybox : public Actor
{
public:
	Skybox(Renderer* renderer, std::string texturePath[6]);
	~Skybox();


private:
	CubeMap* m_cubeMap;
};


#endif
