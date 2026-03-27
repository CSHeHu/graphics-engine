#include "Scene.h"

#include <glad/glad.h>
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include "Camera.h"
#include "MeshData.h"
#include "Object.h"
#include "Shader.h"

Scene::Scene() : elapsedTime(0.0f) {}

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
    lightShader = std::make_shared<Shader>("shaders/lightSource.vs", "shaders/lightSource.fs");
    lightTargetShader = std::make_shared<Shader>("shaders/lightTarget.vs", "shaders/lightTarget.fs");

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

void Scene::update(float timeSeconds)
{
    elapsedTime = timeSeconds;
    lightCube->setPosition(lightCube->getPosition() + std::sin(timeSeconds) / 100.0f);
}

void Scene::render(const Camera &camera, const glm::mat4 &projection, const glm::mat4 &view)
{
    lightCube->shader->use();
    lightCube->shader->setMat4("projection", projection);
    lightCube->shader->setMat4("view", view);
    glBindVertexArray(lightCube->getVAO());
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightCube->getPosition());
    lightCube->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 6);

    lightTargetCube->shader->use();
    lightTargetCube->shader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    lightTargetCube->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    lightTargetCube->shader->setVec3("viewPos", camera.Position);
    lightTargetCube->shader->setVec3("lightPos", lightCube->getPosition());
    lightTargetCube->shader->setMat4("projection", projection);
    lightTargetCube->shader->setMat4("view", view);
    glBindVertexArray(lightTargetCube->getVAO());
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightTargetCube->getPosition());
    model = glm::rotate(model, glm::radians(45.0f) + elapsedTime,
                        glm::vec3(1.0f, 5.0f, 45.0f));
    lightTargetCube->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertices.size() / 6);

    ground->shader->use();
    ground->shader->setVec3("objectColor", 0.2f, 0.7f, 0.2f);
    ground->shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    ground->shader->setVec3("viewPos", camera.Position);
    ground->shader->setVec3("lightPos", lightCube->getPosition());
    ground->shader->setMat4("projection", projection);
    ground->shader->setMat4("view", view);
    glBindVertexArray(ground->getVAO());
    model = glm::mat4(1.0f);
    ground->shader->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
