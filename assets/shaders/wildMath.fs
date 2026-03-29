#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float uTime;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    float diffuse = max(dot(norm, lightDir), 0.0);
    float specular = pow(max(dot(viewDir, reflect(-lightDir, norm)), 0.0), 16.0);

    // Animated field in world-space: moving bands + radial pulse.
    float waveA = sin(FragPos.x * 0.8 + uTime * 2.2);
    float waveB = cos(FragPos.z * 1.1 - uTime * 1.6);
    float radial = sin(length(FragPos.xz) * 2.4 - uTime * 3.1);
    float field = 0.5 + 0.5 * (waveA * waveB * radial);

    vec3 colorA = vec3(0.12, 0.95, 0.85);
    vec3 colorB = vec3(0.98, 0.20, 0.95);
    vec3 mathColor = mix(colorA, colorB, field);

    float ambient = 0.15;
    vec3 lit = (ambient + 0.8 * diffuse) * mathColor + 0.35 * specular * lightColor;

    // Blend with objectColor so JSON color still influences the result.
    vec3 finalColor = mix(lit, lit * objectColor, 0.25);
    FragColor = vec4(finalColor, 1.0);
}
