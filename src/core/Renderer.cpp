#include <core/Renderer.h>

#include <iostream>

#include <geGL/geGL.h>
#include <geUtil/OrbitCamera.h>
#include <geUtil/PerspectiveCamera.h>


#include <graphics/RaytracingTechnique.h>
#include <util/ShaderReader.h>



msg::Renderer::Renderer(QObject *parent) :
    GERendererBase(parent),
    orbitCamera(std::make_shared<ge::util::OrbitCamera>()),
    perspectiveCamera(std::make_shared<ge::util::PerspectiveCamera>())
{
    std::cout << "Renderer ctor" << std::endl;
    setupCamera();
}

void msg::Renderer::onViewportChanged() {
    std::cout << "onViewportChanged" << std::endl;
    _gl->glViewport(0, 0, _viewport->x, _viewport->y);
    perspectiveCamera->setAspect(_viewport->x / _viewport->y);
}

void msg::Renderer::onContextCreated() {
    initVT();
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
    for(auto &vt : _visualizationTechniques) {
        vt->update();
    }
}

void msg::Renderer::setupCamera() {
    std::cout << "Renderer setupCamera" << std::endl;
    orbitCamera->setDistance(10.f);
    //orbitCamera->setYAngle(3.1415f/1.2f);
    //orbitCamera->setXAngle(3.1415f/6.5f);
    orbitCamera->setFocus(glm::vec3(0.0f, 0.5f, 0.0f));

    perspectiveCamera->setNear(1.f);
    perspectiveCamera->setFar(2.f);
    perspectiveCamera->setFovy(45.0f);
    perspectiveCamera->setAspect(1000.0f / 800.0f);
}

bool msg::Renderer::initRaytracingVT() {
    std::cout << "Renderer initRaytracingVT" << std::endl;

    std::string shaderDir(APP_RESOURCES"/shaders/");
    /* TESTING */
    auto vs(std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER  , util::loadFile(shaderDir + "triag_vs.glsl")));
    auto fs(std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, util::loadFile(shaderDir + "red_fs.glsl")));
    /***********/
    auto vs_d(std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER, util::loadFile(shaderDir + "texture_quad_vs.glsl")));
    auto fs_d(std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER, util::loadFile(shaderDir + "texture_quad_fs.glsl")));
    
    auto cs(std::make_shared<ge::gl::Shader>(GL_COMPUTE_SHADER , util::loadFile(shaderDir + "raytracing_cs.glsl")));


    auto program(std::make_shared<ge::gl::Program>(vs, fs));
    auto computeShader(std::make_shared<ge::gl::Program>(cs));
    auto drawQuad(std::make_shared<ge::gl::Program>(vs_d, fs_d));
  
    auto raytracingVT(std::make_unique<msg::RaytracingTechnique>());
    raytracingVT->gl = _gl;
    raytracingVT->program = program;
    raytracingVT->computeShader = computeShader;
    raytracingVT->drawProgram = drawQuad;
    raytracingVT->viewport = _viewport;
    raytracingVT->orbitCamera = orbitCamera;
    raytracingVT->perspectiveCamera = perspectiveCamera;
    raytracingVT->init();

    _visualizationTechniques.emplace_back(std::move(raytracingVT));
    return true;
}


bool msg::Renderer::initVT() {
    std::cout << "Renderer initVT" << std::endl;
    bool i = initRaytracingVT();

    return i;
}

void msg::Renderer::drawVT() {
    // draw VT
    for(auto &vt : _visualizationTechniques) {
        vt->draw();
    }
    std::cout << orbitCamera->getDistance() << std::endl;
}
