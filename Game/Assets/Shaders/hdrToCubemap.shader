--- fragHDRToCubemap

#define PI 3.1415926538

uniform sampler2D hdr;

in vec3 texcoords;

out vec4 color;

vec2 CartesianToEquirectangular(in vec3 dir) {
	float phi = atan(dir.z, dir.x);
	phi = phi / (2.0 * PI) + 0.5;

	float theta = asin(dir.y);
	theta = theta / PI + 0.5;

	return vec2(phi, theta);
}

void main() {
	vec3 dir = normalize(texcoords);
	vec2 uv = CartesianToEquirectangular(dir);
	color = texture(hdr, uv);
}