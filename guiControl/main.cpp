#include "parameter.h"
#include "tcpServer.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <iostream>
#include <string>
#include <QQmlContext>
#include "udp_client.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    Parameter* parameter = new Parameter();
    engine.rootContext()->setContextProperty("parameter", parameter);

    TcpServer server("0.0.0.0", 8888, parameter);
    udp_client client("192.168.68", 8888, parameter);

    std::thread udp_client([&client]() {
        client.startConnect();
    });
    udp_client.detach();

    std::thread serverThread([&server]() {
        server.start_accept();
    });
    serverThread.detach();

    std::cout << "Main thread is running." << std::endl;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("guiControl", "Main");

    return app.exec();
}
