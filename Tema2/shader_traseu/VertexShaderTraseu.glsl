#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;  
layout(location = 2) in vec3 color;    

out vec3 fragColor;

uniform mat4 model; 
uniform mat4 view;       
uniform mat4 projection;
uniform vec3 lightDirection; // Directia luminii (diagonala)

void main() {
    vec4 worldPosition = model * vec4(position, 1.0);

    // Proiectam umbra pe planul XOZ in functie de directia luminii
    vec4 shadowPosition = worldPosition;

    // Proiectia pe planul XOZ: ajustam pozitia pe baza directiei luminii
    shadowPosition.x -= lightDirection.x * worldPosition.y / lightDirection.y;
    shadowPosition.z -= lightDirection.z * worldPosition.y / lightDirection.y;
    shadowPosition.y = 0.7; // Fixam umbra pe planul XOZ (la y = 0.7) din cauza terenului

    // Proiectam umbra în spatiul ecranului
    gl_Position = projection * view * shadowPosition;

    // Transmitem culoarea 
    fragColor = color;
}
