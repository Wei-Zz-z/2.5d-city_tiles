#include "input_osgbinput.h"
#include <QDebug>
#include <QFileInfo>
#include <QProgressDialog>
#include <QApplication>
#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>


InputOSGBInput::InputOSGBInput(QObject *parent) : QObject(parent){
}

//QList<QString> InputOSGBInput::geterrorfilename(){
//    return error_file_name;
//}

QList<double> InputOSGBInput::readOSGBBoundingBoxes(const QString& dataPath,QString srctype,QgsPointXY originpoint)
{
    QList<QList<QVariant>> resultList;
    QDir dataDir(dataPath);

    QList<double> globalBounds;
    globalBounds.clear();
    if (!dataDir.exists()) {
        qWarning() << "Data directory does not exist:" << dataPath;
        return globalBounds;
    }

    // 根据图片中的实际命名格式查找Tile文件夹
    QStringList tileFilters;
    tileFilters << "Tile_*";  // 匹配Tile_+000_+000等格式
    QFileInfoList tileDirs = dataDir.entryInfoList(tileFilters, QDir::Dirs | QDir::NoDotAndDotDot);

    qDebug() << "Found" << tileDirs.size() << "tile directories in:" << dataPath;

    // 处理每个Tile目录  
    // 创建进度对话框
    QProgressDialog progressDialog("正在处理文件夹...", "取消", 0, tileDirs.count());
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumDuration(0); // 立即显示
    progressDialog.setWindowTitle("处理进度");
    int progress = 0;

    foreach (const QFileInfo& tileDir, tileDirs) {
        // 检查是否取消
        if (progressDialog.wasCanceled()) {
            break;
        }

        // 更新进度文本
        progressDialog.setLabelText(QString("正在处理: %1").arg(tileDir.fileName()));
        progressDialog.setValue(progress);

        // 处理过程
        processTileDirectory(tileDir.absoluteFilePath());
        // 进度增加
        progress++;

        // 处理事件，确保UI更新
        QApplication::processEvents();
    }

    osg::ref_ptr<osg::Group> root = new osg::Group;

        for(const auto& path:fileList){
            if (osg::ref_ptr<osg::Node> tile = osgDB::readNodeFile(path.absoluteFilePath().toStdString())){
               root->addChild(tile);
            }
        }
        osg::ComputeBoundsVisitor boundsVisitor_all;
        root->accept(boundsVisitor_all);
        bbox_all.init();
        bbox_all = boundsVisitor_all.getBoundingBox();

        QgsPointXY point_all_min(bbox_all.xMin(),bbox_all.yMin());
        QgsPointXY point_all_max(bbox_all.xMax(),bbox_all.yMax());
        QgsPointXY Pointmin_WGS84_all = convertLocalToWGS84(point_all_min,originpoint);
        QgsPointXY Pointmax_WGS84_all = convertLocalToWGS84(point_all_max,originpoint);

        progressDialog.setValue(tileDirs.count());
        globalBounds <<Pointmin_WGS84_all.x()<<Pointmin_WGS84_all.y()<< Pointmax_WGS84_all.x() <<Pointmax_WGS84_all.y();
        return globalBounds;
}

void InputOSGBInput::processTileDirectory(const QString& tilePath)
{
    QList<QList<QVariant>> tileList;
        QString tileName = QFileInfo(tilePath).fileName();
        QDir tileDir(tilePath);
        if (!tileDir.exists()) {
            QMessageBox::information(nullptr, "error", "进程1处理失败");
            return;
        }
        // 查找所有.osgb文件
        QStringList filters;
        filters << "*.osgb";
        // 获取文件列表后过滤掉包含层级信息的文件
        QFileInfoList fileList_origin = tileDir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo &fileInfo : fileList_origin) {
            QString fileName = fileInfo.fileName();
            if (!fileName.contains("_L")) {
                fileList.append(fileInfo);
            }
        }
        qDebug() << "Processing tile:" << tileName << "with" << fileList.size() << "OSGB files";
}

double InputOSGBInput::get_height_of_osgb(){
    return  bbox_all.center().z();
}



QgsPointXY InputOSGBInput::convertLocalToWGS84(const QgsPointXY& localPoint,const QgsPointXY& originWGS84)
{
    try {
        // 获取原点纬度（用于计算经度方向的比例）
        double originLat = originWGS84.y();


        // 计算每度的米数（近似值）
        // 纬度方向：1度 ≈ 111,000米
        // 经度方向：1度 ≈ 111,000 * cos(纬度)米
        const double metersPerDegreeLat = 111000.0;
        double metersPerDegreeLon = 111000.0 * std::cos(originLat * M_PI / 180.0);

        // 计算经纬度偏移量
        double deltaLon = localPoint.x() / metersPerDegreeLon;
        qDebug()<<localPoint.x();
        double deltaLat = localPoint.y() / metersPerDegreeLat;
        qDebug()<<localPoint.y();

        // 创建临时点
        QgsPointXY tempPoint(
            originWGS84.y() + deltaLat,
            originWGS84.x() + deltaLon
        );

        // 创建坐标转换对象（从WGS84到WGS84，用于精确计算）
        QgsCoordinateTransform transform(
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsProject::instance()
        );

        // 使用转换对象进行精确计算
        return transform.transform(tempPoint);

    } catch (const QgsCsException &e) {
        // 处理异常情况
        qWarning() << "坐标转换失败:" << e.what();
        return QgsPointXY(); // 返回无效点
    }
}


