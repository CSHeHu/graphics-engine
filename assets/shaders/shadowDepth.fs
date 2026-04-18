#version 330 core

in vec3 WorldPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    float lightDistance = length(WorldPos - lightPos);
    gl_FragDepth = lightDistance / farPlane;
}