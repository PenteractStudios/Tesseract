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

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform int hasBloom;
uniform float bloomIntensity;

uniform int hasChromaticAberration;
uniform float chromaticAberrationStrength;

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
	vec4 hdrColor = texture(sceneTexture, uv);

	// Apply chromatic aberration
	if (hasChromaticAberration == 1) {
		vec2 d = ((uv - vec2(.5)) * .0075) * chromaticAberrationStrength;
		vec3 color = vec3(texture(sceneTexture, uv - 0.0 * d).r,
			texture(sceneTexture, uv - 1.0 * d).g,
			texture(sceneTexture, uv - 2.0 * d).b);

		hdrColor = vec4(color, hdrColor.a);
	}

	// Apply bloom
	if (hasBloom == 1) {
		hdrColor.rgb += textureLod(bloomTexture, uv, 0).rgb * bloomIntensity; // additive blending
	}

	// ACES Tonemapping
	vec3 ldr = ACESFilm(hdrColor.rgb);

	// Gamma Correction
	ldr = pow(ldr, vec3(1 / GAMMA));

	// Output
	outColor = vec4(ldr, 1.0);
}