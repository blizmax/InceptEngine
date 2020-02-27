#ifndef STATIC_MESH_H
#define STATIC_MESH_H

#include "Math.h"
#include "Vertex.h"
#include <vector>


class StaticMesh
{
public:
	StaticMesh(glm::mat4 rootTransform = glm::mat4(1.0));

	std::vector<Vertex> m_vertices = {};

	std::vector<uint32_t> m_indices = {};

	glm::mat4 m_rootTransform;
};

#endif
