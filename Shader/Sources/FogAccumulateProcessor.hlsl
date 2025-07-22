Texture3D<float4> Input : register(t0);
RWTexture3D<float4> RWOutput : register(u0);



static const float DensityOffset = 0.0006f;
static float fogBegin = 0.0f;
static float fogEnd = 700.0f;

float ComputeSliceDepthDelta(float ndcZ, uint pixelZ)
{
    
    

    float result = pow(ndcZ + 1.0f / float(pixelZ), 2.0f) * (fogEnd - fogBegin) + fogBegin;
    
    float result2 = pow(ndcZ, 2.0f) * (fogEnd - fogBegin) + fogBegin;
    
    return result - result2;
}

float4 AccumulateScattering(float3 accumLight, float accumTransmittance, float3 sliceLight, float sliceDensity, float thickness)
{
    sliceDensity = max(sliceDensity, 0.000001f);
    sliceDensity *= DensityOffset;
    float sliceTransmittance = exp(-sliceDensity * thickness);

    sliceLight = sliceLight * (1.0f - sliceTransmittance) / sliceDensity;

    accumLight += sliceLight * accumTransmittance;
    accumTransmittance *= sliceTransmittance;

    return float4(accumLight, accumTransmittance);
}

[numthreads(8, 8, 1)]
void FogAccumulateProcessor_CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint3 volumPixel;
    RWOutput.GetDimensions(volumPixel.x, volumPixel.y, volumPixel.z);

    if (all(dispatchThreadID < volumPixel))
    {
        uint3 pos = uint3(dispatchThreadID.xy, 0);
        float4 accumulationLight = float4(0.0f, 0.0f, 0.0f, 1.0f);

		
        for (uint z = 0; z < volumPixel.z; ++z)
        {
            pos.z = z;
            float4 slicePixel = Input[pos];
            float thickness = ComputeSliceDepthDelta((float) z / volumPixel.z, volumPixel.z);

            accumulationLight = AccumulateScattering(accumulationLight.rgb, accumulationLight.a, slicePixel.rgb, slicePixel.a, thickness);
            RWOutput[pos] = accumulationLight;
        }
    }
}