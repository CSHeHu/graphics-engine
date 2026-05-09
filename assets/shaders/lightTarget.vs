#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

#define INSTANCE_MODEL_LOCATION 2
layout (location = INSTANCE_MODEL_LOCATION) in mat4 instanceModel;

out vec3 FragPos;
out vec3 Normal;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(instanceModel * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(instanceModel))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}