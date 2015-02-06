#ifndef GUI_H
#define GUI_H

#include <QObject>

class QWidget;

/// Abstracts the GUI from the target backend (AndroidGUI, DesktopGUI, ...)
/// All the functions exposed here are thread-safe
/// Prefer calling this class to calling a GUI backend directly
class GUI : public QObject
{
    Q_OBJECT
public:
    static GUI& getInstance();
    static QString itemInputDialog(QWidget * parent, const QString & title,
                    const QString & label, const QStringList & items,
                    int current = 0, bool editable = true, bool * ok = 0,
                    Qt::WindowFlags flags = 0,
                    Qt::InputMethodHints hints = Qt::ImhNone);

private:
    explicit GUI(QObject *parent = 0);

private slots:
    QString _itemInputDialog(QWidget * parent, const QString & title,
                        const QString & label, const QStringList & items,
                        int current = 0, bool editable = true, bool * ok = 0,
                        Qt::WindowFlags flags = 0,
                        Qt::InputMethodHints inputMethodHints = Qt::ImhNone);
};

#endif // GUI_H
