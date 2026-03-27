#include "Application.h"

Application::Application() : window(nullptr), camera(nullptr), deltaTime(0.0f), lastFrame(0.0f) {}

Application::~Application()
{
    lightCube.reset();
    lightTargetCube.reset();
    lightShader.reset();
    lightTargetShader.reset();

    if (camera != nullptr)
    {
        delete camera;
        camera = nullptr;
    }

    if (window != nullptr)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

bool Application::init()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Create and set up camera
    camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    InputManager::setCamera(camera);

    // callbacks for window resize, mouse movement and scroll movement
    glfwSetFramebufferSizeCallback(window, InputManager::framebufferSizeCallback);
    glfwSetCursorPosCallback(window, InputManager::mouseCallback);
    glfwSetScrollCallback(window, InputManager::scrollCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create shaders
    lightShader = std::make_shared<Shader>("shaders/lightSource.vs", "shaders/lightSource.fs");
    lightTargetShader =
        std::make_shared<Shader>("shaders/lightTarget.vs", "shaders/lightTarget.fs");

    // Create scene objects
    glm::vec3 lightPos(1.0f, 1.0f, -5.0f);
    lightCube = std::make_shared<Object>(lightShader,
                                         cubeVertices,
                                         lightPos,
                                         Object::VertexLayout::PositionNormal);

    glm::vec3 lightTargetPos(3.0f, 2.0f, -7.0f);
    lightTargetCube = std::make_shared<Object>(lightTargetShader,
                                               cubeVertices,
                                               lightTargetPos,
                                               Object::VertexLayout::PositionNormal);

    return true;
}

void Application::run()
{
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        InputManager::processInput(window, deltaTime);

        // render
        renderFrame();

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::renderFrame()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                  0.1f, 100.0f);

    glm::mat4 view = glm::mat4(1.0f);
    view = camera->GetViewMatrix();

    // move light source
    lightCube->setPosition(lightCube->getPosition() + (float)sin(glfwGetTime()) / 100);

    // render objects
    lightCube->shader->use();
    lightCube->shader->setMat4("projection", projection);
    lightCube->shader->setMat4("view", view);
    glBindVertexArray(lightCube->VAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightCube->getPosition());
    lightCube->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 5);

    lightTargetCube->shader->use();
    lightTargetCube->shader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    lightTargetCube->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    lightTargetCube->shader->setVec3("viewPos", camera->Position);
    lightTargetCube->shader->setVec3("lightPos", lightCube->getPosition());
    lightTargetCube->shader->setMat4("projection", projection);
    lightTargetCube->shader->setMat4("view", view);
    glBindVertexArray(lightTargetCube->VAO);
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightTargetCube->getPosition());
    model = glm::rotate(model, glm::radians(45.0f) + (float)glfwGetTime(),
                        glm::vec3(1.0f, 5.0f, 45.0f));
    lightTargetCube->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 5);
}