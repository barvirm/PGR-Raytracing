#include <graphics/RaytracingTechnique.h>
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <geUtil/OrbitCamera.h>
#include <geUtil/PerspectiveCamera.h>

#include <geGL/geGL.h>

void msg::RaytracingTechnique::init() {
    gl->glBindTexture(GL_TEXTURE_2D, tex);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport->x, viewport->y, 0, GL_RGBA, GL_FLOAT, NULL);
    gl->glBindTexture(GL_TEXTURE_2D, 0);

    std::array<float, 12> quad{-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f};
    auto FT = gl->getFunctionTable();
    auto quad_buff = std::make_shared<ge::gl::Buffer>(FT, sizeof(float) * quad.size(), quad.data());
    VAO = std::make_shared<ge::gl::VertexArray>(FT);
    VAO->addAttrib(quad_buff, 0, 2, GL_FLOAT);
}

void msg::RaytracingTechnique::draw() {
    //std::cout << "RT draw()" << std::endl;
    /*
    auto VAO = ge::gl::VertexArray(gl->getFunctionTable());
    program->use();
    VAO.bind();
    gl->glDrawArrays(GL_TRIANGLES, 0, 3);
    */

    glm::vec4 test(1,0,0,0);
    drawProgram->use();
    drawProgram->set4fv("c", glm::value_ptr(test));
    VAO->bind();
    //texture->bind(0);
    gl->glBindTexture(GL_TEXTURE_2D, tex);
    gl->glDrawArrays(GL_TRIANGLES, 0, 6);


    /*
    drawProgram->set1i("tex", 0);
    drawProgram->use();
    */

}
std::string msg::RaytracingTechnique::to_string(const glm::vec3 &d) {
    std::string res = "";
    res += std::to_string(d.x) + " ";
    res += std::to_string(d.y) + " ";
    res += std::to_string(d.z);
    return res;
}

void msg::RaytracingTechnique::update() {
    //std::cout << "RT update()" << std::endl;

    auto cameraPosition = glm::vec3(glm::inverse(orbitCamera->getView())[3]);
/*
    std::cout << "ray00 " << to_string(getRay(-1, 1, cameraPosition)) << std::endl;
    std::cout << "ray01 " << to_string(getRay(-1, 1, cameraPosition)) << std::endl;
    std::cout << "ray10 " << to_string(getRay(-1, 1, cameraPosition)) << std::endl;
    std::cout << "ray11 " << to_string(getRay(-1, 1, cameraPosition)) << std::endl;
*/
    computeShader->use();
    computeShader->set3fv("eye", glm::value_ptr(cameraPosition));
    computeShader->set3fv("ray00", glm::value_ptr(getRay(-1, -1, cameraPosition)));
    computeShader->set3fv("ray01", glm::value_ptr(getRay(-1,  1, cameraPosition)));
    computeShader->set3fv("ray10", glm::value_ptr(getRay( 1, -1, cameraPosition)));
    computeShader->set3fv("ray11", glm::value_ptr(getRay( 1,  1, cameraPosition)));
/*
    auto dbg_b(std::make_shared<ge::gl::Buffer>(
        gl->getFunctionTable(),
        sizeof(glm::vec4) * 1920 * 1080 
    ));
    dbg_b->bindBase(GL_SHADER_STORAGE_BUFFER, 3);
*/
    //texture->bindImage(0, 0, GL_RGBA32F, GL_WRITE_ONLY, GL_FALSE, 0);
    gl->glBindImageTexture(0, tex, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);

    int workGroupSizeX = viewport->x;
    int workGroupSizeY = viewport->y;

    gl->glDispatchCompute(100,100,1);
/*
    auto *dbg = static_cast<glm::vec4 *>(gl->glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));
    for(int i = 0; i < 100; ++i) {
        std::cout << dbg[i].x << " " << dbg[i].y << std::endl;
    }
*/
    //texture->bindImage(0, 0, GL_RGBA32F, GL_READ_WRITE, GL_FALSE, 0);
    gl->glBindImageTexture(0, 0, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
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