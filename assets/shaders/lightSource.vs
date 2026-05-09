#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in mat4 instanceModel;
layout (location = 6) in vec4 instanceColor;

uniform mat4 view;
uniform mat4 projection;

out vec4 vColor;

void main()
{
    vColor = instanceColor;
    gl_Position = projection * view * instanceModel * vec4(aPos, 1.0);
} 