#include <graphics/RaytracingTechnique.h>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <geUtil/OrbitCamera.h>
#include <geUtil/PerspectiveCamera.h>

#include <graphics/Scene.h>
#include <geGL/geGL.h>

void msg::RaytracingTechnique::init() {

    texture = std::make_shared<ge::gl::Texture>(gl->getFunctionTable(), GL_TEXTURE_2D, GL_RGBA32F, 0, viewport->x, viewport->y);
    texture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    texture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    VAO = std::make_shared<ge::gl::VertexArray>(gl->getFunctionTable());
    
    int wgl[3];
    gl->glGetProgramiv(computeShader->getId(), GL_COMPUTE_WORK_GROUP_SIZE, wgl); 
    workingGroupLayout = glm::vec3(wgl[0], wgl[1], wgl[2]);
}

void msg::RaytracingTechnique::draw() {
    //std::cout << "RT draw()" << std::endl;
    drawProgram->use();
    VAO->bind();
    texture->bind(0);
    gl->glDrawArrays(GL_POINTS, 0, 1);

}

void msg::RaytracingTechnique::setScene(std::shared_ptr<msg::Scene> &_scene) {
    scene = _scene;

    std::vector<float> AABB_GPU;
    for(auto &aabb : scene->AABBes() ) {
        AABB_GPU.emplace_back(aabb.min.x);
        AABB_GPU.emplace_back(aabb.min.y);
        AABB_GPU.emplace_back(aabb.min.z);
        AABB_GPU.emplace_back(aabb.max.x);
        AABB_GPU.emplace_back(aabb.max.y);
        AABB_GPU.emplace_back(aabb.max.z);
        AABB_GPU.emplace_back(0.0f);
        AABB_GPU.emplace_back(0.0f);
    }

    int num_aabb = scene->AABBes().size();

    AABB_SSBO = std::make_shared<ge::gl::Buffer>(gl->getFunctionTable(), sizeof(float) * AABB_GPU.size(), AABB_GPU.data());
    AABB_SSBO->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
    computeShader->set("num_aabb", num_aabb);
    // TODO create more SSBO buffers, one per primitive
    // send to cpu a left it for eternity



}

void msg::RaytracingTechnique::update() {
    //std::cout << "RT update()" << std::endl;

    auto cameraPosition = glm::vec3(glm::inverse(orbitCamera->getView())[3]);

    computeShader->use();
    computeShader->set3fv("eye", glm::value_ptr(cameraPosition));
    computeShader->set3fv("ray00", glm::value_ptr(getRay(-1, -1, cameraPosition)));
    computeShader->set3fv("ray01", glm::value_ptr(getRay(-1,  1, cameraPosition)));
    computeShader->set3fv("ray10", glm::value_ptr(getRay( 1, -1, cameraPosition)));
    computeShader->set3fv("ray11", glm::value_ptr(getRay( 1,  1, cameraPosition)));

    //std::cout << viewport->x << " " << viewport->y << std::endl;
    auto debug(std::make_shared<ge::gl::Buffer>(gl->getFunctionTable(), sizeof(glm::vec4) * 100));
    
    texture->bindImage(0, 0, GL_RGBA32F, GL_WRITE_ONLY, false, 0);
    debug->bindBase(GL_SHADER_STORAGE_BUFFER, 3);

    auto roundUpToPowerOfTwo = [](int x) -> int { 
        x--;
        x |= x >> 1;
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;
        return ++x;
    };

    gl->glDispatchCompute(
        roundUpToPowerOfTwo(viewport->x) / workingGroupLayout.x,
        roundUpToPowerOfTwo(viewport->y) / workingGroupLayout.y,
        1
    );
    gl->glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

glm::vec3 msg::RaytracingTechnique::getRay(float x, float y,const glm::vec3 &eye) {
    glm::vec4 ray(x, y, 0.0f, 1.0f);
    auto VP = perspectiveCamera->getProjection() * orbitCamera->getView();
    auto IVP = glm::inverse(VP);
    ray = IVP * ray;
    ray *= (1.0f / ray.w);
    ray -= glm::vec4(eye,0.0);
    return ray;
}

std::string msg::RaytracingTechnique::to_string(const glm::vec3 &d) {
    std::string res = "";
    res += std::to_string(d.x) + " ";
    res += std::to_string(d.y) + " ";
    res += std::to_string(d.z);
    return res;
}

std::string msg::RaytracingTechnique::to_string(const glm::ivec3 &d) {
    std::string res = "";
    res += std::to_string(d.x) + " ";
    res += std::to_string(d.y) + " ";
    res += std::to_string(d.z);
    return res;
}

void msg::RaytracingTechnique::onViewportChanged() {
    std::cout << "RaytracingTechnique onViewportChanged" << std::endl;
    texture->setData2D(NULL, GL_RGBA, GL_FLOAT, 0, 0, 0, viewport->x, viewport->y);
}
