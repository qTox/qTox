#ifndef BOOTSTRAPNODEUPDATER_H
#define BOOTSTRAPNODEUPDATER_H

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>

#include "src/core/dhtserver.h"
#include "src/net/ibootstraplistgenerator.h"

class QNetworkReply;
class Paths;

class BootstrapNodeUpdater : public QObject, public IBootstrapListGenerator
{
    Q_OBJECT
public:
    explicit BootstrapNodeUpdater(const QNetworkProxy& proxy, Paths& _paths, QObject* parent = nullptr);
    QList<DhtServer> getBootstrapnodes() override;
    void requestBootstrapNodes();
    static QList<DhtServer> loadDefaultBootstrapNodes();

signals:
    void availableBootstrapNodes(QList<DhtServer> nodes);

private slots:
    void onRequestComplete(QNetworkReply* reply);

private:
    QList<DhtServer> loadUserBootrapNodes();
    QString getUserNodesFilePath();

private:
    QNetworkProxy proxy;
    QNetworkAccessManager nam;
    Paths& paths;
};

#endif // BOOTSTRAPNODEUPDATER_H
