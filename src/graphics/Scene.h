#pragma once

#include <vector>
#include <graphics/Shapes/Sphere.h>
#include <graphics/Shapes/AABB.h>
#include <graphics/Shapes/Cylinder.h>

namespace msg {
    class Scene {
        public:
            Scene() = default;
            inline void addShape(msg::AABB &aabb) {AABBes.emplace_back(aabb); }
            inline void addShape(msg::Sphere &sphere) {spheres.emplace_back(sphere); }
            inline void addShape(msg::Cylinder &cylinder) {cylinders.emplace_back(cylinder); }
            inline bool isDirty() { return _dirty; }

            // gettors
        protected:
            std::vector<msg::Sphere> spheres;
            std::vector<msg::AABB> AABBes;
            std::vector<msg::Cylinder> cylinders;
            bool _dirty;
    };
}