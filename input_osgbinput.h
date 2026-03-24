#ifndef INPUT_OSGBINPUT_H
#define INPUT_OSGBINPUT_H

#include <QObject>
#include <QDir>
#include <QList>
#include <QVariant>
#include <QString>
#include <qgscoordinatetransform.h>
#include <qgspoint.h>
#include <qgsgeometry.h>
#include <qgsproject.h>
// 先包含OSG头文件
#include <osgDB/ReadFile>
#include <osg/Node>
#include <osg/BoundingBox>
#include <osg/ComputeBoundsVisitor>
#include <osg/BoundingSphere>
#include <qgscoordinatetransform.h>
#include <qgspoint.h>
#include <qgsgeometry.h>
#include <qgsproject.h>

// 只包含必要的OSG前向声明
class InputOSGBInput : public QObject
{
    Q_OBJECT

public:
    explicit InputOSGBInput(QObject *parent = nullptr);

    double get_height_of_osgb();
    // 读取指定目录下所有OSGB文件的包围盒信息
    QList<double> readOSGBBoundingBoxes(const QString& dataPath,QString srctype,QgsPointXY originpoint);
    QList<double> calculateGlobalBoundingBox(const QList<QList<QVariant>>& allBoundingBoxes);


    osg::BoundingBox getCityBox(){
        return this->bbox_all;
    }
private:

    QFileInfoList fileList;
    // 处理单个Tile目录
    osg::BoundingBox bbox_all;
    // 处理单个Tile目录
    void processTileDirectory(const QString& tilePath);

    //坐标转换
    QgsPointXY convertLocalToWGS84(const QgsPointXY& localPoint,const QgsPointXY& originWGS84);

};

#endif // INPUT_OSGBINPUT_H
