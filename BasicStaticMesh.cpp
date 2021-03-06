#include "BasicStaticMesh.h"

Cube::Cube(glm::mat4 initialTransform) : StaticMesh(initialTransform)
{
	m_vertices = {
	{{-200.0, 200.0, -100.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{-200.0, 200.0, 100.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{0.0, 200.0, 100.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{0.0, 200.0, -100.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{0.0, 0.0, -100.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{0.0, 0.0, 100.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{-200.0, 0.0, 100.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{-200.0, 0.0, -100.0, 1.0}, {0.0,0.0,0.0,1.0}}
	};



	m_indices = { 0,1,2,1,2,3,
				  2,3,5,3,4,5,
				  0,3,4,0,4,7,
				  1,2,5,5,1,6,
				  4,5,6,6,4,7,
				  0,7,6,6,0,1 };
}

Plane::Plane(glm::mat4 initialTransform) : StaticMesh(initialTransform)
{
	m_vertices = {
	{{150, 50, 0.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{50.0, 50, 0.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{50.0, -50, 0.0, 1.0}, {0.0,0.0,0.0,1.0}},
	{{150.0, -50, 0.0, 1.0}, {0.0,0.0,0.0,1.0}}
	};

	m_indices = { 0,1,2,2,3,0 };

}
