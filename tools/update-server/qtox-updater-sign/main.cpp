#include <sodium.h>
#include <QByteArray>
#include <QFile>

using namespace std;

int main(int argc, char* argv[])
{    
    QFile io;
    QByteArray msg;
    if (argc > 1)
    {
        msg = QByteArray(argv[1]);
    }
    else
    {
        io.open(stdin, QIODevice::ReadOnly);
        msg = io.readAll();
        io.close();
    }
    io.open(stdout, QIODevice::WriteOnly);

    QFile skeyFile("qtox-updater-skey");
    if (!skeyFile.open(QIODevice::ReadOnly))
    {
        io.write("ERROR: qtox-updater-sign can't open the secret (private) key file\n");
        io.close();
        return 1;
    }
    QByteArray skeyData = skeyFile.readAll();
    skeyFile.close();

    unsigned char sig[crypto_sign_BYTES];
    crypto_sign_detached(sig, nullptr, (unsigned char*)msg.data(), msg.size(), (unsigned char*)skeyData.data());

    io.write((char*)sig, crypto_sign_BYTES);
    io.write(msg);
    io.close();

    return 0;
}

