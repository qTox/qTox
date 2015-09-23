#include "removefrienddialog.h"
#include <QPushButton>


RemoveFriendDialog::RemoveFriendDialog(QWidget *parent, const Friend *f)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
    ui.setupUi(this);
    ui.label->setText(ui.label->text().replace("&lt;name&gt;", f->getDisplayedName()));
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
