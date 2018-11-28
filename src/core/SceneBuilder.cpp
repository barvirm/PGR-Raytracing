#include <core/SceneBuilder.h>

#include <graphics/Scene.h>
#include <graphics/Shapes/AABB.h>

std::shared_ptr<msg::Scene> SceneBuilder::build() {
    auto scene(std::make_shared<msg::Scene>());
    msg::AABB box1(glm::vec3(-5.0, -0.1, -5.0), glm::vec3(5.0, 0.0, 5.0));
    msg::AABB box2(glm::vec3(-0.5,  0.0, -0.5), glm::vec3(0.5, 1.0, 0.5));
    scene->addShape(box1);
    scene->addShape(box2);
    return scene;
}