--- fragDrawLightTiles

in vec2 uv;

out vec4 color;

layout(std430, binding = 0) readonly buffer LightTilesBuffer
{
	LightTile data[];
} lightTilesBuffer;

uniform int tilesPerRow;

uniform sampler2D sceneTexture;

void main() {
	ivec2 tile = ivec2(gl_FragCoord.xy) / ivec2(LIGHT_TILE_SIZE, LIGHT_TILE_SIZE);
	int index = tile.x + tile.y * tilesPerRow;
	float lightRatio = float(lightTilesBuffer.data[index].count) / float(MAX_LIGHTS_PER_TILE);
	color = vec4(vec3(lightRatio), 1.0);
}