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
    painter.setPen(Qt::green);
    painter.drawPoint(QPoint(1, 0));
    // First column has a blue dot in the middle
    painter.setPen(Qt::blue);
    painter.drawPoint(QPoint(0, 1));
}

void TestExifTransform::testTopLeft()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::TopLeft);
    QVERIFY(image.pixelColor(1, 0) == Qt::green);
    QVERIFY(image.pixelColor(0, 1) == Qt::blue);
}

void TestExifTransform::testTopRight()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::TopRight);
    QVERIFY(image.pixelColor(1, 0) == Qt::green);
    QVERIFY(image.pixelColor(2, 1) == Qt::blue);
}

void TestExifTransform::testBottomRight()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::BottomRight);
    QVERIFY(image.pixelColor(1, 2) == Qt::green);
    QVERIFY(image.pixelColor(2, 1) == Qt::blue);
}

void TestExifTransform::testBottomLeft()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::BottomLeft);
    QVERIFY(image.pixelColor(1, 2) == Qt::green);
    QVERIFY(image.pixelColor(0, 1) == Qt::blue);
}

void TestExifTransform::testLeftTop()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::LeftTop);
    QVERIFY(image.pixelColor(0, 1) == Qt::green);
    QVERIFY(image.pixelColor(1, 0) == Qt::blue);
}

void TestExifTransform::testRightTop()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::RightTop);
    QVERIFY(image.pixelColor(2, 1) == Qt::green);
    QVERIFY(image.pixelColor(1, 0) == Qt::blue);
}

void TestExifTransform::testRightBottom()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::RightBottom);
    QVERIFY(image.pixelColor(2, 1) == Qt::green);
    QVERIFY(image.pixelColor(1, 2) == Qt::blue);
}

void TestExifTransform::testLeftBottom()
{
    auto image = ExifTransform::applyExifTransformation(inputImage, ExifTransform::ExifOrientation::LeftBottom);
    QVERIFY(image.pixelColor(0, 1) == Qt::green);
    QVERIFY(image.pixelColor(1, 2) == Qt::blue);
}

QTEST_GUILESS_MAIN(TestExifTransform)
#include "exiftransform_test.moc"
