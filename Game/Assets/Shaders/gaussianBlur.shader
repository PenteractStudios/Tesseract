--- fragGaussianBlur

in vec2 uv;

out vec4 color;

uniform sampler2D inputTexture;

uniform float smallKernel[40];
uniform float mediumKernel[40];
uniform float largeKernel[40];

uniform float smallWeight;
uniform float mediumWeight;
uniform float largeWeight;

uniform int smallRadius;
uniform int mediumRadius;
uniform int largeRadius;

uniform int horizontal;

void main() {
	vec3 resultColor = textureLod(inputTexture, uv, 0).rgb * smallKernel[0];
	vec2 texel = 1 / textureSize(inputTexture, 0);
	for (int i = 1; i < smallRadius; ++i) {
		float kernelVal = smallKernel[i];
		vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) * texel;
		resultColor += textureLod(inputTexture, uv + offsetUV, 0).rgb * kernelVal;
		resultColor += textureLod(inputTexture, uv - offsetUV, 0).rgb * kernelVal;
	}
	resultColor *= smallWeight;

	if (mediumWeight > 0) {
		vec3 resultColor2 = textureLod(inputTexture, uv, 3).rgb * mediumKernel[0];
		vec2 texel = 1 / textureSize(inputTexture, 3);
		for (int i = 1; i < mediumRadius; ++i) {
			float kernelVal = mediumKernel[i];
			vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) * texel;
			resultColor2 += textureLod(inputTexture, uv + offsetUV, 3).rgb * kernelVal;
			resultColor2 += textureLod(inputTexture, uv - offsetUV, 3).rgb * kernelVal;
		}
		resultColor += resultColor2 * mediumWeight;
	}

	if (largeWeight > 0) {
		vec3 resultColor3 = textureLod(inputTexture, uv, 5).rgb * largeKernel[0];
		vec2 texel = 1 / textureSize(inputTexture, 5);
		for (int i = 1; i < largeRadius; ++i) {
			float kernelVal = largeKernel[i];
			vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) * texel;
			resultColor3 += textureLod(inputTexture, uv + offsetUV, 5).rgb * kernelVal;
			resultColor3 += textureLod(inputTexture, uv - offsetUV, 5).rgb * kernelVal;
		}
		resultColor += resultColor3 * largeWeight;
	}

	color = vec4(resultColor, 1.0);
}