#include "osgbmapper.h"
#include <QDebug>
#include "appconstants.h"
OSGBMapper::OSGBMapper(QObject *parent)
    : QObject(parent)
    , m_minOsgbLevel(15)
    , m_maxOsgbLevel(22)
{
    initializeMapping();
}

void OSGBMapper::initializeMapping()
{
    m_gridToOsgbMap.clear();

    // 计算网格层级范围 (从0到MAX_ZOOM_LEVEL)
    int gridLevels =AppConstants::MAX_ZOOM_LEVEL + 1;
    int osgbLevels = m_maxOsgbLevel - m_minOsgbLevel + 1;

    // 如果网格层级多于OSGB层级
    if (gridLevels > osgbLevels) {
        // 计算需要跳过的网格层级数量
        int levelsToSkip = gridLevels - osgbLevels;

        // 使用更均匀的跳过策略
        // 计算跳过间隔，使跳过的层级均匀分布
        double skipInterval = static_cast<double>(gridLevels) / (levelsToSkip + 1);

        int osgbLevel = m_minOsgbLevel;
        int lastAssignedOsgbLevel = m_minOsgbLevel;

        // 创建一个集合来记录需要跳过的层级
        QSet<int> skipLevels;
        for (int i = 1; i <= levelsToSkip; i++) {
            int skipLevel = static_cast<int>(i * skipInterval);
            if (skipLevel >= gridLevels) skipLevel = gridLevels - 1;
            skipLevels.insert(skipLevel);
        }

        for (int gridLevel = 0; gridLevel <= AppConstants::MAX_ZOOM_LEVEL; gridLevel++) {
            // 如果当前层级需要跳过，分配与前一级相同的OSGB层级
            if (skipLevels.contains(gridLevel)) {
                m_gridToOsgbMap[gridLevel] = lastAssignedOsgbLevel;
            } else {
                // 确保不超过OSGB最大层级
                if (osgbLevel <= m_maxOsgbLevel) {
                    m_gridToOsgbMap[gridLevel] = osgbLevel;
                    lastAssignedOsgbLevel = osgbLevel;
                    osgbLevel++;
                } else {
                    // 如果OSGB层级用完，分配最大OSGB层级
                    m_gridToOsgbMap[gridLevel] = m_maxOsgbLevel;
                    lastAssignedOsgbLevel = m_maxOsgbLevel;
                }
            }
        }
    }
    // 如果OSGB层级多于网格层级
    else if (osgbLevels > gridLevels) {
        // 计算需要跳过的OSGB层级数量
        int osgbLevelsToSkip = osgbLevels - gridLevels;

        // 使用更均匀的跳过策略
        // 计算跳过间隔，使跳过的OSGB层级均匀分布
        double skipInterval = static_cast<double>(osgbLevels) / (osgbLevelsToSkip + 1);

        // 创建一个集合来记录需要跳过的OSGB层级
        QSet<int> skipOsgbLevels;
        for (int i = 1; i <= osgbLevelsToSkip; i++) {
            int skipLevel = static_cast<int>(m_minOsgbLevel + i * skipInterval);
            if (skipLevel > m_maxOsgbLevel) skipLevel = m_maxOsgbLevel;
            skipOsgbLevels.insert(skipLevel);
        }

        int osgbLevel = m_minOsgbLevel;
        for (int gridLevel = 0; gridLevel <= AppConstants::MAX_ZOOM_LEVEL; gridLevel++) {
            // 跳过需要跳过的OSGB层级
            while (skipOsgbLevels.contains(osgbLevel) && osgbLevel <= m_maxOsgbLevel) {
                osgbLevel++;
            }

            // 确保不超过OSGB最大层级
            if (osgbLevel <= m_maxOsgbLevel) {
                m_gridToOsgbMap[gridLevel] = osgbLevel;
                osgbLevel++;
            } else {
                // 如果OSGB层级用完，分配最大OSGB层级
                m_gridToOsgbMap[gridLevel] = m_maxOsgbLevel;
            }
        }
    }
    // 如果层级数量相等
    else {
        // 直接一对一映射
        for (int gridLevel = 0; gridLevel <= AppConstants::MAX_ZOOM_LEVEL; gridLevel++) {
            m_gridToOsgbMap[gridLevel] = m_minOsgbLevel + gridLevel;
        }
    }

    qDebug() << "改进后的网格到OSGB映射已初始化:" << m_gridToOsgbMap;

    // 添加详细日志输出
    for (int i = 0; i <= AppConstants::MAX_ZOOM_LEVEL; i++) {
        if (m_gridToOsgbMap.contains(i)) {
            qDebug() << "网格层级" << i << "-> OSGB层级" << m_gridToOsgbMap[i];
        } else {
            qDebug() << "网格层级" << i << "没有映射到任何OSGB层级";
        }
    }
}

void OSGBMapper::updateMappingFromCurrentLevel(int currentLevel, int endLevel,int mapping_state,QMap<int,int> maprule)
{
    if(mapping_state != -1){
        m_gridToOsgbMap = maprule;
        qDebug()<<m_gridToOsgbMap;
        qDebug()<<"目前正在使用重置后的";
    }else{
        m_gridToOsgbMap.clear();
        qDebug()<<"目前正在使用重置前的";
        // 计算从当前层级到最大层级的网格数量
        int gridLevelsFromCurrent = endLevel - currentLevel + 1;
        int osgbLevels = m_maxOsgbLevel - m_minOsgbLevel + 1;

        qDebug() << "计算映射: 当前层级" << currentLevel << "到最大层级" << endLevel;
        qDebug() << "网格层级数量:" << gridLevelsFromCurrent << "OSGB层级数量:" << osgbLevels;

        // 如果从当前层级开始的网格数量多于OSGB层级
        if (gridLevelsFromCurrent > osgbLevels) {
            // 计算需要跳过的网格层级数量
            int levelsToSkip = gridLevelsFromCurrent - osgbLevels;

            // 使用更均匀的跳过策略
            // 计算跳过间隔，使跳过的层级均匀分布
            double skipInterval = static_cast<double>(gridLevelsFromCurrent) / (levelsToSkip + 1);

            int osgbLevel = m_minOsgbLevel;
            int lastAssignedOsgbLevel = m_minOsgbLevel;

            // 创建一个集合来记录需要跳过的层级
            QSet<int> skipLevels;
            for (int i = 1; i <= levelsToSkip; i++) {
                int skipLevel = currentLevel + static_cast<int>(i * skipInterval);
                if (skipLevel > endLevel) skipLevel =endLevel;
                skipLevels.insert(skipLevel);
                qDebug() << "将跳过网格层级:" << skipLevel;
            }

            for (int gridLevel = currentLevel; gridLevel <= endLevel; gridLevel++) {
                // 如果当前层级需要跳过，分配与前一级相同的OSGB层级
                if (skipLevels.contains(gridLevel)) {
                    m_gridToOsgbMap[gridLevel] = lastAssignedOsgbLevel;
                    qDebug() << "跳过网格层级" << gridLevel << "，分配OSGB层级" << lastAssignedOsgbLevel;
                } else {
                    // 确保不超过OSGB最大层级
                    if (osgbLevel <= m_maxOsgbLevel) {
                        m_gridToOsgbMap[gridLevel] = osgbLevel;
                        lastAssignedOsgbLevel = osgbLevel;
                        qDebug() << "映射网格层级" << gridLevel << "到OSGB层级" << osgbLevel;
                        osgbLevel++;
                    } else {
                        // 如果OSGB层级用完，分配最大OSGB层级
                        m_gridToOsgbMap[gridLevel] = m_maxOsgbLevel;
                        lastAssignedOsgbLevel = m_maxOsgbLevel;
                        qDebug() << "OSGB层级用完，映射网格层级" << gridLevel << "到最大OSGB层级" << m_maxOsgbLevel;
                    }
                }
            }
        }
        // 如果OSGB层级多于从当前层级开始的网格数量
        else if (osgbLevels > gridLevelsFromCurrent) {
            // 计算需要跳过的OSGB层级数量
            int osgbLevelsToSkip = osgbLevels - gridLevelsFromCurrent;

            // 使用更均匀的跳过策略
            // 计算跳过间隔，使跳过的OSGB层级均匀分布
            double skipInterval = static_cast<double>(osgbLevels) / (osgbLevelsToSkip + 1);

            // 创建一个集合来记录需要跳过的OSGB层级
            QSet<int> skipOsgbLevels;
            for (int i = 1; i <= osgbLevelsToSkip; i++) {
                int skipLevel = static_cast<int>(m_minOsgbLevel + i * skipInterval);
                if (skipLevel > m_maxOsgbLevel) skipLevel = m_maxOsgbLevel;
                skipOsgbLevels.insert(skipLevel);
                qDebug() << "将跳过OSGB层级:" << skipLevel;
            }

            int osgbLevel = m_minOsgbLevel;
            for (int gridLevel = currentLevel; gridLevel <= endLevel; gridLevel++) {
                // 跳过需要跳过的OSGB层级
                while (skipOsgbLevels.contains(osgbLevel) && osgbLevel <= m_maxOsgbLevel) {
                    qDebug() << "跳过OSGB层级:" << osgbLevel;
                    osgbLevel++;
                }

                // 确保不超过OSGB最大层级
                if (osgbLevel <= m_maxOsgbLevel) {
                    m_gridToOsgbMap[gridLevel] = osgbLevel;
                    qDebug() << "映射网格层级" << gridLevel << "到OSGB层级" << osgbLevel;
                    osgbLevel++;
                } else {
                    // 如果OSGB层级用完，分配最大OSGB层级
                    m_gridToOsgbMap[gridLevel] = m_maxOsgbLevel;
                    qDebug() << "OSGB层级用完，映射网格层级" << gridLevel << "到最大OSGB层级" << m_maxOsgbLevel;
                }
            }
        }
        // 如果层级数量相等
        else {
            // 直接一对一映射
            int osgbLevel = m_minOsgbLevel;
            for (int gridLevel = currentLevel; gridLevel <= endLevel; gridLevel++) {
                m_gridToOsgbMap[gridLevel] = osgbLevel;
                qDebug() << "一对一映射网格层级" << gridLevel << "到OSGB层级" << osgbLevel;
                osgbLevel++;
            }
        }

        qDebug() << "从当前层级开始的改进网格到OSGB映射已更新:" << m_gridToOsgbMap;

    }

}

void OSGBMapper::setOsgbLevelRange(int minLevel, int maxLevel)
{
    if (minLevel < 0 || maxLevel < minLevel) {
        qWarning() << "无效的OSGB层级范围:" << minLevel << "-" << maxLevel;
        return;
    }

    m_minOsgbLevel = minLevel;
    m_maxOsgbLevel = maxLevel;

    // 重新初始化映射
    initializeMapping();

    qDebug() << "OSGB层级范围已设置为:" << m_minOsgbLevel << "-" << m_maxOsgbLevel;
}

int OSGBMapper::getOsgbLevelForGridLevel(int gridLevel) const
{
    if (m_gridToOsgbMap.contains(gridLevel)) {
        return m_gridToOsgbMap[gridLevel];
    }

    // 如果没有映射关系，使用默认值
    if (gridLevel >= 0 && gridLevel <= AppConstants::MAX_ZOOM_LEVEL) {
        int osgbLevel = m_minOsgbLevel + gridLevel;
        if (osgbLevel > m_maxOsgbLevel) osgbLevel = m_maxOsgbLevel;
        if (osgbLevel < m_minOsgbLevel) osgbLevel = m_minOsgbLevel;
        return osgbLevel;
    }

    // 无效的网格层级
    qWarning() << "无效的网格层级:" << gridLevel;
    return -1;
}

