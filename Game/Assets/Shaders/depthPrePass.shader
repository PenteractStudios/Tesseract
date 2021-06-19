--- fragDepthPrepass

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;

uniform mat4 view;

in vec3 fragPos;
in vec3 fragNormal;

void main() {
    position = view * vec4(fragPos, 1.0);
    normal = vec4(mat3(view) * normalize(fragNormal), 1.0);
}