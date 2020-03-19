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

#include "emoticonswidget.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/style.h"

#include <QFile>
#include <QGridLayout>
#include <QLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>

#include <math.h>

EmoticonsWidget::EmoticonsWidget(QWidget* parent)
    : QMenu(parent)
{
    setStyleSheet(Style::getStylesheet("emoticonWidget/emoticonWidget.css"));
    setLayout(&layout);
    layout.addWidget(&stack);

    QWidget* pageButtonsContainer = new QWidget;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    pageButtonsContainer->setLayout(buttonLayout);

    layout.addWidget(pageButtonsContainer);

    const int maxCols = 8;
    const int maxRows = 8;
    const int itemsPerPage = maxRows * maxCols;

    const QList<QStringList>& emoticons = SmileyPack::getInstance().getEmoticons();
    int itemCount = emoticons.size();
    int pageCount = ceil(float(itemCount) / float(itemsPerPage));
    int currPage = 0;
    int currItem = 0;
    int row = 0;
    int col = 0;

    // respect configured emoticon size
    const int px = Settings::getInstance().getEmojiFontPointSize();
    const QSize size(px, px);

    // create pages
    buttonLayout->addStretch();
    for (int i = 0; i < pageCount; ++i) {
        QGridLayout* pageLayout = new QGridLayout;
        pageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
                            maxRows, 0);
        pageLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0,
                            maxCols);

        QWidget* page = new QWidget;
        page->setLayout(pageLayout);
        stack.addWidget(page);

        // page buttons are only needed if there is more than 1 page
        if (pageCount > 1) {
            QRadioButton* pageButton = new QRadioButton;
            pageButton->setProperty("pageIndex", i);
            pageButton->setCursor(Qt::PointingHandCursor);
            pageButton->setChecked(i == 0);
            buttonLayout->addWidget(pageButton);

            connect(pageButton, &QRadioButton::clicked, this, &EmoticonsWidget::onPageButtonClicked);
        }
    }
    buttonLayout->addStretch();

    SmileyPack& smileyPack = SmileyPack::getInstance();
    for (const QStringList& set : emoticons) {
        QPushButton* button = new QPushButton;
        std::shared_ptr<QIcon> icon = smileyPack.getAsIcon(set[0]);
        emoticonsIcons.append(icon);
        button->setIcon(icon->pixmap(size));
        button->setToolTip(set.join(" "));
        button->setProperty("sequence", set[0]);
        button->setCursor(Qt::PointingHandCursor);
        button->setFlat(true);
        button->setIconSize(size);
        button->setFixedSize(size);

        connect(button, &QPushButton::clicked, this, &EmoticonsWidget::onSmileyClicked);

        qobject_cast<QGridLayout*>(stack.widget(currPage)->layout())->addWidget(button, row, col);

        ++col;
        ++currItem;

        // next row
        if (col >= maxCols) {
            col = 0;
            ++row;
        }

        // next page
        if (currItem >= itemsPerPage) {
            row = 0;
            currItem = 0;
            ++currPage;
        }
    }

    // calculates sizeHint
    layout.activate();
}

void EmoticonsWidget::onSmileyClicked()
{
    // emit insert emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender) {
        QString sequence =
            sender->property("sequence").toString().replace("&lt;", "<").replace("&gt;", ">");
        emit insertEmoticon(sequence);
    }
}

void EmoticonsWidget::onPageButtonClicked()
{
    QWidget* sender = qobject_cast<QRadioButton*>(QObject::sender());
    if (sender) {
        int page = sender->property("pageIndex").toInt();
        stack.setCurrentIndex(page);
    }
}

QSize EmoticonsWidget::sizeHint() const
{
    return layout.sizeHint();
}

void EmoticonsWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (!rect().contains(ev->pos()))
        hide();
}

void EmoticonsWidget::mousePressEvent(QMouseEvent*)
{
}

void EmoticonsWidget::wheelEvent(QWheelEvent* e)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    const bool vertical = qAbs(e->angleDelta().y()) >= qAbs(e->angleDelta().x());
    const int delta = vertical ? e->angleDelta().y() : e->angleDelta().x();

    if (vertical) {
        if (delta < 0) {
#else
    if (e->orientation() == Qt::Vertical) {
        if (e->delta() < 0) {
#endif
            stack.setCurrentIndex(stack.currentIndex() + 1);
        } else {
            stack.setCurrentIndex(stack.currentIndex() - 1);
        }
        emit PageButtonsUpdate();
    }
}

void EmoticonsWidget::PageButtonsUpdate()
{
    QList<QRadioButton*> pageButtons = this->findChildren<QRadioButton*>(QString());
    foreach (QRadioButton* t_pageButton, pageButtons) {
        if (t_pageButton->property("pageIndex").toInt() == stack.currentIndex())
            t_pageButton->setChecked(true);
        else
            t_pageButton->setChecked(false);
    }
}

void EmoticonsWidget::keyPressEvent(QKeyEvent* e)
{
    Q_UNUSED(e)
    hide();
}
