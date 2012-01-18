/*!
 * The CaSSiS Graphical User Interface.
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) GUI.
 *
 * Copyright (C) 2011,2012
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

#include "mainwindow.h"
#include <QtGui/QApplication>

int main(int argc, char **argv) {
    // Initialize the Qt resources... (icons etc.)
    Q_INIT_RESOURCE(cdcassis);

    // Create a Qt application...
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    app.setOrganizationName("TU München");
    app.setApplicationName("CdCaSSiS");
    app.setWindowIcon(QPixmap(":/cassis.png"));

    // Create our main dialog and show it...
    MainWindow window;
    window.show();
    return app.exec();
}
