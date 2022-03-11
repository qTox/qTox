/*
    Copyright © 2017-2019 by The qTox Project Contributors

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

#include "src/widget/splitterrestorer.h"

#include <QSplitter>

/**
 * @class SplitterRestorer
 * @brief Restore splitter from saved state and reset to default
 */

namespace {
/**
 * @brief The width of the default splitter handles.
 * By default, this property contains a value that depends on the user's
 * platform and style preferences.
 */
int defaultWidth = 0;

/**
 * @brief Width of left splitter size in percents.
 */
const int leftWidthPercent = 33;
} // namespace

SplitterRestorer::SplitterRestorer(QSplitter* splitter_)
    : splitter{splitter_}
{
    if (defaultWidth == 0) {
        defaultWidth = QSplitter().handleWidth();
    }
}

/**
 * @brief Restore splitter from state. And reset in case of error.
 * Set the splitter to a reasonnable width by default and on first start
 * @param state State saved by QSplitter::saveState()
 * @param windowSize Widnow size (used to calculate splitter size)
 */
void SplitterRestorer::restore(const QByteArray& state, const QSize& windowSize)
{
    bool brokenSplitter = !splitter->restoreState(state) || splitter->orientation() != Qt::Horizontal
                          || splitter->handleWidth() > defaultWidth;

    if (splitter->count() == 2 && brokenSplitter) {
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(defaultWidth);
        splitter->resize(windowSize);
        QList<int> sizes = splitter->sizes();
        sizes[0] = splitter->width() * leftWidthPercent / 100;
        sizes[1] = splitter->width() - sizes[0];
        splitter->setSizes(sizes);
    }
}
