#pragma once 
#include <type_traits>
#include <tuple>
#include <bitset>
#include <array>
#include <memory>
#include "../Utility/DirectXInclude.h"
#include "../Config/Config.h"

#ifdef max
#undef max
#endif

template <typename T>
concept HasIndex = requires {
	{ T::index } -> std::convertible_to<size_t>;
};

struct CameraConstants {
    SimpleMath::Matrix view;
    SimpleMath::Matrix proj;
    SimpleMath::Matrix viewProj;
    SimpleMath::Matrix middleViewProj;
    //SimpleMath::Matrix farViewProj;

    SimpleMath::Vector3 cameraPosition;
    int isShadow{ 0 };
    SimpleMath::Vector3 lengthOffset;
};

using MaterialIndex = UINT;

struct ModelContext {
	SimpleMath::Matrix world;
    SimpleMath::Vector3 BBCenter{}; 
	SimpleMath::Vector3 BBextents{};
	UINT material;
};

struct AnimationModelContext {
	SimpleMath::Matrix world;
	SimpleMath::Vector3 BBCenter{};
	SimpleMath::Vector3 BBextents{};
	UINT material;
	UINT boneIndexStart{ 0 };
};

struct ModelContext2D {
    DirectX::XMFLOAT3X3 Transform{
        1.f,0.f,0.f,
        0.f,1.f,0.f,
        0.f,0.f,1.f
    };

    DirectX::XMFLOAT3X3 UVTransform{
        1.f,0.f,0.f,
        0.f,1.f,0.f,
        0.f,0.f,1.f
    };

    UINT ImageIndex{ 0 };
    float GreyScale{ 1.f };
};


class IScene abstract : public std::enable_shared_from_this<IScene> {
public:
	virtual ~IScene() = default;
	virtual void Init(ComPtr<ID3D12Device10> device, ComPtr<ID3D12GraphicsCommandList> commandList) PURE;
	virtual void ProcessNetwork() PURE;
	virtual void Update() PURE;
	virtual void SendNetwork() PURE;
    virtual void Exit() PURE; 
};

using SceneFeatureType = std::tuple<bool, bool, bool>;

struct BoneTransformBuffer {
	std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<size_t>> boneTransforms;
	UINT boneCount;
};

struct TerrainHeader {
    int globalWidth;
    int globalHeight;
    float gridSpacing;
    float minX;
    float minZ;
};

template<typename T>
constexpr size_t GetCBVSize() {
	return (sizeof(T) + 255) & ~255;
}

constexpr size_t GetCBVSize(size_t size) {
	return (size + 255) & ~255;
}

/*
B 가 A 의 부분집합인지 확인한다. 
매개변수의 순서에 주목
*/
template<size_t N>
bool IsSubSet(const std::bitset<N>& a, const std::bitset<N>& b) {
	return (a & b) == b;
}

enum : UINT {
	ParticleType_emit = 1,
	ParticleType_shell = 2,
	ParticleType_ember = 3,
};


enum PlayerRole : BYTE {
	PlayerRole_None = 0,
    PlayerRole_SwordMan, 
	PlayerRole_Archer,
	PlayerRole_Mage,
	PlayerRole_ShieldMan,
	PlayerRole_Demon,
    PlayerRole_END,
};

// 계층 구조를 따르는 Emit 파티클의 경우, 부모의 정보가 필요하다. 
// 이미 만들어져 있는 월드 행렬에는 위치, 로컬 축 3개가 포함되어 있다. 이정도면 충분. 
// 만약 계층 구조를 이루는 파티클을 만들고 싶은 경우, Transform* 를 받아와서, 부모의 위치를 업데이트 하여 사용한다.
// 부모의 계층만 사용하여 계산하며, offset 을 통해 계층 구조 파티클을 구현한다. 
// 이 때 계층 구조를 가지는 파티클의 개수는 512개로 제한한다. 
// 만약 parentID 가 활성화 되어 있는 경우, 방향&속도 에 따른 위치 계산을 하지 않고, 부모의 위치 + offset 을 위치로 삼는다.
// 이 외의 계산 과정은 동일. 

struct ParticleVertex {
    DirectX::XMFLOAT3 position{ 0.f, 0.f, 0.f };
    float halfWidth{ 0.f };
    float halfheight{ 0.f };
    UINT material{};

#pragma region Sprite
    UINT spritable{ 0 };
    UINT spriteFrameInRow{ 0 };
    UINT spriteFrameInCol{ 0 };
    float spriteDuration{ 0 };
#pragma endregion Sprite

    DirectX::XMFLOAT3 direction{ 0.f, 0.f, 0.f };
    float velocity{ 0.f };

    float totalLifeTime{ 0.f };
    float lifeTime{ 0.f };

    UINT type{ ParticleType_ember };
    UINT emitType{ ParticleType_ember };
    UINT remainEmit{ 0 };
    UINT emitIndex{ std::numeric_limits<UINT>::max() };

    float mass{ 1.f };           
    float drag{ 0.1f };          
    float opacity{ 1.f };        
};

/*
* Flags!
* 0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 - Common 
* 0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0001 - Deactivated
* 0b0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0010 - Delete
*/

enum class ParticleFlag : UINT {
    Common      = 0b0000'0000'0000'0000'0000'0000'0000'0000,
    Deactive    = 0b0000'0000'0000'0000'0000'0000'0000'0010,
    Delete      = 0b0000'0000'0000'0000'0000'0000'0000'0100,
    Empty       = 0b1111'1111'1111'1111'1111'1111'1111'1111,
};

struct EmitParticleContext {
    DirectX::XMFLOAT3 position{ 0.f,0.f,0.f };
    UINT Flags{ static_cast<UINT>(ParticleFlag::Empty) };
};

template<typename T, std::size_t N>
std::size_t GetIndexFromAddress(const std::array<T, N>& arr, const T* addr) {
    const T* start = arr.data();
    return static_cast<std::size_t>(addr - start);
}

enum class StringColor : DWORD {
    AliceBlue, AntiqueWhite, Aqua, Aquamarine, Azure,
    Beige, Bisque, Black, BlanchedAlmond, Blue,
    BlueViolet, Brown, BurlyWood, CadetBlue, Chartreuse,
    Chocolate, Coral, CornflowerBlue, Cornsilk, Crimson,
    Cyan, DarkBlue, DarkCyan, DarkGoldenrod, DarkGray,
    DarkGreen, DarkKhaki, DarkMagenta, DarkOliveGreen, DarkOrange,
    DarkOrchid, DarkRed, DarkSalmon, DarkSeaGreen, DarkSlateBlue,
    DarkSlateGray, DarkTurquoise, DarkViolet, DeepPink, DeepSkyBlue,
    DimGray, DodgerBlue, Firebrick, FloralWhite, ForestGreen,
    Fuchsia, Gainsboro, GhostWhite, Gold, Goldenrod,
    Gray, Green, GreenYellow, Honeydew, HotPink,
    IndianRed, Indigo, Ivory, Khaki, Lavender,
    LavenderBlush, LawnGreen, LemonChiffon, LightBlue, LightCoral,
    LightCyan, LightGoldenrodYellow, LightGreen, LightGray, LightPink,
    LightSalmon, LightSeaGreen, LightSkyBlue, LightSlateGray, LightSteelBlue,
    LightYellow, Lime, LimeGreen, Linen, Magenta,
    Maroon, MediumAquamarine, MediumBlue, MediumOrchid, MediumPurple,
    MediumSeaGreen, MediumSlateBlue, MediumSpringGreen, MediumTurquoise, MediumVioletRed,
    MidnightBlue, MintCream, MistyRose, Moccasin, NavajoWhite,
    Navy, OldLace, Olive, OliveDrab, Orange,
    OrangeRed, Orchid, PaleGoldenrod, PaleGreen, PaleTurquoise,
    PaleVioletRed, PapayaWhip, PeachPuff, Peru, Pink,
    Plum, PowderBlue, Purple, Red, RosyBrown,
    RoyalBlue, SaddleBrown, Salmon, SandyBrown, SeaGreen,
    Seashell, Sienna, Silver, SkyBlue, SlateBlue,
    SlateGray, Snow, SpringGreen, SteelBlue, Tan,
    Teal, Thistle, Tomato, Turquoise, Violet,
    Wheat, White, WhiteSmoke, Yellow, YellowGreen,
    END
};

enum class StringFontType : WORD {
	NotoSans, END 
};


#pragma region MY_WINDOW_MSG
#define WM_ADVANCESCENE (WM_USER + 1)
#pragma endregion MY_WINDOW_MSG


inline std::wstring ConvertUtf8ToWstring(const char* utf8Str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);

    std::wstring result(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &result[0], len);

    result.resize(wcslen(result.c_str()));
    return result;
}