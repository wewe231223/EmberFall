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

Texture2D GBuffers[4] : register(t0, space0);


cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    float3 cameraPosition;
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

float4 DirectionalLight(float3 vNormal, float3 toCamera, float2 texcoord)
{
    float3 direct = normalize(float3(-1.0f,3.0f,1.0f));
    float dotNormalLight = dot(direct, vNormal);

    float4 diffuse = float4(1.0f,1.0f,1.0f,1.0f);
    
    
    diffuse.xyz = diffuse.xyz * max(dotNormalLight, 0.0f);

    float3 vReflect = reflect(-direct, vNormal);
    float specularFactor = pow(max(dot(toCamera, vReflect), 0.0f), 32.0f);

    float4 specular = float4(0.8f, 0.8f, 0.8f, 1.0f);
    specular.xyz = specular.xyz * specularFactor;
    
    float4 color = diffuse + specular;
  
    return color;

}



float ComputeShadowFactor(float4 shadowPosH, float bias)
{
    float depth = shadowPosH.z - bias;
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
        percentLit += GBuffers[3].SampleCmpLevelZero(PCFSampler, shadowPosH.xy + offsets[i], depth).r;
    }
    
    
    return lerp(0.1f, 1.0f, percentLit / 9.0f);
    
   
}

float4 Deffered_PS(Deffered_VOUT input) : SV_TARGET
{
    //return float4(GBuffers[1].Sample(linearWrapSampler, input.texcoord).xyz, 1.0f);
    float4 diffuse = GBuffers[0].Sample(linearWrapSampler, input.texcoord);
    float3 normal = normalize(GBuffers[1].Sample(linearWrapSampler, input.texcoord).xyz);
    float4 worldPos = GBuffers[2].Sample(linearWrapSampler, input.texcoord);
    float3 toCamera = normalize(cameraPosition - worldPos.xyz);
    
    float4 LightingColor = DirectionalLight(normal, toCamera, input.texcoord);
    for (int i = 0; i < step(4.0f, GBuffers[1].Sample(linearWrapSampler, input.texcoord).w); ++i)
    {
        LightingColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    float validWorld = step(0.000001, abs(worldPos.x));

    float4 texPos = mul(worldPos, viewProjection);

    texPos.x = texPos.x * 0.5f + 0.5f;
    texPos.y = -texPos.y * 0.5f + 0.5f;

    float insideX = step(0.0f, texPos.x) * step(texPos.x, 1.0f);
    float insideY = step(0.0f, texPos.y) * step(texPos.y, 1.0f);
    float validTex = insideX * insideY;
    float valid = validWorld * validTex;
    
    
    float bias = 0.003f;

    float shadowFactor = ComputeShadowFactor(texPos, bias);
    float depth = GBuffers[3].Sample(linearWrapSampler, texPos.xy).r;

    float shadowApply = step(depth + bias, texPos.z) * valid;

    float finalFactor = lerp(1.0f, shadowFactor, shadowApply);

    return diffuse * finalFactor * LightingColor;
}