#include "MainWindow.h"

#include <QApplication>
#include <QIcon>
#include <QSurfaceFormat>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("xLights Qt");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("xLights");

    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;
    w.show();

    return app.exec();
}
