#include "GeoViewWidget.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("QGeoView Samples");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    GeoViewWidget window;
    window.show();
    return app.exec();
}
