#ifndef BOOTSTRAPNODEUPDATER_H
#define BOOTSTRAPNODEUPDATER_H

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>

#include "src/core/dhtserver.h"
#include "src/net/ibootstraplistgenerator.h"

class QNetworkReply;

class BootstrapNodeUpdater : public QObject, public IBootstrapListGenerator
{
    Q_OBJECT
public:
    explicit BootstrapNodeUpdater(const QNetworkProxy& proxy, QObject* parent = nullptr);
    QList<DhtServer> getBootstrapnodes() override;
    void requestBootstrapNodes();
    static QList<DhtServer> loadDefaultBootstrapNodes();

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
