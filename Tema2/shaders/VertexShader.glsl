#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;     // Vertex normala
layout(location = 2) in vec3 color;

out vec3 fragColor;
out vec3 fragPosition;
out vec3 fragnormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float noiseFactor;
uniform bool applyNoise;

uniform float heightScale; // factor de scalare a inaltimii

float noise(vec2 p) {
	return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
	
	//NORMALA
	fragnormal = normal;

	vec3 modifiedPosition;
	if(applyNoise) {
		// modificam inaltimea punctelor in functie de noise
		float noiseHeight = noise(vec2(position.x, position.z) * noiseFactor);

		modifiedPosition = vec3(position.x, noiseHeight * heightScale, position.z);
		//modifiedPosition = vec3(position.x, noiseHeight, position.z);
	} else {
		modifiedPosition = position;
	}

	fragPosition = modifiedPosition;  // positia fragmentului
	fragColor = color;  // culoarea fragmentului
	
	//calculam pozitia finala a punctului
	gl_Position = projection * view * model * vec4(modifiedPosition, 1.0f);
}