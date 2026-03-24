#include "gridmanager.h"
#include <qgsmapcanvas.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsrulebasedrenderer.h>
#include <qgslinesymbollayer.h>
#include <QDebug>
#include "appconstants.h"
#include <QtConcurrent>
#include <QtConcurrent> // 如果使用并行处理
#include <QFutureWatcher> // 如果使用并行处理
#include <QEventLoop>
// 天地图比例尺分母数组（对应层级1-11）

GridManager::GridManager(QObject *parent)
    : QObject(parent)
    , m_gridLayer(nullptr)
    , m_gridRenderer(new GridRenderer(this))  // 创建 GridRenderer 实例
    , m_isGlobalGridCreated(false)
    , m_isIndexBuilt(false)
    , m_currentGridLevel(-1)

{
    m_gridSpatialIndex = std::make_unique<QgsSpatialIndex>();
    m_dynamicGridByLevel.clear();
}

GridManager::~GridManager()
{
    if (m_gridLayer) {
        delete m_gridLayer;
        m_gridLayer = nullptr;
    }
    for (auto index : m_levelSpatialIndexes) {
        delete index;
    }
    m_levelSpatialIndexes.clear();
    // GridRenderer 实例由父对象管理，不需要手动删除
}

void GridManager::createGlobalGrid()
{
    // 清理旧网格图层
    if (m_gridLayer) {
        delete m_gridLayer;
        m_gridLayer = nullptr;
    }

    // 墨卡托坐标系的全球范围
    QgsRectangle globalExtent(-20037508.3427892, -20037508.3427892,
                              20037508.3427892, 20037508.3427892);

    // 创建网格图层
    m_gridLayer = new QgsVectorLayer(
        "Polygon?crs=EPSG:3857", "全球多层级网格", "memory");

    if (!m_gridLayer->isValid()) {
        qWarning() << "无法创建全球网格图层";
        delete m_gridLayer;
        m_gridLayer = nullptr;
        return;
    }

    // 添加字段
    QgsFields fields;
    fields.append(QgsField("zoom", QVariant::Int));
    fields.append(QgsField("row", QVariant::Int));
    fields.append(QgsField("col", QVariant::Int));
    fields.append(QgsField("width_m", QVariant::Double));
    fields.append(QgsField("height_m", QVariant::Double));
    fields.append(QgsField("contains_user_extent", QVariant::Int));
    fields.append(QgsField("boundary_distance", QVariant::Double));
    m_gridLayer->dataProvider()->addAttributes(fields.toList());
    m_gridLayer->updateFields();

//    // 生成多层级网格 (0-10级)
    m_globalGridByLevel.clear();
    for (auto index : m_levelSpatialIndexes) {
        delete index;
    }
    m_levelSpatialIndexes.clear();
    for (int z = 0; z <= AppConstants::PRE_GENERATED_MAX_LEVEL; z++) {
        int tiles = static_cast<int>(pow(2, z));
        double tileSize = globalExtent.width() / tiles;
        QgsFeatureList levelFeatures;
        QgsSpatialIndex* spatialIndex = new QgsSpatialIndex();
        for (int row = 0; row < tiles; row++) {
            for (int col = 0; col < tiles; col++) {
                double x1 = globalExtent.xMinimum() + col * tileSize;
                double y1 = globalExtent.yMaximum() - (row + 1) * tileSize;
                double x2 = x1 + tileSize;
                double y2 = y1 + tileSize;

                QgsRectangle gridRect(x1, y1, x2, y2);
                QgsFeature gridFeature;
                gridFeature.setGeometry(QgsGeometry::fromRect(gridRect));

                // 初始化为不包含用户范围，距离为最大值
                gridFeature.setAttributes(QgsAttributes()
                    << z << row << col << tileSize << tileSize << 0 << std::numeric_limits<double>::max());
                spatialIndex->addFeature(gridFeature);
                levelFeatures.append(gridFeature);
            }
        }
        m_globalGridByLevel[z] = levelFeatures;
    }

    m_dynamicGridByLevel.clear();
    // 设置初始显示层级
    changeGridLevel(0);

    // 使用 GridRenderer 设置基于规则的渲染器
    setupGridRenderer();

    m_isGlobalGridCreated = true;
    m_isIndexBuilt = true;

    emit gridCreated();
}

void GridManager::changeGridLevel(int level) //已使用
{
    if (!m_gridLayer || !m_globalGridByLevel.contains(level)) {
        return;
    }

    // 清除当前图层中的所有要素
    m_gridLayer->dataProvider()->truncate();

    // 添加指定层级的网格要素
    QgsFeatureList features = m_globalGridByLevel[level];
    m_gridLayer->dataProvider()->addFeatures(features);

    // 强制更新图层
    m_gridLayer->updateExtents();
    m_gridLayer->triggerRepaint();

    // 更新当前全局网格层级
    m_currentGridLevel = level;

    // 发出层级变化信号
    emit gridLevelChanged(level);

    qDebug() << "切换到全球网格层级:" << level << "，要素数量:" << features.size();
}

void GridManager::updateGridWithUserExtent(const QgsRectangle &userExtent)
{
    if (userExtent.isEmpty()) {
        return;
    }

    // 首先重置所有网格的标记
    resetGridAttributes();
    qDebug() << "检查前:";
    // 从最粗层级(0级)开始，递归检查每个层级的网格
//        checkGridLevelForUserExtent(0,userExtent);
    checkPreGeneratedGridForUserExtent(0,userExtent);

    qDebug() << "检查后:";

    // 如果当前显示的是全球网格，刷新显示
    if (m_isGlobalGridCreated) {
        changeGridLevel(m_currentGridLevel);
    }

    emit gridUpdated();
}

void GridManager::updateGridWithUserExtent_largelevel(const QgsRectangle &userExtent)
{
    if (userExtent.isEmpty()) {
        return;
    }

    // 首先重置所有网格的标记
    resetGridAttributes();
    qDebug() << "检查前:";
    // 从最粗层级(0级)开始，递归检查每个层级的网格
        checkGridLevelForUserExtent(0,userExtent);
//    checkPreGeneratedGridForUserExtent(0,userExtent);

    qDebug() << "检查后:";

    // 如果当前显示的是全球网格，刷新显示
    if (m_isGlobalGridCreated) {
        changeGridLevel(m_currentGridLevel);
    }

    emit gridUpdated();
}

QList<QgsFeature> GridManager::getParentFeaturesWithUserExtent(int level)
{
    QList<QgsFeature> result;

    if (!m_globalGridByLevel.contains(level)) {
        return result;
    }

    QgsFeatureList &features = m_globalGridByLevel[level];
    for (const auto &feature : features) {
        QgsAttributes attrs = feature.attributes();
        if (attrs.size() > 5 && attrs[5].toInt() == 1) {
            result.append(feature);
        }
    }

    return result;
}

void GridManager::setupGridRenderer()
{
    if (!m_gridLayer) return;

    // 使用 GridRenderer 设置基于规则的渲染器
    if (m_gridRenderer) {
        m_gridRenderer->setupRuleBasedRenderer(m_gridLayer);
    } else {
        qWarning() << "GridRenderer 未初始化，无法设置渲染器";
    }
}

void GridManager::checkPreGeneratedGridForUserExtent(int level, const QgsRectangle &userExtentWebMercator)
{
//    if (level > AppConstants::PRE_GENERATED_MAX_LEVEL) {
//        return;
//    }

    if (!m_globalGridByLevel.contains(level)) {
        return;
    }

    QgsFeatureList &features = m_globalGridByLevel[level];

    // 使用 QtConcurrent::blockingMap 并行处理，它返回 void，所以不能调用 .results()
    QMutex mutex;
    QtConcurrent::blockingMap(features, [&](QgsFeature &feature) {
        QMutexLocker locker(&mutex);
        QgsGeometry gridGeom = feature.geometry();
        bool intersects = gridGeom.intersects(userExtentWebMercator);

        // 更新属性
        QgsAttributes attrs = feature.attributes();
        if (attrs.size() > 5) {
            attrs[5] = intersects ? 1 : 0;
        } else {
            while (attrs.size() < 6) {
                attrs.append(0);
            }
            attrs[5] = intersects ? 1 : 0;
        }
        feature.setAttributes(attrs);
    });

    // 对于相交的网格，递归检查子网格
    for (auto &feature : features) {
        QgsAttributes attrs = feature.attributes();
        if (attrs.size() > 5 && attrs[5].toInt() == 1 &&
            level < AppConstants::MAX_ZOOM_LEVEL) {
            checkPreGeneratedChildGrids(level, feature, userExtentWebMercator);
        }
    }
}

void GridManager::checkPreGeneratedChildGrids(int parentLevel, const QgsFeature &parentFeature,
                                            const QgsRectangle &userExtentWebMercator)
{
    int childLevel = parentLevel + 1;

    if (childLevel > AppConstants::PRE_GENERATED_MAX_LEVEL) {
        return;
    }

    // 检查是否有所需的数据
    if (!m_globalGridByLevel.contains(childLevel) || !m_levelSpatialIndexes.contains(childLevel)) {
        return;
    }

    // 获取父网格的行列信息
    QgsAttributes parentAttrs = parentFeature.attributes();
    if (parentAttrs.size() < 3) return;

    int parentRow = parentAttrs[1].toInt();
    int parentCol = parentAttrs[2].toInt();

    // 每个父网格有4个子网格 (2x2)
    int startRow = parentRow * 2;
    int startCol = parentCol * 2;

    // 使用空间索引快速找到可能相交的子网格
    QList<QgsFeatureId> candidateIds = m_levelSpatialIndexes[childLevel]->intersects(userExtentWebMercator);
    QgsFeatureList &childFeatures = m_globalGridByLevel[childLevel];

    // 获取用户范围的边界框
    QgsRectangle userBBox = userExtentWebMercator;

    // 只检查属于当前父网格的子网格
    for (QgsFeatureId id : candidateIds) {
        // 找到对应的要素
        for (auto &childFeature : childFeatures) {
            if (childFeature.id() == id) {
                QgsAttributes childAttrs = childFeature.attributes();
                if (childAttrs.size() < 3) continue;

                int childRow = childAttrs[1].toInt();
                int childCol = childAttrs[2].toInt();

                // 检查是否属于当前父网格
                if (childRow >= startRow && childRow < startRow + 2 &&
                    childCol >= startCol && childCol < startCol + 2) {

                    // 先进行边界框快速检查
                    QgsRectangle childBBox = childFeature.geometry().boundingBox();
                    if (!childBBox.intersects(userBBox)) {
                        // 边界框不相交，肯定不相交
                        while (childAttrs.size() < 6) {
                            childAttrs.append(0);
                        }
                        childAttrs[5] = 0;
                        childFeature.setAttributes(childAttrs);
                        continue;
                    }

                    // 边界框相交，进行精确的几何检查
                    QgsGeometry gridGeom = childFeature.geometry();
                    bool intersects = gridGeom.intersects(userExtentWebMercator);

                    // 更新属性
                        if (childAttrs.size() > 5) {
                             childAttrs[5] = intersects ? 1 : 0;
                        } else {
                            while (childAttrs.size() < 6) {
                                childAttrs.append(0);
                            }
                             childAttrs[5] = intersects ? 1 : 0;
                        }
                        childFeature.setAttributes(childAttrs);

                    // 如果相交且不是预生成最大层级，递归检查子网格
                    if (intersects && childLevel < AppConstants::MAX_ZOOM_LEVEL) {
                        checkPreGeneratedChildGrids(childLevel, childFeature, userExtentWebMercator);
                    }

                    break; // 找到后跳出内层循环
                }
            }
        }
    }
}
void GridManager::checkGridLevelForUserExtent(int level,const QgsRectangle &m_userExtentWebMercator)  //使用
{
    if (!m_globalGridByLevel.contains(level)) {
        return;
    }

    QgsFeatureList &features = m_globalGridByLevel[level];

    // 如果是第0级，检查所有网格
    if (level == 0) {
        for (auto &feature : features) {
            QgsGeometry gridGeom = feature.geometry();
             bool intersects = gridGeom.intersects(m_userExtentWebMercator);

            // 更新属性
            QgsAttributes attrs = feature.attributes();
            if (attrs.size() > 5) {
                 attrs[5] = intersects ? 1 : 0;
            } else {
                while (attrs.size() < 6) {
                    attrs.append(0);
                }
                 attrs[5] = intersects ? 1 : 0;
            }
            feature.setAttributes(attrs);

            // 如果相交且不是最大层级，递归检查子网格
             if (intersects && level < AppConstants::MAX_ZOOM_LEVEL) {
                 checkChildGridsForUserExtent(level, feature,m_userExtentWebMercator);
             }
        }
    } else {
        // 对于更高级别，只检查上一层级中标记为相交的网格对应的子网格
        QList<QgsFeature> parentFeatures = getParentFeaturesWithUserExtent(level - 1);

        for (const auto &parentFeature : parentFeatures) {
            checkChildGridsForUserExtent(level - 1, parentFeature,m_userExtentWebMercator);
        }
    }
}

void GridManager::checkChildGridsForUserExtent(int parentLevel, const QgsFeature &parentFeature,const QgsRectangle &m_userExtentWebMercator) //使用
{
    int childLevel = parentLevel + 1;
    if (childLevel > AppConstants::PRE_GENERATED_MAX_LEVEL) {
        return;
    }
    if (!m_globalGridByLevel.contains(childLevel) || childLevel > AppConstants::MAX_ZOOM_LEVEL) {
        return;
    }

    // 获取父网格的行列信息
    QgsAttributes parentAttrs = parentFeature.attributes();
    if (parentAttrs.size() < 3) return; // 确保有zoom, row, col字段

    int parentRow = parentAttrs[1].toInt();
    int parentCol = parentAttrs[2].toInt();

    // 每个父网格有4个子网格 (2x2)
    int startRow = parentRow * 2;
    int startCol = parentCol * 2;

    QgsFeatureList &childFeatures = m_globalGridByLevel[childLevel];

    // 检查所有可能的子网格
    for (int row = startRow; row < startRow + 2; row++) {
        for (int col = startCol; col < startCol + 2; col++) {
            // 找到对应的子网格
            for (auto &childFeature : childFeatures) {
                QgsAttributes childAttrs = childFeature.attributes();
                if (childAttrs.size() < 3) continue;

                int childRow = childAttrs[1].toInt();
                int childCol = childAttrs[2].toInt();

                if (childRow == row && childCol == col) {
                    QgsGeometry gridGeom = childFeature.geometry();
                     bool intersects = gridGeom.intersects(m_userExtentWebMercator);

                    // 更新属性
                    if (childAttrs.size() > 5) {
                         childAttrs[5] = intersects ? 1 : 0;
                    } else {
                        while (childAttrs.size() < 6) {
                            childAttrs.append(0);
                        }
                         childAttrs[5] = intersects ? 1 : 0;
                    }
                    childFeature.setAttributes(childAttrs);

                    // 如果相交且不是最大层级，递归检查子网格
                     if (intersects && childLevel < AppConstants::MAX_ZOOM_LEVEL) {
                         checkChildGridsForUserExtent(childLevel, childFeature,m_userExtentWebMercator);
                     }

                    break; // 找到对应网格后跳出内层循环
                }
            }
        }
    }
}

void GridManager::resetGridAttributes()
{
    for (auto &level : m_globalGridByLevel.keys()) {
        QgsFeatureList &features = m_globalGridByLevel[level];
        for (auto &feature : features) {
            QgsAttributes attrs = feature.attributes();
            if (attrs.size() > 5) {
                attrs[5] = 0; // 重置为不包含用户范围
            } else {
                while (attrs.size() < 6) {
                    attrs.append(0);
                }
                attrs[5] = 0;
            }
            feature.setAttributes(attrs);
        }
    }
}

void GridManager::generateDynamicGridsFromOptimalLevel(int optimalLevel, const QgsRectangle& userExtent,int m_targetLevel)
{
    // 清空之前动态生成的网格
    m_dynamicGridByLevel.clear();

    // 如果适宜层级已经大于等于最大层级，不需要生成动态网格
    if (optimalLevel >= AppConstants::MAX_ZOOM_LEVEL) {
        return;
    }

    // 获取适宜层级中与用户范围相交的网格
    QgsFeatureList optimalFeatures;
    if (optimalLevel <= AppConstants::PRE_GENERATED_MAX_LEVEL && m_globalGridByLevel.contains(optimalLevel)) {
        optimalFeatures = m_globalGridByLevel[optimalLevel];
    } else if (optimalLevel > AppConstants::PRE_GENERATED_MAX_LEVEL && m_dynamicGridByLevel.contains(optimalLevel)) {
        optimalFeatures = m_dynamicGridByLevel[optimalLevel];
    } else {
        qDebug() << "无法找到层级" << optimalLevel << "的网格数据";
        return;
    }

    qDebug() << "在层级" << optimalLevel << "中找到" << optimalFeatures.size() << "个网格";

    int intersectingCount = 0;

    // 遍历适宜层级的每个网格，只对与用户范围相交的网格进行细分
    for (const QgsFeature& feature : optimalFeatures) {
        QgsAttributes attrs = feature.attributes();
        // 确保属性数量正确
        if (attrs.size() > 5 && attrs[5].toInt() == 1) { // 检查是否包含用户范围
            intersectingCount++;
            // 递归生成子网格直到最大层级
            generateDynamicGridsRecursive(optimalLevel, feature, userExtent,m_targetLevel);
        }
    }

    qDebug() << "在层级" << optimalLevel << "中找到" << intersectingCount << "个与用户范围相交的网格";
    qDebug() << "动态网格生成完成，最高层级:" << m_targetLevel;
}

void GridManager::generateDynamicGridsRecursive(int currentLevel, const QgsFeature& parentFeature, const QgsRectangle& userExtent,int m_targetLevel)
{
    // 如果已经达到最大层级，停止递归
    if (currentLevel >=m_targetLevel) {
        return;
    }

    // 获取父网格的几何和属性
    QgsGeometry parentGeom = parentFeature.geometry();
    QgsAttributes parentAttrs = parentFeature.attributes();

    if (parentAttrs.size() < 3) return; // 确保有zoom, row, col字段

    int parentZoom = parentAttrs[0].toInt();
    int parentRow = parentAttrs[1].toInt();
    int parentCol = parentAttrs[2].toInt();

    // 计算子网格层级
    int childLevel = parentZoom + 1;

    // 如果这个层级的网格还没有生成，初始化一个空列表
    if (!m_dynamicGridByLevel.contains(childLevel)) {
        m_dynamicGridByLevel[childLevel] = QgsFeatureList();
    }

    // 获取父网格的范围
    QgsRectangle parentRect = parentGeom.boundingBox();
    double parentWidth = parentRect.width();
    double parentHeight = parentRect.height();

    // 计算子网格的大小
    double childWidth = parentWidth / 2.0;
    double childHeight = parentHeight / 2.0;

    // 生成4个子网格
    for (int rowOffset = 0; rowOffset < 2; rowOffset++) {
        for (int colOffset = 0; colOffset < 2; colOffset++) {
            int childRow = parentRow * 2 + rowOffset;
            int childCol = parentCol * 2 + colOffset;

            // 计算子网格的范围
            double x1 = parentRect.xMinimum() + colOffset * childWidth;
            double y1 = parentRect.yMaximum() - (rowOffset + 1) * childHeight;
            double x2 = x1 + childWidth;
            double y2 = y1 + childHeight;

            QgsRectangle childRect(x1, y1, x2, y2);
            QgsFeature childFeature;
            childFeature.setGeometry(QgsGeometry::fromRect(childRect));

            // 检查是否与用户范围相交
            bool intersects = childRect.intersects(userExtent);

            // 设置属性
            childFeature.setAttributes(QgsAttributes()
                << childLevel << childRow << childCol << childWidth << childHeight
                << (intersects ? 1 : 0) << std::numeric_limits<double>::max());

            // 添加到动态网格列表
            m_dynamicGridByLevel[childLevel].append(childFeature);

            // 只有相交的网格才继续划分
            if (intersects && childLevel < AppConstants::MAX_ZOOM_LEVEL) {
                generateDynamicGridsRecursive(childLevel, childFeature, userExtent,m_targetLevel);
            }
        }
    }
}

QgsFeatureList GridManager::getGridFeaturesForLevel(int level) const
{
    if (level <= AppConstants::PRE_GENERATED_MAX_LEVEL && m_globalGridByLevel.contains(level)) {
        return m_globalGridByLevel[level];
    } else if (level > AppConstants::PRE_GENERATED_MAX_LEVEL && m_dynamicGridByLevel.contains(level)) {
        return m_dynamicGridByLevel[level];
    }
    return QgsFeatureList();
}

bool GridManager::hasGridDataForLevel(int level) const
{
    return (level <= AppConstants::PRE_GENERATED_MAX_LEVEL && m_globalGridByLevel.contains(level)) ||
           (level > AppConstants::PRE_GENERATED_MAX_LEVEL && m_dynamicGridByLevel.contains(level));
}
