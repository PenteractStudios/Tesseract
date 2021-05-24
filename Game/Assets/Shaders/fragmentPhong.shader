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
    vec4 colorSpecular = hasSpecularMap * pow(texture(specularMap, tiledUV), vec4(2.2)) + (1 - hasSpecularMap) * vec4(specularColor, 1.0);
    vec3 Rf0 = colorSpecular.rgb;

    // shininess
    float shininess = hasSmoothnessAlpha * exp2(colorSpecular.a * 7 + 1) + (1 - hasSmoothnessAlpha) * smoothness;

    // Ambient Color
    vec3 colorAmbient = GetAmbientOcclusion(tiledUV);

    vec3 colorAccumulative = colorDiffuse.rgb * colorAmbient;

    // Directional Light
    if (light.directional.isActive == 1) {
        vec3 directionalDir = normalize(light.directional.direction);
        float NL = max(dot(normal, - directionalDir), 0.0);

        vec3 reflectDir = reflect(directionalDir, normal);
        float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);


        vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

        vec3 directionalColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * light.directional.color * light.directional.intensity * NL;

        colorAccumulative += directionalColor;
    }

    // Point Light
    for (int i = 0; i < light.numPoints; i++) {
        float pointDistance = length(light.points[i].pos - fragPos);
        float distAttenuation = 1.0 / (light.points[i].kc + light.points[i].kl * pointDistance + light.points[i].kq * pointDistance * pointDistance);

        vec3 pointDir = normalize(fragPos - light.points[i].pos);
        float NL = max(dot(normal, -pointDir), 0.0);

        vec3 reflectDir = reflect(pointDir, normal);
        float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

        vec3 pointColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * light.points[i].color * light.points[i].intensity * distAttenuation * NL;

        colorAccumulative += pointColor;
    }

    // Spot Light
    for (int i = 0; i < light.numSpots; i++) {
        float spotDistance = length(light.spots[i].pos - fragPos);
        float distAttenuation = 1.0 / (light.spots[i].kc + light.spots[i].kl * spotDistance + light.spots[i].kq * spotDistance * spotDistance);

        vec3 spotDir = normalize(fragPos - light.spots[i].pos);

        vec3 aimDir = normalize(light.spots[i].direction);
        float C = dot(aimDir, spotDir);
        float cAttenuation = 0;
        float cosInner = cos(light.spots[i].innerAngle);
        float cosOuter = cos(light.spots[i].outerAngle);
        if (C > cosInner) {
            cAttenuation = 1;
        } else if (cosInner > C && C > cosOuter) {
            cAttenuation = (C - cosOuter) / (cosInner - cosOuter);
        }

        float NL = max(dot(normal, -spotDir), 0.0);

        vec3 reflectDir = reflect(spotDir, normal);
        float VRn = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 Rf = Rf0 + (1 - Rf0) * pow(1 - NL, 5);

        vec3 spotColor = (colorDiffuse.rgb * (1 - Rf0) + (shininess + 2) / 2 * Rf * VRn) * light.spots[i].color * light.spots[i].intensity * distAttenuation * cAttenuation * NL;

        colorAccumulative += spotColor;
    }

    // Emission
    colorAccumulative += GetEmissive(tiledUV).rgb;

    vec3 ldr = colorAccumulative.rgb / (colorAccumulative.rgb + vec3(1.0)); // reinhard tone mapping
    ldr = pow(ldr, vec3(1/2.2)); // gamma correction
    outColor = vec4(ldr, 1.0);
}