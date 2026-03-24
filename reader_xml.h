#ifndef READER_XML_H
#define READER_XML_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariant>
#include <QDomDocument>

class ReaderXML : public QObject
{
    Q_OBJECT

public:
    explicit ReaderXML(QObject *parent = nullptr);

    // 读取XML文件信息
    QList<QVariant> readXMLFile(const QString& filePath);

    // 获取错误信息
    QString getError() const { return m_error; }

private:
    QString m_error;
};

#endif // READER_XML_H
