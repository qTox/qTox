#include "removefrienddialog.h"
#include <QPushButton>


RemoveFriendDialog::RemoveFriendDialog(QWidget *parent, const Friend f)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
    ui.setupUi(this);
    QString name = f.getDisplayedName().toHtmlEscaped();
    QString text = tr("Are you sure you want to remove %1 from your contacts list?")
            .arg(QString("<b>%1</b>").arg(name));

    ui.label->setText(text);
    auto removeButton = ui.buttonBox->button(QDialogButtonBox::Ok);
    auto cancelButton = ui.buttonBox->button(QDialogButtonBox::Cancel);
    removeButton->setText(tr("Remove"));
    cancelButton->setDefault(true);
    adjustSize();
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &RemoveFriendDialog::onAccepted);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &RemoveFriendDialog::close);
    setFocus();
}

void RemoveFriendDialog::onAccepted()
{
    _accepted = true;
    close();
}
