#include "ClusterWorker.hpp"
#include <QDebug>

ClusterWorker::ClusterWorker(ClusterBackend *backend, QObject *parent)
    : QObject(parent), m_backend(backend)
{
}

void ClusterWorker::run()
{
    try
    {
        qDebug() << "[ClusterWorker] Starting SOME/IP background client thread...";
        m_backend->initializeSomeIpClient();
        emit finished();
    }
    catch (const std::exception &e)
    {
        qCritical() << "[ClusterWorker] Error in client thread:" << e.what();
        emit error(QString::fromUtf8(e.what()));
    }
}
