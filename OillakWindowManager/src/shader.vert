#version 450

// Kamera/yleiset tiedot (jos tarpeen)
layout(binding = 0) uniform UniformBufferObject {
    mat4 viewProj; // Esim. kamera
} ubo;

// Objekti-kohtainen tieto (TÄMÄ ON UUSI)
layout(push_constant) uniform Push {
    mat4 model;
} push;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inFragColor;

layout(location = 0) out vec3 fragColor;

void main() {
    // Lasketaan lopullinen sijainti: Model * ViewProj * Position
    gl_Position = ubo.viewProj * push.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inFragColor;
}