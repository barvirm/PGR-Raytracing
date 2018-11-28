#pragma once

#include <vector>
#include <graphics/Shapes/Sphere.h>
#include <graphics/Shapes/AABB.h>
#include <graphics/Shapes/Cylinder.h>

namespace msg {
    class Scene {
        public:
            Scene() : _dirty(false) {};
            inline void addShape(msg::AABB &aabb) { _dirty = true; _AABBes.emplace_back(aabb); }
            inline void addShape(msg::Sphere &sphere) { _dirty = true; _spheres.emplace_back(sphere); }
            inline void addShape(msg::Cylinder &cylinder) { _dirty = true; _cylinders.emplace_back(cylinder); }
            inline bool isDirty() const { return _dirty; }

            inline std::vector<msg::Sphere> &spheres() { return  _spheres; }
            inline std::vector<msg::AABB> &AABBes() { return  _AABBes; }
            inline std::vector<msg::Cylinder> &cylinders() { return  _cylinders; }

            // gettors
        protected:
            std::vector<msg::Sphere> _spheres;
            std::vector<msg::AABB> _AABBes;
            std::vector<msg::Cylinder> _cylinders;
            bool _dirty;
    };
}