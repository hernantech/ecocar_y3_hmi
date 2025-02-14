


class DataModel : public QObject {
    Q_OBJECT
    
    // Properties
    Q_PROPERTY(double vehicleSpeed READ vehicleSpeed NOTIFY vehicleSpeedChanged)
    Q_PROPERTY(double batteryVoltage READ batteryVoltage NOTIFY batteryVoltageChanged)
    Q_PROPERTY(double motorTemp READ motorTemp NOTIFY motorTempChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionStatusChanged)
    
public:
    explicit DataModel(QObject *parent = nullptr);
    
    // Getters
    double vehicleSpeed() const;
    double batteryVoltage() const;
    double motorTemp() const;
    bool isConnected() const;
    
signals:
    void vehicleSpeedChanged();
    void batteryVoltageChanged();
    void motorTempChanged();
    void connectionStatusChanged();
    void error(const QString &message);
    
private slots:
    void updateData();
    void handleNetworkError(const QString &error);
    
private:
    QTimer *updateTimer;
    NetworkManager *network;
    
    double m_vehicleSpeed;
    double m_batteryVoltage;
    double m_motorTemp;
    bool m_connected;
};