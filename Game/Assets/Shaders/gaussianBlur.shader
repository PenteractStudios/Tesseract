--- fragGaussianBlur

in vec2 uv;

out vec4 color;

uniform sampler2D inputTexture;
uniform int textureLevel;

uniform float kernel[40];
uniform int kernelRadius;
uniform int horizontal;

void main()
{
	vec3 resultColor = textureLod(inputTexture, uv, textureLevel).rgb * kernel[0];
	vec2 texelSize = 1.0 / textureSize(inputTexture, textureLevel);
	for (int i = 1; i < kernelRadius; ++i) {
		float kernelVal = kernel[i];
		vec2 offsetUV = vec2(horizontal * i, (1.0 - horizontal) * i) * texelSize;
		resultColor += textureLod(inputTexture, uv + offsetUV, textureLevel).rgb * kernelVal;
		resultColor += textureLod(inputTexture, uv - offsetUV, textureLevel).rgb * kernelVal;
	}
	color = vec4(resultColor, 1.0);
}