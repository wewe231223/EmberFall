#include "pch.h"
#include "ObjectBuilder.h"
#include "PlayerScript.h"
#include "MonsterScript.h"
#include "CorruptedGem.h"
#include "Trigger.h"

void ObjectBuilder::BuildObjectComponent(std::shared_ptr<GameObject>& gameObject, ObjectTag objectTag) {
    gameObject->ClearComponents();

    switch (objectTag) {
    case ObjectTag::MONSTER:
        gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
        gameObject->CreateComponent<MonsterScript>(gameObject);
        break;

    default:
        break;
    }

    gameObject->Init();
}

void ObjectBuilder::BuildTrigger(std::shared_ptr<GameObject>& gameObject, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
    const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents, const SimpleMath::Vector3& dir) {
    gameObject->ClearComponents();

    gameObject->GetTransform()->Translate(center);
    gameObject->GetTransform()->Rotation(MathUtil::GetQuatFromLook(dir));
    gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, extents);
    gameObject->CreateComponent<Trigger>(gameObject, event, lifeTime, eventDelay, eventCount);

    gameObject->Init();
}
