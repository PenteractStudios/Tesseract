#ifdef VERTEX

#define MAX_BONES 100

in layout(location=0) vec3 pos;
in layout(location=1) vec3 norm;
in layout(location=2) vec2 uvs;
in layout(location=3) uvec4 boneIndices;
in layout(location=4) vec4 boneWeitghts;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 palette[MAX_BONES];
uniform bool hasBones;

out vec3 fragNormal;
out vec3 fragPos;
out vec2 uv;

void main()
{
    vec4 position;
    vec4 normal;
    if (hasBones) {
        mat4 skinT = palette[boneIndices[0]] * boneWeitghts[0] + palette[boneIndices[1]] * boneWeitghts[1]
        + palette[boneIndices[2]] * boneWeitghts[2] + palette[boneIndices[3]] * boneWeitghts[3];
        position = skinT * vec4(pos, 1.0);
        normal = skinT * vec4(norm, 0.0);
    }
    else {
        position = vec4(pos, 1.0);
        normal = vec4(norm, 0.0);
    }

    gl_Position = proj * view * model * position;
    fragNormal = normalize(transpose(inverse(mat3(model))) * normal.xyz);
    fragPos = vec3(model * position);
    uv = uvs;
}
#endif

#ifdef FRAGMENT

#define PI 3.1415926538

in vec3 fragNormal;
in vec3 fragPos;
in vec2 uv;
out vec4 outColor;

// Material
uniform vec3 diffuseColor;
uniform sampler2D diffuseMap;
uniform float metalness;
uniform sampler2D metallicMap;

uniform int hasDiffuseMap;
uniform int hasMetallicMap;
uniform int hasMetallicAlpha; // else: Diffuse alpha channel
uniform float smoothness;

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
uniform vec3 viewPos;

float Pow2(float a)
{
    return a * a;
}

float GGXNormalDistribution(float NH, float roughness)
{
    return roughness * roughness / (PI * Pow2(NH * NH * (roughness * roughness - 1) + 1));
}

vec3 ProcessDirectionalLight(DirLight directional, vec3 fragNormal, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
    vec3 directionalDir = - normalize(directional.direction);

    float NL = max(dot(fragNormal, directionalDir), 0.0);
    float NV = max(dot(fragNormal, viewDir), 0.0);
    vec3 H = (directionalDir + viewDir) / length(directionalDir + viewDir);
    float NH = max(dot(fragNormal, H), 0.0);
    float LH = max(dot(directionalDir, H), 0.0);

    // Schlick Fresnel
    vec3 Fn = F0 + (1 - F0) * pow(1 - LH, 5); 

    // Smith Visibility Function
    float V = 0.5 / (NL * (NV * (1 - roughness) + roughness) + NV * (NL * (1 - roughness) + roughness));

    float NDF = GGXNormalDistribution(NH, roughness);

   	// Cook-Torrance BRDF
    return (Cd * (1 - F0) + 0.25 * Fn * V * NDF) * directional.color * directional.intensity * NL;
}

vec3 ProcessPointLight(PointLight point, vec3 fragNormal, vec3 fragPos, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
    float pointDistance = length(point.pos - fragPos);
    float distAttenuation = 1.0 / (point.kc + point.kl * pointDistance + point.kq * pointDistance * pointDistance);

    vec3 pointDir = normalize(point.pos - fragPos);

    float NL = max(dot(fragNormal, pointDir), 0.0);
    float NV = max(dot(fragNormal, viewDir), 0.0);
    vec3 H = (pointDir + viewDir) / length(pointDir + viewDir);
    float NH = max(dot(fragNormal, H), 0.0);
    float LH = max(dot(pointDir, H), 0.0);

    // Schlick Fresnel
    vec3 Fn = F0 + (1 - F0) * pow(1 - LH, 5); 

    // Smith Visibility Function
    float V = 0.5 / (NL * (NV * (1 - roughness) + roughness) + NV * (NL * (1 - roughness) + roughness));

    float NDF = GGXNormalDistribution(NH, roughness);

   	// Cook-Torrance BRDF
    return (Cd * (1 - F0) + 0.25 * Fn * V * NDF) * point.color * point.intensity * distAttenuation * NL;
}

vec3 ProcessSpotLight(SpotLight spot, vec3 fragNormal, vec3 fragPos, vec3 viewDir, vec3 Cd, vec3 F0, float roughness)
{
    float spotDistance = length(spot.pos - fragPos);
	float distAttenuation = 1.0 / (spot.kc + spot.kl * spotDistance + spot.kq * spotDistance * spotDistance);

    vec3 spotDir = normalize(spot.pos - fragPos);

    vec3 aimDir = normalize(spot.direction);
    float C = dot( aimDir , -spotDir);
    float cAttenuation = 0;
    float cosInner = cos(spot.innerAngle);
    float cosOuter = cos(spot.outerAngle);
    if( C > cosInner)
    {
        cAttenuation = 1;
    }
    else if (cosInner > C && C > cosOuter)
    {
    	cAttenuation = (C - cosOuter) / (cosInner - cosOuter);
    }

    float NL = max(dot(fragNormal, spotDir), 0.0);
    float NV = max(dot(fragNormal, viewDir), 0.0);
    vec3 H = (spotDir + viewDir) / length(spotDir + viewDir);
    float NH = max(dot(fragNormal, H), 0.0);
    float LH = max(dot(spotDir, H), 0.0);

    // Schlick Fresnel
    vec3 Fn = F0 + (1 - F0) * pow(1 - LH, 5); 

    // Smith Visibility Function
    float V = 0.5 / (NL * (NV * (1 - roughness) + roughness) + NV * (NL * (1 - roughness) + roughness));

    float NDF = GGXNormalDistribution(NH, roughness);

   	// Cook-Torrance BRDF
    return (Cd * (1 - F0) + 0.25 * Fn * V * NDF) * spot.color * spot.intensity * distAttenuation * cAttenuation * NL;
}

void main()
{    
    vec3 viewDir = normalize(viewPos - fragPos);
    vec4 colorDiffuse = hasDiffuseMap * pow(texture(diffuseMap, uv), vec4(2.2)) * vec4(diffuseColor, 0.0) + (1 - hasDiffuseMap) * vec4(diffuseColor, 0.0);
    vec4 colorMetallic = pow(texture(metallicMap, uv), vec4(2.2));
    float metalnessMask = hasMetallicMap * colorMetallic.r + (1 - hasMetallicMap) * metalness;

    float roughness = Pow2(1 - smoothness * (hasMetallicAlpha * colorMetallic.a + (1 - hasMetallicAlpha) * colorDiffuse.a));

    vec3 colorAccumulative = colorDiffuse.rgb * light.ambient.color;

    // Schlick Fresnel
    vec3 Cd = colorDiffuse.rgb * (1 - metalnessMask);
    vec3 F0 = mix(vec3(0.04), colorDiffuse.rgb, metalnessMask);

    // Directional Light
    if (light.directional.isActive == 1)
    {
    	colorAccumulative += ProcessDirectionalLight(light.directional, fragNormal, viewDir, Cd, F0, roughness);
    }

	// Point Light
	for(int i = 0; i < light.numPoints; i++)
	{
    	colorAccumulative += ProcessPointLight(light.points[i], fragNormal, fragPos, viewDir, Cd, F0, roughness);
	}

    // Spot Light
	for(int i = 0; i < light.numSpots; i++)
	{
    	colorAccumulative += ProcessSpotLight(light.spots[i], fragNormal, fragPos, viewDir, Cd, F0, roughness);
	}

    vec3 ldr = colorAccumulative.rgb / (colorAccumulative.rgb + vec3(1.0)); // reinhard tone mapping
	ldr = pow(ldr, vec3(1/2.2)); // gamma correction
	outColor = vec4(ldr, 1.0);
}
#endif