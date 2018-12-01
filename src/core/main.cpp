#include <iostream>
#include <QGuiApplication>
#include <core/MouseEventHandler.h>
#include <QQuickWindow>
#include <core/Renderer.h>
#include <memory>
#include <core/SceneBuilder.h>



int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QQuickWindow qw;


    auto scene = SceneBuilder::build();
    msg::Renderer renderer(&qw);
    renderer.setScene(scene);

    std::shared_ptr<app::MouseEventHandler> mouseEventHandler(std::make_shared<app::MouseEventHandler>(renderer.orbitCamera));
    app.installEventFilter(mouseEventHandler.get());
    qw.resize(1200,1000);
    qw.show();
    return app.exec();
}
