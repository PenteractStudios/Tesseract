--- fragDrawTexture

in vec2 uv;

out vec4 color;

uniform sampler2D textureToDraw;

void main() {
	color = texture(textureToDraw, uv);
}