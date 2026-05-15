#version 330 core
out vec4 FragColor;

const int MAX_LIGHTS = 8;
const int BEHAVIOR_NONE = 0;
const int BEHAVIOR_OSCILLATE = 1;
const int BEHAVIOR_SPIN = 2;
const int BEHAVIOR_FLY = 3;

in vec3 Normal;
in vec3 FragPos;

uniform int lightCount;
uniform vec3 lightBasePos[MAX_LIGHTS];
uniform int lightBehaviorType[MAX_LIGHTS];
uniform float lightBehaviorSpeed[MAX_LIGHTS];
uniform vec3 lightBehaviorAxis[MAX_LIGHTS];
uniform float lightBehaviorAmplitude[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float uTime;

vec3 safeNormalize(vec3 value, vec3 fallbackValue)
{
    float lengthValue = length(value);
    if (lengthValue > 0.0001)
    {
        return value / lengthValue;
    }
    return fallbackValue;
}

vec3 evaluateLightPosition(int index)
{
    vec3 position = lightBasePos[index];
    int behaviorType = lightBehaviorType[index];
    if (behaviorType == BEHAVIOR_OSCILLATE)
    {
        position += safeNormalize(lightBehaviorAxis[index], vec3(0.0, 1.0, 0.0)) *
                    sin(uTime * lightBehaviorSpeed[index]) *
                    lightBehaviorAmplitude[index];
    }
    else if (behaviorType == BEHAVIOR_FLY)
    {
        position += safeNormalize(lightBehaviorAxis[index], vec3(0.0, 1.0, 0.0)) *
                    (lightBehaviorSpeed[index] * uTime);
    }

    return position;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    float ambientStrength = 0.15;
    float specularStrength = 0.5;
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);

    int effectiveLightCount = min(lightCount, MAX_LIGHTS);
    for (int i = 0; i < effectiveLightCount; ++i)
    {
        vec3 lightPosition = evaluateLightPosition(i);
        vec3 lightDir = normalize(lightPosition - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor[i];

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
        vec3 specular = specularStrength * spec * lightColor[i];

        float distanceToLight = length(lightPosition - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distanceToLight +
                                   0.032 * distanceToLight * distanceToLight);

        totalAmbient += ambientStrength * lightColor[i] * attenuation;
        totalDiffuse += diffuse * attenuation;
        totalSpecular += specular * attenuation;
    }

    // Animated field in world-space: moving bands + radial pulse.
    float waveA = sin(FragPos.x * 0.8 + uTime * 2.2);
    float waveB = cos(FragPos.z * 1.1 - uTime * 1.6);
    float radial = sin(length(FragPos.xz) * 2.4 - uTime * 3.1);
    float field = 0.5 + 0.5 * (waveA * waveB * radial);

    vec3 colorA = vec3(0.12, 0.95, 0.85);
    vec3 colorB = vec3(0.98, 0.20, 0.95);
    vec3 mathColor = mix(colorA, colorB, field);

    vec3 lit = (totalAmbient + totalDiffuse) * mathColor + totalSpecular;

    // Blend with objectColor so JSON color still influences the result.
    vec3 finalColor = mix(lit, lit * objectColor, 0.25);
    FragColor = vec4(finalColor, 1.0);
}
