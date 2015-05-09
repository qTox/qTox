#include "qrwidget.h"
#include <QPainter>
#include <QDebug>
#include <QBuffer>
#include <QImage>
#include <qrencode.h>

#ifdef Q_OS_WIN32
    #include <errno.h>
#else
    #include <sys/errno.h>
#endif

QRWidget::QRWidget(QWidget *parent) : QWidget(parent), data("0")
//Note: The encoding fails with empty string so I just default to something else.
//Use the setQRData() call to change this.
{
    //size of the qimage might be problematic in the future, but it works for me
    size.setWidth(480);
    size.setHeight(480);
    image = new QImage(size, QImage::Format_RGB32);
}

QRWidget::~QRWidget()
{
    delete image;
}

void QRWidget::setQRData(const QString& data)
{
    this->data = data;
    paintImage();
}

QImage* QRWidget::getImage()
{
    return image;
}

/**
 * @brief QRWidget::saveImage
 * @param path Full path to the file with extension.
 * @return indicate if saving was successful.
 */
bool QRWidget::saveImage(QString path)
{
    return image->save(path, 0, 75); //0 - image format same as file extension, 75-quality, png file is ~6.3kb
}

//http://stackoverflow.com/questions/21400254/how-to-draw-a-qr-code-with-qt-in-native-c-c
void QRWidget::paintImage()
{
    QPainter painter(image);
    //NOTE: I have hardcoded some parameters here that would make more sense as variables.
    // ECLEVEL_M is much faster recognizable by barcodescanner any any other type
    // https://fukuchi.org/works/qrencode/manual/qrencode_8h.html#a4cebc3c670efe1b8866b14c42737fc8f
    // any mode other than QR_MODE_8 or QR_MODE_KANJI results in EINVAL. First 1 is version, second is case sensitivity
    QRcode* qr = QRcode_encodeString(data.toStdString().c_str(), 1, QR_ECLEVEL_M, QR_MODE_8, 1);

    if (qr != nullptr)
    {
        QColor fg("black");
        QColor bg("white");
        painter.setBrush(bg);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, size.width(), size.height());
        painter.setBrush(fg);
        const int s = qr->width > 0 ? qr->width : 1;
        const double w = width();
        const double h = height();
        const double aspect = w / h;
        const double scale = ((aspect > 1.0) ? h : w) / s;

        for (int y = 0; y < s; y++)
        {
            const int yy = y * s;
            for (int x = 0; x < s; x++)
            {
                const int xx = yy + x;
                const unsigned char b = qr->data[xx];
                if (b & 0x01)
                {
                    const double rx1 = x * scale,
                                 ry1 = y * scale;
                    QRectF r(rx1, ry1, scale, scale);
                    painter.drawRects(&r, 1);
                }
            }
        }
        QRcode_free(qr);
        painter.save();
    }
    else
    {
        QColor error("red");
        painter.setBrush(error);
        painter.drawRect(0, 0, width(), height());
        qDebug() << "QR FAIL: " << strerror(errno);
    }

    qr = nullptr;
}
