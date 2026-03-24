#include "widget.h"
#include "ui_widget.h"
#include <osgtopng.h>
#include <QMessageBox>
#include <generatexml.h>
#include <QVector>
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMap>
#include <QFileDialog>
Widget::Widget(osg::BoundingBox CityBox, double Lon, double Lat, const QMap<int, GridTileInfo>& fileinfo,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_fileinfo(fileinfo)
{
    ui->setupUi(this);

    osgWidget = new osgQOpenGLWidget(ui->widget);          // 指定osg窗口显示位置
    osgWidget->setGeometry(0,0,ui->widget->geometry().width(),ui->widget->geometry().height());        // 指定osg窗口显示大小
    this->Lon = Lon;
    this->Lat = Lat;

    this->CityBox = CityBox;

    for(int i = 0;i<m_fileinfo.size();i++){
        std::vector<std::string> rowData;
        rowData.push_back(QString::number(m_fileinfo[i].fid).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].zoom).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].row).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].col).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].width_m).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].height_m).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].osgb_level).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].min_x,'g', 17).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].min_y,'g', 17).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].max_x,'g', 17).toStdString());
        rowData.push_back(QString::number(m_fileinfo[i].max_y,'g', 17).toStdString());
        rowData.push_back(m_fileinfo[i].files.join(';').toStdString());
        dataArray.push_back(rowData);
    }

    for (int i = 0; i < dataArray.size(); i++) {
        std::vector<std::string> firstRow = dataArray[i];
        QString filesVariant = QString::fromStdString(firstRow[firstRow.size()-1]);
        QStringList pathList = filesVariant.split(';');
//        for(int n = 0 ;n<pathList.size();n++){
//            qDebug()<<pathList[n]<<endl;
//        }
//        qDebug()<<"____________________________________________________________"<<endl;
        std::vector<QString> tempPaths = {};
        for (const QString& path : pathList) {
            tempPaths.push_back(path);
        }
        tilePathMap.insert(QString::fromStdString(dataArray[i][0]).toInt(),tempPaths);
    }
}

Widget::~Widget()
{
    delete ui;
    if(osgWidget != NULL)
    {
        delete osgWidget;
    }
}


void Widget::initOSG()
{
    osgViewer::Viewer* pViewer = osgWidget->getOsgViewer();
    pViewer->setCameraManipulator(new osgGA::TrackballManipulator);

    // 创建场景根节点
    osg::ref_ptr<osg::Group> sceneRoot = new osg::Group;

    sceneRoot->addChild(mergeTilesets());//合并子节点
    pViewer->setSceneData(sceneRoot);
    osg::ComputeBoundsVisitor boundsVisitor;
    sceneRoot->accept(boundsVisitor);
    osg::BoundingBox bb = boundsVisitor.getBoundingBox();
    qDebug()<<bb.xMin()<<bb.yMin()<<endl;
    qDebug()<<bb.xMax()<<bb.yMax()<<endl;
}

osg::ref_ptr<osg::LightSource> Widget::createCustomLight()
{
    // 1. 创建灯光对象
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum(0); // 绑定到第0号光源（OSG默认使用0号）

    // 2. 设置灯光类型（方向光，适合全局照明）
    light->setPosition(osg::Vec4(0.0f, 0.0f, 10.0f, 0.0f)); // w=0表示方向光，方向为(0,0,10)
    light->setDirection(osg::Vec3(-0.5f, -0.5f, -1.0f));    // 光源照射方向（微调让低俯角也能照到）

    // 3. 关键：设置光照参数（解决变暗的核心）
    light->setAmbient(osg::Vec4(0.6f, 0.6f, 0.6f, 1.0f));  // 环境光（填充暗部，值越大暗部越亮）
    light->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));  // 漫反射光（主体光照）
    light->setSpecular(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f)); // 镜面反射光（可选，增加高光）
    light->setConstantAttenuation(1.0f);                   // 衰减系数（方向光无衰减，设为1）

    // 4. 创建光源节点
    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->setLight(light);
    lightSource->setLocalStateSetModes(osg::StateAttribute::ON); // 启用灯光
    //lightSource->setReferenceFrame(osg::Transform::ABSOLUTE_RF); // 灯光跟随世界坐标系（不随模型旋转）

    return lightSource;
}
void Widget::on_Output_clicked()
{
    qDebug()<<"shuchuzhong1___________________________________________________________________________"<<endl;
    //用户输入俯角方位角
    int aod = 0;
    int azi = 0;
    if(ui->lineEdit_3->text()==""){
        aod = 45;
    }
    else{
        aod = ui->lineEdit_3->text().toInt();
    }
    if(ui->lineEdit_4->text()==""){
        azi = 270;
    }
    else{
        azi = ui->lineEdit_4->text().toInt();
    }


    //打开文件目录
    QString output_path = QFileDialog::getExistingDirectory(this, "选择文件夹", "./");
    if(output_path.isEmpty()){
        qDebug()<<"文件夹打开失败"<<endl;
        return;
    }
    for(int i =0;i<tilePathMap.size();i++){
        tilePaths = tilePathMap.value(i);
        if(tilePaths[0] == "NONE"){
            continue;
        }
        else{
            qDebug()<<"FID"<<i;
            osg::ref_ptr<osg::Group> sceneRoot = new osg::Group;
            sceneRoot->addChild(mergeTilesets());//添加合并所有子节点，即合并osgb文件
            sceneRoot->addChild(createCustomLight());
            //创建渲染类，调用渲染输出函数
            double x_min = QString::fromStdString(dataArray[i][7]).toDouble();
            double y_min = QString::fromStdString(dataArray[i][8]).toDouble();
            double x_max = QString::fromStdString(dataArray[i][9]).toDouble();
            double y_max = QString::fromStdString(dataArray[i][10]).toDouble();
            int lev = QString::fromStdString(dataArray[i][1]).toInt();
            int row = QString::fromStdString(dataArray[i][2]).toInt();
            int col = QString::fromStdString(dataArray[i][3]).toInt();

            OsgToPng otp(aod,azi,Lon,Lat);//元数据坐标原点，不同数据集需要更换
            otp.renderToPNG(sceneRoot,256,256,x_max,y_max,x_min,y_min,CityBox,lev,row,col,output_path);//2048是图片的长宽大小，天地图为256，角点参数接口

        }
    }
}

osg::ref_ptr<osg::Node> Widget::loadTileset(const std::string &path)
{
    // 加载OSGB格式倾斜摄影数据
    osg::ref_ptr<osg::Node> tile = osgDB::readNodeFile(path);
    // debug
    if (!tile) {
        osg::notify(osg::WARN) << "Failed to load tileset at " << path << std::endl;
        return nullptr;
    }
    return tile;
}

osg::ref_ptr<osg::Group> Widget::mergeTilesets()
{
    //合并子结点
    osg::ref_ptr<osg::Group> root = new osg::Group;

    for (const auto& path : tilePaths) {
        if (osg::ref_ptr<osg::Node> tile = loadTileset(path.toStdString())) {
            root->addChild(tile);
        }
    }
    return root;
}



//void Widget::on_OutputXml_clicked()//这个函数为打开文件按钮，选择文件打开后会把文件路径添加到tilePaths数组当中
//{
//    QString file_path = QFileDialog::getExistingDirectory(this, "请选择文件路径...", "./");
//    if(!file_path.isEmpty())
//    {
//        qDebug()<<file_path<<endl;
//    }

//    file_path = file_path + "/WMTSCapabilities.xml";
//    QString serviceUrl = "http://localhost:8066"; // 服务地址
//    QString tileDir = "tiles"; // 存放CGCS2000瓦片的目录
//    QString layerName = "tianditu_cgcs2000_layer"; // 图层唯一标识
//    QString layerTitle = "倾斜摄影测量图层"; // 显示名称
//    // 瓦片覆盖范围（示例：北京某区域的CGCS2000球面墨卡托坐标）
//    generatexml gxl;
//    bool success = gxl.generateCGCS2000WMTSCapabilities(file_path, serviceUrl, tileDir, layerName, layerTitle, bbox);

//    if (!success) {
//        qDebug() << "生成失败";
//    }
//}

void Widget::on_Show3DModle_clicked()//三维模型显示到窗口按钮
{
//    QString x_min_str = dataArray[1][7].toString();
//    QString y_min_str = dataArray[1][8].toString();
//    QString x_max_str = dataArray[1][9].toString();
//    QString y_max_str = dataArray[1][10].toString();

    ui->lineEdit->setText("第"+QString::number(count)+"个,还剩"+QString::number(tilePathMap.size())+"个");
    tilePaths = tilePathMap.value(count);
    if(tilePaths[0] == "NONE"){
    }
    else{
        qDebug()<<"FID"<<count;
        osg::ref_ptr<osg::Group> sceneRoot = new osg::Group;
        sceneRoot->addChild(mergeTilesets());//添加合并所有子节点，即合并osgb文件
    }
    initOSG();//加载文件
    count++;
}


