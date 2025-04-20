SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);

struct Light
{
    
    float4 Diffuse;
    float4 Ambient; 
    float4 Specular;
    float3 Position; 
    float3 Direction; 
    float3 Attenuation;
    float Range;
    
};

Texture2D GBuffers[6] : register(t0, space0);
StructuredBuffer<Light> gLight : register(t1, space1);

cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    Matrix middleViewProjection;
    Matrix farViewProjection;

    float3 cameraPosition;
    int isShadow;
    float3 shadowOffset;
}

struct Deffered_VIN
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct Deffered_VOUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};


Deffered_VOUT Deffered_VS(Deffered_VIN input)
{
    Deffered_VOUT output;
    output.position = float4(input.position, 1.f);
    output.texcoord = input.texcoord;
    return output;
}


float4 DirectionalLight(float3 normal, float3 toCamera, float2 texcoord)
{
    float3 direct = normalize(float3(-1.0f,3.0f,1.0f));
    float dotNormalLight = dot(direct, normal);

    float4 diffuse = float4(1.0f,1.0f,1.0f,1.0f);
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    diffuse.xyz = diffuse.xyz * max(dotNormalLight, 0.0f);

    float3 vReflect = reflect(-direct, normal);
    float specularFactor = pow(max(dot(toCamera, vReflect), 0.0f), 32.0f);

    float4 specular = float4(0.4f, 0.4f, 0.4f, 1.0f);
    specular.xyz = specular.xyz * specularFactor;
    
    float4 color = diffuse + specular + ambient;
    return color;

}

float4 PointLight(int index, float3 position, float3 normal, float3 toCamera, float2 texcoord)
{
    float3 toLight = gLight[index].Position - position;
    float distance = length(toLight);
    
    float4 ambient = gLight[index].Ambient;
    
    if (distance > gLight[index].Range)
    {
        float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
        return color;
    }
    toLight = normalize(toLight);
    normal = normalize(normal);
    float dotNormalLight = dot(toLight, normal);
    
    float4 diffuse = float4(gLight[index].Diffuse.xyz * max(dotNormalLight, 0.0f), 1.0f);
    
        
    float3 vReflect = reflect(-toLight, normal);
    float specularSpot = pow(max(dot(toCamera, vReflect), 0.0f), 32.0f);
    float4 specular = float4(gLight[index].Specular.xyz * specularSpot, 1.0f);
    
    float attenFactor = 1.0f / dot(gLight[index].Attenuation, float3(1.0f, distance, distance * distance));
    
    float4 color = (diffuse + specular) * attenFactor + ambient;
    return color;

}

float4 Lighting(float3 normal, float3 toCamera, float3 worldPos, float2 texcoord)
{
    float4 Color = DirectionalLight(normal, toCamera, texcoord);
    
    [unroll]
    for (int i = 0; i < 8; ++i)
    {
        Color += PointLight(i, worldPos.xyz, normal, toCamera, texcoord);

    }
    return Color;
}

float ComputeShadowFactor(float4 shadowPosH, float bias, float depth)
{
    depth = depth - bias;
    uint width, height, numMips;
    GBuffers[3].GetDimensions(0, width, height, numMips);
    float dx = 1.0f / (float) width;
    float dy = 1.0f / (float) height;

    const float2 offsets[9] =
    {
        float2(-dx, -dy), float2(0.0f, -dy), float2(dx, -dy),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, dy), float2(0.0f, dy), float2(dx, dy)
    };
    float percentLit = 0.0f;
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        if (depth >= 0.1f && depth <= shadowOffset.x )
        {
            percentLit += GBuffers[3].SampleCmpLevelZero(PCFSampler, shadowPosH.xy + offsets[i], depth).r;

        }
        else if (depth <= shadowOffset.y)
        {
            percentLit += GBuffers[4].SampleCmpLevelZero(PCFSampler, shadowPosH.xy + offsets[i], depth).r;
        }
        else if (depth <= shadowOffset.z)
        {
            percentLit += GBuffers[5].SampleCmpLevelZero(PCFSampler, shadowPosH.xy + offsets[i], depth).r;
        }
    }
    
    
    return lerp(0.0f, 1.0f, percentLit / 9.0f);
    
   
}

float4 Deffered_PS(Deffered_VOUT input) : SV_TARGET
{
    //return float4(GBuffers[5].Sample(linearWrapSampler, input.texcoord).xxx, 1.0f);
    float4 diffuse = GBuffers[0].Sample(linearWrapSampler, input.texcoord);
    float3 normal = normalize(GBuffers[1].Sample(linearWrapSampler, input.texcoord).xyz);
    float4 worldPos = GBuffers[2].Sample(linearWrapSampler, input.texcoord);
    float3 toCamera = normalize(cameraPosition - worldPos.xyz);
  
    float4 LightingColor = Lighting(normal, toCamera, worldPos.xyz, input.texcoord);
        
    for (int i = 0; i < step(4.0f, GBuffers[1].Sample(linearWrapSampler, input.texcoord).w); ++i)
    {
        LightingColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    float validWorld = step(0.000001, abs(worldPos.x));

    float4 viewPos = mul(worldPos, view);
    
    float4 texPos;
    if ( viewPos.z >= 0.1f && viewPos.z <= shadowOffset.x)
    {
        texPos = mul(worldPos, viewProjection);

    }
    else if (viewPos.z <= shadowOffset.y)
    {
        texPos = mul(worldPos, middleViewProjection);
    }
    else if (viewPos.z <= shadowOffset.z)
    {
        texPos = mul(worldPos, farViewProjection);
    }
 
    texPos.x = texPos.x * 0.5f + 0.5f;
    texPos.y = -texPos.y * 0.5f + 0.5f;
    float insideX = step(0.0f, texPos.x) * step(texPos.x, 1.0f);
    float insideY = step(0.0f, texPos.y) * step(texPos.y, 1.0f);
    float validTex = insideX * insideY;
    float valid = validWorld * validTex;
    
    
    float bias = 0.003f;

    
    float depth;
    
    if (viewPos.z >= 0.1f && viewPos.z <= shadowOffset.x )
    {
        depth = GBuffers[3].Sample(linearWrapSampler, texPos.xy).r;
    }
    else if (viewPos.z <= shadowOffset.y)
    {
        depth = GBuffers[4].Sample(linearWrapSampler, texPos.xy).r;
    }
    else if (viewPos.z <= shadowOffset.z)
    {
        depth = GBuffers[5].Sample(linearWrapSampler, texPos.xy).r;
    }
    
    //float shadowFactor = ComputeShadowFactor(texPos, bias, viewPos.z);
    float shadowFactor = 0.5f;
   
    
    if (dot((cameraPosition - worldPos.xyz), normal) >= 0)
    {
        shadowFactor = 1.0f;
    }

    float shadowApply = step(depth + bias, texPos.z) * valid;

    float finalFactor = lerp(1.0f, shadowFactor, shadowApply);

    return diffuse * LightingColor * finalFactor;
}