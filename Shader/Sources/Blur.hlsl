RWTexture2D<float4> RWOutput : register(u0);


//static const float gGaussianBlurMask1D[11] = { 0.05f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.05f };
//static const int maskWidth = 5;

static const float gGaussianBlurMask1D[15] = { 0.01f, 0.015f, 0.03f, 0.05f, 0.10f, 0.10f, 0.10f, 
                                                                 0.20f, 
                                             0.10f, 0.10f, 0.10f, 0.05f, 0.03f, 0.015f, 0.01f };
static const int maskWidth = 7;

//static const float gGaussianBlurMask1D[25] =
//{
//    0.0473f, 0.0472f, 0.0467f, 0.0459f, 0.0448f,
//    0.0434f, 0.0418f, 0.0399f, 0.0379f, 0.0357f,
//    0.0335f, 0.0311f, 0.0287f, 0.0311f, 0.0335f,
//    0.0357f, 0.0379f, 0.0399f, 0.0418f, 0.0434f,
//    0.0448f, 0.0459f, 0.0467f, 0.0472f, 0.0473f
//};
//static const int maskWidth = 12;

static const int threadGroupSize = 256;


groupshared float4 gGroupSharedCache[threadGroupSize + 2 * maskWidth];

[numthreads(threadGroupSize, 1, 1)]
void HorzBlur_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    //int2 uv = dispatchThreadID.xy;
    
    


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
    
    color.a = RWOutput[dispatchThreadID.xy].a;


    RWOutput[dispatchThreadID.xy] = color;
}

[numthreads(1, threadGroupSize, 1)]
void VertBlur_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    

    int topEdge = step(groupThreadID.y, maskWidth - 1);

    [unroll]
    for (int i = 0; i < 1 * topEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.y] = RWOutput[int2(dispatchThreadID.x, max(dispatchThreadID.y - maskWidth, 0))];
    }
    
    int bottomEdge = step(threadGroupSize - maskWidth, groupThreadID.y);
    
    [unroll]
    for (int i = 0; i < 1 * bottomEdge; ++i)
    {
        gGroupSharedCache[groupThreadID.y + (2 * maskWidth)] = RWOutput[int2(dispatchThreadID.x, min(dispatchThreadID.y + maskWidth, RWOutput.Length.y - 1))];

    }

    gGroupSharedCache[groupThreadID.y + maskWidth] = RWOutput[dispatchThreadID.xy];


    GroupMemoryBarrierWithGroupSync();

    float4 color;
   
    color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    [unroll]
    for (int i = -maskWidth; i <= maskWidth; ++i)
    {
        color += gGaussianBlurMask1D[i + maskWidth] * gGroupSharedCache[groupThreadID.y + maskWidth + i];
    }
    
    color.a = RWOutput[dispatchThreadID.xy].a;

    RWOutput[dispatchThreadID.xy] = color;

}
