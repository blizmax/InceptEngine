#ifndef GAME_WORLD_H
#define GAME_WORLD_H



#include <memory>
#include <chrono>
#include <vector>
#include "Math.h"

class Renderer;
class Actor;
class Player;
struct Light;
class Skybox;
class Terrain;
class Camera;
struct GLFWwindow;
class Sword;

struct MousePosition
{
	void syncPosition();

	double curXPos = 0;
	double curYPos = 0;
	double lastXPos = 0;
	double lastYPos = 0;
};


class GameWorld
{
public:
	GameWorld();

	~GameWorld();

	void play();

	void setWindowConfiguration();

	void showMouse();

	void hideMouse();

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void mouseCallback(GLFWwindow* window, int button, int action, int mods);

	bool m_enableCameraMove = false;

	GLFWwindow** m_window;

	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<Player> m_player;

	std::unique_ptr<Terrain> m_terrain;

	std::unique_ptr<Skybox> m_skybox;

	std::unique_ptr<Light> m_light;

	std::unique_ptr<Camera> m_camera;

	std::unique_ptr<Sword> m_sword;

	MousePosition m_mousePostion;

	std::chrono::time_point<std::chrono::steady_clock> m_currentTime;

	std::chrono::time_point<std::chrono::steady_clock> m_lastTime;

	float m_deltaTime;

	std::vector<glm::mat4> m_hornetBoneT;

	std::vector<glm::mat4> m_identityMeshBoneT;

	std::vector<glm::mat4> m_terrainMeshBoneT;

	bool m_motionControl = true;
private:
	void updateStates();
	void loadWorld();
};


#endif // !GAME_WORLD_H

