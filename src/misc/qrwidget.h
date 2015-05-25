#ifndef QRWIDGET_H
#define QRWIDGET_H

// https://stackoverflow.com/questions/21400254/how-to-draw-a-qr-code-with-qt-in-native-c-c

#include <QWidget>

class QRWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QRWidget(QWidget *parent = 0);
    ~QRWidget();
    void setQRData(const QString& data);
    QImage* getImage();
    bool saveImage(QString path);

private:
    QString data;
    void paintImage();
    QImage* image;
    QSize size;
};

#endif // QRWIDGET_H
