--- fragGaussianBlur

in vec2 uv;

out vec4 color;

uniform sampler2D inputTexture;

uniform float smallKernel[200];
uniform float mediumKernel[200];
uniform float largeKernel[200];

uniform float smallWeight;
uniform float mediumWeight;
uniform float largeWeight;

uniform int smallRadius;
uniform int mediumRadius;
uniform int largeRadius;

uniform int horizontal;

void main() {
	vec4 resultColor = textureLod(inputTexture, uv, 0) * smallKernel[0];
	for (int i = 1; i < smallRadius; ++i) {
		float kernelVal = smallKernel[i];
		vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) / textureSize(inputTexture, 0);
		resultColor += textureLod(inputTexture, uv + offsetUV, 0) * kernelVal;
		resultColor += textureLod(inputTexture, uv - offsetUV, 0) * kernelVal;
	}
	resultColor *= smallWeight;

	if (mediumWeight > 0) {
		vec4 resultColor2 = textureLod(inputTexture, uv, 1) * mediumKernel[0];
		for (int i = 1; i < mediumRadius; ++i) {
			float kernelVal = mediumKernel[i];
			vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) / textureSize(inputTexture, 0);
			resultColor2 += textureLod(inputTexture, uv + offsetUV, 1) * kernelVal;
			resultColor2 += textureLod(inputTexture, uv - offsetUV, 1) * kernelVal;
		}
		resultColor += resultColor2 * mediumWeight;
	}

	if (largeWeight > 0) {
		vec4 resultColor3 = textureLod(inputTexture, uv, 2) * largeKernel[0];
		for (int i = 1; i < largeRadius; ++i) {
			float kernelVal = largeKernel[i];
			vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) / textureSize(inputTexture, 0);
			resultColor3 += textureLod(inputTexture, uv + offsetUV, 2) * kernelVal;
			resultColor3 += textureLod(inputTexture, uv - offsetUV, 2) * kernelVal;
		}
		resultColor += resultColor3 * largeWeight;
	}

	color = resultColor;
}