--- gammaCorrection

#define GAMMA 2.2

vec4 SRGB(in vec4 color) {
	return vec4(pow(color.rgb, vec3(GAMMA)), color.a);
}

--- fragColorCorrection

in vec2 uv;

out vec4 color;

uniform sampler2D inputTexture;

vec3 ACESFilm(in vec3 x) {
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
	// ACES Tonemapping
	vec3 ldr = ACESFilm(texture(inputTexture, uv).rgb);
	
	// Gamma Correction
    ldr = pow(ldr, vec3(1/GAMMA)); 

	// Output
	color = vec4(ldr, 1.0);
}