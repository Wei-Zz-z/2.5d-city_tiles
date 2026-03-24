#ifndef COORDINATECONVERTER_H
#define COORDINATECONVERTER_H

#include <QObject>
#include <qgsrectangle.h>
#include <qgscoordinatereferencesystem.h>

class QWidget;
class QDialog;

class CoordinateConverter : public QObject
{
    Q_OBJECT

public:
    explicit CoordinateConverter(QObject *parent = nullptr);

    // 显示对话框获取用户输入的经纬度范围
    QgsRectangle getUserExtentFromDialog(QWidget *parent = nullptr);

    // 坐标转换方法
    QgsRectangle transformToWebMercator(const QgsRectangle &wgs84Extent);
    QgsRectangle transformToWGS84(const QgsRectangle &webMercatorExtent);

    // 经纬度格式转换方法
    static double dmsToDecimal(const QString &dms);
    static QString decimalToDms(double decimal, bool isLongitude);

    // 验证范围有效性
    bool validateExtent(const QgsRectangle &extent);
    QgsRectangle convertListToWebMercator(const QList<double> &coords);

signals:
    void extentConverted(const QgsRectangle &extent);
    void conversionError(const QString &message);

private:
    // 创建范围输入对话框
    QDialog* createExtentDialog(QWidget *parent);

    // 从对话框获取范围
    QgsRectangle getExtentFromDialog(QDialog *dialog);
};

#endif // COORDINATECONVERTER_H
