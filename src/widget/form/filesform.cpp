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

#include "filesform.h"
#include "src/core/toxfile.h"
#include "src/widget/contentlayout.h"
#include "src/widget/translator.h"
#include "src/widget/style.h"
#include "src/widget/widget.h"
#include "src/widget/tool/imessageboxmanager.h"
#include "src/friendlist.h"
#include "util/display.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QWindow>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

namespace {
    QRect pauseRect(const QStyleOptionViewItem& option)
    {
        float controlSize = option.rect.height() * 0.8f;
        float rectWidth = option.rect.width();
        float buttonHorizontalArea = rectWidth / 2;

        // To center the button, we find the horizontal center and subtract half
        // our width from it
        int buttonXPos = std::round(option.rect.x() + buttonHorizontalArea / 2 - controlSize / 2);
        int buttonYPos = std::round(option.rect.y() + option.rect.height() * 0.1f);
        return QRect(buttonXPos, buttonYPos, controlSize, controlSize);
    }

    QRect stopRect(const QStyleOptionViewItem& option)
    {
        float controlSize = option.rect.height() * 0.8;
        float rectWidth = option.rect.width();
        float buttonHorizontalArea = rectWidth / 2;

        // To center the button, we find the horizontal center and subtract half
        // our width from it
        int buttonXPos = std::round(option.rect.x() + buttonHorizontalArea + buttonHorizontalArea / 2 - controlSize / 2);
        int buttonYPos = std::round(option.rect.y() + option.rect.height() * 0.1f);
        return QRect(buttonXPos, buttonYPos, controlSize, controlSize);
    }

    QString fileStatusString(ToxFile file)
    {
        switch (file.status)
        {
            case ToxFile::INITIALIZING:
                return QObject::tr("Initializing");
            case ToxFile::TRANSMITTING:
                return QObject::tr("Transmitting");
            case ToxFile::FINISHED:
                return QObject::tr("Finished");
            case ToxFile::BROKEN:
                return QObject::tr("Broken");
            case ToxFile::CANCELED:
                return QObject::tr("Canceled");
            case ToxFile::PAUSED:
                if (file.pauseStatus.localPaused()) {
                    return QObject::tr("Paused");
                } else {
                    return QObject::tr("Remote paused");
                }
        }

        qWarning("Corrupt file status %d", file.status);
        return "";
    }

    bool fileTransferFailed(const ToxFile::FileStatus& status) {
        switch (status)
        {
            case ToxFile::INITIALIZING:
            case ToxFile::PAUSED:
            case ToxFile::TRANSMITTING:
            case ToxFile::FINISHED:
                return false;
            case ToxFile::BROKEN:
            case ToxFile::CANCELED:
                return true;
        }

        qWarning("Invalid file status: %d", status);
        return true;
    }

    bool shouldProcessFileKind(uint8_t inKind)
    {
        auto kind = static_cast<TOX_FILE_KIND>(inKind);

        switch (kind)
        {
            case TOX_FILE_KIND_DATA: return true;
            // Avatar sharing should be seamless, the user does not need to see
            // these in their file transfer list.
            case TOX_FILE_KIND_AVATAR: return false;
        }

        qWarning("Unexpected file kind %d", kind);
        return false;
    }

} // namespace

namespace FileTransferList
{
    Column toFileTransferListColumn(int in) {
        if (in >= 0 && in < static_cast<int>(Column::invalid)) {
            return static_cast<Column>(in);
        }

        qWarning("Invalid file transfer list column %d", in);
        return Column::invalid;
    }

    QString toQString(Column column) {
        switch (column)
        {
            case Column::fileName:
                return QObject::tr("File Name");
            case Column::contact:
                return QObject::tr("Contact");
            case Column::progress:
                return QObject::tr("Progress");
            case Column::size:
                return QObject::tr("Size");
            case Column::speed:
                return QObject::tr("Speed");
            case Column::status:
                return QObject::tr("Status");
            case Column::control:
                return QObject::tr("Control");
            case Column::invalid:
                break;
        }

        return "";
    }

    EditorAction toEditorAction(int in) {
        if (in < 0 || in >= static_cast<int>(EditorAction::invalid)) {
            qWarning("Unexpected editor action %d", in);
            return EditorAction::invalid;
        }

        return static_cast<EditorAction>(in);
    }

    Model::Model(FriendList& friendList_, QObject* parent)
        : QAbstractTableModel(parent)
        , friendList{friendList_}
    {}

    QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }

        if (orientation != Qt::Orientation::Horizontal) {
            return QVariant();
        }

        const auto column = toFileTransferListColumn(section);
        return toQString(column);
    }

    void Model::onFileUpdated(const ToxFile& file)
    {
        if (!shouldProcessFileKind(file.fileKind)) {
            return;
        }

        auto idxIt = idToRow.find(file.resumeFileId);
        int rowIdx = 0;

        if (idxIt == idToRow.end()) {
            if (files.size() >= std::numeric_limits<int>::max()) {
                // Bug waiting to happen, but also what can we do if qt just doesn't
                // support this many items in a list
                qWarning("Too many file transfers rendered, ignoring");
                return;
            }

            auto insertedIdx = files.size();

            emit rowsAboutToBeInserted(QModelIndex(), insertedIdx, insertedIdx, {});

            files.push_back(file);
            idToRow.insert(file.resumeFileId, insertedIdx);

            emit rowsInserted(QModelIndex(), insertedIdx, insertedIdx, {});
        } else {
            rowIdx = idxIt.value();
            files[rowIdx] = file;
            if (fileTransferFailed(file.status)) {
                emit rowsAboutToBeRemoved(QModelIndex(), rowIdx, rowIdx, {});

                for (auto it = idToRow.begin(); it != idToRow.end(); ++it) {
                    if (it.value() > rowIdx) {
                        it.value() -= 1;
                    }
                }
                idToRow.remove(file.resumeFileId);
                files.erase(files.begin() + rowIdx);

                emit rowsRemoved(QModelIndex(), rowIdx, rowIdx, {});
            }
            else {
                emit dataChanged(index(rowIdx, 0), index(rowIdx, columnCount()));
            }
        }

    }

    int Model::rowCount(const QModelIndex& parent) const
    {
        std::ignore = parent;
        return files.size();
    }

    int Model::columnCount(const QModelIndex& parent) const
    {
        std::ignore = parent;
        return static_cast<int>(Column::invalid);
    }

    QVariant Model::data(const QModelIndex& index, int role) const
    {
        const auto row = index.row();
        if (row < 0 || static_cast<size_t>(row) > files.size()) {
            qWarning("Invalid file transfer row %d (files: %zu)", row, files.size());
            return QVariant();
        }

        if (role == Qt::UserRole) {
            return files[row].filePath;
        }

        if (role != Qt::DisplayRole) {
            return QVariant();
        }

        const auto column = toFileTransferListColumn(index.column());

        switch (column)
        {
            case Column::fileName:
                return files[row].fileName;
            case Column::contact:
            {
                auto f = friendList.findFriend(friendList.id2Key(files[row].friendId));
                if (f == nullptr) {
                    qWarning("Invalid friend for file transfer");
                    return "Unknown";
                }

                return f->getDisplayedName();
            }
            case Column::progress:
                return files[row].progress.getProgress() * 100.0;
            case Column::size:
                return getHumanReadableSize(files[row].progress.getFileSize());
            case Column::speed:
                return getHumanReadableSize(files[row].progress.getSpeed()) + "/s";
            case Column::status:
                return fileStatusString(files[row]);
            case Column::control:
                return files[row].pauseStatus.localPaused();
            case Column::invalid:
                break;
        }

        return QVariant();
    }

    bool Model::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        std::ignore = role;
        const auto column = toFileTransferListColumn(index.column());

        if (column != Column::control) {
            return false;
        }

        if (!value.canConvert<int>()) {
            qWarning("Unexpected model data");
            return false;
        }

        const auto action = toEditorAction(value.toInt());

        switch (action)
        {
            case EditorAction::cancel:
                emit cancel(files[index.row()]);
                break;
            case EditorAction::pause:
                emit togglePause(files[index.row()]);
                break;
            case EditorAction::invalid:
                return false;
        }

        return true;
    }

    Delegate::Delegate(Settings& settings_, Style& style_, QWidget* parent)
        : QStyledItemDelegate(parent)
        , settings{settings_}
        , style{style_}
    {}

    void Delegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        const auto column = toFileTransferListColumn(index.column());
        switch (column)
        {
            case Column::progress:
            {
                int progress = index.data().toInt();

                QStyleOptionProgressBar progressBarOption;
                progressBarOption.rect = option.rect;
                progressBarOption.minimum = 0;
                progressBarOption.maximum = 100;
                progressBarOption.progress = progress;
                progressBarOption.text = QString::number(progress) + "%";
                progressBarOption.textVisible = true;

                QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                                &progressBarOption, painter);
                return;
            }
            case Column::control:
            {
                const auto data = index.data();
                if (!data.canConvert<bool>()) {
                    qWarning("Unexpected control type, not rendering controls");
                    return;
                }
                const auto localPaused = data.toBool();
                QPixmap pausePixmap = localPaused
                    ? QPixmap(style.getImagePath("fileTransferInstance/arrow_black.svg", settings))
                    : QPixmap(style.getImagePath("fileTransferInstance/pause_dark.svg", settings));
                QApplication::style()->drawItemPixmap(painter, pauseRect(option), Qt::AlignCenter, pausePixmap);

                QPixmap stopPixmap(style.getImagePath("fileTransferInstance/no_dark.svg", settings));
                QApplication::style()->drawItemPixmap(painter, stopRect(option), Qt::AlignCenter, stopPixmap);
                return;
            }
            case Column::fileName:
            case Column::contact:
            case Column::size:
            case Column::speed:
            case Column::status:
            case Column::invalid:
                break;
        }

        QStyledItemDelegate::paint(painter, option, index);
    }

    bool Delegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                            const QStyleOptionViewItem& option, const QModelIndex& index)
    {
        if (toFileTransferListColumn(index.column()) == Column::control)
        {
            if (event->type() == QEvent::MouseButtonPress) {
                auto mouseEvent = reinterpret_cast<QMouseEvent*>(event);
                const auto pos = mouseEvent->pos();
                const auto posRect = pauseRect(option);
                const auto stRect = stopRect(option);

                if (posRect.contains(pos)) {
                    model->setData(index, static_cast<int>(EditorAction::pause));
                } else if (stRect.contains(pos)) {
                    model->setData(index, static_cast<int>(EditorAction::cancel));
                }
            }
            return true;
        }
        return false;
    }


    View::View(QAbstractItemModel* model, Settings& settings, Style& style,
        QWidget* parent)
        : QTableView(parent)
    {
        setModel(model);

        // Resize to contents but stretch the file name to fill the full view
        horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        // Visually tuned until it looked ok
        horizontalHeader()->setMinimumSectionSize(75);
        horizontalHeader()->setStretchLastSection(false);
        verticalHeader()->hide();
        setShowGrid(false);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setItemDelegate(new Delegate(settings, style, this));
    }

    View::~View() = default;

} // namespace FileTransferList

FilesForm::FilesForm(CoreFile& coreFile, Settings& settings, Style& style,
    IMessageBoxManager& messageBoxManager_, FriendList& friendList)
    : QObject()
    , messageBoxManager{messageBoxManager_}
{
    head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);
    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    recvdModel = new FileTransferList::Model(friendList, this);
    sentModel = new FileTransferList::Model(friendList, this);

    auto pauseFile = [&coreFile] (ToxFile file) {
        coreFile.pauseResumeFile(file.friendId, file.fileNum);
    };

    auto cancelFileRecv = [&coreFile] (ToxFile file) {
        coreFile.cancelFileRecv(file.friendId, file.fileNum);
    };

    auto cancelFileSend = [&coreFile] (ToxFile file) {
        coreFile.cancelFileSend(file.friendId, file.fileNum);
    };

    connect(recvdModel, &FileTransferList::Model::togglePause, pauseFile);
    connect(recvdModel, &FileTransferList::Model::cancel, cancelFileRecv);
    connect(sentModel, &FileTransferList::Model::togglePause, pauseFile);
    connect(sentModel, &FileTransferList::Model::cancel, cancelFileSend);

    recvd = new FileTransferList::View(recvdModel, settings, style);
    sent = new FileTransferList::View(sentModel, settings, style);

    main.addTab(recvd, QString());
    main.addTab(sent, QString());

    connect(sent, &QTableView::activated, this, &FilesForm::onSentFileActivated);
    connect(recvd, &QTableView::activated, this, &FilesForm::onReceivedFileActivated);

    retranslateUi();
    Translator::registerHandler(std::bind(&FilesForm::retranslateUi, this), this);
}

FilesForm::~FilesForm()
{
    Translator::unregister(this);
    delete recvd;
    delete sent;
    head->deleteLater();
}

bool FilesForm::isShown() const
{
    if (main.isVisible()) {
        head->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void FilesForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(&main);
    contentLayout->mainHead->layout()->addWidget(head);
    main.show();
    head->show();
}

void FilesForm::onFileUpdated(const ToxFile& inFile)
{
    if (!shouldProcessFileKind(inFile.fileKind)) {
        return;
    }

    if (inFile.direction == ToxFile::SENDING) {
        sentModel->onFileUpdated(inFile);
    }
    else if (inFile.direction == ToxFile::RECEIVING) {
        recvdModel->onFileUpdated(inFile);
    }
    else {
        qWarning("Unexpected file direction");
    }
}

void FilesForm::onSentFileActivated(const QModelIndex& index)
{
    const auto& filePath = sentModel->data(index, Qt::UserRole).toString();
    messageBoxManager.confirmExecutableOpen(filePath);
}

void FilesForm::onReceivedFileActivated(const QModelIndex& index)
{
    const auto& filePath = recvdModel->data(index, Qt::UserRole).toString();
    messageBoxManager.confirmExecutableOpen(filePath);
}

void FilesForm::retranslateUi()
{
    headLabel.setText(tr("Transferred files", "\"Headline\" of the window"));
    main.setTabText(0, tr("Downloads"));
    main.setTabText(1, tr("Uploads"));
}
