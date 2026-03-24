#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include <QObject>
#include <qgsvectorlayer.h>
#include <qgsrulebasedrenderer.h>
#include "appconstants.h"
class GridRenderer : public QObject
{
    Q_OBJECT

public:
    explicit GridRenderer(QObject *parent = nullptr);

    // 设置基于规则的网格渲染器
    void setupRuleBasedRenderer(QgsVectorLayer *layer);

    // 创建高亮符号（包含用户范围的网格）
//    static QgsSymbol* createHighlightSymbol();

    // 创建默认符号（不包含用户范围的网格）
    static QgsSymbol* createDefaultSymbol();

private:
    // 创建根规则
    QgsRuleBasedRenderer::Rule* createRootRule();

    // 创建高亮规则
//    QgsRuleBasedRenderer::Rule* createHighlightRule();

    // 创建默认规则
    QgsRuleBasedRenderer::Rule* createDefaultRule();
};

#endif // GRIDRENDERER_H
