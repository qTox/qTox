#include "toxme.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCoreApplication>
#include <sodium/crypto_box.h>
#include <string>

const QString Toxme::apiUrl{"https://toxme.se/api"};

void Toxme::incrementNonce(unsigned char nonce[])
{
    auto nonceSize = crypto_box_NONCEBYTES;
    for (decltype(nonceSize) i=0; i<nonceSize; ++i)
    {
        if (nonce[i]==0xFFU)
        {
            if (i==nonceSize-1)
            {
                // Shouldn't matter, since we don't reuse keys
                // But still gets a warning because that's exceedingly unlikely to happen
                qWarning() << "Toxme::incrementNonce: Nonce overflow!";
                memset(nonce,0,nonceSize);
                return;
            }
            else
            {
                nonce[i]=0;
            }
        }
        else
        {
            nonce[i]++;
            return;
        }
    }
}

QByteArray Toxme::makeJsonRequest(QString json)
{
    QNetworkAccessManager netman;
    QNetworkRequest request{apiUrl};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = netman.post(request,json.toUtf8());
    while (!reply->isFinished())
        qApp->processEvents();

    return reply->readAll();
}

QByteArray Toxme::prepareEncryptedJson(int action, QString payload)
{
    static unsigned char nonce[crypto_box_NONCEBYTES]={0};

    unsigned char pk[crypto_box_PUBLICKEYBYTES];
    unsigned char sk[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(pk,sk);

    QByteArray payloadData = payload.toUtf8();
    const size_t mlen = crypto_box_ZEROBYTES+payloadData.size();
    unsigned char* payloadMsg = new unsigned char[mlen];
    unsigned char* payloadEnc = new unsigned char[mlen];
    memcpy(payloadMsg+crypto_box_ZEROBYTES,payloadData.data(),payloadData.size());

    crypto_box(payloadEnc,payloadMsg,mlen,nonce,pk,sk);
    QByteArray payloadEncData(reinterpret_cast<char*>(payloadEnc), mlen);
    delete[] payloadMsg;
    delete[] payloadEnc;

    const QString json{"{\"action\":"+QString().setNum(action)+","
                       "\"public_key\":\""+QByteArray(reinterpret_cast<char*>(pk),
                                                      crypto_box_PUBLICKEYBYTES)+"\","
                       "\"encrypted\":\""+payloadEncData+"\","
                       "\"nonce\":\""+QByteArray(reinterpret_cast<char*>(nonce),
                                                 crypto_box_NONCEBYTES)+"\"}"};
    incrementNonce(nonce);
    return json.toUtf8();
}

ToxID Toxme::lookup(QString address)
{
    // JSON injection ?
    address.replace('\\',"\\\\");
    address.replace('"',"\"");

    ToxID id;
    const QString json{"{\"action\":3,\"name\":\""+address+"\"}"};
    static const QByteArray pattern{"public_key\""};

    QByteArray response = makeJsonRequest(json);
    const int index = response.indexOf(pattern);
    if (index == -1)
        return id;
    response = response.mid(index+pattern.size());

    const int idStart = response.indexOf('"');
    if (idStart == -1)
        return id;
    response = response.mid(idStart+1);

    const int idEnd = response.indexOf('"');
    if (idEnd == -1)
        return id;
    response.truncate(idEnd);

    id = ToxID::fromString(response);
    return id;
}

bool Toxme::createAddress(ToxID id, QString address,
                              bool keepPrivate, QString bio)
{
    int privacy = keepPrivate ? 0 : 2;
    // JSON injection ?
    bio.replace('\\',"\\\\");
    bio.replace('"',"\"");
    address.replace('\\',"\\\\");
    address.replace('"',"\"");

    const QString payload{"{\"tox_id\":\""+id.toString()+"\","
                          "\"name\":\""+address+"\","
                          "\"privacy\":"+QString().setNum(privacy)+","
                          "\"bio\":\""+bio+"\","
                          "\"timestamp\":"+QString().setNum(time(0))+"}"};

    QByteArray response = makeJsonRequest(prepareEncryptedJson(1,payload));
    qDebug() << "payload:"<<payload;
    qDebug() << "response:"<<response;

    return true;
}
