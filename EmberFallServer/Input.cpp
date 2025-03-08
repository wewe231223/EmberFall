#include "pch.h"
#include "Input.h"

Input::Input() {
    mKeys.fill(KeyState::RELEASE);
}

Input::~Input() { }

void Input::UpdateInput(Key key) {
    UpdateInput(key.key, key.state);
}

void Input::UpdateInput(BYTE key, bool state) {
    if (Key::DOWN == state) { // DOWN 상태가 올 경우
        switch (mKeys[key]) {
        case KeyState::DOWN:
            mKeys[key] = KeyState::PRESS;
            break;

        case KeyState::UP:
        case KeyState::RELEASE:
            mKeys[key] = KeyState::DOWN;
            break;

        default:
            break;
        }
    }
    else { // UP 상태가 올경우
        switch (mKeys[key]) {
        case KeyState::DOWN:
        case KeyState::PRESS:
            mKeys[key] = KeyState::UP;
            break;

        case KeyState::UP:
            mKeys[key] = KeyState::RELEASE;
            break;

        default:
            break;
        }
    }
}

KeyState Input::GetState(BYTE key) const {
    return mKeys[key];
}

void Input::Update() {
    for (auto& state : mKeys) {
        if (KeyState::DOWN == state) {
            state = KeyState::PRESS;
        }
        else if (KeyState::UP == state) {
            state = KeyState::RELEASE;
        }
    }
}

bool Input::IsDown(BYTE key) const {
    return KeyState::DOWN == mKeys[key];
}

bool Input::IsReleased(BYTE key) const {
    return KeyState::RELEASE == mKeys[key];
}

bool Input::IsUp(BYTE key) const {
    return KeyState::UP == mKeys[key];
}

bool Input::IsPressed(BYTE key) const {
    return KeyState::PRESS == mKeys[key];
}

bool Input::IsActiveKey(BYTE key) {
    return static_cast<BYTE>(mKeys[key]) & 0b10; // if mKeys[key] == KeyState::UP or mKeys[key] == KeyState::RELEASE return true
}

bool Input::IsInactiveKey(BYTE key) const {
    return ~(static_cast<BYTE>(mKeys[key]) & 0b10); // if mKeys[key] == KeyState::UP or mKeys[key] == KeyState::RELEASE return true
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
