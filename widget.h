#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QFileDialog>
#include <osgQOpenGL/osgQOpenGLWidget>
#include <osgViewer/Viewer>
#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osgGA/TrackballManipulator>
#include "txtexport.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
   explicit Widget(osg::BoundingBox CityBox, double Lon, double Lat,const QMap<int, GridTileInfo>& fileinfo = QMap<int, GridTileInfo>(), QWidget *parent = nullptr);
    ~Widget();

    osg::ref_ptr<osg::Node> loadTileset(const std::string& path);
    osg::ref_ptr<osg::Group> mergeTilesets();
    std::vector<std::vector<std::string>> exportor_txtcontent();
    osg::ref_ptr<osg::LightSource> createCustomLight();
protected slots:
    void initOSG();

private slots:
    void on_Output_clicked();

    //void on_OutputXml_clicked();

    void on_Show3DModle_clicked();

private:
    int count = 0;
    double Lat,Lon;
    osg::BoundingBox CityBox;

    QMap<int, GridTileInfo> m_fileinfo;
    std::vector<std::vector<std::string>>dataArray;
    QMap<int, std::vector<QString>> tilePathMap;
    Ui::Widget* ui;

    osgQOpenGLWidget* osgWidget;

    std::vector<QString> tilePaths = {};//文件路径接口

};

#endif // WIDGET_H
