#include <graphics/Scene.h>
#include <iostream>

void msg::Scene::addShape(msg::AABB &aabb, msg::Material material) {
    _materials.emplace_back(material);
    _AABBes.emplace_back(aabb); 
}
void msg::Scene::addShape(msg::Sphere &sphere, msg::Material material) {
    glm::vec3 &v = material.objectColor;
    std::cout << v.x << " " << v.y << " " << v.z << std::endl;
    _materials.emplace_back(material);
    _spheres.emplace_back(sphere); 
}
void msg::Scene::addShape(msg::Cylinder &cylinder, msg::Material material) {
    _materials.emplace_back(material);
    _cylinders.emplace_back(cylinder); 
}