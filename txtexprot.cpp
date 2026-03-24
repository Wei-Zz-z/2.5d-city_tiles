//#ifdef USE_GDAL
//#include <gdal.h>
//#include <ogr_api.h>
//#include <ogrsf_frmts.h>
//#endif
//#include "txtexport.h"
//#include <QVBoxLayout>
//#include <QHBoxLayout>
//#include <QPushButton>
//#include <QFileDialog>
//#include <QMessageBox>
//#include <QProgressDialog>
//#include <QTextStream>
//#include <QFile>
//#include <QFileInfo>
//#include <QApplication>
//#include <QDebug>
//#include "gridmanager.h"
//#include <qgsproject.h>


//QMap<int, GridTileInfo> g_gridTilesMap;

//Eport::Eport(QWidget *parent)
//    : QWidget(parent)
//{
//    QString rootDir = "D:/baoli_OSGB_8CM/Production_3/Data";
//    dataRootDir = rootDir;
//    TileCatalog::scan(rootDir, tileIndex);
//}

//QStringList Eport::getFilesByFid(int fid)
//{
//    if (g_gridTilesMap.contains(fid)) {
//        return g_gridTilesMap[fid].files;
//    }
//    return QStringList(); // 返回空列表表示未找到
//}
//QMap<int, GridTileInfo> Eport::getAllGridTiles()
//{
//    return g_gridTilesMap;
//}
//void Eport::clearGridTilesData()
//{
//    g_gridTilesMap.clear();
//}
//Eport::~Eport() { }

//void Eport::exportGridTilesToTxt(GridManager * g_gridManager,OSGBMapper* m_osgbMapper,int startlevel,int targetlevel)
//{
//#ifdef USE_GDAL

//    g_gridTilesMap.clear();

//    // 设置 PROJ 数据路径（如果尚未设置）
//    if(getenv("PROJ_LIB") == nullptr) {
//        QString projDataPath = "D:/osgeo4w64/OSGeo4W64/share/proj";
//        qputenv("PROJ_LIB", projDataPath.toUtf8());
//        qDebug() << "Set PROJ_LIB to:" << projDataPath;
//    }

//    // 注册所有GDAL驱动
//    GDALAllRegister();

//    if(!g_gridManager) {
//        QMessageBox::warning(this, "Export Grid Tiles", "GridManager is not available");
//        return;
//    }
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    // 打开Shapefile
////    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(shpPath.toUtf8().constData(), GDAL_OF_VECTOR, NULL, NULL, NULL);
////    if(!poDS) {
////        QMessageBox::warning(this, "Export Grid Tiles", "Failed to open shapefile: " + shpPath);
////        return;
////    }

////    // 获取第一个图层
////    OGRLayer* poLayer = poDS->GetLayer(0);
////    if(!poLayer) {
////        GDALClose(poDS);
////        QMessageBox::warning(this, "Export Grid Tiles", "No layers found in shapefile");
////        return;
////    }

////    // 获取字段信息
////    OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();

////    // 获取字段索引
////    int fidFieldIndex = -1;
////    int zoomFieldIndex = -1;
////    int rowFieldIndex = -1;
////    int colFieldIndex = -1;
////    int widthMFieldIndex = -1;
////    int heightMFieldIndex = -1;
////    int levelFieldIndex = -1;
////    int minXFieldIndex = -1;
////    int minYFieldIndex = -1;
////    int maxXFieldIndex = -1;
////    int maxYFieldIndex = -1;

////    for(int i = 0; i < poFDefn->GetFieldCount(); i++) {
////        QString fieldName = QString(poFDefn->GetFieldDefn(i)->GetNameRef()).toLower();

////        if(fieldName == "fid") fidFieldIndex = i;
////        else if(fieldName == "zoom") zoomFieldIndex = i;
////        else if(fieldName == "row") rowFieldIndex = i;
////        else if(fieldName == "col") colFieldIndex = i;
////        else if(fieldName == "width_m") widthMFieldIndex = i;
////        else if(fieldName == "height_m") heightMFieldIndex = i;
////        else if(fieldName == "osgb_level") levelFieldIndex = i;
////        else if(fieldName == "min_x") minXFieldIndex = i;
////        else if(fieldName == "min_y") minYFieldIndex = i;
////        else if(fieldName == "max_x") maxXFieldIndex = i;
////        else if(fieldName == "max_y") maxYFieldIndex = i;
////    }

////    // 检查必需字段
////    if(levelFieldIndex == -1) {
////        QMessageBox::warning(this, "Export Grid Tiles",
////            "No 'osgb_level' field found in shapefile. Available fields:\n" +
////            [poFDefn]() {
////                QStringList fields;
////                for(int i = 0; i < poFDefn->GetFieldCount(); i++) {
////                    fields << QString(poFDefn->GetFieldDefn(i)->GetNameRef());
////                }
////                return fields.join(", ");
////            }()
////        );
////        GDALClose(poDS);
////        return;
////    }
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//    // 选择输出文件
//    QString outTxt = QFileDialog::getSaveFileName(this, "Save Grid Tiles Info", "grid_tiles.txt", "Text Files (*.txt)");
//    if(outTxt.isEmpty()) {
//        //shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
////        GDALClose(poDS);
//        //shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        return;
//    }

//    QFile file(outTxt);
//    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//        QMessageBox::warning(this, "Export Grid Tiles", "Cannot open output file for writing: " + outTxt);
//        //shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
////        GDALClose(poDS);
//        //shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        return;
//    }

//    QTextStream out(&file);
//    out.setCodec("UTF-8");

//    // 写入文件头
//    out << "FID\tZoom\tRow\tCol\tWidth_m\tHeight_m\tOSGB_Level\tMin_X\tMin_Y\tMax_X\tMax_Y\tFiles\n";

//    QVector<QgsFeature> allFeatures;

//    if (startlevel <= AppConstants::PRE_GENERATED_MAX_LEVEL){
//    for(int level = startlevel; level <= AppConstants::PRE_GENERATED_MAX_LEVEL; level++) {
//            if(g_gridManager->hasGridDataForLevel(level)) {
//                QgsFeatureList features = g_gridManager->getGridFeaturesForLevel(level);
//                // 只处理包含用户范围的网格
//                for(const auto& feature : features) {
//                    QgsAttributes attrs = feature.attributes();
//                    if(attrs.size() > 5 && attrs[5].toInt() == 1) {
//                        allFeatures.append(feature);
//                    }
//                }
//            }
//        }
//    }else{
//    for(int level = startlevel; level <= targetlevel; level++) {
//            if(g_gridManager->hasGridDataForLevel(level)) {
//                QgsFeatureList features = g_gridManager->getGridFeaturesForLevel(level);
//                // 只处理包含用户范围的网格
//                for(const auto& feature : features) {
//                    QgsAttributes attrs = feature.attributes();
//                    if(attrs.size() > 5 && attrs[5].toInt() == 1) {
//                        allFeatures.append(feature);
//                    }
//                }
//            }
//        }
//    }
//    // 获取要素数量
//    int totalFeatures = allFeatures.size();
//    qDebug() << "Total features to process:" << totalFeatures;
//    if(totalFeatures == 0) {
//        QMessageBox::information(this, "Export Grid Tiles", "No grid features found to export");
//        file.close();
//        return;
//    }
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
////    int totalFeatures = static_cast<int>(poLayer->GetFeatureCount());
////    qDebug() << "Total features:" << totalFeatures;
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//    QProgressDialog progress("Processing grid cells...", "Cancel", 0, totalFeatures, this);
//    progress.setWindowModality(Qt::WindowModal);

//    int featureCount = 0;
//    int fid = 0;
//    int totalFiles = 0;

//    for(const QgsFeature& feature : allFeatures) {
//        if(progress.wasCanceled()) {
//            break;
//        }

//        progress.setValue(featureCount++);
//        if(featureCount % 10 == 0) {
//            QApplication::processEvents();
//        }

//        // 获取字段值
//        QgsAttributes attrs = feature.attributes();
//        int zoom = (attrs.size() > 0) ? attrs[0].toInt() : -1;
//        int row = (attrs.size() > 1) ? attrs[1].toInt() : -1;
//        int col = (attrs.size() > 2) ? attrs[2].toInt() : -1;
//        double width_m = (attrs.size() > 3) ? attrs[3].toDouble() : -1;
//        double height_m = (attrs.size() > 4) ? attrs[4].toDouble() : -1;
//        int level =m_osgbMapper->getOsgbLevelForGridLevel(zoom); // OSGB_Level 使用zoom值

//        // 获取几何边界（Web墨卡托坐标）
//        QgsGeometry geometry = feature.geometry();
//        QgsRectangle bbox = geometry.boundingBox();

//        double min_x = bbox.xMinimum();
//        double min_y = bbox.yMinimum();
//        double max_x = bbox.xMaximum();
//        double max_y = bbox.yMaximum();

//        // 将Web墨卡托坐标转换为WGS84经纬度
//        QgsCoordinateTransform transform(
//            QgsCoordinateReferenceSystem("EPSG:3857"), // Web墨卡托
//            QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84
//            QgsProject::instance()
//        );

//        try {
//            QgsPointXY wgs84Min = transform.transform(QgsPointXY(min_x, min_y));
//            QgsPointXY wgs84Max = transform.transform(QgsPointXY(max_x, max_y));

//            min_x = wgs84Min.x();
//            min_y = wgs84Min.y();
//            max_x = wgs84Max.x();
//            max_y = wgs84Max.y();
//        } catch (...) {
//            qWarning() << "Coordinate transformation failed for feature" << fid;
//            // 如果转换失败，使用原始坐标
//        }

//        // 使用现有函数查找该范围内的文件
//        std::vector<QString> cellFiles;

//        // 直接使用属性表中的层级值，不需要转换
//        int L = level;

//        auto pLL = geo.WGStoLocal(min_x, min_y);
//        auto pUR = geo.WGStoLocal(max_x, max_y);

//        double qxMin = std::min(pLL.first, pUR.first)/*-abs((pUR.first-pLL.first))*/;
//        double qxMax = std::max(pLL.first, pUR.first)/*+abs((pUR.first-pLL.first))*/;
//        double qyMin = std::min(pLL.second, pUR.second)/*-abs((pUR.second-pLL.second))*/;
//        double qyMax = std::max(pLL.second, pUR.second)/*+abs((pUR.second-pLL.second))*/;

////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    // 重置图层读取位置
////    poLayer->ResetReading();

//    // 遍历所有要素
////    OGRFeature* poFeature;
////    int featureCount = 0;
////    int totalFiles = 0;

////    while((poFeature = poLayer->GetNextFeature()) != NULL) {
////        if(progress.wasCanceled()) {
////            break;
////        }

////        progress.setValue(featureCount++);
////        if(featureCount % 10 == 0) {
////            QApplication::processEvents();
////        }

////        // 获取字段值
////        int fid = (fidFieldIndex != -1) ? poFeature->GetFieldAsInteger(fidFieldIndex) : poFeature->GetFID();
////        int zoom = (zoomFieldIndex != -1) ? poFeature->GetFieldAsInteger(zoomFieldIndex) : -1;
////        int row = (rowFieldIndex != -1) ? poFeature->GetFieldAsInteger(rowFieldIndex) : -1;
////        int col = (colFieldIndex != -1) ? poFeature->GetFieldAsInteger(colFieldIndex) : -1;
////        double width_m = (widthMFieldIndex != -1) ? poFeature->GetFieldAsDouble(widthMFieldIndex) : -1;
////        double height_m = (heightMFieldIndex != -1) ? poFeature->GetFieldAsDouble(heightMFieldIndex) : -1;
////        int level = (levelFieldIndex != -1) ? poFeature->GetFieldAsInteger(levelFieldIndex) : -1;
////        double min_x = (minXFieldIndex != -1) ? poFeature->GetFieldAsDouble(minXFieldIndex) : 0.0;
////        double min_y = (minYFieldIndex != -1) ? poFeature->GetFieldAsDouble(minYFieldIndex) : 0.0;
////        double max_x = (maxXFieldIndex != -1) ? poFeature->GetFieldAsDouble(maxXFieldIndex) : 0.0;
////        double max_y = (maxYFieldIndex != -1) ? poFeature->GetFieldAsDouble(maxYFieldIndex) : 0.0;

////        // 使用属性表中的边界值作为经纬度范围
////        double lonMin = min_x;
////        double lonMax = max_x;
////        double latMin = min_y;
////        double latMax = max_y;

////        // 使用现有函数查找该范围内的文件
////        std::vector<QString> cellFiles;

////        // 直接使用属性表中的层级值，不需要转换
////        int L = level;

////        auto pLL = geo.WGStoLocal(lonMin, latMin);
////        auto pUR = geo.WGStoLocal(lonMax, latMax);

////        double qxMin = std::min(pLL.first, pUR.first)-abs((pUR.first-pLL.first));
////        double qxMax = std::max(pLL.first, pUR.first)+abs((pUR.first-pLL.first));
////        double qyMin = std::min(pLL.second, pUR.second)-abs((pUR.second-pLL.second));
////        double qyMax = std::max(pLL.second, pUR.second)+abs((pUR.second-pLL.second));
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        // 遍历所有瓦片记录
//        for(const auto& kv : tileIndex) {
//            const TileCatalog::TileRecord& rec = kv.second;
//            std::vector<QString> candidates;
//            auto itL = rec.levelFiles.find(L);
//            if(itL != rec.levelFiles.end()) {
//                candidates = itL->second;
//            } else {
//                continue;
//            }

//            for(const QString& path : candidates) {
//                osg::BoundingBox bb;
//                auto it = bboxCache.find(path.toStdString());
//                if(it != bboxCache.end()) {
//                    bb = it->second;
//                } else {
//                    osg::ref_ptr<osg::Node> n = osgDB::readNodeFile(path.toStdString());
//                    if(!n) continue;
//                    osg::ComputeBoundsVisitor cb;
//                    n->accept(cb);
//                    bb = cb.getBoundingBox();
//                    bboxCache.emplace(path.toStdString(), bb);
//                }

//                double bxMin = bb.xMin(), bxMax = bb.xMax();
//                double byMin = bb.yMin(), byMax = bb.yMax();
//                bool intersect = !(bxMax < qxMin || bxMin > qxMax || byMax < qyMin || byMin > qyMax);

//                if(intersect) {
//                    cellFiles.push_back(path);
//                }
//            }
//        }

//        totalFiles += static_cast<int>(cellFiles.size());

//        GridTileInfo tileInfo;
//               tileInfo.fid = fid;
//               tileInfo.zoom = zoom;
//               tileInfo.row = row;
//               tileInfo.col = col;
//               tileInfo.width_m = width_m;
//               tileInfo.height_m = height_m;
//               tileInfo.osgb_level = level;
//               tileInfo.min_x = min_x;
//               tileInfo.min_y = min_y;
//               tileInfo.max_x = max_x;
//               tileInfo.max_y = max_y;

//               // 转换文件路径列表
//           for(const QString& filePath : cellFiles) {
//               QString normPath = QDir::fromNativeSeparators(filePath);
//               tileInfo.files.append(normPath);
//           }
//           if(cellFiles.empty()) {
//               tileInfo.files.append("NONE");
//           }

//           // 保存到全局Map中
//           g_gridTilesMap[fid] = tileInfo;
////           qDebug()<<g_gridTilesMap[0].files;

//        // 输出到文件 - 先输出属性信息，经纬度保存为7位小数
//        out << fid << "\t"
//            << zoom << "\t"
//            << row << "\t"
//            << col << "\t"
//            << QString::number(width_m, 'f', 7) << "\t"
//            << QString::number(height_m, 'f', 7) << "\t"
//            << level << "\t"
//            << QString::number(min_x, 'f', 7) << "\t"  // 保存为7位小数
//            << QString::number(min_y, 'f', 7) << "\t"  // 保存为7位小数
//            << QString::number(max_x, 'f', 7) << "\t"  // 保存为7位小数
//            << QString::number(max_y, 'f', 7) << "\t"; // 保存为7位小数
//        fid++;
//        // 输出文件列表
//        if(cellFiles.empty()) {
//            out << "NONE";
//        } else {
//            // 创建一个临时列表来存储文件路径
//            QStringList fileInfoList;

//            for(const QString& filePath : cellFiles) {
//                // 规范化文件路径
//                QString normPath = QDir::fromNativeSeparators(filePath);
//                fileInfoList << normPath; // 只保存完整路径
//            }
//            out << fileInfoList.join(";"); // 使用分号分隔多个文件
//        }
//        out << "\n";
//    }
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
////        OGRFeature::DestroyFeature(poFeature);
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
////shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    file.close();
//    //shapefile文件导出——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
////    GDALClose(poDS);

//    progress.close();

//    QMessageBox::information(this, "Export Complete",
//        QString("Exported %1 grid cells with %2 total files to %3")
//        .arg(featureCount).arg(totalFiles).arg(outTxt));
//#else
//    QMessageBox::information(this, "Export Grid Tiles",
//        "This feature requires GDAL support. Please rebuild with GDAL enabled.");
//#endif
//}


#ifdef USE_GDAL
#include <gdal.h>
#include <ogr_api.h>
#include <ogrsf_frmts.h>
#endif
#include "txtexport.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include "gridmanager.h"
#include <qgsproject.h>

QMap<int, GridTileInfo> g_gridTilesMap;

Eport::Eport(QWidget *parent, double originLon, double originLat, QString rootDir)
    : QWidget(parent)
{

    qDebug()<<originLat<<originLon<<rootDir;
    geo.setOrigin(originLon,originLat);
    //rootDir = "D:/baoli_OSGB_8CM/Production_3/Data";
    dataRootDir = rootDir;
    TileCatalog::scan(rootDir, tileIndex);
}

QStringList Eport::getFilesByFid(int fid)
{
    if (g_gridTilesMap.contains(fid)) {
        return g_gridTilesMap[fid].files;
    }
    return QStringList(); // 返回空列表表示未找到
}

QMap<int, GridTileInfo> Eport::getAllGridTiles()
{
    return g_gridTilesMap;
}

void Eport::clearGridTilesData()
{
    g_gridTilesMap.clear();
}

Eport::~Eport() { }

void Eport::exportGridTilesToTxt(GridManager * g_gridManager, OSGBMapper* m_osgbMapper, int startlevel, int targetlevel)
{
#ifdef USE_GDAL

    g_gridTilesMap.clear();

    // 设置 PROJ 数据路径（如果尚未设置）
    if(getenv("PROJ_LIB") == nullptr) {
        QString projDataPath = "D:/osgeo4w64/OSGeo4W64/share/proj";
        qputenv("PROJ_LIB", projDataPath.toUtf8());
        qDebug() << "Set PROJ_LIB to:" << projDataPath;
    }

    // 注册所有GDAL驱动
    GDALAllRegister();

    if(!g_gridManager) {
        QMessageBox::warning(this, "Export Grid Tiles", "GridManager is not available");
        return;
    }

    QVector<QgsFeature> allFeatures;

    if (startlevel <= AppConstants::PRE_GENERATED_MAX_LEVEL){
        for(int level = startlevel; level <= AppConstants::PRE_GENERATED_MAX_LEVEL; level++) {
            if(g_gridManager->hasGridDataForLevel(level)) {
                QgsFeatureList features = g_gridManager->getGridFeaturesForLevel(level);
                // 只处理包含用户范围的网格
                for(const auto& feature : features) {
                    QgsAttributes attrs = feature.attributes();
                    if(attrs.size() > 5 && attrs[5].toInt() == 1) {
                        allFeatures.append(feature);
                    }
                }
            }
        }
    } else {
        for(int level = startlevel; level <= targetlevel; level++) {
            if(g_gridManager->hasGridDataForLevel(level)) {
                QgsFeatureList features = g_gridManager->getGridFeaturesForLevel(level);
                // 只处理包含用户范围的网格
                for(const auto& feature : features) {
                    QgsAttributes attrs = feature.attributes();
                    if(attrs.size() > 5 && attrs[5].toInt() == 1) {
                        allFeatures.append(feature);
                    }
                }
            }
        }
    }

    // 获取要素数量
    int totalFeatures = allFeatures.size();
    qDebug() << "Total features to process:" << totalFeatures;
    if(totalFeatures == 0) {
        QMessageBox::information(this, "Export Grid Tiles", "No grid features found to process");
        return;
    }

    QProgressDialog progress("Processing grid cells...", "Cancel", 0, totalFeatures, this);
    progress.setWindowModality(Qt::WindowModal);

    int featureCount = 0;
    int fid = 0;
    int totalFiles = 0;

    for(const QgsFeature& feature : allFeatures) {
        if(progress.wasCanceled()) {
            break;
        }

        progress.setValue(featureCount++);
        if(featureCount % 10 == 0) {
            QApplication::processEvents();
        }

        // 获取字段值
        QgsAttributes attrs = feature.attributes();
        int zoom = (attrs.size() > 0) ? attrs[0].toInt() : -1;
        int row = (attrs.size() > 1) ? attrs[1].toInt() : -1;
        int col = (attrs.size() > 2) ? attrs[2].toInt() : -1;
        double width_m = (attrs.size() > 3) ? attrs[3].toDouble() : -1;
        double height_m = (attrs.size() > 4) ? attrs[4].toDouble() : -1;
        int level = m_osgbMapper->getOsgbLevelForGridLevel(zoom); // OSGB_Level 使用zoom值

        // 获取几何边界（Web墨卡托坐标）
        QgsGeometry geometry = feature.geometry();
        QgsRectangle bbox = geometry.boundingBox();

        double min_x = bbox.xMinimum();
        double min_y = bbox.yMinimum();
        double max_x = bbox.xMaximum();
        double max_y = bbox.yMaximum();

        // 将Web墨卡托坐标转换为WGS84经纬度
        QgsCoordinateTransform transform(
            QgsCoordinateReferenceSystem("EPSG:3857"), // Web墨卡托
            QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84
            QgsProject::instance()
        );

        try {
            QgsPointXY wgs84Min = transform.transform(QgsPointXY(min_x, min_y));
            QgsPointXY wgs84Max = transform.transform(QgsPointXY(max_x, max_y));

            min_x = wgs84Min.x();
            min_y = wgs84Min.y();
            max_x = wgs84Max.x();
            max_y = wgs84Max.y();
        } catch (...) {
            qWarning() << "Coordinate transformation failed for feature" << fid;
            // 如果转换失败，使用原始坐标
        }

        // 使用现有函数查找该范围内的文件
        std::vector<QString> cellFiles;

        // 直接使用属性表中的层级值，不需要转换
        int L = level;
        qDebug()<<"____________________________"<<L;
        auto pLL = geo.WGStoLocal(min_x, min_y);
        auto pUR = geo.WGStoLocal(max_x, max_y);

        double qxMin = std::min(pLL.first, pUR.first)-abs((pUR.first-pLL.first));
        double qxMax = std::max(pLL.first, pUR.first)+abs((pUR.first-pLL.first));
        double qyMin = std::min(pLL.second, pUR.second)-abs((pUR.second-pLL.second));
        double qyMax = std::max(pLL.second, pUR.second)+abs((pUR.second-pLL.second));
        // 遍历所有瓦片记录
        for(const auto& kv : tileIndex) {
            const TileCatalog::TileRecord& rec = kv.second;
            std::vector<QString> candidates;
            auto itL = rec.levelFiles.find(L);
            if(itL != rec.levelFiles.end()) {
                candidates = itL->second;
            } else {
                continue;
            }

            for(const QString& path : candidates) {

                osg::BoundingBox bb;
                auto it = bboxCache.find(path.toStdString());
                if(it != bboxCache.end()) {
                    bb = it->second;
                } else {
                    osg::ref_ptr<osg::Node> n = osgDB::readNodeFile(path.toStdString());
                    if(!n) continue;
                    osg::ComputeBoundsVisitor cb;
                    n->accept(cb);
                    bb = cb.getBoundingBox();
                    bboxCache.emplace(path.toStdString(), bb);
                }

                double bxMin = bb.xMin(), bxMax = bb.xMax();
                double byMin = bb.yMin(), byMax = bb.yMax();
                bool intersect = !(bxMax < qxMin || bxMin > qxMax || byMax < qyMin || byMin > qyMax);

                if(intersect) {
                    cellFiles.push_back(path);
                }
            }
        }

        totalFiles += static_cast<int>(cellFiles.size());

        GridTileInfo tileInfo;
        tileInfo.fid = fid;
        tileInfo.zoom = zoom;
        tileInfo.row = row;
        tileInfo.col = col;
        tileInfo.width_m = width_m;
        tileInfo.height_m = height_m;
        tileInfo.osgb_level = level;
        tileInfo.min_x = min_x;
        tileInfo.min_y = min_y;
        tileInfo.max_x = max_x;
        tileInfo.max_y = max_y;

        // 转换文件路径列表
        for(const QString& filePath : cellFiles) {
            QString normPath = QDir::fromNativeSeparators(filePath);
            tileInfo.files.append(normPath);
        }
        if(cellFiles.empty()) {
            tileInfo.files.append("NONE");
        }

        // 保存到全局Map中
        g_gridTilesMap[fid] = tileInfo;
        fid++;
    }

    progress.close();

    QMessageBox::information(this, "Processing Complete",
        QString("Processed %1 grid cells with %2 total files. Data saved internally.")
        .arg(featureCount).arg(totalFiles));
#else
    QMessageBox::information(this, "Export Grid Tiles",
        "This feature requires GDAL support. Please rebuild with GDAL enabled.");
#endif
}
