--- fragVarStandard

#define PI 3.1415926538
#define EPSILON 1e-5

in vec3 fragNormal;
in mat3 TBN;
in vec3 fragPos;
in vec2 uv;
out vec4 outColor;

uniform vec3 viewPos;

// Material
uniform vec3 diffuseColor;
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
    PointLight points[8];
    int numPoints;
    SpotLight spots[8];
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
    return hasDiffuseMap * pow(texture(diffuseMap, tiledUV), vec4(2.2)) * vec4(diffuseColor, 1.0) + (1 - hasDiffuseMap) * vec4(diffuseColor, 1.0);
}

vec4 GetEmissive(vec2 tiledUV)
{
    return hasEmissiveMap * pow(texture(emissiveMap, tiledUV), vec4(2.2));
}

vec3 GetAmbientOcclusion(vec2 tiledUV)
{
    return hasAmbientOcclusionMap * light.ambient.color * texture(ambientOcclusionMap, tiledUV).rgb + (1 - hasAmbientOcclusionMap) * light.ambient.color;
}

vec3 GetNormal(vec2 tiledUV)
{

    vec3 normal = texture(normalMap, tiledUV).rgb;
    normal = normal * 2.0 - 1.0;
    normal.xy *= normalStrength;
    normal = normalize(normal);
    return normalize(TBN * normal);
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

vec3 SchlickFresnel(vec3 F0, float LH)
{
    return F0 + (1 - F0) * pow(1 - LH, 5); 
}

float SmithVisibility(float NL, float NV, float roughness)
{
    return 0.5 / (NL * (NV * (1 - roughness) + roughness) + NV * (NL * (1 - roughness) + roughness));
}

vec3 ProcessDirectionalLight(DirLight directional, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
    vec3 directionalDir = - normalize(directional.direction);

    float NL = max(dot(fragNormal, directionalDir), 0.0);
    float NV = max(dot(fragNormal, viewDir), 0.0)  + EPSILON;
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
    float C = dot( aimDir , -spotDir);
    float cAttenuation = 0;
    float cosInner = cos(spot.innerAngle);
    float cosOuter = cos(spot.outerAngle);
    if ( C > cosInner)
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

    // Ambient Occlusion
    vec3 colorAmbient = GetAmbientOcclusion(tiledUV);

    vec3 colorAccumulative = colorDiffuse.rgb * colorAmbient;

    // Schlick Fresnel
    vec3 Cd = colorDiffuse.rgb * (1 - metalnessMask);
    vec3 F0 = mix(vec3(0.04), colorDiffuse.rgb, metalnessMask);

    // Directional Light
    if (light.directional.isActive == 1)
    {
    	colorAccumulative += ProcessDirectionalLight(light.directional, normal, viewDir, Cd, F0, roughness);
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

    vec3 ldr = colorAccumulative.rgb / (colorAccumulative.rgb + vec3(1.0)); // reinhard tone mapping
	ldr = pow(ldr, vec3(1/2.2)); // gamma correction
	outColor = vec4(ldr, 1.0);
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
    
    // Ambient Occlusion
    vec3 colorAmbient = GetAmbientOcclusion(tiledUV);

    vec3 colorAccumulative = colorDiffuse.rgb * colorAmbient;

    // Directional Light
    if (light.directional.isActive == 1)
    {
        colorAccumulative += ProcessDirectionalLight(light.directional, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness);
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

    vec3 ldr = colorAccumulative.rgb / (colorAccumulative.rgb + vec3(1.0)); // reinhard tone mapping
    ldr = pow(ldr, vec3(1/2.2)); // gamma correction
    outColor = vec4(ldr, 1.0);
}