#ifndef HORNET_H
#define HORNET_H

#include "Player.h"
#include <vector>

class GameWorld;
class Hornet : public Player
{
public:
	Hornet(GameWorld* world);

	~Hornet();

	virtual void update();

	std::vector<glm::mat4> m_boneT;

	bool m_isRunning = false;

private:

};

#endif // !HORNET_H

