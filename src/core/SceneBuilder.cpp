#include <core/SceneBuilder.h>
#include <graphics/Scene.h>

#include <graphics/Shapes.h>
#include <graphics/Light.h>

std::shared_ptr<msg::Scene> SceneBuilder::build() {
    auto scene(std::make_shared<msg::Scene>());
    
    msg::AABB box1(glm::vec3(-5.0, -0.1, -5.0), glm::vec3(5.0, 0.0, 5.0));
    msg::AABB box2(glm::vec3(-0.5,  0.0, -0.5), glm::vec3(0.5, 1.0, 0.5));
    msg::Sphere s1(glm::vec3(-1.0,  3.0, 0.0), 1.0f);
    msg::Sphere s2(glm::vec3( 3.0,  3.0, 0.0), 1.0f);
    msg::Sphere s3(glm::vec3( 0.0, 40.0, 0.0), 0.1f);
    msg::Cylinder c1(glm::vec3(0), glm::vec3(0.0, 1.0, 0.0), 1.5f);

    msg::Light l1(glm::vec3(0,10,0), glm::vec3(0.8));

    scene->addShape(box1);
    scene->addShape(box2);
    scene->addShape(s1);
    scene->addShape(s2);
    scene->addShape(s3);
    scene->addLight(l1);
    //scene->addShape(c1);
    return scene;
}