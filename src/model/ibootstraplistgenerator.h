#ifndef IBOOTSTRAPLISTGENERATOR_H
#define IBOOTSTRAPLISTGENERATOR_H

#include <QList>
class DhtServer;

class IBootstrapListGenerator
{
public:
    virtual ~IBootstrapListGenerator() = default;
    virtual QList<DhtServer> getBootstrapnodes() = 0;
};

#endif // IBOOTSTRAPLISTGENERATOR_H
