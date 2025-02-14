#include "networkmanager.h"
#include <QtCore/QJsonDocument>
#include <QtNetwork/QNetworkRequest>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , manager(new QNetworkAccessManager(this))
    , baseUrl(QUrl("http://localhost:5000/api/v1"))  // From the spec
{
}

void NetworkManager::fetchLatestData()
{
    QUrl url = baseUrl.resolved(QUrl("can/latest"));
    QNetworkRequest request(url);
    
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleNetworkReply(reply);
        reply->deleteLater();
    });
}

void NetworkManager::fetchSystemStatus()
{
    QUrl url = baseUrl.resolved(QUrl("can/status"));
    QNetworkRequest request(url);
    
    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleNetworkReply(reply);
        reply->deleteLater();
    });
}

void NetworkManager::handleNetworkReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        emit error("Invalid JSON response");
        return;
    }

    QJsonObject json = doc.object();
    
    // Determine which type of response this is based on the URL
    QString path = reply->url().path();
    if (path.contains("/latest")) {
        emit dataReceived(json);
    } else if (path.contains("/status")) {
        emit systemStatusReceived(json);
    }
}