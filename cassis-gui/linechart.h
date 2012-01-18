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

#ifndef LINECHART_H
#define LINECHART_H

#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtGui/QWidget>
#include <QtGui/QPen>

class LineChart: public QWidget {
    Q_OBJECT
public:
    // constructor
    LineChart(QWidget *parent = 0);
    void flushall(); // flush all line charts
    void flush(int index); // flushes one line at position 'index'
    // set new view coordinates
    void setViewCoords(double x_min, double y_min, double x_max, double y_max);
    void setStepping(double x_step, double y_step);
    int addLine(const QVector<QPointF> &pointPairs, const QColor &color);
    int addBubble(const QVector<QPointF> &position, qreal diameter,
            const QColor &color);
    void clear();
    void x_ScaleToMax();
    void y_ScaleToMax();
    void ScaleToMax();
    void MouseTracking(bool);
    bool save(QString fn);
public slots:
signals:
void mousePosChangedEvent(double, double);
//void mousePosPressedEvent(uint, uint, uint, uint);
protected:
void paintEvent(QPaintEvent *); // overloaded, triggers a repaint
void resizeEvent(QResizeEvent *); // overloaded, triggers a repaint
void mouseMoveEvent(QMouseEvent *);
void mousePressEvent(QMouseEvent *);
void mouseReleaseEvent(QMouseEvent *);
void drawChart(QPainter &painter);
void drawLines(QPainter &painter);
void drawBubbles(QPainter &painter);
private:
QImage *m_image; // our chart is stored in here
bool m_needs_repaint; // true= a repaint is necessary
bool m_has_data; // true= one or more lines are available
//
double m_x_view_min; // minimum x coordinate of visible area
double m_x_view_max; // maximum x coordinate of visible area
double m_y_view_min; // minimum y coordinate of visible area
double m_y_view_max; // maximum y coordinate of visible area
//
double m_x_stepping; // one step width in our scale
double m_y_stepping; // one step height in our scale
bool m_x_autostepping; //true= scale width is set automatically
bool m_y_autostepping; //true= scale height is set automatically
//
bool m_x_trim2max; // graph width is adjusted to the data range
bool m_y_trim2max; // graph height is adjusted to the data range
//
int m_widget_width; // widget width
int m_widget_height; // widget height
int m_left_border; // left space between border and graph
int m_top_border; // top space between border and graph
int m_right_border; // right space between border and graph
int m_bottom_border; // bottom space between border and graph
int m_coord_x_min; // left position of the coordinate system
int m_coord_x_max; // right position of the coordinate system
int m_coord_y_min; // top position of the coordinate system
int m_coord_y_max; // bottom position of the coordinate system
int m_coord_width; // coordinate system width
int m_coord_height; // coord height
//
// these are the relations between the real-values and the pixels
// on our screen (= multiplicator)
double m_factor_x;
double m_factor_y;
//
// mouse positions (pressed button) are stored here
int m_mouse_x_min;
int m_mouse_x_max;
int m_mouse_x_act;
int m_mouse_y_act;
int m_mouse_y_min;
int m_mouse_y_max;
bool m_mouse_pressed;
//
QVector<QVector<QPointF> > m_lines; // all lines are stored in here
QVector<QColor> m_lines_colors; // all line colors are stored in here
//
QVector<QPointF> m_bubbles; // all bubbles
QVector<QColor> m_bubbles_colors; // all bubble colors are stored in here
//
bool m_mouse_tracking; // signal mousePosChangedEvent _on_ or _off_
};

#endif // LINECHART_H
