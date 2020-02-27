#ifndef PLAYER_H
#define PLAYER_H

#include "Actor.h"


class Camera;
class CapsuleCollision;

class Player : public Actor
{
public:
	Player(glm::mat4 rootTransformation = glm::mat4(1.0));
	Camera* getFollowingCamera();
	void setFollowingCamera(glm::vec4 position, glm::vec4 forward);
	~Player();

private:
	Camera* m_cam;
	CapsuleCollision* m_capsule;
};

#endif