#include "GameWorld.h"
#include "Renderer.h"
#include "Hornet.h"
#include "Light.h"
#include "Camera.h"
#include "Terrain.h"
#include "Skybox.h"
#include "SkeletonMesh.h"
#include <functional>
#include "Sword.h"
#include "SkeletonMesh.h"
struct Timer
{
	Timer()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		auto endTime = std::chrono::high_resolution_clock::now();
		float duration = std::chrono::duration<float>(endTime - startTime).count();
		std::cout << "Operation takes " << duration << "seconds\n";
	}
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

GameWorld::GameWorld()
{
	loadWorld();

	m_lastTime = std::chrono::high_resolution_clock::now();

	m_currentTime = std::chrono::high_resolution_clock::now();

	m_deltaTime = std::chrono::duration<float>(m_lastTime - m_currentTime).count();

	m_renderer->spawnActor(m_skybox.get());

	m_renderer->spawnActor(m_terrain.get());

	m_renderer->spawnActor(m_sword.get());

	m_renderer->spawnActor(m_player.get());
	
}

GameWorld::~GameWorld()
{
	std::cout << "Average frame rate is " << 1 / m_deltaTime << std::endl;
	std::cout << "Good Bye\n";
}

void GameWorld::play()
{
	while (! glfwWindowShouldClose(*m_window))
	{
		m_lastTime = std::chrono::high_resolution_clock::now();

		glfwPollEvents();

		glfwGetCursorPos(*m_window, &m_mousePostion.curXPos, &m_mousePostion.curYPos);

		if (m_enableCameraMove)
		{
			m_camera->rotateHorizontal((float)((m_mousePostion.lastXPos - m_mousePostion.curXPos) * 0.1));
			m_camera->rotateVertical((float)((m_mousePostion.lastYPos - m_mousePostion.curYPos) * 0.05));
		}

		m_mousePostion.syncPosition();

		glfwPollEvents();

		m_renderer->m_mvp.view = m_camera->cameraMatrix();

		float currentTerrainHeight = m_terrain->getTerrainHeight(m_player->getActorLocation());

		m_player->setActorHeight(currentTerrainHeight);

		updateStates();

		m_renderer->drawFrame();

		m_currentTime = std::chrono::high_resolution_clock::now();
		m_deltaTime = std::chrono::duration<float>(m_currentTime - m_lastTime).count();
	}
}


void GameWorld::setWindowConfiguration()
{
	showMouse();

	unsigned char pixels[16 * 16 * 4];
	memset(pixels, 0xff, sizeof(pixels));
	GLFWimage image;
	image.width = 16;
	image.height = 16;
	image.pixels = pixels;
	GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
	glfwSetCursor(*m_window, cursor);

	glfwSetWindowUserPointer(*m_window, this);

	glfwSetKeyCallback(*m_window, keyCallback);

	glfwSetMouseButtonCallback(*m_window, mouseCallback);

}


void GameWorld::showMouse()
{
	glfwSetInputMode(*m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwGetCursorPos(*m_window, &m_mousePostion.curXPos, &m_mousePostion.curYPos);

	glfwGetCursorPos(*m_window, &m_mousePostion.lastXPos, &m_mousePostion.lastYPos);
}

void GameWorld::hideMouse()
{
	glfwSetInputMode(*m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwGetCursorPos(*m_window, &m_mousePostion.curXPos, &m_mousePostion.curYPos);

	glfwGetCursorPos(*m_window, &m_mousePostion.lastXPos, &m_mousePostion.lastYPos);
}

void GameWorld::mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	GameWorld* world = reinterpret_cast<GameWorld*>(glfwGetWindowUserPointer(window));
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		world->m_player->m_speed = 0;
		dynamic_cast<Hornet*>(world->m_player.get())->m_isRunning = false;
		playAnimation(world->m_player->getSkeletonMesh(), 2, &(dynamic_cast<Hornet*>(world->m_player.get())->m_boneT), false);
	}
}

void GameWorld::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	GameWorld* world = reinterpret_cast<GameWorld*>(glfwGetWindowUserPointer(window));
	bool motionControl = ! world->m_player->getSkeletonMesh()->m_motionControlledByAnim;
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
		auto camera = world->m_camera->getForwardVec();
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		if (glm::length(camera - person) > 0.01)
		{
			float angle = glm::dot(camera, person);
			auto axis = glm::cross(camera, person);
			world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
		}

    }
	if (motionControl && key == GLFW_KEY_W && action == GLFW_REPEAT)
	{
		auto camera = world->m_camera->getForwardVec();
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		if (glm::length(camera - person) > 0.01)
		{
			float angle = glm::dot(camera, person);
			auto axis = glm::cross(camera, person);

			world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
		}

		world->m_player->translate(glm::vec3(world->m_player->getForwardVector()), 20.0f);
		world->m_player->m_speed = 1;
	
	}

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
		auto camera = -world->m_camera->getForwardVec();
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		float angle = glm::dot(camera, person);
		if (glm::length(camera - person) > 0.01)
		{
			auto axis = glm::cross(camera, person);
			if (glm::length(axis) > 0.01)
			{
				world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
			}
			else
			{
				if (std::abs(angle) > 0.5f)
				{
					world->m_player->rotate(glm::vec4(0, 1, 0, 0), 180.0f);
				}
			}
			
		}
    }

	if (motionControl && key == GLFW_KEY_S && action == GLFW_REPEAT)
	{
		auto camera = -world->m_camera->getForwardVec();
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		if (glm::length(camera - person) > 0.01)
		{
			float angle = glm::dot(camera, person);
			auto axis = glm::cross(camera, person);
			world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
		}
		world->m_player->translate(glm::vec3(world->m_player->getForwardVector()), 20.0f);
		world->m_player->m_speed = 1;
	}

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
		auto camera = glm::cross(world->m_camera->getForwardVec(), glm::vec3(0,1,0));
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		if (glm::length(camera - person) > 0.01)
		{
			float angle = glm::dot(camera, person);
			auto axis = glm::cross(camera, person);
			world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
		}
    }

	if (motionControl && key == GLFW_KEY_D && action == GLFW_REPEAT)
	{
		auto camera = glm::cross(world->m_camera->getForwardVec(), glm::vec3(0, 1, 0));
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		if (glm::length(camera - person) > 0.01)
		{
			float angle = glm::dot(camera, person);
			auto axis = glm::cross(camera, person);
			world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
		}
		world->m_player->translate(glm::vec3(world->m_player->getForwardVector()), 20.0f);
		world->m_player->m_speed = 1;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		auto camera = -glm::cross(world->m_camera->getForwardVec(), glm::vec3(0, 1, 0));
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		if (glm::length(camera - person) > 0.01)
		{
			float angle = glm::dot(camera, person);
			auto axis = glm::cross(camera, person);
			world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
		}
	}

	if (motionControl && key == GLFW_KEY_A && action == GLFW_REPEAT)
	{
		auto camera = -glm::cross(world->m_camera->getForwardVec(), glm::vec3(0, 1, 0));
		auto person = glm::normalize(glm::vec3(world->m_player->getForwardVector()));
		if (glm::length(camera - person) > 0.01)
		{
			float angle = glm::dot(camera, person);
			auto axis = glm::cross(camera, person);
			world->m_player->rotate(glm::vec4(axis, 0), -glm::degrees(glm::acos(angle)));
		}
		world->m_player->translate(glm::vec3(world->m_player->getForwardVector()), 20.0f);
		world->m_player->m_speed = 1;
	}


	if ((key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D) && action == GLFW_RELEASE)
	{
		world->m_player->m_speed = 0;
	}

    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }


	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		if (world->m_enableCameraMove) world->showMouse();
		else world->hideMouse();

		world->m_mousePostion.syncPosition();
		world->m_enableCameraMove = !world->m_enableCameraMove;
	}
}

void GameWorld::updateStates()
{
	m_player->update();
	m_terrain->update();
	m_sword->update();
	m_skybox->update();
}

void GameWorld::loadWorld()
{
	Timer test;
	m_renderer = std::make_unique<Renderer>();

	m_window = m_renderer->getWindow();

	setWindowConfiguration();

	m_light = std::make_unique<Light>();
	m_light->m_locationAndIntensity = { 0.0f,2000.0f,0.0f, 1.5f };

	
	std::thread loadHornet([this]()
	{
		m_player = std::make_unique<Hornet>(this);
		m_camera = std::make_unique<Camera>(m_player.get(), glm::vec4(0, 119.745132f, 475.598633f, 1.0f), glm::vec4(0, 128.575134f, 76.504005f, 1.0f));
		m_renderer->m_mvp.view = m_camera->cameraMatrix();
		m_sword = std::make_unique<Sword>(this);
		playAnimation(m_player->getSkeletonMesh(), 0, &(dynamic_cast<Hornet*>(m_player.get())->m_boneT), true);
	});

	std::thread loadTerrain([this]()
	{
		m_terrain = std::make_unique<Terrain>(this, "D:\\Inception\\Content\\heightmaps\\HeightMap.png");
	});


	std::thread loadSkybox([this]()
	{
		std::string skyboxTexturePath[6] =
		{
			"D:\\Inception\\Content\\Textures\\Skybox\\front.png",
			"D:\\Inception\\Content\\Textures\\Skybox\\back.png",
			"D:\\Inception\\Content\\Textures\\Skybox\\top.png",
			"D:\\Inception\\Content\\Textures\\Skybox\\bottom.png",
			"D:\\Inception\\Content\\Textures\\Skybox\\right.png",
			"D:\\Inception\\Content\\Textures\\Skybox\\left.png"
		};

		m_skybox = std::make_unique<Skybox>(this, skyboxTexturePath);
	});



	loadSkybox.join();
	loadTerrain.join();
	loadHornet.join();
}

void MousePosition::syncPosition()
{
	lastXPos = curXPos;
	lastYPos = curYPos;
}
