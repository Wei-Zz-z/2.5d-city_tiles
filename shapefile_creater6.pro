QT       += core gui xml
Qt       += widgets
QT       += concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = ShapefileQuadtree

CONFIG += c++17
DEFINES += _USE_MATH_DEFINES  # 解决 M_PI 问题

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    coordinateconverter.cpp \
    generatexml.cpp \
    gridmanager.cpp \
    gridrenderer.cpp \
    input_osgbinput.cpp \
    main.cpp \
    mainwindow.cpp \
    openmapping.cpp\
    osgbmapper.cpp \
    osgtopng.cpp \
    reader_xml.cpp \
    scalecalculator.cpp \
    shapefileexporter.cpp \
    txtexprot.cpp \
    widget.cpp\



HEADERS += \
    appconstants.h \
    coordinateconverter.h \
    generatexml.h \
    gridmanager.h \
    gridrenderer.h \
    input_osgbinput.h \
    mainwindow.h \
    openmapping.h \
    osgbmapper.h \
    osgtopng.h \
    reader_xml.h \
    scalecalculator.h \
    shapefileexporter.h \
    txtexport.h \
    widget.h\
    tile_catalog.h


FORMS += \
    mainwindow.ui\
    openmapping.ui\
    widget.ui

INCLUDEPATH += D:/OSG/OSG-VS2017-msvc64/install/include
DEPENDPATH += D:/OSG/OSG-VS2017-msvc64/install/include

win32:CONFIG(release, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgQOpenGL
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgQOpenGLd

win32:CONFIG(release, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgViewer
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgViewerd

win32:CONFIG(release, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgGA
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgGAd


win32:CONFIG(release, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgDB
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgDBd

win32:CONFIG(release, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losg
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -losgd

win32:CONFIG(release, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -lOpenThreads
else:win32:CONFIG(debug, debug|release): LIBS += -LD:/OSG/OSG-VS2017-msvc64/install/lib/ -lOpenThreads


INCLUDEPATH += "D:/OSGeo4W64/OSGeo4W64/apps/qgis-ltr/include"
INCLUDEPATH += "D:/OSGeo4W64/OSGeo4W64/include"

LIBS += -L"D:/OSGeo4W64/OSGeo4W64/apps/qgis-ltr/lib" -lqgis_core -lqgis_gui


win32 {
    # 添加GDAL库路径（根据您的实际安装路径修改）
    LIBS += -L"D:\OSGeo4W64\OSGeo4W64\lib" \
            -lgdal_i \
            -lgeos_c \
            -lproj
    DEFINES += USE_GDAL
}
QMAKE_CXXFLAGS += /source-charset:utf-8
QMAKE_CXXFLAGS += /execution-charset:utf-8
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
