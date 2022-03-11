/*
    Copyright © 2014-2019 by The qTox Project Contributors

    This file is part of qTox, a Qt-based graphical interface for Tox.

    qTox is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qTox is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qTox.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "setpassworddialog.h"
#include "ui_setpassworddialog.h"

#include <QApplication>
#include <QPushButton>

const double SetPasswordDialog::reasonablePasswordLength = 8.;

SetPasswordDialog::SetPasswordDialog(QString body_, QString extraButton, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SetPasswordDialog)
    , body(body_ + "\n\n")
{
    ui->setupUi(this);

    connect(ui->passwordlineEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEdit()));
    connect(ui->repasswordlineEdit, SIGNAL(textChanged(QString)), this, SLOT(onPasswordEdit()));

    ui->body->setText(body_ + "\n\n");
    QPushButton* ok = ui->buttonBox->button(QDialogButtonBox::Ok);
    ok->setEnabled(false);
    ok->setText(QApplication::tr("Ok"));
    QPushButton* cancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
    cancel->setText(QApplication::tr("Cancel"));

    if (!extraButton.isEmpty()) {
        QPushButton* third = new QPushButton(extraButton);
        ui->buttonBox->addButton(third, QDialogButtonBox::YesRole);
        connect(third, &QPushButton::clicked, this, [&]() { done(Tertiary); });
    }
}

SetPasswordDialog::~SetPasswordDialog()
{
    delete ui;
}

void SetPasswordDialog::onPasswordEdit()
{
    QString pswd = ui->passwordlineEdit->text();

    if (pswd.isEmpty()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body);
    } else if (pswd.length() < 6) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body + tr("The password is too short."));
    } else if (pswd != ui->repasswordlineEdit->text()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body + tr("The password doesn't match."));
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->body->setText(body);
    }

    ui->passStrengthMeter->setValue(getPasswordStrength(pswd));
}

int SetPasswordDialog::getPasswordStrength(QString pass)
{
    if (pass.size() < 6)
        return 0;

    double fscore = 0;
    QHash<QChar, int> charCounts;
    for (QChar c : pass) {
        charCounts[c]++;
        fscore += 5. / charCounts[c];
    }

    int variations = -1;
    variations += pass.contains(QRegExp("[0-9]", Qt::CaseSensitive, QRegExp::RegExp)) ? 1 : 0;
    variations += pass.contains(QRegExp("[a-z]", Qt::CaseSensitive, QRegExp::RegExp)) ? 1 : 0;
    variations += pass.contains(QRegExp("[A-Z]", Qt::CaseSensitive, QRegExp::RegExp)) ? 1 : 0;
    variations += pass.contains(QRegExp("[\\W]", Qt::CaseSensitive, QRegExp::RegExp)) ? 1 : 0;

    int score = fscore;
    score += variations * 10;
    score -= 20;
    score = std::min(score, 100);
    score = std::max(score, 0);

    return score;
}

QString SetPasswordDialog::getPassword()
{
    return ui->passwordlineEdit->text();
}
