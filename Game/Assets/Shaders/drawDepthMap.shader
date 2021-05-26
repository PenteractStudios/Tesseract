--- vertDrawDepthMapTexture

out vec2 texCoords;

void main() {
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	texCoords.x = (x + 1.0) * 0.5;
	texCoords.y = (y + 1.0) * 0.5;

	gl_Position = vec4(x, y, 0.0, 1.0);
}

--- fragDrawDepthMapTexture

in vec2 texCoords;

out vec4 color;

uniform sampler2D depthMapTexture;

void main() {
	
	float depthValue = texture(depthMapTexture, texCoords).z;
	color = vec4(vec3(depthValue), 1.0);
}