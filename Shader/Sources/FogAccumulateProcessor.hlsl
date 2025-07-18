Texture3D<float4> Input : register(t0);
RWTexture3D<float4> RWOutput : register(u0);



static const float DensityScale = 0.01f;
static float fogBegin = 50.0f;
static float fogEnd = 200.0f;

float SliceTickness(float ndcZ, uint pixelZ)
{
    
    

    float result = pow(ndcZ + 1.f / float(pixelZ), 2) * (fogEnd - fogBegin) + fogBegin;
    
    float result2 = pow(ndcZ, 2) * (fogEnd - fogBegin) + fogBegin;
    
    return result - result2;
}

float4 ScatterStep(float3 light, float transmittance, float3 sliceLight, float sliceDensity, float tickness)
{
    sliceDensity = max(sliceDensity, 0.000001f);
    sliceDensity *= DensityScale;
    float sliceTransmittance = exp(-sliceDensity * tickness);

    float3 sliceLightIntegral = sliceLight * (1.f - sliceTransmittance) / sliceDensity;

    light += sliceLightIntegral * transmittance;
    transmittance *= sliceTransmittance;

    return float4(light, transmittance);
}

[numthreads(8, 8, 1)]
void FogAccumulateProcessor_CS(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint3 volumPixel;
    RWOutput.GetDimensions(volumPixel.x, volumPixel.y, volumPixel.z);

    if (all(dispatchThreadID < volumPixel))
    {
        float4 accum = float4(0.f, 0.f, 0.f, 1.f);
        uint3 pos = uint3(dispatchThreadID.xy, 0);

		
        for (uint z = 0; z < volumPixel.z; ++z)
        {
            pos.z = z;
            float4 slice = Input[pos];
            float sliceThickness = (fogEnd - fogBegin) / volumPixel.z;
            float tickness = SliceTickness((float) z / volumPixel.z, volumPixel.z);

            accum = ScatterStep(accum.rgb, accum.a, slice.rgb, slice.a, tickness);
            RWOutput[pos] = accum;
        }
    }
}