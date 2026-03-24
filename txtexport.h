#ifndef EPORT_H
#define EPORT_H
#include <utility>
#include <QWidget>
#include <QDebug>
#include <QString>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QEvent>
#include <QProgressDialog>
#include <unordered_map>
#include <osgQOpenGL/osgQOpenGLWidget>
#include <osgViewer/Viewer>
#include <osg/Node>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osgGA/TrackballManipulator>
#include "tile_catalog.h"
#include "gridmanager.h"
#include "osgbmapper.h"
#ifdef USE_PROJ
#include <proj.h>
#endif


struct GridTileInfo {
    int fid;
    int zoom;
    int row;
    int col;
    double width_m;
    double height_m;
    int osgb_level;
    double min_x;
    double min_y;
    double max_x;
    double max_y;
    QStringList files;
};
class Eport: public QWidget
{
public:
    Eport(QWidget *parent = nullptr, double originLon = 0, double originLat = 0, QString rootDir = "");

    ~Eport();

    QStringList getFilesByFid(int fid);

    // 获取所有网格瓦片数据
    QMap<int, GridTileInfo> getAllGridTiles();

    // 清空网格瓦片数据
    void clearGridTilesData();

    void exportGridTilesToTxt(GridManager *gridManager,OSGBMapper* m_osgbMapper,int startlevel,int targetlevel);
private:
    struct GeoConverter {
        double originLon = 113.06277;
        double originLat = 22.64785;
#ifndef USE_PROJ
        static constexpr double metersPerDegLat = 111320.0;
        static constexpr double DEG2RAD = 0.017453292519943295;
        inline std::pair<double,double> localToWGS(double x, double y) const {
            double lon = originLon + x / (metersPerDegLat * std::cos(originLat * DEG2RAD));
            double lat = originLat + y / metersPerDegLat;
            return {lon, lat};
        }
        inline std::pair<double,double> WGStoLocal(double lon, double lat) const {
            double x = (lon - originLon) * metersPerDegLat * std::cos(originLat * DEG2RAD);
            double y = (lat - originLat) * metersPerDegLat;
            return {x, y};
        }
        inline void setOrigin(double lon, double lat){ originLon=lon; originLat=lat; }
#else
        PJ_CONTEXT* ctx = nullptr; PJ* pipeline = nullptr;
        void ensureInit(){ if(pipeline) return; if(!ctx) ctx = proj_context_create();
            std::ostringstream oss; oss.setf(std::ios::fixed); oss.precision(8);
            oss << "+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad "
                << "+step +proj=cart +ellps=WGS84 "
                << "+step +proj=topocentric +lat_0=" << originLat << " +lon_0=" << originLon << " +h_0=0";
            PJ* pj = proj_create(ctx, oss.str().c_str()); if(!pj) return; PJ* norm = proj_normalize_for_visualization(ctx, pj); if(norm){ proj_destroy(pj); pj=norm; } pipeline=pj; }
        inline std::pair<double,double> WGStoLocal(double lon, double lat){ ensureInit(); if(!pipeline) return {0.0,0.0}; PJ_COORD c=proj_coord(lon,lat,0,0); PJ_COORD r=proj_trans(pipeline, PJ_FWD, c); return {r.enu.e, r.enu.n}; }
        inline std::pair<double,double> localToWGS(double x, double y){ ensureInit(); if(!pipeline) return {originLon,originLat}; PJ_COORD c=proj_coord(x,y,0,0); PJ_COORD r=proj_trans(pipeline, PJ_INV, c); return {r.lp.lam, r.lp.phi}; }
        inline void setOrigin(double lon, double lat){ originLon=lon; originLat=lat; if(pipeline){ proj_destroy(pipeline); pipeline=nullptr; } }
        ~GeoConverter(){ if(pipeline) proj_destroy(pipeline); if(ctx) proj_context_destroy(ctx); }
#endif
    } geo;

    TileCatalog::TileMap tileIndex;
    QString dataRootDir;
    std::unordered_map<std::string, osg::BoundingBox> bboxCache;
    bool useProj = true;
    };

#endif // EPORT_H
