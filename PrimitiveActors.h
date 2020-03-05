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
	Plane(Renderer* renderer, ShaderPath shaderpath, std::string texturePath, Light* light);

private:
	
};

std::vector<glm::mat4> getIdentityTransformationBuffer();


#endif
