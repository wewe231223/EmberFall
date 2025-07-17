
Texture2D<float4> Input : register(t0);
RWTexture2D<float4> RWOutput : register(u0);

#define WEIGHTS float3(0.3126, 0.6152, 0.0722)

static const float gGaussianBlurMask1D[11] = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f};
static const int maskWidth = 5;

//static const float gGaussianBlurMask1D[15] = {  0.01f,0.015f, 0.03f, 0.05f, 0.10f, 0.10f, 0.10f, 0.20f,0.10f,0.10f,0.10f,0.05f,0.03f,0.015f,0.01f };
//static const int maskWidth = 7;

//static const float gGaussianBlurMask1D[19] = { 0.0015f, 0.0038f, 0.0087f, 0.0180f, 0.0332f, 0.0548f, 0.0808f, 0.1067f, 0.1260f, 0.1332f, 0.1260f, 0.1067f, 0.0808f, 0.0548f, 0.0332f, 0.0180f, 0.0087f, 0.0038f, 0.0015f };
//static const int maskWidth = 9;



static const int threadGroupSize = 256;


groupshared float4 gGroupSharedCache[threadGroupSize + 2 * maskWidth];

[numthreads(threadGroupSize, 1, 1)]
void HorzBloom_CS( int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    int2 uv = dispatchThreadID.xy;
    
    
   
    float3 emissive = RWOutput[uv].rgb;
    float brightness = dot(emissive, WEIGHTS);
 
    
 
    float4 midColor = pow(float4(emissive, 1.0f), 4.0f);
    //float4 midColor = pow(float4(brightness, brightness, brightness, 1.0f), 4.0f);

    float mask1 = step(0.8f, brightness); 
    float mask0 = step(0.5f, brightness); 


    float4 result = lerp(float4(0.0, 0.0, 0.0, 0.0), midColor, mask0);
    result = lerp(result, RWOutput[uv], mask1);

    RWOutput[uv] = result;

    emissive = Input[uv].rgb;
    float useEmissiveMap = step(0.00001f, length(emissive));
    [unroll]
    for (int i = 0; i < useEmissiveMap; ++i)
    {
       RWOutput[uv] = float4(emissive, 1.0f);

    }
    
    AllMemoryBarrierWithGroupSync();

    int leftEdge = step(groupThreadID.x, maskWidth - 1);

    [unroll]
    for (int i = 0; i < 1 * leftEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.x] = RWOutput[int2(max(dispatchThreadID.x - maskWidth, 0), dispatchThreadID.y)];
    }
    
    int rightEdge = step(threadGroupSize - maskWidth, groupThreadID.x);
    
    [unroll]
    for (int i = 0; i < 1 * rightEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.x + (2 * maskWidth)] = RWOutput[int2(min(dispatchThreadID.x + maskWidth, RWOutput.Length.x - 1), dispatchThreadID.y)];

    }

    gGroupSharedCache[groupThreadID.x + maskWidth] = RWOutput[dispatchThreadID.xy];


    GroupMemoryBarrierWithGroupSync();


    float4 color;
    
    color = float4(0, 0, 0, 0);
    [unroll]
    for (int i = -maskWidth; i <= maskWidth; ++i)
    {
        color += gGaussianBlurMask1D[i + maskWidth] * gGroupSharedCache[groupThreadID.x + maskWidth + i];
    }
        
    
   
   


    RWOutput[dispatchThreadID.xy] = color;
}

[numthreads(1, threadGroupSize, 1)]
void VertBloom_CS( int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    

    int topEdge = step(groupThreadID.y, maskWidth - 1);

    [unroll]
    for (int i = 0; i < 1 * topEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.y] = Input[int2(dispatchThreadID.x, max(dispatchThreadID.y - maskWidth, 0))];
    }
    
    int bottomEdge = step(threadGroupSize - maskWidth, groupThreadID.y);
    
    [unroll]
    for (int i = 0; i < 1 * bottomEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.y + (2 * maskWidth)] = Input[int2(dispatchThreadID.x, min(dispatchThreadID.y + maskWidth, Input.Length.y - 1))];

    }

    gGroupSharedCache[groupThreadID.y + maskWidth] = Input[dispatchThreadID.xy];


    GroupMemoryBarrierWithGroupSync();

    float4 color;
   
    color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll]
    for (int i = -maskWidth; i <= maskWidth; ++i)
    {
       color += gGaussianBlurMask1D[i + maskWidth] * gGroupSharedCache[groupThreadID.y + maskWidth + i];
    }
    
        
    
    RWOutput[dispatchThreadID.xy] += color;
    //RWOutput[dispatchThreadID.xy] = color;

}
