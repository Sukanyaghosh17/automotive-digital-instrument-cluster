#ifndef CLUSTER_BACKEND_HPP
#define CLUSTER_BACKEND_HPP

#include <QObject>
#include <QString>
#include <vsomeip/vsomeip.hpp>
#include "TripComputer.hpp"
#include <memory>

class ClusterBackend : public QObject
{
    Q_OBJECT

    // Telemetry Properties
    Q_PROPERTY(int speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(int rpm READ rpm NOTIFY rpmChanged)
    Q_PROPERTY(QString gear READ gear NOTIFY gearChanged)
    Q_PROPERTY(QString driveMode READ driveMode NOTIFY driveModeChanged)
    Q_PROPERTY(int fuel READ fuel NOTIFY fuelChanged)
    Q_PROPERTY(int engineTemp READ engineTemp NOTIFY engineTempChanged)

    // Warning Flags Properties
    Q_PROPERTY(bool engineWarning READ engineWarning NOTIFY warningsChanged)
    Q_PROPERTY(bool lowFuelWarning READ lowFuelWarning NOTIFY warningsChanged)
    Q_PROPERTY(bool absFault READ absFault NOTIFY warningsChanged)

    // Trip Computer Properties (Original Feature)
    Q_PROPERTY(double tripDistance READ tripDistance NOTIFY tripDataChanged)
    Q_PROPERTY(double avgSpeed READ avgSpeed NOTIFY tripDataChanged)

public:
    explicit ClusterBackend(QObject *parent = nullptr);
    ~ClusterBackend();

    int speed() const { return m_speed; }
    int rpm() const { return m_rpm; }
    QString gear() const { return m_gear; }
    QString driveMode() const { return m_driveMode; }
    int fuel() const { return m_fuel; }
    int engineTemp() const { return m_engineTemp; }

    bool engineWarning() const { return m_engineWarning; }
    bool lowFuelWarning() const { return m_lowFuelWarning; }
    bool absFault() const { return m_absFault; }

    double tripDistance() const { return m_trip.getTripDistanceKm(); }
    double avgSpeed() const { return m_trip.getAverageSpeedKmh(); }

    void initializeSomeIpClient();
    void onServiceRegistered();
    void requestService();

signals:
    void speedChanged();
    void rpmChanged();
    void gearChanged();
    void driveModeChanged();
    void fuelChanged();
    void engineTempChanged();
    void warningsChanged();
    void tripDataChanged();

private:
    void onPowertrainReceived(const std::shared_ptr<vsomeip::message>& msg);
    void onVehicleStatusReceived(const std::shared_ptr<vsomeip::message>& msg);
    void onWarningReceived(const std::shared_ptr<vsomeip::message>& msg);

    int m_speed;
    int m_rpm;
    QString m_gear;
    QString m_driveMode;
    int m_fuel;
    int m_engineTemp;
    bool m_engineWarning;
    bool m_lowFuelWarning;
    bool m_absFault;

    TripComputer m_trip;
    std::shared_ptr<vsomeip::application> m_app;

    static constexpr vsomeip::service_t    SERVICE_ID              = 0x1234;
    static constexpr vsomeip::instance_t   INSTANCE_ID             = 0x5678;
    static constexpr vsomeip::eventgroup_t EVENTGROUP_ID           = 0x0001;

    static constexpr vsomeip::event_t      EVENT_ID_POWERTRAIN     = 0x8001;
    static constexpr vsomeip::event_t      EVENT_ID_VEHICLE_STATUS = 0x8002;
    static constexpr vsomeip::event_t      EVENT_ID_WARNINGS       = 0x8003;
};

#endif // CLUSTER_BACKEND_HPP
