
#include "Animation.h"
#include "Renderer.h"
#include "WindowEventCallback.h"
#include "Player.h"
#include "Camera.h"
#include "PrimitiveActors.h"

#include <fstream>
#include <iostream>


int main()
{
	Renderer* renderer = new Renderer();

	std::string modelPath = "D:\\Inception\\Content\\Models\\HornetGL.FBX";
	std::string texturePath = "D:\\Inception\\Content\\Textures\\T_Hornet.BMP";
	std::string grassTexturePath = "D:\\Inception\\Content\\Textures\\T_Grass.BMP";

	std::string animationPath = "D:\\Inception\\Content\\Models\\HornetAttackGL.FBX";


	SkeletonMesh* hornetMesh = SkeletonMesh::loadSkeletonMesh(renderer, modelPath, texturePath, "root");

	SkeletonMesh* hornet22Mesh = SkeletonMesh::loadSkeletonMesh(renderer, modelPath, texturePath, "root");



	Animation* hornetNormalAttackOne = loadAnimation(animationPath, hornetMesh, "root");

	
	auto hornetRootRotate =  glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Player* hornet = new Player(hornetRootRotate);
	Actor* hornet22 = new Actor(glm::translate(glm::vec3(150, 0, 0)) * hornetRootRotate);

	hornet->setSkeletonMesh(hornetMesh);
	hornet22->setSkeletonMesh(hornet22Mesh);

	Actor* plane = new Plane(renderer, grassTexturePath);

	std::vector<glm::mat4> planeTransformation = { glm::mat4(1.0f), glm::scale(glm::vec3(3000,0,3000)) };
	plane->getSkeletonMesh()->initializeTransformationBuffer(renderer, planeTransformation);


	renderer->spawnActor(plane);

	renderer->spawnActor(hornet);

	renderer->spawnActor(hornet22);

	

	auto m_window = renderer->getWindow();

	glfwSetInputMode(*m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	unsigned char pixels[16 * 16 * 4];
	memset(pixels, 0xff, sizeof(pixels));
	GLFWimage image;
	image.width = 16;
	image.height = 16;
	image.pixels = pixels;
	GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
	glfwSetCursor(*m_window, cursor);
	auto startTime = std::chrono::high_resolution_clock::now(); 
	
	float time = 0.0f;

	std::vector<glm::mat4> displayMesh(100, glm::mat4(1.0));

	Camera followingCam = Camera(hornet, glm::vec4(0, 119.745132f, 475.598633f, 1.0f), glm::vec4(0, 128.575134f, 76.504005f, 1.0f));

	bool enableCameraMove = false;
	Data data = { renderer, &followingCam, &enableCameraMove, hornet };

	glfwSetWindowUserPointer(*m_window, &data);
	glfwSetFramebufferSizeCallback(*renderer->getWindow(), framebufferResizeCallback);
	glfwSetKeyCallback(*renderer->getWindow(), key_callback);


	glfwSetInputMode(*m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	double xPos;
	double yPos;
	double lastXPos;
	double lastYPos;

	glfwGetCursorPos(*renderer->getWindow(), &xPos, &yPos);

	glfwGetCursorPos(*renderer->getWindow(), &lastXPos, &lastYPos);

	while (!glfwWindowShouldClose(*m_window))
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		if (time > 1.8f)
		{
			startTime = std::chrono::high_resolution_clock::now();
		}

		auto boneT = getBonesTransformation(*hornet->getSkeletonMesh()->getSkeleton(), *hornetNormalAttackOne, time);

		glfwGetCursorPos(*renderer->getWindow(), &xPos, &yPos);

		if (enableCameraMove)
		{
			followingCam.rotateHorizontal((float)((lastXPos- xPos)*0.1));
			followingCam.rotateVertical((float)((lastYPos - yPos)*0.05));
		}

		lastXPos = xPos;
		lastYPos = yPos;
	
		
		glfwPollEvents();

		renderer->m_mvp.view = followingCam.cameraMatrix();

		
		boneT[1] = hornet->getModelTransformation();

		hornet->getSkeletonMesh()->updateTransformationBuffer(renderer, boneT);

		boneT[1] = hornet22->getModelTransformation();

		hornet22->getSkeletonMesh()->updateTransformationBuffer(renderer, boneT);

		
		renderer->drawFrame();
	
	}
	

	delete hornet;
	
	delete hornet22;

	delete plane;

	delete renderer;
	
}




