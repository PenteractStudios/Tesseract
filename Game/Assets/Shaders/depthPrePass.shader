--- fragDepthPrePass

out layout(location = 0) vec4 pos;
out layout(location = 1) vec4 norm;

uniform mat4 view;

in vec3 fragPos;
in vec3 fragNormal;

void main() {
    pos = view * vec4(fragPos, 1.0);
    norm = vec4(mat3(view) * normalize(fragNormal), 1.0);
}