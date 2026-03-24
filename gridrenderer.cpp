#include "gridrenderer.h"
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsrulebasedrenderer.h>
#include <qgslinesymbollayer.h>
#include <QColor>
#include "appconstants.h"
GridRenderer::GridRenderer(QObject *parent)
    : QObject(parent)
{
}

void GridRenderer::setupRuleBasedRenderer(QgsVectorLayer *layer)
{
    if (!layer || !layer->isValid()) {
        qWarning() << "无效的图层，无法设置渲染器";
        return;
    }

    // 创建根规则
    QgsRuleBasedRenderer::Rule *rootRule = createRootRule();

    if (!rootRule) {
        qWarning() << "无法创建根规则";
        return;
    }

    // 创建基于规则的渲染器
    QgsRuleBasedRenderer *renderer = new QgsRuleBasedRenderer(rootRule);

    // 应用渲染器到图层
    layer->setRenderer(renderer);

    qDebug() << "基于规则的网格渲染器已设置";
}

//QgsSymbol* GridRenderer::createHighlightSymbol()
//{
//    // 创建高亮符号（包含用户范围的网格）
//    QgsSymbol *highlightSymbol = QgsSymbol::defaultSymbol(QgsWkbTypes::PolygonGeometry);
//    if (!highlightSymbol) {
//        qWarning() << "无法创建高亮符号";
//        return nullptr;
//    }

//    // 设置半透明红色填充
//    highlightSymbol->setColor(QColor(255, 0, 0, 50));

//    // 设置红色边框
//    QgsSimpleLineSymbolLayer *highlightLineLayer = new QgsSimpleLineSymbolLayer();
//    if (!highlightLineLayer) {
//        delete highlightSymbol;
//        qWarning() << "无法创建高亮线符号层";
//        return nullptr;
//    }

//    highlightLineLayer->setColor(QColor(255, 0, 0, 200));
//    highlightLineLayer->setWidth(1.0);

//    // 替换默认的符号层
//    highlightSymbol->changeSymbolLayer(0, highlightLineLayer);

//    return highlightSymbol;
//}

QgsSymbol* GridRenderer::createDefaultSymbol()
{
    // 创建默认符号（不包含用户范围的网格）
    QgsSymbol *defaultSymbol = QgsSymbol::defaultSymbol(QgsWkbTypes::PolygonGeometry);
    if (!defaultSymbol) {
        qWarning() << "无法创建默认符号";
        return nullptr;
    }

    // 设置透明填充
    defaultSymbol->setColor(QColor(0, 0, 0, 0));

    // 设置灰色边框
    QgsSimpleLineSymbolLayer *defaultLineLayer = new QgsSimpleLineSymbolLayer();
    if (!defaultLineLayer) {
        delete defaultSymbol;
        qWarning() << "无法创建默认线符号层";
        return nullptr;
    }

    defaultLineLayer->setColor(QColor(200, 200, 200, 100));
    defaultLineLayer->setWidth(0.3);

    // 替换默认的符号层
    defaultSymbol->changeSymbolLayer(0, defaultLineLayer);

    return defaultSymbol;
}

QgsRuleBasedRenderer::Rule* GridRenderer::createRootRule()
{
    // 创建空的根规则
    QgsRuleBasedRenderer::Rule *rootRule = new QgsRuleBasedRenderer::Rule(nullptr);
    if (!rootRule) {
        qWarning() << "无法创建根规则";
        return nullptr;
    }

    // 添加高亮规则
//    QgsRuleBasedRenderer::Rule *highlightRule = createHighlightRule();
//    if (highlightRule) {
//        rootRule->appendChild(highlightRule);
//    }

    // 添加默认规则
    QgsRuleBasedRenderer::Rule *defaultRule = createDefaultRule();
    if (defaultRule) {
        rootRule->appendChild(defaultRule);
    }

    return rootRule;
}

//QgsRuleBasedRenderer::Rule* GridRenderer::createHighlightRule()
//{
//    // 创建高亮符号
//    QgsSymbol *highlightSymbol = createHighlightSymbol();
//    if (!highlightSymbol) {
//        qWarning() << "无法创建高亮规则：符号创建失败";
//        return nullptr;
//    }

//    // 创建高亮规则：包含用户范围的网格
//    QgsRuleBasedRenderer::Rule *highlightRule = new QgsRuleBasedRenderer::Rule(
//        highlightSymbol,
//        10000,  // 缩放范围最小值
//        0,      // 缩放范围最大值（0表示无限制）
//        "\"contains_user_extent\" = 1",  // 过滤表达式
//        "包含用户范围"  // 标签
//    );

//    if (!highlightRule) {
//        delete highlightSymbol;
//        qWarning() << "无法创建高亮规则";
//        return nullptr;
//    }

//    return highlightRule;
//}

QgsRuleBasedRenderer::Rule* GridRenderer::createDefaultRule()
{
    // 创建默认符号
    QgsSymbol *defaultSymbol = createDefaultSymbol();
    if (!defaultSymbol) {
        qWarning() << "无法创建默认规则：符号创建失败";
        return nullptr;
    }

    // 创建默认规则：其他网格
    QgsRuleBasedRenderer::Rule *defaultRule = new QgsRuleBasedRenderer::Rule(
        defaultSymbol,
        10000,  // 缩放范围最小值
        0,      // 缩放范围最大值（0表示无限制）
        "ELSE",  // 过滤表达式（其他所有情况）
        "其他网格"  // 标签
    );

    if (!defaultRule) {
        delete defaultSymbol;
        qWarning() << "无法创建默认规则";
        return nullptr;
    }

    return defaultRule;
}
