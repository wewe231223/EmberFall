#define ParticleType_emit   1
#define ParticleType_shell  2
#define ParticleType_ember  3
#define ParticleType_smoke  4
#define ParticleType_explode 5 
#define ParticleType_path 6 
#define ParticleType_blood 7
#define ParticleType_magicPath 8
#define ParticleType_magicExplode 9 
 
#define ember_LifeTime      6.f

#define ParticleFlag_Delete 2

#define RANDOM_BUFFER_SIZE  4096
#define NULL_INDEX 0xFFFFFFFF

#define MAX_STREAM_SIZE 37

cbuffer GlobalCB : register(b0)
{
    float globalTime; // 초 단위 
    float deltaTime; // 초 단위 
}

StructuredBuffer<float> RandomBuffer : register(t0);

struct EmitParticleContext
{
    float3 position;
    uint flag;
};

StructuredBuffer<EmitParticleContext> EmitPosition : register(t1);

cbuffer TerrainGlobal : register(b1)
{
    int globalWidth;
    int globalHeight;
    float gridSpacing;
    float minX;
    float minZ;
};

StructuredBuffer<float3> TerrainVertices : register(t2);

struct ParticleVertex
{
    float3 position : POSITION;
    float halfWidth : WIDTH;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;
    
    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;

    float3 direction : DIRECTION;
    float3 velocity : VELOCITY;
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;

    uint type : PARTICLETYPE;
    uint emitType : EMITTYPE;
    uint remainEmit : REMAINEMIT;
    uint emitIndex : EMITINDEX;

    float mass : MASS;
    float3 drag : DRAG;
    float opacity : OPACITY;
};

struct ParticleSO_GS_IN
{
    float3 position : POSITION;
    float halfWidth : WIDTH;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;
    
    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;

    float3 direction : DIRECTION;
    float3 velocity : VELOCITY;
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;
    
    uint type : PARTICLETYPE;
    uint emitType : EMITTYPE;
    uint remainEmit : REMAINEMIT;
    uint emitIndex : EMITINDEX;

    float mass : MASS;
    float3 drag : DRAG;
    float opacity : OPACITY;
    
    uint vertexID : VERTEXID;
};

//----------------------------------------------------------[ Random ]----------------------------------------------------------

uint FastHash(uint seed)
{
    uint hash = seed * 1665u + 101423u;
    return hash ^ (hash >> 16);
}

float FetchRandomValue(uint index)
{
    return RandomBuffer[index % RANDOM_BUFFER_SIZE];
}

float GenerateRandom(uint seed)
{
    uint timeSeed = uint(globalTime * 1000.f);
    uint combinedSeed = FastHash(seed) ^ FastHash(timeSeed);
    return FetchRandomValue(combinedSeed);
}

uint GenerateRandomUint(uint seed)
{
    uint timeSeed = uint(globalTime * 1000.f);
    uint combinedSeed = FastHash(seed) ^ FastHash(timeSeed);
    float randValue = RandomBuffer[combinedSeed % RANDOM_BUFFER_SIZE];
    return asuint(randValue);
}

float GenerateRandomInRange(float min, float max, uint seed)
{
    float r = GenerateRandom(seed);
    return lerp(min, max, r);
}

uint GenerateRandomUintInRange(uint minValue, uint maxValue, uint seed)
{
    uint raw = GenerateRandomUint(seed);
    return minValue + (raw % (maxValue - minValue + 1));
}

float3 GenerateRandomDirection(uint seed)
{
    float rand1 = GenerateRandom(seed) * 2.0 - 1.0;
    float rand2 = GenerateRandom(seed + 1) * 2.0 - 1.0;
    float z = rand1;
    float xy = sqrt(max(1.0 - z * z, 0.0));
    float theta = rand2 * 3.14159;
    float3 dir = float3(xy * cos(theta), xy * sin(theta), z);
    return normalize(dir);
}

//----------------------------------------------------------[ Terrain Height ]----------------------------------------------------------

float GetHeight(float x, float z)
{
    float localX = x - minX;
    float localZ = z - minZ;
    float fcol = localX / gridSpacing;
    float frow = localZ / gridSpacing;
    int col = clamp((int) fcol, 0, globalWidth - 2);
    int row = clamp((int) frow, 0, globalHeight - 2);
    float t = fcol - col;
    float u = frow - row;
    int idx00 = row * globalWidth + col;
    int idx10 = idx00 + 1;
    int idx01 = idx00 + globalWidth;
    int idx11 = idx01 + 1;
    float y0 = lerp(TerrainVertices[idx00].y, TerrainVertices[idx10].y, t);
    float y1 = lerp(TerrainVertices[idx01].y, TerrainVertices[idx11].y, t);
    return lerp(y0, y1, u);
}

//----------------------------------------------------------[ Physics Helpers ]----------------------------------------------------------

void UpdateParticle(inout ParticleVertex p, float deltaTime)
{
    // --- 중력 상수: 지구 기준 (m/s²), Y-축 아래로 향함 ---
    const float3 GRAVITY_ACCEL = float3(0.0f, -9.81f, 0.0f); // 상수로 내장

    // --- 중력 가속도 (F = m * g) ---
    float3 gravityForce = GRAVITY_ACCEL * p.mass;
    float3 gravityAccel = gravityForce / max(p.mass, 0.0001f); // 또는 그냥 GRAVITY_ACCEL

    // --- 항력 (drag = -k * velocity) ---
    float3 dragForce = -p.drag * p.velocity;
    float3 dragAccel = dragForce / max(p.mass, 0.0001f);

    // --- 전체 가속도 = 중력 + 항력 ---
    float3 totalAccel = gravityAccel + dragAccel;

    // --- 속도/위치 업데이트 ---
    p.velocity += totalAccel * deltaTime;
    p.position += p.velocity * deltaTime;

    // --- 투명도 선형 감소 ---
    p.opacity = saturate(p.lifetime / p.totalLifetime);
}


void OnTerrain(inout ParticleVertex v)
{
    float h = GetHeight(v.position.x, v.position.z);
    if (v.position.y < h + v.halfHeight * 0.5f)
    {
        v.position.y = h + v.halfHeight * 0.5f;
    }
}

void OnTerrainStop(inout ParticleVertex v)
{
    float h = GetHeight(v.position.x, v.position.z);
    if (v.position.y < h + v.halfHeight * 0.5f)
    {
        v.position.y = h + v.halfHeight * 0.5f;
        v.velocity = float3(0.0f, 0.0f, 0.0f); // 속도 초기화 
    }
}


//----------------------------------------------------------[ Emit Particle Update ]----------------------------------------------------------

uint CreateSmokeParticle(ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    ParticleVertex p = (ParticleVertex) 0;

    p.position = emitter.position;

    p.halfWidth = GenerateRandomInRange(0.3f, 0.5f, vertexID);
    p.halfHeight = p.halfWidth;

    p.material = emitter.material;

    p.spritable = emitter.spritable;
    p.spriteFrameInRow = emitter.spriteFrameInRow;
    p.spriteFrameInCol = emitter.spriteFrameInCol;
    p.spriteDuration = ember_LifeTime;

    p.opacity = 1.0f;

    p.mass = 0.5f;
    p.drag = float3(0.1f, 10.0f, 0.1f);

    p.totalLifetime = ember_LifeTime;
    p.lifetime = ember_LifeTime;

    p.type = ParticleType_smoke;
    p.emitType = ParticleType_ember;
    p.remainEmit = 0;
    p.emitIndex = emitter.emitIndex;

    float spinAngle = GenerateRandomInRange(0.0f, 6.28318f, vertexID); // 0 ~ 2π
    
    static const float3 baseDirs[36] =
    {
    float3(1.000000f, 0.0f, 0.000000f),
    float3(0.984808f, 0.0f, 0.173648f),
    float3(0.939693f, 0.0f, 0.342020f),
    float3(0.866025f, 0.0f, 0.500000f),
    float3(0.766044f, 0.0f, 0.642788f),
    float3(0.642788f, 0.0f, 0.766044f),
    float3(0.500000f, 0.0f, 0.866025f),
    float3(0.342020f, 0.0f, 0.939693f),
    float3(0.173648f, 0.0f, 0.984808f),
    float3(0.000000f, 0.0f, 1.000000f),
    float3(-0.173648f, 0.0f, 0.984808f),
    float3(-0.342020f, 0.0f, 0.939693f),
    float3(-0.500000f, 0.0f, 0.866025f),
    float3(-0.642788f, 0.0f, 0.766044f),
    float3(-0.766044f, 0.0f, 0.642788f),
    float3(-0.866025f, 0.0f, 0.500000f),
    float3(-0.939693f, 0.0f, 0.342020f),
    float3(-0.984808f, 0.0f, 0.173648f),
    float3(-1.000000f, 0.0f, 0.000000f),
    float3(-0.984808f, 0.0f, -0.173648f),
    float3(-0.939693f, 0.0f, -0.342020f),
    float3(-0.866025f, 0.0f, -0.500000f),
    float3(-0.766044f, 0.0f, -0.642788f),
    float3(-0.642788f, 0.0f, -0.766044f),
    float3(-0.500000f, 0.0f, -0.866025f),
    float3(-0.342020f, 0.0f, -0.939693f),
    float3(-0.173648f, 0.0f, -0.984808f),
    float3(0.000000f, 0.0f, -1.000000f),
    float3(0.173648f, 0.0f, -0.984808f),
    float3(0.342020f, 0.0f, -0.939693f),
    float3(0.500000f, 0.0f, -0.866025f),
    float3(0.642788f, 0.0f, -0.766044f),
    float3(0.766044f, 0.0f, -0.642788f),
    float3(0.866025f, 0.0f, -0.500000f),
    float3(0.939693f, 0.0f, -0.342020f),
    float3(0.984808f, 0.0f, -0.173648f)
    };
    
    
    [unroll]
    for (int i = 0; i < 36; ++i)
    {
        float baseAngle = (2.f * 3.141592f * i) / 36.f;
        float3 baseDir = baseDirs[i]; 

        float cosA = cos(spinAngle);
        float sinA = sin(spinAngle);

        float3 rotatedDir;
        rotatedDir.x = cosA * baseDir.x - sinA * baseDir.z;
        rotatedDir.z = sinA * baseDir.x + cosA * baseDir.z;
        rotatedDir.y = GenerateRandomInRange(1.0f, 2.5f, vertexID + i + 10);

        rotatedDir = normalize(rotatedDir);
        p.direction = rotatedDir;

        float speed = GenerateRandomInRange(1.5f, 3.f, vertexID + i + 100);
        p.velocity = p.direction * speed;
        p.velocity.y *= 4.f;

        OnTerrain(p);
        stream.Append(p);
    }
    
    return 36; 
}


uint CreateExplodeParticle(ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    ParticleVertex p = (ParticleVertex) 0;

    const float lifeTime = 1.5f; 
    
    p.position = emitter.position;

    p.halfWidth = GenerateRandomInRange(0.5f, 1.f, vertexID);
    p.halfHeight = p.halfWidth;

    p.material = emitter.material;

    p.spritable = emitter.spritable;
    p.spriteFrameInRow = emitter.spriteFrameInRow;
    p.spriteFrameInCol = emitter.spriteFrameInCol;
    p.spriteDuration = lifeTime;

    p.opacity = 1.0f;

    p.mass = 0.5f;
    p.drag = float3(0.1f, 0.1f, 0.1f);

    p.totalLifetime = lifeTime;
    p.lifetime = lifeTime;

    p.type = ParticleType_smoke;
    p.emitType = ParticleType_ember;
    p.remainEmit = 0;
    p.emitIndex = emitter.emitIndex;
  
    
     [unroll]
    for (int i = 0; i < 25; ++i)
    {
        p.direction = GenerateRandomDirection(vertexID + i);
        

        float speed = GenerateRandomInRange(3.f, 5.5f, vertexID + i + 100);
        p.velocity = p.direction * speed;

        OnTerrain(p);
        stream.Append(p);
    } 
    
    return 25; 
}

uint CreatePathParticle(ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    ParticleVertex p = (ParticleVertex) 0;

    const float lifeTime = 0.5f;
    
    p.position = emitter.position;

    p.halfWidth = GenerateRandomInRange(0.3f, 0.5f, vertexID);
    p.halfHeight = p.halfWidth;

    p.material = emitter.material;

    p.spritable = emitter.spritable;
    p.spriteFrameInRow = emitter.spriteFrameInRow;
    p.spriteFrameInCol = emitter.spriteFrameInCol;
    p.spriteDuration = lifeTime;

    p.opacity = 1.0f;

    p.mass = 0.f;
    p.drag = float3(0.f,0.f,0.f);

    p.totalLifetime = lifeTime;
    p.lifetime = lifeTime;

    p.type = ParticleType_smoke;
    p.emitType = ParticleType_ember;
    p.remainEmit = 0;
    p.emitIndex = emitter.emitIndex;
  
    
     [unroll]
    for (int i = 0; i < 5; ++i)
    {
        p.direction = GenerateRandomDirection(vertexID + i);
        

        float speed = GenerateRandomInRange(0.f, 1.f, vertexID + i + 100);
        p.velocity = p.direction * speed;

        OnTerrain(p);
        stream.Append(p);
    }
    
    return 5;
}

uint CreateMagicPathParticle(ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    ParticleVertex p = (ParticleVertex) 0;

    const float lifeTime = 0.5f;
    
    p.position = emitter.position;

    p.halfWidth = GenerateRandomInRange(0.3f, 0.5f, vertexID);
    p.halfHeight = p.halfWidth;

    p.material = emitter.material;

    p.spritable = emitter.spritable;
    p.spriteFrameInRow = emitter.spriteFrameInRow;
    p.spriteFrameInCol = emitter.spriteFrameInCol;
    p.spriteDuration = lifeTime;

    p.opacity = 1.0f;

    p.mass = 0.f;
    p.drag = float3(0.f, 0.f, 0.f);

    p.totalLifetime = lifeTime;
    p.lifetime = lifeTime;

    p.type = ParticleType_smoke;
    p.emitType = ParticleType_ember;
    p.remainEmit = 0;
    p.emitIndex = emitter.emitIndex;
  
    
     [unroll]
    for (int i = 0; i < 25; ++i)
    {
        p.direction = GenerateRandomDirection(vertexID + i);
        

        float speed = GenerateRandomInRange(3.f, 5.f, vertexID + i + 100);
        p.velocity = p.direction * speed;

        OnTerrain(p);
        stream.Append(p);
    }
    
    return 25;
}

uint CreateMagicExplodeParticle(ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    ParticleVertex p = (ParticleVertex) 0;

    const float lifeTime = 1.5f;
    
    p.position = emitter.position;

    p.halfWidth = GenerateRandomInRange(2.f, 4.f, vertexID);
    p.halfHeight = p.halfWidth;

    p.material = emitter.material;

    p.spritable = emitter.spritable;
    p.spriteFrameInRow = emitter.spriteFrameInRow;
    p.spriteFrameInCol = emitter.spriteFrameInCol;
    p.spriteDuration = lifeTime;

    p.opacity = 1.0f;

    p.mass = 1.f;
    p.drag = float3(0.f, 0.f, 0.f);

    p.totalLifetime = lifeTime;
    p.lifetime = lifeTime;

    p.type = ParticleType_magicExplode;
    p.emitType = ParticleType_ember;
    p.emitIndex = emitter.emitIndex;
  
    
     [unroll]
    for (int i = 0; i < 36; ++i)
    {
        p.direction = GenerateRandomDirection(vertexID + i);

        p.remainEmit = GenerateRandomUintInRange(0, 7, vertexID + i);

        float speed = GenerateRandomInRange(6.f, 8.f, vertexID + i + 100);
        p.velocity = p.direction * speed;

        OnTerrain(p);
        stream.Append(p);
    }
    
    return 36;
}

uint CreateBloodParticle(ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    ParticleVertex p = (ParticleVertex) 0;

    const float lifeTime = 2.3f;
    
    p.position = emitter.position;

    p.halfWidth = GenerateRandomInRange(0.1f, 0.3f, vertexID);
    p.halfHeight = p.halfWidth;

    p.material = emitter.material;

    p.spritable = emitter.spritable;
    p.spriteFrameInRow = emitter.spriteFrameInRow;
    p.spriteFrameInCol = emitter.spriteFrameInCol;
    p.spriteDuration = lifeTime;

    p.opacity = 1.0f;

    p.mass = 0.5f;
    p.drag = float3(0.1f, 0.1f, 0.1f);

    p.totalLifetime = lifeTime;
    p.lifetime = lifeTime;

    p.type = ParticleType_blood;
    p.emitType = ParticleType_ember;
    p.remainEmit = 0;
    p.emitIndex = emitter.emitIndex;
  
    
     [unroll]
    for (int i = 0; i < 36; ++i)
    {
        p.direction = GenerateRandomDirection(vertexID + i);
        

        float speed = GenerateRandomInRange(2.f, 3.5f, vertexID + i + 100);
        p.velocity = p.direction * speed;

        OnTerrain(p);
        stream.Append(p);
    }
    
    return 36;
}

void EmitParticleUpdate(inout ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{    
    if (emitter.emitIndex != NULL_INDEX)
    {
        if (EmitPosition[emitter.emitIndex].flag == ParticleFlag_Delete)
        {
            return; 
        }
    }
    
    ParticleVertex v = emitter;
    // 에미터 위치 갱신
    int remain = v.remainEmit;
    
    if (v.lifetime <= 0.0f && v.remainEmit > 0 && globalTime != 0.f)
    {        
        uint emitCount = 0;
        
        switch (v.emitType)
        {
            case ParticleType_smoke:
                emitCount = CreateSmokeParticle(v, vertexID, stream);
                break;
            case ParticleType_explode:
                emitCount = CreateExplodeParticle(v, vertexID, stream);
                break;
            case ParticleType_path:
                emitCount = CreatePathParticle(v, vertexID, stream); 
                break;
            case ParticleType_blood:
                emitCount = CreateBloodParticle(v, vertexID, stream);
                break;
            case ParticleType_magicPath:
                emitCount = CreateMagicPathParticle(v, vertexID, stream);
                break;
            case ParticleType_magicExplode:
                emitCount = CreateMagicExplodeParticle(v, vertexID, stream);
                break; 

        }
        
        
        
        
        v.lifetime = v.totalLifetime;
        remain -= emitCount;

    }

    
    if (remain < 0)
    {
        return; 
    }
    
    v.remainEmit = remain;
    stream.Append(v);
}



//----------------------------------------------------------[ Ember Particle Update ]----------------------------------------------------------

void EmberParticleUpdate(inout ParticleVertex v, inout PointStream<ParticleVertex> stream)
{
    if (v.lifetime >= 0.f)
    {
        ParticleVertex n = v;

        UpdateParticle(n, deltaTime);
        
        if (v.type == ParticleType_blood)
        {
            OnTerrainStop(n); // 지면 충돌 처리
        }
        else
        {
            OnTerrain(n); // 지면 충돌 처리
        }
        

        stream.Append(n);
    }
}

//----------------------------------------------------------[ Stream-Out VS ]----------------------------------------------------------

ParticleSO_GS_IN ParticleSOPassVS(ParticleVertex inV, uint vid : SV_VertexID)
{
    ParticleSO_GS_IN o = (ParticleSO_GS_IN) 0;
    o.position = inV.position;
    o.halfWidth = inV.halfWidth;
    o.halfHeight = inV.halfHeight;
    o.material = inV.material;
    o.spritable = inV.spritable;
    o.spriteFrameInRow = inV.spriteFrameInRow;
    o.spriteFrameInCol = inV.spriteFrameInCol;
    o.spriteDuration = inV.spriteDuration;
    o.direction = inV.direction;
    o.velocity = inV.velocity;
    o.totalLifetime = inV.totalLifetime;
    o.lifetime = inV.lifetime;
    o.type = inV.type;
    o.emitType = inV.emitType;
    o.remainEmit = inV.remainEmit;
    o.emitIndex = inV.emitIndex;
    o.vertexID = vid;
    o.mass = inV.mass;
    o.drag = inV.drag;
    o.opacity = inV.opacity;
    
    return o;
}

//----------------------------------------------------------[ Stream-Out GS ]----------------------------------------------------------

[maxvertexcount(MAX_STREAM_SIZE)]
void ParticleSOPassGS(point ParticleSO_GS_IN input[1], inout PointStream<ParticleVertex> output)
{
    ParticleVertex outP = (ParticleVertex) 0;
    outP.position = input[0].position;
    outP.halfWidth = input[0].halfWidth;
    outP.halfHeight = input[0].halfHeight;
    outP.material = input[0].material;
    outP.spritable = input[0].spritable;
    outP.spriteFrameInRow = input[0].spriteFrameInRow;
    outP.spriteFrameInCol = input[0].spriteFrameInCol;
    outP.spriteDuration = input[0].spriteDuration;
    outP.direction = input[0].direction;
    outP.velocity = input[0].velocity;
    outP.totalLifetime = input[0].totalLifetime;
    outP.lifetime = input[0].lifetime - deltaTime;
    outP.type = input[0].type;
    outP.emitType = input[0].emitType;
    outP.remainEmit = input[0].remainEmit;
    outP.emitIndex = input[0].emitIndex;
    outP.mass = input[0].mass;
    outP.drag = input[0].drag;
    outP.opacity = input[0].opacity;

    if (outP.type == ParticleType_emit)
    {
        if (outP.emitIndex != NULL_INDEX)
        {
            outP.position = EmitPosition[outP.emitIndex].position;
        }
        EmitParticleUpdate(outP, input[0].vertexID, output);
    }
    else {
        EmberParticleUpdate(outP, output);
    }
}
