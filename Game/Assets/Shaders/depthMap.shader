--- vertDepthMap

#define MAX_BONES 100

in layout(location=0) vec3 pos;
in layout(location=1) vec3 norm;
in layout(location=3) vec2 uvs;
in layout(location=4) uvec4 boneIndices;
in layout(location=5) vec4 boneWeitghts;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 palette[MAX_BONES];
uniform bool hasBones;

out vec2 uv;

void main() {

    vec4 position = vec4(pos, 1.0);
    vec4 normal = vec4(norm, 0.0);

    if (hasBones)
    { 
        mat4 skinT = palette[boneIndices[0]] * boneWeitghts[0] + palette[boneIndices[1]] * boneWeitghts[1]
        + palette[boneIndices[2]] * boneWeitghts[2] + palette[boneIndices[3]] * boneWeitghts[3];

        position = skinT * vec4(pos, 1.0);
        normal = skinT * vec4(norm, 0.0);
    }
    uv = uvs;
    gl_Position = proj * view * model * position;
}

--- fragDepthMap

in vec2 uv;

// Diffuse Material
uniform vec4 diffuseColor;
uniform sampler2D diffuseMap;
uniform int hasDiffuseMap;
uniform vec2 tiling;
uniform vec2 offset;

void main() {
    float diffuseAlpha = (hasDiffuseMap * pow(texture(diffuseMap, uv * tiling + offset), vec4(2.2)) + (1 - hasDiffuseMap) * diffuseColor).a;
    if(diffuseAlpha < 0.78)  // Tweakable value
    {
        discard;
    }
	// gl_FragDepth = gl_FragCoord.z;
}