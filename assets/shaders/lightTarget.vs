#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 instanceBasePositionRotationAngle;
layout (location = 3) in vec4 instanceBaseScaleBehaviorType;
layout (location = 4) in vec4 instanceBaseRotationAxisSpeed;
layout (location = 5) in vec4 instanceBehaviorAxisAmplitude;

out vec3 FragPos;
out vec3 Normal;
uniform mat4 view;
uniform mat4 projection;
uniform float uTime;

const int BEHAVIOR_NONE = 0;
const int BEHAVIOR_OSCILLATE = 1;
const int BEHAVIOR_SPIN = 2;
const int BEHAVIOR_FLY = 3;

vec3 safeNormalize(vec3 value, vec3 fallbackValue)
{
    float lengthValue = length(value);
    if (lengthValue > 0.0001)
    {
        return value / lengthValue;
    }
    return fallbackValue;
}

mat4 translationMatrix(vec3 translation)
{
    return mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0),
                vec4(0.0, 0.0, 1.0, 0.0), vec4(translation, 1.0));
}

mat4 scaleMatrix(vec3 scaleValue)
{
    return mat4(vec4(scaleValue.x, 0.0, 0.0, 0.0),
                vec4(0.0, scaleValue.y, 0.0, 0.0),
                vec4(0.0, 0.0, scaleValue.z, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    vec3 normalizedAxis = safeNormalize(axis, vec3(0.0, 1.0, 0.0));
    float cosineValue = cos(angle);
    float sineValue = sin(angle);
    float oneMinusCosine = 1.0 - cosineValue;

    return mat4(
        vec4(oneMinusCosine * normalizedAxis.x * normalizedAxis.x + cosineValue,
             oneMinusCosine * normalizedAxis.x * normalizedAxis.y +
                 sineValue * normalizedAxis.z,
             oneMinusCosine * normalizedAxis.x * normalizedAxis.z -
                 sineValue * normalizedAxis.y,
             0.0),
        vec4(oneMinusCosine * normalizedAxis.x * normalizedAxis.y -
                 sineValue * normalizedAxis.z,
             oneMinusCosine * normalizedAxis.y * normalizedAxis.y + cosineValue,
             oneMinusCosine * normalizedAxis.y * normalizedAxis.z +
                 sineValue * normalizedAxis.x,
             0.0),
        vec4(oneMinusCosine * normalizedAxis.x * normalizedAxis.z +
                 sineValue * normalizedAxis.y,
             oneMinusCosine * normalizedAxis.y * normalizedAxis.z -
                 sineValue * normalizedAxis.x,
             oneMinusCosine * normalizedAxis.z * normalizedAxis.z + cosineValue,
             0.0),
        vec4(0.0, 0.0, 0.0, 1.0));
}

void main()
{
    vec3 basePosition = instanceBasePositionRotationAngle.xyz;
    float baseRotationAngle = instanceBasePositionRotationAngle.w;
    vec3 baseScale = instanceBaseScaleBehaviorType.xyz;
    int behaviorType = int(instanceBaseScaleBehaviorType.w + 0.5);
    vec3 baseRotationAxis = instanceBaseRotationAxisSpeed.xyz;
    float behaviorSpeed = instanceBaseRotationAxisSpeed.w;
    vec3 behaviorAxis = instanceBehaviorAxisAmplitude.xyz;
    float behaviorAmplitude = instanceBehaviorAxisAmplitude.w;

    vec3 animatedPosition = basePosition;
    float animatedRotationAngle = baseRotationAngle;
    vec3 animatedRotationAxis = baseRotationAxis;

    if (behaviorType == BEHAVIOR_OSCILLATE)
    {
        animatedPosition += safeNormalize(behaviorAxis, vec3(0.0, 1.0, 0.0)) *
                            sin(uTime * behaviorSpeed) * behaviorAmplitude;
    }
    else if (behaviorType == BEHAVIOR_SPIN)
    {
        animatedRotationAngle = baseRotationAngle + behaviorSpeed * uTime;
        animatedRotationAxis = safeNormalize(behaviorAxis, baseRotationAxis);
    }
    else if (behaviorType == BEHAVIOR_FLY)
    {
        animatedPosition += safeNormalize(behaviorAxis, vec3(0.0, 1.0, 0.0)) *
                            (behaviorSpeed * uTime);
    }

    mat4 model = translationMatrix(animatedPosition) *
                 rotationMatrix(animatedRotationAxis, animatedRotationAngle) *
                 scaleMatrix(baseScale);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}