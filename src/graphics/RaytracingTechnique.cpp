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

    using namespace std;

    // TODO need more refactoring
    // define stuctures
    // define convert function
    // convert data
    // bind SSBO
    // set num variables in ComputeShader
    struct GPU_AABB { 
        glm::vec3 min;
        float padding;
        glm::vec3 max;
        float padding2;
        GPU_AABB(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max), padding(0.0f), padding2(0.0f) {}; 
    };

    struct GPU_SPHERE {
        glm::vec3 center;
        float radius;
        GPU_SPHERE(const glm::vec3 &center,const float &radius) : center(center), radius(radius) {};
    };

    struct GPU_CYLINDER {
        glm::vec3 center;
        float padding;
        glm::vec3 direction;
        float radius;
        GPU_CYLINDER(const glm::vec3 &center, const glm::vec3 &direction, const float &radius) : 
            center(center), direction(direction), radius(radius), padding(0.0f) {};
    };

    auto convert_AABB = [](const msg::AABB &aabb) -> GPU_AABB { return {aabb.min, aabb.max}; };
    vector<GPU_AABB> gpu_aabb_data; 
    transform(begin(scene->AABBes()), end(scene->AABBes()), back_inserter(gpu_aabb_data), convert_AABB);
    AABB_SSBO = make_shared<ge::gl::Buffer>(gl->getFunctionTable(), sizeof(GPU_AABB) * gpu_aabb_data.size(), gpu_aabb_data.data());
    AABB_SSBO->bindBase(GL_SHADER_STORAGE_BUFFER, 1);
    computeShader->set1i("num_aabb", scene->AABBes().size());

    auto convert_SPHERE = [](const msg::Sphere &sphere) -> GPU_SPHERE { return {sphere.center, sphere.radius}; };
    vector<GPU_SPHERE> gpu_sphere_data;
    transform(scene->spheres().begin(), scene->spheres().end(), back_inserter(gpu_sphere_data), convert_SPHERE);
    spheres_SSBO = make_shared<ge::gl::Buffer>(gl->getFunctionTable(), sizeof(GPU_SPHERE) * gpu_sphere_data.size(), gpu_sphere_data.data());
    spheres_SSBO->bindBase(GL_SHADER_STORAGE_BUFFER, 2);
    computeShader->set1i("num_spheres", scene->spheres().size());

    auto convert_CYLINDER = [](const msg::Cylinder &cylinder) -> GPU_CYLINDER { return {cylinder.center, cylinder.direction, cylinder.radius}; };
    vector<GPU_CYLINDER> gpu_cylinder_data;
    transform(scene->cylinders().begin(), scene->cylinders().end(), back_inserter(gpu_cylinder_data), convert_CYLINDER);
    cylinder_SSBO = make_shared<ge::gl::Buffer>(gl->getFunctionTable(), sizeof(GPU_CYLINDER) * gpu_cylinder_data.size(), gpu_cylinder_data.data());
    cylinder_SSBO->bindBase(GL_SHADER_STORAGE_BUFFER, 3);
    computeShader->set1i("num_cylinders", scene->cylinders().size());
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
    debug->bindBase(GL_SHADER_STORAGE_BUFFER, 10);

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

/*
    auto *dbg = static_cast<float *>(gl->glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
    for(auto i = 0; i < 6; ++i) {
        std::cout << dbg[i] << " ";
    }
    std::cout << std::endl;
*/

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
