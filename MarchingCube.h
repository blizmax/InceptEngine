#ifndef MARCHING_CUBE_H
#define MARCHING_CUBE_H
#include "MarchingCubeTables.h"
#include <vector>
#include <string>
#include <array>
#include "Math.h"
#include "Vertex.h"
class SkeletonMesh;
class Renderer;

// a cell is a essentially a cube, with vertex and index convention is this website
//http://paulbourke.net/geometry/polygonise/
//It contains information for its eight vertices, each with a float number weights, which 
//decides it's position relative to the surface. By convention, negative represents under 
//the surface, and positive represents above the surface.
//a cell also has a position which determines the cell's world position
//Assume all cell has zero rotation, and at it's local coordinate, origin
//is at the center of the cube, X points rightward, Y points upward, and Z points outward, 
//the same as the world coordinate convention.
struct Cell
{
	std::array<glm::vec4, 8> getCellVertexPosition();
	std::array<float, 8> m_vertexWeights = { 1.0f,1.0f,1.0,1.0f,1.0f,1.0f,1.0f,1.0f};
	glm::vec3 m_position = { 0.0f,0.0f,0.0f };
};

//Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::string texturePath, Skeleton* skeleton

void marchingSingleCell(const Cell& cell, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);


SkeletonMesh* marchingSpace(const std::vector<Cell> cellsInSpace, Renderer* renderer, std::string texturePath);


#endif // !MARCHING_CUBE_H

