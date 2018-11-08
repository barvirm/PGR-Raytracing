#include <graphics/RaytracingTechnique.h>
#include <iostream>

#include <geGL/geGL.h>

void msg::RaytracingTechnique::draw() {
    std::cout << "RT draw()" << std::endl;
    auto VAO = ge::gl::VertexArray(gl->getFunctionTable());
    program->use();
    VAO.bind();
    gl->glDrawArrays(GL_TRIANGLES, 0, 3);


}

void msg::RaytracingTechnique::update() {
    std::cout << "RT update()" << std::endl;
}