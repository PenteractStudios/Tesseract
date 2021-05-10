--- vertDepthMap

layout (location = 0) in vec3 position;

uniform mat4 view;
uniform mat4 projection;

out vec2 texcoords;

void main() {
	gl_Position = projection * vec4(mat3(view) * position, 1.0);
}

--- fragDepthMap

void main() {
	
	// gl_FragDepth = gl_FragCoord.z;

}