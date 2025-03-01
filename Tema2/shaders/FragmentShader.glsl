#version 330

// Input
in vec3 fragPosition;
in vec3 fragnormal;
in vec3 fragColor;

//output
out vec4 FragColor;

uniform vec3 object_color;
uniform bool applyNoise;


void main() {
    vec3 color;

    if(applyNoise) {
        // aflam noise-ul = inaltimea punctului
        float height = fragPosition.y;
		// adaugam noise la culoare
        // culoarea v-a fi verde daca inaltimea este mai mica decat 0.5 si galben daca este mai mare
		color = mix(vec3(0.0, 0.5, 0.0), vec3(0.9, 0.7, 0.2), height); 
    } else {
		color = object_color;
	}

    FragColor = vec4(color, 1.0);
}