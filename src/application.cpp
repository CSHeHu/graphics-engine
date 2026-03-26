#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H  
#include <map>

#include "Shader.h"
#include "Camera.h"
#include "InputManager.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "InputManager.h"
#include "MeshData.h"
#include "TextureManager.h"
#include "Object.h"
#include "TextManager.h"

void renderLoop(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
const float COLLISION_BUFFER = 1.2f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// callbacks for window resize, mouse movement and scroll movement
	InputManager::setCamera(&camera);
	glfwSetFramebufferSizeCallback(window, InputManager::framebufferSizeCallback);
	glfwSetCursorPosCallback(window, InputManager::mouseCallback);
	glfwSetScrollCallback(window, InputManager::scrollCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}



	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderLoop(window);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

void renderLoop(GLFWwindow* window){
	
	// create shaders for view and projection
	Shader projectionShader("shaders/projection.vs", "shaders/projection.fs");
	Shader viewShader("shaders/view.vs", "shaders/view.fs");
	Shader lightShader("shaders/lightSource.vs", "shaders/lightSource.fs");
	Shader lightTargetShader("shaders/lightTarget.vs", "shaders/lightTarget.fs");
	
	//light cube
	glm::vec3 lightPos(1.0f, 1.0f, -5.0f);
	Object lightCube("shaders/lightSource.vs", "shaders/lightSource.fs", cubeVertices, lightPos);
	glm::vec3 lightTargetPos(3.0f, 2.0f, -7.0f);
	Object lightTargetCube("shaders/lightTarget.vs", "shaders/lightTarget.fs", cubeVertices, lightTargetPos);
	
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{

		// per-frame time logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		InputManager::processInput(window, deltaTime);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		// Set up projection matrix
		projectionShader.use();
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		projectionShader.setMat4("projection", projection);

		// Set up view matrix
		viewShader.use();
		glm::mat4 view = glm::mat4(1.0f);
		view = camera.GetViewMatrix();
		viewShader.setMat4("view", view);


		//move light source
		lightCube.setPosition(lightCube.getPosition() + (float)sin(glfwGetTime())/100);

		// renred objects
		lightCube.shader->use();
		lightCube.shader->setMat4("projection", projection);
		lightCube.shader->setMat4("view", view);
		glBindVertexArray(lightCube.VAO);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, lightCube.getPosition());
		lightCube.shader->setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 5);

		lightTargetCube.shader->use();
		lightTargetCube.shader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightTargetCube.shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightTargetCube.shader->setVec3("viewPos", camera.Position);
		lightTargetCube.shader->setVec3("lightPos", lightCube.getPosition());
		lightTargetCube.shader->setMat4("projection", projection);
		lightTargetCube.shader->setMat4("view", view);
		glBindVertexArray(lightTargetCube.VAO);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightTargetCube.getPosition());
		model = glm::rotate(model, glm::radians(45.0f) + (float)glfwGetTime(), glm::vec3(1.0f, 5.0f, 45.0f));
		lightTargetCube.shader->setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 5);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}