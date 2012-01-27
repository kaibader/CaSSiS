/*!
 * This is a linechart widget that was adapted from the Chipanalyser
 * source code.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) GUI.
 *
 * Copyright (C) 2001-2012
 *     Kai Christian Bader <mail@kaibader.de>
 *
 * CaSSiS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CaSSiS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CaSSiS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "linechart.h"

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QPixmap>
#include <QtGui/QMouseEvent>

//// Every array of values is stored in a LineData struct.
//// This way, multiple arrays can be stored and displayed in one QLineChart.
//typedef struct linedata {
//    QVector<double> x; // array of all x-values
//    QVector<double> y; // array of all y-values
//    uint count; // number of values in x and y
//    QPen pen; // QPen definition (pen color & style)
//} LineData, *LineDataPtr;

// QLineChart - constructor
LineChart::LineChart(QWidget *parentwidget) :
QWidget(parentwidget) {
    m_needs_repaint = true; // default: repaint is necessary
    m_has_data = false; // default: no data available
    m_image = NULL; // default: no pixmap available
    m_x_autostepping = true; // enable automatic scale for x axis
    m_y_autostepping = true; // enable automatic scale for y axis
    m_mouse_pressed = false; // mouse is not pressed at first

    // just let all variables be defined
    // (even if their value is nonsense, they will be filled later)
    m_left_border = 0;
    m_top_border = 0;
    m_right_border = 0;
    m_bottom_border = 0;
    m_widget_width = 0;
    m_widget_height = 0;
    m_left_border = 0;
    m_top_border = 0;
    m_right_border = 0;
    m_bottom_border = 0;
    m_coord_x_min = 0;
    m_coord_x_max = 0;
    m_coord_y_min = 0;
    m_coord_y_max = 0;
    m_coord_width = 0;
    m_coord_height = 0;
    m_factor_x = 0;
    m_factor_y = 0;
    // m_LineArray.setAutoDelete(true); // TODO: Qt4 DEPRECATED

    m_mouse_tracking = false;

    // setBackgroundMode(Qt::NoBackground); // widget has no background -- TODO
    setMouseTracking(true); // enable mouse tracking
    // setCursor(Qt::crossCursor); // cursor style = cross -- TODO
}

// method is called each time our chart needs to be repainted
void LineChart::paintEvent(QPaintEvent *) {
    // Repaint is only done if values has changed
    if (m_needs_repaint) {
        if (m_image != NULL)
            delete m_image;

        m_image = new QImage(size(), QImage::Format_ARGB32);

        QPainter image_painter(m_image); // create new empty (white) painter
        image_painter.begin(m_image);
        image_painter.fillRect(m_image->rect(), Qt::white);

        QFont smallFont("Helvetica", 8);
        image_painter.setFont(smallFont); // standard font settings

        // draw lines if data is available
        if (m_has_data) {
            drawChart(image_painter);
            drawLines(image_painter);
        }

        m_needs_repaint = false; // nearly all work done... ;-)
    }

    QPainter this_painter(this);
    this_painter.drawImage(QPoint(0, 0), *m_image);

    if (m_mouse_pressed) {
        QPen coord_pen(Qt::yellow, 1); // yellow, 1 pixel wide
        this_painter.setPen(coord_pen);
        this_painter.drawRect(m_mouse_x_min, m_mouse_y_min,
                (m_mouse_x_act - m_mouse_x_min),
                (m_mouse_y_act - m_mouse_y_min));
    }
}

// method is called every time the widget is resized
void LineChart::resizeEvent(QResizeEvent *) {
    // every resize-event initiates a repaint
    m_needs_repaint = true;
}

// method draws a coordinate system and the data lines
void LineChart::drawChart(QPainter &painter) {
    m_widget_width = this->width(); // get widget width
    m_widget_height = this->height(); // get widget height

    // these are hardcoded borders of the coordinate system
    m_left_border = 60;
    m_top_border = 20;
    m_right_border = 20;
    m_bottom_border = 30;

    // set/update the position and dimension of the coordinate system
    // (their origin lies in the upper left corner)
    m_coord_x_min = m_left_border;
    m_coord_x_max = m_widget_width - m_right_border;
    m_coord_y_min = m_top_border;
    m_coord_y_max = m_widget_height - m_bottom_border;
    m_coord_width = m_coord_x_max - m_coord_x_min;
    m_coord_height = m_coord_y_max - m_coord_y_min;

    // Activate antialiasing when drawing the background
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Create a background gradient (gray)
    QLinearGradient bg_gradient(m_coord_x_min, m_coord_y_min, m_coord_x_max,
            m_coord_y_max);
    bg_gradient.setColorAt(0, QColor(255, 255, 255));
    bg_gradient.setColorAt(1, QColor(192, 192, 192));
    painter.fillRect(
            QRectF(m_coord_x_min, m_coord_y_min, m_coord_width, m_coord_height),
            QBrush(bg_gradient));

    // a hardcoded size check
    // (minimum size for coordinate system: 10x10)
    if ((m_coord_width < 10) || (m_coord_height < 10))
        return;

    // these factors are essential for forward- & backward lookup
    // of every coordinate in our graph
    m_factor_x = (double) m_coord_width / (m_x_view_max - m_x_view_min);
    m_factor_y = (double) m_coord_height / (m_y_view_max - m_y_view_min);

    // let us check if the x- and y-steps are too small or large
    double x_stepping = m_x_stepping;
    double y_stepping = m_y_stepping;
    while ((m_factor_x * x_stepping) < 20)
        x_stepping = x_stepping * 10;
    while ((m_factor_y * y_stepping) < 20)
        y_stepping = y_stepping * 10;
    while (((m_x_view_max - m_x_view_min) / x_stepping) < 5)
        x_stepping = x_stepping / 10;
    while (((m_y_view_max - m_y_view_min) / y_stepping) < 5)
        y_stepping = y_stepping / 10;

    if (x_stepping == 0)
        return; // prevent infinite loops
    if (y_stepping == 0)
        return; // prevent infinite loops

    // first step: draw a nice coordinate system *************************
    QPen coord_pen(Qt::gray); // gray dots, 1 pixel wide
    coord_pen.setWidth(1);
    painter.setPen(coord_pen);

    // Deactivate antialiasing when drawing the coordinate system
    painter.setRenderHint(QPainter::Antialiasing, false);

    // find coord-borders using our given stepping
    // (equals: find the nearest steps below and above the
    //  given x- any y-view)
    double coord_border_x_min = (double) ((int) (m_x_view_min / x_stepping) - 1)
                    * x_stepping;
    double coord_border_x_max = (double) ((int) (m_x_view_max / x_stepping) + 1)
                    * x_stepping;
    double coord_border_y_min = (double) ((int) (m_y_view_min / y_stepping) - 1)
                    * y_stepping;
    double coord_border_y_max = (double) ((int) (m_y_view_max / y_stepping) + 1)
                    * y_stepping;

    // draw all x steps/lines in our view
    for (double my_x = coord_border_x_min; my_x <= coord_border_x_max;
            my_x = my_x + x_stepping) {
        if ((my_x >= m_x_view_min) && (my_x <= m_x_view_max)) {
            // btw: origin is in the bottom left corner
            int x_pos = (int) (m_coord_x_min
                    + ((my_x - m_x_view_min) * m_factor_x));

            QString temp;
            temp.setNum(my_x);

            painter.drawLine(x_pos, m_coord_y_min, x_pos, m_coord_y_max);

            painter.drawText(x_pos, m_coord_y_max + 10, temp);
        }
    }

    // draw all y steps/lines in our view
    for (double my_y = coord_border_y_min; my_y <= coord_border_y_max;
            my_y = my_y + y_stepping) {
        if ((my_y >= m_y_view_min) && (my_y <= m_y_view_max)) {
            // btw: origin is in the bottom left corner
            int y_pos = (int) (m_coord_y_max
                    - ((my_y - m_y_view_min) * m_factor_y));

            QString temp;
            temp.setNum(my_y);

            painter.drawLine(m_coord_x_min, y_pos, m_coord_x_max, y_pos);

            painter.drawText(m_coord_x_min - 40, y_pos, temp);
        }
    }

    painter.drawRect(m_coord_x_min, m_coord_y_min, m_coord_width,
            m_coord_height);
}

void LineChart::drawLines(QPainter &painter) {
    // Activate antialiasing for the lines...
    painter.setRenderHint(QPainter::Antialiasing, true);

    // second step: insert all line charts into our coordinate system ****
    for (int i = 0; i < m_lines.size(); i++) {
        QVector<QPointF> line = m_lines.at(i);

        for (int j = 1; j < line.size(); j++) {
            // fetch coordinates
            qreal x_old = line.at(j - 1).x();
            qreal y_old = line.at(j - 1).y();
            qreal x_pos = line.at(j).x();
            qreal y_pos = line.at(j).y();

            // coordinates for our drawn line
            qreal line_x1 = x_old;
            qreal line_y1 = y_old;
            qreal line_x2 = x_pos;
            qreal line_y2 = y_pos;

            // check for (x,y) - if x in left boundary
            if (x_pos < m_x_view_min) {
                // FIXME: Avoid division by zero!
                if ((x_pos - x_old) == 0)
                    x_pos = x_old + 1;

                qreal view_y = (y_pos * (m_x_view_min - x_old)
                        - y_old * (m_x_view_min - x_pos)) / (x_pos - x_old);

                if ((view_y > m_y_view_min) && (view_y < m_y_view_max)) {
                    line_x2 = m_x_view_min;
                    line_y2 = view_y;
                }
            }

            // check for (x,y) - if x in right boundry
            if (x_pos > m_x_view_max) {
                // FIXME: Avoid division by zero!
                if ((x_pos - x_old) == 0)
                    x_pos = x_old + 1;

                double view_y = (y_pos * (m_x_view_max - x_old)
                        - y_old * (m_x_view_max - x_pos)) / (x_pos - x_old);

                if ((view_y > m_y_view_min) && (view_y < m_y_view_max)) {
                    line_x2 = m_x_view_max;
                    line_y2 = view_y;
                }
            }

            // check for (x,y) - if y in top boundry
            if (y_pos < m_y_view_min) {
                // FIXME: Avoid division by zero!
                if ((y_pos - y_old) == 0)
                    y_pos = y_old + 1;

                double view_x = (x_pos * (m_y_view_min - y_old)
                        - x_old * (m_y_view_min - y_pos)) / (y_pos - y_old);

                if ((view_x > m_x_view_min) && (view_x < m_x_view_max)) {
                    line_y2 = m_y_view_min;
                    line_x2 = view_x;
                }
            }

            // check for (x,y) - if y in bottom boundry
            if (y_pos > m_y_view_max) {
                // FIXME: Avoid division by zero!
                if ((y_pos - y_old) == 0)
                    y_pos = y_old + 1;

                double view_x = (x_pos * (m_y_view_max - y_old)
                        - x_old * (m_y_view_max - y_pos)) / (y_pos - y_old);

                if ((view_x > m_x_view_min) && (view_x < m_x_view_max)) {
                    line_y2 = m_y_view_max;
                    line_x2 = view_x;
                }
            }

            // check for (x_old,y_old) - left
            if (x_old < m_x_view_min) {
                // FIXME: Avoid division by zero!
                if ((x_pos - x_old) == 0)
                    x_pos = x_old + 1;

                double view_y = (y_pos * (m_x_view_min - x_old)
                        - y_old * (m_x_view_min - x_pos)) / (x_pos - x_old);

                if ((view_y > m_y_view_min) && (view_y < m_y_view_max)) {
                    line_x1 = m_x_view_min;
                    line_y1 = view_y;
                }
            }

            // check for (x_old,y_old) - right
            if (x_old > m_x_view_max) {
                // FIXME: Avoid division by zero!
                if ((x_pos - x_old) == 0)
                    x_pos = x_old + 1;

                double view_y = (y_pos * (m_x_view_max - x_old)
                        - y_old * (m_x_view_max - x_pos)) / (x_pos - x_old);

                if ((view_y > m_y_view_min) && (view_y < m_y_view_max)) {
                    line_x1 = m_x_view_max;
                    line_y1 = view_y;
                }
            }

            // check for (x_old,y_old) - top
            if (y_old < m_y_view_min) {
                // FIXME: Avoid division by zero!
                if ((y_pos - y_old) == 0)
                    y_pos = y_old + 1;

                double view_x = (x_pos * (m_y_view_min - y_old)
                        - x_old * (m_y_view_min - y_pos)) / (y_pos - y_old);

                if ((view_x > m_x_view_min) && (view_x < m_x_view_max)) {
                    line_y1 = m_y_view_min;
                    line_x1 = view_x;
                }
            }

            // check for (x_old,y_old) - bottom
            if (y_old > m_y_view_max) {
                // FIXME: Avoid division by zero!
                if ((y_pos - y_old) == 0)
                    y_pos = y_old + 1;

                double view_x = (x_pos * (m_y_view_max - y_old)
                        - x_old * (m_y_view_max - y_pos)) / (y_pos - y_old);

                if ((view_x > m_x_view_min) && (view_x < m_x_view_max)) {
                    line_y1 = m_y_view_max;
                    line_x1 = view_x;
                }
            }

            int x1_pos = m_coord_x_min
                    + (int) ((line_x1 - m_x_view_min) * m_factor_x);
            int y1_pos = m_coord_y_max
                    - (int) ((line_y1 - m_y_view_min) * m_factor_y);
            int x2_pos = m_coord_x_min
                    + (int) ((line_x2 - m_x_view_min) * m_factor_x);
            int y2_pos = m_coord_y_max
                    - (int) ((line_y2 - m_y_view_min) * m_factor_y);

            if ((x1_pos >= m_coord_x_min) && (x1_pos <= m_coord_x_max)
                    && (y1_pos >= m_coord_y_min) && (y1_pos <= m_coord_y_max)
                    && (x2_pos >= m_coord_x_min) && (x2_pos <= m_coord_x_max)
                    && (y2_pos >= m_coord_y_min) && (y2_pos <= m_coord_y_max)) {

                QPen line_pen = QPen(m_lines_colors.at(i));
                line_pen.setWidth(3);
                painter.setPen(line_pen);

                painter.drawLine(x1_pos, y1_pos, x2_pos, y2_pos);

                painter.drawEllipse(QPointF(x1_pos, y1_pos), 3, 3);
                painter.drawEllipse(QPointF(x2_pos, y2_pos), 3, 3);
                painter.setPen(QPen(QColor(255, 255, 255)));
                painter.drawEllipse(QPointF(x1_pos, y1_pos), 1, 1);
                painter.drawEllipse(QPointF(x2_pos, y2_pos), 1, 1);
            }
        }
    }
}

void LineChart::drawBubbles(QPainter &/*painter*/) {

}

int LineChart::addBubble(const QVector<QPointF> &/*position*/,
        qreal /*diameter*/, const QColor &/*color*/) {
    // FIXME / TODO
    return 0;
}

int LineChart::addLine(const QVector<QPointF> &pointPairs,
        const QColor &color) {
    m_lines.append(pointPairs);
    m_lines_colors.append(color);

    m_has_data = true;
    m_needs_repaint = true;
    return m_lines.size() - 1; // Return index position.
}

void LineChart::setViewCoords(double x_min, double y_min, double x_max,
        double y_max) {
    m_x_view_min = x_min;
    m_x_view_max = x_max;
    m_y_view_min = y_min;
    m_y_view_max = y_max;

    m_needs_repaint = true;
}

void LineChart::setStepping(double x_step, double y_step) {
    m_x_stepping = x_step;
    m_y_stepping = y_step;
}

void LineChart::clear() {
    m_lines.clear();

    m_needs_repaint = true;
}

void LineChart::mousePressEvent(QMouseEvent *qme) {
    m_mouse_x_min = qme->x();
    m_mouse_y_min = qme->y();

    // check, if our position is out of border
    if (m_mouse_x_min < m_coord_x_min)
        m_mouse_x_min = m_coord_x_min;
    if (m_mouse_x_min > m_coord_x_max)
        m_mouse_x_min = m_coord_x_max;
    if (m_mouse_y_min < m_coord_y_min)
        m_mouse_y_min = m_coord_y_min;
    if (m_mouse_y_min > m_coord_y_max)
        m_mouse_y_min = m_coord_y_max;

    m_mouse_pressed = true;
    // m_needs_repaint = true;
}

void LineChart::mouseReleaseEvent(QMouseEvent *qme) {
    // fetch mouse coordinates
    m_mouse_x_max = qme->x();
    m_mouse_y_max = qme->y();

    // check, if our position is out of border
    if (m_mouse_x_max < m_coord_x_min)
        m_mouse_x_max = m_coord_x_min;
    if (m_mouse_x_max > m_coord_x_max)
        m_mouse_x_max = m_coord_x_max;
    if (m_mouse_y_max < m_coord_y_min)
        m_mouse_y_max = m_coord_y_min;
    if (m_mouse_y_max > m_coord_y_max)
        m_mouse_y_max = m_coord_y_max;

    // check in minimum and maximum x position are exchanged
    if (m_mouse_x_min > m_mouse_x_max) {
        int temp = m_mouse_x_min;
        m_mouse_x_min = m_mouse_x_max;
        m_mouse_x_max = temp;
    }

    // check in minimum and maximum x position are exchanged
    if (m_mouse_y_min > m_mouse_y_max) {
        int temp = m_mouse_y_min;
        m_mouse_y_min = m_mouse_y_max;
        m_mouse_y_max = temp;
    }

    // do not zoom if the selected area is < 2 pixel
    if ((m_mouse_x_max - m_mouse_x_min < 2)
            && (m_mouse_y_max - m_mouse_y_min < 2)) {
        // let our widget redraw itself
        m_mouse_pressed = false;
        // m_needs_repaint = true;
        this->update();

        return; // bye bye ...
    }

    // the back-referencing from the mouse coordinates to original data
    // is done here
    double x_min = (((double) m_mouse_x_min - (double) m_left_border)
            / m_factor_x) + m_x_view_min;
    double x_max = (((double) m_mouse_x_max - (double) m_left_border)
            / m_factor_x) + m_x_view_min;
    // caution: coordinate origin is not equal to the widget origin,
    // the widget origin is in the upper left corner, the coordinate
    // origin in the bottom left corner -> y_min and y_max exchanged
    double y_max = (((double) m_coord_y_max - (double) m_mouse_y_min)
            / m_factor_y) + m_y_view_min;
    double y_min = (((double) m_coord_y_max - (double) m_mouse_y_max)
            / m_factor_y) + m_y_view_min;

    // set new minima and maxima for our coordinate system / view
    m_x_view_min = x_min;
    m_x_view_max = x_max;
    m_y_view_min = y_min;
    m_y_view_max = y_max;

    // let our widget redraw itself
    m_mouse_pressed = false;
    m_needs_repaint = true;
    this->update();
}

void LineChart::mouseMoveEvent(QMouseEvent *qme) {
    if (m_mouse_pressed) {
        // fetch actual mouse positions
        m_mouse_x_act = qme->x();
        m_mouse_y_act = qme->y();

        // check, if our position is out of border
        if (m_mouse_x_act < m_coord_x_min)
            m_mouse_x_act = m_coord_x_min;
        if (m_mouse_x_act > m_coord_x_max)
            m_mouse_x_act = m_coord_x_max;
        if (m_mouse_y_act < m_coord_y_min)
            m_mouse_y_act = m_coord_y_min;
        if (m_mouse_y_act > m_coord_y_max)
            m_mouse_y_act = m_coord_y_max;

        // m_needs_repaint = true;
        this->update();

        // draw a rectangle
        /*
         QPixmap temp_pixmap= new QPixmap(m_pixmap);
         QPainter painter;
         painter.begin(temp_pixmap, this);

         QPen coord_pen(yellow, 1); // yellow, 1 pixel wide
         painter->setPen(coord_pen);

         painter->drawRect(m_mouse_x_min, m_mouse_y_min,
         (m_mouse_x_act - m_mouse_x_min),
         (m_mouse_y_act - m_mouse_y_min));

         bitBlt(this, 0, 0, this_pixmap);
         */
    }

    // This clause is called if mouse tracking is enabled. It returns the
    // actual values at the mouse position
    if (m_mouse_tracking) {
        // fetch actual mouse positions
        int mouse_x = qme->x();
        int mouse_y = qme->y();

        // the back-referencing from the mouse coordinates to original data
        // is done here
        double x_pos =
                (((double) mouse_x - (double) m_left_border) / m_factor_x)
                + m_x_view_min;
        // caution: coordinate origin is not equal to the widget origin,
        // the widget origin is in the upper left corner, the coordinate
        // origin in the bottom left corner -> y_min and y_max exchanged
        double y_pos =
                (((double) m_coord_y_max - (double) mouse_y) / m_factor_y)
                + m_y_view_min;

        emit mousePosChangedEvent(x_pos, y_pos);
    }
}

void LineChart::x_ScaleToMax() {
    qreal x_min = 0, x_max = 0;

    for (int i = 0; i < m_lines.size(); i++) {
        QVector<QPointF> line = m_lines.at(i);

        for (int j = 0; j < line.size(); j++) {
            qreal x_pos = line.at(j).x();

            if (!i && !j) // predefinition: first value of first line
            {
                x_min = x_pos;
                x_max = x_pos;
            }

            if (x_pos < x_min)
                x_min = x_pos; // got new minimum value?
            if (x_pos > x_max)
                x_max = x_pos; // got new maximum value?
        }
    }

    m_x_view_min = x_min;
    m_x_view_max = x_max;

    m_needs_repaint = true;
    this->repaint();
}

void LineChart::y_ScaleToMax() {
    double y_min = 0, y_max = 0;

    for (int i = 0; i < m_lines.size(); i++) {
        QVector<QPointF> line = m_lines.at(i);

        for (int j = 0; j < line.size(); j++) {
            qreal y_pos = line.at(j).y();

            if (!i && !j) // predefinition: first value of first line
            {
                y_min = y_pos;
                y_max = y_pos;
            }

            if (y_pos < y_min)
                y_min = y_pos; // got new minimum value?
            if (y_pos > y_max)
                y_max = y_pos; // got new maximum value?
        }
    }

    m_y_view_min = y_min;
    m_y_view_max = y_max;

    m_needs_repaint = true;
    this->repaint();
}

void LineChart::ScaleToMax() {
    this->setUpdatesEnabled(false);

    this->x_ScaleToMax();
    this->y_ScaleToMax();

    this->setUpdatesEnabled(true);

    m_needs_repaint = true;
    this->repaint();
}

void LineChart::MouseTracking(bool state) {
    m_mouse_tracking = state;
}

bool LineChart::save(QString fn) {
    return m_image->save(fn, "PNG"); // save the pixmap as 'fn.png'
}
