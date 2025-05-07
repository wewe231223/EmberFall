SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);

#define MAX_LIGHT_COUNT 8 
#define LightType_Directional   1 
#define LightType_Point         2
#define LightType_Spot          3

struct Light
{
    uint lightType; 
    float4 Diffuse;
    float4 Ambient; 
    float4 Specular;
    float3 Position; 
    float3 Direction; 
    float3 Attenuation;
    float InnerAngle;
    float OuterAngle; 
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
    //Matrix farViewProjection;

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


float4 DirectionalLight(float3 direct, float3 normal, float3 toCamera, float2 texcoord)
{
    float3 nDir = normalize(direct);
    float dotNormalLight = dot(nDir, normal);

    float4 diffuse = float4(1.0f,1.0f,1.0f,1.0f);
    float4 ambient = float4(0.20f, 0.20f, 0.20f, 1.0f);
    
    diffuse.xyz = diffuse.xyz * max(dotNormalLight, 0.0f);

    float3 vReflect = reflect(-nDir, normal);
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
    toLight = normalize(toLight);
    normal = normalize(normal);

    float3 reflectVec = reflect(-toLight, normal);
    float NdotL = max(dot(toLight, normal), 0.0f);
    float spec = pow(max(dot(toCamera, reflectVec), 0.0f), 32.0f);

    float sharpness = 4.0f; 
    float rangeFade = saturate(1.0f - pow(distance / gLight[index].Range, sharpness));

    float4 ambient = gLight[index].Ambient;
    float4 diffuse = float4(gLight[index].Diffuse.rgb * NdotL, 1.0f);
    float4 specular = float4(gLight[index].Specular.rgb * spec, 1.0f);

    return ambient * rangeFade + (diffuse + specular) * rangeFade;
}

float4 SpotLight(int index, float3 position, float3 normal, float3 toCamera, float2 texcoord)
{
    float3 toLight = gLight[index].Position - position;
    float distance = length(toLight);
    toLight = normalize(toLight);
    normal = normalize(normal);

    float NdotL = max(dot(toLight, normal), 0.0f);
    float3 reflectVec = reflect(-toLight, normal);
    float spec = pow(max(dot(toCamera, reflectVec), 0.0f), 32.0f);

    // 기본 감쇄 (거리 기반)
    float sharpness = 4.0f;
    float rangeFade = saturate(1.0f - pow(distance / gLight[index].Range, sharpness));

    float3 lightDir = normalize(-gLight[index].Direction); 
    float spotCos = dot(toLight, lightDir); 

    float innerCos = cos(radians(gLight[index].InnerAngle));
    float outerCos = cos(radians(gLight[index].OuterAngle));

    float spotFade = saturate((spotCos - outerCos) / (innerCos - outerCos));

    float attenuation = rangeFade * spotFade;

    float4 ambient = gLight[index].Ambient;
    float4 diffuse = float4(gLight[index].Diffuse.rgb * NdotL, 1.0f);
    float4 specular = float4(gLight[index].Specular.rgb * spec, 1.0f);

    return ambient * attenuation + (diffuse + specular) * attenuation;
}

float4 Lighting(float3 normal, float3 toCamera, float3 worldPos, float2 texcoord)
{
    float4 Color = float4(0.f, 0.f, 0.f, 1.f);
    [unroll]
    for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
    {
        if(gLight[i].lightType == LightType_Directional)
        {
            Color = DirectionalLight(gLight[i].Direction, normal, toCamera, texcoord);
        }
        else if (gLight[i].lightType == LightType_Point)
        {
            Color = PointLight(i, worldPos, normal, toCamera, texcoord);
        }
        else if (gLight[i].lightType == LightType_Spot)
        {
            Color = SpotLight(i, worldPos, normal, toCamera, texcoord);
        }
    }
    
    
    
    return Color;
}

float ComputeShadowFactor(float4 shadowPosH, float bias, float depth)
{
    depth = depth + bias;
    uint width, height, numMips;
    //GBuffers[3].GetDimensions(0, width, height, numMips);
    
    if (depth >= 0.1f && depth <= shadowOffset.x)
    {
        GBuffers[3].GetDimensions(0, width, height, numMips);

    }
    else
    {
        GBuffers[4].GetDimensions(0, width, height, numMips);
    }
  
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
        else
        {
            percentLit += GBuffers[4].SampleCmpLevelZero(PCFSampler, shadowPosH.xy + offsets[i], depth).r;
        }
      
    }
    
    
    return percentLit / 9.0f;
    
   
}

float4 Deffered_PS(Deffered_VOUT input) : SV_TARGET
{
    //return float4(GBuffers[1].Sample(linearWrapSampler, input.texcoord).rgb, 1.0f);
    float4 diffuse = GBuffers[0].Sample(linearWrapSampler, input.texcoord);
    float3 normal = normalize(GBuffers[1].Sample(linearWrapSampler, input.texcoord).xyz);
    float4 worldPos = GBuffers[2].Sample(linearWrapSampler, input.texcoord);
    float3 toCamera = normalize(cameraPosition - worldPos.xyz);
    float4 emissive = GBuffers[3].Sample(linearWrapSampler, input.texcoord);
    float white = 21.0f;

    emissive = emissive * (1.0f + emissive / (white * white)) / (emissive + 1.0f);
    
    
    float4 LightingColor = Lighting(normal, toCamera, worldPos.xyz, input.texcoord);
    
    for (int i = 0; i < step(4.0f, GBuffers[1].Sample(linearWrapSampler, input.texcoord).w); ++i)
    {
        LightingColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    
    
    float validWorld = step(0.000001, abs(worldPos.x));

    float4 viewPos = mul(worldPos, view);
    
    float shadowIndex = step(shadowOffset.x, viewPos.z);

    float4 posNear = mul(worldPos, viewProjection);
    float4 posFar = mul(worldPos, middleViewProjection);


    float4 texPos = lerp(posNear, posFar, shadowIndex);
    
 
    texPos.x = texPos.x * 0.5f + 0.5f;
    texPos.y = -texPos.y * 0.5f + 0.5f;
    float insideX = step(0.0f, texPos.x) * step(texPos.x, 1.0f);
    float insideY = step(0.0f, texPos.y) * step(texPos.y, 1.0f);
    float validTex = insideX * insideY;
    float valid = validWorld * validTex;
    
    
    float bias = 0.003f;

    float blendOffset = 2.0f;
    float depth;
    
    
  
    depth = GBuffers[4 + shadowIndex].Sample(linearWrapSampler, texPos.xy).r;
    
    

    
    //float shadowFactor = ComputeShadowFactor(texPos, bias, viewPos.z);
   
    float shadowFactor = 0.5f;
   
    float isBackFace = step(0, dot((cameraPosition - worldPos.xyz), normal));
    shadowFactor = 0.5f + 0.5f * isBackFace;
   
    
  

    float shadowApply = step(texPos.z + bias, depth) * valid;

    float finalFactor = lerp(1.0f, shadowFactor, shadowApply);

    return diffuse * LightingColor * finalFactor + emissive;
    
    

}