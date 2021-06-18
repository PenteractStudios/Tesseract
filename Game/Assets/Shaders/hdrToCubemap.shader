--- fragHDRToCubemap

#define PI 3.1415926538

uniform sampler2D hdri;
uniform int face;

in vec2 uv;

out vec4 color;

vec2 CartesianToEquirectangular(in vec3 dir) {
	float phi = atan(dir.z, dir.x);
	phi = phi / (2.0 * PI) + 0.5;

	float theta = asin(dir.y);
	theta = theta / PI + 0.5;

	return vec2(phi, theta);
}

vec3 UVToXYZ(int face, vec2 uv) {
	if (face == 0) return vec3(1.0, uv.y, -uv.x);
	else if (face == 1) return vec3(-1.0, uv.y, uv.x);
	else if(face == 2) return vec3(uv.x, -1.0, uv.y);
	else if(face == 3) return vec3(uv.x, 1.0, -uv.y);
	else if(face == 4) return vec3(uv.x, uv.y, 1.0);
	else return vec3(-uv.x, uv.y, -1.0);
}

void main() {
	vec2 newUV = uv * 2.0 - 1.0;
	vec3 xyz = UVToXYZ(face, newUV); 
	vec3 dir = normalize(UVToXYZ(face, newUV));
	vec2 rectUV = CartesianToEquirectangular(dir);
	color = texture(hdri, rectUV);
}