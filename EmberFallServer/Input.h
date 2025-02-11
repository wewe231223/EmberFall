#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Input.h
// 
// 2025 - 02 - 10   : Input을 컴포넌트로 만드려고 하니 객체간의 결합이 너무 강해지는거 같다.
//                    어짜피 Player에서만 쓰기는 하나 별도의 클래스로 분리하는게 합당한거 같다.
//                    Input은 어디에서 생성? -> Player마다 Input클래스 하나씩이 있어야함.
//                    Player 생성할 때 같이 처리하자.
// 
//                    PlayerScript를 생성할때 인자로 Input을 받고 생성하게 하자.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameObjectComponent.h"

enum class KeyState : BYTE {
    DOWN,
    PRESS,
    UP,
    RELEASE,
};

class Input {
public:
    Input();
    ~Input();

public:
    void UpdateInput(Key key);
    void UpdateInput(BYTE key, bool state);
    bool GetState(BYTE key) const;

private:
    std::array<bool, MAX_KEY_SIZE> mKeys{ };
};

class InputManager { // PlayerLock으로 동기화
public:
    InputManager();
    ~InputManager();

public:
    void DeleteInput(SessionIdType id);

    std::shared_ptr<Input> GetInput(SessionIdType id);

private:
    std::unordered_map<SessionIdType, std::shared_ptr<Input>> mInputMap{ };
};