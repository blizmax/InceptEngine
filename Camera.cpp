#include "Camera.h"
#include "Actor.h"

Camera::Camera(Actor* owner, glm::vec4 position, glm::vec4 forward)
{
	m_owner = owner;
	m_position = position;
	m_forward = forward;
	m_upward = { 0,1,0 };
}

glm::mat4 Camera::cameraMatrix()
{
	auto ownerFrame = m_owner->m_localFrame.getLocalFrame();
	auto worldPosition = ownerFrame * m_position;
	auto worldForward = ownerFrame * m_forward;
	return glm::lookAt(glm::vec3(worldPosition.x, worldPosition.y, worldPosition.z), glm::vec3(worldForward.x, worldForward.y, worldForward.z), m_upward);
}
