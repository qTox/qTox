#include "bootstrapnodeupdater.h"

#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>

namespace {
const QUrl NodeListAddress{"https://nodes.tox.chat/json"};
const QLatin1Literal jsonNodeArrayName{"nodes"};
const QLatin1Literal emptyAddress{"-"};
const QRegularExpression ToxPkRegEx(QString("(^|\\s)[A-Fa-f0-9]{%1}($|\\s)").arg(64));
const QLatin1Literal builtinNodesFile{":/conf/nodes.json"};
} // namespace

namespace NodeFields {
const QLatin1Literal status_udp{"status_udp"};
const QLatin1Literal status_tcp{"status_tcp"};
const QLatin1Literal ipv4{"ipv4"};
const QLatin1Literal ipv6{"ipv6"};
const QLatin1Literal public_key{"public_key"};
const QLatin1Literal port{"port"};
const QLatin1Literal maintainer{"maintainer"};
// TODO(sudden6): make use of this field once we differentiate between TCP nodes, and bootstrap nodes
const QLatin1Literal tcp_ports{"tcp_ports"};
const QStringList neededFields{status_udp, status_tcp, ipv4, ipv6, public_key, port, maintainer};
} // namespace NodeFields

/**
 * @brief Fetches a list of currently online bootstrap nodes from node.tox.chat
 * @param proxy Proxy to use for the lookup, must outlive this object
 */
BootstrapNodeUpdater::BootstrapNodeUpdater(const QNetworkProxy& proxy, QObject* parent)
    : QObject{parent}
    , proxy{proxy}
{}

void BootstrapNodeUpdater::requestBootstrapNodes()
{
    nam.setProxy(proxy);
    connect(&nam, &QNetworkAccessManager::finished, this, &BootstrapNodeUpdater::onRequestComplete);

    QNetworkRequest request{NodeListAddress};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    nam.get(request);
}

/**
 * @brief Loads the list of built in boostrap nodes
 * @return List of bootstrapnodes on success, empty list on error
 */
QList<DhtServer> BootstrapNodeUpdater::loadDefaultBootstrapNodes()
{
    QFile nodesFile{builtinNodesFile};
    if (!nodesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Couldn't read bootstrap nodes";
        return {};
    }

    QString nodesJson = nodesFile.readAll();
    nodesFile.close();
    QJsonDocument d = QJsonDocument::fromJson(nodesJson.toUtf8());
    if (d.isNull()) {
        qWarning() << "Failed to parse JSON document";
        return {};
    }

    return jsonToNodeList(d);
}

void BootstrapNodeUpdater::onRequestComplete(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        nam.clearAccessCache();
        emit availableBootstrapNodes({});
        return;
    }

    // parse the reply JSON
    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll());
    if (jsonDocument.isNull()) {
        emit availableBootstrapNodes({});
        return;
    }

    QList<DhtServer> result = jsonToNodeList(jsonDocument);

    emit availableBootstrapNodes(result);
}

void BootstrapNodeUpdater::jsonNodeToDhtServer(const QJsonObject& node, QList<DhtServer>& outList)
{
    // first check if the node in question has all needed fields
    bool found = true;
    for (const auto& key : NodeFields::neededFields) {
        found |= node.contains(key);
    }

    if (!found) {
        return;
    }

    // only use nodes that provide at least UDP connection
    if (!node[NodeFields::status_udp].toBool(false)) {
        return;
    }

    const QString public_key = node[NodeFields::public_key].toString({});
    const int port = node[NodeFields::port].toInt(-1);

    // nodes.tox.chat doesn't use empty strings for empty addresses
    QString ipv6_address = node[NodeFields::ipv6].toString({});
    if (ipv6_address == emptyAddress) {
        ipv6_address = QString{};
    }

    QString ipv4_address = node[NodeFields::ipv4].toString({});
    if (ipv4_address == emptyAddress) {
        ipv4_address = QString{};
    }

    const QString maintainer = node[NodeFields::maintainer].toString({});

    if (port < 1 || port > std::numeric_limits<uint16_t>::max()) {
        return;
    }
    const quint16 port_u16 = static_cast<quint16>(port);

    if (!public_key.contains(ToxPkRegEx)) {
        return;
    }

    DhtServer server;
    server.userId = public_key;
    server.port = port_u16;
    server.name = maintainer;

    if (!ipv4_address.isEmpty()) {
        server.address = ipv4_address;
        outList.append(server);
    }
    // avoid adding the same server twice in case they use the same dns name for v6 and v4
    if (!ipv6_address.isEmpty() && ipv4_address != ipv6_address) {
        server.address = ipv6_address;
        outList.append(server);
    }

    return;
}

QList<DhtServer> BootstrapNodeUpdater::jsonToNodeList(const QJsonDocument& nodeList)
{
    QList<DhtServer> result;

    if (!nodeList.isObject()) {
        return result;
    }

    QJsonObject rootObj = nodeList.object();
    if (!(rootObj.contains(jsonNodeArrayName) && rootObj[jsonNodeArrayName].isArray())) {
        return result;
    }
    QJsonArray nodes = rootObj[jsonNodeArrayName].toArray();
    for (const auto& node : nodes) {
        if (node.isObject()) {
            jsonNodeToDhtServer(node.toObject(), result);
        }
    }

    return result;
}
