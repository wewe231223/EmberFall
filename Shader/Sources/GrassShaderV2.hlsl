cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float4x4 middleViewProjection;
    float3 cameraPosition;
    int isShadow;
}

cbuffer Time : register(b1)
{
    uint globalTime; // milliseconds
}

cbuffer MaterialIndex : register(b2)
{
    uint materialIndex;
}

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

struct Grass_VIN
{
    float3 position : POSITION; 
    uint instanceID : SV_INSTANCEID;
};

struct Grass_GIN
{
    float3 position : POSITION; 
    uint textureID : TEXID;
};

struct Grass_PIN
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    uint textureID : TEXID;
    float2 texCoord : TEXCOORD0;
};

struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
    float4 emissive : SV_TARGET3;
};

struct MaterialConstants
{
    float4 diffuse;
    float4 specular;
    float4 emissive;
    
    uint diffuseTexture[8];
    uint specularTexture[8];
    uint metalicTexture[8];
    uint emissiveTexture[8];
    uint normalTexture[8];
    uint alphaTexture[8];
};

StructuredBuffer<MaterialConstants> materialConstants : register(t0);
Texture2D textures[1024] : register(t1);

uint GetIndexFromFloat3(float3 v) 
{
    float hash = frac(dot(v, float3(12.9898, 78.233, 37.719)));
    return (uint) (hash * 4.0);
}

Grass_GIN Grass_VS(Grass_VIN input)
{
    Grass_GIN output;
    MaterialConstants material = materialConstants[materialIndex];
    
    output.position = input.position;
    output.textureID = GetIndexFromFloat3(output.position); 
    
    return output;
}


#define WIND_STRENGTH 0.2f
#define WIND_FREQ     0.002f

#define GRASS_HALF_HEIGHT_RATIO 0.9f
#define GRASS_HALF_WIDTH_RATIO  0.4f

[maxvertexcount(12)]
void Grass_GS(point Grass_GIN input[1], inout TriangleStream<Grass_PIN> TriStream)
{
    float3 basePos = input[0].position;

    float scale = 1.0f;
    float halfSize = scale * GRASS_HALF_WIDTH_RATIO;

    // 흔들림
    float randPhase = frac(dot(basePos.xz, float2(12.9898, 78.233)));
    float sway = sin(globalTime * WIND_FREQ + randPhase * 6.2831f) * (WIND_STRENGTH * halfSize);
    float3 swayDir = float3(0.0f, 0.0f, 1.0f);
    float3 swayOffset = swayDir * sway;

    basePos.y += halfSize * GRASS_HALF_HEIGHT_RATIO;

    // 회전
    float randRotation = randPhase * 6.2831f;
    float cosTheta = cos(randRotation);
    float sinTheta = sin(randRotation);
    float3 right = float3(cosTheta, 0.0f, -sinTheta);
    float3 forward = float3(sinTheta, 0.0f, cosTheta);
    const float3 up = float3(0.0f, 1.0f, 0.0f);

    float3 verts[8];
    verts[0] = basePos + (-right + up) * halfSize + swayOffset;
    verts[1] = basePos + (right + up) * halfSize + swayOffset;
    verts[2] = basePos + (right - up) * halfSize;
    verts[3] = basePos + (-right - up) * halfSize;
    verts[4] = basePos + (-forward + up) * halfSize + swayOffset;
    verts[5] = basePos + (forward + up) * halfSize + swayOffset;
    verts[6] = basePos + (forward - up) * halfSize;
    verts[7] = basePos + (-forward - up) * halfSize;

    float3 normal1 = normalize(cross(verts[1] - verts[0], verts[2] - verts[0]));
    float3 normal2 = normalize(cross(verts[5] - verts[4], verts[6] - verts[4]));

    float2 uvs[4] = { float2(0, 0), float2(1, 0), float2(1, 1), float2(0, 1) };

    // 앞쪽 quad
    {
        Grass_PIN p0 = { mul(float4(verts[0], 1), viewProj), normal1, input[0].textureID, uvs[0] };
        Grass_PIN p1 = { mul(float4(verts[1], 1), viewProj), normal1, input[0].textureID, uvs[1] };
        Grass_PIN p2 = { mul(float4(verts[2], 1), viewProj), normal1, input[0].textureID, uvs[2] };
        Grass_PIN p3 = { mul(float4(verts[3], 1), viewProj), normal1, input[0].textureID, uvs[3] };
        TriStream.Append(p2);
        TriStream.Append(p1);
        TriStream.Append(p0);
        TriStream.RestartStrip();
        TriStream.Append(p0);
        TriStream.Append(p3);
        TriStream.Append(p2);
        TriStream.RestartStrip();
    }

    // 옆쪽 quad
    {
        Grass_PIN p4 = { mul(float4(verts[4], 1), viewProj), normal2, input[0].textureID, uvs[0] };
        Grass_PIN p5 = { mul(float4(verts[5], 1), viewProj), normal2, input[0].textureID, uvs[1] };
        Grass_PIN p6 = { mul(float4(verts[6], 1), viewProj), normal2, input[0].textureID, uvs[2] };
        Grass_PIN p7 = { mul(float4(verts[7], 1), viewProj), normal2, input[0].textureID, uvs[3] };
        TriStream.Append(p6);
        TriStream.Append(p5);
        TriStream.Append(p4);
        TriStream.RestartStrip();
        TriStream.Append(p4);
        TriStream.Append(p7);
        TriStream.Append(p6);
        TriStream.RestartStrip();
    }
}

Deffered_POUT Grass_PS(Grass_PIN input)
{
    Deffered_POUT output = (Deffered_POUT)0;

    MaterialConstants material = materialConstants[materialIndex];
    float4 color = textures[material.diffuseTexture[input.textureID]].Sample(anisotropicWrapSampler, input.texCoord);

    clip(color.a - 0.2f);

    output.diffuse = color;
    output.normal = float4(input.normal, 1.0f);
    output.position = input.position;
    output.emissive = material.emissive;

    return output;
}