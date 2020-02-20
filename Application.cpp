
#include "Animation.h"
#include "Renderer.h"
#include "WindowEventCallback.h"
#include "Player.h"


int main()
{
	
	SkeletonMesh hornetMesh = loadSkeletonMesh("D:\\Inception\\Content\\Models\\HornetGL.FBX", "root");


	Animation hornetNormalAttackOne = loadAnimation("D:\\Inception\\Content\\Models\\HornetAttackGL.FBX", hornetMesh, "root");

	cleanAnimation(&hornetNormalAttackOne, "root");

	Player hornet;
	hornet.setSkeletonMesh(&hornetMesh);


	Renderer renderer;
	renderer.setVertices(hornet.getSkeletonMesh()->m_vertices, hornet.getSkeletonMesh()->m_indices);
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

	glfwSetWindowUserPointer(*m_window, &renderer);
	glfwSetFramebufferSizeCallback(*renderer.getWindow(), framebufferResizeCallback);
	glfwSetKeyCallback(*renderer.getWindow(), key_callback);

	auto startTime = std::chrono::high_resolution_clock::now(); 
	
	float time = 0.0f;

	std::vector<glm::mat4> displayMesh(100, glm::mat4(1.0));

	while (!glfwWindowShouldClose(*m_window))
	{
		auto currentTime = std::chrono::high_resolution_clock::now();
		time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		if (time > 1.8f)
		{
			startTime = std::chrono::high_resolution_clock::now();
		}

		auto boneT = getBonesTransformation(hornet.getSkeletonMesh()->m_skeleton, hornetNormalAttackOne, time);

		glfwPollEvents();

		renderer.drawTriangle(boneT);
	
	}
	
	
}




