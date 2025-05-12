#define ParticleType_emit   1
#define ParticleType_shell  2
#define ParticleType_ember  3

#define ember_LifeTime      2.f

#define RANDOM_BUFFER_SIZE  4096
#define NULL_INDEX 0xFFFFFFFF

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
    
    float3 direction : DIRECTION;
    float3 velocity : VELOCITY;
    
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;

    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;

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
    
    float3 direction : DIRECTION;
    float3 velocity : VELOCITY;
    
    float totalLifetime : TOTALLIFETIME;
    float lifetime : LIFETIME;
    float halfHeight : HEIGHT;
    uint material : MATERIAL;

    uint spritable : SPRITABLE;
    uint spriteFrameInRow : SPRITEFRAMEINROW;
    uint spriteFrameInCol : SPRITEFRAMEINCOL;
    float spriteDuration : SPRITEDURATION;

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

float GenerateRandomInRange(float min, float max, uint seed)
{
    float r = GenerateRandom(seed);
    return lerp(min, max, r);
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

#define GRAVITY_CONST 9.8f

void ApplyPhysics(inout ParticleVertex v)
{
    // 중력 가속도 (y축 방향)
    const float3 gravity = float3(0.0f, -GRAVITY_CONST, 0.0f);
    // 선형 드래그 가속도: a_drag = -drag * velocity / mass
    float3 a_drag = -v.drag * v.velocity / v.mass;
    // 총 가속도
    float3 accel = gravity + a_drag;
    // 속도 적분
    v.velocity += accel * deltaTime;
    // 위치 적분
    v.position += v.velocity * deltaTime;
}

void OnTerrain(inout ParticleVertex v)
{
    float h = GetHeight(v.position.x, v.position.z);
    if (v.position.y < h + v.halfHeight)
    {
        v.position.y = h + v.halfHeight;
        v.velocity = float3(0.0f, 0.0f, 0.0f);
    }
}

//----------------------------------------------------------[ Emit Particle Update ]----------------------------------------------------------

void EmitParticleUpdate(inout ParticleVertex emitter, uint vertexID, inout PointStream<ParticleVertex> stream)
{
    // 에미터 위치 갱신
    emitter.position = EmitPosition[emitter.emitIndex].position;

    // 타이머 만료 시 새로운 입자 생성
    if (emitter.lifetime <= 0.0f && emitter.remainEmit != 0)
    {
        ParticleVertex p = (ParticleVertex) 0;
        p.position = emitter.position;
        p.halfWidth = emitter.halfWidth;
        p.halfHeight = emitter.halfHeight;
        p.material = emitter.material;
        
        p.spritable = emitter.spritable;
        p.spriteFrameInRow = emitter.spriteFrameInRow;
        p.spriteFrameInCol = emitter.spriteFrameInCol;
       
        
        p.opacity = 1.0f;

        // 랜덤 방향 생성 (수평 확산 위주, 약간의 상승 성분만)
        float3 dir = GenerateRandomDirection(vertexID);
        dir.y = abs(dir.y); // 아래로 떨어지는 방향 제거
        dir.y *= 0.3f; // 상승 성분 약화
        dir = normalize(dir);

        p.direction = dir;
        // 초기 속도: 1 ~ 2.5 범위의 랜덤
        float speed = GenerateRandomInRange(1.0f, 2.5f, vertexID + 1);
        p.velocity = dir * speed;

        // 물리 파라미터
        p.mass = emitter.mass;
        p.drag = emitter.drag;

        // 수명
        p.totalLifetime = ember_LifeTime;
        p.lifetime = 1.f;
        p.spriteDuration = p.totalLifetime;
        
        // 파티클 타입 설정
        p.type = ParticleType_ember;
        p.emitType = ParticleType_ember;
        p.remainEmit = 0;
        p.emitIndex = emitter.emitIndex;

        // 에미터 리셋
        emitter.lifetime = emitter.totalLifetime;
        if (emitter.remainEmit > 0)
            emitter.remainEmit--;

        stream.Append(p);
    }

    // 에미터 라이프 타이머 감소 및 스트림에 다시 추가
    emitter.lifetime -= deltaTime;
    stream.Append(emitter);
}



//----------------------------------------------------------[ Ember Particle Update ]----------------------------------------------------------

void EmberParticleUpdate(inout ParticleVertex v, inout PointStream<ParticleVertex> stream)
{
    if (v.lifetime >= 0.f)
    {
        ParticleVertex n = v;

        ApplyPhysics(n); // 중력 + 공기저항
        OnTerrain(n); // 지면 충돌 처리

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

[maxvertexcount(16)]
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
        EmitParticleUpdate(outP, input[0].vertexID, output);
    }
    else if (outP.type == ParticleType_ember)
    {
        EmberParticleUpdate(outP, output);
    }
}