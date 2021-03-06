#pragma once

#include "component.h"
#include <glm/glm.hpp>

class ColliderComponent : public Component{
  public:
    virtual glm::vec3 GetCenter() = 0;
    virtual glm::vec3 Support(glm::vec3 dir) = 0;
    static ComponentType cType;
    ComponentType Type();
};

