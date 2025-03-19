#pragma once

#include "Trigger.h"
#include "GameObject.h"

class ObjectBuilder{
public:
    static void BuildObjectComponent(std::shared_ptr<GameObject>& gameObject, ObjectTag objectTag);

    static void BuildTrigger(std::shared_ptr<GameObject>& gameObject, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
        const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents, const SimpleMath::Vector3& dir);

    //static void BuildItem(std::shared_ptr<GameObject>& gameObject, ItemTag item);
};

