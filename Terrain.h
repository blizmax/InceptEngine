#ifndef TERRAIN_H
#define TERRAIN_H

#include "Actor.h"
class Renderer;

class Terrain : public Actor
{
public:
	Terrain(GameWorld* world, std::string_view heightMap);
	~Terrain();
	uint32_t getNumVertices();
	float getTerrainHeight(glm::vec4 worldPosition);

	virtual void update();
private:
	uint32_t m_numVerties;
	std::vector<float*> m_vertexHeights;
	glm::mat4 m_inverseTransformation;
	std::vector<glm::mat4> m_terrainBoneT;
};

#endif
