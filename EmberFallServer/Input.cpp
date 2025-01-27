#include "pch.h"
#include "Input.h"

Input::Input() { }

Input::~Input() { }

void Input::Update(BYTE key, bool state) {
    mKeys[key] = state;
}

bool Input::GetState(BYTE key) const {
    return mKeys[key];
}
