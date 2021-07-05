--- fragGaussianBlur

in vec2 uv;

out vec4 color;

uniform sampler2D inputTexture;

uniform float smallKernel[3];
uniform float mediumKernel[11];
uniform float largeKernel[19];
uniform float smallWeight;
uniform float mediumWeight;
uniform float largeWeight;

uniform int horizontal;

void main() {
	vec4 resultColor = texture(inputTexture, uv) * smallKernel[0];
	for (int i = 1; i < 3; ++i) {
		float kernelVal = smallKernel[i];
		vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) / textureSize(inputTexture, 0);
		resultColor += texture(inputTexture, uv + offsetUV) * kernelVal;
		resultColor += texture(inputTexture, uv - offsetUV) * kernelVal;
	}
	resultColor *= smallWeight;

	if (mediumWeight > 0) {
		vec4 resultColor2 = texture(inputTexture, uv) * mediumKernel[0];
		for (int i = 1; i < 11; ++i) {
			float kernelVal = mediumKernel[i];
			vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) / textureSize(inputTexture, 0);
			resultColor2 += texture(inputTexture, uv + offsetUV) * kernelVal;
			resultColor2 += texture(inputTexture, uv - offsetUV) * kernelVal;
		}
		resultColor += resultColor2 * mediumWeight;
	}

	if (largeWeight > 0) {
		vec4 resultColor3 = texture(inputTexture, uv) * largeKernel[0];
		for (int i = 1; i < 19; ++i) {
			float kernelVal = largeKernel[i];
			vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) / textureSize(inputTexture, 0);
			resultColor3 += texture(inputTexture, uv + offsetUV) * kernelVal;
			resultColor3 += texture(inputTexture, uv - offsetUV) * kernelVal;
		}
		resultColor += resultColor3 * largeWeight;
	}

	color = resultColor;
}