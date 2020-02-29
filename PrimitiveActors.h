#ifndef PRIMITIVE_ACTORS_H
#define PRIMITIVE_ACTORS_H

#include "Actor.h"
#include <string>


class Renderer;

class Plane : public Actor
{
public:
	Plane(Renderer* renderer, std::string texturePath);

private:
	
};

std::vector<glm::mat4> getIdentityTransformationBuffer();


#endif
