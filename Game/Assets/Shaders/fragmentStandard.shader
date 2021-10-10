--- fragVarStandard

#define PI 3.1415926538
#define EPSILON 1e-5
#define MAX_CASCADES 10

in vec3 fragNormal;
in mat3 TBN;
in vec3 fragPos;
in vec2 uv;

// Cascade Shadow Mapping
in vec4 fragPosLightStatic[MAX_CASCADES];
in vec4 fragPosLightDynamic[MAX_CASCADES];
flat in unsigned int cascadesCount;

out vec4 outColor;

// Depth Map
uniform sampler2D depthMapTexturesStatic[MAX_CASCADES];
uniform sampler2D depthMapTexturesDynamic[MAX_CASCADES];
uniform float farPlaneDistancesStatic[MAX_CASCADES];
uniform float farPlaneDistancesDynamic[MAX_CASCADES];

// SSAO texture
uniform sampler2D ssaoTexture;
uniform float ssaoDirectLightingStrength;

uniform mat4 proj;
uniform mat4 view;

uniform vec3 viewPos;

// Material
uniform int isOpaque;
uniform vec4 diffuseColor;
uniform sampler2D diffuseMap;
uniform int hasDiffuseMap;
uniform sampler2D normalMap;
uniform bool hasNormalMap;
uniform float normalStrength;
uniform float smoothness;
uniform sampler2D emissiveMap;
uniform int hasEmissiveMap;
uniform vec4 emissiveColor;
uniform float emissiveIntensity;
uniform sampler2D ambientOcclusionMap;
uniform int hasAmbientOcclusionMap;
uniform int hasSmoothnessAlpha; // Generic used for Specular and Metallic
uniform vec2 tiling;
uniform vec2 offset;

// IBL
uniform int hasIBL;
uniform samplerCube diffuseIBL;
uniform samplerCube prefilteredIBL;
uniform sampler2D environmentBRDF;
uniform int prefilteredIBLNumLevels;
uniform float strengthIBL;

// Lights
struct DirLight
{
	vec3 direction;
	vec3 color;
	float intensity;
	int isActive;
};

uniform vec3 ambientColor;
uniform DirLight dirLight;

float Pow2(float a)
{
	return a * a;
}

vec2 GetTiledUVs()
{
    return uv * tiling + offset;
}

vec4 GetDiffuse(vec2 tiledUV)
{
	vec4 projectedPos = proj * view * vec4(fragPos, 1.0);
	vec2 occlusionUV = (projectedPos.xy / projectedPos.w) * 0.5 + 0.5;
	float occlusionFactor = 1.0 - ((1.0 - texture(ssaoTexture, occlusionUV).r) * ssaoDirectLightingStrength);
    return (hasDiffuseMap * SRGBA(texture(diffuseMap, tiledUV)) + (1 - hasDiffuseMap)) * SRGBA(diffuseColor) * vec4(vec3(occlusionFactor), 1.0);
}

vec4 GetEmissive(vec2 tiledUV)
{
    //return hasEmissiveMap * SRGBA(texture(emissiveMap, tiledUV)) * emissiveIntensity;
	return (hasEmissiveMap * SRGBA(texture(emissiveMap, tiledUV)) * emissiveColor + (1 - hasEmissiveMap) * SRGBA(emissiveColor)) * emissiveIntensity;
}

vec3 GetAmbientLight(in vec3 R, in vec3 normal, in vec3 viewDir, in vec3 Cd, in vec3 F0, float roughness)
{
	if (hasIBL == 1)
	{
		float NV = max(dot(fragNormal, viewDir), 0.0) + EPSILON;
		vec3 irradiance = texture(diffuseIBL, normal).rgb;
		vec3 radiance = textureLod(prefilteredIBL, R, roughness * prefilteredIBLNumLevels).rgb;
		vec2 fab = texture(environmentBRDF, vec2(NV, roughness)).rg;
		vec3 diffuse = (Cd * (1 - F0));
		return (diffuse * irradiance + radiance * (F0 * fab.x + fab.y)) * strengthIBL;
	}
	else
	{
		vec3 diffuse = (Cd * (1 - F0));
		return diffuse * ambientColor;
	}
}

vec3 GetOccludedAmbientLight(in vec3 R, in vec3 normal, in vec3 viewDir, in vec3 Cd, in vec3 F0, float roughness, vec2 tiledUV)
{
	vec4 projectedPos = proj * view * vec4(fragPos, 1.0);
	vec2 occlusionUV = (projectedPos.xy / projectedPos.w) * 0.5 + 0.5;
	float occlusionFactor = texture(ssaoTexture, occlusionUV).r;
	vec3 ambientLight = GetAmbientLight(R, normal, viewDir, Cd, F0, roughness) * occlusionFactor;
    return (hasAmbientOcclusionMap * texture(ambientOcclusionMap, tiledUV).rgb + (1 - hasAmbientOcclusionMap)) * ambientLight.rgb;
}

vec3 GetNormal(vec2 tiledUV)
{

    vec3 normal = texture(normalMap, tiledUV).rgb;
    normal = normal * 2.0 - 1.0;
    normal.xy *= normalStrength;
    normal = normalize(normal);
    return normalize(TBN * normal);
}

unsigned int DepthMapIndexStatic(){

	for(unsigned int i = 0; i < cascadesCount; ++i){

		if(fragPosLightStatic[i].z < farPlaneDistancesStatic[i]) return i;

	}

	return cascadesCount - 1;

}

unsigned int DepthMapIndexDynamic(){

	for(unsigned int i = 0; i < cascadesCount; ++i){

		if(fragPosLightDynamic[i].z < farPlaneDistancesDynamic[i]) return i;

	}

	return cascadesCount - 1;

}

float Shadow(vec4 lightPos, vec3 normal, vec3 lightDirection, sampler2D shadowMap) {

	vec3 projCoords;
	//projCoords = lightPos.xyz / lightPos.w; // If perspective, we need to apply perspective division
	projCoords = lightPos.xyz;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, projCoords.xy).r;

	if(	projCoords.x < 0.0 || projCoords.x > 1.0 ||
		projCoords.y < 0.0 || projCoords.y > 1.0 ||
		closestDepth == 1.0
	) {
		return 0.0;
	}

    float currentDepth = projCoords.z;
	float bias = min(0.05 * (1 - dot(normal, lightDirection)), 0.005);

	float shadow = 0.0;

	vec2 texelSize = 1.0/textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x){
		for(int y = -1; y <= 1; ++y){
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth > pcfDepth + bias ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	return shadow;

}

--- fragVarLights

layout(std430, binding = 0) readonly buffer LightBuffer
{
	Light data[];
} lightBuffer;

layout(std430, binding = 1) readonly buffer LightIndicesBufferOpaque
{
	int data[];
} lightIndicesBufferOpaque;

layout(std430, binding = 2) readonly buffer LightTilesBufferOpaque
{
	LightTile data[];
} lightTilesBufferOpaque;

layout(std430, binding = 3) readonly buffer LightIndicesBufferTransparent
{
	int data[];
} lightIndicesBufferTransparent;

layout(std430, binding = 4) readonly buffer LightTilesBufferTransparent
{
	LightTile data[];
} lightTilesBufferTransparent;

uniform int tilesPerRow;

int GetTileIndex()
{
	ivec2 tile = ivec2(gl_FragCoord.xy) / ivec2(LIGHT_TILE_SIZE, LIGHT_TILE_SIZE);
	return tile.x + tile.y * tilesPerRow;
}

--- fragVarMetallic

uniform float metalness;
uniform sampler2D metallicMap;
uniform int hasMetallicMap;

--- fragVarSpecular

uniform vec3 specularColor;
uniform sampler2D specularMap;
uniform int hasSpecularMap;

--- fragFunctionLight

float GGXNormalDistribution(float NH, float roughness)
{
	return roughness * roughness / max(PI * Pow2(NH * NH * (roughness * roughness - 1) + 1), EPSILON);
}

vec3 SchlickFresnel(vec3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float SmithVisibility(float NL, float NV, float roughness)
{
	return 0.5 / (NL * (NV * (1 - roughness) + roughness) + NV * (NL * (1 - roughness) + roughness));
}

vec3 ProcessDirectionalLight(DirLight directional, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
	vec3 directionalDir = -normalize(directional.direction);

	float NL = max(dot(fragNormal, directionalDir), 0.0);
	float NV = max(dot(fragNormal, viewDir), 0.0) + EPSILON;
	vec3 H = (directionalDir + viewDir) / length(directionalDir + viewDir);
	float NH = max(dot(fragNormal, H), 0.0);
	float LH = max(dot(directionalDir, H), 0.0);

	vec3 Fn = SchlickFresnel(F0, LH);
	float V = SmithVisibility(NL, NV, roughness);
	float NDF = GGXNormalDistribution(NH, roughness);

	// Cook-Torrance BRDF
	return (Cd * (1 - F0) + 0.25 * Fn * V * NDF) * directional.color * directional.intensity * NL;
}

vec3 ProcessLight(Light light, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
	float lightDistance = length(light.pos - fragPos);
	float falloffExponent = light.useCustomFalloff * light.falloffExponent + (1 - light.useCustomFalloff) * 4.0;
	float distAttenuation = clamp(1.0 - pow(lightDistance / light.radius, falloffExponent), 0.0, 1.0);
	distAttenuation = light.useCustomFalloff * distAttenuation + (1 - light.useCustomFalloff) * distAttenuation * distAttenuation / (lightDistance * lightDistance + 1.0);

	vec3 lightDir = normalize(light.pos - fragPos);

	float cAttenuation = 1.0;
	if (light.isSpotLight == 1)
	{
		vec3 aimDir = normalize(light.direction);
		float C = dot(aimDir, -lightDir);
		float cosInner = cos(light.innerAngle);
		float cosOuter = cos(light.outerAngle);
		if (C > cosInner)
		{
			cAttenuation = 1.0;
		}
		else if (cosInner > C && C > cosOuter)
		{
			cAttenuation = (C - cosOuter) / (cosInner - cosOuter);
		}
		else
		{
			cAttenuation = 0.0;
		}
	}

	float NL = max(dot(fragNormal, lightDir), 0.0);
	float NV = max(dot(fragNormal, viewDir), 0.0) + EPSILON;
	vec3 H = (lightDir + viewDir) / length(lightDir + viewDir);
	float NH = max(dot(fragNormal, H), 0.0);
	float LH = max(dot(lightDir, H), 0.0);

	vec3 Fn = SchlickFresnel(F0, LH);
	float V = SmithVisibility(NL, NV, roughness);
	float NDF = GGXNormalDistribution(NH, roughness);

	// Cook-Torrance BRDF
	return (Cd * (1 - F0) + 0.25 * Fn * V * NDF) * light.color * light.intensity * distAttenuation * cAttenuation * NL;
}

--- fragMainMetallic

void main()
{
    vec3 viewDir = normalize(viewPos - fragPos);
    vec2 tiledUV = GetTiledUVs();
    vec3 normal = fragNormal;

    if (hasNormalMap)
    {
	    normal = GetNormal(tiledUV);
    }

    vec4 colorDiffuse = GetDiffuse(tiledUV);
    vec4 colorMetallic = texture(metallicMap, tiledUV);
    float metalnessMask = hasMetallicMap * colorMetallic.r + (1 - hasMetallicMap) * metalness;

	float roughness = Pow2(1 - smoothness * (hasSmoothnessAlpha * colorMetallic.a + (1 - hasSmoothnessAlpha) * colorDiffuse.a)) + EPSILON;

	// Schlick Fresnel
	vec3 Cd = colorDiffuse.rgb * (1 - metalnessMask);
	vec3 F0 = mix(vec3(0.04), colorDiffuse.rgb, metalnessMask);

    // Ambient Light
    vec3 R = reflect(-viewDir, normal);
    vec3 colorAccumulative = GetOccludedAmbientLight(R, normal, viewDir, Cd, F0, roughness, tiledUV);

	unsigned int indexS = DepthMapIndexStatic();
	unsigned int indexD = DepthMapIndexDynamic();
	float shadowS = Shadow(fragPosLightStatic[indexS], normal,  normalize(dirLight.direction), depthMapTexturesStatic[indexS]);
	float shadowD = Shadow(fragPosLightDynamic[indexD], normal,  normalize(dirLight.direction), depthMapTexturesDynamic[indexD]);

	float shadow = max(min(shadowD, 1), shadowS);

	// Directional Light
	if (dirLight.isActive == 1)
	{
		colorAccumulative += (1 - shadow) * ProcessDirectionalLight(dirLight, normal, viewDir, Cd, F0, roughness);
	}
	
	// Lights
	int tileIndex = GetTileIndex();
	if (isOpaque == 1)
	{
		LightTile lightTile = lightTilesBufferOpaque.data[tileIndex];
		for (uint i = 0; i < lightTile.count; i++ )
		{
			uint lightIndex = lightIndicesBufferOpaque.data[lightTile.offset + i];
			Light light = lightBuffer.data[lightIndex];
			colorAccumulative += ProcessLight(light, normal, viewDir, Cd, F0, roughness);
		}
	}
	else
	{
		LightTile lightTile = lightTilesBufferTransparent.data[tileIndex];
		for (uint i = 0; i < lightTile.count; i++ )
		{
			uint lightIndex = lightIndicesBufferTransparent.data[lightTile.offset + i];
			Light light = lightBuffer.data[lightIndex];
			colorAccumulative += ProcessLight(light, normal, viewDir, Cd, F0, roughness);
		}
	}

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;

	// Add dissolve	effect
	outColor = vec4(Dissolve(vec4(colorAccumulative, colorDiffuse.a), tiledUV).rgb, colorDiffuse.a);
}

--- fragMainSpecular

void main()
{
    vec3 viewDir = normalize(viewPos - fragPos);
    vec2 tiledUV = GetTiledUVs();
    vec3 normal = fragNormal;

    if (hasNormalMap)
    {
	    normal = GetNormal(tiledUV);
    }

    vec4 colorDiffuse = GetDiffuse(tiledUV);
    vec4 colorSpecular = hasSpecularMap * SRGBA(texture(specularMap, tiledUV)) + (1 - hasSpecularMap) * vec4(SRGB(specularColor), 1.0);

    float roughness = Pow2(1 - smoothness * (hasSmoothnessAlpha * colorSpecular.a + (1 - hasSmoothnessAlpha) * colorDiffuse.a)) + EPSILON;

    // Ambient Light
    vec3 R = reflect(-viewDir, normal);
    vec3 colorAccumulative = GetOccludedAmbientLight(R, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness, tiledUV);

	unsigned int indexS = DepthMapIndexStatic();
	unsigned int indexD = DepthMapIndexDynamic();
	float shadowS = Shadow(fragPosLightStatic[indexS], normal,  normalize(dirLight.direction), depthMapTexturesStatic[indexS]);
	float shadowD = Shadow(fragPosLightDynamic[indexD], normal,  normalize(dirLight.direction), depthMapTexturesDynamic[indexD]);

	float shadow = max(min(shadowD, 1), shadowS);

    // Directional Light
    if (dirLight.isActive == 1)
    {
        colorAccumulative += (1 - shadow) * ProcessDirectionalLight(dirLight, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
    }
	
	// Lights
	int tileIndex = GetTileIndex();
	if (isOpaque == 1)
	{
		LightTile lightTile = lightTilesBufferOpaque.data[tileIndex];
		for (uint i = 0; i < lightTile.count; i++)
		{
			uint lightIndex = lightIndicesBufferOpaque.data[lightTile.offset + i];
			Light light = lightBuffer.data[lightIndex];
			colorAccumulative += ProcessLight(light, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
		}
	}
	else
	{
		LightTile lightTile = lightTilesBufferTransparent.data[tileIndex];
		for (uint i = 0; i < lightTile.count; i++)
		{
			uint lightIndex = lightIndicesBufferTransparent.data[lightTile.offset + i];
			Light light = lightBuffer.data[lightIndex];
			colorAccumulative += ProcessLight(light, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
		}
	}

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;

    outColor = vec4(colorAccumulative, colorDiffuse.a);
}