--- fragDrawSSAOTexture

in vec2 uv;

out vec4 color;

uniform sampler2D ssaoTexture;

void main() {
	float occlusion = texture(ssaoTexture, uv).x;
	color = vec4(vec3(occlusion), 1.0);
}