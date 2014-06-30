#include "netcamview.h"

NetCamView::NetCamView(QWidget* parent)
    : QWidget(parent), displayLabel{new QLabel},
      mainLayout{new QHBoxLayout()}
{
    setLayout(mainLayout);
    setWindowTitle("Tox video");
    setMinimumSize(320,240);

    displayLabel->setScaledContents(true);

    mainLayout->addWidget(displayLabel);
}

void NetCamView::updateDisplay(vpx_image frame)
{
    int w = frame.w, h = frame.h;
    int bpl = frame.stride[VPX_PLANE_Y], cxbpl = frame.stride[VPX_PLANE_V];
    QImage img(w, h, QImage::Format_RGB32);

    uint8_t* yData = frame.planes[VPX_PLANE_Y];
    uint8_t* uData = frame.planes[VPX_PLANE_V];
    uint8_t* vData = frame.planes[VPX_PLANE_U];
    for (int i = 0; i< h; i++)
    {
        uint32_t* scanline = (uint32_t*)img.scanLine(i);
        for (int j=0; j < bpl; j++)
        {
            float Y = yData[i*bpl + j];
            float U = uData[i*cxbpl/2 + j/2];
            float V = vData[i*cxbpl/2 + j/2];

            uint8_t R = qMax(qMin((int)(Y + 1.402 * (V - 128)),255),0);
            uint8_t G = qMax(qMin((int)(Y - 0.344 * (U - 128) - 0.714 * (V - 128)),255),0);
            uint8_t B = qMax(qMin((int)(Y + 1.772 * (U - 128)),255),0);

            scanline[j] = (0xFF<<24) + (R<<16) + (G<<8) + B;
        }
    }

    displayLabel->setPixmap(QPixmap::fromImage(img));
}
