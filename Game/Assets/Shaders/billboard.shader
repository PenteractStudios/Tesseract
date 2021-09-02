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

uniform float near;
uniform float far;

uniform sampler2D depths;

uniform sampler2D diffuseMap;
uniform int hasDiffuseMap;
uniform vec4 inputColor;
uniform vec3 intensity;

uniform int transparent;

uniform float currentFrame;
uniform int Xtiles;
uniform int Ytiles;

uniform int flipX;
uniform int flipY;

uniform int isSoft;
uniform float softRange;

out vec4 outColor;

float LinearizeDepth(float depth) 
{
    return (far * near) / (far - depth * (far - near));	
}

void main()
{	
	float u = flipX * (1 - uv0.x) + (1 - flipX) * uv0.x;
	float v = flipY * (1 - uv0.y) + (1 - flipY) * uv0.y;

	float X = trunc(mod(currentFrame, Xtiles));
	float Y = trunc(currentFrame / Xtiles);
	Y = (Ytiles - 1) - Y;
	X = mix(X, X + 1, u);
	Y = mix(Y, Y + 1, v);
	
	u = X / Xtiles;
	v = Y / Ytiles;
	
	outColor = SRGBA(inputColor) * (hasDiffuseMap * SRGBA(texture2D(diffuseMap,  vec2(u, v)) * vec4(intensity.r, intensity.g, intensity.b, 1.0)) + (1 - hasDiffuseMap));
	
	if (isSoft == 1) {
		vec2 screenUV = gl_FragCoord.xy / vec2(textureSize(depths, 0));
		float sceneDepth = LinearizeDepth(texture2D(depths, screenUV).r);
		float fragDepth = LinearizeDepth(gl_FragCoord.z);
		float depthDelta = sceneDepth - fragDepth;
		float opacity = smoothstep(0.0, 1.0, depthDelta / softRange);
		outColor *= vec4(vec3((1 - transparent) * opacity + transparent), transparent * opacity + (1 - transparent));
	}
}

