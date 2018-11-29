#include <core/Renderer.h>

#include <iostream>

#include <geGL/geGL.h>
#include <geUtil/OrbitCamera.h>
#include <geUtil/PerspectiveCamera.h>

#include <util/ShaderReader.h>

msg::Renderer::Renderer(QObject *parent) :
    GERendererBase(parent),
    orbitCamera(std::make_shared<ge::util::OrbitCamera>()),
    perspectiveCamera(std::make_shared<ge::util::PerspectiveCamera>()),
    _inicializedVT(false)
{
    std::cout << "Renderer ctor" << std::endl;
    setupCamera();
}

void msg::Renderer::onViewportChanged() {
    std::cout << "onViewportChanged" << std::endl;
    _gl->glViewport(0, 0, _viewport->x, _viewport->y);
    perspectiveCamera->setAspect(_viewport->x / _viewport->y);
    
    if (_inicializedVT) {
        raytracingTechnique->onViewportChanged();
    }
}

void msg::Renderer::onContextCreated() {
    std::cout << "Renderer onContextCreated -> initVT" << std::endl;
    initRaytracingVT();
    _inicializedVT = true;
}


void msg::Renderer::setupGLState() {
    //std::cout << "Renderer setupGLState" << std::endl;
    _gl->glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    _gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    _gl->glEnable(GL_DEPTH_TEST);
}

void msg::Renderer::beforeRendering() {
    //std::cout << "Renderer beforeRendering" << std::endl;
    setupGLState();
    update();
    drawVT();
    _qqw->resetOpenGLState();
    _qqw->update();
}


void msg::Renderer::update() {

   
    if (_sceneToProcess) {
        std::cout << "Scene Processing" << std::endl;
        _sceneToProcess = false;
       
    }

    raytracingTechnique->update();
}

void msg::Renderer::setupCamera() {
    std::cout << "Renderer setupCamera" << std::endl;
    orbitCamera->setDistance(10.f);
    orbitCamera->setFocus(glm::vec3(0.0f, 0.5f, 0.0f));

    perspectiveCamera->setNear(1.f);
    perspectiveCamera->setFar(2.f);
    perspectiveCamera->setFovy(45.0f);
    perspectiveCamera->setAspect(1000.0f / 800.0f);
}

void msg::Renderer::setScene(std::shared_ptr<msg::Scene> &_scene) {
    scene = _scene;
}

bool msg::Renderer::initRaytracingVT() {
    std::cout << "Renderer initRaytracingVT" << std::endl;

    std::string shaderDir(APP_RESOURCES"/shaders/");
    auto vs(std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER  , util::loadFile(shaderDir + "texture_quad_vs.glsl")));
    auto fs(std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, util::loadFile(shaderDir + "texture_quad_fs.glsl")));
    auto gs(std::make_shared<ge::gl::Shader>(GL_GEOMETRY_SHADER, util::loadFile(shaderDir + "texture_quad_gs.glsl")));

    auto cs(std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER , util::loadFile(shaderDir + "raytracing_cs.glsl")));


    auto computeShader(std::make_shared<ge::gl::Program>(cs));
    auto drawProgram(std::make_shared<ge::gl::Program>(vs, gs, fs));
  
    raytracingTechnique = std::make_unique<msg::RaytracingTechnique>();
    raytracingTechnique->gl = _gl;
    raytracingTechnique->computeShader = computeShader;
    raytracingTechnique->drawProgram = drawProgram;
    raytracingTechnique->viewport = _viewport;
    raytracingTechnique->orbitCamera = orbitCamera;
    raytracingTechnique->perspectiveCamera = perspectiveCamera;
    raytracingTechnique->setScene(scene);
    raytracingTechnique->init();

    return true;
}


void msg::Renderer::drawVT() {
    raytracingTechnique->draw();
}
