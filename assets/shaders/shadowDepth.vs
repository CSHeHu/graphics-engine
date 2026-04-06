#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 model;

void main()
{
    WorldPos = vec3(model * vec4(aPos, 1.0));
}