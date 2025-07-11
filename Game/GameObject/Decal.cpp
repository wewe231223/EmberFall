#include "pch.h"
#include "../Utility/DirectXInclude.h"
#include "Decal.h"

Decal::Decal() : mPosition(0, 0, 0), mDirection(0, -1, 0), mSize(1, 1, 1) {
    ComputeMatrix();
}

Decal::Decal(const SimpleMath::Vector3& position, const SimpleMath::Vector3& direction, const SimpleMath::Vector3& size)
    : mPosition(position), mDirection(direction), mSize(size)
{
    ComputeMatrix();
}

void Decal::SetPosition(const SimpleMath::Vector3& pos) { 
    mPosition = pos; 
}

void Decal::SetDirection(const SimpleMath::Vector3& dir) { 
    mDirection = dir; 
}

void Decal::SetSize(const SimpleMath::Vector3& size) { 
    mSize = size; 
}

void Decal::SetMaterial(UINT mat) {
	mMaterial = mat;
}

void Decal::ComputeMatrix()
{
    SimpleMath::Vector3 dir{ mDirection };
    dir.Normalize();

    SimpleMath::Vector3 absDir{ dir };
    absDir.x = std::abs(absDir.x);
    absDir.y = std::abs(absDir.y);
    absDir.z = std::abs(absDir.z);

    SimpleMath::Vector3 up{};
    if (absDir.x <= absDir.y && absDir.x <= absDir.z) {
        up = SimpleMath::Vector3::UnitX;
    }
    else if (absDir.y <= absDir.z) {
        up = SimpleMath::Vector3::UnitY;
    }
    else {
        up = SimpleMath::Vector3::UnitZ;
    }

    SimpleMath::Matrix view = SimpleMath::Matrix::CreateLookAt(mPosition, mPosition + dir, up);
    SimpleMath::Matrix proj = SimpleMath::Matrix::CreateOrthographic(mSize.x, mSize.y, 0.0f, mSize.z);

    mViewProjMatrix = proj * view;
}
