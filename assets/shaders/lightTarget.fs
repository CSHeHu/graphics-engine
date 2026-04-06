#version 330 core
out vec4 FragColor;

const int MAX_LIGHTS = 8;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform int lightCount;
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform vec3 objectColor;
uniform vec3 viewPos; 

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Small base ambient term so fully unlit fragments are still visible.
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0);

    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    float specularStrength = 0.5;
    int effectiveLightCount = min(lightCount, MAX_LIGHTS);
    for (int i = 0; i < effectiveLightCount; ++i)
    {
        vec3 lightDir = normalize(lightPos[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor[i];

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor[i];

        float distanceToLight = length(lightPos[i] - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * distanceToLight * distanceToLight);

        totalDiffuse += diffuse * attenuation;
        totalSpecular += specular * attenuation;
    }

    vec3 result = (ambient + totalDiffuse + totalSpecular) * objectColor;
    FragColor = vec4(result, 1.0);
} 