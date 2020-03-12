#ifndef PRIMITIVE_ACTORS_H
#define PRIMITIVE_ACTORS_H

#include "Actor.h"
#include <string>


class Renderer;
struct Light;
struct ShaderPath;

class Plane : public Actor
{
public:
	Plane(GameWorld* world, ShaderPath shaderpath, std::string texturePath);

private:
	std::vector<glm::mat4> m_planeBoneT;
};



#endif
