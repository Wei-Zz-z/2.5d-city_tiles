#include "mainwindow.h"
#include "txtexport.h"
#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgsmaplayer.h>
#include <QToolBar>
#include <QAction>
#include <QStatusBar>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDebug>
#include "widget.h"
#include "reader_xml.h"
#include "input_osgbinput.h"
// 修改最大网格等级为10

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 初始化模块
    m_gridManager = new GridManager(this);
    m_gridRenderer = new GridRenderer(this);
    m_osgbMapper = new OSGBMapper(this);
    m_shapefileExporter = new ShapefileExporter(this);
    m_scaleCalculator = new ScaleCalculator(this);
    m_coordinateConverter = new CoordinateConverter(this);
    openmappiing = new OpenMapping(fitlevel);
    open3Drender = nullptr;
    // 初始化状态变量
    m_isIndexBuilt = false;
    m_currentGridLevel = -1;
    fit_level_mapping = -1;
    mapping_state = -1;

    // 设置UI
    setupUI();
    // 连接信号和槽
    connect(m_gridManager, &GridManager::gridCreated, this, &MainWindow::onGridCreated);
    connect(m_gridManager, &GridManager::gridLevelChanged, this, &MainWindow::onGridLevelChanged);
    connect(m_shapefileExporter, &ShapefileExporter::exportFinished,
            this, &MainWindow::onExportFinished);
    // 创建全局网格
    m_gridManager->createGlobalGrid();
    QgsCoordinateTransform transform(
        QgsCoordinateReferenceSystem("EPSG:3857"), // 源坐标系：Web 墨卡托
        QgsCoordinateReferenceSystem("EPSG:4326"), // 目标坐标系：WGS84
        QgsProject::instance()
    );

    // 初始化LOD更新定时器
    m_lodUpdateTimer = new QTimer(this);
    m_lodUpdateTimer->setInterval(500); // 每500毫秒检查一次
    connect(m_lodUpdateTimer, &QTimer::timeout, this, &MainWindow::updateGlobalGridLOD);
    m_lodUpdateTimer->start();
}
void MainWindow::exporttile(){
    QMessageBox::information(nullptr, "开始", "开始匹配网格");
    QString folderPath = QFileDialog::getExistingDirectory(this,tr("Select Input Folder"));
    m_eport = new Eport(this,xml_result[3].toDouble(),xml_result[4].toDouble(),folderPath);
    if(m_userExtentWGS84.isEmpty()){
    QMessageBox::warning(this, "Export Grid Tiles", "未设置用户范围" );
    return;
    }
    if(mapping_state == 0){
        QMap <int,int> m_maprule_eport = openmappiing->getLevelMapping();
        int user_define_start = m_maprule_eport.firstKey();
        int user_define_target = m_maprule_eport.lastKey();
        //     QString shp = QFileDialog::getOpenFileName(this, "Open Grid Shapefile", QString(), "Shapefile (*.shp)");
        m_osgbMapper->updateMappingFromCurrentLevel(fitlevel, m_targetLevel,mapping_state,m_maprule_eport);
        qDebug()<<"muqiandewanggezhuangtai"<<mapping_state;
        m_eport->exportGridTilesToTxt(m_gridManager,m_osgbMapper,user_define_start,user_define_target);
        fileinfo = m_eport->getAllGridTiles();
    }else{
        m_osgbMapper->updateMappingFromCurrentLevel(fitlevel, m_targetLevel,mapping_state);
        qDebug()<<"muqiandewanggezhuangtai"<<mapping_state;
        m_eport->exportGridTilesToTxt(m_gridManager,m_osgbMapper,fitlevel,m_targetLevel);
        fileinfo = m_eport->getAllGridTiles();
    }
}

QList<QVariant> MainWindow::reader_xml(){
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("XML Files (*.xml);;All Files (*)")
    );
    if(fileName.isEmpty())
    {
        return xml_result;
    }
    xml_result = readerxml.readXMLFile(fileName);
    QString message = QString(
                "原点读取完成！\n\n"
                "Lon: %1\n"
                "Lat: %2\n")
                .arg(xml_result[3].toDouble())
                .arg(xml_result[4].toDouble());
    QMessageBox::information(nullptr, "完成", message);
    return xml_result;
}
QList<double> MainWindow::reader_osgb(){
    inputtype = "default";
    if(xml_result.isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("Please read XML file first!"));
        return QList<double>();
    }
    QString folderPath = QFileDialog::getExistingDirectory(this,tr("Select Input Folder"));

    QgsPointXY originpoint(xml_result[3].toDouble(),xml_result[4].toDouble());
    QDateTime startTime = QDateTime::currentDateTime();
    globalBounds = inputosgbinput.readOSGBBoundingBoxes(folderPath,xml_result[2].toString(),originpoint);
    inputtype = "osgb";
    QDateTime endTime = QDateTime::currentDateTime();
    // 计算耗时
    qint64 duration = startTime.secsTo(endTime);
    QString message = QString(
                "范围输入完成！\n\n"
                "minX: %1\n"
                "minY: %2\n"
                "maxX: %3\n"
                "maxY: %4\n"
                "开始时间: %5\n"
                "结束时间: %6\n"
                "耗时: %7 秒")
                .arg(globalBounds[0])
                .arg(globalBounds[1])
                .arg(globalBounds[2])
                .arg(globalBounds[3])
                .arg(startTime.toString("yyyy-MM-dd hh:mm:ss"))
                .arg(endTime.toString("yyyy-MM-dd hh:mm:ss"))
                .arg(duration);
    QMessageBox::information(nullptr, "完成", message);
    return globalBounds;
}

QgsRectangle MainWindow::inputnumber(){
    inputtype = "default";
    numberextent = m_coordinateConverter->getUserExtentFromDialog(this);
    return numberextent;
}

void MainWindow::setOsgbLevelRange()
{
    QDialog dialog(this);
    dialog.setWindowTitle("设置OSGB层级范围");
    dialog.resize(400, 150);

    QFormLayout *form = new QFormLayout(&dialog);

    QSpinBox *minLevelSpin = new QSpinBox(&dialog);
    minLevelSpin->setRange(0, 30); // 合理的OSGB层级范围
    minLevelSpin->setValue(m_osgbMapper->minOsgbLevel());
    form->addRow("最小OSGB层级:", minLevelSpin);

    QSpinBox *maxLevelSpin = new QSpinBox(&dialog);
    maxLevelSpin->setRange(0, 30);
    maxLevelSpin->setValue(m_osgbMapper->maxOsgbLevel());
    form->addRow("最大OSGB层级:", maxLevelSpin);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    // 更新OSGB层级范围
    m_osgbMapper->setOsgbLevelRange(minLevelSpin->value(), maxLevelSpin->value());

    QMessageBox::information(this, "设置成功",
        QString("OSGB层级范围已设置为: %1-%2").arg(minLevelSpin->value()).arg(maxLevelSpin->value()));
}

void MainWindow::on_mapping_clicked(){
    for_getextent();
    if(m_userExtentWGS84.isEmpty()){
        fitlevel = -1;
        qDebug()<<"-----------------------------"<<fitlevel;
    }else{
        fitlevel = for_getextent()[0];
        qDebug()<<"-----------------------------"<<fitlevel;
    }
    if(!openmappiing) {
        openmappiing = new OpenMapping(fitlevel);
    }else{
        delete openmappiing;
        openmappiing = new OpenMapping(fitlevel);
    }
    openmappiing->show();
    openmappiing->activateWindow();
}

void MainWindow::on_open3Drender_clicked(){

    osg::BoundingBox CityBox = inputosgbinput.getCityBox();

    if(!open3Drender) {
        open3Drender = new Widget(CityBox, xml_result[3].toDouble(),xml_result[4].toDouble(),fileinfo);
    }else{
        delete open3Drender;
        open3Drender = new Widget(CityBox, xml_result[3].toDouble(),xml_result[4].toDouble(),fileinfo);
    }
    open3Drender->show();
    open3Drender->activateWindow();
}

void MainWindow::onGridLevelChanged(int newLevel) //已检查
{
    m_currentGridLevel = newLevel;
    qDebug() << "网格层级已切换到:" << newLevel;

    // 更新状态栏显示
    m_scaleLabel->setText(QString("当前层级: %1").arg(newLevel));
}
MainWindow::~MainWindow()  //已检查
{
    // 清理定时器
    if (m_lodUpdateTimer) {
        m_lodUpdateTimer->stop();
        delete m_lodUpdateTimer;
    }
    if(open3Drender) delete open3Drender;
}


void MainWindow::setupUI()  //已检查
{
    setWindowTitle("Shapefile Spatial Index");
    resize(1200, 800);

    // 创建地图画布
    setupMapCanvas();

    // 创建工具栏
    setupToolBar();

    // 创建状态栏和比例尺显示
    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);

    m_scaleLabel = new QLabel("比例尺: 1:0", this);
    m_statusBar->addPermanentWidget(m_scaleLabel); // 永久部件，位于右侧

    // 连接地图画布缩放信号
    connect(m_mapCanvas, &QgsMapCanvas::extentsChanged, this, &MainWindow::updateScaleFromExtent);
}

void MainWindow::setupMapCanvas() //已检查
{
    // 创建地图画布
    m_mapCanvas = new QgsMapCanvas(this);
    m_mapCanvas->setCanvasColor(Qt::white);
    setCentralWidget(m_mapCanvas);

    // 设置初始范围（全球范围，Web墨卡托）
    QgsRectangle globalExtent(-20037508.3427892, -20037508.3427892,
                             20037508.3427892, 20037508.3427892);
    m_mapCanvas->setExtent(globalExtent);
    m_mapCanvas->refresh();
}

void MainWindow::setupToolBar()  //已检查
{
    // 创建工具栏
    m_toolBar = addToolBar("Tools");

    // 添加工具栏动作
    m_toolBar->addAction("读取xml文件信息",this,&MainWindow::reader_xml);
    m_toolBar->addAction("读取osgb文件范围",this,&MainWindow::reader_osgb);
    m_toolBar->addAction("输入范围",this,&MainWindow::inputnumber);
    m_toolBar->addAction("划分网格", this, &MainWindow::createGridFromExtent);
    m_toolBar->addAction("设置OSGB层级", this, &MainWindow::setOsgbLevelRange);
    m_toolBar->addAction("自定义osgb层级匹配规则",this,&MainWindow::on_mapping_clicked);
    m_toolBar->addAction("匹配范围", this, &MainWindow::exporttile);
    m_toolBar->addAction("三维图片导出",this,&MainWindow::on_open3Drender_clicked);

}

QList<int> MainWindow::for_getextent(){
    QgsRectangle wgs84Extent_for_extent;
    QList<int> all_level;
    // 使用CoordinateConverter获取用户范围
    if(inputtype == "osgb"){
        qDebug()<<globalBounds;
        wgs84Extent_for_extent = m_coordinateConverter->convertListToWebMercator(globalBounds);
    }else{
        if (numberextent.isEmpty()) {
            qDebug()<<"numberextent为空";
            return all_level;
        }
        wgs84Extent_for_extent = numberextent;}
    if (wgs84Extent_for_extent.isEmpty()) {
        qDebug()<<"wgs84extent为空";
        return all_level;
    }

    // 保存WGS84范围
    m_userExtentWGS84 = wgs84Extent_for_extent;

    // 转换为Web墨卡托
    m_userExtentWebMercator = m_coordinateConverter->transformToWebMercator(wgs84Extent_for_extent);

    // 计算最优比例尺
    double optimalScale = m_scaleCalculator->calculateOptimalScaleForExtent(
        wgs84Extent_for_extent, m_mapCanvas->size());

    if (optimalScale <= 0) {
        showError("无法计算适宜的比例尺");
        return all_level;
    }

    // 根据比例尺计算合适的网格等级
    int optimalLevel = m_scaleCalculator->calculateGridLevelFromScale(optimalScale);
    fitlevel = optimalLevel;
    qDebug() << "计算出的最优比例尺:" << optimalScale;
    qDebug() << "对应的网格层级:" << optimalLevel;
    m_targetLevel = qMin(optimalLevel + 6, AppConstants::MAX_ZOOM_LEVEL);
    qDebug() << "目标层级:" << m_targetLevel;
    all_level.append(fitlevel);
    all_level.append(m_targetLevel);
    return all_level;
}

void MainWindow::createGridFromExtent()
{
    qDebug()<<"11111111111111111111111111111111111111";
    QMap <int,int> m_maprule = openmappiing->getLevelMapping();
    qDebug()<<"22222222222222222222222222222222222222"<<m_maprule[18];
    mapping_state = openmappiing->getmapstate();
    qDebug()<<"__________________________"<<mapping_state;
    int optimalLevel = for_getextent()[0];
    // 如果计算出的层级大于预生成的最大层级，使用预生成的最大层级
    if (optimalLevel > AppConstants::PRE_GENERATED_MAX_LEVEL) {
        optimalLevel = AppConstants::PRE_GENERATED_MAX_LEVEL;
        qDebug() << "调整最优层级为预生成最大层级:" << optimalLevel;
    }
        // 更新网格与用户范围的相交关系
    qDebug() << "相较关系前";

    qDebug() << "相较关系前";
    if (optimalLevel < AppConstants::PRE_GENERATED_MAX_LEVEL) {
        qDebug() << "最适宜等级小于10级，使用预生成网格";
        m_gridManager->updateGridWithUserExtent(m_userExtentWebMercator);
        if(mapping_state == -1){
            m_osgbMapper->updateMappingFromCurrentLevel(optimalLevel, AppConstants::PRE_GENERATED_MAX_LEVEL,mapping_state);
        }else{
            m_osgbMapper->updateMappingFromCurrentLevel(optimalLevel, AppConstants::PRE_GENERATED_MAX_LEVEL,mapping_state,m_maprule);
        }

        // 对于小于10级的情况，直接使用预生成网格
        // 不需要生成动态网格
    } else {
        m_gridManager->updateGridWithUserExtent_largelevel(m_userExtentWebMercator);
        qDebug() << "最适宜等级大于等于10级，使用动态生成网格";
        // 对于大于等于10级的情况，使用动态生成网格
        // 确定动态生成的起始层级
        int dynamicStartLevel = optimalLevel;
        if (optimalLevel > AppConstants::PRE_GENERATED_MAX_LEVEL) {
            dynamicStartLevel = AppConstants::PRE_GENERATED_MAX_LEVEL;
            qDebug() << "适宜层级大于预生成最大层级，调整动态生成起始层级为:" << dynamicStartLevel;
        }

        // 生成动态网格（从起始层级到目标层级）
       m_gridManager->generateDynamicGridsFromOptimalLevel(optimalLevel, m_userExtentWebMercator,m_targetLevel);
       if(mapping_state == -1){
           m_osgbMapper->updateMappingFromCurrentLevel(optimalLevel,m_targetLevel,mapping_state);
       }else{
           m_osgbMapper->updateMappingFromCurrentLevel(optimalLevel,m_targetLevel,mapping_state,m_maprule);
       }
    }
    // 显示成功消息
    QMessageBox::information(this, "范围设置成功",
        QString("已设置用户范围\n覆盖范围: %1°E, %2°N 到 %3°E, %4°N")
        .arg(m_userExtentWGS84.xMinimum(), 0, 'f', 6)
        .arg(m_userExtentWGS84.yMinimum(), 0, 'f', 6)
        .arg(m_userExtentWGS84.xMaximum(), 0, 'f', 6)
        .arg(m_userExtentWGS84.yMaximum(), 0, 'f', 6));

//    // 自动保存网格
    saveGridAsShapefile();
}

void MainWindow::saveGridAsShapefile()
{
    // 检查是否有用户范围
    if (m_userExtentWebMercator.isEmpty()) {
        showError("没有设置用户范围，请先使用'指定范围划分网格'功能设置范围");
        return;
    }

    // 计算最适宜层级
    double optimalScale = m_scaleCalculator->calculateOptimalScaleForExtent(
        m_userExtentWGS84, m_mapCanvas->size());

    if (optimalScale <= 0) {
        showError("无法计算适宜的比例尺");
        return;
    }

    int optimalLevel = m_scaleCalculator->calculateGridLevelFromScale(optimalScale);

    qDebug() << "保存时所用的等级为" << optimalLevel;
    // 获取保存路径
    QString savePath = QFileDialog::getSaveFileName(
        this, "保存网格数据", "", "Shapefile (*.shp)");

    if (savePath.isEmpty()) {
        return;
    }
   qDebug() << "保存时的目标层级:" << m_targetLevel;
    // 保存网格
   if (optimalLevel < 10) {
       m_shapefileExporter->savePreGeneratedGrid(
           m_gridManager, m_osgbMapper, m_userExtentWebMercator, optimalLevel, AppConstants::PRE_GENERATED_MAX_LEVEL, savePath);

   } else {
       m_shapefileExporter->saveGridFromLevel(
           m_gridManager, m_osgbMapper, m_userExtentWebMercator, optimalLevel, savePath,m_targetLevel);
   }

}


void MainWindow::updateScaleFromExtent()
{
    // 计算当前比例尺分母
    double scaleDenominator = m_scaleCalculator->calculateCurrentScaleDenominator(m_mapCanvas);

    if (scaleDenominator <= 0) {
        m_scaleLabel->setText("比例尺: 未知");
        return;
    }

    qDebug() << "比例尺分母:" << scaleDenominator;

    // 计算网格等级
    int newGridLevel = m_scaleCalculator->calculateGridLevelFromScale(scaleDenominator);

    // 如果网格等级发生变化，更新OSGB映射
    if (newGridLevel != m_currentGridLevel) {
        m_currentGridLevel = newGridLevel;
        qDebug() << "网格等级变化为:" << m_currentGridLevel;

        // 更新映射关系
        m_osgbMapper->updateMappingFromCurrentLevel(m_currentGridLevel,AppConstants::PRE_GENERATED_MAX_LEVEL,-1);

        // 更新网格层级 - 这是关键修复
        m_gridManager->changeGridLevel(m_currentGridLevel);
    }

    // 格式化比例尺显示
    if (scaleDenominator > 1000000) {
        m_scaleLabel->setText(QString("比例尺: 1:%1百万 (层级: %2)")
                             .arg(static_cast<int>(scaleDenominator / 1000000))
                             .arg(m_currentGridLevel));
    } else if (scaleDenominator > 1000) {
        m_scaleLabel->setText(QString("比例尺: 1:%1千 (层级: %2)")
                             .arg(static_cast<int>(scaleDenominator / 1000))
                             .arg(m_currentGridLevel));
    } else {
        m_scaleLabel->setText(QString("比例尺: 1:%1 (层级: %2)")
                             .arg(static_cast<int>(scaleDenominator))
                             .arg(m_currentGridLevel));
    }
}


void MainWindow::updateGlobalGridLOD()
{
    // 如果当前显示的是全球网格，更新LOD
    if (m_gridManager->isGlobalGridCreated()) {
        // 计算当前比例尺分母
        double scaleDenominator = m_scaleCalculator->calculateCurrentScaleDenominator(m_mapCanvas);

        if (scaleDenominator <= 0) {
            return;
        }

        qDebug() << "全球网格LOD比例尺分母:" << scaleDenominator;

        // 计算应该显示的网格层级
        int newLevel = m_scaleCalculator->calculateGridLevelFromScale(scaleDenominator);
        qDebug() << "计算出的全球网格层级:" << newLevel;

        // 如果层级发生变化，切换到新层级
        if (newLevel != m_currentGridLevel) {
            qDebug() << "比例尺变化，从层级" << m_currentGridLevel << "切换到层级" << newLevel;
            m_gridManager->changeGridLevel(newLevel);
        }
    }
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
}

void MainWindow::onGridCreated()
{
    // 网格创建完成后，将其添加到地图画布
    if (m_gridManager->gridLayer()) {
        QList<QgsMapLayer*> layers;
        layers.append(m_gridManager->gridLayer());
        m_mapCanvas->setLayers(layers);
        m_mapCanvas->refresh();

        qDebug() << "网格图层已添加到地图画布";
    }
}

void MainWindow::onExportFinished(bool success, const QString &message)
{
    if (success) {
        QMessageBox::information(this, "导出完成", message);
    } else {
        QMessageBox::warning(this, "导出失败", message);
    }
}
