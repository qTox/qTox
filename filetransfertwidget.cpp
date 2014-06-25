#include "filetransfertwidget.h"

FileTransfertWidget::FileTransfertWidget(ToxFile *File)
    : file{File}, lastUpdate{QDateTime::currentDateTime()}, lastBytesSent{0}
{
    pic=new QLabel(), filename=new QLabel(), size=new QLabel(), speed=new QLabel(), eta=new QLabel();
    topright = new QPushButton(), bottomright = new QPushButton();
    progress = new QProgressBar();
    mainLayout = new QHBoxLayout(), textLayout = new QHBoxLayout();
    infoLayout = new QVBoxLayout(), buttonLayout = new QVBoxLayout();
    QFont prettysmall;
    prettysmall.setPixelSize(10);
    QPalette greybg;
    greybg.setColor(QPalette::Window, Qt::gray);
    setPalette(greybg);
    setAutoFillBackground(true);

    setFixedSize(250,50);
    setLayout(mainLayout);

    filename->setText(file->fileName);
    filename->setFont(prettysmall);
    size->setText(getHumanReadableSize(file->fileData.size()));
    size->setFont(prettysmall);
    speed->setText("0B/s");
    speed->setFont(prettysmall);
    eta->setText("00:00");
    eta->setFont(prettysmall);
    progress->setValue(0);

    topright->setIcon(QIcon("img/button icons/no_2x.png"));
    if (file->direction == ToxFile::SENDING)
        bottomright->setIcon(QIcon("img/button icons/pause_2x.png"));
    else
        bottomright->setIcon(QIcon("img/button icons/yes_2x.png"));

    topright->setIconSize(QSize(25,25));
    bottomright->setIconSize(QSize(25,25));

    mainLayout->addWidget(pic);
    mainLayout->addLayout(infoLayout);
    mainLayout->addLayout(buttonLayout);

    infoLayout->addWidget(filename);
    infoLayout->addLayout(textLayout);
    infoLayout->addWidget(progress);

    textLayout->addWidget(size);
    textLayout->addWidget(speed);
    textLayout->addWidget(eta);
    textLayout->setMargin(0);
    textLayout->setSpacing(5);
}

QString FileTransfertWidget::getHumanReadableSize(int size)
{
    static const char* suffix[] = {"B","kB","MB","GB","TB"};
    int exp = 0;
    if (size)
        exp = std::min( (int) (log(size) / log(1000)), (int) (sizeof(suffix) / sizeof(suffix[0]) - 1));
    return QString().setNum(size / pow(1000, exp),'g',3).append(suffix[exp]);
}

void FileTransfertWidget::onFileTransferInfo(ToxFile* File)
{
    if (File != file)
            return;
    QDateTime newtime = QDateTime::currentDateTime();
    int timediff = lastUpdate.secsTo(newtime);
    if (!timediff)
        return;
    int diff = File->bytesSent - lastBytesSent;
    int rawspeed = diff / timediff;
    speed->setText(getHumanReadableSize(rawspeed)+"/s");
    size->setText(getHumanReadableSize(File->fileData.size()));
    if (!rawspeed)
        return;
    int etaSecs = (File->fileData.size() - File->bytesSent) / rawspeed;
    QTime etaTime(0,0);
    etaTime = etaTime.addSecs(etaSecs);
    eta->setText(etaTime.toString("mm:ss"));
    progress->setValue(File->bytesSent*100/File->fileData.size());
    lastUpdate = newtime;
    lastBytesSent = File->bytesSent;
}

void FileTransfertWidget::onFileTransferCancelled(ToxFile* File)
{

}

void FileTransfertWidget::onFileTransferFinished(ToxFile* File)
{

}
