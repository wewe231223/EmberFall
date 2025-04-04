#include "pch.h"
#include "ObjectBuilder.h"
#include "PlayerScript.h"
#include "MonsterScript.h"
#include "ArrowScript.h"
#include "ItemScript.h"
#include "CorruptedGem.h"
#include "EventTrigger.h"

void ObjectBuilder::BuildObjectComponent(std::shared_ptr<GameObject>& gameObject, ObjectTag objectTag) {
    gameObject->Reset();

    switch (objectTag) {
    case ObjectTag::MONSTER:
        gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
        gameObject->GetCollider()->SetTransform(gameObject->GetTransform());
        gameObject->GetCollider()->Update();
        gameObject->CreateComponent<MonsterScript>(gameObject);
        break;

    case ObjectTag::PLAYER:
        gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
        //gameObject->CreateComponent<PlayerScript>();
        break;

    case ObjectTag::CORRUPTED_GEM:
        gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
        gameObject->GetCollider()->SetTransform(gameObject->GetTransform());
        gameObject->GetCollider()->Update();
        gameObject->CreateComponent<CorruptedGemScript>(gameObject);
        break;

    default:
        break;
    }

    gameObject->Init();
}

void ObjectBuilder::BuildTrigger(std::shared_ptr<GameObject>& gameObject, float lifeTime, const SimpleMath::Vector3& pos, const SimpleMath::Vector3& extents) {
    gameObject->Reset();
    gameObject->DisablePhysics();

    gameObject->GetTransform()->Translate(pos);
    gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, extents);
    gameObject->GetCollider()->SetTransform(gameObject->GetTransform());
    gameObject->GetCollider()->Update();

    gameObject->CreateComponent<Trigger>(gameObject, lifeTime);
    gameObject->Init();
}

void ObjectBuilder::BuildTrigger(std::shared_ptr<GameObject>& gameObject, float lifeTime, const SimpleMath::Vector3& pos, std::shared_ptr<Collider> collider) {
    gameObject->Reset();
    gameObject->DisablePhysics();

    gameObject->GetTransform()->Translate(pos);
    gameObject->SetCollider(collider);
    gameObject->GetCollider()->SetTransform(gameObject->GetTransform());
    gameObject->GetCollider()->Update();

    gameObject->CreateComponent<Trigger>(gameObject, lifeTime);
    gameObject->Init();
}

void ObjectBuilder::BuildEventTrigger(std::shared_ptr<GameObject>& gameObject, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
    const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents, const SimpleMath::Vector3& dir) {
    gameObject->Reset();
    gameObject->DisablePhysics();

    gameObject->GetTransform()->Translate(center);
    gameObject->GetTransform()->Rotation(MathUtil::GetQuatFromLook(dir));
    gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, extents);
    gameObject->GetCollider()->SetTransform(gameObject->GetTransform());
    gameObject->GetCollider()->Update();

    gameObject->CreateComponent<EventTrigger>(gameObject, event, lifeTime, eventDelay, eventCount);

    gameObject->Init();
}

void ObjectBuilder::BuildEventTrigger(std::shared_ptr<GameObject>& gameObject, std::shared_ptr<GameEvent> event, float lifeTime, float eventDelay, int32_t eventCount,
    const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, std::shared_ptr<Collider> collider) {
    gameObject->Reset();
    gameObject->DisablePhysics();

    gameObject->GetTransform()->Translate(pos);
    gameObject->GetTransform()->Rotation(SimpleMath::Quaternion::FromToRotation(SimpleMath::Vector3::Forward, dir));
    gameObject->SetCollider(collider);
    gameObject->GetCollider()->SetTransform(gameObject->GetTransform());
    gameObject->GetCollider()->Update();

    gameObject->CreateComponent<EventTrigger>(gameObject, event, lifeTime, eventDelay, eventCount);

    gameObject->Init();
}

void ObjectBuilder::BuildProjectile(std::shared_ptr<GameObject>& gameObject, ObjectTag objectTag,
    const SimpleMath::Vector3& pos, const SimpleMath::Vector3& dir, GameUnits::GameUnit<GameUnits::StandardSpeed> speed) {
    gameObject->Reset();

    switch (objectTag) {
    case ObjectTag::ARROW:
        gameObject->CreateCollider<OrientedBoxCollider>(SimpleMath::Vector3::Zero, SimpleMath::Vector3{ 0.5f });
        gameObject->GetCollider()->SetTransform(gameObject->GetTransform());
        gameObject->GetCollider()->Update();
        gameObject->CreateComponent<ArrowScript>(gameObject, pos, dir, speed);
        break;

    default:
        break;
    }
}

void ObjectBuilder::BuildItem(std::shared_ptr<GameObject>& gameObject, ItemTag item) {
    gameObject->Reset();

    gameObject->CreateComponent<ItemScript>(gameObject, item);
}
