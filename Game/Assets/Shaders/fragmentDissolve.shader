--- fragFunctionDissolveCommon

uniform vec4 dissolveColor;
uniform float dissolveIntensity;
uniform float dissolveScale;
uniform float dissolveThreshold;
uniform vec2 dissolveOffset;
uniform float edgeSize;

// Some useful functions
vec3 Mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 Mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 Permute(vec3 x) { return Mod289(((x * 34.0) + 1.0) * x); }

//
// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
//
float SimplexNoise(vec2 v) {

    // Precompute values for skewed triangular grid
    const vec4 C = vec4(0.211324865405187,
        // (3.0-sqrt(3.0))/6.0
        0.366025403784439,
        // 0.5*(sqrt(3.0)-1.0)
        -0.577350269189626,
        // -1.0 + 2.0 * C.x
        0.024390243902439);
    // 1.0 / 41.0

    // First corner (x0)
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    // Other two corners (x1, x2)
    vec2 i1 = vec2(0.0);
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec2 x1 = x0.xy + C.xx - i1;
    vec2 x2 = x0.xy + C.zz;

    // Do some permutations to avoid
    // truncation effects in permutation
    i = Mod289(i);
    vec3 p = Permute(
        Permute(i.y + vec3(0.0, i1.y, 1.0))
        + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(
        dot(x0, x0),
        dot(x1, x1),
        dot(x2, x2)
    ), 0.0);

    m = m * m;
    m = m * m;

    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    // Compute final noise value at P
    vec3 g = vec3(0.0);
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * vec2(x1.x, x2.x) + h.yz * vec2(x1.y, x2.y);
    return 130.0 * dot(m, g);
}

--- fragFunctionDissolveFunction

vec4 Dissolve(vec4 finalColor, vec2 tiledUV) {
    if (dissolveThreshold == 0) {
        return finalColor;
    }

    vec3 color = vec3(0.0);
    vec2 transformedUV = (tiledUV * dissolveScale) + dissolveOffset;
    color = vec3(SimplexNoise(transformedUV) * .5 + .5);

    if (color.x > dissolveThreshold * (1.0 + edgeSize)) {
        return vec4(finalColor.xyz, color.x);
    } else if (color.x > dissolveThreshold) {
        return vec4(finalColor.xyz, color.x) * dissolveColor * dissolveIntensity;
    }

    discard;
}

--- fragFunctionDepthDissolve

bool MustDissolve(vec2 tiledUV) {
    if (dissolveThreshold == 0) {
        return false;
    }

    vec3 color = vec3(0.0);
    vec2 transformedUV = (tiledUV * dissolveScale) + dissolveOffset;
    color = vec3(SimplexNoise(transformedUV) * .5 + .5);

    if (color.x > dissolveThreshold * (1.0 + edgeSize)) {
        return false;
    }
    return true;
}