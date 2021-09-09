--- fragBloomCombine

in vec2 uv;

out vec4 color;

uniform sampler2D brightTexture;
uniform sampler2D bloomTexture;
uniform int brightLevel;
uniform int bloomLevel;
uniform float bloomWeight;

void main()
{
	vec3 bloom = textureLod(bloomTexture, uv, bloomLevel).rgb;
	vec3 bright = textureLod(brightTexture, uv, brightLevel).rgb;
	color = vec4(bloom * bloomWeight + bright * (1.0 - bloomWeight), 1.0);
}