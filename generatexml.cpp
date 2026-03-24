#include "generatexml.h"

generatexml::generatexml()
{

}

bool generatexml::generateCGCS2000WMTSCapabilities(const QString& xmlPath,
                                      const QString& serviceUrl,
                                      const QString& tileDir,
                                      const QString& layerName,
                                      const QString& layerTitle,
                                      const QVector<double>& bbox) {
    // 校验输入参数
    if (bbox.size() != 4) {
        qDebug() <<"校验输入参数";
        return false;
    }
    double minX = bbox[0], minY = bbox[1], maxX = bbox[2], maxY = bbox[3];

    // 1. 创建XML文档
    QDomDocument doc;
    auto pi = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(pi);

    // 2. 根节点：WMTS_Capabilities（OGC 1.0.0标准）
    QDomElement root = doc.createElementNS("http://www.opengis.net/wmts/1.0", "WMTS_Capabilities");
    root.setAttribute("version", "1.0.0");
    root.setAttribute("xmlns:ows", "http://www.opengis.net/ows/1.1");
    root.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    root.setAttribute("updateSequence", "1");
    doc.appendChild(root);

    // 3. 服务标识（ServiceIdentification）
    QDomElement serviceId = doc.createElement("ows:ServiceIdentification");
    auto title = doc.createElement("ows:Title");
    title.appendChild(doc.createTextNode(layerTitle));
    serviceId.appendChild(title);
    auto serviceType = doc.createElement("ows:ServiceType");
    serviceType.appendChild(doc.createTextNode("OGC WMTS"));
    serviceId.appendChild(serviceType);
    auto serviceVer = doc.createElement("ows:ServiceTypeVersion");
    serviceVer.appendChild(doc.createTextNode("1.0.0"));
    serviceId.appendChild(serviceVer);
    root.appendChild(serviceId);

    // 4. 操作元数据（支持GetCapabilities和GetTile）
    QDomElement operations = doc.createElement("ows:OperationsMetadata");

    // 4.1 GetCapabilities操作
    QDomElement getCaps = doc.createElement("ows:Operation");
    getCaps.setAttribute("name", "GetCapabilities");
    auto capsDcp = doc.createElement("ows:DCP");
    auto capsHttp = doc.createElement("ows:HTTP");
    auto capsGet = doc.createElement("ows:Get");
    capsGet.setAttribute("xlink:href", serviceUrl + "/WMTSCapabilities.xml");
    capsHttp.appendChild(capsGet);
    capsDcp.appendChild(capsHttp);
    getCaps.appendChild(capsDcp);
    operations.appendChild(getCaps);

    // 4.2 GetTile操作
    QDomElement getTile = doc.createElement("ows:Operation");
    getTile.setAttribute("name", "GetTile");
    auto tileDcp = doc.createElement("ows:DCP");
    auto tileHttp = doc.createElement("ows:HTTP");
    auto tileGet = doc.createElement("ows:Get");
    tileGet.setAttribute("xlink:href", serviceUrl + "/" + tileDir + "/");
    tileHttp.appendChild(tileGet);
    tileDcp.appendChild(tileHttp);
    getTile.appendChild(tileDcp);
    operations.appendChild(getTile);

    root.appendChild(operations);

    // 5. 内容（图层+瓦片矩阵集）
    QDomElement contents = doc.createElement("Contents");

    // 5.1 图层定义（关联CGCS2000瓦片）
    QDomElement layer = doc.createElement("Layer");
    auto layerId = doc.createElement("ows:Identifier");
    layerId.appendChild(doc.createTextNode(layerName));
    layer.appendChild(layerId);
    auto layerTitleElem = doc.createElement("ows:Title");
    layerTitleElem.appendChild(doc.createTextNode(layerTitle));
    layer.appendChild(layerTitleElem);
    auto format = doc.createElement("Format");
    format.appendChild(doc.createTextNode("image/png")); // 天地图瓦片格式
    layer.appendChild(format);
    // 关联瓦片矩阵集（CGCS2000球面墨卡托）
    auto tmsLink = doc.createElement("TileMatrixSetLink");
    auto tms = doc.createElement("TileMatrixSet");
    tms.appendChild(doc.createTextNode("EPSG:4490_WebMercator")); // 天地图官方标识
    tmsLink.appendChild(tms);
    layer.appendChild(tmsLink);
    // 瓦片URL模板（z={TileMatrix}, x={TileCol}, y={TileRow}）
    auto resUrl = doc.createElement("ResourceURL");
    resUrl.setAttribute("format", "image/png");
    resUrl.setAttribute("resourceType", "tile");
    resUrl.setAttribute("template",
        QString("%1/%2/{TileMatrix}/{TileCol}/{TileRow}.png")
            .arg(serviceUrl).arg(tileDir));
    layer.appendChild(resUrl);
    // 图层范围（CGCS2000坐标）
    auto bboxElem = doc.createElement("ows:BoundingBox");
    bboxElem.setAttribute("crs", "EPSG:4490");
    auto lower = doc.createElement("ows:LowerCorner");
    lower.appendChild(doc.createTextNode(QString("%1 %2").arg(minX).arg(minY)));
    auto upper = doc.createElement("ows:UpperCorner");
    upper.appendChild(doc.createTextNode(QString("%1 %2").arg(maxX).arg(maxY)));
    bboxElem.appendChild(lower);
    bboxElem.appendChild(upper);
    layer.appendChild(bboxElem);
    contents.appendChild(layer);

    // 5.2 瓦片矩阵集（TileMatrixSet）：天地图CGCS2000球面墨卡托标准参数（0-18级）
    QDomElement tileMatrixSet = doc.createElement("TileMatrixSet");
    auto tmsId = doc.createElement("ows:Identifier");
    tmsId.appendChild(doc.createTextNode("EPSG:4490_WebMercator")); // 与天地图一致
    tileMatrixSet.appendChild(tmsId);
    // 坐标系（CGCS2000）
    auto supportedCRS = doc.createElement("ows:SupportedCRS");
    supportedCRS.appendChild(doc.createTextNode("urn:ogc:def:crs:EPSG::4490"));
    tileMatrixSet.appendChild(supportedCRS);

    // 天地图CGCS2000球面墨卡托各级参数（0-18级）
    const double initialResolution = 156543.0339280410; // 级别0分辨率（米/像素）
    const QString topLeftCorner = "-20037508.3427892 20037508.3427892"; // 左上角原点
    const int tileSize = 256; // 瓦片尺寸
    const double dpi = 96; // 天地图默认DPI
    const double mmPerInch = 25.4; // 英寸转毫米
    // 级别z的比例尺分母 = (分辨率 * dpi) / mmPerInch
    // 即：(initialResolution / 2^z) * dpi / mmPerInch

    for (int z = 0; z <= 18; ++z) {
        QDomElement tileMatrix = doc.createElement("TileMatrix");
        // 级别标识
        auto tmId = doc.createElement("ows:Identifier");
        tmId.appendChild(doc.createTextNode(QString::number(z)));
        tileMatrix.appendChild(tmId);
        // 比例尺分母（天地图官方计算方式）
        auto scaleDen = doc.createElement("ScaleDenominator");
        double resolution = initialResolution / (1 << z); // 分辨率 = 初始分辨率 / 2^z
        double scale = (resolution * dpi) / mmPerInch;
        scaleDen.appendChild(doc.createTextNode(QString::number(scale, 'f', 6)));
        tileMatrix.appendChild(scaleDen);
        // 左上角坐标
        auto tlCorner = doc.createElement("TopLeftCorner");
        tlCorner.appendChild(doc.createTextNode(topLeftCorner));
        tileMatrix.appendChild(tlCorner);
        // 瓦片宽高
        auto tw = doc.createElement("TileWidth");
        tw.appendChild(doc.createTextNode(QString::number(tileSize)));
        tileMatrix.appendChild(tw);
        auto th = doc.createElement("TileHeight");
        th.appendChild(doc.createTextNode(QString::number(tileSize)));
        tileMatrix.appendChild(th);
        // 矩阵行列数（2^z）
        auto mw = doc.createElement("MatrixWidth");
        mw.appendChild(doc.createTextNode(QString::number(1 << z)));
        tileMatrix.appendChild(mw);
        auto mh = doc.createElement("MatrixHeight");
        mh.appendChild(doc.createTextNode(QString::number(1 << z)));
        tileMatrix.appendChild(mh);

        tileMatrixSet.appendChild(tileMatrix);
    }

    contents.appendChild(tileMatrixSet);
    root.appendChild(contents);

    // 6. 保存XML文件
    QFile file(xmlPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() <<"文件创建失败"<<endl;
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc.toString(4); // 格式化输出
    file.close();

    qDebug() << "CGCS2000 WMTS元数据生成成功：" << xmlPath;
    return true;
}
