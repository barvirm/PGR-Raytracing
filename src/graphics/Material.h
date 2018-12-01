#pragma once

#include <glm/vec3.hpp>

namespace msg {
    class Material {
    public:
        glm::vec3 objectColor;
        float reflectance;
        Material(const glm::vec3 &objectColor, const float &reflectance) : objectColor(objectColor), reflectance(reflectance) {};
    };
}