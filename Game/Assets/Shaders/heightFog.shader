--- fragHeightFog

uniform sampler2DMS originalRender;
uniform sampler2DMS positions;

uniform vec3 viewPos;

uniform float falloff;

in vec2 uv;

layout(location = 0) out vec4 result;

void main()
{
	ivec2 vp = textureSize(originalRender);
	vp = ivec2(vec2(vp) * uv);
    vec3 position = texelFetch(positions, vp, gl_SampleID).xyz;
    vec3 V = position;
	float viewDist = length(V);
    vec3 viewDir = normalize(V);

    float fogAmount = 1.0 - exp(-viewDist * falloff);
    vec3 fogColor = vec3(0.5, 0.6, 0.7);
    result = vec4(mix(texelFetch(originalRender, vp, gl_SampleID).rgb, fogColor, fogAmount), 1.0);
}