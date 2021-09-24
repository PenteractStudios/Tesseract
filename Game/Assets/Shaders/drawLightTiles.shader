--- fragDrawLightTiles

in vec2 uv;

out vec4 color;

uniform sampler2D sceneTexture;

void main() {
	ivec2 tile = ivec2(gl_FragCoord.xy) / ivec2(LIGHT_TILE_SIZE, LIGHT_TILE_SIZE);
	int index = tile.x + tile.y * tilesPerRow;
	float pointRatio = float(lightTiles.data[index].numPoints) / float(POINT_LIGHTS);
	float spotRatio = float(lightTiles.data[index].numSpots) / float(SPOT_LIGHTS);
	color = vec4(vec3(0.0, spotRatio, 0.0), 1.0);
}