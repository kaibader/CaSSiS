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

#include "treeview.h"

#include <Qt/QtGui>
#ifndef QT_NO_OPENGL
#include <Qt/QtOpenGL>
#endif

#include <QtCore/qmath.h>

#include "tree.h"

/*!
 * libBGRT includes...
 */
//#include <cassis/io.h>
//#include <cassis/search.h>
//#include <cassis/bgrt.h>
#include <cassis/namemap.h>
//#include <cassis/thermodynamics.h>
// TODO: Dirty include!
#include "../cassis-cli/newick.h"

BigView::BigView(QWidget *p) :
QGraphicsView(p), m_mousePressed(false) {
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void BigView::wheelEvent(QWheelEvent *e) {
    qreal factor = 1.2;
    if (e->delta() < 0)
        factor = 1.0 / factor;
    scale(factor, factor);
}

void BigView::mouseMoveEvent(QMouseEvent *e) {
    //    if (m_mousePressed) {
    //        QPoint old_pos = m_scenePos;
    //        m_scenePos = mapFromScene(event->posF());
    //
    //        scrollContentsBy(m_scenePos.x() - old_pos.x(),
    //                m_scenePos.y() - old_pos.y());
    //    } else
    QGraphicsView::mouseMoveEvent(e);
}

void BigView::mousePressEvent(QMouseEvent *e) {
    QGraphicsView::mousePressEvent(e);

    //    // Fetch the mouse if no other item has accepted the event...
    //    if (!event->isAccepted()) {
    //        setCursor(Qt::SizeAllCursor);
    //        m_mousePressed = true;
    //        m_scenePos = mapFromScene(event->posF());
    //        event->accept();
    //    }
}

void BigView::mouseReleaseEvent(QMouseEvent *e) {
    //    if (m_mousePressed) {
    //        setCursor(Qt::ArrowCursor);
    //        m_mousePressed = false;
    //        event->accept();
    //    } else
    QGraphicsView::mouseReleaseEvent(e);
}

TreeView::TreeView(QWidget *p) :
                QFrame(p), tree(NULL) {
    setupUI();

    //    setFrameStyle(Sunken | StyledPanel);
    //    graphicsView = new QGraphicsView;
    //    graphicsView->setRenderHint(QPainter::Antialiasing, false);
    //    graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    //    graphicsView->setOptimizationFlags(QGraphicsView::DontSavePainterState);
    //    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    //
    //    int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
    //    QSize iconSize(size, size);
    //
    //    QToolButton *zoomInIcon = new QToolButton;
    //    zoomInIcon->setAutoRepeat(true);
    //    zoomInIcon->setAutoRepeatInterval(33);
    //    zoomInIcon->setAutoRepeatDelay(0);
    //    zoomInIcon->setIcon(QPixmap(":/zoomin.png"));
    //    zoomInIcon->setIconSize(iconSize);
    //    QToolButton *zoomOutIcon = new QToolButton;
    //    zoomOutIcon->setAutoRepeat(true);
    //    zoomOutIcon->setAutoRepeatInterval(33);
    //    zoomOutIcon->setAutoRepeatDelay(0);
    //    zoomOutIcon->setIcon(QPixmap(":/zoomout.png"));
    //    zoomOutIcon->setIconSize(iconSize);
    //    zoomSlider = new QSlider;
    //    zoomSlider->setMinimum(0);
    //    zoomSlider->setMaximum(500);
    //    zoomSlider->setValue(250);
    //    zoomSlider->setTickPosition(QSlider::TicksRight);
    //
    //    // Zoom slider layout
    //    QVBoxLayout *zoomSliderLayout = new QVBoxLayout;
    //    zoomSliderLayout->addWidget(zoomInIcon);
    //    zoomSliderLayout->addWidget(zoomSlider);
    //    zoomSliderLayout->addWidget(zoomOutIcon);
    //
    //    QToolButton *rotateLeftIcon = new QToolButton;
    //    rotateLeftIcon->setIcon(QPixmap(":/rotateleft.png"));
    //    rotateLeftIcon->setIconSize(iconSize);
    //    QToolButton *rotateRightIcon = new QToolButton;
    //    rotateRightIcon->setIcon(QPixmap(":/rotateright.png"));
    //    rotateRightIcon->setIconSize(iconSize);
    //    rotateSlider = new QSlider;
    //    rotateSlider->setOrientation(Qt::Horizontal);
    //    rotateSlider->setMinimum(-360);
    //    rotateSlider->setMaximum(360);
    //    rotateSlider->setValue(0);
    //    rotateSlider->setTickPosition(QSlider::TicksBelow);
    //
    //    // Rotate slider layout
    //    QHBoxLayout *rotateSliderLayout = new QHBoxLayout;
    //    rotateSliderLayout->addWidget(rotateLeftIcon);
    //    rotateSliderLayout->addWidget(rotateSlider);
    //    rotateSliderLayout->addWidget(rotateRightIcon);
    //
    //    resetButton = new QToolButton;
    //    resetButton->setText(tr("0"));
    //    resetButton->setEnabled(false);
    //
    //    // Label layout
    //    QHBoxLayout *labelLayout = new QHBoxLayout;
    //    label = new QLabel(tr("Phylogenetic tree"));
    //    antialiasButton = new QToolButton;
    //    antialiasButton->setText(tr("Antialiasing"));
    //    antialiasButton->setCheckable(true);
    //    antialiasButton->setChecked(false);
    //    openGlButton = new QToolButton;
    //    openGlButton->setText(tr("OpenGL"));
    //    openGlButton->setCheckable(true);
    //#ifndef QT_NO_OPENGL
    //    openGlButton->setEnabled(QGLFormat::hasOpenGL());
    //#else
    //    openGlButton->setEnabled(false);
    //#endif
    //    printButton = new QToolButton;
    //    printButton->setIcon(QIcon(QPixmap(":/fileprint.png")));
    //
    //    labelLayout->addWidget(label);
    //    labelLayout->addStretch();
    //    labelLayout->addWidget(antialiasButton);
    //    labelLayout->addWidget(openGlButton);
    //    labelLayout->addWidget(printButton);
    //
    //    QGridLayout *topLayout = new QGridLayout;
    //    topLayout->addLayout(labelLayout, 0, 0);
    //    topLayout->addWidget(graphicsView, 1, 0);
    //    topLayout->addLayout(zoomSliderLayout, 1, 1);
    //    topLayout->addLayout(rotateSliderLayout, 2, 0);
    //    topLayout->addWidget(resetButton, 2, 1);
    //    setLayout(topLayout);
    //
    //    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetView()));
    //    connect(zoomSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    //    connect(rotateSlider, SIGNAL(valueChanged(int)), this, SLOT(setupMatrix()));
    //    connect(graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this,
    //            SLOT(setResetButtonEnabled()));
    //    connect(graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
    //            this, SLOT(setResetButtonEnabled()));
    //    connect(antialiasButton, SIGNAL(toggled(bool)), this,
    //            SLOT(toggleAntialiasing()));
    //    connect(openGlButton, SIGNAL(toggled(bool)), this, SLOT(toggleOpenGL()));
    //    connect(rotateLeftIcon, SIGNAL(clicked()), this, SLOT(rotateLeft()));
    //    connect(rotateRightIcon, SIGNAL(clicked()), this, SLOT(rotateRight()));
    //    connect(zoomInIcon, SIGNAL(clicked()), this, SLOT(zoomIn()));
    //    connect(zoomOutIcon, SIGNAL(clicked()), this, SLOT(zoomOut()));
    //    connect(printButton, SIGNAL(clicked()), this, SLOT(print()));
    //
    //    setupMatrix();
}

void TreeView::setupUI() {
    this->resize(800, 600);
    this->setFrameStyle(Sunken | StyledPanel);

    gridLayout = new QGridLayout(this);
    buttonBar = new QHBoxLayout();
    rotateLeftButton = new QToolButton(this);
    rotateLeftButton->setIcon(QPixmap(":/rotateleft.png"));
    rotateLeftButton->setEnabled(false); // TODO: NOT YET IMPLEMENTED!

    buttonBar->addWidget(rotateLeftButton);

    rotateRightButton = new QToolButton(this);
    rotateRightButton->setIcon(QPixmap(":/rotateright.png"));
    rotateRightButton->setEnabled(false); // TODO: NOT YET IMPLEMENTED!

    buttonBar->addWidget(rotateRightButton);

    openGLButton = new QToolButton(this);
    openGLButton->setEnabled(false); // TODO: NOT YET IMPLEMENTED!

    buttonBar->addWidget(openGLButton);

    aliasingButton = new QToolButton(this);
    aliasingButton->setEnabled(false); // TODO: NOT YET IMPLEMENTED!

    buttonBar->addWidget(aliasingButton);

    buttonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding,
            QSizePolicy::Minimum);

    buttonBar->addItem(buttonSpacer);

    matchButton = new QPushButton(this);
    connect(matchButton, SIGNAL(clicked()), this, SLOT(slotTriggerMatch()));

    buttonBar->addWidget(matchButton);

    gridLayout->addLayout(buttonBar, 0, 0, 1, 2);

    viewSplitter = new QSplitter(this);
    viewSplitter->setOrientation(Qt::Horizontal);

    miniView = new QGraphicsView(viewSplitter);
    miniView->setMaximumSize(QSize(160, 16777215));
    miniView->setInteractive(false);
    viewSplitter->addWidget(miniView);

    bigView = new BigView(viewSplitter);
    viewSplitter->addWidget(bigView);

    gridLayout->addWidget(viewSplitter, 1, 0, 1, 1);

    distLayout = new QVBoxLayout();
    incDistButton = new QToolButton(this);
    connect(incDistButton, SIGNAL(clicked()), this, SLOT(incDist()));

    distLayout->addWidget(incDistButton);

    distSlider = new QSlider(this);
    distSlider->setOrientation(Qt::Vertical);
    distSlider->setMinimum(0);
    distSlider->setMaximum(50);
    distSlider->setValue(10);
    // distSlider->setTickPosition(QSlider::TicksRight);
    connect(distSlider, SIGNAL(valueChanged(int)), this,
            SLOT(updateYDistance()));

    distLayout->addWidget(distSlider);

    decDistButton = new QToolButton(this);
    connect(decDistButton, SIGNAL(clicked()), this, SLOT(decDist()));

    distLayout->addWidget(decDistButton);

    gridLayout->addLayout(distLayout, 1, 1, 1, 1);

    distanceLayout = new QHBoxLayout();
    decDistanceButton = new QToolButton(this);

    distanceLayout->addWidget(decDistanceButton);

    distanceSlider = new QSlider(this);
    distanceSlider->setOrientation(Qt::Horizontal);

    distanceLayout->addWidget(distanceSlider);

    incDistanceButton = new QToolButton(this);

    distanceLayout->addWidget(incDistanceButton);

    gridLayout->addLayout(distanceLayout, 2, 0, 1, 1);

    resetButton = new QToolButton(this);
    resetButton->setIcon(QPixmap(":/reset.png"));

    gridLayout->addWidget(resetButton, 2, 1, 1, 1);

    line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    gridLayout->addWidget(line, 3, 0, 1, 2);

    phyloLayout = new QHBoxLayout();
    phyloLabel = new QLabel(this);

    phyloLayout->addWidget(phyloLabel);

    phyloEdit = new QLineEdit(this);
    phyloEdit->setReadOnly(true);

    phyloLayout->addWidget(phyloEdit);

    phyloButton = new QToolButton(this);
    connect(phyloButton, SIGNAL(clicked()), this, SLOT(slotOpenPhylo()));

    phyloLayout->addWidget(phyloButton);

    gridLayout->addLayout(phyloLayout, 4, 0, 1, 2);

    retranslateUi();
}

void TreeView::retranslateUi() {
    this->setWindowTitle(
            QApplication::translate("this", "Frame", 0,
                    QApplication::UnicodeUTF8));
    rotateLeftButton->setText(
            QApplication::translate("this", "RL", 0,
                    QApplication::UnicodeUTF8));
    rotateRightButton->setText(
            QApplication::translate("this", "RR", 0,
                    QApplication::UnicodeUTF8));
    openGLButton->setText(
            QApplication::translate("this", "GL", 0,
                    QApplication::UnicodeUTF8));
    aliasingButton->setText(
            QApplication::translate("this", "AL", 0,
                    QApplication::UnicodeUTF8));
    matchButton->setText(
            QApplication::translate("this", "Match Selection", 0,
                    QApplication::UnicodeUTF8));
    incDistButton->setText(
            QApplication::translate("this", "+", 0, QApplication::UnicodeUTF8));
    decDistButton->setText(
            QApplication::translate("this", "-", 0, QApplication::UnicodeUTF8));
    decDistanceButton->setText(
            QApplication::translate("this", "...", 0,
                    QApplication::UnicodeUTF8));
    incDistanceButton->setText(
            QApplication::translate("this", "...", 0,
                    QApplication::UnicodeUTF8));
    resetButton->setText(
            QApplication::translate("this", "0", 0, QApplication::UnicodeUTF8));
    phyloLabel->setText(
            QApplication::translate("this", "Phylogram:", 0,
                    QApplication::UnicodeUTF8));
    phyloButton->setText(
            QApplication::translate("this", "...", 0,
                    QApplication::UnicodeUTF8));
}

//QGraphicsView *TreeView::view() const {
//    return graphicsView;
//}

//void TreeView::resetView() {
//    zoomSlider->setValue(250);
//    rotateSlider->setValue(0);
//    setupMatrix();
//    graphicsView->ensureVisible(QRectF(0, 0, 0, 0));
//
//    resetButton->setEnabled(false);
//}

//void TreeView::setResetButtonEnabled() {
//    resetButton->setEnabled(true);
//}

void TreeView::updateYDistance() {
    if (!tree)
        return;
    tree->setYDistance(distSlider->value());
    tree->invalidate();
    // miniView->invalidateScene();
    // bigView->invalidateScene();
}

//void TreeView::toggleOpenGL() {
//#ifndef QT_NO_OPENGL
//    graphicsView->setViewport(
//            (QWidget*) (openGlButton->isChecked() ? new QGLWidget(
//                    QGLFormat(QGL::SampleBuffers)) : new QWidget));
//#endif
//}

//void TreeView::toggleAntialiasing() {
//    graphicsView->setRenderHint(QPainter::Antialiasing,
//            antialiasButton->isChecked());
//}

//void TreeView::print() {
//#ifndef QT_NO_PRINTER
//    QPrinter printer;
//    QPrintDialog dialog(&printer, this);
//    if (dialog.exec() == QDialog::Accepted) {
//        QPainter painter(&printer);
//        graphicsView->render(&painter);
//    }
//#endif
//}

void TreeView::incDist() {
    distSlider->setValue(distSlider->value() + 1);
}

void TreeView::decDist() {
    distSlider->setValue(distSlider->value() - 1);
}

//void TreeView::rotateLeft() {
//    rotateSlider->setValue(rotateSlider->value() - 10);
//}

//void TreeView::rotateRight() {
//    rotateSlider->setValue(rotateSlider->value() + 10);
//}

/*!
 * Slot: open/load a phylogenetic tree (Newick format).
 */
void TreeView::slotOpenPhylo() {
    // Create an open file dialog and fetch the name of the Newick tree file.
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Tree File"),
            "", tr("Newick Tree (*.tree *.tre)"));
    if (fileName.isEmpty())
        return;

    //  Create a name map for the phylogenetic tree and parse it.
    NameMap phy_tree_map;
    CaSSiSTree *cassis_tree = Newick2CaSSiSTree(fileName.toAscii(), 0);
    if (!cassis_tree)
        return;

    //  Create a new tree scene, based on the phylogenetic tree...
    tree = new Tree(cassis_tree, phy_tree_map);

    // We do not need the phylogenetic tree anymore (we have the scene),
    // so free it.
    delete cassis_tree;

    //// Append/output some status messages...
    //statusEdit->append(QString("Opened Newick tree file: " + fileName));
    //statusEdit->append(
    //        QString("- Binary tree with depth %1 and %2 nodes").arg(
    //                tree->depth()).arg(tree->numNodes()));
    //statusEdit->append(QString("- %1 Species entries").arg(tree->numSpecies()));

    // Connect and enable the tree view.
    bigView->setScene(tree);
    miniView->setScene(tree);
    connect(tree, SIGNAL(changed(const QList<QRectF> &)), this,
            SLOT(slotUpdateMiniView()));

    phyloEdit->setText(fileName);
    this->setEnabled(true);
}

//void TreeView::triggerGroupMatch(const QStringList &identifier) {
//    if (!tree)
//        return;
//
//    QList<QString*> ids = tree->selectedIdentifiers();
//
//    //for (int i = 0; i < ids.length(); ++i) {
//    //    QString id= *ids.at(i);
//    //    identifier.append(id);
//    //}
//}

/*!
 * Slot: submit group match button was clicked...
 */
void TreeView::slotTriggerMatch() {
    if (!tree)
        return;

    QStringList identifier = tree->selectedIdentifiers();
    emit triggerGroupMatch(identifier);
}

/*!
 * Slot: Update the miniature view...
 */
void TreeView::slotUpdateMiniView() {
    miniView->fitInView(tree->sceneRect(), Qt::IgnoreAspectRatio);
    // miniMap->drawFrame()
}
