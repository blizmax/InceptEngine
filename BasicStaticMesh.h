#ifndef BASIC_STATIC_MESH_H
#define BASIC_STATIC_MESH_H


#include "StaticMesh.h"

class Cube : public StaticMesh
{
public:
	Cube(glm::mat4 initialTransform = glm::mat4(1.0));
};

class Plane : public StaticMesh
{
public:
	Plane(glm::mat4 initialTransform = glm::mat4(1.0));
};
#endif
