#ifndef CAMERA_H
#define CAMERA_H

#include "Math.h"
#include "ActorComponent.h"


class Actor;

class Camera : public ActorComponent
{
public:
	Camera(Actor* owner, glm::vec4 position, glm::vec4 forward);
	glm::mat4 cameraMatrix();
	
private:
	glm::vec4 m_position;
	glm::vec4 m_forward;
	glm::vec3 m_upward;

};

#endif
