--- gammaCorrection

#define GAMMA 2.2

vec4 SRGBA(in vec4 color) {
	return vec4(pow(color.rgb, vec3(GAMMA)), color.a);
}

vec3 SRGB(in vec3 color) {
	return pow(color.rgb, vec3(GAMMA));
}

--- fragColorCorrection

in vec2 uv;

out vec4 outColor;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

vec3 ACESFilm(in vec3 x) {
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
	vec4 hdrColor = texture(scene, uv);
	//vec3 bloomColor = texture(bloomBlur, uv).rgb;
	//hdrColor.rgb += bloomColor; // additive blending

	// ACES Tonemapping
	vec3 ldr = ACESFilm(hdrColor.rgb);

	// Gamma Correction
	ldr = pow(ldr, vec3(1 / GAMMA));

	// Output
	outColor = vec4(ldr, 1.0);
}