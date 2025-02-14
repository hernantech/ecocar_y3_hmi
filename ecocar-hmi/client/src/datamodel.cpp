#include "datamodel.h"
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>

DataModel::DataModel(QObject *parent)
    : QObject(parent)
    , updateTimer(new QTimer(this))
    , network(new NetworkManager(this))
    , m_vehicleSpeed(0.0)
    , m_batteryVoltage(0.0)
    , m_motorTemp(0.0)
    , m_connected(false)
{
    // Connect network signals
    connect(network, &NetworkManager::dataReceived, 
            this, &DataModel::handleDataReceived);
    connect(network, &NetworkManager::systemStatusReceived,
            this, &DataModel::handleStatusReceived);
    connect(network, &NetworkManager::error,
            this, &DataModel::handleNetworkError);
    
    // Set up update timer (100ms from spec)
    updateTimer->setInterval(100);
    connect(updateTimer, &QTimer::timeout,
            this, &DataModel::updateData);
    
    // Start updates
    updateTimer->start();
}

double DataModel::vehicleSpeed() const
{
    return m_vehicleSpeed;
}

double DataModel::batteryVoltage() const
{
    return m_batteryVoltage;
}

double DataModel::motorTemp() const
{
    return m_motorTemp;
}

bool DataModel::isConnected() const
{
    return m_connected;
}

void DataModel::updateData()
{
    network->fetchLatestData();
    network->fetchSystemStatus();
}

void DataModel::handleNetworkError(const QString &error)
{
    emit this->error(error);
    m_connected = false;
    emit connectionStatusChanged();
}

void DataModel::handleDataReceived(const QJsonObject &data)
{
    QJsonObject messages = data["messages"].toObject();
    
    // Update vehicle speed
    if (messages.contains("speed")) {
        double newSpeed = messages["speed"].toObject()["value"].toDouble();
        if (m_vehicleSpeed != newSpeed) {
            m_vehicleSpeed = newSpeed;
            emit vehicleSpeedChanged();
        }
    }
    
    // Update battery voltage
    if (messages.contains("battery_voltage")) {
        double newVoltage = messages["battery_voltage"].toObject()["value"].toDouble();
        if (m_batteryVoltage != newVoltage) {
            m_batteryVoltage = newVoltage;
            emit batteryVoltageChanged();
        }
    }
    
    // Update motor temperature
    if (messages.contains("motor_temp")) {
        double newTemp = messages["motor_temp"].toObject()["value"].toDouble();
        if (m_motorTemp != newTemp) {
            m_motorTemp = newTemp;
            emit motorTempChanged();
        }
    }
}

void DataModel::handleStatusReceived(const QJsonObject &status)
{
    bool newConnected = status["connected"].toBool();
    if (m_connected != newConnected) {
        m_connected = newConnected;
        emit connectionStatusChanged();
    }
}