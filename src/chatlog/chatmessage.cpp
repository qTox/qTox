#include "chatmessage.h"
#include "content/text.h"
#include "content/spinner.h"

#include <QDateTime>

ChatMessage::ChatMessage(QGraphicsScene *scene, const QString& author ,ChatLineContent *content)
    : ChatLine(scene)
{
    addColumn(new Text(author, true), ColumnFormat(75.0, ColumnFormat::FixedSize, 1, ColumnFormat::Right));
    addColumn(content, ColumnFormat(1.0, ColumnFormat::VariableSize));
    addColumn(new Spinner(QSizeF(16, 16)), ColumnFormat(50.0, ColumnFormat::FixedSize, 1, ColumnFormat::Right));
}

void ChatMessage::markAsSent(const QDateTime &time)
{
    // remove the spinner and replace it by $time
    replaceContent(2, new Text(time.toString("hh:mm"), true));
}
