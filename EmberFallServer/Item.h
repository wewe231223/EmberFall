#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Item.h
// 
// 2025 - 03 - 11 : Item 효과들에 대한 정의
// 
//                  아이템은 그냥 함수로 만들자
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

using ItemEffectFn = std::function<void(const std::shared_ptr<class GameObject>&)>;

class Item abstract {
public:
    Item();
    virtual ~Item();

public:
    bool IsActive() const;
    void SetActive(bool active);

    virtual void UseItem(const std::shared_ptr<class GameObject>& obj) abstract;

private:
    bool mActive{ false };
};

class Potion : public Item {
public:
    Potion();
    virtual ~Potion();

public:
    virtual void UseItem(const std::shared_ptr<class GameObject>& obj) override;
};

class HolyWater : public Item {
public:
    HolyWater();
    virtual ~HolyWater();

public:
    virtual void UseItem(const std::shared_ptr<class GameObject>& obj) override;
};

class Cross : public Item {
public:
    Cross();
    virtual ~Cross();

public:
    virtual void UseItem(const std::shared_ptr<class GameObject>& obj) override;
};