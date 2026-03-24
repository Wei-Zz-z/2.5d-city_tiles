#ifndef LEVELMAPPINGDIALOG_H
#define LEVELMAPPINGDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

class OpenMapping : public QDialog
{
    Q_OBJECT

public:
    explicit OpenMapping(int currentlevel=-1,QWidget *parent = nullptr);
    ~OpenMapping();

    QMap<int, int> getLevelMapping() const;
    int getmapstate() const;

private slots:
    void onGenerateButtonClicked();
    void onConfirmButtonClicked();
    void deleterule();

private:
    void setupUI();
    void clearDynamicWidgets();

    QSpinBox *m_gridCountSpinBox;
    QSpinBox *m_osgbCountSpinBox;
    QPushButton *m_generateButton;
    QPushButton *m_confirmButton;

    QVBoxLayout *m_dynamicLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_scrollWidget;

    QList<QSpinBox*> m_gridLevelSpinners;
    QList<QSpinBox*> m_osgbLevelSpinners;

    QMap<int, int> m_levelMapping;
    int m_currentlevel_formapping;
    int mapstate = -1;
};

#endif // LEVELMAPPINGDIALOG_H
