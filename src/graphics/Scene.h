#pragma once

#include <vector>
#include <unordered_map>
#include <graphics/Shapes.h>
#include <graphics/Light.h>
#include <graphics/Material.h>

namespace msg {
    class Scene {
        public:
            Scene() = default;
            void addShape(msg::AABB &aabb, msg::Material material = msg::Material(glm::vec3(1), 0.0f));
            void addShape(msg::Sphere &sphere, msg::Material material = msg::Material(glm::vec3(1), 0.0f));
            void addShape(msg::Cylinder &cylinder, msg::Material material = msg::Material(glm::vec3(1), 0.0f));
            void addLight(msg::Light &light) { _lights.emplace_back(light); {}}

            inline std::vector<msg::Sphere> &spheres() { return  _spheres; }
            inline std::vector<msg::AABB> &AABBes() { return  _AABBes; }
            inline std::vector<msg::Cylinder> &cylinders() { return  _cylinders; }
            inline std::vector<msg::Light> &lights() { return _lights; }
            inline std::vector<msg::Material> &materials() { return _materials; }

            // gettors
        protected:
            std::vector<msg::Sphere> _spheres;
            std::vector<msg::AABB> _AABBes;
            std::vector<msg::Cylinder> _cylinders;
            std::vector<msg::Light> _lights;
            std::vector<msg::Material> _materials;
    };
}