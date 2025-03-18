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

