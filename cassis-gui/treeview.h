/*!
 * Tree view
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) GUI.
 *
 * Copyright (C) 2011
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

#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QtGui/QFrame>
#include <QtGui/QGraphicsView>

/*!
 * Various forward declarations...
 */
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpacerItem;
class QSplitter;
class QToolButton;
class QVBoxLayout;
class Tree;

class BigView: public QGraphicsView {
    Q_OBJECT
public:
    BigView(QWidget *parent = 0);
protected:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
private:
    QPoint m_scenePos;
    bool m_mousePressed;
};

class TreeView: public QFrame {
    Q_OBJECT
public:
    TreeView(QWidget* parent = 0);
    //    QGraphicsView *view() const;
    signals:
    void triggerGroupMatch(const QStringList &identifier);
private slots:
//    void resetView();
//    void setResetButtonEnabled();
void updateYDistance();
//    void toggleOpenGL();
//    void toggleAntialiasing();
//    void print();

void incDist();
void decDist();
//    void rotateLeft();
//    void rotateRight();

/*!
 * Slot: open/load a phylogenetic tree (Newick format).
 */
void slotOpenPhylo();

/*!
 * Slot: submit group match button was clicked...
 */
void slotTriggerMatch();

/*!
 * Slot: Update the minimap...
 */
void slotUpdateMiniView();
private:
void setupUI();
void retranslateUi();

QGridLayout *gridLayout;
QHBoxLayout *buttonBar;
QToolButton *rotateLeftButton;
QToolButton *rotateRightButton;
QToolButton *openGLButton;
QToolButton *aliasingButton;
QSpacerItem *buttonSpacer;
QPushButton *matchButton;
QSplitter *viewSplitter;
QGraphicsView *miniView;
BigView *bigView;
QVBoxLayout *distLayout;
QToolButton *incDistButton;
QSlider *distSlider;
QToolButton *decDistButton;
QHBoxLayout *distanceLayout;
QToolButton *decDistanceButton;
QSlider *distanceSlider;
QToolButton *incDistanceButton;
QToolButton *resetButton;
QFrame *line;
QHBoxLayout *phyloLayout;
QLabel *phyloLabel;
QLineEdit *phyloEdit;
QToolButton *phyloButton;

//QGraphicsView *graphicsView;
//QLabel *label;
//QToolButton *openGlButton;
//QToolButton *antialiasButton;
//QToolButton *printButton;
//QToolButton *resetButton;
//QSlider *zoomSlider;
//QSlider *rotateSlider;
Tree *tree;
};

#endif // TREEVIEW_H
