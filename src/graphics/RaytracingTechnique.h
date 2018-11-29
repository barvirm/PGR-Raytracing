#pragma once
#include <graphics/VisualizationTechnique.h>
#include <memory>
#include <functional>

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <graphics/Shapes.h>


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
    class Scene;
    class AABB;
}


namespace util {
    template<class T, typename FillFunction>
    std::vector<glm::vec4> convertToGPUFriendly(T &collection, FillFunction fill) {
        std::vector<glm::vec4> result;
        for(auto &t: collection) { fill(t, result); }
        return result;
    }
}


namespace msg {
    class RaytracingTechnique : public VisualizationTechnique {
    public:
        void init();
        void update();
        void draw();
        void onViewportChanged();
        void setScene(std::shared_ptr<msg::Scene> &_scene);

        std::shared_ptr<ge::gl::Program> program;
        std::shared_ptr<ge::gl::Program> computeShader;
        std::shared_ptr<ge::gl::Program> drawProgram;
        std::shared_ptr<ge::gl::Context> gl;
        std::shared_ptr<glm::vec2> viewport;
        std::shared_ptr<msg::Scene> scene;
        
        
        std::shared_ptr<ge::util::OrbitCamera> orbitCamera;
        std::shared_ptr<ge::util::PerspectiveCamera> perspectiveCamera;

    protected:
        std::shared_ptr<ge::gl::Texture> texture; 
        std::shared_ptr<ge::gl::VertexArray> VAO;
        std::shared_ptr<ge::gl::Buffer> SSBO;

        std::shared_ptr<ge::gl::Buffer> AABB_SSBO;
        std::shared_ptr<ge::gl::Buffer> spheres_SSBO;
        std::shared_ptr<ge::gl::Buffer> cylinder_SSBO;
        std::shared_ptr<ge::gl::Buffer> prefix_sum_primitives;

        glm::ivec3 workingGroupLayout;
        glm::vec3 getRay(float x, float y,const glm::vec3 &eye);

        std::string to_string(const glm::vec4 &d);
        std::string to_string(const glm::ivec3 &d);

    };
}

