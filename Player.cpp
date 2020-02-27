#include "Player.h"
#include "Camera.h"

Player::Player(glm::mat4 rootTransformation)
	:Actor{ rootTransformation }
{
	m_cam = nullptr;
	m_capsule = nullptr;
}

Camera* Player::getFollowingCamera()
{
	return m_cam;
}

void Player::setFollowingCamera(glm::vec4 position, glm::vec4 forward)
{
	m_cam = new Camera(this, position, forward);
}

Player::~Player()
{
	if (m_cam != nullptr)
	{
		delete m_cam;
	}
}
