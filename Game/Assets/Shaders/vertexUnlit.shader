--- vertUnlit

in layout(location=0) vec3 pos;
in layout(location=1) vec3 norm;
in layout(location=2) vec3 tangent;
in layout(location=3) vec2 uvs;
in layout(location=4) uvec4 boneIndices;
in layout(location=5) vec4 boneWeitghts;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 fragPos;
out vec2 uv;

void main()
{
    vec4 position = vec4(pos, 1.0);
    vec4 normal = vec4(norm, 0.0);

    gl_Position = proj * view * model * position;
    fragPos = vec3(model * position);

    uv = uvs;
}