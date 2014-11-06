#ifndef TOXURI_H
#define TOXURI_H

#include <QDialog>

/// Shows a dialog asking whether or not to add this tox address as a friend
/// Will wait until the core is ready first
void handleToxURI(const QString& toxURI);

// Internals
class QByteArray;
class QPlainTextEdit;
void toxURIEventHandler(const QByteArray& eventData);
class ToxURIDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ToxURIDialog(QWidget *parent, const QString &userId, const QString &message);
    QString getRequestMessage();

private:
    QPlainTextEdit *messageEdit;
};

#endif // TOXURI_H
