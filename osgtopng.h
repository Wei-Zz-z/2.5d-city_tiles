#ifndef OSGTOPNG_H
#define OSGTOPNG_H

#include <QDebug>
#include <QString>
#include <filesystem>
#include <osgQOpenGL/osgQOpenGLWidget>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerBase>
#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osgGA/TrackballManipulator>
#include <osg/ComputeBoundsVisitor>
#include <osg/Matrix>
#include <osg/Texture2D>

class OsgToPng
{
public:
    OsgToPng(int aod ,int azimuth, double ox, double oy);
    const double WGS84_A = 6378137.0;        // 赤道半径 (米)
    const double WGS84_F = 1.0 / 298.257223563; // 扁率
    const double WGS84_B = WGS84_A * (1.0 - WGS84_F); // 极半径
    const double WGS84_E2 = 1.0 - (WGS84_B * WGS84_B) / (WGS84_A * WGS84_A); // 第一偏心率平方


    osg::Camera* createIsometricCamera(int width, int height);
    void setupIsometricView(osg::Camera* camera, osg::Node* scene, double xmax, double ymax, double xmin, double ymin, osg::BoundingBox cityBox);
    void renderToPNG(osg::Node* model, int width, int height, double xmax, double ymax, double xmin, double ymin, osg::BoundingBox cityBox, int lev, int row, int col, QString output);
    osg::Vec3d geographicToECEF(double lon, double lat, double height);
    osg::Vec3d geographicToLocal(double lon, double lat, double height,double ref_lon, double ref_lat, double ref_alt);
    osg::Vec2d geographicToLocalSimple(double lon, double lat, double ref_lon, double ref_lat);

private:
    double Ox;
    double Oy;
    int Aod;
    int Azimuth;


};

#endif // OSGTOPNG_H
