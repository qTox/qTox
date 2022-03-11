/*
    Copyright © 2020 by The qTox Project Contributors

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

#include "src/model/exiftransform.h"

#include <QPainter>
#include <QTest>

namespace {
const auto rowColor = QColor(Qt::green).rgb();
const auto colColor = QColor(Qt::blue).rgb();

enum class Side
{
    top,
    bottom,
    left,
    right
};

QPoint getPosition(Side side)
{
    int x, y;
    switch (side)
    {
    case Side::top:
    {
        x = 1;
        y = 0;
        break;
    }
    case Side::bottom:
    {
        x = 1;
        y = 2;
        break;
    }
    case Side::left:
    {
        x = 0;
        y = 1;
        break;
    }
    case Side::right:
    {
        x = 2;
        y = 1;
        break;
    }
    }

    return {x, y};
}

QRgb getColor(const QImage& image, Side side)
{
    return image.pixel(getPosition(side));
}
} // namespace

class TestExifTransform : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void testTopLeft();
    void testTopRight();
    void testBottomRight();
    void testBottomLeft();
    void testLeftTop();
    void testRightTop();
    void testRightBottom();
    void testLeftBottom();

private:
    QImage inputImage;
};

void TestExifTransform::init()
{
    inputImage = QImage(QSize(3, 3), QImage::Format_RGB32);
    QPainter painter(&inputImage);
    painter.fillRect(QRect(0, 0, 3, 3), Qt::white);
    // First row has a green dot in the middle
    painter.setPen(rowColor);
    painter.drawPoint(getPosition(Side::top));
    // First column has a blue dot in the middle
    painter.setPen(colColor);
    painter.drawPoint(getPosition(Side::left));
}

void TestExifTransform::testTopLeft()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::TopLeft);
    QVERIFY(getColor(image, Side::top) == rowColor);
    QVERIFY(getColor(image, Side::left) == colColor);
}

void TestExifTransform::testTopRight()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::TopRight);
    QVERIFY(getColor(image, Side::top) == rowColor);
    QVERIFY(getColor(image, Side::right) == colColor);
}

void TestExifTransform::testBottomRight()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::BottomRight);
    QVERIFY(getColor(image, Side::bottom) == rowColor);
    QVERIFY(getColor(image, Side::right) == colColor);
}

void TestExifTransform::testBottomLeft()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::BottomLeft);
    QVERIFY(getColor(image, Side::bottom) == rowColor);
    QVERIFY(getColor(image, Side::left) == colColor);
}

void TestExifTransform::testLeftTop()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::LeftTop);
    QVERIFY(getColor(image, Side::left) == rowColor);
    QVERIFY(getColor(image, Side::top) == colColor);
}

void TestExifTransform::testRightTop()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::RightTop);
    QVERIFY(getColor(image, Side::right) == rowColor);
    QVERIFY(getColor(image, Side::top) == colColor);
}

void TestExifTransform::testRightBottom()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::RightBottom);
    QVERIFY(getColor(image, Side::right) == rowColor);
    QVERIFY(getColor(image, Side::bottom) == colColor);
}

void TestExifTransform::testLeftBottom()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::LeftBottom);
    QVERIFY(getColor(image, Side::left) == rowColor);
    QVERIFY(getColor(image, Side::bottom) == colColor);
}

QTEST_APPLESS_MAIN(TestExifTransform)
#include "exiftransform_test.moc"
