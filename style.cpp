#include "style.h"
#include "settings.h"

#include <QFile>
#include <QDebug>

QString Style::get(const QString &filename)
{
    if (!Settings::getInstance().getUseNativeStyle())
    {
        QFile file(filename);
        if (file.open(QFile::ReadOnly | QFile::Text))
            return file.readAll();
        else
            qWarning() << "Style " << filename << " not found";
    }

    return QString();
}
