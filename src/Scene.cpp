#include "Scene.h"

#include <glad/glad.h>
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "AssetManager.h"
#include "Camera.h"
#include "Object.h"
#include "Shader.h"

Scene::Scene(AssetManager &assetManager)
    : assets(assetManager), elapsedTime(0.0f), cubeVertexCount(0), groundVertexCount(0)
{
}

Scene::~Scene()
{
    ground.reset();
    lightCube.reset();
    lightTargetCube.reset();
    lightShader.reset();
    lightTargetShader.reset();
}

bool Scene::init()
{
    const std::vector<float> &cubeVertices = assets.getMeshVertices("cube.obj");
    const std::vector<float> &groundVertices = assets.getMeshVertices("ground.obj");

    cubeVertexCount = assets.getMeshVertexCount("cube.obj");
    groundVertexCount = assets.getMeshVertexCount("ground.obj");

    lightShader = assets.getShader("assets/shaders/lightSource.vs", "assets/shaders/lightSource.fs");
    lightTargetShader = assets.getShader("assets/shaders/lightTarget.vs", "assets/shaders/lightTarget.fs");

    const glm::vec3 lightPos(1.0f, 1.0f, -5.0f);
    lightCube = std::make_shared<Object>(
        lightShader,
        cubeVertices,
        lightPos,
        Object::VertexLayout::PositionNormal);

    const glm::vec3 lightTargetPos(3.0f, 2.0f, -7.0f);
    lightTargetCube = std::make_shared<Object>(
        lightTargetShader,
        cubeVertices,
        lightTargetPos,
        Object::VertexLayout::PositionNormal);

    ground = std::make_shared<Object>(
        lightTargetShader,
        groundVertices,
        glm::vec3(0.0f, 0.0f, 0.0f),
        Object::VertexLayout::PositionNormal);

    return true;
}

void Scene::update(float deltaTime)
{
    elapsedTime += deltaTime;
    lightCube->setPosition(lightCube->getPosition() + std::sin(elapsedTime) / 100.0f);
    lightTargetCube->rotate(glm::radians(20.0f) * deltaTime, glm::vec3(1.0f, 0.0f, 1.0f));
}

void Scene::render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view)
{
    lightCube->shader->use();
    lightCube->shader->setMat4("projection", projection);
    lightCube->shader->setMat4("view", view);
    glBindVertexArray(lightCube->getVAO());
    glm::mat4 model = lightCube->getModelMatrix();
    lightCube->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cubeVertexCount));

    lightTargetCube->shader->use();
    lightTargetCube->shader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    lightTargetCube->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    lightTargetCube->shader->setVec3("viewPos", camera.Position);
    lightTargetCube->shader->setVec3("lightPos", lightCube->getPosition());
    lightTargetCube->shader->setMat4("projection", projection);
    lightTargetCube->shader->setMat4("view", view);
    glBindVertexArray(lightTargetCube->getVAO());
    model = lightTargetCube->getModelMatrix();
    lightTargetCube->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cubeVertexCount));

    ground->shader->use();
    ground->shader->setVec3("objectColor", 0.2f, 0.7f, 0.2f);
    ground->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    ground->shader->setVec3("viewPos", camera.Position);
    ground->shader->setVec3("lightPos", lightCube->getPosition());
    ground->shader->setMat4("projection", projection);
    ground->shader->setMat4("view", view);
    glBindVertexArray(ground->getVAO());
    model = ground->getModelMatrix();
    ground->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(groundVertexCount));
}
