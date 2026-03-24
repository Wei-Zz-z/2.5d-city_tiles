#ifndef GENERATEXML_H
#define GENERATEXML_H

#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include <QRectF>
class generatexml
{
public:
    generatexml();

    bool generateCGCS2000WMTSCapabilities(const QString& xmlPath,   //输出路径
                                          const QString& serviceUrl,//localhost:8066
                                          const QString& tileDir,   //tiles
                                          const QString& layerName, //
                                          const QString& layerTitle,
                                          const QVector<double>& bbox);
};

#endif // GENERATEXML_H
