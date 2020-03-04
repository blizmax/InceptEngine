#include "Camera.h"
#include "Actor.h"

#include <iostream>


Camera::Camera(Actor* owner, glm::vec4 position, glm::vec4 forward)
{
	m_owner = owner;
	m_position = position;
	m_forwardPoint = forward;
}

glm::mat4 Camera::cameraMatrix()
{
	auto ownerFrame = m_owner->getActorTransformation();



	auto worldPosition = ownerFrame * m_position;
	auto worldForward = ownerFrame * m_forwardPoint;


	return glm::lookAt(glm::vec3(worldPosition.x, worldPosition.y, worldPosition.z), glm::vec3(worldForward.x, worldForward.y, worldForward.z), glm::vec3(0,1,0));
}

void Camera::rotateHorizontal(float degree)
{
	m_position = glm::rotate(m_position, glm::radians(degree), glm::vec3(0,1,0));
	m_forwardPoint = glm::rotate(m_forwardPoint, glm::radians(degree), glm::vec3(0, 1, 0));
}

void Camera::rotateVertical(float degree)
{
	auto lookAtDirection = m_forwardPoint - m_position;
	auto rotateionAxis = glm::cross(glm::vec3(lookAtDirection.x, lookAtDirection.y, lookAtDirection.z), glm::vec3(0, 1, 0));
	m_position = glm::rotate(m_position, glm::radians(degree), rotateionAxis);
	m_forwardPoint = glm::rotate(m_forwardPoint, glm::radians(degree), rotateionAxis);
}

void Camera::printCameraPramameter()
{
	std::cout << "position: " << glm::to_string(m_position) << std::endl;
	std::cout << "position: " << glm::to_string(m_forwardPoint) << std::endl;
}

void Camera::lightUp(float amount)
{
	m_position.y += amount;
	m_forwardPoint.y += amount;
}
