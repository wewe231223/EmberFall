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
//        02 - 24    : input의 상태를 세분화할 필요성이 있어보인다...
//                     처음에는 DOWN, UP으로 전부 처리하려 했지만 만들어 놨던 KeyState를 다시 써야할거 같다.
//                     처음으로 키가 눌렸을 때 해야하는 행동을 Input에서가 아닌 다른 클래스에서 처리하려하면
//                     너무 복잡해진다.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "GameObjectComponent.h"

enum class KeyState : BYTE {
    RELEASE,
    UP,
    DOWN,
    PRESS,
};

class Input {
public:
    Input();
    ~Input();

public:
    void UpdateInput(Key key);
    void UpdateInput(BYTE key, bool state);

    KeyState GetState(BYTE key) const;

    void Update();

    bool IsDown(BYTE key) const;
    bool IsReleased(BYTE key) const;
    bool IsUp(BYTE key) const;
    bool IsPressed(BYTE key) const;

    // if KeyState::DOWN or KeyState::PRESS Return True
    bool IsActiveKey(BYTE key);
    // if KeyState::UP or KeyState::RELEASE Return True
    bool IsInactiveKey(BYTE key) const;

private:
    std::array<KeyState, MAX_KEY_SIZE> mKeys{ };
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