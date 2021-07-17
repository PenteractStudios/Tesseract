--- fragUnlit

in vec2 uv;
out vec4 outColor;

// Material
uniform vec4 diffuseColor;
uniform sampler2D diffuseMap;
uniform int hasDiffuseMap;
uniform sampler2D emissiveMap;
uniform int hasEmissiveMap;
uniform float emissiveIntensity;

uniform vec2 tiling;
uniform vec2 offset;

vec2 GetTiledUVs()
{
    return uv * tiling + offset;
}

vec4 GetDiffuse(vec2 tiledUV)
{
    return hasDiffuseMap * SRGBA(texture(diffuseMap, tiledUV)) * diffuseColor + (1 - hasDiffuseMap) * SRGBA(diffuseColor);
}

vec4 GetEmissive(vec2 tiledUV)
{
    return hasEmissiveMap * SRGBA(texture(emissiveMap, tiledUV)) * emissiveIntensity;
}

void main()
{
    vec2 tiledUV = GetTiledUVs();
    outColor = GetDiffuse(tiledUV) + vec4(GetEmissive(tiledUV).rgb, 0);
}