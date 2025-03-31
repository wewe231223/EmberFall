#pragma once

#include "Item.h"
#include "Trigger.h"
#include "GameObject.h"

class ObjectBuilder{
public:
    static void BuildObjectComponent(std::shared_ptr<GameObject>& gameObject, ObjectTag objectTag);

    static void BuildTrigger(std::shared_ptr<GameObject>& gameObject, float lifeTime, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& extents);
    static void BuildTrigger(std::shared_ptr<GameObject>& gameObject, float lifeTime, const SimpleMath::Vector3& pos, std::shared_ptr<Collider> collider);

    static void BuildEventTrigger(std::shared_ptr<GameObject>& gameObject, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
        const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents, const SimpleMath::Vector3& dir);    
    static void BuildEventTrigger(std::shared_ptr<GameObject>& gameObject, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
        const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, std::shared_ptr<Collider> collider);

    static void BuildProjectile(std::shared_ptr<GameObject>& gameObject, ObjectTag objectTag,
        const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, GameUnits::GameUnit<GameUnits::StandardSpeed> speed);
    static void BuildItem(std::shared_ptr<GameObject>& gameObject, ItemTag item);
};

