--- fragSkybox

in vec3 texcoords;

uniform samplerCube cubemap;

out vec4 outColor;

void main() {
	outColor = texture(cubemap, texcoords);
}