#include "pch.h"
#include "Input.h"

Input::Input() {
    mKeys.fill(Key::UP);
}

Input::~Input() { }

void Input::UpdateInput(Key key) {
    UpdateInput(key.key, key.state);
}

void Input::UpdateInput(BYTE key, bool state) {
    mKeys[key] = state;
}

bool Input::GetState(BYTE key) const {
    return mKeys[key];
}

InputManager::InputManager() { }

InputManager::~InputManager() { }

void InputManager::DeleteInput(SessionIdType id) {
    auto it = mInputMap.find(id);
    if (it == mInputMap.end()) {
        mInputMap.erase(it);
    }
}

std::shared_ptr<Input> InputManager::GetInput(SessionIdType id) {
    if (false == mInputMap.contains(id)) {
        mInputMap[id] = std::make_shared<Input>();
    }

    return mInputMap[id];
}
