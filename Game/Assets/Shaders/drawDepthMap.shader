--- vertDrawDepthMap

out vec2 texCoords;

void main() {

	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	texCoords.x = (x + 1.0) * 0.5;
	texCoords.y = (y + 1.0) * 0.5;

	gl_Position = vec4(x, y, 0.0, 1.0);
}

--- fragDrawDepthMap

in vec2 texCoords;

out vec4 color;

uniform sampler2D depthMap;
uniform float nearPlane;
uniform float farPlane;

float LinearizeDepth(float depth) {
	float z = depth * 2.0 - 1.0; // Back to NDC
	return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main() {
	
	float depthValue = texture(depthMap, texCoords).r;
	color = vec4(vec3(LinearizeDepth(depthValue) / farPlane), 1.0);

}