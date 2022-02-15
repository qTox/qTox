/*
    Copyright © 2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "bootstrapnodeupdater.h"

#include "src/persistence/paths.h"

#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>

namespace NodeFields {
const QLatin1String status_udp{"status_udp"};
const QLatin1String status_tcp{"status_tcp"};
const QLatin1String ipv4{"ipv4"};
const QLatin1String ipv6{"ipv6"};
const QLatin1String public_key{"public_key"};
const QLatin1String udp_port{"port"};
const QLatin1String maintainer{"maintainer"};
// TODO(sudden6): make use of this field once we differentiate between TCP nodes, and bootstrap nodes
const QLatin1String tcp_ports{"tcp_ports"};
const QStringList neededFields{status_udp, status_tcp, ipv4, ipv6, public_key, udp_port, maintainer};
} // namespace NodeFields

namespace {
const QUrl NodeListAddress{"https://nodes.tox.chat/json"};
const QLatin1String jsonNodeArrayName{"nodes"};
const QLatin1String emptyAddress{"-"};
const QRegularExpression ToxPkRegEx(QString("(^|\\s)[A-Fa-f0-9]{%1}($|\\s)").arg(64));
const QLatin1String builtinNodesFile{":/conf/nodes.json"};

void jsonNodeToDhtServer(const QJsonObject& node, QList<DhtServer>& outList)
{
    // first check if the node in question has all needed fields
    bool found = true;
    for (const auto& key : NodeFields::neededFields) {
        found &= node.contains(key);
    }

    if (!found) {
        qDebug() << "Node is missing required fields.";
        return;
    }

    // only use nodes that provide at least UDP connection
    if (!node[NodeFields::status_udp].toBool(false)) {
        return;
    }

    const QString public_key = node[NodeFields::public_key].toString({});
    const auto udp_port = node[NodeFields::udp_port].toInt(-1);

    // nodes.tox.chat doesn't use empty strings for empty addresses
    QString ipv6_address = node[NodeFields::ipv6].toString({});
    if (ipv6_address == emptyAddress) {
        ipv6_address = QString{};
    }

    QString ipv4_address = node[NodeFields::ipv4].toString({});
    if (ipv4_address == emptyAddress) {
        ipv4_address = QString{};
    }

    if (ipv4_address.isEmpty() && ipv6_address.isEmpty()) {
        qWarning() << "Both ipv4 and ipv4 addresses are empty for" << public_key;
    }

    if (udp_port < 1 || udp_port > std::numeric_limits<uint16_t>::max()) {
        qDebug() << "Invalid port in nodes list:" << udp_port;
        return;
    }
    const quint16 udp_port_u16 = static_cast<quint16>(udp_port);

    if (!public_key.contains(ToxPkRegEx)) {
        qDebug() << "Invalid public key in nodes list" << public_key;
        return;
    }

    DhtServer server;
    server.statusUdp = true;
    server.statusTcp = node[NodeFields::status_udp].toBool(false);
    server.userId = public_key;
    server.udpPort = udp_port_u16;
    server.maintainer = maintainer;
    server.ipv4 = ipv4_address;
    server.ipv6 = ipv6_address;
    outList.append(server);
    return;
}

QList<DhtServer> jsonToNodeList(const QJsonDocument& nodeList)
{
    QList<DhtServer> result;

    if (!nodeList.isObject()) {
        qWarning() << "Bootstrap JSON is missing root object";
        return result;
    }

    QJsonObject rootObj = nodeList.object();
    if (!(rootObj.contains(jsonNodeArrayName) && rootObj[jsonNodeArrayName].isArray())) {
        qWarning() << "Bootstrap JSON is missing nodes array";
        return result;
    }
    QJsonArray nodes = rootObj[jsonNodeArrayName].toArray();
    for (const QJsonValueRef node : nodes) {
        if (node.isObject()) {
            jsonNodeToDhtServer(node.toObject(), result);
        }
    }

    return result;
}

QList<DhtServer> loadNodesFile(QString file)
{
    QFile nodesFile{file};
    if (!nodesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Couldn't read bootstrap nodes";
        return {};
    }

    QString nodesJson = nodesFile.readAll();
    nodesFile.close();

    auto jsonDoc = QJsonDocument::fromJson(nodesJson.toUtf8());
    if (jsonDoc.isNull()) {
        qWarning() << "Failed to parse JSON document";
        return {};
    }

    return jsonToNodeList(jsonDoc);
}

QByteArray serialize(QList<DhtServer> nodes)
{
    QJsonArray jsonNodes;
    for (auto& node : nodes) {
        QJsonObject nodeJson;
        nodeJson.insert(NodeFields::status_udp, node.statusUdp);
        nodeJson.insert(NodeFields::status_tcp, node.statusTcp);
        nodeJson.insert(NodeFields::ipv4, node.ipv4);
        nodeJson.insert(NodeFields::ipv6, node.ipv6);
        nodeJson.insert(NodeFields::public_key, node.userId);
        nodeJson.insert(NodeFields::udp_port, node.udpPort);
        nodeJson.insert(NodeFields::maintainer, node.maintainer);
        jsonNodes.append(nodeJson);
    }
    QJsonObject rootObj;
    rootObj.insert("nodes", jsonNodes);

    QJsonDocument doc{rootObj};
    return doc.toJson(QJsonDocument::Indented);
}
} // namespace

/**
 * @brief Fetches a list of currently online bootstrap nodes from node.tox.chat
 * @param proxy Proxy to use for the lookup, must outlive this object
 */
BootstrapNodeUpdater::BootstrapNodeUpdater(const QNetworkProxy& proxy, Paths& _paths, QObject* parent)
    : proxy{proxy}
    , paths{_paths}
    , QObject{parent}
{}

QList<DhtServer> BootstrapNodeUpdater::getBootstrapnodes()
{
    auto userFilePath = paths.getUserNodesFilePath();
    if (!QFile(userFilePath).exists()) {
        qInfo() << "Bootstrap node list not found, creating one with default nodes.";
        // deserialize and reserialize instead of just copying to strip out any unnecessary json, making it easier for
        // users to edit
        auto buildInNodes = loadNodesFile(builtinNodesFile);
        auto serializedNodes = serialize(buildInNodes);

        QFile outFile(userFilePath);
        outFile.open(QIODevice::WriteOnly | QIODevice::Text);
        outFile.write(serializedNodes.data(), serializedNodes.size());
        outFile.close();
    }

    return loadNodesFile(userFilePath);
}

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
 * @return List of bootstrap nodes on success, empty list on error
 */
QList<DhtServer> BootstrapNodeUpdater::loadDefaultBootstrapNodes()
{
    return loadNodesFile(builtinNodesFile);
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
