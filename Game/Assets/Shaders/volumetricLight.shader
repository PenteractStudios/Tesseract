--- vertVolumetricLight

#define MAX_BONES 100

in layout(location=0) vec3 pos;
in layout(location=1) vec3 norm;
in layout(location=2) vec3 tangent;
in layout(location=3) vec2 uvs;
in layout(location=4) uvec4 boneIndices;
in layout(location=5) vec4 boneWeitghts;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 palette[MAX_BONES];
uniform bool hasBones;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 uv;

void main()
{
    vec4 position = vec4(pos, 1.0);
    vec4 normal = vec4(norm, 0.0);

    if (hasBones)
    { 
        mat4 skinT = palette[boneIndices[0]] * boneWeitghts[0] + palette[boneIndices[1]] * boneWeitghts[1]
        + palette[boneIndices[2]] * boneWeitghts[2] + palette[boneIndices[3]] * boneWeitghts[3];

        position = skinT * vec4(pos, 1.0);
        normal = skinT * vec4(norm, 0.0);
    }

    gl_Position = proj * view * model * position;
    fragPos = vec3(model * position);
    fragNormal = normalize(transpose(inverse(mat3(model))) * normal.xyz);
	uv = uvs;
}

--- fragVolumetricLight

in vec3 fragPos;
in vec3 fragNormal;
in vec2 uv;

uniform vec3 viewPos;

uniform float near;
uniform float far;

uniform sampler2D depths;

uniform vec4 lightColor;
uniform sampler2D lightMap;
uniform int hasLightMap;
uniform float intensity;
uniform float attenuationExponent;

uniform int isSoft;
uniform float softRange;

out vec4 outColor;

float LinearizeDepth(float depth) 
{
    return (far * near) / (far - depth * (far - near));	
}

vec4 GetLight(vec2 uv)
{
    return (hasLightMap * SRGBA(texture(lightMap, uv)) + (1 - hasLightMap)) * SRGBA(lightColor) * vec4(vec3(intensity), 1.0);
}

void main()
{
	vec3 V = viewPos - fragPos;
	float viewDist = length(V);
    vec3 viewDir = normalize(V);
    float NV = dot(viewDir, fragNormal);
	float fragAlpha = clamp(intensity * pow(NV, attenuationExponent) * min(viewDist * 0.5 - 1.0, 1.0), 0.0, 1.0);

	outColor = GetLight(uv);
	outColor.a *= fragAlpha;
	
	if (isSoft == 1) {
		vec2 screenUV = gl_FragCoord.xy / vec2(textureSize(depths, 0));
		float sceneDepth = LinearizeDepth(texture2D(depths, screenUV).r);
		float fragDepth = LinearizeDepth(gl_FragCoord.z);
		float depthDelta = sceneDepth - fragDepth;
		float opacity = smoothstep(0.0, 1.0, depthDelta / softRange);
		outColor.a *= opacity;
	}
}