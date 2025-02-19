#include "pch.h"
#include "Grid.h"

#include "GameObject.h"
#include "Collider.h"

Grid::Grid(SimpleMath::Vector2 position, float cellWidth, float cellHeight) 
    : mCellPosition{ position }, mCellSize{ cellWidth, cellHeight } { }

Grid::~Grid() { }

void Grid::HandleCollision() {    
    for (size_t next{ 1 }; auto& obj1 : mObjects) {
        for (auto& obj2 : mObjects | std::views::drop(next)) {
            auto collider1 = obj1->GetCollider();
            auto collider2 = obj2->GetCollider();

            auto collisionResult = collider1->CheckCollision(collider2);
            collider1->UpdateState(collisionResult, obj2->GetId());
            collider2->UpdateState(collisionResult, obj1->GetId());

            SimpleMath::Vector3 impulse{ SimpleMath::Vector3::Zero };
            if (true == collisionResult) {
                impulse = Collision::GetMinTransVec(
                    std::static_pointer_cast<OrientedBoxCollider>(collider1)->GetBoundingBox(),
                    std::static_pointer_cast<OrientedBoxCollider>(collider2)->GetBoundingBox()
                );
            }

            obj1->OnCollision("", obj2, impulse);
            obj2->OnCollision("", obj1, -impulse);
        }

        ++next;
    }
}

void Grid::Clear() {
    mObjects.clear();
}

bool Grid::Intersects(const std::shared_ptr<Collider>& collider) {
    auto oriented = std::static_pointer_cast<OrientedBoxCollider>(collider);
    decltype(auto) box = oriented->GetBoundingBox();
   
    SimpleMath::Vector2 extents = SimpleMath::Vector2{ box.Extents.x, box.Extents.z } * 1.415;
    SimpleMath::Vector2 cellLB = mCellPosition - mCellSize / 2.0f;
    SimpleMath::Vector2 cellRT = mCellPosition + mCellSize / 2.0f;
    if (extents.x < cellLB.x or cellRT.x > -extents.x
        or extents.y < cellLB.y or cellRT.y > -extents.y) {
        return false;
    }

    return true;
}

void Grid::AddObject(const std::shared_ptr<class GameObject>& object) {
    mObjects.push_back(object);
}
