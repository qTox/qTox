#ifndef CALLCONFIRMWIDGET_H
#define CALLCONFIRMWIDGET_H

#include <QWidget>
#include <QRect>
#include <QPolygon>
#include <QBrush>

class QPaintEvent;

class CallConfirmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CallConfirmWidget(QWidget *anchor);

signals:
    void accepted();
    void rejected();

protected:
    virtual void paintEvent(QPaintEvent* event) override;

private:
    QRect mainRect;
    QPolygon spikePoly;
    QBrush brush;

    const int rectW, rectH;
    const int spikeW, spikeH;
    const int roundedFactor;
};

#endif // CALLCONFIRMWIDGET_H
