#ifndef PROFILEIMPORTER_H
#define PROFILEIMPORTER_H

#include <QWidget>

class ProfileImporter : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileImporter(QWidget *parent = 0);
    bool importProfile();

signals:

public slots:
};

#endif // PROFILEIMPORTER_H
