@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   开始打包 ShapefileQuadtree 应用程序
echo ========================================

set "RELEASE_DIR=release"
set "OSG_INSTALL_DIR=D:\OSG\OSG-VS2017-msvc64\install"
set "QGIS_INSTALL_DIR=D:\OSGeo4W64\OSGeo4W64"
set "QT_DIR=D:\QT5.12.6\5.12.6\msvc2017_64"  # 请根据您的Qt安装路径修改

echo 清理并创建输出目录...
if exist "%RELEASE_DIR%" rmdir /s /q "%RELEASE_DIR%"
mkdir "%RELEASE_DIR%"

echo 复制主程序...
copy ShapefileQuadtree.exe "%RELEASE_DIR%\"

echo 使用 windeployqt 收集 Qt 依赖...
cd "%RELEASE_DIR%"
windeployqt --release --no-compiler-runtime --no-angle --no-opengl-sw ShapefileQuadtree.exe
cd ..

echo 复制 OSG 库文件...
copy "%OSG_INSTALL_DIR%\bin\*.dll" "%RELEASE_DIR%\"

echo 复制 OSG 插件...
if not exist "%RELEASE_DIR%\osgPlugins-3.6.5" mkdir "%RELEASE_DIR%\osgPlugins-3.6.5"
xcopy "%OSG_INSTALL_DIR%\bin\osgPlugins-3.6.5\*" "%RELEASE_DIR%\osgPlugins-3.6.5\" /E /Y /I

echo 复制 QGIS 核心库...
copy "%QGIS_INSTALL_DIR%\bin\qgis_core.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\qgis_gui.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\qgis_analysis.dll" "%RELEASE_DIR%\"

echo 复制 QGIS 插件...
if not exist "%RELEASE_DIR%\plugins" mkdir "%RELEASE_DIR%\plugins"
xcopy "%QGIS_INSTALL_DIR%\apps\qgis-ltr\plugins\*" "%RELEASE_DIR%\plugins\" /E /Y /I

echo 复制 QGIS 资源文件...
if not exist "%RELEASE_DIR%\resources" mkdir "%RELEASE_DIR%\resources"
xcopy "%QGIS_INSTALL_DIR%\apps\qgis-ltr\resources\*" "%RELEASE_DIR%\resources\" /E /Y /I

echo 复制 GDAL 相关库...
copy "%QGIS_INSTALL_DIR%\bin\gdal*.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\geos_c.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\proj.dll" "%RELEASE_DIR%\"

echo 复制 PROJ 数据...
if not exist "%RELEASE_DIR%\proj" mkdir "%RELEASE_DIR%\proj"
xcopy "%QGIS_INSTALL_DIR%\share\proj\*" "%RELEASE_DIR%\proj\" /E /Y /I

echo 复制 GDAL 数据...
if not exist "%RELEASE_DIR%\gdal-data" mkdir "%RELEASE_DIR%\gdal-data"
xcopy "%QGIS_INSTALL_DIR%\share\gdal\*" "%RELEASE_DIR%\gdal-data\" /E /Y /I

echo 复制其他必要的运行时库...
copy "%QGIS_INSTALL_DIR%\bin\sqlite3.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\spatialite.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\expat.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\zlib.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\libpng16.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\jpeg.dll" "%RELEASE_DIR%\"
copy "%QGIS_INSTALL_DIR%\bin\tiff.dll" "%RELEASE_DIR%\"

echo 创建必要的配置文件...
echo [General] > "%RELEASE_DIR%\qt.conf"

echo ========================================
echo   打包完成！
echo   输出目录: %RELEASE_DIR%
echo ========================================

pause