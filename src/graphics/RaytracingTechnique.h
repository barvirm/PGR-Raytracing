#pragma once
#include <graphics/VisualizationTechnique.h>
#include <memory>

namespace ge {
    namespace gl {
        class Program;
        class Context;
    }
}

namespace msg {
    class RaytracingTechnique : public VisualizationTechnique {
    public:
        void update();
        void draw();

        std::shared_ptr<ge::gl::Program> program;
        std::shared_ptr<ge::gl::Context> gl;
    };
}