#include "ping_viewer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PingViewer viewer;
    viewer.show();

    return app.exec();
}
