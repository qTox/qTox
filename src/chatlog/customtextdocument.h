#ifndef CUSTOMTEXTDOCUMENT_H
#define CUSTOMTEXTDOCUMENT_H

#include <QTextDocument>

class CustomTextDocument : public QTextDocument
{
    Q_OBJECT
public:
    explicit CustomTextDocument(QObject *parent = 0);

protected:
    virtual QVariant loadResource(int type, const QUrl &name);
};

#endif // CUSTOMTEXTDOCUMENT_H
