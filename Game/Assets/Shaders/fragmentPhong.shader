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
        
        float shadow = (shadowS == 1 || shadowD == 1) ? min(shadowS, shadowD) : shadowD * shadowS; //mix(shadowD, shadowS, 0.5);

        colorAccumulative += shadow * directionalColor;
    }

	
	// Lights
	int tileIndex = GetTileIndex();
	LightTile lightTile = lightTilesBufferTransparent.data[tileIndex];
	for (uint i = 0; i < lightTile.count; i++ )
    {
		uint lightIndex = lightIndicesBufferTransparent.data[lightTile.offset + i];
		Light light = lightBuffer.data[lightIndex];
		if (light.isSpotLight == 1)
		{
            float spotDistance = length(light.pos - fragPos);
            float falloffExponent = light.useCustomFalloff * light.falloffExponent + (1 - light.useCustomFalloff) * 4.0;
            float distAttenuation = clamp(1.0 - pow(spotDistance / light.radius, falloffExponent), 0.0, 1.0);
            distAttenuation = light.useCustomFalloff * distAttenuation + (1 - light.useCustomFalloff) * distAttenuation * distAttenuation / (spotDistance * spotDistance + 1.0);
            
            vec3 spotDir = normalize(fragPos - light.pos);

            vec3 aimDir = normalize(light.direction);
            float C = dot(aimDir, spotDir);
            float cAttenuation = 0;
            float cosInner = cos(light.innerAngle);
            float cosOuter = cos(light.outerAngle);
            if (C > cosInner) {
                cAttenuation = 1;
            } else if (cosInner > C && C > cosOuter) {
                cAttenuation = (C - cosOuter) / (cosInner - cosOuter);
            }

            float NL = max(dot(normal, -spotDir), 0.0);

            vec3 reflectDir = reflect(spotDir, normal);
            float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

            vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

            vec3 spotColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * light.color * light.intensity * distAttenuation * cAttenuation * NL;

            colorAccumulative += spotColor;
		}
		else
		{
            float pointDistance = length(light.pos - fragPos);
            float falloffExponent = light.useCustomFalloff * light.falloffExponent + (1 - light.useCustomFalloff) * 4.0;
            float distAttenuation = clamp(1.0 - pow(pointDistance / light.radius, falloffExponent), 0.0, 1.0);
            distAttenuation = light.useCustomFalloff * distAttenuation + (1 - light.useCustomFalloff) * distAttenuation * distAttenuation / (pointDistance * pointDistance + 1.0);

            vec3 pointDir = normalize(fragPos - light.pos);
            float NL = max(dot(normal, -pointDir), 0.0);

            vec3 reflectDir = reflect(pointDir, normal);
            float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

            vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

            vec3 pointColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * light.color * light.intensity * distAttenuation * NL;

            colorAccumulative += pointColor;
		}
	}

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;
    
    outColor = vec4(colorAccumulative, colorDiffuse.a);
}