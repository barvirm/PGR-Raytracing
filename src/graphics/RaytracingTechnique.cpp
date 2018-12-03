#include <graphics/RaytracingTechnique.h>
#include <graphics/Scene.h>

#include <iostream>
#include <numeric>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <geUtil/OrbitCamera.h>
#include <geUtil/PerspectiveCamera.h>
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

    struct GPU_LIGHT {
        glm::vec3 position;
        float p;
        glm::vec3 color;
        float x;
        GPU_LIGHT(const glm::vec3 &position, const glm::vec3 &color) : position(position), color(color), p(0.0f), x(0.0f) {};
    };

    // Convert CPU representation without padding to GPU representation with padding
    auto transformToGpuStruct = [](auto convertToGpuStructFunction, const auto &collection) {
        using ConvertType = decltype(convertToGpuStructFunction(collection[0]));
        vector<ConvertType> preparedGpuStruct;
        transform(begin(collection), end(collection), back_inserter(preparedGpuStruct), convertToGpuStructFunction);
        return preparedGpuStruct;
    };

    // Bind converted GPU representation of data to SSBO 
    auto sendToGpuAsSSBO = [this](auto &data, auto &ssbo, const int &bindingPoint) {
        using GPU_DATA_TYPE = decltype(data[0]);
        ssbo = make_shared<ge::gl::Buffer>(gl->getFunctionTable(), sizeof(GPU_DATA_TYPE) * data.size(), data.data());
        ssbo->bindBase(GL_SHADER_STORAGE_BUFFER, bindingPoint);
    };


    auto convert_AABB = [](const msg::AABB &aabb) -> GPU_AABB { return {aabb.min, aabb.max}; };
    auto GpuAABBes = transformToGpuStruct(convert_AABB, scene->AABBes());
    sendToGpuAsSSBO(GpuAABBes, AABB_SSBO, 1);
    int num_aabb = scene->AABBes().size();
    computeShader->set1i("num_aabb", num_aabb);

    auto convert_SPHERE = [](const msg::Sphere &sphere) -> GPU_SPHERE { return {sphere.center, sphere.radius}; };
    auto GpuSpheres = transformToGpuStruct(convert_SPHERE, scene->spheres());
    sendToGpuAsSSBO(GpuSpheres, spheres_SSBO, 2);
    int num_spheres = scene->spheres().size();
    computeShader->set1i("num_spheres", num_spheres);

    auto convert_CYLINDER = [](const msg::Cylinder &cylinder) -> GPU_CYLINDER { return {cylinder.center, cylinder.direction, cylinder.radius}; };
    auto GpuCylinders = transformToGpuStruct(convert_CYLINDER, scene->cylinders());
    sendToGpuAsSSBO(GpuCylinders, cylinder_SSBO, 3);
    int num_cylinders = scene->cylinders().size();
    computeShader->set1i("num_cylinders", num_cylinders);

    auto convert_LIGHT = [](const msg::Light &light) -> GPU_LIGHT { return {light.position, light.color}; };
    auto GpuLights = transformToGpuStruct(convert_LIGHT, scene->lights());
    sendToGpuAsSSBO(GpuLights, light_SSBO, 4);
    computeShader->set1i("num_lights", scene->lights().size());

    sendToGpuAsSSBO(scene->materials(), material_SSBO, 5);
    std::vector<int> materialStartIndex {0, num_aabb, num_spheres, num_cylinders};
    std::partial_sum(begin(materialStartIndex), end(materialStartIndex), begin(materialStartIndex));
    computeShader->set4iv("ps", materialStartIndex.data());
}

void msg::RaytracingTechnique::update() {
    //std::cout << "RT update()" << std::endl;

    auto cameraPosition = glm::vec3(glm::inverse(orbitCamera->getView())[3]);

    computeShader->use();
    computeShader->set3fv("cameraPosition", glm::value_ptr(cameraPosition));
    computeShader->set3fv("ray00", glm::value_ptr(getRay(-1, -1, cameraPosition)));
    computeShader->set3fv("ray01", glm::value_ptr(getRay(-1,  1, cameraPosition)));
    computeShader->set3fv("ray10", glm::value_ptr(getRay( 1, -1, cameraPosition)));
    computeShader->set3fv("ray11", glm::value_ptr(getRay( 1,  1, cameraPosition)));

    
    texture->bindImage(0, 0, GL_RGBA32F, GL_WRITE_ONLY, false, 0);

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

void msg::RaytracingTechnique::onViewportChanged() {
    std::cout << "RaytracingTechnique onViewportChanged" << std::endl;
    std::cout << viewport->x << " " << viewport->y << std::endl;
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport->x, viewport->y, 0, GL_RGBA, GL_FLOAT, NULL);
}
