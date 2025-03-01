#version 330

in vec3 fragColor;

uniform vec3 shadow_color; // Culoarea umbrei

out vec4 outColor;

void main() {
    // Aplicam culoarea umbrei
    outColor = vec4(shadow_color, 1.0);
}
