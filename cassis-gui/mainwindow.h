/*!
 * Main user interface window (Qt).
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

/*!
 * Various forward declarations...
 */
class QFrame;
class QSplitter;
class QVBoxLayout;
class ResultTab;
class TreeView;

/*!
 * The CdCaSSiS main window class.
 */
class MainWindow: public QMainWindow {
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    MainWindow();

    /*!
     * Destructor
     */
    virtual ~MainWindow();
private slots:
/*!
 * Slot: Close/Exit CdCaSSiS.
 */
void slotExit();

/*!
 * Slot: About CdCaSSiS
 */
void slotAbout();

/*!
 * Slot: About Qt
 */
void slotAboutQt();
private:
/*!
 * Setup the user interface.
 */
void setupUi();

/*!
 * Add user interface text/captions.
 */
void retranslateUi();

/*!
 * Widgets...
 */
QWidget *centralwidget;
QVBoxLayout *verticalLayout;
QSplitter *mainSplitter;
QMenuBar *menubar;
QMenu *menuFile;
QMenu *menuAbout;
QStatusBar *statusbar;
ResultTab *resultTab;
TreeView *treeView;

/*!
 * Actions...
 */
QAction *actionAboutCaSSiS;
QAction *actionAboutQt;
QAction *actionOpenBGRT;
QAction *actionOpenPhylogram;
QAction *actionManualSearch;
QAction *actionSaveResults;
QAction *actionClose;
QAction *actionExit;
QAction *actionMatchSelection;
QAction *actionMarkResults;
};

#endif // MAINWINDOW_H
