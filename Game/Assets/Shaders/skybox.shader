--- fragSkybox

in vec3 texcoords;

uniform samplerCube cubemap;

out vec4 outColor;

void main() {
	outColor = pow(texture(cubemap, texcoords), vec4(2.2));
}