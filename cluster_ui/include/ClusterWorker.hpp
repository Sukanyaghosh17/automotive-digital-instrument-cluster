#ifndef CLUSTER_WORKER_HPP
#define CLUSTER_WORKER_HPP

#include <QObject>
#include "ClusterBackend.hpp"

class ClusterWorker : public QObject
{
    Q_OBJECT

public:
    explicit ClusterWorker(ClusterBackend *backend, QObject *parent = nullptr);
    ~ClusterWorker() override = default;

public slots:
    void run();

signals:
    void finished();
    void error(const QString &err);

private:
    ClusterBackend *m_backend;
};

#endif // CLUSTER_WORKER_HPP
