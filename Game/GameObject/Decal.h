#pragma once 

class Decal {
public:
    Decal();
    Decal(const SimpleMath::Vector3& position, const SimpleMath::Vector3& direction, const SimpleMath::Vector3& size);

	~Decal() = default;

	Decal(const Decal&) = default;
	Decal& operator=(const Decal&) = default;

	Decal(Decal&&) = default;
	Decal& operator=(Decal&&) = default;
public:
    void SetPosition(const SimpleMath::Vector3& pos);
    void SetDirection(const SimpleMath::Vector3& dir);
    void SetSize(const SimpleMath::Vector3& size);
    void SetMaterial(UINT mat); 

    void ComputeMatrix(); 


private:
    SimpleMath::Vector3 mPosition{};
    SimpleMath::Vector3 mDirection{};
    SimpleMath::Vector3 mSize{};      // x: width, y: height, z: depth

    UINT mMaterial{ 0 }; 
    SimpleMath::Matrix mViewProjMatrix{};
};