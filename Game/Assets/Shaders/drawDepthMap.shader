--- fragDrawDepthMapTexture

in vec2 uv;

out vec4 color;

uniform sampler2D depthMapTexture;

void main() {
	
	float depthValue = texture(depthMapTexture, uv).z;
	color = vec4(vec3(depthValue), 1.0);
}