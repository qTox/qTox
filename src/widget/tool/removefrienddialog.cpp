#include "removefrienddialog.h"
#include <QPushButton>


RemoveFriendDialog::RemoveFriendDialog(QWidget *parent, const Friend *f)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
    ui.setupUi(this);
    ui.label->setText(ui.label->text().replace("&lt;name&gt;", f->getDisplayedName()));
    auto removeButton = ui.buttonBox->button(QDialogButtonBox::Ok);
    removeButton->setEnabled(false);
    removeButton->setText(tr("Remove"));
    connect(ui.yes, &QLineEdit::textChanged, this, &RemoveFriendDialog::onTextChanged);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &RemoveFriendDialog::onAccepted);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &RemoveFriendDialog::close);
    setFocus();
}

void RemoveFriendDialog::onAccepted()
{
    _accepted = true;
    close();
}

void RemoveFriendDialog::onTextChanged(QString text)
{
    ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(text == ui.yes->placeholderText());
}

