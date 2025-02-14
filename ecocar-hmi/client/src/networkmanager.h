class NetworkManager : public QObject {
    Q_OBJECT
    
public:
    explicit NetworkManager(QObject *parent = nullptr);
    
    void fetchLatestData();
    void fetchSystemStatus();
    
signals:
    void dataReceived(const QJsonObject &data);
    void systemStatusReceived(const QJsonObject &status);
    void error(const QString &message);
    
private:
    QNetworkAccessManager *manager;
    QUrl baseUrl;
    
    void handleNetworkReply(QNetworkReply *reply);
};