--- fragHeightFog

uniform mat4 view;
uniform mat4 proj;

uniform sampler2DMS originalRender;
uniform sampler2DMS positions;

uniform vec3 viewPos;

uniform float density;
uniform float falloff;
uniform float height;
uniform vec4 inscatteringColor;

in vec2 uv;

layout(location = 0) out vec4 result;

void main()
{
	ivec2 vp = textureSize(originalRender);
	vp = ivec2(vec2(vp) * uv);
    vec4 position = texelFetch(positions, vp, gl_SampleID);
    if (position.z > -0.0001)
    {
        position = inverse(proj) * normalize(vec4(uv * 2.0 - 1.0, 1.0, 1.0));
        position /= position.w;
    }

    vec3 cameraToFrag = vec3(inverse(view) * position) - viewPos;
	float cameraToFragDist = length(cameraToFrag);
    vec3 cameraToFragDir = normalize(cameraToFrag);
    
    float rayFalloff = max(-127.0, cameraToFrag.y * falloff);
    float rayIntegral = cameraToFragDist * density * exp2(-falloff * (viewPos.y - height)) * (1.0 - exp2(-rayFalloff)) / rayFalloff;
    float fogAmount = clamp(1.0 - exp2(-rayIntegral), 0.0, inscatteringColor.a);
    vec3 fogColor = SRGB(inscatteringColor.rgb);
    result = vec4(mix(texelFetch(originalRender, vp, gl_SampleID).rgb, fogColor, fogAmount), 1.0);
}