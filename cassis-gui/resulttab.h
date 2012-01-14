/*!
 * Result tab
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

#ifndef RESULTTAB_H
#define RESULTTAB_H

#include <QtGui/QTabWidget>

/*!
 * Various forward declarations...
 */
class NameMap;
class QFrame;
class QGridLayout;
class QGroupBox;
class QToolButton;
class QHBoxLayout;
class QLineEdit;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpacerItem;
class QTableWidget;
class QVBoxLayout;
struct BgrTree;
class LineChart;

class ResultTab: public QTabWidget {
    Q_OBJECT

public:
    /*!
     * Constructor
     */
    ResultTab(QWidget *parent = 0);
    /*!
     * Destructor
     */
    virtual ~ResultTab();
private slots:
/*!
 * Slot: Open/load a BGRT file.
 */
void slotOpenBGRT();
/*!
 * Slot: Update the BGRT preferences.
 */
void slotUpdatePrefs();
/*!
 * Slot: Update the BGRT preferences.
 */
void slotGroupMatch(const QStringList &identifier);
/*!
 * Increment/Decrement outgroup limit
 */
void outgroupSliderChanged();
void outgroupEditChanged();
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
QWidget *prefsTab;
QVBoxLayout *verticalLayout_4;
QFrame *infoFrame;
QVBoxLayout *verticalLayout_5;
QLabel *bgrtInfoLabel;
QHBoxLayout *bgrtNameLayout;
QLineEdit *bgrtNameEdit;
QToolButton *bgrtOpenButton;
QHBoxLayout *infoLayout;
QLabel *bgrtIngroupLabel;
QLineEdit *bgrtIngroupEdit;
QLabel *bgrtOutgroupLabel;
QLineEdit *bgrtOutgroupEdit;
QSpacerItem *infoSpacer;
QFrame *prefsFrame;
QVBoxLayout *verticalLayout_3;
QGroupBox *outgrpBox;
QGridLayout *gridLayout_4;
QSlider *outgrpSlider;
QLineEdit *outgrpEdit;
QLabel *outgrpLabel;
QGroupBox *lenBox;
QGridLayout *gridLayout_3;
QLabel *minLenLabel;
QLineEdit *minLenEdit;
QSlider *minLenSlider;
QSlider *maxLenSlider;
QLineEdit *maxLenEdit;
QLabel *maxLenLabel;
QGroupBox *tempBox;
QGridLayout *gridLayout;
QLabel *minTempLabel;
QLineEdit *minTempEdit;
QSlider *minTempSlider;
QSlider *maxTempSlider;
QLineEdit *maxTempEdit;
QLabel *maxTempLabel;
QGroupBox *GCBox;
QGridLayout *gridLayout_2;
QLabel *minGCLabel;
QLineEdit *minGCEdit;
QSlider *minGCSlider;
QSlider *maxGCSlider;
QLineEdit *maxGCEdit;
QLabel *maxGCLabel;
QSpacerItem *prefsSpacer;
QWidget *resultsTab;
QVBoxLayout *verticalLayout_2;
QTableWidget *resultTable;
QHBoxLayout *resultsLayout;
QLabel *selectionSizeLabel;
QLineEdit *selectionSizeEdit;
QSpacerItem *resultSpacer;
QPushButton *markResultsButton;
QPushButton *saveResultsButton;
QWidget *statsTab;
QVBoxLayout *verticalLayout;
LineChart *statsChart;

/*!
 * Member variables...
 */
 BgrTree *BGRT;
NameMap *nameMap;
};

#endif /* RESULTTAB_H */
