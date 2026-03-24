#ifndef SCALECALCULATOR_H
#define SCALECALCULATOR_H

#include <QObject>
#include <qgsrectangle.h>
#include <QSize>
#include "appconstants.h"
class QgsMapCanvas;

class ScaleCalculator : public QObject
{
    Q_OBJECT

public:
    explicit ScaleCalculator(QObject *parent = nullptr);

    // 计算当前地图视图的比例尺分母
    double calculateCurrentScaleDenominator(QgsMapCanvas *mapCanvas);

    // 根据比例尺分母计算网格等级
    int calculateGridLevelFromScale(double scaleDenominator);

    // 计算给定范围的最优比例尺
    double calculateOptimalScaleForExtent(const QgsRectangle &extent, const QSize &canvasSize);

private:
    // 天地图比例尺分母数组（对应层级1-18）
};

#endif // SCALECALCULATOR_H
