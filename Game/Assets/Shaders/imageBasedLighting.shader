--- fragFunctionIBL

#define PI 3.1415926538

#define NUM_SAMPLES 4096

vec2 CartesianToEquirectangular(in vec3 dir) {
	float phi = atan(dir.z, dir.x);
	phi = phi / (2.0 * PI) + 0.5;

	float theta = asin(dir.y);
	theta = theta / PI + 0.5;

	return vec2(phi, theta);
}

vec3 HemisphereSample(float u1, float u2) {
	float phi = u1 * 2.0 * PI;
	float sinTheta = sqrt(u2);

	// spherical to cartesian conversion
	vec3 dir;
	dir.x = cos(phi) * sinTheta;
	dir.y = sin(phi) * sinTheta;
	dir.z = sqrt(1.0 - u2);
	return dir;
}

vec3 HemisphereSampleGGX(float u1, float u2, float roughness) {
    float a = roughness * roughness;

	float phi = u1 * 2.0 * PI;
	float cosTheta = sqrt((1.0 - u2) / (u2 * (a * a - 1.0) + 1.0));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
	// spherical to cartesian conversion
	vec3 dir;
	dir.x = cos(phi) * sinTheta;
	dir.y = sin(phi) * sinTheta;
	dir.z = cosTheta;
	return dir;
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

float Pow2(float a)
{
	return a * a;
}

float GGXNormalDistribution(float NH, float roughness)
{
	return roughness * roughness / (PI * Pow2(NH * NH * (roughness * roughness - 1) + 1));
}

float SmithVisibility(float NL, float NV, float roughness)
{
	return 0.5 / (NL * (NV * (1 - roughness) + roughness) + NV * (NL * (1 - roughness) + roughness));
}

--- fragHDRToCubemap

uniform sampler2D hdr;

in vec3 texcoords;

out vec4 fragColor;

void main() {
	vec3 dir = normalize(texcoords);
	vec2 uv = CartesianToEquirectangular(dir);
	fragColor = texture(hdr, uv);
}

--- fragIrradianceMap

uniform samplerCube environment;

in vec3 texcoords;

out vec4 fragColor;

void main() {
	vec3 irradiance = vec3(0.0);
	vec3 normal = normalize(texcoords);
	mat3 tangentSpace = ComputeTangentSpace(normal);
	for(int i = 0; i < NUM_SAMPLES; ++i) {
		vec2 randValue = Hammersley2D(i, NUM_SAMPLES);
		vec3 sampleDir = tangentSpace * HemisphereSample(randValue[0], randValue[1]);
		irradiance += texture(environment, sampleDir).rgb;
	}
	fragColor = vec4(irradiance * (1.0 / float(NUM_SAMPLES)), 1.0);
}

--- fragPreFilteredMap

uniform samplerCube environment;
uniform float environmentResolution;
uniform float roughness;

in vec3 texcoords;

out vec4 fragColor;

void main() {
	vec3 R = normalize(texcoords);
	vec3 N = R, V = R;
	vec3 color = vec3(0.0);
	float weight = 0.0f;
	mat3 tangentSpace = ComputeTangentSpace(N);
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		vec2 randValue = Hammersley2D(i, NUM_SAMPLES);
		vec3 H = normalize(tangentSpace * HemisphereSampleGGX(randValue[0], randValue[1], roughness));
		vec3 L = reflect(-V, H);
		float NL = max(dot(N, L), 0.0);
		float NH = max(dot(N, H), 0.0);
		float VH = max(dot(V, H), 0.0);
		if (NL > 0) {
			float D = GGXNormalDistribution(NH, roughness);
			float pdf = (D * NH / (4.0 * VH)) + 0.0001;
			float saTexel  = 4.0 * PI / (6.0 * environmentResolution * environmentResolution);
			float saSample = 1.0 / (float(NUM_SAMPLES) * pdf + 0.0001);
			float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
			color += textureLod(environment, L, mipLevel).rgb * NL;
			weight += NL;
		}
	}
	fragColor = vec4(color / weight, 1.0);
}

--- fragEnvironmentBRDF

in vec2 uv;

out vec4 fragColor;

void main() {
	float NV = uv.x, roughness = uv.y;
	vec3 V = vec3(sqrt(1.0 - NV * NV), 0.0, NV);
	vec3 N = vec3(0.0, 0.0, 1.0);
	float fa = 0.0;
	float fb = 0.0;
	mat3 tangentSpace = ComputeTangentSpace(N);
	for (int i = 0; i < NUM_SAMPLES; i++) {
		vec2 randValue = Hammersley2D(i, NUM_SAMPLES);
		vec3 H = normalize(tangentSpace * HemisphereSampleGGX(randValue[0], randValue[1], roughness));
		vec3 L = reflect(-V, H);
		float NL = max(dot(N, L), 0.0);
		float NH = max(dot(N, H), 0.0);
		float VH = max(dot(V, H), 0.0);
		if (NL > 0.0) {
			float VPDF = SmithVisibility(NL, NV, roughness) * VH * NL / NH;
			float Fc = pow(1.0 - VH, 5.0);
			fa += (1.0 - Fc) * VPDF;
			fb += Fc * VPDF;
		}
	}
	fragColor = vec4(4.0 * fa / float(NUM_SAMPLES), 4.0 * fb / float(NUM_SAMPLES), 1.0, 1.0);
}