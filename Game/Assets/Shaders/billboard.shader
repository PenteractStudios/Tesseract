--- billboardVertex

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUV0;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec2 uv0;


void main()
{
	gl_Position = proj*view*model*vec4(vertexPosition.xyz , 1.0);
	uv0 = vertexUV0;
}

--- billboardFragment

in vec2 uv0;

uniform sampler2D diffuseMap;
uniform int hasDiffuseMap;
uniform vec4 inputColor;

uniform float currentFrame; 
uniform int Xtiles;
uniform int Ytiles;

uniform bool flipX;
uniform bool flipY;

float X;
float Y;
float u;
float v;
out vec4 outColor;

void main()
{	
	vec2 uvs = uv0;
	u = uvs.x;
	v = uvs.y;
	
	if (flipX) 
	{
		uvs.x = 1 - uv0.x;
		u = uvs.x;
	}

	if (flipY) 
	{
		uvs.y = 1 - uv0.y;
		v = uvs.y;
	}

	X = trunc(mod(currentFrame, Xtiles));
	Y = trunc(currentFrame/Xtiles);
	Y = (Ytiles - 1) - Y ;
	X = mix(X,X+1,u);
	Y = mix(Y,Y+1,v);
	
	u = X/Xtiles;
	v = Y/Ytiles;
	
	outColor = SRGBA(inputColor) * (hasDiffuseMap * SRGBA(texture2D(diffuseMap,  vec2(u, v))) + (1 - hasDiffuseMap));
}

