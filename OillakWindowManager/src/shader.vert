#version 450


vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),  // Ylin piste
    vec2(0.5, 0.5),   // Oikea alanurkka
    vec2(-0.5, 0.5)   // Vasen alanurkka
);

// Kovakoodatut värit (R, G, B) jokaiselle pisteelle
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0), // Punainen
    vec3(0.0, 1.0, 0.0), // Vihreä
    vec3(0.0, 0.0, 1.0)  // Sininen
);

// Lähetetään väri eteenpäin Fragment Shaderille
layout(location = 0) out vec3 fragColor;

void main() {
    // gl_VertexIndex kertoo, monesko piste on menossa (0, 1 tai 2)
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}