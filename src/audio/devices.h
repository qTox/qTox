#ifndef QTOX_AUDIO_H
#define QTOX_AUDIO_H

#include <QObject>
#include <QSharedData>

namespace qTox {
namespace Audio {

class Device
{
    class Private;

public:
    typedef QExplicitlySharedDataPointer<Private> PrivatePtr;

public:
    Device(Private* dev);
    Device(Device&&) = default;

    Device& operator=(Device& other);

public:
    bool isValid() const;
    QString name() const;

private:
    PrivatePtr d;
};

class Devices : public QObject
{
    Q_OBJECT
public:
    static Devices& self();
    inline static Devices& getInstance()
    {
        return self();
    }

private:
    Devices();
    ~Devices();

private:
    class Private;
    Private* d;
};

}
}

#endif
