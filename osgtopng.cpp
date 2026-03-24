#include "osgtopng.h"


OsgToPng::OsgToPng(int aod ,int azimuth, double ox, double oy)
{
    Aod = aod;
    Azimuth = azimuth;
    Ox = ox;
    Oy = oy;
}

void OsgToPng::setupIsometricView(osg::Camera* camera, osg::Node* scene, double xmax, double ymax, double xmin, double ymin, osg::BoundingBox cityBox)
{
    // 1. 获取场景包围球
    osg::BoundingSphere bs = scene->getBound();

    //    获取场景包围盒
    osg::ComputeBoundsVisitor boundsVisitor;
    scene->accept(boundsVisitor);
    osg::BoundingBox bb = boundsVisitor.getBoundingBox();

    if (!bs.valid()) return;

    //  计算2.5D视角参数
    osg::Vec3f center;
    const float radius = bs.radius();

    //qDebug()<<center.x()<<center.y()<<endl;
    //计算网格角点，后面一组经纬度坐标为元数据坐标原点
//    osg::Vec3d localPosmin = geographicToLocal(xmin,ymin,0,Ox,Oy,0);
//    osg::Vec3d localPosmax = geographicToLocal(xmax,ymax,0,Ox,Oy,0);

    osg::Vec2d localPosmin = geographicToLocalSimple(xmin,ymin,Ox,Oy);
    osg::Vec2d localPosmax = geographicToLocalSimple(xmax,ymax,Ox,Oy);

    //计算相机朝向
    center.x() = (localPosmin.x() + localPosmax.x()) / 2;
    center.y() = (localPosmin.y() + localPosmax.y()) / 2;
    center.z() = cityBox.center().z();

    //qDebug()<<center.x()<<center.y()<<endl;

    //计算投影范围参数
    float x = (localPosmax.x() - localPosmin.x()) / 2;
    float y = abs(cityBox.center().y()-cityBox.yMin());



    //  设置等轴测投影参数
    const float angle = osg::PI * (Aod/180.0);    // 默认45度俯角 (俯视角度)
    const float azimuth = osg::PI * (Azimuth/180.0);  // 默认45度方位角 (东北方向)

    // 相机距离
    float distance;
    if(Aod == 90) {
        distance = x;
    }
    else {
        distance = x / cos(angle);
    }


    //  计算相机位置
    osg::Vec3f eye(
        center.x() + distance * cos(angle) * cos(azimuth),
        center.y() + distance * cos(angle) * sin(azimuth) + sin(azimuth),
        center.z() + distance * sin(angle)
    );

    //  设置视图矩阵 (看向场景中心)
    camera->setViewMatrixAsLookAt(eye, center, osg::Vec3(0, 0, 1));

    // 获取视图矩阵
    osg::Matrix viewMatrix = camera->getViewMatrix();

    //世界坐标角点
    std::vector<osg::Vec3> worldCorners = {
        /*z无用*/
        osg::Vec3(localPosmin.x(), localPosmin.y(), 1),   // 左下近
        osg::Vec3(localPosmax.x(), localPosmax.y(), 1),   // 右上近

    };

    // 存储转换后的相机坐标角点
    std::vector<osg::Vec3> cameraCorners;
    // 核心：视图矩阵 × 世界坐标点 = 相机坐标点
    // OSG中，matrix * vec3 会自动处理齐次坐标转换（w=1，乘法后归一化）
    for (const auto& worldPt : worldCorners) {
        osg::Vec3 cameraPt = worldPt * viewMatrix ;
        cameraCorners.push_back(cameraPt);
    }
    double cameraX = abs((cameraCorners[1].x() - cameraCorners[0].x()) / 2);
    double cameraY = abs((cameraCorners[1].y() - cameraCorners[0].y()) / 2);
    qDebug()<<localPosmax.x()<<localPosmax.y()<<localPosmin.x()<<localPosmin.y()<<endl;
    qDebug()<<cameraCorners[1].x()<<cameraCorners[1].y()<<cameraCorners[0].x()<<cameraCorners[0].y()<<endl;
    qDebug()<<cityBox.center().x()<<cityBox.xMin()<<cityBox.xMax()<<endl;
    qDebug()<<cityBox.center().y()<<cityBox.yMin()<<cityBox.yMax()<<endl;

    // 6. 设置正交投影 (等轴测投影)
    camera->setProjectionMatrixAsOrtho(
        -cameraX, cameraX,  // left, right
        -cameraY, cameraY,  // bottom, top
        0.1,      // near (足够近)
        radius * 8.0f        // far (足够远)
    );
}

void OsgToPng::renderToPNG(osg::Node* model, int width, int height, double xmax, double ymax, double xmin, double ymin, osg::BoundingBox cityBox, int lev, int row, int col, QString output)
{

    //  创建离屏渲染的图形上下文
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->width = width;  // 图像宽度
    traits->height = height; // 图像高度
    traits->windowDecoration = false;
    traits->pbuffer = true; // 使用离屏缓冲区

    traits->red = 8;
    traits->green = 8;
    traits->blue = 8;
    traits->alpha = 8;  // Windows通常需要alpha
    // 正确的深度缓冲区设置
    traits->depth = 24;  // 24位深度

    traits->sampleBuffers = 1;
    traits->samples = 4;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();
    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits);

    //  创建相机并设置视点
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext(gc);
    camera->setViewport(0, 0, traits->width, traits->height);
    camera->setClearColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f)); // 背景色
    camera->setRenderOrder(osg::Camera::PRE_RENDER); // 优先渲染
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT); // 使用FBO

    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->setClearDepth(1.0);

    osg::StateSet* stateset = camera->getOrCreateStateSet();
    stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    osg::ref_ptr<osg::Texture2D> colorTex = new osg::Texture2D;
    colorTex->setTextureSize(width, height);
    colorTex->setInternalFormat(GL_RGBA);
    colorTex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    camera->attach(osg::Camera::COLOR_BUFFER, colorTex);
    camera->attach(osg::Camera::DEPTH_BUFFER, GL_DEPTH_COMPONENT24);



    //  设置视点
    setupIsometricView(camera, model, xmax, ymax, xmin, ymin,cityBox);

    //  创建图像缓冲区用于捕获
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    camera->attach(osg::Camera::COLOR_BUFFER, image);

    //  创建查看器并渲染
    osgViewer::Viewer viewer;
    viewer.setCamera(camera);
    viewer.setSceneData(model);
    viewer.setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext);

    viewer.realize();

    for(int i=0; i<100; ++i) { // 多渲染几帧确保稳定
        viewer.frame();
    }

    //  保存图像到文件
    output = output + "/"+QString::number(lev)+"/"+QString::number(col)+"/"+QString::number(row)+".png";    //拼接标准瓦片目录
    std::string outputPath = output.toStdString();                                                          //QString转string
    try {
        // 提取目录部分并创建
        std::filesystem::path outputDir = std::filesystem::path(outputPath).parent_path();
        std::filesystem::create_directories(outputDir); // 递归创建所有不存在的目录

        // 写入图像
        bool success = osgDB::writeImageFile(*image, outputPath);
        if (!success){
            qDebug()<<lev<<row<<col<<"失败"<<endl;
        }
        else{
            qDebug()<<output<<endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // 处理目录创建失败的异常（如权限不足）
    }

}

osg::Vec3d OsgToPng::geographicToECEF(double lon, double lat, double height)
{

    // 将角度转换为弧度
    double lon_rad = osg::DegreesToRadians(lon);
    double lat_rad = osg::DegreesToRadians(lat);

    // 计算辅助变量
    double sin_lat = sin(lat_rad);
    double cos_lat = cos(lat_rad);
    double sin_lon = sin(lon_rad);
    double cos_lon = cos(lon_rad);

    // 计算卯酉圈曲率半径
    double N = WGS84_A / sqrt(1.0 - WGS84_E2 * sin_lat * sin_lat);

    // 计算ECEF坐标
    double x = (N + height) * cos_lat * cos_lon;
    double y = (N + height) * cos_lat * sin_lon;
    double z = (N * (1.0 - WGS84_E2) + height) * sin_lat;

    return osg::Vec3d(x, y, z);
}

osg::Vec3d OsgToPng::geographicToLocal(double lon, double lat, double height,
                            double ref_lon, double ref_lat, double ref_alt)
{
    // 将地理坐标转换为ECEF
    osg::Vec3d ecef = geographicToECEF(lon, lat, height);

    // 将参考点转换为ECEF
    osg::Vec3d ref_ecef = geographicToECEF(ref_lon, ref_lat, ref_alt);

    // 计算相对于参考点的坐标
    osg::Vec3d relativePos = ecef - ref_ecef;

    // 计算ENU旋转矩阵（修正版本）
    double ref_lon_rad = osg::DegreesToRadians(ref_lon);
    double ref_lat_rad = osg::DegreesToRadians(ref_lat);

    double sin_lon = sin(ref_lon_rad);
    double cos_lon = cos(ref_lon_rad);
    double sin_lat = sin(ref_lat_rad);
    double cos_lat = cos(ref_lat_rad);

    // 正确的ENU变换矩阵
    osg::Matrixd ecefToEnu(
        -sin_lon,                cos_lon,               0.0,     0.0,
        -sin_lat * cos_lon,     -sin_lat * sin_lon,    cos_lat, 0.0,
        cos_lat * cos_lon,      cos_lat * sin_lon,     sin_lat, 0.0,
        0.0,                    0.0,                   0.0,     1.0
    );

    // 转换为ENU坐标
    osg::Vec3d enu = ecefToEnu * relativePos;  // 注意：这里是矩阵乘以向量，不是向量乘以矩阵

    return enu;
}

osg::Vec2d OsgToPng::geographicToLocalSimple(double lon, double lat,
                                            double ref_lon, double ref_lat)
{
    // 常量定义
    const double METERS_PER_DEGREE_LAT = 111319.9;  // 每度纬度的米数
    const double EARTH_RADIUS = 6371000.0;         // 地球平均半径（米）

    // 直接计算（不需要转换为弧度再进行三角计算）
    double east = EARTH_RADIUS * cos(osg::DegreesToRadians(ref_lat))
                  * osg::DegreesToRadians(lon - ref_lon);

    double north = METERS_PER_DEGREE_LAT * (lat - ref_lat);

    return osg::Vec2d(east, north);
}
