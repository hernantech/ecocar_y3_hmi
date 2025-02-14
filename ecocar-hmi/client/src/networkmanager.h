#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QtCore/QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>

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

#endif // NETWORKMANAGER_H