--- fragGaussianBlur

in vec2 uv;

out vec4 color;

uniform sampler2D inputTexture;
uniform float kernel[3];
uniform int horizontal;

void main() {
	vec4 resultColor = texture(inputTexture, uv) * kernel[0];
	for (int i = 1; i < 3; ++i) {
		float kernelVal = kernel[i];
		vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) / textureSize(inputTexture, 0);
		resultColor += texture(inputTexture, uv + offsetUV) * kernelVal;
		resultColor += texture(inputTexture, uv - offsetUV) * kernelVal;
	}
	color = resultColor;
}