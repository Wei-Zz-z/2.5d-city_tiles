#ifndef OSGBMAPPER_H
#define OSGBMAPPER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include "appconstants.h"
class OSGBMapper : public QObject
{
    Q_OBJECT

public:
    explicit OSGBMapper(QObject *parent = nullptr);

    // 初始化网格到OSGB的映射
    void initializeMapping();

    // 从当前层级开始更新映射
    void updateMappingFromCurrentLevel(int currentLevel, int endLevel,int current_state,QMap<int,int> maprule = QMap<int, int>());

    // 设置OSGB层级范围
    void setOsgbLevelRange(int minLevel, int maxLevel);

    // 获取指定网格层级对应的OSGB层级
    int getOsgbLevelForGridLevel(int gridLevel) const;

    // 获取完整的映射关系
    const QMap<int, int>& mapping() const { return m_gridToOsgbMap; }

    // 获取最小OSGB层级
    int minOsgbLevel() const { return m_minOsgbLevel; }

    // 获取最大OSGB层级
    int maxOsgbLevel() const { return m_maxOsgbLevel; }

private:
    // 创建均匀分布的映射关系

    // 成员变量
    QMap<int, int> m_gridToOsgbMap;
    int m_minOsgbLevel;
    int m_maxOsgbLevel;

    // 常量
};

#endif // OSGBMAPPER_H
