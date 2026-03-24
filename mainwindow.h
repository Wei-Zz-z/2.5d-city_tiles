#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMap>
#include <QSet>
#include "gridmanager.h"
#include "gridrenderer.h"
#include "osgbmapper.h"
#include "shapefileexporter.h"
#include "scalecalculator.h"
#include "coordinateconverter.h"
#include "appconstants.h"
#include "txtexport.h"
#include "widget.h"
#include "reader_xml.h"
#include "input_osgbinput.h"
#include "openmapping.h"
class QgsMapCanvas;
class QStatusBar;
class QLabel;
class QToolBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void onGridLevelChanged(int newLevel);
    // 工具栏动作槽函数
    void createGridFromExtent();
    void saveGridAsShapefile();
    void setOsgbLevelRange();
    void on_open3Drender_clicked();
    void on_mapping_clicked();

    // 其他槽函数
    void updateScaleFromExtent();
    void updateGlobalGridLOD();
    void showError(const QString &message);

    // 新增：处理网格创建完成信号
    void onGridCreated();
    // 新增：处理导出完成信号
    void onExportFinished(bool success, const QString &message);

private:
//    void print(GridManager *gridManager, OSGBMapper *osgbMapper,
//               const QgsRectangle &userExtent);
    // UI设置
    void setupUI();
    void setupToolBar();

    // 新增：初始化地图画布
    void setupMapCanvas();
    void exporttile();
    QList<int> for_getextent();
    ReaderXML readerxml;
    QList<QVariant> reader_xml();
    QList<QVariant> xml_result;
    InputOSGBInput inputosgbinput;
    QList<double> reader_osgb();
    QList<double> globalBounds;
    QString inputtype ="default";
    QgsRectangle inputnumber();
    QgsRectangle numberextent;

    // 模块实例
    GridManager* m_gridManager;
    GridRenderer* m_gridRenderer;
    OSGBMapper* m_osgbMapper;
    ShapefileExporter* m_shapefileExporter;
    ScaleCalculator* m_scaleCalculator;
    CoordinateConverter* m_coordinateConverter;
    Eport * m_eport;

    // UI组件
    QgsMapCanvas* m_mapCanvas;
    QStatusBar* m_statusBar;
    QLabel* m_scaleLabel;
    QToolBar* m_toolBar;
    OpenMapping * openmappiing;
    Widget * open3Drender;

    // 定时器
    QTimer* m_lodUpdateTimer;

    // 状态变量
    int m_targetLevel = 18;
    bool m_isIndexBuilt;
    int m_currentGridLevel;
    QgsRectangle m_userExtentWebMercator;
    int fitlevel;
    QMap<int, GridTileInfo> fileinfo;
    int fit_level_mapping;
    int mapping_state;
    // 新增：保存当前用户范围（WGS84）
    QgsRectangle m_userExtentWGS84;
};

#endif // MAINWINDOW_H
