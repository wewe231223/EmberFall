Texture2D<float4> Input : register(t0);
Texture2D<float4> velocity : register(t1);
RWTexture2D<float4> RWOutput : register(u0);


[numthreads(16, 16, 1)]
void MotionBlur_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
    float2 ScreenSize = float2(1920.0f, 1080.0f);
    int SampleCount = 4; 
    
    
    int2 pix = dispatchThreadID.xy;

    //float4 baseColor = Input.Load(int3(pix, 0));
    float2 vel = velocity.Load(int3(pix, 0)).xy;
    
    
    vel = vel * ScreenSize;

    float4 accum = float4(0, 0, 0, 0);
    float weightSum = 0;

    
    [unroll]
    for (int i = 0; i <= SampleCount; ++i)
    {
        float t = i / float(SampleCount); 
        float2 offset = vel * t; 

        int2 coord = pix + int2(offset + 0.5);
        coord = clamp(coord, int2(0, 0), int2(ScreenSize) - 1);

        float4 sample = Input.Load(int3(coord, 0));

        float w = exp(-t * t * 4.0);

        accum += sample * w;
        weightSum += w;
    }

    RWOutput[pix] = accum / weightSum;
}