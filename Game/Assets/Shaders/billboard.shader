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

uniform sampler2D diffuse;

uniform float currentFrame;
uniform int Xtiles;
uniform int Ytiles;

uniform bool flipX;
uniform bool flipY;

uniform float colorFrame;
uniform vec4 initColor;
uniform vec4 finalColor;
uniform float startTransition;
uniform float endTransition;

float X;
float Y;
float u;
float v;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 brightColor;

void main()
{
	vec2 uvs = uv0;
	if (flipX)
	{
		uvs.x = 1 - uv0.x;
	}

	if (flipY)
	{
		uvs.y = 1 - uv0.y;
	}

	X = trunc(mod(currentFrame,Xtiles));
	Y = trunc(currentFrame/Ytiles);

	X = mix(X,X+1, uvs.x);
	Y = mix(Y,Y+1, uvs.y);
	u = X/Xtiles;
	v = Y/Ytiles;

	if (colorFrame < startTransition)
	{
		outColor = initColor * texture2D(diffuse,  vec2(u, v));
	}
	else if (colorFrame <= endTransition)
	{
		float a = (colorFrame - startTransition) / (endTransition - startTransition);
		outColor = mix(initColor, finalColor, a) * texture2D(diffuse, vec2(u, v));
	}else
	{
		outColor = finalColor * texture2D(diffuse,  vec2(u, v) ) ;
	}

	// Bloom
	float brightness = dot(outColor.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0) brightColor = outColor;
	else brightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

