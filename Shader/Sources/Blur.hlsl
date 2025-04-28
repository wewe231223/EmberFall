
Texture2D gInput : register(t0);
Texture2D EmissiveGBuffer : register(t1);

RWTexture2D<float4> gRWOutput : register(u0);

static float gGaussianBlurMask1D[11] = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f };
static int threadGroupSize = 256;
static int maskWidth = 5;

groupshared float4 gGroupSharedCache[256 + 5 + 5];



[numthreads(threadGroupSize, 1, 1)]
void HorzBlur_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
   
    int leftEdge = (int)step(groupThreadID.x, 4);

    [unroll]
    for (int i = 0; i < 1 * leftEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.x] = gInput[int2(max(dispatchThreadID.x - 5, 0), dispatchThreadID.y)];
    }
    
    int rightEdge = (int)step(threadGroupSize - maskWidth, groupThreadID.x);
    
    [unroll]
    for (int i = 0; i < 1 * rightEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.x + (2 * 5)] = gInput[int2(min(dispatchThreadID.x + 5, gInput.Length.x - 1), dispatchThreadID.y)];

    }
    
    gGroupSharedCache[groupThreadID.x + 5] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];
    
    
   
    
    
        GroupMemoryBarrierWithGroupSync();

    float4 color = float4(0, 0, 0, 0);
    [unroll]
    for (int i = -5; i <= 5; i++)
        color += gGaussianBlurMask1D[i + 5] * gGroupSharedCache[groupThreadID.x + 5 + i];

    gRWOutput[dispatchThreadID.xy] = color;
}

[numthreads(1, threadGroupSize, 1)]
void VertBlur_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    
    
    int topEdge = (int) step(groupThreadID.y, 4);

    [unroll]
    for (int i = 0; i < 1 * topEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, max(dispatchThreadID.y - 5, 0))];
    }
    
    int bottomEdge = (int) step(threadGroupSize - maskWidth, groupThreadID.y);
    
    [unroll]
    for (int i = 0; i < 1 * bottomEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.y + (2 * 5)] = gInput[int2(dispatchThreadID.x, min(dispatchThreadID.y + 5, gInput.Length.x - 1))];

    }
    
    gGroupSharedCache[groupThreadID.y + 5] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];
    
    

    
    
    GroupMemoryBarrierWithGroupSync();

    float4 color = float4(0, 0, 0, 0);
    [unroll]
    for (int i = -5; i <= 5; i++)
        color += gGaussianBlurMask1D[i + 5] * gGroupSharedCache[groupThreadID.y + 5 + i];

    gRWOutput[dispatchThreadID.xy] = color;
}