--- fragPostprocess

in vec2 uv;

out vec4 outColor;
uniform sampler2D scene;
uniform sampler2D bloomBlur;

void main()
{
    vec4 hdrColor = texture(scene, uv);
    vec3 bloomColor = texture(bloomBlur, uv).rgb;
    hdrColor.rgb += bloomColor; // additive blending

    vec3 ldr = hdrColor.rgb / (hdrColor.rgb + vec3(1.0)); // reinhard tone mapping
    ldr = pow(ldr, vec3(1 / 2.2)); // gamma correction
    outColor = vec4(ldr, hdrColor.a);
}