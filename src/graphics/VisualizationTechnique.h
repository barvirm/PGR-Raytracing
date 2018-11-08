#pragma once

namespace msg {
    class VisualizationTechnique {
    public:
        virtual ~VisualizationTechnique() = default;

        virtual void update() = 0;
        virtual void draw() = 0;
    };
}