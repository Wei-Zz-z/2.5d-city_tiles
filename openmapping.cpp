#include "openmapping.h"
#include <QMessageBox>
#include <QDebug>

OpenMapping::OpenMapping(int currentlevel,QWidget *parent)
    : QDialog(parent)
    , m_gridCountSpinBox(nullptr)
    , m_osgbCountSpinBox(nullptr)
    , m_generateButton(nullptr)
    , m_confirmButton(nullptr)
    , m_dynamicLayout(nullptr)
    , m_scrollArea(nullptr)
    , m_scrollWidget(nullptr)
    , m_currentlevel_formapping(currentlevel)
{
    setupUI();
}

OpenMapping::~OpenMapping()
{
    clearDynamicWidgets();
}

void OpenMapping::setupUI()
{
    setWindowTitle("层级映射配置");
    setFixedSize(500, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("层级映射配置", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QLabel *descLabel = new QLabel("请输入网格层级个数，系统将生成对应数量的输入框组", this);
    descLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(descLabel);

    QFrame *inputFrame = new QFrame(this);
    inputFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputFrame);

    QLabel *gridLabel = new QLabel("网格层级个数:", inputFrame);
    m_gridCountSpinBox = new QSpinBox(inputFrame);
    m_gridCountSpinBox->setRange(1, 50);
    m_gridCountSpinBox->setValue(3);
    m_gridCountSpinBox->setMinimumWidth(80);

    QLabel *osgbLabel = new QLabel("OSGB层级个数:", inputFrame);
    m_osgbCountSpinBox = new QSpinBox(inputFrame);
    m_osgbCountSpinBox->setRange(1, 50);
    m_osgbCountSpinBox->setValue(3);
    m_osgbCountSpinBox->setMinimumWidth(80);

    inputLayout->addWidget(gridLabel);
    inputLayout->addWidget(m_gridCountSpinBox);
    inputLayout->addSpacing(20);
    inputLayout->addWidget(osgbLabel);
    inputLayout->addWidget(m_osgbCountSpinBox);
    inputLayout->addStretch();

    mainLayout->addWidget(inputFrame);

    QHBoxLayout *buttonLayout1 = new QHBoxLayout();
    m_generateButton = new QPushButton("生成输入框", this);
    m_generateButton->setMinimumHeight(40);
    m_generateButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    buttonLayout1->addWidget(m_generateButton);
    mainLayout->addLayout(buttonLayout1);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameStyle(QFrame::Box | QFrame::Sunken);
    m_scrollArea->setMinimumHeight(300);

    m_scrollWidget = new QWidget();
    m_dynamicLayout = new QVBoxLayout(m_scrollWidget);
    m_dynamicLayout->setAlignment(Qt::AlignTop);
    m_scrollArea->setWidget(m_scrollWidget);

    mainLayout->addWidget(new QLabel("层级映射输入区域:", this));
    if(m_currentlevel_formapping!=-1){
         mainLayout->addWidget(new QLabel("当前区域最适宜网格层级为："+QString::number(m_currentlevel_formapping)));
    }else{
         mainLayout->addWidget(new QLabel("未输入范围"));
    }
    mainLayout->addWidget(m_scrollArea);

    QHBoxLayout *buttonLayout2 = new QHBoxLayout();
    m_confirmButton = new QPushButton("确认", this);
    m_confirmButton->setMinimumHeight(40);
    m_confirmButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; }");
    QPushButton *cancelButton = new QPushButton("取消", this);
    cancelButton->setMinimumHeight(40);
    cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    QPushButton * deleteButton = new QPushButton("删除规则", this);
    deleteButton->setMinimumHeight(40);
    deleteButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");

    buttonLayout2->addWidget(m_confirmButton);
    buttonLayout2->addWidget(cancelButton);
    buttonLayout2->addWidget(deleteButton);
    mainLayout->addLayout(buttonLayout2);

    connect(m_generateButton, &QPushButton::clicked, this, &OpenMapping::onGenerateButtonClicked);
    connect(m_confirmButton, &QPushButton::clicked, this, &OpenMapping::onConfirmButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(deleteButton, &QPushButton::clicked, this, &OpenMapping::deleterule);
}

void OpenMapping::onGenerateButtonClicked()
{
    clearDynamicWidgets();

    int gridCount = m_gridCountSpinBox->value();

    for (int i = 0; i < gridCount; ++i) {
        QFrame *inputGroup = new QFrame(m_scrollWidget);
        inputGroup->setFrameStyle(QFrame::Box | QFrame::Raised);
        inputGroup->setMinimumHeight(60);

        QHBoxLayout *groupLayout = new QHBoxLayout(inputGroup);

        QLabel *indexLabel = new QLabel(QString("映射 %1").arg(i + 1), inputGroup);
        indexLabel->setMinimumWidth(60);
        indexLabel->setAlignment(Qt::AlignCenter);

        QLabel *gridInputLabel = new QLabel("网格层级:", inputGroup);
        QSpinBox *gridSpinBox = new QSpinBox(inputGroup);
        gridSpinBox->setRange(1, 100);
        gridSpinBox->setValue(m_currentlevel_formapping + i);
        gridSpinBox->setMinimumWidth(80);

        QLabel *osgbInputLabel = new QLabel("OSGB层级:", inputGroup);
        QSpinBox *osgbSpinBox = new QSpinBox(inputGroup);
        osgbSpinBox->setRange(1, 100);
        osgbSpinBox->setValue(5 + i);
        osgbSpinBox->setMinimumWidth(80);

        groupLayout->addWidget(indexLabel);
        groupLayout->addWidget(gridInputLabel);
        groupLayout->addWidget(gridSpinBox);
        groupLayout->addWidget(osgbInputLabel);
        groupLayout->addWidget(osgbSpinBox);
        groupLayout->addStretch();

        m_dynamicLayout->addWidget(inputGroup);
        m_gridLevelSpinners.append(gridSpinBox);
        m_osgbLevelSpinners.append(osgbSpinBox);
    }

    m_scrollWidget->setLayout(m_dynamicLayout);
}

void OpenMapping::onConfirmButtonClicked()
{
    m_levelMapping.clear();

    if (m_gridLevelSpinners.isEmpty()) {
        QMessageBox::information(this, "提示", "请先生成输入框！");
        return;
    }

    for (int i = 0; i < m_gridLevelSpinners.size(); ++i) {
        int gridLevel = m_gridLevelSpinners[i]->value();
        int osgbLevel = m_osgbLevelSpinners[i]->value();

        if (m_levelMapping.contains(gridLevel)) {
            QMessageBox::warning(this, "错误",
                                QString("网格层级 %1 重复！每个网格层级必须唯一。").arg(gridLevel));
            m_levelMapping.clear();
            return;
        }

        m_levelMapping.insert(gridLevel, osgbLevel);
    }
    mapstate = 0;
    accept();
}

void OpenMapping::deleterule(){
    m_levelMapping.clear();
    mapstate = -1;
    accept();
}

void OpenMapping::clearDynamicWidgets()
{
    m_gridLevelSpinners.clear();
    m_osgbLevelSpinners.clear();

    QLayoutItem *child;
    while ((child = m_dynamicLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}

QMap<int, int> OpenMapping::getLevelMapping() const
{
    return m_levelMapping;
}
int OpenMapping::getmapstate() const
{
    return mapstate;
}
