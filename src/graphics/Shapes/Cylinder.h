#pragma once

#include <graphics/Shapes/Shape.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace msg {
    class Cylinder : public Shape {
        public:
            Cylinder(glm::vec3 center, glm::vec3 direction, float radius) : center(center), direction(direction), radius(radius) {};
            glm::vec3 center, direction;
            float radius;
    };
}