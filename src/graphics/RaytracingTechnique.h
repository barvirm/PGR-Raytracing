#pragma once
#include <graphics/VisualizationTechnique.h>
#include <memory>

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>


namespace ge {
    namespace gl {
        class Program;
        class Context;
        class Texture;
        class VertexArray;
        class Buffer;
    }
    namespace util {
        class OrbitCamera;
        class PerspectiveCamera;
    }
}


namespace msg {
    class RaytracingTechnique : public VisualizationTechnique {
    public:
        void init();
        void update();
        void draw();
        void onViewportChanged();

        std::shared_ptr<ge::gl::Program> program;
        std::shared_ptr<ge::gl::Program> computeShader;
        std::shared_ptr<ge::gl::Program> drawProgram;
        std::shared_ptr<ge::gl::Context> gl;
        std::shared_ptr<glm::vec2> viewport;
        
        std::shared_ptr<ge::util::OrbitCamera> orbitCamera;
        std::shared_ptr<ge::util::PerspectiveCamera> perspectiveCamera;

    protected:
        std::shared_ptr<ge::gl::Texture> texture; 
        std::shared_ptr<ge::gl::VertexArray> VAO;
        std::shared_ptr<ge::gl::Buffer> SSBO;

        glm::ivec3 workingGroupLayout;
        glm::vec3 getRay(float x, float y,const glm::vec3 &eye);

        unsigned int tex; // TODO do the same with ge::gl::texture

        std::string to_string(const glm::vec3 &d);
        std::string to_string(const glm::ivec3 &d);
    };
}
