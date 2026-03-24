#include "mainwindow.h"
#include <qgsapplication.h>
#include <QDebug>  // 添加调试输出

int main(int argc, char *argv[]) {
//     设置环境变量
    //qputenv("PROJ_LIB", "D:/osgeo4w64/OSGeo4W64/share/proj");
    qputenv("PATH", "D:/osgeo4w64/OSGeo4W64/bin;D:/osgeo4w64/OSGeo4W64/apps/qgis-ltr/bin;" + qgetenv("PATH"));

    // 使用QGIS应用
    QgsApplication app(argc, argv, true);

    // 设置QGIS安装路径
    QgsApplication::setPrefixPath("D:/osgeo4w64/OSGeo4W64/apps/qgis-ltr", true);

    // 调试输出
    qDebug() << "QGIS prefix path:" << QgsApplication::prefixPath();
    qDebug() << "Library paths:" << QgsApplication::libraryPaths();

    // 初始化QGIS
    QgsApplication::initQgis();

    MainWindow mainWindow;
    mainWindow.show();

    int result = app.exec();

    // 清理QGIS资源
    QgsApplication::exitQgis();

    return result;
}
