--- vertUnlit

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

out vec2 uv;

void main()
{
    vec4 position = vec4(pos, 1.0);

    if (hasBones)
    {
        mat4 skinT = palette[boneIndices[0]] * boneWeitghts[0] + palette[boneIndices[1]] * boneWeitghts[1]
            + palette[boneIndices[2]] * boneWeitghts[2] + palette[boneIndices[3]] * boneWeitghts[3];

        position = skinT * vec4(pos, 1.0);
    }

    gl_Position = proj * view * model * position;

    uv = uvs;
}