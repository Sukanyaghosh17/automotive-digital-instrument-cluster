#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QThread>
#include <QDebug>
#include "ClusterBackend.hpp"
#include "ClusterWorker.hpp"

int main(int argc, char *argv[])
{
    // Enable High DPI scaling
    QGuiApplication::setApplicationName("AutomotiveDigitalInstrumentCluster");
    QGuiApplication::setOrganizationName("AutomotiveCluster");
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Backend instance
    ClusterBackend backend;
    engine.rootContext()->setContextProperty("cluster", &backend);

    // QThread worker for vsomeip event processing
    QThread workerThread;
    ClusterWorker worker(&backend);
    worker.moveToThread(&workerThread);

    QObject::connect(&workerThread, &QThread::started, &worker, &ClusterWorker::run);
    QObject::connect(&worker, &ClusterWorker::finished, &workerThread, &QThread::quit);

    QObject::connect(&worker, &ClusterWorker::error, [](const QString &err) {
        qCritical() << "[Main] Cluster Worker failure:" << err;
    });

    workerThread.start();

    // Load main QML
    const QUrl url(QStringLiteral("qrc:/ClusterUI/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    int result = app.exec();

    workerThread.quit();
    workerThread.wait(1000);

    return result;
}
