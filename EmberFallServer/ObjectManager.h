#pragma once

#include "GameObject.h"

class ObjectManager {
public:
    std::shared_ptr<GameObject> GetObjectFromId() const;

private:

};