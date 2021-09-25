--- varLights

#define GRID_FRUSTUM_WORK_GROUP_SIZE 16
#define LIGHT_TILE_SIZE 16
#define MAX_LIGHTS_PER_TILE 1024

struct Light
{
	vec3 pos;
	int isSpotLight;
	vec3 direction;
	float intensity;
	vec3 color;
	float radius;
	int useCustomFalloff;
	float falloffExponent;
	float innerAngle;
	float outerAngle;
};

struct LightTile
{
	uint count;
	uint offset;
};

struct TileFrustum
{
    vec3 planeNormals[4];
};

--- compGridFrustums

layout(std430, binding = 0) writeonly buffer TileFrustumsBuffer
{
	TileFrustum data[];
} tileFrustumsBuffer;

uniform mat4 invProj;

uniform vec2 screenSize;
uniform uvec2 numThreads;

layout(local_size_x = GRID_FRUSTUM_WORK_GROUP_SIZE, local_size_y = GRID_FRUSTUM_WORK_GROUP_SIZE, local_size_z = 1) in;
void main()
{
    // Calculate normalized tile bounds
    vec2 tileRects[4];
    vec2 BiS2 = GRID_FRUSTUM_WORK_GROUP_SIZE * 2.0 / screenSize;
    tileRects[0] = vec2(gl_GlobalInvocationID.xy) * BiS2 - 1.0; // Top left
    tileRects[1] = vec2(gl_GlobalInvocationID.x + 1, gl_GlobalInvocationID.y) * BiS2 - 1.0; // Top right
    tileRects[2] = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y + 1) * BiS2 - 1.0; // Bottom left
    tileRects[3] = vec2(gl_GlobalInvocationID.x + 1, gl_GlobalInvocationID.y + 1) * BiS2- 1.0; // Bottom right

    // Calculate view space points (in far plane)
    vec3 tileRectsVS[4];
    for (int i = 0; i < 4; i++)
    {
        vec4 tileRectVS = invProj * vec4(tileRects[i], -1.0, 1.0);
        tileRectsVS[i] = (tileRectVS / tileRectVS.w).xyz;
    }

    // Build the frustum plane normals
    TileFrustum tileFrustum;
    tileFrustum.planeNormals[0] = normalize(cross(tileRectsVS[2], tileRectsVS[0])); // Left
    tileFrustum.planeNormals[1] = normalize(cross(tileRectsVS[1], tileRectsVS[3])); // Right
    tileFrustum.planeNormals[2] = normalize(cross(tileRectsVS[0], tileRectsVS[1])); // Top
    tileFrustum.planeNormals[3] = normalize(cross(tileRectsVS[3], tileRectsVS[2])); // Bottom

    // Store the frustum
    if (gl_GlobalInvocationID.x < numThreads.x && gl_GlobalInvocationID.y < numThreads.y)
    {
        uint index = gl_GlobalInvocationID.x + (gl_GlobalInvocationID.y * numThreads.x);
        tileFrustumsBuffer.data[index] = tileFrustum;
    }
}

--- compLightCulling

layout(std430, binding = 0) readonly buffer TileFrustumsBuffer
{
	TileFrustum data[];
} tileFrustumsBuffer;

layout(std430, binding = 1) readonly buffer LightBuffer
{
	Light data[];
} lightBuffer;

layout(std430, binding = 2) buffer LightIndicesCountBuffer
{
	uint count;
} lightIndicesCountBuffer;

layout(std430, binding = 3) writeonly buffer LightIndicesBuffer
{
	uint data[];
} lightIndicesBuffer;

layout(std430, binding = 4) writeonly buffer LightTilesBuffer
{
	LightTile data[];
} lightTilesBuffer;

uniform mat4 invProj;
uniform mat4 view;

uniform vec2 screenSize;
uniform int lightCount;

uniform sampler2D depths;

shared uint minDepthBits;
shared uint maxDepthBits;
shared TileFrustum tileFrustum;
shared uint tileLightCount;
shared uint tileIndexListOffset;
shared uint tileLightIndices[MAX_LIGHTS_PER_TILE];

void AddLightIndex(uint lightIndex)
{
    uint index = atomicAdd(tileLightCount, 1);
    if (index < MAX_LIGHTS_PER_TILE)
    {
        tileLightIndices[index] = lightIndex;
    }
}

float DepthToView(float depth)
{
    vec4 depthPoint = invProj * vec4(0.0, 0.0, depth, 1.0);
    return depthPoint.z / depthPoint.w;
}

bool SphereInsidePlane(vec3 spherePosition, float sphereRadius, vec3 planeNormal, float planeDistance)
{
    return dot(planeNormal, spherePosition) - planeDistance < -sphereRadius;
}

bool SphereInsideTileFrustum(vec3 spherePosition, float sphereRadius, TileFrustum tileFrustum, float near, float far)
{
    bool isInsideFrustum = true;
 
    if (spherePosition.z - sphereRadius > near || spherePosition.z + sphereRadius < far)
    {
        isInsideFrustum = false;
    }
 
    for (int i = 0; isInsideFrustum && i < 4; i++)
    {
        if (dot(tileFrustum.planeNormals[i], spherePosition) < -sphereRadius)
        {
            isInsideFrustum = false;
        }
    }
 
    return isInsideFrustum;
}

float LinearizeDepth(float depth, float near, float far)
{
    return 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
}

layout(local_size_x = LIGHT_TILE_SIZE, local_size_y = LIGHT_TILE_SIZE, local_size_z = 1) in;
void main()
{
    // Get depth from depth texture and transform to uint bits
    vec2 uv = vec2(gl_GlobalInvocationID.xy) / screenSize;
    float depth = 2.0 * texture(depths, uv).r - 1.0;
    uint depthBits = floatBitsToUint(depth);

    // Initialize shared variables (only once)
    if (gl_LocalInvocationIndex  == 0)
    {
        minDepthBits = 0xFFFFFFFF;
        maxDepthBits = 0;
        tileLightCount = 0;
        uint index = gl_WorkGroupID.x + (gl_WorkGroupID.y * gl_NumWorkGroups.x);
        tileFrustum = tileFrustumsBuffer.data[index];
    }
 
    barrier();

    // Update min and max depths
    atomicMin(minDepthBits, depthBits);
    atomicMax(maxDepthBits, depthBits);

    barrier();

    // Cull lights
    float minDepth = uintBitsToFloat(minDepthBits);
    float maxDepth = uintBitsToFloat(maxDepthBits);
 
    float minDepthVS = DepthToView(minDepth);
    float maxDepthVS = DepthToView(maxDepth);
    float nearPlaneVS = DepthToView(0.0);
 
    vec3 minPlaneNormal = vec3(0, 0, -1);
    float minPlaneDistance = -minDepthVS;

    for (uint i = gl_LocalInvocationIndex; i < lightCount; i += LIGHT_TILE_SIZE * LIGHT_TILE_SIZE)
    {
        Light light = lightBuffer.data[i];
        vec3 lightPosVS = (view * vec4(light.pos, 1.0)).xyz;
        if (SphereInsideTileFrustum(lightPosVS, light.radius, tileFrustum, nearPlaneVS, maxDepthVS))
        {
            // TODO: Add transparent light list
            // AddLightIndex(i);

            if (!SphereInsidePlane(lightPosVS, light.radius, minPlaneNormal, minPlaneDistance))
            {
                AddLightIndex(i);
            }
        }
    }

    barrier();

    // Reserve space in the index buffer and update tile
    if (gl_LocalInvocationIndex == 0)
    {
        tileIndexListOffset = atomicAdd(lightIndicesCountBuffer.count, tileLightCount);
        uint index = gl_WorkGroupID.x + (gl_WorkGroupID.y * gl_NumWorkGroups.x);
        lightTilesBuffer.data[index].count = tileLightCount;
        lightTilesBuffer.data[index].offset = tileIndexListOffset;
    }

    barrier();

    // Write indices to index buffer
    for (uint i = gl_LocalInvocationIndex; i < tileLightCount; i += LIGHT_TILE_SIZE * LIGHT_TILE_SIZE)
    {
        lightIndicesBuffer.data[tileIndexListOffset + i] = tileLightIndices[i];
    }
}