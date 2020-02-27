
#include "Animation.h"
#include "Renderer.h"
#include "WindowEventCallback.h"
#include "Player.h"
#include "Camera.h"
#include "BasicStaticMesh.h"

#include <fstream>
#include <iostream>

void skeletonMeshExport(const SkeletonMesh& sk)
{
	std::string vertexInfo;
	for (auto vertex : sk.m_vertices)
	{
		vertexInfo.append(glm::to_string(vertex.position));
		vertexInfo.append("@");
		vertexInfo.append(glm::to_string(vertex.boneWeights));
		vertexInfo.append("@");
		vertexInfo.append(glm::to_string(vertex.affectedBonesID));
		vertexInfo.append("\n");
	}

	std::string indicesInfo;
	for (auto index : sk.m_indices)
	{
		indicesInfo.append(std::to_string(index));
		indicesInfo.append("\n");
	}

	std::ofstream myfile;
	myfile.open("hornetVertices.txt");
	myfile << vertexInfo;
	myfile.close();


	myfile.open("hornetIndices.txt");
	myfile << indicesInfo;
	myfile.close();



	std::cout << "Export finish" << std::endl;
}

void animationExport(const Skeleton& sk, const Animation& anim)
{
	std::string animInfo = "";
	float step = anim.m_duration / 250;
	float time = 0;
	for (int i = 0; i < 245; i++)
	{
		animInfo.append(std::to_string(time));
		auto boneT = getBonesTransformation(sk, anim, time);


		for (auto transformation : boneT)
		{
			animInfo.append("@");
			animInfo.append(glm::to_string(transformation));
		}
		animInfo.append("\n");
		time += step;
	}

	std::ofstream myfile;
	myfile.open("hornetNormalAttackOne.txt");
	myfile << animInfo;
	myfile.close();

	std::cout << "Export finish" << std::endl;
}


int main()
{
	
	SkeletonMesh hornetMesh = loadSkeletonMesh("D:\\Inception\\Content\\Models\\HornetGL.FBX", "root");

	Animation hornetNormalAttackOne = loadAnimation("D:\\Inception\\Content\\Models\\HornetAttackGL.FBX", hornetMesh, "root");

	cleanAnimation(&hornetNormalAttackOne, "root");


	
	auto hornetRootRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Player hornet(hornetRootRotate);
	hornet.setSkeletonMesh(&hornetMesh);
	
	Cube mycube;
	Plane myplane;

	Renderer renderer;
	renderer.setVertices(hornet.getSkeletonMesh()->m_vertices, hornet.getSkeletonMesh()->m_indices);
	//renderer.addVertices(myplane.m_vertices, myplane.m_indices);

	renderer.m_mvp.model = hornet.getModelTransformation();

	//renderer.m_mvp.model = glm::mat4(1.0);

	renderer.init();


	auto m_window = renderer.getWindow();

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

	Camera followingCam = Camera(&hornet, glm::vec4(0, 119.745132f, 475.598633f, 1.0f), glm::vec4(0, 128.575134f, 76.504005f, 1.0f));

	bool enableCameraMove = false;
	Data data = { &renderer, &followingCam, &enableCameraMove, &hornet };

	glfwSetWindowUserPointer(*m_window, &data);
	glfwSetFramebufferSizeCallback(*renderer.getWindow(), framebufferResizeCallback);
	glfwSetKeyCallback(*renderer.getWindow(), key_callback);


	glfwSetInputMode(*m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	double xPos;
	double yPos;
	double lastXPos;
	double lastYPos;

	glfwGetCursorPos(*renderer.getWindow(), &xPos, &yPos);

	glfwGetCursorPos(*renderer.getWindow(), &lastXPos, &lastYPos);

	while (!glfwWindowShouldClose(*m_window))
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		if (time > 1.8f)
		{
			startTime = std::chrono::high_resolution_clock::now();
		}

		auto boneT = getBonesTransformation(hornet.getSkeletonMesh()->m_skeleton, hornetNormalAttackOne, time);

		glfwGetCursorPos(*renderer.getWindow(), &xPos, &yPos);

		if (enableCameraMove)
		{
			followingCam.rotateHorizontal((float)((lastXPos- xPos)*0.1));
			followingCam.rotateVertical((float)((lastYPos - yPos)*0.05));
		}

		lastXPos = xPos;
		lastYPos = yPos;
	
		
		glfwPollEvents();

		renderer.m_mvp.model = hornet.getModelTransformation();
		//renderer.m_mvp.model = hornet.m_localFrame.getLocalFrame();
		renderer.m_mvp.view = followingCam.cameraMatrix();
		//renderer.m_mvp.view = glm::lookAt(glm::vec3(0, 0, 500), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		

		renderer.updateUniformBuffer(boneT);

		renderer.drawFrame();
	
	}
	
	
}




