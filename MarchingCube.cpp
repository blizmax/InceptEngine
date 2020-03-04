#include "MarchingCube.h"
#include "SkeletonMesh.h"
#include <random>

float randFloat(float a, float b)
{
	return ((b - a) * ((float)std::rand() / RAND_MAX)) + a;
}

void marchingSingleCell(const Cell& cell, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	int triIndex = 0;
	if (cell.m_vertexWeights[0] < 0) triIndex |= 1;
	if (cell.m_vertexWeights[1] < 0) triIndex |= 2;
	if (cell.m_vertexWeights[2] < 0) triIndex |= 4;
	if (cell.m_vertexWeights[3] < 0) triIndex |= 8;
	if (cell.m_vertexWeights[4] < 0) triIndex |= 16;
	if (cell.m_vertexWeights[5] < 0) triIndex |= 32;
	if (cell.m_vertexWeights[6] < 0) triIndex |= 64;
	if (cell.m_vertexWeights[7] < 0) triIndex |= 128;


	glm::mat4 cellScale = glm::scale(glm::vec3(1, 1, 1));
	glm::mat4 cellTranslate = glm::translate(cell.m_position);
	const int* triList = triTable[triIndex];
	for (; *triList != -1; triList = triList + 3)
	{
		
		float randX = randFloat(0.0, 1.0);
		float randY = randFloat(0.0, 1.0);
		Vertex v1;
		float weightOfSmallVertex = abs(cell.m_vertexWeights[edgeToVertexTable[*triList][0]]) / (abs(cell.m_vertexWeights[edgeToVertexTable[*triList][0]]) + abs(cell.m_vertexWeights[edgeToVertexTable[*triList][1]]));
		v1.position = cellTranslate * cellScale *(weightOfSmallVertex * vertexPositionTable[edgeToVertexTable[*triList][0]] + (1-weightOfSmallVertex) * vertexPositionTable[edgeToVertexTable[*triList][1]]);
		
		v1.texCoord = { randFloat(randX - 0.1f, randX + 0.1f), randFloat(randY - 0.1f, randY + 0.1f), 0.0f };


		Vertex v2;
		weightOfSmallVertex = abs(cell.m_vertexWeights[edgeToVertexTable[*(triList+1)][0]]) / (abs(cell.m_vertexWeights[edgeToVertexTable[*(triList+1)][0]]) + abs(cell.m_vertexWeights[edgeToVertexTable[*(triList+1)][1]]));
		v2.position = cellTranslate * cellScale *(weightOfSmallVertex * vertexPositionTable[edgeToVertexTable[*(triList + 1)][0]] + (1-weightOfSmallVertex) * vertexPositionTable[edgeToVertexTable[*(triList + 1)][1]]);
		v2.texCoord = { randFloat(randX - 0.1f, randX + 0.1f), randFloat(randY - 0.1f, randY + 0.1f), 0.0f };


		Vertex v3;
		weightOfSmallVertex = abs(cell.m_vertexWeights[edgeToVertexTable[*(triList + 2)][0]]) / (abs(cell.m_vertexWeights[edgeToVertexTable[*(triList + 2)][0]]) + abs(cell.m_vertexWeights[edgeToVertexTable[*(triList + 2)][1]]));
		v3.position = cellTranslate * cellScale *(weightOfSmallVertex * vertexPositionTable[edgeToVertexTable[*(triList + 2)][0]] + (1-weightOfSmallVertex) * vertexPositionTable[edgeToVertexTable[*(triList + 2)][1]]);
		v3.texCoord = { randFloat(randX - 0.1f, randX + 0.1f), randFloat(randY - 0.1f, randY + 0.1f), 0.0f };


		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		indices.push_back(static_cast<uint32_t>(indices.size()));
		indices.push_back(static_cast<uint32_t>(indices.size()));
		indices.push_back(static_cast<uint32_t>(indices.size()));
	}

}

SkeletonMesh* marchingSpace(const std::vector<Cell> cellsInSpace, Renderer* renderer, std::string texturePath)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	for (const auto& cell : cellsInSpace)
	{
		marchingSingleCell(cell, vertices, indices);
	}

	return new SkeletonMesh(renderer, vertices, indices, texturePath, nullptr);
}

std::array<glm::vec4, 8> Cell::getCellVertexPosition()
{
	glm::mat4 transformation = glm::translate(m_position);
	std::array<glm::vec4, 8> positions = {};
	for (int i = 0; i < 8; i++)
	{
		positions[i] = transformation * vertexPositionTable[i];
	}
	return positions;
}
