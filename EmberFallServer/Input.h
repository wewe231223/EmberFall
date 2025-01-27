#pragma once

class Input {
public:
    Input();
    ~Input();

public:
    void Update(BYTE key, bool state);
    bool GetState(BYTE key) const;

private:
    std::array<bool, MAX_KEY_SIZE> mKeys{ };
};