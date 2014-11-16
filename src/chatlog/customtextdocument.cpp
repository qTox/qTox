#include "customtextdocument.h"
#include "../misc/smileypack.h"

#include <QIcon>
#include <QDebug>

CustomTextDocument::CustomTextDocument(QObject *parent)
    : QTextDocument(parent)
{

}

QVariant CustomTextDocument::loadResource(int type, const QUrl &name)
{
    if (type == QTextDocument::ImageResource && name.scheme() == "key")
        return SmileyPack::getInstance().getAsImage(name.fileName());

    return QTextDocument::loadResource(type, name);
}
