--- vertDepthMap

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 texcoords;

void main() {
	gl_Position = proj * view * model * vec4(position, 1.0f);
}

--- fragDepthMap

void main() {
	
	// gl_FragDepth = gl_FragCoord.z;

}