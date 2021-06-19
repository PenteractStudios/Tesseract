--- fragDepthPrepass

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;

uniform mat4 view;

in vec3 fragPos;
in vec3 fragNormal;
in vec2 uv;

uniform vec4 diffuseColor;
uniform sampler2D diffuseMap;
uniform int hasDiffuseMap;
uniform vec2 tiling;
uniform vec2 offset;

vec2 GetTiledUVs()
{
    return uv * tiling + offset; 
}

vec4 GetDiffuse(vec2 tiledUV)
{
    return hasDiffuseMap * pow(texture(diffuseMap, tiledUV), vec4(2.2)) * diffuseColor + (1 - hasDiffuseMap) * diffuseColor;
}

void main() {
    vec2 tiledUV = GetTiledUVs(); 
    vec4 colorDiffuse = GetDiffuse(tiledUV);
	if (colorDiffuse.a < 0.1) discard; // Alpha testing
    position = view * vec4(fragPos, 1.0);
    normal = vec4(mat3(view) * normalize(fragNormal), 1.0);
}