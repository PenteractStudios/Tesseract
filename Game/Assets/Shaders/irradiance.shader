--- fragIrradiance

#define PI 3.1415926538
#define NUM_SAMPLES 3000

uniform samplerCube environment;

in vec3 texcoords;

out vec4 color;

vec3 HemisphereSample(float u1, float u2) {
	float phi = u1 * 2.0 * PI;
	float r = sqrt(u2);
	return vec3(r * cos(phi), r * sin(phi), sqrt(1 - u2));
}

mat3 ComputeTangentSpace(in vec3 normal) {
	vec3 up = abs(normal.y) > 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up = cross(normal, right);
	return mat3(right, up, normal);
}

float RadicalInverseVdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley2D(uint i, uint N) {
	return vec2(float(i) / float(N), RadicalInverseVdC(i));
}

void main() {
	vec3 irradiance = vec3(0.0);
	vec3 normal = normalize(texcoords);
	mat3 tangentSpace = ComputeTangentSpace(normal);
	for(int i = 0; i < NUM_SAMPLES; ++i) {
		vec2 randValue = Hammersley2D(i, NUM_SAMPLES);
		vec3 sampleDir = tangentSpace * HemisphereSample(randValue[0], randValue[1]);
		irradiance += texture(environment, sampleDir).rgb;
	}
	color = vec4(irradiance * (1.0 / float(NUM_SAMPLES)), 1.0);
}