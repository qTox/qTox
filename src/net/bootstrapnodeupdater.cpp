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
#include "src/core/toxpk.h"
#include "src/core/toxid.h"

#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QJsonArray>

#include <cstdint>

namespace NodeFields {
const QLatin1String status_udp{"status_udp"};
const QLatin1String status_tcp{"status_tcp"};
const QLatin1String ipv4{"ipv4"};
const QLatin1String ipv6{"ipv6"};
const QLatin1String public_key{"public_key"};
const QLatin1String udp_port{"port"};
const QLatin1String maintainer{"maintainer"};
const QLatin1String tcp_ports{"tcp_ports"};
const QStringList neededFields{status_udp, status_tcp, ipv4, ipv6, public_key, udp_port, tcp_ports, maintainer};
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

    const QString public_key = node[NodeFields::public_key].toString({});
    const auto udp_port = node[NodeFields::udp_port].toInt(-1);
    const auto status_udp = node[NodeFields::status_udp].toBool(false);
    const auto status_tcp = node[NodeFields::status_tcp].toBool(false);
    const QString maintainer = node[NodeFields::maintainer].toString({});

    std::vector<uint16_t> tcp_ports;
    const auto jsonTcpPorts = node[NodeFields::tcp_ports].toArray();
    for (int i = 0; i < jsonTcpPorts.count(); ++i) {
        const auto port = jsonTcpPorts.at(i).toInt();
        if (port < 1 || port > std::numeric_limits<uint16_t>::max()) {
            qDebug  () << "Invalid TCP port in nodes list:" << port;
            return;
        }
        tcp_ports.emplace_back(static_cast<uint16_t>(port));
    }

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

    if (status_udp && udp_port == -1) {
        qWarning() << "UDP enabled but no UDP port for" << public_key;
    }

    if (status_tcp && tcp_ports.empty()) {
        qWarning() << "TCP enabled but no TCP ports for:" << public_key;
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
    server.statusTcp = status_tcp;
    server.tcpPorts = tcp_ports;
    server.publicKey = ToxPk{public_key};
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

    QString nodesJson = QString::fromUtf8(nodesFile.readAll());
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
        nodeJson.insert(NodeFields::public_key, node.publicKey.toString());
        nodeJson.insert(NodeFields::udp_port, node.udpPort);
        nodeJson.insert(NodeFields::maintainer, node.maintainer);

        QJsonArray tcp_ports;
        for (size_t i = 0; i < node.tcpPorts.size(); ++i) {
            tcp_ports.push_back(node.tcpPorts.at(i));
        }
        nodeJson.insert(NodeFields::tcp_ports, tcp_ports);
        jsonNodes.append(nodeJson);
    }
    QJsonObject rootObj;
    rootObj.insert("nodes", jsonNodes);

    QJsonDocument doc{rootObj};
    return doc.toJson(QJsonDocument::Indented);
}

void createExampleBootstrapNodesFile(const Paths& paths)
{
    // deserialize and reserialize instead of just copying to strip out any unnecessary json, making it easier for
    // users to edit. Overwrite the file on every start to keep it up to date when our internal list updates.
    auto buildInNodes = loadNodesFile(builtinNodesFile);
    auto serializedNodes = serialize(buildInNodes);

    QFile outFile(paths.getExampleNodesFilePath());
    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    outFile.write(serializedNodes.data(), serializedNodes.size());
    outFile.close();
}
} // namespace

/**
 * @brief Fetches a list of currently online bootstrap nodes from node.tox.chat
 * @param proxy Proxy to use for the lookup, must outlive this object
 */
BootstrapNodeUpdater::BootstrapNodeUpdater(const QNetworkProxy& proxy_, Paths& paths_, QObject* parent)
    : QObject{parent}
    , proxy{proxy_}
    , paths{paths_}
{
    createExampleBootstrapNodesFile(paths_);
}

QList<DhtServer> BootstrapNodeUpdater::getBootstrapNodes() const
{
    auto userFilePath = paths.getUserNodesFilePath();
    if (QFile::exists(userFilePath)) {
        return loadNodesFile(userFilePath);
    } else {
        return loadNodesFile(builtinNodesFile);
    }
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
