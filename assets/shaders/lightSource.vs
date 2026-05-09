#version 330 core
layout (location = 0) in vec3 aPos;

#define INSTANCE_MODEL_LOCATION 2
#define INSTANCE_COLOR_LOCATION 6

layout (location = INSTANCE_MODEL_LOCATION) in mat4 instanceModel;
layout (location = INSTANCE_COLOR_LOCATION) in vec4 instanceColor;

uniform mat4 view;
uniform mat4 projection;

out vec4 vColor;

void main()
{
    vColor = instanceColor;
    gl_Position = projection * view * instanceModel * vec4(aPos, 1.0);
} 