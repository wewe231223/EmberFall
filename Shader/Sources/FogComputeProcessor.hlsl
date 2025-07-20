SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);
SamplerComparisonState PCFSampler : register(s6);


cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
    float4x4 middleViewProjection;
    float4x4 prevViewProjection;
    float4x4 invView;
    float4x4 invProjection;

    float3 cameraPosition;
    int isShadow;
    float3 shadowOffset;
};

#define MAX_LIGHT_COUNT 8 
#define LightType_Directional   1 
#define LightType_Point         2
#define LightType_Spot          3


static float density = 5.0f;

   
static float4 HemisphereColor = float4(0.015f, 0.07f, 0.095f, 1.0f);

static float Intensity = 1.0f;

static float fogBegin = 0.0f;
static float fogEnd = 100.0f;

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
RWTexture3D<float4> RWOutput : register(u0);
Texture2D shadowMaps[2] : register(t0);
StructuredBuffer<Light> gLight : register(t2);

float3 ComputeWorldPosition(int3 dispatchThreadID, int3 volumePixel)
{
    float3 ndc = dispatchThreadID;
    ndc += 0.5f;
    ndc *= float3(2.0f / volumePixel.x, -2.0f / volumePixel.y, 1.0f / volumePixel.z);
    ndc += float3(-1.0f, 1.0f, 0.0f);
    
    float depth = ndc.z * (fogEnd - fogBegin) + fogBegin;
    
    float4 viewRay = mul(float4(ndc, 1.0f), invProjection);
    viewRay /= viewRay.w;
    viewRay /= viewRay.z; 

	
    float4 worldPosition = mul(float4(viewRay.xyz * depth, 1.0f), invView);

    return worldPosition.xyz;
}





[numthreads(8, 8, 8)]
void FogComputeProcessor_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int3 volumePixel;
    
    RWOutput.GetDimensions(volumePixel.x, volumePixel.y, volumePixel.z);
    
    if (all(dispatchThreadID < volumePixel))
    {
        float3 worldPos = ComputeWorldPosition(dispatchThreadID, volumePixel);
        float3 toCamera = normalize(cameraPosition - worldPos);
        
        float3 hemisphereLight = HemisphereColor.rgb * HemisphereColor.a;
        
        for (int i = 0; i < MAX_LIGHT_COUNT; ++i)
        {
            float3 lightDir;
            float3 toLight;
            if (gLight[i].lightType == LightType_Directional)
            {
                lightDir = -normalize(gLight[i].Direction); //조명이 들어오는 방향이 저장되어있으므로 음수로 전환
                toLight = -lightDir;


            }
            else if (gLight[i].lightType == LightType_Point || gLight[i].lightType == LightType_Spot)
            {
                lightDir = normalize(worldPos - gLight[i].Position);
                toLight = -lightDir;


            }
            
            float visibility = 1.0f;
            
            
            float cosTheta = dot(lightDir, toCamera);
            float g2 = 0.7f * 0.7f;
            float denom = pow(1.f + g2 - 2.f * 0.7f * cosTheta, 3.f / 2.f);
            float phaseFuntion = (1.f / (4.f * 3.14f)) * ((1.f - g2) / denom );
            
            hemisphereLight += visibility * gLight[i].Diffuse.rgb * gLight[i].Diffuse.a * phaseFuntion;

        }
        RWOutput[dispatchThreadID] = float4(hemisphereLight * Intensity * density, density);

    }

}