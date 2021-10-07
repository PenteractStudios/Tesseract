--- fragVarStandard

#define PI 3.1415926538
#define EPSILON 1e-5
#define MAX_CASCADES 4

in vec3 fragNormal;
in mat3 TBN;
in vec3 fragPos;
in vec2 uv;

// Cascade Shadow Mapping
in vec4 fragPosLightStatic[MAX_CASCADES];
in vec4 fragPosLightDynamic[MAX_CASCADES];

in vec3 viewFragPosStatic[MAX_CASCADES];
in vec3 viewFragPosDynamic[MAX_CASCADES];

flat in unsigned int cascadesCount;

out vec4 outColor;

// Depth Map
uniform sampler2DShadow depthMapTexturesStatic[MAX_CASCADES];
uniform sampler2DShadow depthMapTexturesDynamic[MAX_CASCADES];

uniform float farPlaneDistancesStatic[MAX_CASCADES];
uniform float farPlaneDistancesDynamic[MAX_CASCADES];

// SSAO texture
uniform sampler2D ssaoTexture;
uniform float ssaoDirectLightingStrength;

uniform mat4 proj;
uniform mat4 view;

uniform vec3 viewPos;

// Material
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
		if(viewFragPosStatic[i].z < farPlaneDistancesStatic[i]) return i;
	}

	return cascadesCount - 1;

}

unsigned int DepthMapIndexDynamic(){

	for(unsigned int i = 0; i < cascadesCount; ++i){
		if(viewFragPosDynamic[i].z < farPlaneDistancesDynamic[i]) return i;
	}

	return cascadesCount - 1;

}

float Shadow(vec4 lightPos, vec3 normal, vec3 lightDirection, sampler2DShadow shadowMap) {

	if(	lightPos.x < 0.0 || lightPos.x > 1.0 ||
		lightPos.y < 0.0 || lightPos.y > 1.0
	) {
		return 1.0;
	}

	//float bias = min(0.05 * (1 - dot(normal, lightDirection)), 0.005);
	float bias = 0.0;
	float shadow = 0.0;

	vec2 texelSize = 1.0/textureSize(shadowMap, 0);
	
	for(int x = -1; x <= 1; ++x){
		for(int y = -1; y <= 1; ++y){
			vec3 coord = vec3(lightPos.xy + vec2(x, y) * texelSize, lightPos.z - bias);
			shadow += texture(shadowMap, coord);
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

layout(std430, binding = 1) readonly buffer LightIndicesBuffer
{
	int data[];
} lightIndicesBuffer;

layout(std430, binding = 2) readonly buffer LightTilesBuffer
{
	LightTile data[];
} lightTilesBuffer;

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

vec3 ProcessPointLight(Light point, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
	float pointDistance = length(point.pos - fragPos);
	float falloffExponent = point.useCustomFalloff * point.falloffExponent + (1 - point.useCustomFalloff) * 4.0;
	float distAttenuation = clamp(1.0 - pow(pointDistance / point.radius, falloffExponent), 0.0, 1.0);
	distAttenuation = point.useCustomFalloff * distAttenuation + (1 - point.useCustomFalloff) * distAttenuation * distAttenuation / (pointDistance * pointDistance + 1.0);

	vec3 pointDir = normalize(point.pos - fragPos);

	float NL = max(dot(fragNormal, pointDir), 0.0);
	float NV = max(dot(fragNormal, viewDir), 0.0) + EPSILON;
	vec3 H = (pointDir + viewDir) / length(pointDir + viewDir);
	float NH = max(dot(fragNormal, H), 0.0);
	float LH = max(dot(pointDir, H), 0.0);

	vec3 Fn = SchlickFresnel(F0, LH);
	float V = SmithVisibility(NL, NV, roughness);
	float NDF = GGXNormalDistribution(NH, roughness);

	// Cook-Torrance BRDF
	return (Cd * (1 - F0) + 0.25 * Fn * V * NDF) * point.color * point.intensity * distAttenuation * NL;
}

vec3 ProcessSpotLight(Light spot, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
	float spotDistance = length(spot.pos - fragPos);
	float falloffExponent = spot.useCustomFalloff * spot.falloffExponent + (1 - spot.useCustomFalloff) * 4.0;
	float distAttenuation = clamp(1.0 - pow(spotDistance / spot.radius, falloffExponent), 0.0, 1.0);
	distAttenuation = spot.useCustomFalloff * distAttenuation + (1 - spot.useCustomFalloff) * distAttenuation * distAttenuation / (spotDistance * spotDistance + 1.0);

	vec3 spotDir = normalize(spot.pos - fragPos);

	vec3 aimDir = normalize(spot.direction);
	float C = dot(aimDir, -spotDir);
	float cAttenuation = 0;
	float cosInner = cos(spot.innerAngle);
	float cosOuter = cos(spot.outerAngle);
	if (C > cosInner)
	{
		cAttenuation = 1;
	}
	else if (cosInner > C && C > cosOuter)
	{
		cAttenuation = (C - cosOuter) / (cosInner - cosOuter);
	}

	float NL = max(dot(fragNormal, spotDir), 0.0);
	float NV = max(dot(fragNormal, viewDir), 0.0) + EPSILON;
	vec3 H = (spotDir + viewDir) / length(spotDir + viewDir);
	float NH = max(dot(fragNormal, H), 0.0);
	float LH = max(dot(spotDir, H), 0.0);

	vec3 Fn = SchlickFresnel(F0, LH);
	float V = SmithVisibility(NL, NV, roughness);
	float NDF = GGXNormalDistribution(NH, roughness);

	// Cook-Torrance BRDF
	return (Cd * (1 - F0) + 0.25 * Fn * V * NDF) * spot.color * spot.intensity * distAttenuation * cAttenuation * NL;
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
		colorAccumulative += shadow * ProcessDirectionalLight(dirLight, normal, viewDir, Cd, F0, roughness);
	}

	float shadowFake = 1.0;

	if(shadow < 1.0){
		shadowFake = shadow + (1.0 - shadow) * 0.35;
	}
	
	// Lights
	int tileIndex = GetTileIndex();
	LightTile lightTile = lightTilesBuffer.data[tileIndex];
	for (uint i = 0; i < lightTile.count; i++ )
    {
		uint lightIndex = lightIndicesBuffer.data[lightTile.offset + i];
		Light light = lightBuffer.data[lightIndex];
		if (light.isSpotLight == 1)
		{
    		colorAccumulative += ProcessSpotLight(light, normal, viewDir, Cd, F0, roughness);
		}
		else
		{
    		colorAccumulative += ProcessPointLight(light, normal, viewDir, Cd, F0, roughness);
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
        colorAccumulative += shadow * ProcessDirectionalLight(dirLight, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
    }
	
	float shadowFake = 1.0;

	if(shadow < 1.0){
		shadowFake = shadow + (1.0 - shadow) * 0.35;
	}

	// Lights
	int tileIndex = GetTileIndex();
	LightTile lightTile = lightTilesBuffer.data[tileIndex];
	for (uint i = 0; i < lightTile.count; i++ )
    {
		uint lightIndex = lightIndicesBuffer.data[lightTile.offset + i];
		Light light = lightBuffer.data[lightIndex];
		if (light.isSpotLight == 1)
		{
    		colorAccumulative += ProcessSpotLight(light, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
		}
		else
		{
    		colorAccumulative += ProcessPointLight(light, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
		}
	}

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;

    outColor = vec4(colorAccumulative, colorDiffuse.a);
}