--- fragSSAO

#define KERNEL_SIZE 64
#define RANDOM_TANGENTS_ROWS 4
#define RANDOM_TANGENTS_COLS 4

uniform mat4 proj;

uniform sampler2D positions;
uniform sampler2D normals;
uniform vec3 kernelSamples[KERNEL_SIZE];
uniform vec3 randomTangents[RANDOM_TANGENTS_ROWS * RANDOM_TANGENTS_COLS];
uniform vec2 screenSize;
uniform float bias;
uniform float range;

in vec2 uv;

layout(location = 0) out vec4 result;

vec3 GetRandomTangent() {
    vec2 screenPos = uv * screenSize;
    ivec2 index = ivec2(int(mod(screenPos.y, RANDOM_TANGENTS_ROWS)), int(mod(screenPos.x, RANDOM_TANGENTS_COLS)));
    return randomTangents[index.x * RANDOM_TANGENTS_ROWS + index.y];
}

mat3 CreateTangentSpace(const vec3 normal, const vec3 randomTangent) {
    vec3 tangent = normalize(randomTangent - normal * dot(normal, randomTangent)); // Gram-Schmidt
    vec3 bitangent = normalize(cross(tangent, normal));
    return mat3(bitangent, tangent, normal);
}

float GetSceneDepthAtSamplePos(const vec3 samplePos) {
    vec4 clippingSpace = proj * vec4(samplePos, 1.0);
    vec2 sampleUV = (clippingSpace.xy / clippingSpace.w) * 0.5 + 0.5;
    return texture(positions, sampleUV).z;
}

void main() {
    vec3 position = texture(positions, uv).xyz;
    vec3 normal = normalize(texture(normals, uv).xyz);
    mat3 tangentSpace = CreateTangentSpace(normal, GetRandomTangent());
    int occlusion = 0;
    for (int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 samplePos = position + tangentSpace * kernelSamples[i];
        float sampleDepth = GetSceneDepthAtSamplePos(samplePos);
        if (sampleDepth + bias > samplePos.z && abs(sampleDepth - position.z) < range) {
            ++occlusion;
        }
    }
    result = vec4(vec3(1.0 - float(occlusion) / float(KERNEL_SIZE)), 1.0f);
}