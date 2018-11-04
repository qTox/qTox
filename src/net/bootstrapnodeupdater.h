#ifndef BOOTSTRAPNODEUPDATER_H
#define BOOTSTRAPNODEUPDATER_H

#include <QList>
#include <QObject>
#include <QNetworkProxy>
#include <QNetworkAccessManager>

#include "src/core/dhtserver.h"

class QNetworkReply;

class BootstrapNodeUpdater : public QObject
{
    Q_OBJECT
public:
    explicit BootstrapNodeUpdater(const QNetworkProxy& proxy, QObject* parent = nullptr);
    bool requestBootstrapNodes();

signals:
    void availableBootstrapNodes(QList<DhtServer> nodes);

private slots:
    void onRequestComplete(QNetworkReply* reply);
private:
    static QList<DhtServer> jsonToNodeList(const QJsonDocument& nodeList);
    static void jsonNodeToDhtServer(const QJsonObject& node, QList<DhtServer>& outList);

private:
    QNetworkProxy proxy;
    QNetworkAccessManager nam;
};

#endif // BOOTSTRAPNODEUPDATER_H
