#include "Terrain.h"
#include "SkeletonMesh.h"
#include "Renderer.h"
#include "Global.h"
#include "stb_image.h"


float getPixelHeight(stbi_uc* image, int numOfPixelPerRow, int z, int x)
{

	int zz = z + 5;
	int xx = x + 5;

	if (z >= numOfPixelPerRow-15)
	{
		zz = zz - 15;
	}
	if (x >= numOfPixelPerRow-15)
	{
		xx = xx - 15;
	}

	stbi_uc* index = image + 4 * (xx * numOfPixelPerRow + zz);

	int r = index[0];

	return (r / 255.0f) * terrainMaxHeight;
}

float getHeight(stbi_uc* image, int numOfPixelPerRow, int z, int x)
{

	if (z <= 1) z += 1;
	if (x <= 1) x += 1;

	if (z >= numOfPixelPerRow) z -= 1;

	if (x >= numOfPixelPerRow) x -= 1;

	float height = getPixelHeight(image, numOfPixelPerRow, z, x + 1) + getPixelHeight(image, numOfPixelPerRow, z, x - 1) + getPixelHeight(image, numOfPixelPerRow, z + 1, x) + getPixelHeight(image, numOfPixelPerRow, z - 1, x);

	float r = height / 4;
	assert(r >= 0);
	return r;
}

/*
Terrain::Terrain(Renderer* renderer, std::string_view heightMap)
	:Actor(glm::mat4(1.0))
{
	int height, width, nChannel;
	stbi_uc* image = stbi_load(heightMap.data(), &height, &width, &nChannel, 4);
	assert(height == width);

	int nQuad = height;
	m_numVerties = (2 * (nQuad-2) + 1) * (nQuad-2) + 1;
	std::vector<Vertex> vertices;
	vertices.reserve(m_numVerties);
	float terrainLength = static_cast<float>(nQuad) * 0.5f * lengthOfTerrainQuad;
	glm::vec4 previousPos = { -terrainLength+lengthOfTerrainQuad, 0.0f, -terrainLength+lengthOfTerrainQuad, 1.0 };
	float texCoordStep = 0.01f;

	int direction = 1;
	int z = 0;
	int x = 0;
	int xCoord = 1;
	for (z = 1; z < nQuad-1; z++)
	{
		for (x = 1; x < nQuad-1; x++)
		{
			Vertex v1;
			v1.position = previousPos;
			v1.position[1] = getHeight(image, nQuad - 2, xCoord, z);
			v1.texCoord = { xCoord * texCoordStep, z * texCoordStep, 0 };
			Vertex v2;
			v2.position = previousPos + glm::vec4(0.0f, 0.0f, lengthOfTerrainQuad, 0.0);
			v2.position[1] = getHeight(image, nQuad - 2, xCoord, z+1);
			v2.texCoord = { xCoord * texCoordStep, (z+1) * texCoordStep, 0 };
			vertices.push_back(v1);
			vertices.push_back(v2);
			previousPos = previousPos + static_cast<float>(direction) * glm::vec4(lengthOfTerrainQuad, 0, 0, 0);
			xCoord += direction;
		}

		Vertex lastInRow;
		lastInRow.position = previousPos;
		lastInRow.position.y = getHeight(image, nQuad - 2, xCoord, z);
		lastInRow.texCoord = { xCoord % 2, 1, 0 };

		vertices.push_back(lastInRow);
		previousPos = previousPos + glm::vec4(0, 0, lengthOfTerrainQuad, 0);
		direction *= -1;
	}

	Vertex last;
	last.position = previousPos;
	last.position.y = getHeight(image, nQuad - 2, xCoord, z);
	last.texCoord = { xCoord % 2, 0, 0 };
	vertices.push_back(last);

	assert(static_cast<uint32_t>(vertices.size()) == m_numVerties);
	
	std::vector<uint32_t> indices = { 1,2,3 };
	ShaderPath shaderpath = { "D:\\Inception\\Content\\Shaders\\spv\\vertex.spv","D:\\Inception\\Content\\Shaders\\spv\\fragment.spv" };
	std::string texturepath = "D:\\Inception\\Content\\Textures\\T_Grass.BMP";
	SkeletonMesh* mesh = new SkeletonMesh(renderer, vertices, indices, shaderpath, texturepath, nullptr, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

	setSkeletonMesh(mesh);

	stbi_image_free(image);
}
*/




Terrain::Terrain(Renderer* renderer, std::string_view heightMap)
	:Actor(glm::mat4(1.0))
{
	int height, width, nChannel;
	stbi_uc* image = stbi_load(heightMap.data(), &height, &width, &nChannel, 4);
	assert(height == width);


	int length = height + 1;
	m_numVerties = length * length;
	m_vertexHeights.resize(length);
	for (int i = 0; i < length; i++)
	{
		m_vertexHeights[i] = new float[length];
	}

	int numIndices = height * width * 6;

	std::vector<Vertex> vertices;
	vertices.reserve(m_numVerties);
	std::vector<uint32_t> indices;
	indices.reserve(numIndices);


	float terrainLength = static_cast<float>(length) * lengthOfTerrainQuad;

	float texCoordStep = 0.01f;
	glm::mat4 terrainTranslate = glm::translate(glm::vec3(-terrainLength * 0.5f, 0, -terrainLength * 0.5f));
	glm::mat4 terrainScale = glm::scale(glm::vec3(20, 1, 20));
	setActorTransformation(terrainScale * terrainTranslate);
	m_inverseTransformation = glm::inverse(getActorTransformation());

	for (int x = 0; x <= height; x++)
	{
		for (int z = 0; z <= width; z++)
		{
			Vertex v;
			float vertexHeight = getPixelHeight(image, width, z, x);
			v.position = glm::vec4(x * lengthOfTerrainQuad, vertexHeight, z * lengthOfTerrainQuad, 1.0);
			v.texCoord = { x * texCoordStep, z * texCoordStep, 0 };
			vertices.push_back(v);
			m_vertexHeights[x][z] = vertexHeight;
		}
	}

	for (int x = 0; x < height; x++)
	{
		for (int z = 0; z < width; z++)
		{
			int corner = x * length + z;
			indices.push_back(corner);
			indices.push_back(corner + 1);
			indices.push_back(corner + 1 + length);
			indices.push_back(corner + 1 + length);
			indices.push_back(corner + length);
			indices.push_back(corner);
		}
	}


	assert(static_cast<uint32_t>(vertices.size()) == m_numVerties);
	assert(indices.size() == numIndices);

	ShaderPath shaderpath = { "D:\\Inception\\Content\\Shaders\\spv\\vertex.spv","D:\\Inception\\Content\\Shaders\\spv\\fragment.spv" };
	std::string texturepath = "D:\\Inception\\Content\\Textures\\T_Grass.BMP";
	SkeletonMesh* mesh = new SkeletonMesh(renderer, vertices, indices, shaderpath, texturepath, nullptr, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	setSkeletonMesh(mesh);

	stbi_image_free(image);
}





Terrain::~Terrain()
{
	for (int i = 0; i < m_vertexHeights.size(); i++) delete[] m_vertexHeights[i];
}

uint32_t Terrain::getNumVertices()
{
	return m_numVerties;
}

float Terrain::getTerrainHeight(glm::vec4 worldPosition)
{
	glm::vec4 positionInLocal = m_inverseTransformation * glm::vec4(worldPosition.x, 0.0f, worldPosition.z, 1.0f);
	int X = static_cast<int>(positionInLocal.x / lengthOfTerrainQuad);
	int Z = static_cast<int>(positionInLocal.z / lengthOfTerrainQuad);
	return m_vertexHeights[X][Z];

}
