--- vertImageUI

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV0;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec2 uv0;

void main()
{
	gl_Position = proj*view*model*vec4(vertexPosition.xy, 0.0 , 1.0);
	uv0 = vertexUV0;
}

--- fragImageUI

in vec2 uv0;

uniform sampler2D diffuse;
uniform int hasDiffuse;
uniform vec4 inputColor;

out vec4 outColor;

void main()
{	
	outColor = (hasDiffuse * SRGB(texture2D(diffuse, uv0)) + 1 - hasDiffuse) * SRGB(inputColor);
}