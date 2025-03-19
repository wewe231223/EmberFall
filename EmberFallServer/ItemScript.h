#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ItemScript.h
// 
// 2025 - 03 - 11 : ItemScript
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "Item.h"
#include "Script.h"

class ItemScript : public Script {
public:
    ItemScript(std::shared_ptr<GameObject> owner, ItemTag item);
    virtual ~ItemScript();

private:
    virtual void Init() override;

    virtual void Update(const float deltaTime) override;
    virtual void LateUpdate(const float deltaTime) override;

    virtual void OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;
    virtual void OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) override;

    virtual void DispatchGameEvent(struct GameEvent* event) override;

private:
    ItemTag mItemTag{ };
};