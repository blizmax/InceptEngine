#ifndef PRIMITIVE_ACTORS_H
#define PRIMITIVE_ACTORS_H

#include "Actor.h"
#include <string>


class Renderer;
struct Light;

class Plane : public Actor
{
public:
	Plane(Renderer* renderer, std::string texturePath, Light* light);

private:
	
};

std::vector<glm::mat4> getIdentityTransformationBuffer();


#endif
