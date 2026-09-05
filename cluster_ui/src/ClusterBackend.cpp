#include "ClusterBackend.hpp"
#include <QDebug>

ClusterBackend::ClusterBackend(QObject *parent)
    : QObject(parent),
      m_speed(0),
      m_rpm(800),
      m_gear("P"),
      m_driveMode("COMFORT"),
      m_fuel(85),
      m_engineTemp(75),
      m_engineWarning(false),
      m_lowFuelWarning(false),
      m_absFault(false),
      m_app(nullptr)
{
}

ClusterBackend::~ClusterBackend()
{
    if (m_app)
    {
        m_app->clear_all_handler();
        m_app->stop();
    }
}

void ClusterBackend::initializeSomeIpClient()
{
    try
    {
        m_app = vsomeip::runtime::get()->create_application("ClusterUI");
        if (!m_app || !m_app->init())
        {
            throw std::runtime_error("[ClusterBackend] Failed to initialize vsomeip application");
        }

        m_app->register_state_handler(
            [this](vsomeip::state_type_e state)
            {
                if (state == vsomeip::state_type_e::ST_REGISTERED)
                {
                    this->onServiceRegistered();
                }
            }
        );

        // Handler for Powertrain Event (0x8001)
        m_app->register_message_handler(
            SERVICE_ID, INSTANCE_ID, EVENT_ID_POWERTRAIN,
            [this](const std::shared_ptr<vsomeip::message>& msg)
            {
                this->onPowertrainReceived(msg);
            }
        );

        // Handler for Vehicle Status Event (0x8002)
        m_app->register_message_handler(
            SERVICE_ID, INSTANCE_ID, EVENT_ID_VEHICLE_STATUS,
            [this](const std::shared_ptr<vsomeip::message>& msg)
            {
                this->onVehicleStatusReceived(msg);
            }
        );

        // Handler for Warning Event (0x8003)
        m_app->register_message_handler(
            SERVICE_ID, INSTANCE_ID, EVENT_ID_WARNINGS,
            [this](const std::shared_ptr<vsomeip::message>& msg)
            {
                this->onWarningReceived(msg);
            }
        );

        // Availability handler
        m_app->register_availability_handler(
            SERVICE_ID, INSTANCE_ID,
            [this](vsomeip::service_t service, vsomeip::instance_t instance, bool is_available)
            {
                qDebug() << "[ClusterBackend] Gateway Service 0x" << QString::number(service, 16)
                         << "is" << (is_available ? "AVAILABLE" : "UNAVAILABLE");
                if (is_available)
                {
                    this->requestService();
                }
            }
        );

        m_app->start();
    }
    catch (const std::exception& e)
    {
        qCritical() << "[ClusterBackend] Exception in initializeSomeIpClient:" << e.what();
        throw;
    }
}

void ClusterBackend::onServiceRegistered()
{
    try
    {
        m_app->request_service(SERVICE_ID, INSTANCE_ID);

        std::set<vsomeip::eventgroup_t> groups;
        groups.insert(EVENTGROUP_ID);

        m_app->request_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_POWERTRAIN, groups);
        m_app->request_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_VEHICLE_STATUS, groups);
        m_app->request_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_WARNINGS, groups);

        qDebug() << "[ClusterBackend] Service requested and events registered.";
    }
    catch (const std::exception& e)
    {
        qCritical() << "[ClusterBackend] Exception in onServiceRegistered:" << e.what();
    }
}

void ClusterBackend::requestService()
{
    try
    {
        std::set<vsomeip::eventgroup_t> groups;
        groups.insert(EVENTGROUP_ID);

        m_app->request_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_POWERTRAIN, groups);
        m_app->request_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_VEHICLE_STATUS, groups);
        m_app->request_event(SERVICE_ID, INSTANCE_ID, EVENT_ID_WARNINGS, groups);

        m_app->subscribe(SERVICE_ID, INSTANCE_ID, EVENTGROUP_ID);
        qDebug() << "[ClusterBackend] Subscribed to Eventgroup 0x" << QString::number(EVENTGROUP_ID, 16);
    }
    catch (const std::exception& e)
    {
        qCritical() << "[ClusterBackend] Exception in requestService:" << e.what();
    }
}

// =========================================================================
// SOME/IP Payload Deserialization (AUTOSAR Standard: Big-Endian)
// =========================================================================

void ClusterBackend::onPowertrainReceived(const std::shared_ptr<vsomeip::message>& msg)
{
    auto payload = msg->get_payload();
    if (!payload) return;

    const vsomeip::byte_t* data = payload->get_data();
    vsomeip::length_t length = payload->get_length();

    // PowertrainEvent (0x8001): [uint16 speed][uint16 rpm][uint8 gear][uint8 mode]
    if (length >= 2)
    {
        uint16_t speed = (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
        if (m_speed != static_cast<int>(speed))
        {
            m_speed = static_cast<int>(speed);
            emit speedChanged();

            m_trip.update(speed);
            emit tripDataChanged();
        }
    }

    if (length >= 4)
    {
        uint16_t rpm = (static_cast<uint16_t>(data[2]) << 8) | static_cast<uint16_t>(data[3]);
        if (m_rpm != static_cast<int>(rpm))
        {
            m_rpm = static_cast<int>(rpm);
            emit rpmChanged();
        }
    }

    if (length >= 5)
    {
        QString gearStr = "P";
        switch (data[4])
        {
            case 0: gearStr = "P"; break;
            case 1: gearStr = "R"; break;
            case 2: gearStr = "N"; break;
            case 3: gearStr = "D"; break;
            default: gearStr = "P"; break;
        }
        if (m_gear != gearStr)
        {
            m_gear = gearStr;
            emit gearChanged();
        }
    }

    if (length >= 6)
    {
        QString modeStr = "COMFORT";
        switch (data[5])
        {
            case 0: modeStr = "ECO"; break;
            case 1: modeStr = "COMFORT"; break;
            case 2: modeStr = "SPORT"; break;
            default: modeStr = "COMFORT"; break;
        }
        if (m_driveMode != modeStr)
        {
            m_driveMode = modeStr;
            emit driveModeChanged();
        }
    }
}

void ClusterBackend::onVehicleStatusReceived(const std::shared_ptr<vsomeip::message>& msg)
{
    auto payload = msg->get_payload();
    if (!payload) return;

    const vsomeip::byte_t* data = payload->get_data();
    vsomeip::length_t length = payload->get_length();

    // VehicleStatusEvent (0x8002): [uint8 fuel][int16 temp]
    if (length >= 1)
    {
        uint8_t fuel = data[0];
        if (m_fuel != static_cast<int>(fuel))
        {
            m_fuel = static_cast<int>(fuel);
            emit fuelChanged();
        }
    }

    if (length >= 3)
    {
        uint16_t rawTemp = (static_cast<uint16_t>(data[1]) << 8) | static_cast<uint16_t>(data[2]);
        int16_t temp = static_cast<int16_t>(rawTemp);
        if (m_engineTemp != static_cast<int>(temp))
        {
            m_engineTemp = static_cast<int>(temp);
            emit engineTempChanged();
        }
    }
}

void ClusterBackend::onWarningReceived(const std::shared_ptr<vsomeip::message>& msg)
{
    auto payload = msg->get_payload();
    if (!payload) return;

    const vsomeip::byte_t* data = payload->get_data();
    vsomeip::length_t length = payload->get_length();

    // WarningEvent (0x8003): [uint8 flags]
    if (length >= 1)
    {
        uint8_t flags = data[0];
        bool overheat = (flags & 0x01) != 0;
        bool lowFuel  = (flags & 0x02) != 0;
        bool absFault = (flags & 0x04) != 0;

        if (m_engineWarning != overheat || m_lowFuelWarning != lowFuel || m_absFault != absFault)
        {
            m_engineWarning = overheat;
            m_lowFuelWarning = lowFuel;
            m_absFault = absFault;
            emit warningsChanged();
        }
    }
}
