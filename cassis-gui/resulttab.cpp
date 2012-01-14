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

#include "resulttab.h"
#include "linechart.h"

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QToolButton>
#include <QtGui/QFileDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

/*!
 * libBGRT includes...
 */
#include <cassis/io.h>
#include <cassis/search.h>
#include <cassis/bgrt.h>
#include <cassis/namemap.h>
#include <cassis/thermodynamics.h>

/*!
 * Constructor
 */
ResultTab::ResultTab(QWidget *parentwidget) :
QTabWidget(parentwidget), BGRT(NULL), nameMap(NULL) {
    setupUi();
}

/*!
 * Destructor
 */
ResultTab::~ResultTab() {

}

/*!
 * Setup the user interface
 */
void ResultTab::setupUi() {
    prefsTab = new QWidget();
    verticalLayout_4 = new QVBoxLayout(prefsTab);
    infoFrame = new QFrame(prefsTab);
    infoFrame->setFrameShape(QFrame::StyledPanel);
    infoFrame->setFrameShadow(QFrame::Raised);
    verticalLayout_5 = new QVBoxLayout(infoFrame);
    bgrtInfoLabel = new QLabel(infoFrame);

    verticalLayout_5->addWidget(bgrtInfoLabel);

    bgrtNameLayout = new QHBoxLayout();

    bgrtNameEdit = new QLineEdit(infoFrame);
    bgrtNameEdit->setReadOnly(true);

    bgrtNameLayout->addWidget(bgrtNameEdit);

    bgrtOpenButton = new QToolButton(infoFrame);
    connect(bgrtOpenButton, SIGNAL(clicked()), this, SLOT(slotOpenBGRT()));

    bgrtNameLayout->addWidget(bgrtOpenButton);
    verticalLayout_5->addLayout(bgrtNameLayout);

    infoLayout = new QHBoxLayout();
    bgrtIngroupLabel = new QLabel(infoFrame);

    infoLayout->addWidget(bgrtIngroupLabel);

    bgrtIngroupEdit = new QLineEdit(infoFrame);
    bgrtIngroupEdit->setMaximumSize(QSize(60, 16777215));
    bgrtIngroupEdit->setReadOnly(true);

    infoLayout->addWidget(bgrtIngroupEdit);

    bgrtOutgroupLabel = new QLabel(infoFrame);

    infoLayout->addWidget(bgrtOutgroupLabel);

    bgrtOutgroupEdit = new QLineEdit(infoFrame);
    bgrtOutgroupEdit->setMaximumSize(QSize(60, 16777215));
    bgrtOutgroupEdit->setReadOnly(true);

    infoLayout->addWidget(bgrtOutgroupEdit);

    infoSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding,
            QSizePolicy::Minimum);

    infoLayout->addItem(infoSpacer);

    verticalLayout_5->addLayout(infoLayout);

    verticalLayout_4->addWidget(infoFrame);

    prefsFrame = new QFrame(prefsTab);
    prefsFrame->setFrameShape(QFrame::StyledPanel);
    prefsFrame->setFrameShadow(QFrame::Raised);
    verticalLayout_3 = new QVBoxLayout(prefsFrame);

    outgrpBox = new QGroupBox(prefsFrame);
    gridLayout_4 = new QGridLayout(outgrpBox);
    outgrpSlider = new QSlider(outgrpBox);
    outgrpSlider->setOrientation(Qt::Horizontal);
    connect(outgrpSlider, SIGNAL(valueChanged(int)), this,
            SLOT(outgroupSliderChanged()));

    gridLayout_4->addWidget(outgrpSlider, 0, 0, 1, 1);

    outgrpEdit = new QLineEdit(outgrpBox);
    outgrpEdit->setMaximumSize(QSize(60, 16777215));
    connect(outgrpEdit, SIGNAL(textEdited(const QString&)), this,
            SLOT(outgroupEditChanged()));

    gridLayout_4->addWidget(outgrpEdit, 0, 1, 1, 1);

    outgrpLabel = new QLabel(outgrpBox);

    gridLayout_4->addWidget(outgrpLabel, 0, 2, 1, 1);

    verticalLayout_3->addWidget(outgrpBox);

    lenBox = new QGroupBox(prefsFrame);
    gridLayout_3 = new QGridLayout(lenBox);
    minLenLabel = new QLabel(lenBox);

    gridLayout_3->addWidget(minLenLabel, 0, 0, 1, 1);

    minLenEdit = new QLineEdit(lenBox);
    minLenEdit->setMaximumSize(QSize(60, 16777215));

    gridLayout_3->addWidget(minLenEdit, 0, 1, 1, 1);

    minLenSlider = new QSlider(lenBox);
    minLenSlider->setOrientation(Qt::Horizontal);

    gridLayout_3->addWidget(minLenSlider, 0, 2, 1, 1);

    maxLenSlider = new QSlider(lenBox);
    maxLenSlider->setOrientation(Qt::Horizontal);
    maxLenSlider->setInvertedAppearance(true);

    gridLayout_3->addWidget(maxLenSlider, 1, 2, 1, 1);

    maxLenEdit = new QLineEdit(lenBox);
    maxLenEdit->setMaximumSize(QSize(60, 16777215));

    gridLayout_3->addWidget(maxLenEdit, 1, 3, 1, 1);

    maxLenLabel = new QLabel(lenBox);

    gridLayout_3->addWidget(maxLenLabel, 1, 4, 1, 1);

    verticalLayout_3->addWidget(lenBox);

    tempBox = new QGroupBox(prefsFrame);
    gridLayout = new QGridLayout(tempBox);
    minTempLabel = new QLabel(tempBox);

    gridLayout->addWidget(minTempLabel, 0, 0, 1, 1);

    minTempEdit = new QLineEdit(tempBox);
    minTempEdit->setMaximumSize(QSize(60, 16777215));

    gridLayout->addWidget(minTempEdit, 0, 1, 1, 1);

    minTempSlider = new QSlider(tempBox);
    minTempSlider->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(minTempSlider, 0, 2, 1, 1);

    maxTempSlider = new QSlider(tempBox);
    maxTempSlider->setOrientation(Qt::Horizontal);
    maxTempSlider->setInvertedAppearance(true);
    maxTempSlider->setInvertedControls(false);

    gridLayout->addWidget(maxTempSlider, 1, 2, 1, 1);

    maxTempEdit = new QLineEdit(tempBox);
    maxTempEdit->setMaximumSize(QSize(60, 16777215));

    gridLayout->addWidget(maxTempEdit, 1, 3, 1, 1);

    maxTempLabel = new QLabel(tempBox);

    gridLayout->addWidget(maxTempLabel, 1, 4, 1, 1);

    verticalLayout_3->addWidget(tempBox);

    GCBox = new QGroupBox(prefsFrame);
    gridLayout_2 = new QGridLayout(GCBox);
    minGCLabel = new QLabel(GCBox);

    gridLayout_2->addWidget(minGCLabel, 0, 0, 1, 1);

    minGCEdit = new QLineEdit(GCBox);
    minGCEdit->setMaximumSize(QSize(60, 16777215));

    gridLayout_2->addWidget(minGCEdit, 0, 1, 1, 1);

    minGCSlider = new QSlider(GCBox);
    minGCSlider->setOrientation(Qt::Horizontal);

    gridLayout_2->addWidget(minGCSlider, 0, 2, 1, 1);

    maxGCSlider = new QSlider(GCBox);
    maxGCSlider->setOrientation(Qt::Horizontal);
    maxGCSlider->setInvertedAppearance(true);

    gridLayout_2->addWidget(maxGCSlider, 1, 2, 1, 1);

    maxGCEdit = new QLineEdit(GCBox);
    maxGCEdit->setMaximumSize(QSize(60, 16777215));

    gridLayout_2->addWidget(maxGCEdit, 1, 3, 1, 1);

    maxGCLabel = new QLabel(GCBox);

    gridLayout_2->addWidget(maxGCLabel, 1, 4, 1, 1);

    verticalLayout_3->addWidget(GCBox);

    verticalLayout_4->addWidget(prefsFrame);

    prefsSpacer = new QSpacerItem(20, 81, QSizePolicy::Minimum,
            QSizePolicy::Expanding);

    verticalLayout_4->addItem(prefsSpacer);

    this->addTab(prefsTab, QString());
    resultsTab = new QWidget();
    verticalLayout_2 = new QVBoxLayout(resultsTab);
    resultTable = new QTableWidget(0, 7, resultsTab);
    //
    QStringList tableHeader;
    tableHeader << "Signature" << "Len" << "IN" << "OUT" << "Tm_basic"
            << "Tm_nn" << "G+C";
    resultTable->setHorizontalHeaderLabels(tableHeader);
    //
    // Sorting will be enabled as soon as data is present.
    resultTable->setSortingEnabled(false);

    verticalLayout_2->addWidget(resultTable);

    resultsLayout = new QHBoxLayout();
    selectionSizeLabel = new QLabel(resultsTab);
    resultsLayout->addWidget(selectionSizeLabel);

    selectionSizeEdit = new QLineEdit(resultsTab);
    selectionSizeEdit->setMaximumSize(QSize(120, 16777215));
    selectionSizeEdit->setMaxLength(10);

    resultsLayout->addWidget(selectionSizeEdit);

    resultSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding,
            QSizePolicy::Minimum);

    resultsLayout->addItem(resultSpacer);

    markResultsButton = new QPushButton(resultsTab);
    markResultsButton->setEnabled(false); // TODO: NOT YET IMPLEMENTED!

    resultsLayout->addWidget(markResultsButton);

    saveResultsButton = new QPushButton(resultsTab);
    saveResultsButton->setEnabled(false); // TODO: NOT YET IMPLEMENTED!

    resultsLayout->addWidget(saveResultsButton);

    verticalLayout_2->addLayout(resultsLayout);

    this->addTab(resultsTab, QString());
    statsTab = new QWidget();
    verticalLayout = new QVBoxLayout(statsTab);
    statsChart = new LineChart(statsTab);

    verticalLayout->addWidget(statsChart);

    this->addTab(statsTab, QString());

    retranslateUi();

    this->setCurrentIndex(0);
}

void ResultTab::retranslateUi() {
    this->setWindowTitle(
            QApplication::translate("this", "this", 0,
                    QApplication::UnicodeUTF8));
    bgrtInfoLabel->setText(
            QApplication::translate("this",
                    "Bipartite Graph Representation Tree:", 0,
                    QApplication::UnicodeUTF8));
    bgrtOpenButton->setText(
            QApplication::translate("this", "...", 0,
                    QApplication::UnicodeUTF8));
    bgrtIngroupLabel->setText(
            QApplication::translate("this", "Allowed ingroup mismatches:", 0,
                    QApplication::UnicodeUTF8));
    bgrtOutgroupLabel->setText(
            QApplication::translate("this", "Mismatch-distance to outgroup:", 0,
                    QApplication::UnicodeUTF8));
    outgrpBox->setTitle(
            QApplication::translate("this", "Outgroup hits (species)", 0,
                    QApplication::UnicodeUTF8));
    outgrpLabel->setText(
            QApplication::translate("this", "Max.", 0,
                    QApplication::UnicodeUTF8));
    lenBox->setTitle(
            QApplication::translate("this", "Signature length (bases)", 0,
                    QApplication::UnicodeUTF8));
    minLenLabel->setText(
            QApplication::translate("this", "Min.", 0,
                    QApplication::UnicodeUTF8));
    maxLenLabel->setText(
            QApplication::translate("this", "Max.", 0,
                    QApplication::UnicodeUTF8));
    tempBox->setTitle(
            QApplication::translate("this", "Melting Temperature (\302\260C)",
                    0, QApplication::UnicodeUTF8));
    minTempLabel->setText(
            QApplication::translate("this", "Min.", 0,
                    QApplication::UnicodeUTF8));
    maxTempLabel->setText(
            QApplication::translate("this", "Max.", 0,
                    QApplication::UnicodeUTF8));
    GCBox->setTitle(
            QApplication::translate("this", "G+C Content (%)", 0,
                    QApplication::UnicodeUTF8));
    minGCLabel->setText(
            QApplication::translate("this", "Min.", 0,
                    QApplication::UnicodeUTF8));
    maxGCLabel->setText(
            QApplication::translate("this", "Max.", 0,
                    QApplication::UnicodeUTF8));
    this->setTabText(
            this->indexOf(prefsTab),
            QApplication::translate("this", "Preferences", 0,
                    QApplication::UnicodeUTF8));
    selectionSizeLabel->setText(
            QApplication::translate("this", "Selection size:", 0,
                    QApplication::UnicodeUTF8));
    markResultsButton->setText(
            QApplication::translate("this", "Mark Results", 0,
                    QApplication::UnicodeUTF8));
    saveResultsButton->setText(
            QApplication::translate("this", "Save Results", 0,
                    QApplication::UnicodeUTF8));
    this->setTabText(
            this->indexOf(resultsTab),
            QApplication::translate("this", "Result Table", 0,
                    QApplication::UnicodeUTF8));
    this->setTabText(
            this->indexOf(statsTab),
            QApplication::translate("this", "Statistics", 0,
                    QApplication::UnicodeUTF8));
}

void ResultTab::slotOpenBGRT() {
    // Create an open file dialog and fetch the name of the BGRT file.
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open BGRT File"),
            "", tr("BGRTree (*.bgrt *.bgr)"));
    if (fileName.isEmpty())
        return;

    // Create a new name map and fetch the BGRT from the file.
    nameMap = new NameMap();
    BGRT = readBGRTFile(nameMap, fileName.toAscii());

    // Abort, if an error occurred.
    if (BGRT == NULL || nameMap == NULL) {
        QMessageBox::critical(this, "CdCaSSiS Error",
                "Error while opening BGRT file:\n" + fileName);
        delete nameMap;
        return;
    }

    // Update the BGRT info fields...
    bgrtNameEdit->setText(fileName);
    bgrtIngroupEdit->setText(
            QString("%1").arg(BGRT->ingroup_mismatch_distance));
    bgrtOutgroupEdit->setText(
            QString("%1").arg(BGRT->outgroup_mismatch_distance));

    // Update the preferences.
    slotUpdatePrefs();

    //  Various status messages (based on the info from the BGRT).
    //statusEdit->append(QString("Opened BGRT file: " + fileName));
    //statusEdit->append(QString("- %1 species entries").arg(BGRT->max_species));
    //statusEdit->append(
    //        QString("- %1 allowed ingroup mismatches").arg(
    //                BGRT->ingroup_mismatch_distance));
    //statusEdit->append(
    //        QString("- %1 mismatches distance to outgroup").arg(
    //                BGRT->outgroup_mismatch_distance));
    //if (BGRT->min_gc == BGRT->max_gc)
    //    statusEdit->append(
    //            QString("- %1 bases length").arg(BGRT->min_gc));
    //else
    //    statusEdit->append(
    //            QString("- %1-%2 bases length").arg(BGRT->min_gc).arg(
    //                    BGRT->max_gc));
    //if (BGRT->min_gc > 0 || BGRT->max_gc < 100)
    //    statusEdit->append(
    //            QString("- %1-%2 % G+C content").arg(BGRT->min_gc).arg(
    //                    BGRT->max_gc));
    //if (BGRT->min_temp > -270 || BGRT->max_temp < 270)
    //    statusEdit->append(
    //            QString("- %1-%2 °C basic melting Temp.").arg(BGRT->min_temp).arg(
    //                    BGRT->max_temp));
    //if (BGRT->comment)
    //    statusEdit->append(QString("- Comment: %1").arg(BGRT->comment));

    //  Enable the 'Manual Search' action.
    //    actionManualSearch->setEnabled(true);
}

void ResultTab::slotUpdatePrefs() {
    // Update "outgroup hits": default= 16 allowed outgroup hits
    outgrpSlider->setMinimum(0);
    outgrpSlider->setMaximum(BGRT->num_species);
    outgrpSlider->setValue(9);
    outgrpEdit->setText(QString::number(outgrpSlider->value()));

    // Update "length"
    minLenSlider->setMinimum(BGRT->min_oligo_len);
    minLenSlider->setMaximum(BGRT->max_oligo_len);
    minLenSlider->setValue(BGRT->min_oligo_len);
    //
    maxLenSlider->setMinimum(BGRT->min_oligo_len);
    maxLenSlider->setMaximum(BGRT->max_oligo_len);
    maxLenSlider->setValue(BGRT->max_oligo_len);
    //
    minLenEdit->setText(QString("%1").arg(BGRT->min_oligo_len));
    maxLenEdit->setText(QString("%1").arg(BGRT->max_oligo_len));

    // Update "temperature"
    minTempSlider->setMinimum(BGRT->min_temp);
    minTempSlider->setMaximum(BGRT->max_temp);
    minTempSlider->setValue(BGRT->min_temp);
    //
    maxTempSlider->setMinimum(BGRT->min_temp);
    maxTempSlider->setMaximum(BGRT->max_temp);
    maxTempSlider->setValue(BGRT->max_temp);
    //
    minTempEdit->setText(QString("%1").arg(BGRT->min_temp));
    maxTempEdit->setText(QString("%1").arg(BGRT->max_temp));

    // Update "G+C content"
    minGCSlider->setMinimum(BGRT->min_gc);
    minGCSlider->setMaximum(BGRT->max_gc);
    minGCSlider->setValue(BGRT->min_gc);
    //
    maxGCSlider->setMinimum(BGRT->min_gc);
    maxGCSlider->setMaximum(BGRT->max_gc);
    maxGCSlider->setValue(BGRT->max_gc);
    //
    minGCEdit->setText(QString("%1").arg(BGRT->min_gc));
    maxGCEdit->setText(QString("%1").arg(BGRT->max_gc));
}

/*!
 * Slot: Update the BGRT preferences.
 */
void ResultTab::slotGroupMatch(const QStringList &identifier) {
    // Abort, if an error occurred.
    if (BGRT == NULL || nameMap == NULL) {
        QMessageBox::critical(this, "CdCaSSiS Error",
                "Please load a BGRT file before matching species groups.");
        return;
    }
    if (identifier.size() == 0) {
        QMessageBox::critical(this, "CdCaSSiS Error",
                "No species entries were selected.");
        return;
    }

    // Set of referenced identifiers...
    IntSet *ids = new IntSet();

    // Unreferenced identifiers are stored in here...
    QStringList unref_ids;

    // Map the string identifiers to integers.
    for (int i = 0; i < identifier.size(); ++i) {
        unsigned int id = nameMap->id(identifier.at(i).toAscii().constData());

        if (!id)
            // Identifier not found in the BGRT nameMap!
            unref_ids.append(identifier.at(i));
        else
            // Add identifier to our set.
            ids->add(id);
    }

    // Update result size info...
    selectionSizeEdit->setText(QString("%1").arg(ids->size()));

    // FIXME: Hardcoded outgroup limit!
    unsigned int outgroup_limit = outgrpSlider->value();

    // TODO: Update the line chart...
    QVector<QPointF> linechart_matches;
    qreal linechart_y_max = 0.0;

    // Fetch specific signatures for the selected group.
    unsigned int *num_matches = NULL;
    StrRefSet *signatures = NULL;
    findGroupSpecificSignatures(BGRT, ids, num_matches, signatures,
            outgroup_limit);

    Thermodynamics thermo;

    unsigned int row_counter = 0;
    if (num_matches && signatures) {
        resultTable->setSortingEnabled(false);
        resultTable->clearContents();

        for (unsigned int outg = 0; outg <= outgroup_limit; ++outg) {
            // struct ResultEntry *entry = ResultList_get(results, outg, 0);

            if (signatures[outg].size() > 0) {
                linechart_matches.append(QPointF(outg, num_matches[outg]));
                if (linechart_y_max < num_matches[outg])
                    linechart_y_max = num_matches[outg];

                // unsigned int my_size = 0;
                // if (results->base4_compressed)
                // my_size = entry->signatures.base4->size();
                // else
                unsigned int my_size = signatures[outg].size();

                for (unsigned int j = 0; j < my_size; ++j) {
                    resultTable->insertRow(row_counter);

                    // const char *signature = NULL;
                    // if (results->base4_compressed)
                    // signature = entry->signatures.base4->val(j)->toChar(
                    // true); // TODO: RNA!
                    // else
                    const char *signature = signatures[outg].val(j);

                    // Set table item 'Signature'...
                    QTableWidgetItem *item = new QTableWidgetItem(
                            QString(signature));
                    resultTable->setItem(row_counter, 0, item);

                    // Set table item 'Length'...
                    item = new QTableWidgetItem();
                    item->setData(Qt::DisplayRole,
                            (unsigned int) strlen(signature));
                    resultTable->setItem(row_counter, 1, item);

                    // Set table item 'Ingroup'...
                    item = new QTableWidgetItem();
                    item->setData(Qt::DisplayRole, num_matches[outg]);
                    resultTable->setItem(row_counter, 2, item);

                    // Set table item 'Outgroup'...
                    item = new QTableWidgetItem();
                    item->setData(Qt::DisplayRole, +outg);
                    resultTable->setItem(row_counter, 3, item);

                    thermo.process(signature);

                    // Free signature string, if it was computed from a
                    // base4 compressed string.
                    // if (results->base4_compressed)
                    // free((void*) signature);

                    // Set table item 'Tm_basic'...
                    item = new QTableWidgetItem();
                    item->setData(Qt::DisplayRole, thermo.get_tm_basic());
                    resultTable->setItem(row_counter, 4, item);

                    // Set table item 'Tm_nn'...
                    item = new QTableWidgetItem();
                    item->setData(Qt::DisplayRole,
                            thermo.get_tm_base_stacking());
                    resultTable->setItem(row_counter, 5, item);

                    // Set table item 'G+C'...
                    item = new QTableWidgetItem();
                    item->setData(Qt::DisplayRole, thermo.get_gc_content());
                    resultTable->setItem(row_counter, 6, item);

                    ++row_counter;
                }
            }
        }
        resultTable->setRowCount(row_counter);
        resultTable->resizeColumnsToContents();
        resultTable->setSortingEnabled(true);
    }

    // Switch to the result table tab.
    this->setCurrentIndex(1);

    // TODO: Update the line chart...
    statsChart->clear();
    statsChart->setViewCoords(0, 0, outgroup_limit, linechart_y_max);
    statsChart->setStepping(10, 10);
    statsChart->addLine(linechart_matches, QColor(127, 127, 255));
}

/*!
 * Increment/Decrement outgroup limit
 */
void ResultTab::outgroupSliderChanged() {
    outgrpEdit->setText(QString::number(outgrpSlider->value()));
}

void ResultTab::outgroupEditChanged() {
    outgrpSlider->setValue(outgrpEdit->text().toInt());
    // Back-reference the change... (filters erroneous values)
    outgroupSliderChanged();
}
