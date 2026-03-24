#include "reader_xml.h"
#include <qgscoordinatetransform.h>
#include <qgspoint.h>
#include <qgsgeometry.h>
#include <QDebug>
#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <qgsproject.h>

ReaderXML::ReaderXML(QObject *parent) : QObject(parent), m_error("")
{
}

QList<QVariant> ReaderXML::readXMLFile(const QString& filePath)
{
    QList<QVariant> result;
    m_error = "";

    // 检查文件是否存在
    if (!QFile::exists(filePath)) {
        m_error = QString("XML文件不存在: %1").arg(filePath);
        qWarning() << m_error;
        return result;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = QString("无法打开XML文件: %1").arg(filePath);
        qWarning() << m_error;
        return result;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        m_error = QString("XML解析错误: %1 (行: %2, 列: %3)").arg(errorMsg).arg(errorLine).arg(errorColumn);
        qWarning() << m_error;
        file.close();
        return result;
    }

    file.close();

    // 获取根元素
    QDomElement root = doc.documentElement();
    if (root.tagName() != "ModelMetadata") {
        m_error = "根元素不是ModelMetadata";
        qWarning() << m_error;
        return result;
    }

    // 检查版本
    QString version = root.attribute("version", "1");
    qDebug() << "XML版本:" << version;

    // 查找SRS元素
    QDomElement srsElement = root.firstChildElement("SRS");
    if (srsElement.isNull()) {
        m_error = "未找到SRS元素";
        qWarning() << m_error;
        return result;
    }

    QString srsValue = srsElement.text().trimmed();
    qDebug() << "SRS值:" << srsValue;

    // 查找SRSOrigin元素
    QDomElement originElement = root.firstChildElement("SRSOrigin");
    if (originElement.isNull()) {
        m_error = "未找到SRSOrigin元素";
        qWarning() << m_error;
        return result;
    }

    QString originValue = originElement.text().trimmed();
    qDebug() << "SRSOrigin值:" << originValue;

    // 解析SRS类型和参数
    QString srsType, srsParam1, srsParam2;
    if (srsValue.contains(":")) {
        QStringList srsParts = srsValue.split(":");
        srsType = srsParts[0].trimmed();
        if (srsParts.size() > 1) {
            QString params = srsParts[1];
            if (params.contains(",")) {
                QStringList paramList = params.split(",");
                srsParam1 = paramList[0].trimmed();
                srsParam2 = paramList.size() > 1 ? paramList[1].trimmed() : "";
            } else {
                srsParam1 = params.trimmed();
            }
        }
    } else {
        srsType = srsValue;
    }
    // 将结果存储在列表中
    result << filePath;           // 文件路径
    result << version;            // 版本号
    result << srsValue;
    qDebug()<<"------------------------------"<<srsValue<<endl;
    if(srsType == "ENU"){
        // SRS类型 (ENU/EPSG等)
          result << srsParam1;          // SRS参数1
          result << srsParam2;          // SRS参数2
    }else{
        QStringList originValueList = originValue.split(',');
        double Lat = 0.0;
        double Lon = 0.0;
        double z = 0.0;
        // 检查是否有足够的数据
        if(originValueList.size() >= 3) {
            // 转换为double并存储
            Lat =originValueList[0].toDouble();
            Lon =originValueList[1].toDouble();
            z = originValueList[2].toDouble();

            // 现在你可以使用这三个double值
            // value1, value2, value3 分别对应三个数字
        }
        QgsPointXY point(Lat,Lon);
            // 创建坐标转换：从Web墨卡托到WGS84
        qDebug()<<srsValue<<endl;
        QgsCoordinateTransform transform(
            QgsCoordinateReferenceSystem(srsValue), // Web墨卡托
            QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84
            QgsProject::instance()
        );
        QgsPointXY Point_WGS84 = transform.transform(point);
        result << Point_WGS84.x();
        result << Point_WGS84.y();

    }

    qDebug() << "XML文件读取成功";
    return result;
}
