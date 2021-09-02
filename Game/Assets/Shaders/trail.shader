--- trailVertex

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV0;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec2 uv0;

void main()
{
	gl_Position = proj * view * model * vec4(vertexPosition.xyz, 1.0);
	uv0 = vertexUV0;

}

--- trailFragment

in vec2 uv0;

uniform sampler2D diffuseMap;
uniform vec4 inputColor;
uniform int hasDiffuse;

uniform int flipX;
uniform int flipY;

out vec4 outColor;

void main()
{	
	float u = flipX * (1 - uv0.x) + (1 - flipX) * uv0.x;
	float v = flipY * (1 - uv0.y) + (1 - flipY) * uv0.y;

	outColor = SRGBA(inputColor) * (hasDiffuse * SRGBA(texture2D(diffuseMap, vec2(u, v))) + (1 - hasDiffuse));
}

