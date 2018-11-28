#pragma once 

#include <graphics/Shapes/Shape.h>
#include <glm/vec3.hpp>

namespace msg {
    class AABB : public Shape {
        public:
            AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {};
        private:
            glm::vec3 min, max;
    };
}
