#pragma once 

#include <memory>

namespace msg {
    class Scene;
}

class SceneBuilder {
    public:
        static std::shared_ptr<msg::Scene> build();
};