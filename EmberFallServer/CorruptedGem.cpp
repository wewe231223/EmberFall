#include "pch.h"
#include "CorruptedGem.h"
#include "GameObject.h"

CorruptedGemScript::CorruptedGemScript(std::shared_ptr<GameObject> owner) 
    : Script{ owner } { 
    //mOriginColor = owner->GetColor();
}

CorruptedGemScript::~CorruptedGemScript() { }

void CorruptedGemScript::Update(const float deltaTime) { 
    //GetOwner()->SetColor(mOriginColor);
}

void CorruptedGemScript::LateUpdate(const float deltaTime) { }

void CorruptedGemScript::OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void CorruptedGemScript::OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void CorruptedGemScript::OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void CorruptedGemScript::DispatchGameEvent(GameEvent* event) { 
    //switch (event->type) {
    //case GameEventType::DESTROY_GEM_EVENT:
    //    mOriginColor = GetOwner()->GetColor();
    //    GetOwner()->SetColor(SimpleMath::Vector3::Zero);
    //    break;

    //default:
    //    break;
    //}
}
