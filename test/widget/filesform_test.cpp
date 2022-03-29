/*
    Copyright © 2021 by The qTox Project Contributors

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

#include "src/widget/form/filesform.h"
#include "src/friendlist.h"
#include "src/model/friend.h"

#include <QTest>
#include <limits>

class TestFileTransferList : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void testFileTransferListConversion();
    void testEditorActionConversion();

    void testFileName();
    // NOTE: Testing contact return requires a lookup in FriendList which goes
    // down a large dependency chain that is not linked to this test
    // void testContact();
    void testProgress();
    void testSize();
    void testSpeed();
    void testStatus();
    void testControl();
    void testAvatarIgnored();
    void testMultipleFiles();
    void testFileRemoval();
private:
    std::unique_ptr<FileTransferList::Model> model;
    std::unique_ptr<FriendList> friendList;
};

using namespace FileTransferList;

void TestFileTransferList::init()
{
    friendList = std::unique_ptr<FriendList>(new FriendList());
    model = std::unique_ptr<Model>(new Model(*friendList));
}

void TestFileTransferList::testFileTransferListConversion()
{
    for (int i = 0; i < model->columnCount(); ++i) {
        QVERIFY(toFileTransferListColumn(i) != Column::invalid);
    }
    QCOMPARE(toFileTransferListColumn(100), Column::invalid);
}

void TestFileTransferList::testEditorActionConversion()
{
    QCOMPARE(toEditorAction(static_cast<int>(EditorAction::pause)), EditorAction::pause);
    QCOMPARE(toEditorAction(static_cast<int>(EditorAction::cancel)), EditorAction::cancel);
    QCOMPARE(toEditorAction(55), EditorAction::invalid);
}

void TestFileTransferList::testFileName()
{
    ToxFile file;
    file.fileKind = TOX_FILE_KIND_DATA;
    file.fileName = "Test";
    model->onFileUpdated(file);

    const auto idx = model->index(0, static_cast<int>(Column::fileName));
    const auto fileName = idx.data();

    QCOMPARE(fileName.toString(), QString("Test"));
}

void TestFileTransferList::testProgress()
{
    ToxFile file(0, 0, "", "", 1000, ToxFile::FileDirection::SENDING);
    file.progress.addSample(100, QTime(1, 0, 0));
    model->onFileUpdated(file);

    const auto idx = model->index(0, static_cast<int>(Column::progress));
    const auto progress = idx.data();

    // Progress should be in percent units, 100/1000 == 10
    QCOMPARE(progress.toFloat(), 10.0f);
}

void TestFileTransferList::testSize()
{
    ToxFile file(0, 0, "", "", 1000, ToxFile::FileDirection::SENDING);
    model->onFileUpdated(file);

    const auto idx = model->index(0, static_cast<int>(Column::size));
    auto size = idx.data();

    // Size should be a human readable string
    QCOMPARE(size.toString(), QString("1000B"));

    // 1GB + a little to avoid floating point inaccuracy
    file = ToxFile(0, 0, "", "", 1024 * 1024 * 1024 + 2, ToxFile::FileDirection::SENDING);
    model->onFileUpdated(file);
    size = idx.data();
    QCOMPARE(size.toString(), QString("1.00GiB"));
}

void TestFileTransferList::testSpeed()
{
    ToxFile file(0, 0, "", "", 1024 * 1024, ToxFile::FileDirection::SENDING);
    file.progress.addSample(100 * 1024, QTime(1, 0, 0));
    file.progress.addSample(200 * 1024, QTime(1, 0, 1));
    model->onFileUpdated(file);

    const auto idx = model->index(0, static_cast<int>(Column::speed));
    const auto speed = idx.data();

    // Speed should be a human readable string
    QCOMPARE(speed.toString(), QString("100KiB/s"));
}

void TestFileTransferList::testStatus()
{
    ToxFile file(0, 0, "", "", 1024 * 1024, ToxFile::FileDirection::SENDING);
    file.status = ToxFile::TRANSMITTING;
    model->onFileUpdated(file);

    const auto idx = model->index(0, static_cast<int>(Column::status));
    auto status = idx.data();

    QCOMPARE(status.toString(), QString("Transmitting"));

    file.status = ToxFile::PAUSED;
    file.pauseStatus.remotePause();
    model->onFileUpdated(file);
    status = idx.data();

    QCOMPARE(status.toString(), QString("Remote paused"));

    file.status = ToxFile::PAUSED;
    file.pauseStatus.localPause();
    file.pauseStatus.remoteResume();
    model->onFileUpdated(file);
    status = idx.data();

    QCOMPARE(status.toString(), QString("Paused"));
}

void TestFileTransferList::testControl()
{
    bool cancelCalled = false;
    bool pauseCalled = false;

    QObject::connect(model.get(), &Model::cancel, [&] (ToxFile file) {
        std::ignore = file;
        cancelCalled = true;
    });

    QObject::connect(model.get(), &Model::togglePause, [&] (ToxFile file) {
        std::ignore = file;
        pauseCalled = true;
    });

    ToxFile file(0, 0, "", "", 1024 * 1024, ToxFile::FileDirection::SENDING);
    file.status = ToxFile::TRANSMITTING;
    model->onFileUpdated(file);

    const auto idx = model->index(0, static_cast<int>(Column::control));
    model->setData(idx, static_cast<int>(EditorAction::pause));

    QVERIFY(pauseCalled);
    QVERIFY(!cancelCalled);

    pauseCalled = false;
    model->setData(idx, static_cast<int>(EditorAction::cancel));
    QVERIFY(!pauseCalled);
    QVERIFY(cancelCalled);

    file.status = ToxFile::TRANSMITTING;
    model->onFileUpdated(file);
    // True if paused
    QCOMPARE(idx.data().toBool(), false);

    file.status = ToxFile::PAUSED;
    file.pauseStatus.localPause();
    model->onFileUpdated(file);
    // True if _local_ paused
    QCOMPARE(idx.data().toBool(), true);
}

void TestFileTransferList::testAvatarIgnored()
{
    ToxFile file;
    file.fileKind = TOX_FILE_KIND_AVATAR;
    model->onFileUpdated(file);

    QCOMPARE(model->rowCount(), 0);
}

void TestFileTransferList::testMultipleFiles()
{
    ToxFile file;
    file.resumeFileId = QByteArray();
    file.fileKind = TOX_FILE_KIND_DATA;
    file.status = ToxFile::TRANSMITTING;
    file.fileName = "a";
    model->onFileUpdated(file);

    // File map keys off resume file ID
    file.resumeFileId = QByteArray("asdfasdf");
    file.fileName = "b";
    model->onFileUpdated(file);

    QCOMPARE(model->rowCount(), 2);

    auto idx = model->index(0, static_cast<int>(Column::fileName));
    QCOMPARE(idx.data().toString(), QString("a"));

    idx = model->index(1, static_cast<int>(Column::fileName));
    QCOMPARE(idx.data().toString(), QString("b"));

    // File name should be updated instead of inserting a new file since the
    // resume file ID is the same
    file.fileName = "c";
    model->onFileUpdated(file);
    QCOMPARE(model->rowCount(), 2);
    QCOMPARE(idx.data().toString(), QString("c"));
}

void TestFileTransferList::testFileRemoval()
{
    // Model should keep files in the list if they are finished, but not if they
    // were broken or canceled

    ToxFile file;
    file.fileKind = TOX_FILE_KIND_DATA;
    file.status = ToxFile::TRANSMITTING;
    model->onFileUpdated(file);

    QCOMPARE(model->rowCount(), 1);

    file.status = ToxFile::BROKEN;
    model->onFileUpdated(file);
    QCOMPARE(model->rowCount(), 0);

    file.status = ToxFile::TRANSMITTING;
    model->onFileUpdated(file);
    QCOMPARE(model->rowCount(), 1);

    file.status = ToxFile::CANCELED;
    model->onFileUpdated(file);
    QCOMPARE(model->rowCount(), 0);

    file.status = ToxFile::TRANSMITTING;
    model->onFileUpdated(file);
    QCOMPARE(model->rowCount(), 1);

    file.status = ToxFile::FINISHED;
    model->onFileUpdated(file);
    QCOMPARE(model->rowCount(), 1);
}

QTEST_GUILESS_MAIN(TestFileTransferList)
#include "filesform_test.moc"
