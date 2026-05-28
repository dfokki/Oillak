#version 450

// Otetaan vastaan väri Vertex Shaderilta (pitää olla sama location = 0)
layout(location = 0) in vec3 fragColor;

// Lopullinen pikselin väri ruudulle (R, G, B, A)
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}