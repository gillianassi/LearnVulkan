#version 450

// output to location 0
layout (location = 0) in vec3 fragColor;
layout (location = 0) out vec4 OutColor;

void main() {
	OutColor = vec4(fragColor, 1.0);
}