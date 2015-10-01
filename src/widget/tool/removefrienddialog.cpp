#include "removefrienddialog.h"
#include <QPushButton>


RemoveFriendDialog::RemoveFriendDialog(QWidget *parent, const Friend *f)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
    ui.setupUi(this);
    ui.label->setText(ui.label->text().replace("&lt;name&gt;", f->getDisplayedName().toHtmlEscaped()));
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
