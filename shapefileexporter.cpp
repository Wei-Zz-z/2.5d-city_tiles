#include "shapefileexporter.h"
#include "gridmanager.h"
#include "osgbmapper.h"
#include <qgscoordinatetransform.h>
#include <qgsproject.h>
#include <qgsgeometry.h>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include "appconstants.h"

ShapefileExporter::ShapefileExporter(QObject *parent)
    : QObject(parent)
{
}

bool ShapefileExporter::saveGridFromLevel(GridManager *gridManager, OSGBMapper *osgbMapper,
                                        const QgsRectangle &userExtent, int startLevel,
                                        const QString &savePath, int m_targetLevel)
{
    qDebug() << "开始保存网格，从层级" << startLevel << "到" << m_targetLevel;
    if (!gridManager || !osgbMapper) {
        emit exportFinished(false, "网格管理器或OSGB映射器未初始化");
        return false;
    }

    if (userExtent.isEmpty()) {
        emit exportFinished(false, "用户范围为空");
        return false;
    }

    if (savePath.isEmpty()) {
        emit exportFinished(false, "保存路径为空");
        return false;
    }

    // 确保目录存在
    QFileInfo fileInfo(savePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists() && !dir.mkpath(".")) {
        emit exportFinished(false, "无法创建目录: " + dir.path());
        return false;
    }

    // 注册GDAL驱动
    GDALAllRegister();

    GDALDataset *dataset = nullptr;
    OGRLayer *layer = nullptr;

    try {
        // 创建Shapefile数据源
        if (!createShapefile(savePath, dataset)) {
            emit exportFinished(false, "无法创建Shapefile数据源");
            return false;
        }

        // 创建图层
        OGRSpatialReference targetSRS;
        if (targetSRS.importFromEPSG(4326) != OGRERR_NONE) {
            emit exportFinished(false, "无法初始化目标坐标系 (EPSG:4326)");
            GDALClose(dataset);
            return false;
        }

        layer = dataset->CreateLayer(
            QFileInfo(savePath).baseName().toUtf8().constData(),
            &targetSRS,
            wkbPolygon,
            nullptr);

        if (!layer) {
            emit exportFinished(false, "无法创建图层");
            GDALClose(dataset);
            return false;
        }

        // 添加字段
        if (!addFieldsToLayer(layer)) {
            emit exportFinished(false, "无法添加字段到图层");
            GDALClose(dataset);
            return false;
        }

        // 处理要素
        if (!processFeatures(dataset, layer, gridManager, osgbMapper, userExtent, startLevel, m_targetLevel)) {
            emit exportFinished(false, "处理要素时出错");
            GDALClose(dataset);
            return false;
        }

        // 创建CPG文件指定编码
        createCpgFile(savePath);

        // 关闭数据集
        GDALClose(dataset);

        emit exportFinished(true, "导出成功");
        return true;

    } catch (const std::exception &e) {
        qWarning() << "导出Shapefile时发生异常:" << e.what();
        if (dataset) {
            GDALClose(dataset);
        }
        emit exportFinished(false, QString("导出时发生异常: %1").arg(e.what()));
        return false;
    }
}

bool ShapefileExporter::createShapefile(const QString &path, GDALDataset* &dataset)
{
    GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (!driver) {
        qWarning() << "无法获取ESRI Shapefile驱动";
        return false;
    }

    // 删除已存在的文件
    if (QFile::exists(path)) {
        driver->Delete(path.toUtf8().constData());
    }

    dataset = driver->Create(
        path.toUtf8().constData(),
        0, 0, 0, GDT_Unknown, nullptr);

    if (!dataset) {
        qWarning() << "无法创建Shapefile数据源";
        return false;
    }

    return true;
}

bool ShapefileExporter::addFieldsToLayer(OGRLayer *layer)
{
    // 创建字段
    OGRFieldDefn zoomField("zoom", OFTInteger);
    if (layer->CreateField(&zoomField) != OGRERR_NONE) {
        qWarning() << "无法创建zoom字段";
        return false;
    }

    OGRFieldDefn rowField("row", OFTInteger);
    if (layer->CreateField(&rowField) != OGRERR_NONE) {
        qWarning() << "无法创建row字段";
        return false;
    }

    OGRFieldDefn colField("col", OFTInteger);
    if (layer->CreateField(&colField) != OGRERR_NONE) {
        qWarning() << "无法创建col字段";
        return false;
    }

    OGRFieldDefn widthField("width_m", OFTReal);
    if (layer->CreateField(&widthField) != OGRERR_NONE) {
        qWarning() << "无法创建width_m字段";
        return false;
    }

    OGRFieldDefn heightField("height_m", OFTReal);
    if (layer->CreateField(&heightField) != OGRERR_NONE) {
        qWarning() << "无法创建height_m字段";
        return false;
    }

    OGRFieldDefn osgbLevelField("osgb_level", OFTInteger);
    if (layer->CreateField(&osgbLevelField) != OGRERR_NONE) {
        qWarning() << "无法创建osgb_level字段";
        return false;
    }

    OGRFieldDefn minxField("min_x", OFTReal);
    if (layer->CreateField(&minxField) != OGRERR_NONE) {
        qWarning() << "无法创建min_x字段";
        return false;
    }

    OGRFieldDefn minyField("min_y", OFTReal);
    if (layer->CreateField(&minyField) != OGRERR_NONE) {
        qWarning() << "无法创建min_y字段";
        return false;
    }

    OGRFieldDefn maxxField("max_x", OFTReal);
    if (layer->CreateField(&maxxField) != OGRERR_NONE) {
        qWarning() << "无法创建max_x字段";
        return false;
    }

    OGRFieldDefn maxyField("max_y", OFTReal);
    if (layer->CreateField(&maxyField) != OGRERR_NONE) {
        qWarning() << "无法创建max_y字段";
        return false;
    }

    return true;
}

bool ShapefileExporter::processFeatures(GDALDataset *dataset, OGRLayer *layer,
                                      GridManager *gridManager, OSGBMapper *osgbMapper,
                                      const QgsRectangle &userExtent, int startLevel, int m_targetLevel)
{
    if (!gridManager || !osgbMapper) {
        return false;
    }

    // 创建坐标转换：从Web墨卡托到WGS84
    QgsCoordinateTransform transform(
        QgsCoordinateReferenceSystem("EPSG:3857"), // Web墨卡托
        QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84
        QgsProject::instance()
    );

    int successCount = 0;
    int failedCount = 0;
    int totalCount = 0;

    // 计算总要素数量（用于进度显示）
    for (int level = startLevel; level <= m_targetLevel; level++) {
        if (gridManager->hasGridDataForLevel(level)) {
            totalCount += gridManager->getGridFeaturesForLevel(level).size();
        }
    }

    int processedCount = 0;

    // 从指定层级开始，一直到目标层级
    for (int level = startLevel; level <= m_targetLevel; level++) {
        if (!gridManager->hasGridDataForLevel(level)) {
            continue;
        }

        // 获取当前网格层级对应的OSGB层级
        int osgbLevel = osgbMapper->getOsgbLevelForGridLevel(level);

        QgsFeatureList features = gridManager->getGridFeaturesForLevel(level);
        for (const QgsFeature& qgisFeature : features) {
            // 检查是否包含用户范围
            QgsGeometry gridGeom = qgisFeature.geometry();
            bool intersects = gridGeom.intersects(userExtent);

            if (!intersects) {
                processedCount++;
                if (m_progressCallback) {
                    m_progressCallback(processedCount, totalCount);
                }
                emit progressChanged(processedCount, totalCount);
                continue; // 跳过不包含用户范围的网格
            }

            OGRFeature *ogrFeature = OGRFeature::CreateFeature(layer->GetLayerDefn());

            // 获取属性
            QgsAttributes attrs = qgisFeature.attributes();

            // 设置属性
            ogrFeature->SetField("zoom", attrs[0].toInt());
            ogrFeature->SetField("row", attrs[1].toInt());
            ogrFeature->SetField("col", attrs[2].toInt());
            ogrFeature->SetField("width_m", attrs[3].toDouble());
            ogrFeature->SetField("height_m", attrs[4].toDouble());
            ogrFeature->SetField("osgb_level", osgbLevel); // 设置OSGB层级

            // 获取几何并转换为WGS84
            QgsGeometry geom = qgisFeature.geometry();
            try {
                geom.transform(transform);
            } catch (const QgsCsException &e) {
                qWarning() << "坐标转换失败:" << e.what();
                OGRFeature::DestroyFeature(ogrFeature);
                failedCount++;
                processedCount++;
                if (m_progressCallback) {
                    m_progressCallback(processedCount, totalCount);
                }
                emit progressChanged(processedCount, totalCount);
                continue;
            }

            QgsRectangle bbox = geom.boundingBox();

            // 设置边界框字段
            ogrFeature->SetField("min_x", bbox.xMinimum());
            ogrFeature->SetField("min_y", bbox.yMinimum());
            ogrFeature->SetField("max_x", bbox.xMaximum());
            ogrFeature->SetField("max_y", bbox.yMaximum());

            // 确保坐标在有效范围内
            double minLon = qMax(-180.0, bbox.xMinimum());
            double minLat = qMax(-90.0, bbox.yMinimum());
            double maxLon = qMin(180.0, bbox.xMaximum());
            double maxLat = qMin(90.0, bbox.yMaximum());

            // 创建多边形几何 - 使用转换后的几何体，而不是边界框
            OGRGeometry* ogrGeom = nullptr;
            QByteArray wkb = geom.asWkb();
            if (!wkb.isEmpty()) {
                OGRGeometryFactory::createFromWkb(
                    reinterpret_cast<const unsigned char*>(wkb.constData()),
                    nullptr,
                    &ogrGeom,
                    wkb.size()
                );

                if (ogrGeom) {
                    ogrFeature->SetGeometry(ogrGeom);
                    OGRGeometryFactory::destroyGeometry(ogrGeom);
                }
            }

            // 如果WKB转换失败，使用边界框创建多边形作为备选
            if (!ogrGeom) {
                OGRPolygon* poly = new OGRPolygon();
                OGRLinearRing* ring = new OGRLinearRing();

                ring->addPoint(minLon, minLat);
                ring->addPoint(minLon, maxLat);
                ring->addPoint(maxLon, maxLat);
                ring->addPoint(maxLon, minLat);
                ring->addPoint(minLon, minLat); // 闭合多边形

                poly->addRingDirectly(ring);
                ogrFeature->SetGeometry(poly);
            }

            // 写入要素
            if (layer->CreateFeature(ogrFeature) != OGRERR_NONE) {
                qWarning() << "写入要素失败:" << CPLGetLastErrorMsg();
                failedCount++;
            } else {
                successCount++;
            }

            OGRFeature::DestroyFeature(ogrFeature);

            processedCount++;
            if (m_progressCallback) {
                m_progressCallback(processedCount, totalCount);
            }
            emit progressChanged(processedCount, totalCount);
        }
    }

    qDebug() << "成功导出" << successCount << "个要素，" << failedCount << "个要素失败";

    if (successCount == 0) {
        qWarning() << "没有找到与用户范围相交的网格";
        return false;
    }

    return true;
}

void ShapefileExporter::createCpgFile(const QString &shapefilePath)
{
    // 创建CPG文件指定编码
    QString cpgPath = shapefilePath;
    cpgPath.replace(".shp", ".cpg", Qt::CaseInsensitive);

    QFile cpgFile(cpgPath);
    if (cpgFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&cpgFile);
        stream << "UTF-8";
        cpgFile.close();
        qDebug() << "已创建 .cpg 文件指定 UTF-8 编码";
    } else {
        qWarning() << "无法创建 .cpg 文件";
    }
}

void ShapefileExporter::setProgressCallback(std::function<void(int, int)> callback)
{
    m_progressCallback = callback;
}

bool ShapefileExporter::savePreGeneratedGrid(GridManager *gridManager, OSGBMapper *osgbMapper,
                                           const QgsRectangle &userExtent, int startLevel, int endLevel,
                                           const QString &savePath)
{
    // 确保结束层级不超过预生成的最大层级
    endLevel = qMin(endLevel, AppConstants::PRE_GENERATED_MAX_LEVEL);

    qDebug() << "开始保存预生成网格，从层级" << startLevel << "到" << endLevel;

    if (!gridManager || !osgbMapper) {
        emit exportFinished(false, "网格管理器或OSGB映射器未初始化");
        return false;
    }

    if (userExtent.isEmpty()) {
        emit exportFinished(false, "用户范围为空");
        return false;
    }

    if (savePath.isEmpty()) {
        emit exportFinished(false, "保存路径为空");
        return false;
    }

    // 确保目录存在
    QFileInfo fileInfo(savePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists() && !dir.mkpath(".")) {
        emit exportFinished(false, "无法创建目录: " + dir.path());
        return false;
    }

    // 注册GDAL驱动
    GDALAllRegister();

    GDALDataset *dataset = nullptr;
    OGRLayer *layer = nullptr;

    try {
        // 创建Shapefile数据源
        if (!createShapefile(savePath, dataset)) {
            emit exportFinished(false, "无法创建Shapefile数据源");
            return false;
        }

        // 创建图层
        OGRSpatialReference targetSRS;
        if (targetSRS.importFromEPSG(4326) != OGRERR_NONE) {
            emit exportFinished(false, "无法初始化目标坐标系 (EPSG:4326)");
            GDALClose(dataset);
            return false;
        }

        layer = dataset->CreateLayer(
            QFileInfo(savePath).baseName().toUtf8().constData(),
            &targetSRS,
            wkbPolygon,
            nullptr);

        if (!layer) {
            emit exportFinished(false, "无法创建图层");
            GDALClose(dataset);
            return false;
        }

        // 添加字段
        if (!addFieldsToLayer(layer)) {
            emit exportFinished(false, "无法添加字段到图层");
            GDALClose(dataset);
            return false;
        }

        // 处理预生成网格的要素
        if (!processPreGeneratedFeatures(dataset, layer, gridManager, osgbMapper, userExtent, startLevel, endLevel)) {
            emit exportFinished(false, "处理预生成要素时出错");
            GDALClose(dataset);
            return false;
        }

        // 创建CPG文件指定编码
        createCpgFile(savePath);

        // 关闭数据集
        GDALClose(dataset);

        emit exportFinished(true, "预生成网格导出成功");
        return true;

    } catch (const std::exception &e) {
        qWarning() << "导出预生成网格Shapefile时发生异常:" << e.what();
        if (dataset) {
            GDALClose(dataset);
        }
        emit exportFinished(false, QString("导出预生成网格时发生异常: %1").arg(e.what()));
        return false;
    }
}

bool ShapefileExporter::processPreGeneratedFeatures(GDALDataset *dataset, OGRLayer *layer,
                                                  GridManager *gridManager, OSGBMapper *osgbMapper,
                                                  const QgsRectangle &userExtent, int startLevel, int endLevel)
{
    if (!gridManager || !osgbMapper) {
        return false;
    }

    // 创建坐标转换：从Web墨卡托到WGS84
    QgsCoordinateTransform transform(
        QgsCoordinateReferenceSystem("EPSG:3857"), // Web墨卡托
        QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84
        QgsProject::instance()
    );

    int successCount = 0;
    int failedCount = 0;
    int totalCount = 0;

    // 计算总要素数量（用于进度显示）
    for (int level = startLevel; level <= endLevel; level++) {
        if (gridManager->hasGridDataForLevel(level)) {
            totalCount += gridManager->getGridFeaturesForLevel(level).size();
        }
    }

    int processedCount = 0;

    // 从指定层级开始，一直到结束层级
    for (int level = startLevel; level <= endLevel; level++) {
        if (!gridManager->hasGridDataForLevel(level)) {
            continue;
        }

        // 获取当前网格层级对应的OSGB层级
        int osgbLevel = osgbMapper->getOsgbLevelForGridLevel(level);

        QgsFeatureList features = gridManager->getGridFeaturesForLevel(level);
        for (const QgsFeature& qgisFeature : features) {
            // 检查是否包含用户范围
            QgsGeometry gridGeom = qgisFeature.geometry();
            bool intersects = gridGeom.intersects(userExtent);

            if (!intersects) {
                processedCount++;
                if (m_progressCallback) {
                    m_progressCallback(processedCount, totalCount);
                }
                emit progressChanged(processedCount, totalCount);
                continue; // 跳过不包含用户范围的网格
            }

            OGRFeature *ogrFeature = OGRFeature::CreateFeature(layer->GetLayerDefn());

            // 获取属性
            QgsAttributes attrs = qgisFeature.attributes();

            // 设置属性
            ogrFeature->SetField("zoom", attrs[0].toInt());
            ogrFeature->SetField("row", attrs[1].toInt());
            ogrFeature->SetField("col", attrs[2].toInt());
            ogrFeature->SetField("width_m", attrs[3].toDouble());
            ogrFeature->SetField("height_m", attrs[4].toDouble());
            ogrFeature->SetField("osgb_level", osgbLevel); // 设置OSGB层级

            // 获取几何并转换为WGS84
            QgsGeometry geom = qgisFeature.geometry();
            try {
                geom.transform(transform);
            } catch (const QgsCsException &e) {
                qWarning() << "坐标转换失败:" << e.what();
                OGRFeature::DestroyFeature(ogrFeature);
                failedCount++;
                processedCount++;
                if (m_progressCallback) {
                    m_progressCallback(processedCount, totalCount);
                }
                emit progressChanged(processedCount, totalCount);
                continue;
            }

            QgsRectangle bbox = geom.boundingBox();

            // 设置边界框字段
            ogrFeature->SetField("min_x", bbox.xMinimum());
            ogrFeature->SetField("min_y", bbox.yMinimum());
            ogrFeature->SetField("max_x", bbox.xMaximum());
            ogrFeature->SetField("max_y", bbox.yMaximum());

            // 确保坐标在有效范围内
            double minLon = qMax(-180.0, bbox.xMinimum());
            double minLat = qMax(-90.0, bbox.yMinimum());
            double maxLon = qMin(180.0, bbox.xMaximum());
            double maxLat = qMin(90.0, bbox.yMaximum());

            // 创建多边形几何 - 使用转换后的几何体，而不是边界框
            OGRGeometry* ogrGeom = nullptr;
            QByteArray wkb = geom.asWkb();
            if (!wkb.isEmpty()) {
                OGRGeometryFactory::createFromWkb(
                    reinterpret_cast<const unsigned char*>(wkb.constData()),
                    nullptr,
                    &ogrGeom,
                    wkb.size()
                );

                if (ogrGeom) {
                    ogrFeature->SetGeometry(ogrGeom);
                    OGRGeometryFactory::destroyGeometry(ogrGeom);
                }
            }

            // 如果WKB转换失败，使用边界框创建多边形作为备选
            if (!ogrGeom) {
                OGRPolygon* poly = new OGRPolygon();
                OGRLinearRing* ring = new OGRLinearRing();

                ring->addPoint(minLon, minLat);
                ring->addPoint(minLon, maxLat);
                ring->addPoint(maxLon, maxLat);
                ring->addPoint(maxLon, minLat);
                ring->addPoint(minLon, minLat); // 闭合多边形

                poly->addRingDirectly(ring);
                ogrFeature->SetGeometry(poly);
            }

            // 写入要素
            if (layer->CreateFeature(ogrFeature) != OGRERR_NONE) {
                qWarning() << "写入要素失败:" << CPLGetLastErrorMsg();
                failedCount++;
            } else {
                successCount++;
            }

            OGRFeature::DestroyFeature(ogrFeature);

            processedCount++;
            if (m_progressCallback) {
                m_progressCallback(processedCount, totalCount);
            }
            emit progressChanged(processedCount, totalCount);
        }
    }

    qDebug() << "成功导出" << successCount << "个预生成网格要素，" << failedCount << "个要素失败";

    if (successCount == 0) {
        qWarning() << "没有找到与用户范围相交的预生成网格";
        return false;
    }

    return true;
}
