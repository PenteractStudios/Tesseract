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


--- fragDepthPrepassConvertTextures

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;

in vec2 uv;

uniform int samplesNumber;

uniform sampler2DMS positions;
uniform sampler2DMS normals;

void main()
{
    ivec2 positionsSize = textureSize(positions);
    ivec2 vp = ivec2(vec2(positionsSize) * uv);
    int minIndex = 0;
    float minDepth = texelFetch(positions, vp, 0).z;
    for (int i = 1; i < samplesNumber; ++i) {
        float depth = texelFetch(positions, vp, i).z;
        if (depth < minDepth) {
            minIndex = i;
            minDepth = depth;
        }
    }
    position = texelFetch(positions, vp, minIndex);
    normal = texelFetch(normals, vp, minIndex);
}