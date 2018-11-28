#pragma once

#include <graphics/Shapes/Shape.h>
#include <glm/vec3.hpp>

namespace msg {
    class Sphere : public Shape {
        public:
            Sphere(glm::vec3 center, float radius) : center(center), radius(radius) {};
            glm::vec3 center;
            float radius;
    };
}