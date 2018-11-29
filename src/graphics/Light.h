#pragma once

#include <glm/vec3.hpp>

namespace msg {
    class Light {
        Light(glm::vec3 position, glm::vec3 color) : position(position), color(color) {};
        glm::vec3 position, color;
    };
}