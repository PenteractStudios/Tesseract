--- fragVarStandard

#define PI 3.1415926538
#define EPSILON 1e-5
#define POINT_LIGHTS 32
#define SPOT_LIGHTS 8

in vec3 fragNormal;
in mat3 TBN;
in vec3 fragPos;
in vec2 uv;
in vec4 fragPosLight;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 brightColor;

// Depth Map
uniform sampler2D depthMapTexture;

// SSAO texture
uniform sampler2D ssaoTexture;

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
uniform sampler2D ambientOcclusionMap;
uniform int hasAmbientOcclusionMap;
uniform int hasSmoothnessAlpha; // Generic used for Specular and Metallic
uniform vec2 tiling;
uniform vec2 offset;

// IBL
uniform samplerCube diffuseIBL;
uniform samplerCube prefilteredIBL;
uniform sampler2D environmentBRDF;
uniform int prefilteredIBLNumLevels;

struct AmbientLight
{
	vec3 color;
};

struct DirLight
{
	vec3 direction;
	vec3 color;
	float intensity;
	int isActive;
};

struct PointLight
{
	vec3 pos;
	vec3 color;
	float intensity;
	float kc;
	float kl;
	float kq;
};

struct SpotLight
{
	vec3 pos;
	vec3 direction;
	vec3 color;
	float intensity;
	float kc;
	float kl;
	float kq;
	float innerAngle;
	float outerAngle;
};

struct Light
{
	AmbientLight ambient;
	DirLight directional;
	PointLight points[POINT_LIGHTS];
	int numPoints;
	SpotLight spots[SPOT_LIGHTS];
	int numSpots;
};

uniform Light light;

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
    return hasDiffuseMap * pow(texture(diffuseMap, tiledUV), vec4(2.2)) * diffuseColor + (1 - hasDiffuseMap) * diffuseColor;
}

vec4 GetEmissive(vec2 tiledUV)
{
    return hasEmissiveMap * pow(texture(emissiveMap, tiledUV), vec4(2.2));
}

vec3 GetAmbientLight(in vec3 R, in vec3 normal, in vec3 viewDir, in vec3 Cd, in vec3 F0, float roughness)
{
	float NV = max(dot(fragNormal, viewDir), 0.0) + EPSILON;
	vec3 irradiance = texture(diffuseIBL, normal).rgb;
	vec3 radiance = textureLod(prefilteredIBL, R, roughness * prefilteredIBLNumLevels).rgb;
	vec2 fab = texture(environmentBRDF, vec2(NV, roughness)).rg;
	vec3 diffuse = (Cd * (1 - F0));
	return diffuse * irradiance + radiance * (F0 * fab.x + fab.y);
}

vec3 GetOccludedAmbientLight(in vec3 R, in vec3 normal, in vec3 viewDir, in vec3 Cd, in vec3 F0, float roughness, vec2 tiledUV)
{
	vec4 projectedPos = proj * view * vec4(fragPos, 1.0);
	vec2 occlusionUV = (projectedPos.xy / projectedPos.w) * 0.5 + 0.5;
	float occlusionFactor = texture(ssaoTexture, occlusionUV).r;
	vec3 ambientLight = GetAmbientLight(R, normal, viewDir, Cd, F0, roughness) * occlusionFactor;
    return hasAmbientOcclusionMap * ambientLight.rgb * texture(ambientOcclusionMap, tiledUV).rgb + (1 - hasAmbientOcclusionMap) * ambientLight.rgb;
}

vec3 GetNormal(vec2 tiledUV)
{

    vec3 normal = texture(normalMap, tiledUV).rgb;
    normal = normal * 2.0 - 1.0;
    normal.xy *= normalStrength;
    normal = normalize(normal);
    return normalize(TBN * normal);
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
	float bias = max(0.05 * (1 - dot(normal, lightDirection)), 0.005);

	float shadow = 0.0;

	vec2 texelSize = 1.0/textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x){
		for(int y = -1; y <= 1; ++y){
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;

	return shadow;

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
	return roughness * roughness / (PI * Pow2(NH * NH * (roughness * roughness - 1) + 1));
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

vec3 ProcessPointLight(PointLight point, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
	float pointDistance = length(point.pos - fragPos);
	float distAttenuation = 1.0 / (point.kc + point.kl * pointDistance + point.kq * pointDistance * pointDistance);

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

vec3 ProcessSpotLight(SpotLight spot, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
	float spotDistance = length(spot.pos - fragPos);
	float distAttenuation = 1.0 / (spot.kc + spot.kl * spotDistance + spot.kq * spotDistance * spotDistance);

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
    vec4 colorMetallic = pow(texture(metallicMap, tiledUV), vec4(2.2));
    float metalnessMask = hasMetallicMap * colorMetallic.r + (1 - hasMetallicMap) * metalness;

	float roughness = Pow2(1 - smoothness * (hasSmoothnessAlpha * colorMetallic.a + (1 - hasSmoothnessAlpha) * colorDiffuse.a)) + EPSILON;

	// Schlick Fresnel
	vec3 Cd = colorDiffuse.rgb * (1 - metalnessMask);
	vec3 F0 = mix(vec3(0.04), colorDiffuse.rgb, metalnessMask);

    // Ambient Light
    vec3 R = reflect(-viewDir, normal);
    vec3 colorAccumulative = GetOccludedAmbientLight(R, normal, viewDir, Cd, F0, roughness, tiledUV);

	float shadow = Shadow(fragPosLight, normal,  normalize(light.directional.direction), depthMapTexture);

	// Directional Light
	if (light.directional.isActive == 1)
	{
		colorAccumulative += (1 - shadow) * ProcessDirectionalLight(light.directional, normal, viewDir, Cd, F0, roughness);
	}

	// Point Light
	for (int i = 0; i < light.numPoints; i++)
	{
    	colorAccumulative += ProcessPointLight(light.points[i], normal, viewDir, Cd, F0, roughness);
	}

	// Spot Light
	for (int i = 0; i < light.numSpots; i++)
	{
    	colorAccumulative += ProcessSpotLight(light.spots[i], normal, viewDir, Cd, F0, roughness);
	}

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;

	// Bloom
	outColor = vec4(colorAccumulative, colorDiffuse.a);
	float brightness = dot(outColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0) brightColor = outColor;
	else brightColor = vec4(0.0, 0.0, 0.0, 1.0);
    /*vec3 ldr = colorAccumulative.rgb / (colorAccumulative.rgb + vec3(1.0)); // reinhard tone mapping
    ldr = pow(ldr, vec3(1/2.2)); // gamma correction
    outColor = vec4(ldr, colorDiffuse.a);*/
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
    vec4 colorSpecular = hasSpecularMap * pow(texture(specularMap, tiledUV), vec4(2.2)) + (1 - hasSpecularMap) * vec4(specularColor, 1.0);

    float roughness = Pow2(1 - smoothness * (hasSmoothnessAlpha * colorSpecular.a + (1 - hasSmoothnessAlpha) * colorDiffuse.a)) + EPSILON;

    // Ambient Light
    vec3 R = reflect(-viewDir, normal);
    vec3 colorAccumulative = GetOccludedAmbientLight(R, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness, tiledUV);

	float shadow = Shadow(fragPosLight, normal, normalize(light.directional.direction), depthMapTexture);

    // Directional Light
    if (light.directional.isActive == 1)
    {
        colorAccumulative += (1 - shadow) * ProcessDirectionalLight(light.directional, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
    }

	// Point Light
    for(int i = 0; i < light.numPoints; i++)
    {
        colorAccumulative += ProcessPointLight(light.points[i], normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
    }

    // Spot Light
    for(int i = 0; i < light.numSpots; i++)
    {
        colorAccumulative += ProcessSpotLight(light.spots[i], normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
    }

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;

	// Bloom
	outColor = vec4(colorAccumulative, colorDiffuse.a);
	float brightness = dot(outColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0) brightColor = outColor;
	else brightColor = vec4(0.0, 0.0, 0.0, 1.0);
    /*vec3 ldr = colorAccumulative.rgb / (colorAccumulative.rgb + vec3(1.0)); // reinhard tone mapping
    ldr = pow(ldr, vec3(1/2.2)); // gamma correction
    outColor = vec4(ldr, colorDiffuse.a);*/
}