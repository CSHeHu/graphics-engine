#include "Object.h"

#include <gtc/matrix_transform.hpp>

Object::Object(const glm::vec3& position, const glm::vec3& scale)
    : pos(position), size(scale), rotationAngle(0.0f),
      rotationAxis(0.0f, 1.0f, 0.0f)
{
}

const glm::vec3& Object::getPosition() const
{
    return pos;
}

void Object::setPosition(const glm::vec3& position)
{
    pos = position;
}

void Object::setScale(const glm::vec3& scale)
{
    size = scale;
}

float Object::getRotationAngle() const
{
    return rotationAngle;
}

const glm::vec3& Object::getRotationAxis() const
{
    return rotationAxis;
}

void Object::setRotation(float angleRadians, const glm::vec3& axis)
{
    rotationAngle = angleRadians;
    if (glm::length(axis) > 0.0f)
    {
        rotationAxis = glm::normalize(axis);
    }
}

void Object::rotate(float deltaAngleRadians, const glm::vec3& axis)
{
    rotationAngle += deltaAngleRadians;
    if (glm::length(axis) > 0.0f)
    {
        rotationAxis = glm::normalize(axis);
    }
}

const glm::mat4 Object::getModelMatrix() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model           = glm::translate(model, pos);
    model           = glm::rotate(model, rotationAngle, rotationAxis);
    model           = glm::scale(model, size);
    return model;
}
