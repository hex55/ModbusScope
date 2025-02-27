#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "communicationmanager.h"
#include "graphdatamodel.h"
#include "notemodel.h"
#include "errorlogmodel.h"
#include "graphdata.h"
#include "registerdialog.h"
#include "connectiondialog.h"
#include "notesdock.h"
#include "settingsmodel.h"
#include "logdialog.h"
#include "errorlogdialog.h"
#include "aboutdialog.h"
#include "markerinfo.h"
#include "guimodel.h"
#include "extendedgraphview.h"
#include "datafilehandler.h"
#include "projectfilehandler.h"
#include "util.h"

#include <QDateTime>

const QString MainWindow::_cStateRunning = QString("Running");
const QString MainWindow::_cStateStopped = QString("Stopped");
const QString MainWindow::_cStateDataLoaded = QString("Data File loaded");
const QString MainWindow::_cStatsTemplate = QString("Success: %1\tErrors: %2");
const QString MainWindow::_cRuntime = QString("Runtime: %1");

MainWindow::MainWindow(QStringList cmdArguments, QWidget *parent) :
    QMainWindow(parent),
    _pUi(new Ui::MainWindow)
{
    _pUi->setupUi(this);

    _pGuiModel = new GuiModel();

    _pSettingsModel = new SettingsModel();
    _pGraphDataModel = new GraphDataModel(_pSettingsModel);
    _pNoteModel = new NoteModel();
    _pErrorLogModel = new ErrorLogModel();

    _pConnectionDialog = new ConnectionDialog(_pSettingsModel, this);
    _pLogDialog = new LogDialog(_pSettingsModel, _pGuiModel, this);
    _pErrorLogDialog = new ErrorLogDialog(_pErrorLogModel, this);

    _pNotesDock = new NotesDock(_pNoteModel, _pGuiModel, this);

    _pConnMan = new CommunicationManager(_pSettingsModel, _pGuiModel, _pGraphDataModel, _pErrorLogModel);
    _pGraphView = new ExtendedGraphView(_pConnMan, _pGuiModel, _pSettingsModel, _pGraphDataModel, _pNoteModel, _pUi->customPlot, this);

    _pDataFileHandler = new DataFileHandler(_pGuiModel, _pGraphDataModel, _pNoteModel, _pSettingsModel);
    _pProjectFileHandler = new ProjectFileHandler(_pGuiModel, _pSettingsModel, _pGraphDataModel);

    _pLegend = _pUi->legend;
    _pLegend->setModels(_pGuiModel, _pGraphDataModel);
    _pLegend->setGraphview(_pGraphView);

    _pMarkerInfo = _pUi->markerInfo;
    _pMarkerInfo->setModel(_pGuiModel, _pGraphDataModel);

    /*-- Connect menu actions --*/
    connect(_pUi->actionStart, SIGNAL(triggered()), this, SLOT(startScope()));
    connect(_pUi->actionStop, SIGNAL(triggered()), this, SLOT(stopScope()));
    connect(_pUi->actionErrorLog, SIGNAL(triggered()), this, SLOT(showErrorLog()));
    connect(_pUi->actionManageNotes, SIGNAL(triggered()), this, SLOT(showNotesDialog()));
    connect(_pUi->actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));
    connect(_pUi->actionExportDataCsv, SIGNAL(triggered()), _pDataFileHandler, SLOT(selectDataExportFile()));
    connect(_pUi->actionLoadProjectFile, SIGNAL(triggered()), _pProjectFileHandler, SLOT(selectProjectSettingFile()));
    connect(_pUi->actionReloadProjectFile, SIGNAL(triggered()), _pProjectFileHandler, SLOT(reloadProjectFile()));
    connect(_pUi->actionImportDataFile, SIGNAL(triggered()), _pDataFileHandler, SLOT(selectDataImportFile()));
    connect(_pUi->actionExportImage, SIGNAL(triggered()), this, SLOT(selectImageExportFile()));
    connect(_pUi->actionExportSettings, SIGNAL(triggered()), _pProjectFileHandler, SLOT(selectSettingsExportFile()));
    connect(_pUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(_pUi->actionHighlightSamplePoints, SIGNAL(toggled(bool)), _pGuiModel, SLOT(setHighlightSamples(bool)));
    connect(_pUi->actionClearData, SIGNAL(triggered()), this, SLOT(clearData()));
    connect(_pUi->actionClearMarkers, SIGNAL(triggered()), _pGuiModel, SLOT(clearMarkersState()));
    connect(_pUi->actionConnectionSettings, SIGNAL(triggered()), this, SLOT(showConnectionDialog()));
    connect(_pUi->actionLogSettings, SIGNAL(triggered()), this, SLOT(showLogSettingsDialog()));
    connect(_pUi->actionRegisterSettings, SIGNAL(triggered()), this, SLOT(showRegisterDialog()));
    connect(_pUi->actionAddNote, SIGNAL(triggered()), this, SLOT(addNoteToGraph()));

    /*-- connect model to view --*/
    connect(_pGuiModel, SIGNAL(frontGraphChanged()), this, SLOT(updateBringToFrontGrapMenu()));
    connect(_pGuiModel, SIGNAL(frontGraphChanged()), _pGraphView, SLOT(bringToFront()));
    connect(_pGuiModel, SIGNAL(highlightSamplesChanged()), this, SLOT(updateHighlightSampleMenu()));
    connect(_pGuiModel, SIGNAL(highlightSamplesChanged()), _pGraphView, SLOT(enableSamplePoints()));
    connect(_pGuiModel, SIGNAL(cursorValuesChanged()), _pGraphView, SLOT(updateTooltip()));
    connect(_pGuiModel, SIGNAL(cursorValuesChanged()), _pLegend, SLOT(updateDataInLegend()));

    connect(_pGuiModel, SIGNAL(windowTitleChanged()), this, SLOT(updateWindowTitle()));
    connect(_pGuiModel, SIGNAL(projectFilePathChanged()), this, SLOT(projectFileLoaded()));
    connect(_pGuiModel, SIGNAL(dataFilePathChanged()), this, SLOT(dataFileLoaded()));
    connect(_pGuiModel, SIGNAL(guiStateChanged()), this, SLOT(updateGuiState()));

    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), this, SLOT(updatexAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(xAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), this, SLOT(updatexAxisSlidingInterval()));
    connect(_pGuiModel, SIGNAL(xAxisSlidingIntervalChanged()), _pGraphView, SLOT(rescalePlot()));

    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), this, SLOT(updateyAxisSlidingMode()));
    connect(_pGuiModel, SIGNAL(yAxisScalingChanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), this, SLOT(updateyAxisMinMax()));
    connect(_pGuiModel, SIGNAL(yAxisMinMaxchanged()), _pGraphView, SLOT(rescalePlot()));
    connect(_pGuiModel, SIGNAL(communicationStatsChanged()), this, SLOT(updateStats()));

    connect(_pGuiModel, SIGNAL(markerStateChanged()), _pGraphView, SLOT(updateMarkersVisibility()));
    connect(_pGuiModel, SIGNAL(markerStateChanged()), this, SLOT(updateMarkerDockVisibility()));
    connect(_pGuiModel, SIGNAL(startMarkerPosChanged()), _pGraphView, SLOT(setStartMarker()));
    connect(_pGuiModel, SIGNAL(endMarkerPosChanged()), _pGraphView, SLOT(setEndMarker()));

    connect(_pGraphDataModel, SIGNAL(visibilityChanged(quint32)), this, SLOT(handleGraphVisibilityChange(const quint32)));
    connect(_pGraphDataModel, SIGNAL(visibilityChanged(quint32)), _pGraphView, SLOT(showGraph(const quint32)));
    connect(_pGraphDataModel, SIGNAL(graphsAddData(QList<double>, QList<QList<double> >)), _pGraphView, SLOT(addData(QList<double>, QList<QList<double> >)));
    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), this, SLOT(rebuildGraphMenu()));
    connect(_pGraphDataModel, SIGNAL(activeChanged(quint32)), _pGraphView, SLOT(updateGraphs()));
    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), this, SLOT(handleGraphColorChange(quint32)));
    connect(_pGraphDataModel, SIGNAL(colorChanged(quint32)), _pGraphView, SLOT(changeGraphColor(quint32)));
    connect(_pGraphDataModel, SIGNAL(labelChanged(quint32)), this, SLOT(handleGraphLabelChange(quint32)));
    connect(_pGraphDataModel, SIGNAL(labelChanged(quint32)), _pGraphView, SLOT(changeGraphLabel(quint32)));

    connect(_pGraphDataModel, SIGNAL(added(quint32)), this, SLOT(rebuildGraphMenu()));
    connect(_pGraphDataModel, SIGNAL(added(quint32)), _pGraphView, SLOT(updateGraphs()));

    connect(_pGraphDataModel, SIGNAL(removed(quint32)), this, SLOT(rebuildGraphMenu()));
    connect(_pGraphDataModel, SIGNAL(removed(quint32)), _pGraphView, SLOT(updateGraphs()));

    connect(_pGraphDataModel, SIGNAL(unsignedChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(multiplyFactorChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(divideFactorChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(registerAddressChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(bitmaskChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));
    connect(_pGraphDataModel, SIGNAL(shiftChanged(quint32)), _pGraphView, SLOT(clearGraph(quint32)));

    // Update cursor values in legend
    connect(_pGraphView, SIGNAL(cursorValueUpdate()), _pLegend, SLOT(updateDataInLegend()));

    connect(_pGraphView, SIGNAL(dataAddedToPlot(double, QList<double>)), _pDataFileHandler, SLOT(exportDataLine(double, QList <double>)));

    _pGraphShowHide = _pUi->menuShowHide;
    _pGraphBringToFront = _pUi->menuBringToFront;
    _pBringToFrontGroup = new QActionGroup(this);

    // Compose rightclick menu
    _menuRightClick.addMenu(_pUi->menuBringToFront);
    _menuRightClick.addMenu(_pUi->menuShowHide);
    _menuRightClick.addSeparator();
    _menuRightClick.addAction(_pUi->actionHighlightSamplePoints);
    _menuRightClick.addAction(_pUi->actionClearData);
    _menuRightClick.addAction(_pUi->actionClearMarkers);
    _menuRightClick.addSeparator();
    _menuRightClick.addAction(_pUi->actionAddNote);
    _menuRightClick.addAction(_pUi->actionManageNotes);

    // Add multipart status bar
    _pStatusState = new QLabel(_cStateStopped, this);
    _pStatusState->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _pStatusStats = new QLabel("", this);
    _pStatusStats->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    _pStatusRuntime = new QLabel("", this);
    _pStatusRuntime->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    _pUi->statusBar->addPermanentWidget(_pStatusState, 1);
    _pUi->statusBar->addPermanentWidget(_pStatusRuntime, 2);
    _pUi->statusBar->addPermanentWidget(_pStatusStats, 3);

    this->setAcceptDrops(true);

    // For dock undock
    connect(_pUi->scaleOptionsDock, SIGNAL(topLevelChanged(bool)), this, SLOT(scaleWidgetUndocked(bool)));
    connect(_pUi->legendDock, SIGNAL(topLevelChanged(bool)), this, SLOT(legendWidgetUndocked(bool)));

    // For rightclick menu
    _pUi->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_pUi->customPlot, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    connect(_pUi->spinSlidingXInterval, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setxAxisSlidingInterval(qint32)));
    connect(_pUi->spinYMin, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setyAxisMin(qint32)));
    connect(_pUi->spinYMax, SIGNAL(valueChanged(int)), _pGuiModel, SLOT(setyAxisMax(qint32)));

    //valueChanged is only send when done editing...
    _pUi->spinSlidingXInterval->setKeyboardTracking(false);
    _pUi->spinYMin->setKeyboardTracking(false);
    _pUi->spinYMax->setKeyboardTracking(false);

    // Create button group for X axis scaling options
    _pXAxisScaleGroup = new QButtonGroup();
    _pXAxisScaleGroup->setExclusive(true);
    _pXAxisScaleGroup->addButton(_pUi->radioXFullScale, BasicGraphView::SCALE_AUTO);
    _pXAxisScaleGroup->addButton(_pUi->radioXSliding, BasicGraphView::SCALE_SLIDING);
    _pXAxisScaleGroup->addButton(_pUi->radioXManual, BasicGraphView::SCALE_MANUAL);
    connect(_pXAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(xAxisScaleGroupClicked(int)));

    // Create button group for Y axis scaling options
    _pYAxisScaleGroup = new QButtonGroup();
    _pYAxisScaleGroup->setExclusive(true);
    _pYAxisScaleGroup->addButton(_pUi->radioYFullScale, BasicGraphView::SCALE_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYWindowScale, BasicGraphView::SCALE_WINDOW_AUTO);
    _pYAxisScaleGroup->addButton(_pUi->radioYMinMax, BasicGraphView::SCALE_MINMAX);
    _pYAxisScaleGroup->addButton(_pUi->radioYManual, BasicGraphView::SCALE_MANUAL);
    connect(_pYAxisScaleGroup, SIGNAL(buttonClicked(int)), this, SLOT(yAxisScaleGroupClicked(int)));

    /* handle focus change */
    connect(QApplication::instance(), SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(appFocusChanged(QWidget *, QWidget *)));

    /* Update notes in data file when requested by notes model */
    connect(_pNoteModel, SIGNAL(dataFileUpdateRequested()), this, SLOT(updateDataFileNotes()));

    // Default to full auto scaling
    _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
    _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);

    connect(_pConnMan, SIGNAL(handleReceivedData(QList<bool>, QList<double>)), _pGraphView, SLOT(plotResults(QList<bool>, QList<double>)));
    connect(_pConnMan, SIGNAL(handleReceivedData(QList<bool>, QList<double>)), _pLegend, SLOT(addLastReceivedDataToLegend(QList<bool>, QList<double>)));

    /* Update interface via model */
    _pGuiModel->triggerUpdate();
    _pSettingsModel->triggerUpdate();

    handleCommandLineArguments(cmdArguments);

#if 0
    //Debugging
    _pGraphDataModel->add();
    _pGraphDataModel->add();
    _pGraphDataModel->add();
    _pGraphDataModel->add();

    //_pGraphDataModel->setActive(2, false);

    _pSettingsModel->setPollTime(1000);

    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_0, "192.168.0.142");
    _pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_1, "192.168.0.142");

    //_pSettingsModel->setIpAddress(SettingsModel::CONNECTION_ID_0, "127.0.0.1");

    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_0, 5020);
    _pSettingsModel->setPort(SettingsModel::CONNECTION_ID_1, 5020);
#endif

}

MainWindow::~MainWindow()
{
    delete _pGraphView;
    delete _pConnectionDialog;
    delete _pConnMan;
    delete _pSettingsModel;
    delete _pGuiModel;
    delete _pGraphShowHide;
    delete _pGraphBringToFront;
    delete _pErrorLogModel;
    delete _pDataFileHandler;
    delete _pProjectFileHandler;

    delete _pUi;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier)
    {
        _pGuiModel->setCursorValues(true);
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        _pGuiModel->setCursorValues(false);
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (
            (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
            && (_pNoteModel->isNotesDataUpdated())
        )
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, windowTitle(),
                                                                    tr("The notes are changed.\nDo you want discard the changes or update (save) the data file?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::Discard | QMessageBox::Save,
                                                                    QMessageBox::Cancel);
        if (resBtn == QMessageBox::Cancel)
        {
            event->ignore();
        }
        else if (resBtn == QMessageBox::Discard)
        {
            event->accept();
        }
        else if (resBtn == QMessageBox::Save)
        {
            if (_pDataFileHandler->updateNoteLines(_pGuiModel->dataFilePath()))
            {
                event->accept();
            }
            else
            {
                event->ignore();
            }
        }
    }
    else
    {
        event->accept();
    }
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}

void MainWindow::selectImageExportFile()
{
    QString filePath;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::HideNameFilterDetails, false);
    dialog.setDefaultSuffix("png");
    dialog.setWindowTitle(tr("Select png file"));
    dialog.setNameFilter(tr("PNG files (*.png)"));
    dialog.setDirectory(_pGuiModel->lastDir());

    /* Add question wether to save when legend is undocked */
    bool bProceed = false;
    if (_pUi->legendDock->isFloating())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Save screenshot?", "The legend dock is floating, it won't be included in the screenshot. \n\nAre you sure want to proceed?", QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            bProceed = true;
        }
        else
        {
            bProceed = false;
        }
    }
    else
    {
        bProceed = true;
    }

    if (bProceed)
    {
        if (dialog.exec())
        {
            filePath = dialog.selectedFiles().first();
            _pGuiModel->setLastDir(QFileInfo(filePath).dir().absolutePath());

            QPixmap pixMap = this->window()->grab();
            pixMap.save(filePath);
        }
    }
}

void MainWindow::showAbout()
{
    AboutDialog aboutDialog(this);

    aboutDialog.exec();
}

void MainWindow::menuBringToFrontGraphClicked(bool bState)
{
    QAction * pAction = qobject_cast<QAction *>(QObject::sender());

    if (bState)
    {
        _pGuiModel->setFrontGraph(pAction->data().toInt());
    }
}

void MainWindow::menuShowHideGraphClicked(bool bState)
{
    QAction * pAction = qobject_cast<QAction *>(QObject::sender());

    const qint32 graphIdx = _pGraphDataModel->convertToGraphIndex(pAction->data().toInt());
    _pGraphDataModel->setVisible(graphIdx, bState);
}

void MainWindow::showConnectionDialog()
{
    _pConnectionDialog->exec();
}

void MainWindow::showLogSettingsDialog()
{
    _pLogDialog->exec();
}

void MainWindow::showRegisterDialog()
{
    showRegisterDialog(QString(""));
}

void MainWindow::showRegisterDialog(QString mbcFile)
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Clear data?", "An imported data file is loaded. Do you want to clear the data and start adding registers for a new log?", QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes)
        {
            return;
        }

        _pGraphDataModel->clear();
        _pNoteModel->clear();

        _pGuiModel->setGuiState(GuiModel::INIT);
    }

    RegisterDialog registerDialog(_pGuiModel, _pGraphDataModel, _pSettingsModel, this);

    if (mbcFile.isEmpty())
    {
        registerDialog.exec();
    }
    else
    {
        registerDialog.exec(mbcFile);
    }
}

void MainWindow::addNoteToGraph(void)
{
    Note newNote;

    bool ok;
    QString text = QInputDialog::getText(this, tr("Add note"),
                                         tr("Note Text:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok)
    {
        newNote.setKeyData(_pGraphView->pixelToKey(_lastRightClickPos.x()));
        newNote.setValueData(_pGraphView->pixelToValue(_lastRightClickPos.y()));
        newNote.setText(text);

        _pNoteModel->add(newNote);
    }
}

void MainWindow::clearData()
{
    _pConnMan->resetCommunicationStats();
    _pGraphView->clearResults();
    _pGuiModel->clearMarkersState();
    _pDataFileHandler->rewriteDataFile();
    _pNoteModel->clear();
    _pLegend->clearLegendData();
}

void MainWindow::startScope()
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        _pGraphDataModel->clear();
        _pNoteModel->clear();

        _pGuiModel->setGuiState(GuiModel::INIT);
    }

    if (_pGraphDataModel->activeCount() != 0)
    {

        _pGuiModel->setGuiState(GuiModel::STARTED);

        _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));

        if (_pConnMan->startCommunication())
        {
            clearData();
        }

        if (_pSettingsModel->writeDuringLog())
        {
            _pDataFileHandler->enableExporterDuringLog();
        }

        if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_MANUAL)
        {
            _pGuiModel->setxAxisScale(BasicGraphView::SCALE_AUTO);
        }

        if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_MANUAL)
        {
            _pGuiModel->setyAxisScale(BasicGraphView::SCALE_AUTO);
        }

    }
    else
    {
        Util::showError("There are no register in the scope list. Please select at least one register.");
    }
}

void MainWindow::stopScope()
{
    _pConnMan->stopCommunication();

    if (_pSettingsModel->writeDuringLog())
    {
        _pDataFileHandler->disableExporterDuringLog();
    }

    _pGuiModel->setGuiState(GuiModel::STOPPED);
}

void MainWindow::showErrorLog()
{
    _pErrorLogDialog->show();
}

void MainWindow::showNotesDialog()
{
    _pNotesDock->show();
}

void MainWindow::handleGraphVisibilityChange(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pGraphShowHide->actions().at(activeIdx)->setChecked(_pGraphDataModel->isVisible(graphIdx));

        // Show/Hide corresponding "BringToFront" action
        _pGraphBringToFront->actions().at(activeIdx)->setVisible(_pGraphDataModel->isVisible((graphIdx)));

        // Enable/Disable global BringToFront menu
        bool bVisible = false;
        foreach(QAction * pAction, _pGraphBringToFront->actions())
        {
            if (pAction->isVisible())
            {
                bVisible = true;
                break;
            }
        }
        _pGraphBringToFront->setEnabled(bVisible);
    }
}

void MainWindow::handleGraphColorChange(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        QPixmap pixmap(20,5);
        pixmap.fill(_pGraphDataModel->color(graphIdx));

        QIcon showHideIcon = QIcon(pixmap);

        _pGraphShowHide->actions().at(activeIdx)->setIcon(showHideIcon);
        _pGraphBringToFront->actions().at(activeIdx)->setIcon(showHideIcon);
    }
}

void MainWindow::handleGraphLabelChange(const quint32 graphIdx)
{
    if (_pGraphDataModel->isActive(graphIdx))
    {
        const quint32 activeIdx = _pGraphDataModel->convertToActiveGraphIndex(graphIdx);

        _pGraphShowHide->actions().at(activeIdx)->setText(_pGraphDataModel->label(graphIdx));
        _pGraphBringToFront->actions().at(activeIdx)->setText(_pGraphDataModel->label(graphIdx));
    }
}

void MainWindow::updateBringToFrontGrapMenu()
{
    if (_pBringToFrontGroup->actions().size() > 0)
    {
        _pBringToFrontGroup->actions().at(_pGuiModel->frontGraph())->setChecked(true);
    }
}

void MainWindow::updateHighlightSampleMenu()
{
    /* set menu to checked */
    _pUi->actionHighlightSamplePoints->setChecked(_pGuiModel->highlightSamples());
}

void MainWindow::rebuildGraphMenu()
{
    // Regenerate graph menu
    _pGraphShowHide->clear();
    _pGraphBringToFront->clear();

    QList<quint16> activeGraphList;
    _pGraphDataModel->activeGraphIndexList(&activeGraphList);

    for(qint32 activeIdx = 0; activeIdx < activeGraphList.size(); activeIdx++)
    {

        QString label = _pGraphDataModel->label(activeGraphList[activeIdx]);
        QAction * pShowHideAction = _pGraphShowHide->addAction(label);
        QAction * pBringToFront = _pGraphBringToFront->addAction(label);

        QPixmap pixmap(20,5);
        pixmap.fill(_pGraphDataModel->color(activeGraphList[activeIdx]));
        QIcon icon = QIcon(pixmap);

        pShowHideAction->setData(activeIdx);
        pShowHideAction->setIcon(icon);
        pShowHideAction->setCheckable(true);
        pShowHideAction->setChecked(_pGraphDataModel->isVisible(activeGraphList[activeIdx]));

        pBringToFront->setData(activeIdx);
        pBringToFront->setIcon(icon);
        pBringToFront->setCheckable(true);
        pBringToFront->setActionGroup(_pBringToFrontGroup);

        QObject::connect(pShowHideAction, SIGNAL(toggled(bool)), this, SLOT(menuShowHideGraphClicked(bool)));
        QObject::connect(pBringToFront, SIGNAL(toggled(bool)), this, SLOT(menuBringToFrontGraphClicked(bool)));
    }

    if (activeGraphList.size() > 0)
    {
        _pGraphShowHide->setEnabled(true);
        _pGraphBringToFront->setEnabled(true);
    }
    else
    {
        _pGraphShowHide->setEnabled(false);
        _pGraphBringToFront->setEnabled(false);
    }
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(_pGuiModel->windowTitle());
}

void MainWindow::updatexAxisSlidingMode()
{
    if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioXFullScale->setChecked(true);
    }
    else if (_pGuiModel->xAxisScalingMode() == BasicGraphView::SCALE_SLIDING)
    {
        // Sliding window
        _pUi->radioXSliding->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioXManual->setChecked(true);
    }
}

void MainWindow::updatexAxisSlidingInterval()
{
    _pUi->spinSlidingXInterval->setValue(_pGuiModel->xAxisSlidingSec());
}

void MainWindow::updateyAxisSlidingMode()
{
    if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_AUTO)
    {
        // Full auto scaling
        _pUi->radioYFullScale->setChecked(true);
    }
    else if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_MINMAX)
    {
        // Min and max selected
        _pUi->radioYMinMax->setChecked(true);
    }
    else if (_pGuiModel->yAxisScalingMode() == BasicGraphView::SCALE_WINDOW_AUTO)
    {
        // Window auto scale selected
        _pUi->radioYWindowScale->setChecked(true);
    }
    else
    {
        // manual
        _pUi->radioYManual->setChecked(true);
    }
}

void MainWindow::updateyAxisMinMax()
{
    _pUi->spinYMin->setValue(_pGuiModel->yAxisMin());
    _pUi->spinYMax->setValue(_pGuiModel->yAxisMax());
}

void MainWindow::updateGuiState()
{

    if (_pGuiModel->guiState() == GuiModel::INIT)
    {
        _pStatusState->setText(_cStateStopped);

        _pUi->actionStop->setEnabled(false);
        _pUi->actionConnectionSettings->setEnabled(true);
        _pUi->actionLogSettings->setEnabled(true);
        _pUi->actionRegisterSettings->setEnabled(true);
        _pUi->actionStart->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(true);
        _pUi->actionLoadProjectFile->setEnabled(true);
        _pUi->actionExportDataCsv->setEnabled(false);
        _pUi->actionExportImage->setEnabled(false);
        _pUi->actionExportSettings->setEnabled(true);

        _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));
        _pStatusRuntime->setVisible(true);

        _pStatusStats->setText(_cStatsTemplate.arg(0).arg(0));
        _pStatusRuntime->setVisible(true);

        _pGuiModel->setDataFilePath(QString(""));
        _pGuiModel->setProjectFilePath(QString(""));
    }
    else if (_pGuiModel->guiState() == GuiModel::STARTED)
    {
        // Communication active
        _pStatusState->setText(_cStateRunning);

        _pUi->actionStop->setEnabled(true);
        _pUi->actionConnectionSettings->setEnabled(false);
        _pUi->actionLogSettings->setEnabled(false);
        _pUi->actionRegisterSettings->setEnabled(false);
        _pUi->actionStart->setEnabled(false);
        _pUi->actionImportDataFile->setEnabled(false);
        _pUi->actionLoadProjectFile->setEnabled(false);
        _pUi->actionExportDataCsv->setEnabled(false);
        _pUi->actionExportSettings->setEnabled(false);
        _pUi->actionExportImage->setEnabled(false);
        _pUi->actionReloadProjectFile->setEnabled(false);

        _pStatusRuntime->setText(_cRuntime.arg("0 hours, 0 minutes 0 seconds"));
        _pStatusRuntime->setVisible(true);

        _pStatusStats->setText(_cStatsTemplate.arg(_pGuiModel->communicationSuccessCount()).arg(_pGuiModel->communicationErrorCount()));
        _pStatusRuntime->setVisible(true);
    }
    else if (_pGuiModel->guiState() == GuiModel::STOPPED)
    {
        _pStatusState->setText(_cStateStopped);

        // Communication not active
        _pUi->actionStop->setEnabled(false);
        _pUi->actionConnectionSettings->setEnabled(true);
        _pUi->actionLogSettings->setEnabled(true);
        _pUi->actionRegisterSettings->setEnabled(true);
        _pUi->actionStart->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(true);
        _pUi->actionLoadProjectFile->setEnabled(true);
        _pUi->actionExportDataCsv->setEnabled(true);
        _pUi->actionExportSettings->setEnabled(true);
        _pUi->actionExportImage->setEnabled(true);

        if (_pGuiModel->projectFilePath().isEmpty())
        {
            _pUi->actionReloadProjectFile->setEnabled(false);
        }
        else
        {
            _pUi->actionReloadProjectFile->setEnabled(true);
        }
    }
    else if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {

        _pStatusState->setText(_cStateDataLoaded);

        // Communication not active
        _pUi->actionStop->setEnabled(false);
        _pUi->actionConnectionSettings->setEnabled(true);
        _pUi->actionLogSettings->setEnabled(true);
        _pUi->actionRegisterSettings->setEnabled(true);
        _pUi->actionStart->setEnabled(true);
        _pUi->actionImportDataFile->setEnabled(true);
        _pUi->actionLoadProjectFile->setEnabled(true);
        _pUi->actionExportDataCsv->setEnabled(false); // Can't export data when viewing data
        _pUi->actionExportSettings->setEnabled(false); // Can't export data when viewing data
        _pUi->actionExportImage->setEnabled(true);

        _pStatusRuntime->setText(QString(""));
        _pStatusRuntime->setVisible(false);

        _pStatusStats->setText(QString(""));
        _pStatusStats->setVisible(false);

        _pUi->actionReloadProjectFile->setEnabled(false);
    }
}

void MainWindow::projectFileLoaded()
{
    // if a project file is previously loaded, then it can be reloaded
    if (_pGuiModel->projectFilePath().isEmpty())
    {
        _pGuiModel->setWindowTitleDetail("");
        _pUi->actionReloadProjectFile->setEnabled(false);
    }
    else
    {
        _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->projectFilePath()).fileName());
        _pUi->actionReloadProjectFile->setEnabled(true);
    }
}

void MainWindow::dataFileLoaded()
{
    if (_pGuiModel->dataFilePath().isEmpty())
    {
        _pGuiModel->setWindowTitleDetail("");
    }
    else
    {
        _pGuiModel->setWindowTitleDetail(QFileInfo(_pGuiModel->dataFilePath()).fileName());
    }
}

void MainWindow::updateStats()
{
    // Update statistics
    _pStatusStats->setText(_cStatsTemplate.arg(_pGuiModel->communicationSuccessCount()).arg(_pGuiModel->communicationErrorCount()));
}

void MainWindow::updateMarkerDockVisibility()
{
    if (_pGuiModel->markerState())
    {
        splitDockWidget(_pUi->legendDock, _pUi->markerInfoDock, Qt::Vertical);
    }

    _pUi->markerInfoDock->setVisible(_pGuiModel->markerState());
}

void MainWindow::scaleWidgetUndocked(bool bFloat)
{
    if (bFloat)
    {
        _pUi->scaleOptionsDock->adjustSize();
    }
}

void MainWindow::legendWidgetUndocked(bool bFloat)
{
    if (bFloat)
    {
        // TODO: fix scaling to minimum
        //_pUi->legendDock->adjustSize();
    }
}

void MainWindow::showContextMenu(const QPoint& pos)
{
    /* Don't show context menu when control key is pressed */
    if (!(QApplication::keyboardModifiers() & Qt::ControlModifier))
    {
        _lastRightClickPos = pos;
        _menuRightClick.popup(_pUi->customPlot->mapToGlobal(pos));
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *e)
{
    if (!_pConnMan->isActive())
    {
        const QString filename(e->mimeData()->urls().last().toLocalFile());
        QFileInfo fileInfo(filename);
        _pGuiModel->setLastDir(fileInfo.dir().absolutePath());
        if (fileInfo.completeSuffix().toLower() == QString("mbs"))
        {
            _pProjectFileHandler->loadProjectFile(filename);
        }
        else if (fileInfo.completeSuffix().toLower() == QString("csv"))
        {
            _pDataFileHandler->loadDataFile(filename);
        }
        else if (fileInfo.completeSuffix().toLower() == QString("mbc"))
        {
            showRegisterDialog(filename);
        }
        else
        {
            // ignore drop
        }
    }
}

void MainWindow::appFocusChanged(QWidget * old, QWidget * now)
{
    Q_UNUSED(now);
    if (old != NULL)
    {
        _pGuiModel->setCursorValues(false);
    }
}

void MainWindow::xAxisScaleGroupClicked(int id)
{
    _pGuiModel->setxAxisScale((BasicGraphView::AxisScaleOptions)id);
}

void MainWindow::yAxisScaleGroupClicked(int id)
{
    _pGuiModel->setyAxisScale((BasicGraphView::AxisScaleOptions)id) ;
}

void MainWindow::updateRuntime()
{
    qint64 timePassed = QDateTime::currentMSecsSinceEpoch() - _pGuiModel->communicationStartTime();

    // Convert to s
    timePassed /= 1000;

    const quint32 h = (timePassed / 3600);
    timePassed = timePassed % 3600;

    const quint32 m = (timePassed / 60);
    timePassed = timePassed % 60;

    const quint32 s = timePassed;

    QString strTimePassed = QString("%1 hours, %2 minutes %3 seconds").arg(h).arg(m).arg(s);

    _pStatusRuntime->setText(_cRuntime.arg(strTimePassed));

    // restart timer
    if (_pConnMan->isActive())
    {
        _runtimeTimer.singleShot(250, this, SLOT(updateRuntime()));
    }
}

void MainWindow::updateDataFileNotes()
{
    if (_pGuiModel->guiState() == GuiModel::DATA_LOADED)
    {
        if (_pNoteModel->isNotesDataUpdated())
        {
            _pDataFileHandler->updateNoteLines(_pGuiModel->dataFilePath());
        }
    }
}

void MainWindow::handleCommandLineArguments(QStringList cmdArguments)
{
    QCommandLineParser argumentParser;
    argumentParser.setApplicationDescription("Log data through the Modbus protocol");
    argumentParser.addHelpOption();

	// Project file option
    argumentParser.addPositionalArgument("project file", QCoreApplication::translate("main", "Project file (.mbs) to open"));

    // Process arguments
    argumentParser.process(cmdArguments);

    if (!argumentParser.positionalArguments().isEmpty())
    {
        QString filename = argumentParser.positionalArguments().first();
        QFileInfo fileInfo(filename);
        _pGuiModel->setLastDir(fileInfo.dir().absolutePath());
        _pProjectFileHandler->loadProjectFile(filename);
    }
}
