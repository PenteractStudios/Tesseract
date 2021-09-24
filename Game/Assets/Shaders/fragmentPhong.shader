--- fragMainPhong

void main() 
{

    vec3 viewDir = normalize(viewPos - fragPos);
    vec2 tiledUV = GetTiledUVs(); 
    vec3 normal = fragNormal;

    if (hasNormalMap)
    {
	    normal = GetNormal(tiledUV);
    }

    // diffuse
    vec4 colorDiffuse = GetDiffuse(tiledUV);

    // specular
    vec4 colorSpecular = hasSpecularMap * SRGBA(texture(specularMap, tiledUV)) + (1 - hasSpecularMap) * vec4(SRGB(specularColor), 1.0);
    vec3 Rf0 = colorSpecular.rgb;

    // shininess
    float shininess = hasSmoothnessAlpha * exp2(colorSpecular.a * 7 + 1) + (1 - hasSmoothnessAlpha) * smoothness;

    // TODO: IBL doesn't work correctly with Blinn-Phong
    // roughness
    float roughness = Pow2(1 - smoothness * (hasSmoothnessAlpha * colorSpecular.a + (1 - hasSmoothnessAlpha) * colorDiffuse.a)) + EPSILON;

    // Ambient Light
    vec3 R = reflect(-viewDir, normal);
    vec3 colorAccumulative = GetOccludedAmbientLight(R, normal, viewDir, colorDiffuse.rgb, colorSpecular.rgb, roughness, tiledUV);

    // Directional Light
    if (dirLight.isActive == 1) {
        vec3 directionalDir = normalize(dirLight.direction);
        float NL = max(dot(normal, - directionalDir), 0.0);

        vec3 reflectDir = reflect(directionalDir, normal);
        float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

        vec3 directionalColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * dirLight.color * dirLight.intensity * NL;

        unsigned int indexS = DepthMapIndexStatic();
        unsigned int indexD = DepthMapIndexDynamic();
        float shadowS = Shadow(fragPosLightStatic[indexS], normal, normalize(dirLight.direction), depthMapTexturesStatic[indexS]);
        float shadowD = Shadow(fragPosLightDynamic[indexD], normal, normalize(dirLight.direction), depthMapTexturesDynamic[indexD]);
        
        float shadow = max(min(shadowD, 1), shadowS);

        colorAccumulative += (1.0 - shadow) * directionalColor;
    }

	int tileIndex = GetTileIndex();

    // Point Light
    for (int i = 0; i < lightTiles.data[tileIndex].numPoints; i++) {
        float pointDistance = length(lightTiles.data[tileIndex].points[i].pos - fragPos);
        float falloffExponent = lightTiles.data[tileIndex].points[i].useCustomFalloff * lightTiles.data[tileIndex].points[i].falloffExponent + (1 - lightTiles.data[tileIndex].points[i].useCustomFalloff) * 4.0;
        float distAttenuation = clamp(1.0 - pow(pointDistance / lightTiles.data[tileIndex].points[i].radius, falloffExponent), 0.0, 1.0);
        distAttenuation = lightTiles.data[tileIndex].points[i].useCustomFalloff * distAttenuation + (1 - lightTiles.data[tileIndex].points[i].useCustomFalloff) * distAttenuation * distAttenuation / (pointDistance * pointDistance + 1.0);

        vec3 pointDir = normalize(fragPos - lightTiles.data[tileIndex].points[i].pos);
        float NL = max(dot(normal, -pointDir), 0.0);

        vec3 reflectDir = reflect(pointDir, normal);
        float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

        vec3 pointColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * lightTiles.data[tileIndex].points[i].color * lightTiles.data[tileIndex].points[i].intensity * distAttenuation * NL;

        colorAccumulative += pointColor;
    }

    // Spot Light
    for (int i = 0; i < lightTiles.data[tileIndex].numSpots; i++) {
        float spotDistance = length(lightTiles.data[tileIndex].spots[i].pos - fragPos);
        float falloffExponent = lightTiles.data[tileIndex].spots[i].useCustomFalloff * lightTiles.data[tileIndex].spots[i].falloffExponent + (1 - lightTiles.data[tileIndex].spots[i].useCustomFalloff) * 4.0;
        float distAttenuation = clamp(1.0 - pow(spotDistance / lightTiles.data[tileIndex].spots[i].radius, falloffExponent), 0.0, 1.0);
        distAttenuation = lightTiles.data[tileIndex].spots[i].useCustomFalloff * distAttenuation + (1 - lightTiles.data[tileIndex].spots[i].useCustomFalloff) * distAttenuation * distAttenuation / (spotDistance * spotDistance + 1.0);
        
        vec3 spotDir = normalize(fragPos - lightTiles.data[tileIndex].spots[i].pos);

        vec3 aimDir = normalize(lightTiles.data[tileIndex].spots[i].direction);
        float C = dot(aimDir, spotDir);
        float cAttenuation = 0;
        float cosInner = cos(lightTiles.data[tileIndex].spots[i].innerAngle);
        float cosOuter = cos(lightTiles.data[tileIndex].spots[i].outerAngle);
        if (C > cosInner) {
            cAttenuation = 1;
        } else if (cosInner > C && C > cosOuter) {
            cAttenuation = (C - cosOuter) / (cosInner - cosOuter);
        }

        float NL = max(dot(normal, -spotDir), 0.0);

        vec3 reflectDir = reflect(spotDir, normal);
        float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

        vec3 spotColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * lightTiles.data[tileIndex].spots[i].color * lightTiles.data[tileIndex].spots[i].intensity * distAttenuation * cAttenuation * NL;

        colorAccumulative += spotColor;
    }

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;
    
    outColor = vec4(colorAccumulative, colorDiffuse.a);
}