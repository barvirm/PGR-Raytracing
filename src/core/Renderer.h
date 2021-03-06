#pragma once

#include <core/GERendererBase.h>
#include <graphics/RaytracingTechnique.h>

namespace ge {
    namespace util {
        class OrbitCamera;
        class PerspectiveCamera;
    }
}



namespace msg {
    class Renderer : public GERendererBase {
        Q_OBJECT

    public:
        Renderer(QObject *parent = nullptr);
        void setScene(std::shared_ptr<msg::Scene> &_scene);
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
        bool _inicializedVT;
        std::unique_ptr<msg::RaytracingTechnique> raytracingTechnique;
        std::shared_ptr<msg::Scene> scene;

    private:
        void setupCamera();
        bool initVT();
        void drawVT();

        bool initRaytracingVT();
    };
}
