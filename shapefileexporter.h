#ifndef SHAPEFILEEXPORTER_H
#define SHAPEFILEEXPORTER_H

#include <QObject>
#include <qgsrectangle.h>
#include <qgsfeature.h>
#include <qgsvectorlayer.h>
#include <gdal.h>
#include <ogrsf_frmts.h>
#include "appconstants.h"
class GridManager;
class OSGBMapper;

class ShapefileExporter : public QObject
{
    Q_OBJECT

public:
    bool savePreGeneratedGrid(GridManager *gridManager, OSGBMapper *osgbMapper,
                            const QgsRectangle &userExtent, int startLevel, int endLevel,
                            const QString &savePath);
    explicit ShapefileExporter(QObject *parent = nullptr);

    // 从指定层级开始保存网格
    bool saveGridFromLevel(GridManager *gridManager, OSGBMapper *osgbMapper,
                          const QgsRectangle &userExtent, int startLevel,
                          const QString &savePath,int m_targetLevel);

    // 设置进度回调（可选）
    void setProgressCallback(std::function<void(int, int)> callback);

signals:
    void progressChanged(int current, int total);
    void exportFinished(bool success, const QString &message);

private:
    bool processPreGeneratedFeatures(GDALDataset *dataset, OGRLayer *layer,
                                                      GridManager *gridManager, OSGBMapper *osgbMapper,
                                                      const QgsRectangle &userExtent, int startLevel, int endLevel);
    // 内部辅助方法
    bool createShapefile(const QString &path, GDALDataset* &dataset);
    bool addFieldsToLayer(OGRLayer *layer);
    bool processFeatures(GDALDataset *dataset, OGRLayer *layer,
                        GridManager *gridManager, OSGBMapper *osgbMapper,
                        const QgsRectangle &userExtent, int startLevel,int m_targetLevel);
    void createCpgFile(const QString &shapefilePath);

    // 进度回调
    std::function<void(int, int)> m_progressCallback;
};

#endif // SHAPEFILEEXPORTER_H
