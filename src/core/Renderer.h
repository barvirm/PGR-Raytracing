#pragma once

#include <core/GERendererBase.h>
#include <graphics/VisualizationTechnique.h>
namespace ge {
    namespace util {
        class OrbitCamera;
        class PerspectiveCamera;
    }
}

namespace msg {
    class RaytracingTechnique;
    //class VizualizationTechnique;
}


namespace msg {
    class Renderer : public GERendererBase {
        Q_OBJECT

    public:
        Renderer(QObject *parent = nullptr);
        void setupGLState();
        std::shared_ptr<ge::util::OrbitCamera> orbitCamera;
        std::shared_ptr<ge::util::PerspectiveCamera> perspectiveCamera;

    public slots:
        virtual void beforeRendering() override;
   protected:
        virtual void update();
        virtual void onViewportChanged() override;
        virtual void onContextCreated() override;

        bool _sceneToProcess;
        std::vector<std::unique_ptr<msg::VisualizationTechnique>> _visualizationTechniques;

    private:
        void setupCamera();
        bool initVT();
        void drawVT();

        bool initRaytracingVT();
    };
}
