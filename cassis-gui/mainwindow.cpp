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

/*!
 * CdCaSSiS includes...
 */
#include "mainwindow.h"
#include "resulttab.h"
#include "treeview.h"

/*!
 * Qt4 includes...
 */
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

/*!
 * Constructor
 */
MainWindow::MainWindow() {
    // Setup the user interface of the main window...
    setupUi();
}

/*!
 * Destructor
 */
MainWindow::~MainWindow() {

}

/*!
 * Setup the user interface
 */
void MainWindow::setupUi() {
    this->setMinimumSize(640, 480);
    this->resize(800, 600);
    actionAboutCaSSiS = new QAction(this);
    actionAboutQt = new QAction(this);
    actionOpenPhylogram = new QAction(this);
    actionOpenBGRT = new QAction(this);
    actionManualSearch = new QAction(this);
    actionSaveResults = new QAction(this);
    actionClose = new QAction(this);
    actionExit = new QAction(this);
    actionMatchSelection = new QAction(this);
    actionMarkResults = new QAction(this);
    centralwidget = new QWidget(this);
    verticalLayout = new QVBoxLayout(centralwidget);
    mainSplitter = new QSplitter(centralwidget);
    mainSplitter->setOrientation(Qt::Horizontal);
    treeView = new TreeView(mainSplitter);
    mainSplitter->addWidget(treeView);
    resultTab = new ResultTab(mainSplitter);
    mainSplitter->addWidget(resultTab);

    verticalLayout->addWidget(mainSplitter);

    this->setCentralWidget(centralwidget);
    menubar = new QMenuBar(this);
    menubar->setGeometry(QRect(0, 0, 800, 24));
    menuFile = new QMenu(menubar);
    menuAbout = new QMenu(menubar);
    this->setMenuBar(menubar);
    statusbar = new QStatusBar(this);
    this->setStatusBar(statusbar);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuAbout->menuAction());
    //
    menuFile->addAction(actionOpenPhylogram);
    connect(actionOpenPhylogram, SIGNAL(triggered()), treeView,
            SLOT(slotOpenPhylo()));
    //
    menuFile->addAction(actionOpenBGRT);
    connect(actionOpenBGRT, SIGNAL(triggered()), resultTab,
            SLOT(slotOpenBGRT()));
    //
    menuFile->addSeparator();
    menuFile->addAction(actionManualSearch);
    menuFile->addAction(actionMatchSelection);
    menuFile->addSeparator();
    menuFile->addAction(actionMarkResults);
    menuFile->addAction(actionSaveResults);
    menuFile->addSeparator();
    menuFile->addAction(actionClose);
    //
    menuFile->addAction(actionExit);
    connect(actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
    //
    menuAbout->addAction(actionAboutCaSSiS);
    connect(actionAboutCaSSiS, SIGNAL(triggered()), this, SLOT(slotAbout()));
    //
    menuAbout->addAction(actionAboutQt);
    connect(actionAboutQt, SIGNAL(triggered()), this, SLOT(slotAboutQt()));
    //
    connect(treeView, SIGNAL(triggerGroupMatch(const QStringList &)), resultTab,
            SLOT(slotGroupMatch(const QStringList &)));

    retranslateUi();
}

void MainWindow::retranslateUi() {
    this->setWindowTitle(
            QApplication::translate("MainWindow", "CdCaSSiS", 0,
                    QApplication::UnicodeUTF8));
    actionAboutCaSSiS->setText(
            QApplication::translate("MainWindow", "About CdCaSSiS", 0,
                    QApplication::UnicodeUTF8));
    actionAboutQt->setText(
            QApplication::translate("MainWindow", "About Qt", 0,
                    QApplication::UnicodeUTF8));
    actionOpenPhylogram->setText(
            QApplication::translate("MainWindow", "Open Phylogram", 0,
                    QApplication::UnicodeUTF8));
    actionOpenBGRT->setText(
            QApplication::translate("MainWindow", "Open BGRT", 0,
                    QApplication::UnicodeUTF8));
    actionManualSearch->setText(
            QApplication::translate("MainWindow", "Manual Search", 0,
                    QApplication::UnicodeUTF8));
    actionSaveResults->setText(
            QApplication::translate("MainWindow", "Save Results", 0,
                    QApplication::UnicodeUTF8));
    actionClose->setText(
            QApplication::translate("MainWindow", "Close", 0,
                    QApplication::UnicodeUTF8));
    actionExit->setText(
            QApplication::translate("MainWindow", "Exit", 0,
                    QApplication::UnicodeUTF8));
    actionMatchSelection->setText(
            QApplication::translate("MainWindow", "Match Selection", 0,
                    QApplication::UnicodeUTF8));
    actionMarkResults->setText(
            QApplication::translate("MainWindow", "Mark Results", 0,
                    QApplication::UnicodeUTF8));
    menuFile->setTitle(
            QApplication::translate("MainWindow", "File", 0,
                    QApplication::UnicodeUTF8));
    menuAbout->setTitle(
            QApplication::translate("MainWindow", "About", 0,
                    QApplication::UnicodeUTF8));
}

/*!
 * Slot: Close/Exit CdCaSSiS.
 */
void MainWindow::slotExit() {
    close();
}

/*!
 * Slot: About CdCaSSiS
 */
void MainWindow::slotAbout() {
    QMessageBox::about(
            this,
            "About CdCaSSiS",
            "This is a pre-release of "
            "CdCaSSiS.\n\nTHIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY "
            "EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, "
            "THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A "
            "PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR "
            "BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, "
            "OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT "
            "OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; "
            "OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF "
            "LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT "
            "(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE "
            "USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF "
            "SUCH DAMAGE."
            "\n\nFeel free to report bugs or suggestions.\n"
            "Kai Christian Bader <baderk@in.tum.de>");
}

/*!
 * Slot: About Qt
 */
void MainWindow::slotAboutQt() {
    QMessageBox::aboutQt(this, tr("About Qt"));
}
