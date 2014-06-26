#include "filetransfertwidget.h"
#include "widget.h"
#include "core.h"
#include <QFileDialog>
#include <QPixmap>

FileTransfertWidget::FileTransfertWidget(ToxFile File)
    : lastUpdate{QDateTime::currentDateTime()}, lastBytesSent{0},
      fileNum{File.fileNum}, friendId{File.friendId}, direction{File.direction}
{
    pic=new QLabel(), filename=new QLabel(), size=new QLabel(), speed=new QLabel(), eta=new QLabel();
    topright = new QPushButton(), bottomright = new QPushButton();
    progress = new QProgressBar();
    mainLayout = new QHBoxLayout(), textLayout = new QHBoxLayout();
    infoLayout = new QVBoxLayout(), buttonLayout = new QVBoxLayout();
    QFont prettysmall;
    prettysmall.setPixelSize(10);
    QPalette greybg;
    greybg.setColor(QPalette::Window, QColor(209,209,209));
    greybg.setColor(QPalette::Base, QColor(150,150,150));
    setPalette(greybg);
    setAutoFillBackground(true);

    setMinimumSize(250,50);
    setLayout(mainLayout);
    mainLayout->setMargin(0);

    pic->setMaximumHeight(40);
    pic->setContentsMargins(5,0,0,0);
    filename->setText(File.fileName);
    filename->setFont(prettysmall);
    size->setText(getHumanReadableSize(File.filesize));
    size->setFont(prettysmall);
    speed->setText("0B/s");
    speed->setFont(prettysmall);
    eta->setText("00:00");
    eta->setFont(prettysmall);
    progress->setValue(0);
    progress->setMinimumHeight(11);
    progress->setFont(prettysmall);

    topright->setIcon(QIcon("img/button icons/no_2x.png"));
    if (File.direction == ToxFile::SENDING)
    {
        bottomright->setIcon(QIcon("img/button icons/pause_2x.png"));
        connect(topright, SIGNAL(clicked()), this, SLOT(cancelTransfer()));
        connect(bottomright, SIGNAL(clicked()), this, SLOT(pauseResumeSend()));

        QPixmap preview;
        if (preview.loadFromData(File.fileData))
        {
            preview = preview.scaledToHeight(40);
            pic->setPixmap(preview);
        }
    }
    else
    {
        bottomright->setIcon(QIcon("img/button icons/yes_2x.png"));
        connect(topright, SIGNAL(clicked()), this, SLOT(rejectRecvRequest()));
        connect(bottomright, SIGNAL(clicked()), this, SLOT(acceptRecvRequest()));
    }

    QPalette toxgreen;
    toxgreen.setColor(QPalette::Button, QColor(107,194,96)); // Tox Green
    topright->setIconSize(QSize(10,10));
    topright->setFixedSize(25,25);
    topright->setFlat(true);
    topright->setAutoFillBackground(true);
    topright->setPalette(toxgreen);
    bottomright->setIconSize(QSize(10,10));
    bottomright->setFixedSize(25,25);
    bottomright->setFlat(true);
    bottomright->setAutoFillBackground(true);
    bottomright->setPalette(toxgreen);

    mainLayout->addWidget(pic);
    mainLayout->addLayout(infoLayout);
    mainLayout->addLayout(buttonLayout);

    infoLayout->addWidget(filename);
    infoLayout->addLayout(textLayout);
    infoLayout->addWidget(progress);
    infoLayout->setMargin(5);

    textLayout->addWidget(size);
    textLayout->addWidget(speed);
    textLayout->addWidget(eta);
    textLayout->setMargin(0);
    textLayout->setSpacing(5);

    buttonLayout->addWidget(topright);
    buttonLayout->addSpacing(2);
    buttonLayout->addWidget(bottomright);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
}

QString FileTransfertWidget::getHumanReadableSize(int size)
{
    static const char* suffix[] = {"B","kiB","MiB","GiB","TiB"};
    int exp = 0;
    if (size)
        exp = std::min( (int) (log(size) / log(1024)), (int) (sizeof(suffix) / sizeof(suffix[0]) - 1));
    return QString().setNum(size / pow(1024, exp),'g',3).append(suffix[exp]);
}

void FileTransfertWidget::onFileTransferInfo(int FriendId, int FileNum, int Filesize, int BytesSent, ToxFile::FileDirection Direction)
{
    if (FileNum != fileNum || FriendId != friendId || Direction != direction)
            return;
    QDateTime newtime = QDateTime::currentDateTime();
    int timediff = lastUpdate.secsTo(newtime);
    if (timediff <= 0)
        return;
    int diff = BytesSent - lastBytesSent;
    if (diff < 0)
        diff = 0;
    int rawspeed = diff / timediff;
    speed->setText(getHumanReadableSize(rawspeed)+"/s");
    size->setText(getHumanReadableSize(Filesize));
    if (!rawspeed)
        return;
    int etaSecs = (Filesize - BytesSent) / rawspeed;
    QTime etaTime(0,0);
    etaTime = etaTime.addSecs(etaSecs);
    eta->setText(etaTime.toString("mm:ss"));
    if (!Filesize)
        progress->setValue(0);
    else
        progress->setValue(BytesSent*100/Filesize);
    lastUpdate = newtime;
    lastBytesSent = BytesSent;
}

void FileTransfertWidget::onFileTransferCancelled(int FriendId, int FileNum, ToxFile::FileDirection Direction)
{
    if (FileNum != fileNum || FriendId != friendId || Direction != direction)
            return;
    disconnect(topright);
    disconnect(Widget::getInstance()->getCore(),0,this,0);
    progress->hide();
    speed->hide();
    eta->hide();
    topright->hide();
    bottomright->hide();
    QPalette toxred;
    toxred.setColor(QPalette::Window, QColor(200,78,78)); // Tox Red
    setPalette(toxred);
}

void FileTransfertWidget::onFileTransferFinished(ToxFile File)
{
    if (File.fileNum != fileNum || File.friendId != friendId || File.direction != direction)
            return;
    topright->disconnect();
    disconnect(Widget::getInstance()->getCore(),0,this,0);
    progress->hide();
    speed->hide();
    eta->hide();
    topright->setIcon(QIcon("img/button icons/yes_2x.png"));
    buttonLayout->addStretch();
    buttonLayout->setSpacing(0);
    bottomright->hide();
    QPalette toxgreen;
    toxgreen.setColor(QPalette::Window, QColor(107,194,96)); // Tox Green
    setPalette(toxgreen);

    if (File.direction == ToxFile::RECEIVING)
    {
        QFile saveFile(savePath);
        if (!saveFile.open(QIODevice::WriteOnly))
            return;
        saveFile.write(File.fileData);
        saveFile.close();

        QPixmap preview;
        if (preview.loadFromData(File.fileData))
        {
            preview = preview.scaledToHeight(40);
            pic->setPixmap(preview);
        }
    }
}

void FileTransfertWidget::cancelTransfer()
{
    Widget::getInstance()->getCore()->cancelFileSend(friendId, fileNum);
}

void FileTransfertWidget::rejectRecvRequest()
{
    Widget::getInstance()->getCore()->rejectFileRecvRequest(friendId, fileNum);
    onFileTransferCancelled(friendId, fileNum, direction);
}

void FileTransfertWidget::acceptRecvRequest()
{
    QString path = QFileDialog::getSaveFileName(0,"Save a file",QDir::currentPath()+'/'+filename->text());
    if (path.isEmpty())
        return;

    savePath = path;

    bottomright->setIcon(QIcon("img/button icons/pause_2x.png"));
    bottomright->disconnect();
    connect(bottomright, SIGNAL(clicked()), this, SLOT(pauseResumeRecv()));
    Widget::getInstance()->getCore()->acceptFileRecvRequest(friendId, fileNum);
}

void FileTransfertWidget::pauseResumeRecv()
{
    Widget::getInstance()->getCore()->pauseResumeFileRecv(friendId, fileNum);
}

void FileTransfertWidget::pauseResumeSend()
{
    Widget::getInstance()->getCore()->pauseResumeFileSend(friendId, fileNum);
}
