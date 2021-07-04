--- fragPostprocess

in vec2 uv;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bloom;

uniform sampler2DMS sceneTexture;
uniform float bloomThreshold;
uniform int samplesNumber;

vec3 GetTexel(in vec2 uv)
{
	ivec2 vp = textureSize(sceneTexture);
	vp = ivec2(vec2(vp) * uv);
	
	vec3 accumulatedSample = vec3(0.0f);
	for (int i = 0; i < samplesNumber; ++i) {
		accumulatedSample += texelFetch(sceneTexture, vp, i).rgb;
	}

	return accumulatedSample / samplesNumber;
}

void main()
{
	color = vec4(GetTexel(uv), 1.0);
	float bright = dot(color.rgb, vec3(0.3, 0.6, 0.2));
	if (bright > bloomThreshold) bloom = color * smoothstep(0.0, 1.0, bright - bloomThreshold);
	else bloom = vec4(0.0, 0.0, 0.0, 1.0);
}