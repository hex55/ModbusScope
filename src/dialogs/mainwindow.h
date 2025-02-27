#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QTimer>
#include <QMenu>

#include "legend.h"
#include "projectfileparser.h"


namespace Ui {
class MainWindow;
}

// Forward declaration
class CommunicationManager;
class QCustomPlot;
class GraphDataModel;
class NoteModel;
class ErrorLogModel;
class ConnectionDialog;
class SettingsModel;
class LogDialog;
class ErrorLogDialog;
class NotesDock;
class GuiModel;
class ExtendedGraphView;
class MarkerInfo;
class DataFileHandler;
class ProjectFileHandler;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QStringList cmdArguments, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void closeEvent(QCloseEvent* event);

signals:

    void stopThread();
    void startModbus();
    void stopModbus();
    void registerStateChange(quint16 registerAddress);

private slots:

    /* Menu handlers */
    void exitApplication();
    void selectImageExportFile();
    void showAbout();
    void menuBringToFrontGraphClicked(bool bState);
    void menuShowHideGraphClicked(bool bState);
    void showConnectionDialog();
    void showLogSettingsDialog();
    void showRegisterDialog();
    void showRegisterDialog(QString mbcFile);
    void addNoteToGraph(void);
    void clearData();
    void startScope();
    void stopScope();
    void showErrorLog();
    void showNotesDialog();

    /* Model change handlers */
    void handleGraphVisibilityChange(const quint32 graphIdx);
    void handleGraphColorChange(const quint32 graphIdx);
    void handleGraphLabelChange(const quint32 graphIdx);

    void updateBringToFrontGrapMenu();
    void updateHighlightSampleMenu();
    void rebuildGraphMenu();
    void updateWindowTitle();
    void updatexAxisSlidingMode();
    void updatexAxisSlidingInterval();
    void updateyAxisSlidingMode();
    void updateyAxisMinMax();
    void projectFileLoaded();
    void dataFileLoaded();
    void updateGuiState();
    void updateStats();
    void updateMarkerDockVisibility();

    /* Misc */
    void scaleWidgetUndocked(bool bFloat);
    void legendWidgetUndocked(bool bFloat);
    void showContextMenu(const QPoint& pos);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void appFocusChanged(QWidget * old, QWidget * now);
    void xAxisScaleGroupClicked(int id);
    void yAxisScaleGroupClicked(int id);
    void updateRuntime();
    void updateDataFileNotes();

private:

    void handleCommandLineArguments(QStringList cmdArguments);

    Ui::MainWindow * _pUi;
    CommunicationManager * _pConnMan;
    ExtendedGraphView * _pGraphView;

    SettingsModel * _pSettingsModel;
    GraphDataModel * _pGraphDataModel;
    NoteModel * _pNoteModel;
    ErrorLogModel * _pErrorLogModel;
    GuiModel * _pGuiModel;

    ConnectionDialog * _pConnectionDialog;
    LogDialog * _pLogDialog;
    ErrorLogDialog * _pErrorLogDialog;

    DataFileHandler* _pDataFileHandler;
    ProjectFileHandler* _pProjectFileHandler;

    NotesDock * _pNotesDock;
    MarkerInfo * _pMarkerInfo;
    Legend * _pLegend;

    QLabel * _pStatusStats;
    QLabel * _pStatusState;
    QLabel * _pStatusRuntime;
    QButtonGroup * _pXAxisScaleGroup;
    QButtonGroup * _pYAxisScaleGroup;

    QTimer _runtimeTimer;

    QMenu _menuRightClick;

    QMenu * _pGraphBringToFront;
    QMenu * _pGraphShowHide;
    QActionGroup * _pBringToFrontGroup;

    QPoint _lastRightClickPos;

    static const QString _cStateRunning;
    static const QString _cStateStopped;
    static const QString _cStatsTemplate;
    static const QString _cStateDataLoaded;
    static const QString _cRuntime;
};

#endif // MAINWINDOW_H
