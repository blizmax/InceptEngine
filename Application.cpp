
#include "Animation.h"
#include "Renderer.h"
#include "WindowEventCallback.h"
#include "Player.h"
#include "Camera.h"
#include "PrimitiveActors.h"

#include <fstream>
#include <iostream>
#include "Global.h"
#include "Light.h"
#include "MarchingCube.h"

#include "FastNoise.h"



glm::mat4 computeBoneMatrix(glm::vec3 position, glm::vec3 pointDirection)
{
	auto helper = glm::normalize(glm::vec3(1.0f, 2.0f, 5.0f));
	auto X = -glm::vec4(glm::normalize(pointDirection), 0.0f);
	auto Y = glm::vec4(glm::cross(glm::vec3(X), helper), 0.0f);
	auto Z = glm::vec4(glm::cross(glm::vec3(X), glm::vec3(Y)), 0.0f);
	auto O = glm::vec4(position, 1.0f);
	return glm::mat4(X, Y, Z, O);

}

void IkSetBoneWorldTransformationToBoneT(glm::mat4 world, std::vector<glm::mat4>& boneT, Skeleton& skeleton, std::string boneName, bool propogateToChildren);
glm::mat4 getBoneWorldTransformationFromBoneT(const std::vector<glm::mat4>& boneT, const Skeleton& skeleton, std::string boneName);

void IkFAB(std::vector<glm::mat4>& boneT, Skeleton& skeleton, glm::mat4 swordLocation)
{
	glm::mat4 handTransform = swordLocation * FBX_Import_Mesh_Root_Transformation * glm::inverse(swordSocket);
	glm::vec3 p3 = handTransform[3];
	glm::vec3 p1 = getBoneWorldTransformationFromBoneT(boneT, skeleton, "upperarm_r")[3];
	glm::vec3 p2 = getBoneWorldTransformationFromBoneT(boneT, skeleton, "lowerarm_r")[3];
	float l1 = skeleton.m_bones.at("lowerarm_r").m_lengthToParent;
	float l2 = skeleton.m_bones.at("hand_r").m_lengthToParent;

	for (int i = 0; i < 10; i++)
	{
		p2 = p3 + l2 * glm::normalize(p2 - p3);
		p2 = p1 + l1 * glm::normalize(p2 - p1);
	}

	auto upperarmTransform = computeBoneMatrix(p1, p2 - p1);
	auto lowerarmTransform = computeBoneMatrix(p2, p3 - p2);
	handTransform[3] = glm::vec4(p2 + l2 * glm::normalize(p3 - p2), 1.0f);

	IkSetBoneWorldTransformationToBoneT(upperarmTransform, boneT, skeleton, "upperarm_r", false);
	IkSetBoneWorldTransformationToBoneT(lowerarmTransform, boneT, skeleton, "lowerarm_r", false);
	IkSetBoneWorldTransformationToBoneT(handTransform, boneT, skeleton, "hand_r", true);

}

glm::mat4 childBoneInParentCoord(const Skeleton& skeleton, std::string parent, std::string child)
{
	auto parentWorld = FBX_Import_Mesh_Root_Transformation * glm::inverse(skeleton.m_bones.at(parent).m_offset);
	auto childWorld = FBX_Import_Mesh_Root_Transformation * glm::inverse(skeleton.m_bones.at(child).m_offset);
	return glm::inverse(parentWorld) * childWorld;
}


void IkSetBoneWorldTransformationToBoneT(glm::mat4 world, std::vector<glm::mat4>& boneT, Skeleton& skeleton, std::string boneName, bool propogateToChildren)
{
	auto boneOffset = skeleton.m_bones.at(boneName).m_offset;
	auto transformation = world * boneOffset;
	boneT[skeleton.m_bones.at(boneName).m_boneId] = transformation;
	skeleton.m_bones.at(boneName).m_worldCoord = world;
	

	if (propogateToChildren)
	{
		std::stack<std::string> childStack;
		childStack.push(boneName);
		while (!childStack.empty())
		{
			std::string currentBone = childStack.top();
			childStack.pop();
			auto parentTransformation = skeleton.m_bones.at(currentBone).m_worldCoord;
			for (std::string childOfCurBone : skeleton.m_bones.at(currentBone).m_children)
			{
				auto childWorldCoordinate =  parentTransformation * childBoneInParentCoord(skeleton, currentBone, childOfCurBone);
				IkSetBoneWorldTransformationToBoneT(childWorldCoordinate, boneT, skeleton, childOfCurBone, false);
				childStack.push(childOfCurBone);
			}
		}
	}
}



glm::mat4 getBoneWorldTransformationFromBoneT(const std::vector<glm::mat4>& boneT, const Skeleton& skeleton, std::string boneName)
{
	return boneT[skeleton.m_bones.at(boneName).m_boneId] * skeleton.m_bones.at(boneName).m_bindPoseWorldTransform;
}

/*
int main()
{
	Renderer* renderer = new Renderer();


	Light light;
	light.m_locationAndIntensity = { 0.0f,500.0f,0.0f, 3.0f };

	std::string modelPath = "D:\\Inception\\Content\\Models\\HornetGL.FBX";
	std::string texturePath = "D:\\Inception\\Content\\Textures\\Hornet.HDR";

	std::string swordPath = "D:\\Inception\\Content\\Models\\ElementSword.obj";
	std::string swordTexturePath = "D:\\Inception\\Content\\Textures\\Firesword.HDR";


	std::string grassTexturePath = "D:\\Inception\\Content\\Textures\\T_Grass.BMP";

	std::string animationPath = "D:\\Inception\\Content\\Models\\HornetAttackGL.FBX";


	SkeletonMesh* hornetMesh = SkeletonMesh::loadSkeletonMesh(renderer, modelPath, texturePath, "root", false);

	
	SkeletonMesh* swordMesh = SkeletonMesh::loadSkeletonMesh(renderer, swordPath, swordTexturePath, "", false);

	Animation* hornetNormalAttackOne = loadAnimation(animationPath, hornetMesh, "root");


	Player* hornet = new Player(glm::mat4(1.0));


	hornet->setSkeletonMesh(hornetMesh);


	Actor* sword = new Actor(glm::mat4(1.0));
	sword->setSkeletonMesh(swordMesh);



	Actor* plane = new Plane(renderer, grassTexturePath, &light);

	Actor* hand = new Plane(renderer, grassTexturePath, &light);

	//std::vector<glm::mat4> planeTransformation = { glm::mat4(1.0f), glm::scale(glm::vec3(30,0,30)) };

	std::vector<glm::mat4> handTransformation = { glm::mat4(1.0f), glm::mat4(1.0) };

	std::vector<glm::mat4> tPose(70, glm::mat4(1.0));

	std::vector<glm::mat4> planeBoneT(70, glm::mat4(1.0));
	planeBoneT[1] = glm::scale(glm::vec3(3000, 1, 3000));
	
	tPose[1] = hornet->getActorTransformation();

	std::vector<glm::mat4> swordTBuffer = std::vector<glm::mat4>(2, glm::mat4(1.0));

	plane->getSkeletonMesh()->initializeTransformationBuffer(renderer, planeBoneT, &light);
	sword->getSkeletonMesh()->initializeTransformationBuffer(renderer, swordTBuffer, &light);

	renderer->spawnActor(plane);


	

	//renderer->spawnActor(hand);

	renderer->spawnActor(sword);

	renderer->spawnActor(hornet);
	

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
	
	
	float time = 0.0f;

	std::vector<glm::mat4> displayMesh(100, glm::mat4(1.0));

	Camera followingCam = Camera(hornet, glm::vec4(0, 119.745132f, 475.598633f, 1.0f), glm::vec4(0, 128.575134f, 76.504005f, 1.0f));

	bool enableCameraMove = false;
	Data data = { renderer, &followingCam, &enableCameraMove, hornet, sword, IkFAB };
	data.tPose = &tPose;
	data.sk = hornet->getSkeletonMesh()->getSkeleton();
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

	float deltaTime = 0.0f;

	auto startTime = std::chrono::high_resolution_clock::now();


	auto frameRateTestStartTime = std::chrono::high_resolution_clock::now();

	int nFrames = 150;


	
	sword->setActorTransformation(getBoneWorldTransformationFromBoneT(tPose, *hornet->getSkeletonMesh()->getSkeleton(), "hand_r") * swordSocket * FBX_Import_Mesh_Root_Transformation_Inverse);

	while (!glfwWindowShouldClose(*m_window))
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		if (time > 1.8f)
		{
			startTime = std::chrono::high_resolution_clock::now();
		}

		//auto boneT = getBonesTransformation(*hornet->getSkeletonMesh()->getSkeleton(), *hornetNormalAttackOne, time);

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

		
	//	boneT[1] = hornet->getActorTransformation();

		//hornet->getSkeletonMesh()->updateTransformationBuffer(renderer, boneT);
		//tPose[hornet->getSkeletonMesh()->getSkeleton()->m_bones.at("hand_r").m_boneId] = test(hornet->getSkeletonMesh()->getSkeleton()->m_bones.at("hand_r").m_offset);

		
		//IkSetBoneWorldTransformationToBoneT(rightHandTransform, tPose, *hornet->getSkeletonMesh()->getSkeleton(), "hand_r", true);
		//IkSetBoneWorldTransformationToBoneT(upperarmWorldTransform, tPose, *hornet->getSkeletonMesh()->getSkeleton(), "upperarm_r", false);

		//IkSetBoneWorldTransformationToBoneT(lowerarmTransformation, tPose, *hornet->getSkeletonMesh()->getSkeleton(), "lowerarm_r", true);
		swordTBuffer[1] = sword->getActorTransformation();

		data.location = swordTBuffer[1];

		sword->getSkeletonMesh()->updateUniformBuffer(renderer, swordTBuffer, &light);
		
		//IkFAB(tPose, *hornet->getSkeletonMesh()->getSkeleton(), swordTBuffer[1]);

		hornet->getSkeletonMesh()->updateUniformBuffer(renderer, tPose, &light);

		//plane->getSkeletonMesh()->updateUniformBuffer(renderer, planeBoneT, &light);
		//handTransformation[1] = glm::translate(glm::vec3(tPose[hornet->getSkeletonMesh()->getSkeleton()->m_bones.at("hand_r").m_boneId]  * hornet->getSkeletonMesh()->getSkeleton()->m_bones.at("hand_r").m_bindPoseWorldTransform * glm::vec4(0, 0, 0, 1))) * glm::scale(glm::vec3(50, 1, 50));

		//hand->getSkeletonMesh()->updateTransformationBuffer(renderer, handTransformation);

		renderer->drawFrame();

		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - currentTime).count();

		nFrames += 1;
	}
	
	auto testEndTime = std::chrono::high_resolution_clock::now();
	float testDuration = std::chrono::duration<float, std::chrono::seconds::period>(testEndTime - frameRateTestStartTime).count();


	std::cout << "Average Frame: " << nFrames / testDuration << std::endl;


	delete sword;

	delete hornet;
	
	delete hand;

	delete plane;

	delete renderer;
	
}*/

float testInOrOutSphere(glm::vec4 position)
{
	const float r = 10.0f;
	if (glm::length2(glm::vec3(position)) <= r * r)
	{
		return -1.0f;
	}
	else
	{
		return 1.0f;
	}
}


Actor* createSphere(Renderer* renderer, std::string texturePath)
{
	std::vector<Vertex> vertices = {};
	std::vector<uint32_t> indices = {};
	for (int x = -10; x <= 10; x++)
	{
		for (int y = -10; y <= 10; y++)
		{
			for (int z = -10; z <= 10; z++)
			{
				Cell cell;
				cell.m_position = { (float)x, (float)y, (float)z };
				auto vertexPostion = cell.getCellVertexPosition();

				for (int i = 0; i < vertexPostion.size(); i++)
				{
					cell.m_vertexWeights[i] = testInOrOutSphere(vertexPostion[i]);
				}


				marchingSingleCell(cell, vertices, indices);
			}
		}
	}


	Actor* sphere = new Actor(glm::mat4(1.0));
	SkeletonMesh* mesh = new SkeletonMesh(renderer, vertices, indices, texturePath, nullptr);
	sphere->setSkeletonMesh(mesh);
	return sphere;
}




Actor* createrRandomTerrain(Renderer* renderer, std::string texturePath)
{
	

	FastNoise perlinNoise;
	perlinNoise.SetNoiseType(FastNoise::NoiseType::PerlinFractal);
	perlinNoise.SetSeed(1400);
	perlinNoise.SetFractalOctaves(4);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	for (int y = 0; y <= 20; y++)
	{
		for (int z = 50; z >= -50; z--)
		{
			for (int x = -50; x <= 50; x++)
			{
				Cell cell;
				cell.m_position = { float(x), (float(y)), float(z) };
				auto vertexPositions = cell.getCellVertexPosition();
				for (int i = 0; i < 8; i++)
				{
					cell.m_vertexWeights[i] = perlinNoise.GetNoise(vertexPositions[i].x, vertexPositions[i].y*5.0f, vertexPositions[i].z) + vertexPositions[i].y*0.0005f;
				}
				marchingSingleCell(cell, vertices, indices);
			}
		}
	}

	SkeletonMesh* mesh = new SkeletonMesh(renderer, vertices, indices, texturePath, nullptr);

	Actor* terrain = new Actor(glm::mat4(1.0));
	terrain->setSkeletonMesh(mesh);
	return terrain;
}

int main()
{


	Renderer* renderer = new Renderer();

	Light light;
	light.m_locationAndIntensity = { 0.0f,2000.0f,0.0f, 1.0f };

	
	std::string grassTexturePath = "D:\\Inception\\Content\\Textures\\T_Grass.BMP";

	Actor* plane = new Plane(renderer, grassTexturePath, &light);

	std::vector<glm::mat4> planeBoneT(2, glm::mat4(1.0));
	planeBoneT[1] = glm::scale(glm::vec3(3000, 1, 3000));

	plane->getSkeletonMesh()->initializeUniformBuffer(renderer, planeBoneT, &light);

	renderer->spawnActor(plane);

	Actor* dullCam = new Actor(glm::mat4(1.0));

	std::string texturePath = "D:\\Inception\\Content\\Textures\\grey.jpg";
	Actor* terrain = createrRandomTerrain(renderer, texturePath);

	//Actor* sphere = createSphere(renderer, texturePath);
	
	std::vector<glm::mat4> terrainTBuffer = { glm::mat4(1.0), glm::scale(glm::vec3(5,1,5)) };

	//sphere->getSkeletonMesh()->initializeUniformBuffer(renderer, sphereTBuffer, &light);


	terrain->getSkeletonMesh()->initializeUniformBuffer(renderer, terrainTBuffer, &light);

	//renderer->spawnActor(sphere);

	renderer->spawnActor(terrain);

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


	float time = 0.0f;

	Camera followingCam = Camera(dullCam, glm::vec4(0, 50, 20, 1.0f), glm::vec4(0, 0, 0, 1.0f));

	bool enableCameraMove = false;
	Data data = { renderer, &followingCam, &enableCameraMove, dullCam, nullptr, nullptr };

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

	float deltaTime = 0.0f;

	auto startTime = std::chrono::high_resolution_clock::now();


	auto frameRateTestStartTime = std::chrono::high_resolution_clock::now();

	int nFrames = 150;


	assert(renderer->getNumActors() != 0);

	while (!glfwWindowShouldClose(*m_window))
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		if (time > 1.8f)
		{
			startTime = std::chrono::high_resolution_clock::now();
		}



		glfwGetCursorPos(*renderer->getWindow(), &xPos, &yPos);

		if (enableCameraMove)
		{
			followingCam.rotateHorizontal((float)((lastXPos - xPos) * 0.1));
			followingCam.rotateVertical((float)((lastYPos - yPos) * 0.05));
		}

		lastXPos = xPos;
		lastYPos = yPos;


		glfwPollEvents();

		renderer->m_mvp.view = followingCam.cameraMatrix();


		renderer->drawFrame();

		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - currentTime).count();

		nFrames += 1;
	}

	auto testEndTime = std::chrono::high_resolution_clock::now();
	float testDuration = std::chrono::duration<float, std::chrono::seconds::period>(testEndTime - frameRateTestStartTime).count();


	std::cout << "Average Frame: " << nFrames / testDuration << std::endl;

	//delete sphere;
	delete plane;

	delete terrain;


	delete renderer;
	
}




