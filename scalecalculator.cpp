#include "scalecalculator.h"
#include <qgsmapcanvas.h>
#include <qgsmapsettings.h>
#include <qgscoordinatetransform.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include <qgsgeometry.h>
#include <cmath>
#include <QDebug>

// 天地图比例尺分母数组（对应层级1-18）

ScaleCalculator::ScaleCalculator(QObject *parent)
    : QObject(parent)
{
}

double ScaleCalculator::calculateCurrentScaleDenominator(QgsMapCanvas *mapCanvas)
{
    if (!mapCanvas) return 0;

    QgsMapSettings mapSettings = mapCanvas->mapSettings();
    QgsRectangle extent = mapCanvas->extent();

    if (extent.isEmpty() || extent.isNull() || extent.width() <= 0) {
        return 0;
    }

    try {
        // 创建临时图层
        QgsVectorLayer* tempLayer = new QgsVectorLayer("Polygon?crs=EPSG:3857", "temp_scale_layer", "memory");

        // 创建要素并设置几何
        QgsFeature feature;
        QgsGeometry geometry = QgsGeometry::fromRect(extent);
        feature.setGeometry(geometry);

        // 添加要素到临时图层
        tempLayer->dataProvider()->addFeature(feature);
        tempLayer->updateExtents();

        // 设置图层坐标系为4326
        QgsCoordinateTransform transform(
            QgsCoordinateReferenceSystem("EPSG:3857"),
            QgsCoordinateReferenceSystem("EPSG:4326"),
            QgsProject::instance()
        );
        tempLayer->setCrs(QgsCoordinateReferenceSystem("EPSG:4326"));

        // 获取转换后的范围
        QgsRectangle transformedExtent = transform.transformBoundingBox(extent);

        // 创建临时地图设置
        QgsMapSettings tempSettings;
        tempSettings.setDestinationCrs(tempLayer->crs());
        tempSettings.setExtent(transformedExtent);
        tempSettings.setOutputSize(mapCanvas->size());
        tempSettings.setOutputDpi(mapSettings.outputDpi());

        // 使用scale()方法计算比例尺
        double scaleDenominator = tempSettings.scale();

        // 清理临时图层
        delete tempLayer;

        return scaleDenominator;

    } catch (...) {
        qDebug() << "计算比例尺时出错";
        return 0;
    }
}

int ScaleCalculator::calculateGridLevelFromScale(double scaleDenominator)
{
    // 使用天地图的比例尺分母来确定层级
    for (int level = 0; level <= AppConstants::MAX_ZOOM_LEVEL; level++) {
        if (scaleDenominator >= AppConstants::TDT_SCALE_DENOMINATORS[level]) {
            return level;
        }
    }

    // 如果比例尺分母比最小层级还小，返回最大层级
    return AppConstants::MAX_ZOOM_LEVEL;
}


double ScaleCalculator::calculateOptimalScaleForExtent(const QgsRectangle &extent, const QSize &canvasSize)
{
    if (extent.isEmpty()) {
        return 0.0;
    }

    // 获取地图画布尺寸
    if (canvasSize.width() <= 0 || canvasSize.height() <= 0) {
        return 0.0;
    }

    // 使用WMTS标准计算方法
    // 参考天地图TileMatrix中的ScaleDenominator计算方式

    // 地球周长（米） - 使用WGS84椭球体周长
    const double earthCircumference = 40075016.68;

    // 计算范围宽度占地球周长的比例
    double widthRatio = extent.width() / 360.0; // 经度范围占全球的比例

    // 计算中心点纬度的余弦值（用于墨卡托投影校正）
    double centerLat = extent.center().y();
    double latCorrection = cos(centerLat * M_PI / 180.0);
    if (latCorrection < 0.01) latCorrection = 0.01; // 避免除以零

    // 计算实际地面距离（米）
    double groundDistance = earthCircumference * widthRatio * latCorrection;

    // 计算画布上的像素距离
    double pixelDistance = canvasSize.width();

    // 计算比例尺分母（使用WMTS标准公式）
    // ScaleDenominator = groundDistance / (pixelDistance * 0.00028)
    // 其中0.00028是96dpi下每像素的米数（0.0254/96）
    const double metersPerPixel = 0.00028;
    double scaleDenominator = groundDistance / (pixelDistance * metersPerPixel);

    return scaleDenominator;
}
