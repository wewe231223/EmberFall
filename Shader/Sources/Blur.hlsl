
Texture2D gInput : register(t0);

RWTexture2D<float4> gRWOutput : register(u0);






static float gGaussianBlurMask1D[11] = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f };

groupshared float4 gGroupSharedCache[256 + 5 + 5];

[numthreads(256, 1, 1)]
void HorzBlur_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.x < 5)
    {
        int x = max(dispatchThreadID.x - 5, 0);
        gGroupSharedCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
    }
    else if (groupThreadID.x >= (256 - 5))
    {
        int x = min(dispatchThreadID.x + 5, gInput.Length.x - 1);
        gGroupSharedCache[groupThreadID.x + (2 * 5)] = gInput[int2(x, dispatchThreadID.y)];
    }
    gGroupSharedCache[groupThreadID.x + 5] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

    GroupMemoryBarrierWithGroupSync();

    float4 color = float4(0, 0, 0, 0);
    [unroll]
    for (int i = -5; i <= 5; i++)
        color += gGaussianBlurMask1D[i + 5] * gGroupSharedCache[groupThreadID.x + 5 + i];

    gRWOutput[dispatchThreadID.xy] = color;
}

[numthreads(1, 256, 1)]
void VertBlur_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    if (groupThreadID.y < 5)
    {
        int y = max(dispatchThreadID.y - 5, 0);
        gGroupSharedCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
    }
    else if (groupThreadID.y >= 256 - 5)
    {
        int y = min(dispatchThreadID.y + 5, gInput.Length.y - 1);
        gGroupSharedCache[groupThreadID.y + (2 * 5)] = gInput[int2(dispatchThreadID.x, y)];
    }
    gGroupSharedCache[groupThreadID.y + 5] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

    GroupMemoryBarrierWithGroupSync();

    float4 color = float4(0, 0, 0, 0);
    [unroll]
    for (int i = -5; i <= 5; i++)
        color += gGaussianBlurMask1D[i + 5] * gGroupSharedCache[groupThreadID.y + 5 + i];

    gRWOutput[dispatchThreadID.xy] = color;
}