cbuffer Camera : register(b0)
{
    matrix view;
    matrix projection;
    matrix viewProjection;
    float3 cameraPosition;
}

struct ModelContext
{
    matrix world;
    float3 BBCenter; 
    float3 BBExtents;
    uint material;
    uint boneStart;
};

struct BB_VIN
{
    uint instanceID : SV_INSTANCEID;
};

struct BB_GIN
{
    float3 center : CENTER;
    float3 extents : EXTENTS;
    float3 forward : FORWARD;
    float3 right : RIGHT;
    float3 up : UP;
};

struct BB_PIN
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

struct Deffered_POUT
{
    float4 diffuse : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 position : SV_TARGET2;
};


StructuredBuffer<ModelContext> modelContexts : register(t0);

SamplerState pointWrapSampler : register(s0);
SamplerState pointClampSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler : register(s3);
SamplerState anisotropicWrapSampler : register(s4);
SamplerState anisotropicClampSampler : register(s5);

BB_GIN BB_VS(BB_VIN input)
{
    BB_GIN output;
    ModelContext context = modelContexts[input.instanceID];
    
    output.extents = context.BBExtents;
    output.right = normalize(context.world[0].xyz);
    output.up = normalize(context.world[1].xyz);
    output.forward = normalize(context.world[2].xyz);
    output.center = context.BBCenter; 
    
    return output;
}

#define BoundingBoxColor    float4(1.f, 0.f, 0.f, 1.f)


[maxvertexcount(16)]
void BB_GS(point BB_GIN input[1], inout LineStream<BB_PIN> output)
{
    float3 center = input[0].center;
    float3 extents = input[0].extents;
    
    float3 TopPoints[4];
    float3 BottomPoints[4];
    // Top face (위쪽 면)
    TopPoints[0] = center + input[0].forward * extents.z - input[0].right * extents.x + input[0].up * extents.y; // 위 앞 왼쪽 
    TopPoints[1] = center + input[0].forward * extents.z + input[0].right * extents.x + input[0].up * extents.y; // 위 앞 오른쪽
    TopPoints[2] = center - input[0].forward * extents.z + input[0].right * extents.x + input[0].up * extents.y; // 위 뒤 오른쪽
    TopPoints[3] = center - input[0].forward * extents.z - input[0].right * extents.x + input[0].up * extents.y; // 위 뒤 왼쪽
    // Bottom face (아래쪽 면)
    BottomPoints[0] = center + input[0].forward * extents.z - input[0].right * extents.x - input[0].up * extents.y; // 아래 앞 왼쪽
    BottomPoints[1] = center + input[0].forward * extents.z + input[0].right * extents.x - input[0].up * extents.y; // 아래 앞 오른쪽
    BottomPoints[2] = center - input[0].forward * extents.z + input[0].right * extents.x - input[0].up * extents.y; // 아래 뒤 오른쪽
    BottomPoints[3] = center - input[0].forward * extents.z - input[0].right * extents.x - input[0].up * extents.y; // 아래 뒤 왼쪽
    
    
    BB_PIN outpoint;
    outpoint.color = BoundingBoxColor;
    
    // 윗면 엣지들 ( 점 4개 ) 
    [unroll(4)] 
    for (int i = 0; i < 4; ++i)
    {
        outpoint.position = mul(float4(TopPoints[i], 1.f), viewProjection);
        output.Append(outpoint);
    }
    //// 위 앞 왼쪽 - 아래 앞 왼쪽 엣지 ( 점 2개 )   
    outpoint.position = mul(float4(TopPoints[0], 1.f), viewProjection);
    output.Append(outpoint);
    outpoint.position = mul(float4(BottomPoints[0], 1.f), viewProjection);
    output.Append(outpoint);
    
    // 아랫면 엣지들 ( 점 4개 ) 
    [unroll(4)]
    for (int k = 1; k < 4; ++k)
    {
        outpoint.position = mul(float4(BottomPoints[k], 1.f), viewProjection);
        output.Append(outpoint);
    }
    // 아랫면 닫기 ( 점 1개 ) 
    outpoint.position = mul(float4(BottomPoints[0], 1.f), viewProjection);
    output.Append(outpoint);
    
    //// 별도 엣지들 ( 점 6개 ) 
    
    //// 앞 오른쪽 엣지 
    output.RestartStrip();
    outpoint.position = mul(float4(TopPoints[1], 1.f), viewProjection);
    output.Append(outpoint);
    outpoint.position = mul(float4(BottomPoints[1], 1.f), viewProjection);
    output.Append(outpoint);
    
    //// 뒤 왼쪽 엣지
    output.RestartStrip();
    outpoint.position = mul(float4(TopPoints[3], 1.f), viewProjection);
    output.Append(outpoint);
    outpoint.position = mul(float4(BottomPoints[3], 1.f), viewProjection);
    output.Append(outpoint);
    
    //// 뒤 오른쪽 엣지 
    output.RestartStrip();
    outpoint.position = mul(float4(TopPoints[2], 1.f), viewProjection);
    output.Append(outpoint);
    outpoint.position = mul(float4(BottomPoints[2], 1.f), viewProjection);
    output.Append(outpoint);
}


Deffered_POUT BB_PS(BB_PIN input)
{
    Deffered_POUT output = (Deffered_POUT) 0;
    
    output.diffuse = input.color;
    
    return output;
}