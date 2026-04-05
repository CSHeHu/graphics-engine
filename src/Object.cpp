#include "Object.h"

#include <gtc/matrix_transform.hpp>

Object::Object(std::shared_ptr<Shader> shaderProgram,
			   const std::vector<float> &vertices,
			   const glm::vec3 &position,
			   const glm::vec3 &scale,
			   VertexLayout layout,
			   const std::vector<std::string> &texturePaths)
	: shader(std::move(shaderProgram)), pos(position), size(scale), rotationAngle(0.0f), rotationAxis(0.0f, 1.0f, 0.0f)
{
	// Generate vertex array object and buffers
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Bind vertex array object
	glBindVertexArray(VAO);

	// Bind and set vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	if (layout == VertexLayout::PositionUV)
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	else
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);
		// normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	// Note: texture support removed; texturePaths parameter kept for API compatibility
}

Object::~Object()
{
	// Clean up resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

glm::vec3 Object::getPosition() const
{
	return pos;
}

void Object::setPosition(glm::vec3 position)
{
	pos = position;
}

glm::vec3 Object::getScale() const
{
	return size;
}

void Object::setScale(const glm::vec3 &scale)
{
	size = scale;
}

float Object::getRotationAngle() const
{
	return rotationAngle;
}

glm::vec3 Object::getRotationAxis() const
{
	return rotationAxis;
}

void Object::setRotation(float angleRadians, const glm::vec3 &axis)
{
	rotationAngle = angleRadians;
	if (glm::length(axis) > 0.0f)
	{
		rotationAxis = glm::normalize(axis);
	}
}

void Object::rotate(float deltaAngleRadians, const glm::vec3 &axis)
{
	rotationAngle += deltaAngleRadians;
	if (glm::length(axis) > 0.0f)
	{
		rotationAxis = glm::normalize(axis);
	}
}

glm::mat4 Object::getModelMatrix() const
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::rotate(model, rotationAngle, rotationAxis);
	model = glm::scale(model, size);
	return model;
}

unsigned int Object::getVAO() const
{
	return VAO;
}

unsigned int Object::getVBO() const
{
	return VBO;
}
