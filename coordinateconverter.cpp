#include "coordinateconverter.h"
#include <qgscoordinatetransform.h>
#include <qgsproject.h>
#include <qgsgeometry.h>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QRegExp>
#include <QDebug>
#include <cmath>
#include <limits>
#include "appconstants.h"
CoordinateConverter::CoordinateConverter(QObject *parent)
    : QObject(parent)
{
}

QgsRectangle CoordinateConverter::getUserExtentFromDialog(QWidget *parent)
{
    QDialog *dialog = createExtentDialog(parent);
    if (!dialog) {
        return QgsRectangle();
    }

    if (dialog->exec() != QDialog::Accepted) {
        delete dialog;
        return QgsRectangle();
    }

    QgsRectangle extent = getExtentFromDialog(dialog);
    delete dialog;

    return extent;
}

QgsRectangle CoordinateConverter::convertListToWebMercator(const QList<double> &coords)
{
    // 检查输入是否有效
    if (coords.size() != 4) {
        emit conversionError("输入坐标列表必须包含4个值: minX, minY, maxX, maxY");
        return QgsRectangle();
    }
    qDebug()<<coords;
    // 提取坐标值
    double minX = coords[0];
    double minY = coords[1];
    double maxX = coords[2];
    double maxY = coords[3];
    qDebug()<<minX;
    qDebug()<<minY;
    qDebug()<<maxX;
    qDebug()<<maxY;
    // 创建WGS84范围
    QgsRectangle wgs84Extent(minY, minX, maxY, maxX);

    if (wgs84Extent.isEmpty()) {
        qDebug()<<"wgs84extent为空";
    }
    // 验证范围有效性
    if (!validateExtent(wgs84Extent)) {
        emit conversionError("输入坐标范围无效");
        qDebug()<<"输入坐标范围无效";
        return QgsRectangle();
    }

    // 转换为Web墨卡托
    return wgs84Extent;
}

QDialog* CoordinateConverter::createExtentDialog(QWidget *parent)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle("输入经纬度范围");
    dialog->resize(400, 300);

    QFormLayout *form = new QFormLayout(dialog);

    // 添加说明标签
    QLabel *infoLabel = new QLabel(
        "支持多种格式输入：\n"
        "- 十进制度：116.12345\n"
        "- 度分秒：116°7′24.42″E\n"
        "- 度分：116°7.407′", dialog);
    form->addRow(infoLabel);

    // 经度输入
    QLineEdit *minLonEdit = new QLineEdit(dialog);
    minLonEdit->setPlaceholderText("例如: 116.12345 或 116°7′24.42″E");
    form->addRow("最小经度:", minLonEdit);

    QLineEdit *maxLonEdit = new QLineEdit(dialog);
    maxLonEdit->setPlaceholderText("例如: 117.12345 或 117°7′24.42″E");
    form->addRow("最大经度:", maxLonEdit);

    // 纬度输入
    QLineEdit *minLatEdit = new QLineEdit(dialog);
    minLatEdit->setPlaceholderText("例如: 39.12345 或 39°54′20.88″N");
    form->addRow("最小纬度:", minLatEdit);

    QLineEdit *maxLatEdit = new QLineEdit(dialog);
    maxLatEdit->setPlaceholderText("例如: 40.12345 或 40°54′20.88″N");
    form->addRow("最大纬度:", maxLatEdit);

    // 添加预览标签
    QLabel *previewLabel = new QLabel("十进制预览:", dialog);
    form->addRow(previewLabel);

    // 添加按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, dialog);
    form->addRow(buttonBox);

    // 连接信号和槽
    auto updatePreview = [=]() {
        try {
            double minLon = dmsToDecimal(minLonEdit->text());
            double minLat = dmsToDecimal(minLatEdit->text());
            double maxLon = dmsToDecimal(maxLonEdit->text());
            double maxLat = dmsToDecimal(maxLatEdit->text());

            if (!qIsNaN(minLon) && !qIsNaN(minLat) &&
                !qIsNaN(maxLon) && !qIsNaN(maxLat)) {
                QString preview = QString("经度: %1 - %2\n纬度: %3 - %4")
                    .arg(minLon, 0, 'f', 6)
                    .arg(maxLon, 0, 'f', 6)
                    .arg(minLat, 0, 'f', 6)
                    .arg(maxLat, 0, 'f', 6);
                previewLabel->setText(preview);
            } else {
                previewLabel->setText("输入格式错误，请检查");
            }
        } catch (...) {
            previewLabel->setText("计算错误，请检查输入");
        }
    };

    connect(minLonEdit, &QLineEdit::textChanged, updatePreview);
    connect(minLatEdit, &QLineEdit::textChanged, updatePreview);
    connect(maxLonEdit, &QLineEdit::textChanged, updatePreview);
    connect(maxLatEdit, &QLineEdit::textChanged, updatePreview);

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    return dialog;
}

QgsRectangle CoordinateConverter::getExtentFromDialog(QDialog *dialog)
{
    // 获取对话框中的控件
    QFormLayout *form = qobject_cast<QFormLayout*>(dialog->layout());
    if (!form) {
        return QgsRectangle();
    }

    QLineEdit *minLonEdit = qobject_cast<QLineEdit*>(form->itemAt(1, QFormLayout::FieldRole)->widget());
    QLineEdit *maxLonEdit = qobject_cast<QLineEdit*>(form->itemAt(2, QFormLayout::FieldRole)->widget());
    QLineEdit *minLatEdit = qobject_cast<QLineEdit*>(form->itemAt(3, QFormLayout::FieldRole)->widget());
    QLineEdit *maxLatEdit = qobject_cast<QLineEdit*>(form->itemAt(4, QFormLayout::FieldRole)->widget());

    if (!minLonEdit || !maxLonEdit || !minLatEdit || !maxLatEdit) {
        return QgsRectangle();
    }

    // 转换输入值
    double minLon = dmsToDecimal(minLonEdit->text());
    double minLat = dmsToDecimal(minLatEdit->text());
    double maxLon = dmsToDecimal(maxLonEdit->text());
    double maxLat = dmsToDecimal(maxLatEdit->text());

    // 验证输入值
    if (qIsNaN(minLon) || qIsNaN(minLat) || qIsNaN(maxLon) || qIsNaN(maxLat)) {
        emit conversionError("经纬度格式错误");
        return QgsRectangle();
    }

    // 创建范围
    QgsRectangle extent(minLon, minLat, maxLon, maxLat);

    // 验证范围有效性
    if (!validateExtent(extent)) {
        emit conversionError("范围无效");
        return QgsRectangle();
    }

    return extent;
}

QgsRectangle CoordinateConverter::transformToWebMercator(const QgsRectangle &wgs84Extent)
{
    if (wgs84Extent.isEmpty()) {
        emit conversionError("WGS84范围为空");
        return QgsRectangle();
    }

    try {
        // 创建坐标转换：从WGS84到Web墨卡托
        QgsCoordinateTransform transform(
            QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84
            QgsCoordinateReferenceSystem("EPSG:3857"), // Web墨卡托
            QgsProject::instance()
        );

        // 转换范围到Web墨卡托
        QgsRectangle webMercatorExtent = transform.transformBoundingBox(wgs84Extent);

        emit extentConverted(webMercatorExtent);
        return webMercatorExtent;

    } catch (const QgsCsException &e) {
        emit conversionError(QString("坐标转换失败: %1").arg(e.what()));
        return QgsRectangle();
    }
}

QgsRectangle CoordinateConverter::transformToWGS84(const QgsRectangle &webMercatorExtent)
{
    if (webMercatorExtent.isEmpty()) {
        emit conversionError("Web墨卡托范围为空");
        return QgsRectangle();
    }

    try {
        // 创建坐标转换：从Web墨卡托到WGS84
        QgsCoordinateTransform transform(
            QgsCoordinateReferenceSystem("EPSG:3857"), // Web墨卡托
            QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84
            QgsProject::instance()
        );

        // 转换范围到WGS84
        QgsRectangle wgs84Extent = transform.transformBoundingBox(webMercatorExtent);

        emit extentConverted(wgs84Extent);
        return wgs84Extent;

    } catch (const QgsCsException &e) {
        emit conversionError(QString("坐标转换失败: %1").arg(e.what()));
        return QgsRectangle();
    }
}

double CoordinateConverter::dmsToDecimal(const QString &dms)
{
    // 支持多种分隔符：空格、度分秒符号、中文符号等
    QStringList parts = dms.split(QRegExp("[°′″'\"\\s]+"), QString::SkipEmptyParts);
    if (parts.size() < 1) return std::numeric_limits<double>::quiet_NaN();

    double degrees = parts[0].toDouble();
    double minutes = (parts.size() > 1) ? parts[1].toDouble() : 0;
    double seconds = (parts.size() > 2) ? parts[2].toDouble() : 0;

    double decimal = degrees + minutes/60.0 + seconds/3600.0;

    // 处理负值（如 30°30′30″W 或 -30°30′30″）
    if (dms.contains("W", Qt::CaseInsensitive) ||
        dms.contains("S", Qt::CaseInsensitive) ||
        dms.contains("西", Qt::CaseInsensitive) ||
        dms.contains("南", Qt::CaseInsensitive) ||
        dms.startsWith("-")) {
        decimal = -decimal;
    }

    return decimal;
}

QString CoordinateConverter::decimalToDms(double decimal, bool isLongitude)
{
    // 确定方向
    QString direction;
    if (isLongitude) {
        direction = (decimal >= 0) ? "E" : "W";
    } else {
        direction = (decimal >= 0) ? "N" : "S";
    }

    // 取绝对值
    decimal = std::abs(decimal);

    // 计算度分秒
    int degrees = static_cast<int>(decimal);
    double fractional = (decimal - degrees) * 60;
    int minutes = static_cast<int>(fractional);
    double seconds = (fractional - minutes) * 60;

    // 格式化输出
    return QString("%1°%2′%3″%4")
        .arg(degrees)
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 5, 'f', 2, QLatin1Char('0'))
        .arg(direction);
}

bool CoordinateConverter::validateExtent(const QgsRectangle &extent)
{
    if (extent.isEmpty()) {
        return false;
    }

    double minLon = extent.xMinimum();
    double minLat = extent.yMinimum();
    double maxLon = extent.xMaximum();
    double maxLat = extent.yMaximum();

    // 验证范围有效性
    if (minLon >= maxLon || minLat >= maxLat) {
        return false;
    }

    if (minLon < -180 || maxLon > 180 || minLat < -90 || maxLat > 90) {
        return false;
    }

    return true;
}
