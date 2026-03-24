#ifndef GRIDMANAGER_H
#define GRIDMANAGER_H

#include <QObject>
#include <qgsvectorlayer.h>
#include <qgsrectangle.h>
#include <qgsfeature.h>
#include <qgsspatialindex.h>
#include <QMap>
#include "gridrenderer.h"
#include "appconstants.h"

class QgsMapCanvas;

class GridManager : public QObject
{
    Q_OBJECT

public:
    QMap<int, QgsFeatureList> m_dynamicGridByLevel;
    QMap<int, QgsFeatureList> m_globalGridByLevel;
    explicit GridManager(QObject *parent = nullptr);
    ~GridManager();
    // 网格创建和管理方法
    void createGlobalGrid();
    void changeGridLevel(int level);
    void updateGridWithUserExtent(const QgsRectangle &userExtent);
    void updateGridWithUserExtent_largelevel(const QgsRectangle &userExtent);

    // 访问方法
    QgsVectorLayer* gridLayer() const { return m_gridLayer; }
    const QMap<int, QgsFeatureList>& globalGridByLevel() const { return m_globalGridByLevel; }
    bool isGlobalGridCreated() const { return m_isGlobalGridCreated; }
    int currentGridLevel() const { return m_currentGridLevel; }

    // 网格查询方法
    QList<QgsFeature> getParentFeaturesWithUserExtent(int level);
    // 新增方法
    void generateDynamicGridsFromOptimalLevel(int optimalLevel, const QgsRectangle& userExtent,int m_targetLevel);
    void generateDynamicGridsRecursive(int currentLevel, const QgsFeature& parentFeature, const QgsRectangle& userExtent,int m_targetLevel);
    QgsFeatureList getGridFeaturesForLevel(int level) const;
    bool hasGridDataForLevel(int level) const;

signals:
    void gridLevelChanged(int newLevel);
    void gridCreated();
    void gridUpdated();

private:
/*    QMap<int, QgsFeatureList> m_dynamicGridByLevel;*/ // 存储动态生成的网格

    QMap<int, QgsSpatialIndex*> m_levelSpatialIndexes;
    // 内部辅助方法
    void setupGridRenderer();


    void checkPreGeneratedChildGrids(int parentLevel, const QgsFeature &parentFeature,
                                                const QgsRectangle &userExtentWebMercator);
    void checkPreGeneratedGridForUserExtent(int level, const QgsRectangle &userExtentWebMercator);
    void checkGridLevelForUserExtent(int level,const QgsRectangle &userExtent);
    void checkChildGridsForUserExtent(int parentLevel, const QgsFeature &parentFeature,const QgsRectangle &m_userExtentWebMercator);
    void resetGridAttributes();

    // 成员变量
    QgsVectorLayer* m_gridLayer;
//    QMap<int, QgsFeatureList> m_globalGridByLevel;
    std::unique_ptr<QgsSpatialIndex> m_gridSpatialIndex;
    QMap<QgsFeatureId, QgsRectangle> m_gridCells;
    GridRenderer* m_gridRenderer;  // 新增: GridRenderer 实例

    // 状态标志
    bool m_isGlobalGridCreated;
    bool m_isIndexBuilt;
    int m_currentGridLevel;

    // 常量

};

#endif // GRIDMANAGER_H
