/***************************************************************************
*   This file is part of the leonis project                               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   zhouguangming@maccura.com                                             *
*                                                                         *
**                   GNU General Public License Usage                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*                                                                         *
**                  GNU Lesser General Public License                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation, either version 3 of the    *
*   License, or (at your option) any later version.                       *
*   You should have received a copy of the GNU Lesser General Public      *
*   License along with this library.                                      *
*   If not, see <http://www.gnu.org/licenses/>.                           *
*                                                                         *
*   This library is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
****************************************************************************/

///////////////////////////////////////////////////////////////////////////
/// @file     QAssayTabWidgetContainer.cpp
/// @brief    测试项目选择页面容器
///
/// @author   5774/WuHongTao
/// @date     2022年5月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssayTabWidgetContainer.h"
#include "ui_QAssayTabWidgetContainer.h"
#include <QRadioButton>
#include "manager/SystemConfigManager.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "shared/ReagentCommon.h"
#include "shared/QAssayTabWidgets.h"
#include "QSampleAssayModel.h"

#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/common/Mlog/mlog.h"

#define ASSAY_ROW_AMOUNT_SINGLE     5   /// 单项目位置的默认行数
#define AMOUNT_COL                  7   /// 一页的总列数
// 最小值为6
#define ROW_AMOUNT_UPPER_LIMIT      6   /// 组合项目最大的行数限制

#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
}

QAssayTabWidgetContainer::QAssayTabWidgetContainer(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(-1)
    , m_storeDilution(1)
    , m_preDilutionFactor(-1)
    , m_currentSampleType(-1)
    , m_audit(false)
    , m_timer(nullptr)
{
    ui = new Ui::QAssayTabWidgetContainer();
    ui->setupUi(this);
    Init();
}

QAssayTabWidgetContainer::~QAssayTabWidgetContainer()
{
}

///
///  @brief 初始化
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年6月30日，新建函数
///
void QAssayTabWidgetContainer::Init()
{
    // 删除按钮
    connect(ui->pushButton_del, &QPushButton::clicked, this, &QAssayTabWidgetContainer::ClearSingalAssayCardInfo);

    // 清除按钮
    connect(ui->pushButton_clr, &QPushButton::clicked, this, &QAssayTabWidgetContainer::OnClearAllBtn);

    // 组合项目行数改变
    ui->rowAdjust->setValue(ASSAY_ROW_AMOUNT_TOTAL - CommonInformationManager::GetInstance()->GetSingleAssayRowAmount());
    connect(ui->rowAdjust, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &QAssayTabWidgetContainer::OnRowAdjustChanged);

    // 减号“-”按钮
    connect(ui->pushButton_sub, &QPushButton::clicked, this, &QAssayTabWidgetContainer::OnSubBtnClicked);

    // 加号“+”按钮
    connect(ui->pushButton_add, &QPushButton::clicked, this, &QAssayTabWidgetContainer::OnAddBtnClicked);

    connect(this, &QAssayTabWidgetContainer::buttonPress, this,
        [&](int type)
    {
        // 生化选中
        if (type == 0)
        {
            ui->legend_inc->show();
            ui->legend_dec->show();
            ui->legend_inc_icon->show();
            ui->legend_dec_icon->show();
        }

        // 免疫
        if (type == 1)
        {
            ui->legend_inc->hide();
            ui->legend_dec->hide();
            ui->legend_inc_icon->hide();
            ui->legend_dec_icon->hide();
        }
    });

    // 定时器
    m_timer = std::make_shared<QTimer>(new QTimer(this));
    connect(m_timer.get(), &QTimer::timeout, this, &QAssayTabWidgetContainer::OnTimerFunc);
    m_timer->setInterval(500);
    m_timer->start();
}

///
/// @brief 设置当前项目信息
///
/// @param[in]  assayCode  编码
///
/// @return true表示设置成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月2日，新建函数
///
bool QAssayTabWidgetContainer::SetAssayCardInfo(int assayCode)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 判断是否存在对应的类型
    if (m_pageKeyinfo.count(m_currentDevType) <= 0 || m_currentIndex < 0)
    {
        ULOG(LOG_WARN, "Invalid current device type or index.");
        return false;
    }

    // 获取项目信息
    std::shared_ptr<tf::GeneralAssayInfo> spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (spAssay == nullptr)
    {
        ULOG(LOG_WARN, "Invalid general assayinfo.");
        return false;
    }

    // 获取新卡片的位置类型
    AssayButtonData currentAttribute;
    if (!m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByIndex(m_currentIndex, currentAttribute))
    {
        ULOG(LOG_WARN, "Failed to get assay buttondata.");
        return false;
    }

    // 单项目区域不能设置组合项目（TODO：confirm condition：" !spAssay->reportAssay && "）
    if (currentAttribute.bIsProfile == true)
    {
        ULOG(LOG_WARN, "Invalid operation, set union assay at single assay area.");
        return false;
    }

    // 首先清除之前已经设置好的相同属性的卡片
    AssayButtonData attribute;
    int indexPostion = -1;
    if (m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByAssayCode(assayCode, attribute, indexPostion))
    {
        attribute.bIsRequest = false;
        attribute.assayCode = -1;
        attribute.strAssayName = "";
        if (!m_pageKeyinfo[m_currentDevType].assayTable->SetAssayButtonAttribute(indexPostion, attribute))
        {
            ULOG(LOG_WARN, "Failed to set assay buttondata at %d.", indexPostion);
            return false;
        }
    }

    // 设置新的卡片信息
    currentAttribute.bIsRequest = true;
    currentAttribute.assayCode = assayCode;
    currentAttribute.strAssayName = QString::fromStdString(spAssay->assayName);
    if (!m_pageKeyinfo[m_currentDevType].assayTable->SetAssayButtonAttribute(m_currentIndex, currentAttribute))
    {
        ULOG(LOG_WARN, "Failed to set assay buttondata at %d.", m_currentIndex);
        return false;
    }

    m_assayPostionMap[assayCode] = std::make_pair(m_currentDevType, m_currentIndex);

    return true;
}

void QAssayTabWidgetContainer::ClearSingalAssayCardInfo()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 判断是否存在对应的类型
    if (m_pageKeyinfo.count(m_currentDevType) <= 0 || m_currentIndex < 0)
    {
        ULOG(LOG_WARN, "Invalid current device type or index.");
        return;
    }
    // 获取新卡片的位置类型
    AssayButtonData currentAttribute;
    if (m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByIndex(m_currentIndex, currentAttribute))
    {
        // 更新变更记录，（以解决删除位置后，没有获取到变更的问题）
        if (currentAttribute.assayCode != -1)
        {
            m_assayPostionMap[currentAttribute.assayCode] = std::make_pair(m_currentDevType, -1);
        }

        currentAttribute.bIsRequest = false;
        currentAttribute.assayCode = -1;
        currentAttribute.strAssayName = "";
        m_pageKeyinfo[m_currentDevType].assayTable->SetAssayButtonAttribute(m_currentIndex, currentAttribute);
    }
}

void QAssayTabWidgetContainer::ClearAllAssayCardInfo()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 判断是否存在对应的类型
    if (m_pageKeyinfo.count(m_currentDevType) <= 0)
    {
        ULOG(LOG_WARN, "Invalid current device type.");
        return;
    }

    std::vector<int> delAssayCodes;
    m_pageKeyinfo[m_currentDevType].assayTable->ClearAllButtons(delAssayCodes);

    // 更新变化的assayCode
    for (int i = 0; i < delAssayCodes.size(); ++i)
    {
        m_assayPostionMap[delAssayCodes[i]] = std::make_pair(m_currentDevType, -1);
    }
}

void QAssayTabWidgetContainer::ClearAllBtnSelectStat()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 判断是否存在对应的类型
    if (m_pageKeyinfo.count(m_currentDevType) <= 0)
    {
        ULOG(LOG_WARN, "Invalid current device type.");
        return;
    }
    m_pageKeyinfo[m_currentDevType].assayTable->ClearAllSeletedStatu();
}

void QAssayTabWidgetContainer::DeleteAssayAttriCard(const QString& assayName)
{
    ULOG(LOG_INFO, "%s %s", __FUNCTION__, assayName.toStdString());

    // 获取新卡片的位置类型
    AssayButtonData currentAttribute;
    if (m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByName(assayName, currentAttribute, m_currentIndex))
    {
        currentAttribute.bIsRequest = false;
        currentAttribute.assayCode = -1;
        currentAttribute.strAssayName = "";
        m_pageKeyinfo[m_currentDevType].assayTable->SetAssayButtonAttribute(m_currentIndex, currentAttribute);
    }
}

///
/// @brief 设置稀释方式
///
/// @param[in]  absortSize  稀释方式
/// @param[in]  dulitions   稀释倍数
///
/// @return true设置成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月14日，新建函数
///
bool QAssayTabWidgetContainer::SetDilution(SampleSize absortSize, int dulitions)
{
    m_duliton.first = absortSize;
    if (absortSize == USER_SIZE || absortSize == MANUAL_SIZE)
    {
        if (absortSize == USER_SIZE)
        {
            m_preDilutionFactor = 1;
        }

        m_duliton.second = dulitions;
    }
    else
    {
        m_duliton.second = 1;
    }

    return true;
}

///
/// @brief 设置组合项目位置信息
///
/// @param[in]  profileId  组合项目id码
///
/// @return true表示设置成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月28日，新建函数
///
bool QAssayTabWidgetContainer::SetProfileCardInfo(int profileId)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 判断是否存在对应的类型
    if (m_pageKeyinfo.count(m_currentDevType) <= 0 || m_currentIndex < 0)
    {
        ULOG(LOG_WARN, "Invalid device type or index.");
        return false;
    }

    // 获取组合项目的卡片位置信息
    AssayButtonData currentAttribute;
    if (!m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByIndex(m_currentIndex, currentAttribute)
        || currentAttribute.bIsProfile != true)
    {
        ULOG(LOG_WARN, "Invalid device type or index.");
        return false;
    }

    // 获取通用组合项目地图
    auto ProfileAssayMaps = CommonInformationManager::GetInstance()->GetCommonProfileMap();
    if (!ProfileAssayMaps || ProfileAssayMaps.value().count(profileId - UI_PROFILESTART) == 0)
    {
        // 查询失败
        ULOG(LOG_WARN, "Faield to get profile.");
        return false;
    }

    // 首先清除之前已经设置好的相同属性的卡片
    AssayButtonData attribute;
    int indexPostion = -1;
    if (m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByAssayCode(profileId, attribute, indexPostion))
    {
        attribute.bIsRequest = false;
        attribute.assayCode = -1;
        attribute.strAssayName = "";
        if (!m_pageKeyinfo[m_currentDevType].assayTable->SetAssayButtonAttribute(indexPostion, attribute))
        {
            ULOG(LOG_WARN, "Faield to set profile attribute.");
            return false;
        }
    }

    currentAttribute.bIsRequest = true;
    currentAttribute.assayCode = profileId;
    currentAttribute.strAssayName = QString::fromStdString(ProfileAssayMaps.value()[profileId - UI_PROFILESTART]->profileName);
    if (!m_pageKeyinfo[m_currentDevType].assayTable->SetAssayButtonAttribute(m_currentIndex, currentAttribute))
    {
        ULOG(LOG_WARN, "Faield to set profile attribute.");
        return false;
    }

    m_assayPostionMap[profileId] = std::make_pair(m_currentDevType, m_currentIndex);
    return true;
}

bool QAssayTabWidgetContainer::ffbidenAssay(int assayCode)
{
    // 若不是工作页面的选择框，此处选择不做限制
    if (m_currentTableType != ASSAYTYPE1_WORKPAGE)
    {
        return false;
    }

    // 若是血清指数,只支持血清血浆
    if (IsSIND(assayCode))
    {
        if (tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ == m_currentSampleType)
        {
            return false;
        }

        return true;
    }

    // 若是NA,K,CL，支持血清血浆和尿液
    if (IsISE(assayCode))
    {
        if (tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ == m_currentSampleType
            || tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY == m_currentSampleType)
        {
            return false;
        }

        return true;
    }

    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (spAssay == Q_NULLPTR)
    {
        return true;
    }

    // 项目属于免疫，直接返回可选
    if (spAssay->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        return false;
    }

    // 获取项目对应的样本类型
    std::set<int32_t> sampleSourceTypes;
    CommonInformationManager::GetInstance()->GetSampleTypeFromCode(assayCode, sampleSourceTypes);
    if (sampleSourceTypes.count(m_currentSampleType) > 0)
    {
        return false;
    }

    return true;
};

///
/// @brief 清除控件的暂时属性
///
/// @param[in]  assayTable		控件容器
/// @param[in]  indexPostion    控件位置
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月24日，新建函数
///
void QAssayTabWidgetContainer::ClearTempAttribute(QAssayTabWidgets* assayTable, int indexPostion)
{
    // 参数检查
    if (assayTable == nullptr)
    {
        return;
    }

    // 获取属性
    AssayButtonData attribute;
    if (!assayTable->GetAttributeByIndex(indexPostion, attribute))
    {
        return;
    }

    // 重复次数设置为0
    attribute.testTimes = 0;

    if (!assayTable->SetAssayButtonAttribute(indexPostion, attribute))
    {
        return;
    }
}

///
/// @brief 获取免疫项目的默认稀释倍数
///
/// @param[in]  assayCode  免疫项目的项目code
///
/// @return 稀释倍数
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月5日，新建函数
///
int QAssayTabWidgetContainer::GetDefaultDuliTon(int assayCode)
{
    auto spAssayInfo = CommonInformationManager::GetInstance()->GetImmuneAssayInfo(assayCode, tf::DeviceType::DEVICE_TYPE_I6000);
    if (spAssayInfo == Q_NULLPTR || spAssayInfo->reactInfo.dilutionRatio <= 0)
    {
        return 1;
    }

    return spAssayInfo->reactInfo.dilutionRatio;
}

///
/// @brief 根据编号清除暂存参数
///
/// @param[in]  assayTable  页面句柄
/// @param[in]  assaycode   项目编号
///
/// @return true代表成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月24日，新建函数
///
bool QAssayTabWidgetContainer::ClearTempAttributeByAssayCode(QAssayTabWidgets* assayTable, int assaycode)
{
    // 参数检查
    if (assayTable == nullptr)
    {
        return false;
    }

    // 获取属性
    AssayButtonData attribute;
    int indexPostion = -1;
    if (!assayTable->GetAttributeByAssayCode(assaycode, attribute, indexPostion))
    {
        return false;
    }

    // 重复次数设置为0
    attribute.testTimes = 0;
    if (!assayTable->SetAssayButtonAttribute(indexPostion, attribute))
    {
        return false;
    }

    return true;
}

struct AssayTypeBtnInfo
{
    AssayTypeBtnInfo(const QString& txtNm, const QString& objNm, tf::AssayClassify::type aType)
        : m_btnTxtName(txtNm), m_btnObjName(objNm), m_btnAssayType(aType)
    {}
    QString m_btnTxtName;
    QString m_btnObjName;
    tf::AssayClassify::type m_btnAssayType;
};

///
/// @brief 初始化项目类型（生\免）选择控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月18日，新建函数
///
void QAssayTabWidgetContainer::StartContainer(AssayType type)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (SystemConfigManager::GetInstance()->IsDebugMode())
    {
        ULOG(LOG_INFO, "Skiped for debug mode.");
        return;
    }

    m_currentTableType = type;
    auto devices = CommonInformationManager::GetInstance()->GetDeviceFromType(\
        std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_C1000, tf::DeviceType::DEVICE_TYPE_ISE1005
        ,tf::DeviceType::DEVICE_TYPE_C200, tf::DeviceType::DEVICE_TYPE_I6000 }));

    bool hasChemistry = false;
    bool hasImmune = false;
    for (const auto& device : devices)
    {
        if (device->deviceType == tf::DeviceType::DEVICE_TYPE_C1000 || \
            device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            hasChemistry = true;
        }
        else
        {
            hasImmune = true;
        }
    }

    std::vector<AssayTypeBtnInfo> deviceNames;
    if (hasChemistry)
    {
        AssayTypeBtnInfo chemi(tr("生化"), QStringLiteral("item_chemistry"), tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
        deviceNames.push_back(chemi);
    }
    if (hasImmune)
    {
        AssayTypeBtnInfo immy(tr("免疫"), QStringLiteral("item_immune"), tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE);
        deviceNames.push_back(immy);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setObjectName(QStringLiteral("btnSwitch"));
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);

    for (const auto& deviceName : deviceNames)
    {
        // 生成选择控件
        QRadioButton* devTypeButton = new QRadioButton(this);
        devTypeButton->setText(deviceName.m_btnTxtName);
        devTypeButton->setObjectName(deviceName.m_btnObjName);
        buttonLayout->addWidget(devTypeButton);

        AssayPageKeyInfo assayKeyInfo;
        assayKeyInfo.devButton = devTypeButton;
        assayKeyInfo.buttonName = deviceName.m_btnTxtName;
        assayKeyInfo.pageType = type;

        // 页面变化的时候发出这个消息
        QAssayTabWidgets* assayTable = new QAssayTabWidgets(ui->stacked_assay_table);
        connect(assayTable, &QTabWidget::tabBarClicked, this, [&](int type)
        {
            emit buttonPress(2);
        });

        assayKeyInfo.assayTable = assayTable;
        assayKeyInfo.devType = deviceName.m_btnAssayType;

        m_pageKeyinfo[assayKeyInfo.devType] = assayKeyInfo;
        InitAssayTabWidget(type, assayKeyInfo.devType, assayTable);

        ui->stacked_assay_table->addWidget(assayTable);
        connect(devTypeButton, SIGNAL(clicked()), this, SLOT(OnChangeTabIndex()));
    }

    buttonLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QWidget *dev_type_group = new QWidget(this);
    dev_type_group->setGeometry(18, 12, 480, 70);
    dev_type_group->setLayout(buttonLayout);

    // 默认类型是生化
    m_currentDevType = tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY;
    ui->legend_inc->show();
    ui->legend_dec->show();

    // 若只有免疫存在，则默认免疫
    if (hasImmune && !hasChemistry)
    {
        m_currentDevType = tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE;
        emit buttonPress(1);
    }

    // 若没有，则不显示
    (m_pageKeyinfo.size() < 2) ? dev_type_group->hide() : dev_type_group->show();
    if (!m_pageKeyinfo.empty())
    {
        m_pageKeyinfo.rbegin()->second.devButton->setChecked(true);
    }

    // 根据类型设置不同的按钮类型
    switch (type)
    {
    case ASSAYTYPE1_WORKPAGE:
        ui->stacked_assay_info->setCurrentIndex(0);
        break;
    case ASSAYTYPE2:
    {
        SetLengendStatus(false);
        ui->widget->hide();
    }
    break;
    case ASSAYTYPE3_POSITION_SET:
    {
        SetLengendStatus(false);
        ui->stacked_assay_info->setCurrentIndex(1);
    }
    break;
    default:
        break;
    }
}

///
/// @bref
///		重新设定单项目行数
///
/// @param[in] rowAmount 单项目行数
///
/// @par History:
/// @li 8276/huchunli, 2023年6月13日，新建函数
///
void QAssayTabWidgetContainer::ResetSingleAssayRow(int rowAmount)
{
    m_singleAssayRowAmount = rowAmount;

    std::map<tf::AssayClassify::type, AssayPageKeyInfo>::iterator it = m_pageKeyinfo.begin();
    for (; it != m_pageKeyinfo.end(); ++it)
    {
        it->second.assayTable->RefreshAllButtons(rowAmount);

        // 获取位置产生变化的项目与位置
        std::vector<std::pair<int, int>> changedAssays = it->second.assayTable->GetChangedAssaysList();
        for (const auto& pItem : changedAssays)
        {
            m_assayPostionMap[pItem.first] = std::pair<tf::AssayClassify::type, int>(it->second.devType, pItem.second);
        }

        it->second.assayTable->update();
    }
}

///
///  @brief 清除所有选择
///
///
///
///  @return	
///
///  @par History: 
/// @li 8276/huchunli, 2023年6月13日，新建函数
///
void QAssayTabWidgetContainer::OnClearAllBtn()
{
    TipDlg noticeDlg(tr("确认清空所有的选择按钮？"), TipDlgType::TWO_BUTTON);
    if (noticeDlg.exec() != QDialog::Accepted)
    {
        return;
    }

    ClearAllAssayCardInfo();
}

///
/// @bref
///		设置单项目行数
///
/// @param[in] rowValue 行数
///
/// @par History:
/// @li 8276/huchunli, 2023年6月13日，新建函数
///
void QAssayTabWidgetContainer::OnRowAdjustChanged(int rowValue)
{
    if (rowValue < 0 || rowValue > ASSAY_ROW_AMOUNT_TOTAL)
    {
        return;
    }

    ResetSingleAssayRow(ASSAY_ROW_AMOUNT_TOTAL - rowValue);
}

///
///  @brief 减号“-”按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年6月30日，新建函数
///
void QAssayTabWidgetContainer::OnSubBtnClicked()
{
    int iRowValue = ui->rowAdjust->value();
    // 最小值为1
    if (iRowValue <= 1)
    {
        ui->rowAdjust->setValue(1);
        ui->pushButton_sub->setDisabled(true);
        return;
    }
    ui->pushButton_add->setDisabled(false);
    ui->rowAdjust->setValue(--iRowValue);

    std::map<tf::AssayClassify::type, AssayPageKeyInfo>::iterator it = m_pageKeyinfo.find(m_currentDevType);
    if (it != m_pageKeyinfo.end() && it->second.assayTable != nullptr)
    {
        m_currentIndex = it->second.assayTable->GetCurrentSelectePosition();
    }
}

///
///  @brief 加号“+”按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年6月30日，新建函数
///
void QAssayTabWidgetContainer::OnAddBtnClicked()
{
    int iRowValue = ui->rowAdjust->value();
    if (iRowValue >= ROW_AMOUNT_UPPER_LIMIT)
    {
        ui->rowAdjust->setValue(ROW_AMOUNT_UPPER_LIMIT);
        ui->pushButton_add->setDisabled(true);
        return;
    }
    ui->pushButton_sub->setDisabled(false);
    ui->rowAdjust->setValue(++iRowValue);

    std::map<tf::AssayClassify::type, AssayPageKeyInfo>::iterator it = m_pageKeyinfo.find(m_currentDevType);
    if (it != m_pageKeyinfo.end() && it->second.assayTable != nullptr)
    {
        m_currentIndex = it->second.assayTable->GetCurrentSelectePosition();
    }
}

///
/// @bref
///		定时器函数
///
/// @par History:
/// @li 8276/huchunli, 2023年9月28日，新建函数
///
void QAssayTabWidgetContainer::OnTimerFunc()
{
    if (m_currentIndex == -1)
    {
        ui->pushButton_del->setEnabled(false);
    }
    else
    {
        AssayButtonData currentAttribute;
        if (m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByIndex(m_currentIndex, currentAttribute))
        {
            ui->pushButton_del->setEnabled(currentAttribute.assayCode != -1);
        }
    }
}

///
/// @brief 初始化项目选择控件
///
/// @param[in]  cornerWidgets  角落控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月18日，新建函数
///
void QAssayTabWidgetContainer::InitAssayTabWidget(AssayType type, tf::AssayClassify::type devType, QAssayTabWidgets* assayTable)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (assayTable == nullptr)
    {
        ULOG(LOG_WARN, "Null assaytbwidgets.");
        return;
    }
    // 从配置库中获取设定的行数
    m_singleAssayRowAmount = CommonInformationManager::GetInstance()->GetSingleAssayRowAmount();

    assayTable->SetAssayTabAttribute(type, m_singleAssayRowAmount, ASSAY_ROW_AMOUNT_TOTAL, AMOUNT_COL, UI_ASSAYPAGEMAX);
    // 当项目按钮控件被选中的时候,对应的处理函数
    connect(assayTable, SIGNAL(selectAssay(int)), this, SLOT(OnAssaySelected(int)));
    RefreshPageAssayCards(devType, assayTable);
}

///
/// @bref
///		获取所有的按钮信息，用于设置按钮的内容
///
/// @param[out] btnInfo 返回的所有的按钮信息
/// @param[in] devType 项目类型
///
/// @par History:
/// @li 8276/huchunli, 2023年11月14日，新建函数
///
void QAssayTabWidgetContainer::TakeAssayBtnInfor(AssayButtonDatas& btnInfo, tf::AssayClassify::type devType)
{
    int pageIndex = 0;
    while (pageIndex < UI_ASSAYPAGEMAX)
    {
        // 每页的项目地图
        AssayButtons pageAssaymaps;

        // 获取单项目列表
        auto assaySingleMap = GetSingleAssayCardList(devType, pageIndex);
        // 若单项目获取成功，则加入地图中
        if (assaySingleMap)
        {
            pageAssaymaps.insert(assaySingleMap->begin(), assaySingleMap->end());
        }

        // 获取组合项目列表
        auto assayCombMap = GetCombAssayCardList(devType, pageIndex);
        if (assayCombMap)
        {
            pageAssaymaps.insert(assayCombMap->begin(), assayCombMap->end());
        }

        // 加入到总页面地图
        btnInfo.push_back(pageAssaymaps);
        pageIndex++;
    }

}

///
/// @brief 更新测试分类的框表
///
/// @param[in]  devType  测试项目分类
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月18日，新建函数
///
void QAssayTabWidgetContainer::RefreshPageAssayCards(tf::AssayClassify::type devType, QAssayTabWidgets* assayTable)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (assayTable == nullptr)
    {
        ULOG(LOG_WARN, "Invalid assaytable.");
        return;
    }

    AssayButtonDatas pageData;
    TakeAssayBtnInfor(pageData, devType);

    assayTable->UpdateTabAssayStatus(pageData);

    // 更新组合行数+ -按钮的状态[bug 22680]
    int curUnion = ui->rowAdjust->value();
    ui->pushButton_sub->setDisabled(curUnion <= 1);
    ui->pushButton_add->setDisabled(curUnion >= ROW_AMOUNT_UPPER_LIMIT);
}

///
/// @brief 刷新所有的项目状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QAssayTabWidgetContainer::RefreshAssayCards()
{
    m_audit = false;

    // 恢复行数占比
    m_singleAssayRowAmount = CommonInformationManager::GetInstance()->GetSingleAssayRowAmount();
    ui->rowAdjust->setValue(ASSAY_ROW_AMOUNT_TOTAL - m_singleAssayRowAmount);

    // 清除位置修改列表
    for (const auto& ap : m_assayPostionMap)
    {
        int curIndex = ap.second.second;
        if (curIndex == -1)
        {
            continue;
        }
        // 获取新卡片的位置类型
        AssayButtonData currentAttribute;
        if (m_pageKeyinfo[m_currentDevType].assayTable->GetAttributeByIndex(curIndex, currentAttribute))
        {
            currentAttribute.bIsRequest = false;
            currentAttribute.assayCode = -1;
            currentAttribute.strAssayName = "";
            m_pageKeyinfo[m_currentDevType].assayTable->SetAssayButtonAttribute(curIndex, currentAttribute);
        }
    }
    m_assayPostionMap.clear();

    // 刷新原有位置的卡片
    for (const auto& page : m_pageKeyinfo)
    {
        RefreshPageAssayCards(page.first, page.second.assayTable);
    }
}

void QAssayTabWidgetContainer::RefreshAssayCardsWithNoClear()
{
    for (const auto& page : m_pageKeyinfo)
    {
        AssayButtonDatas pageData;
        TakeAssayBtnInfor(pageData, page.first);

        page.second.assayTable->UpdateTabAssayStatusWithNoClear(pageData);
    }
}

///
/// @brief 获取单项目卡片显示列表
///
/// @param[in]  assayType  项目类型
/// @param[in]  pageIndex  页面索引
///
/// @return 返回项目卡片的列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月27日，新建函数
///
boost::optional<AssayButtons> QAssayTabWidgetContainer::GetSingleAssayCardList(tf::AssayClassify::type& assayType, int pageIndex)
{
    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
    std::vector<tf::GeneralAssayPageItem> assayItems;
    if (assayType == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
    {
        assayMgr->GetAssayItems(tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY, pageIndex, assayItems);
        assayMgr->GetAssayItems(tf::AssayClassify::type::ASSAY_CLASSIFY_ISE, pageIndex, assayItems);
    }
    else
    {
        assayMgr->GetAssayItems(assayType, pageIndex, assayItems);
    }

    AssayButtons pageMap;
    // 将结果设置到界面
    for (const auto& assayItem : assayItems)
    {
        // 项目按钮数据
        AssayButtonData assayBtnData;
        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = assayMgr->GetAssayInfo(assayItem.assayCode);
        if (nullptr == spAssayInfo)
        {
            ULOG(LOG_WARN, "Invalid assay code,");
            continue;
        }

        assayBtnData.bIsForbidden = ffbidenAssay(spAssayInfo->assayCode);
        assayBtnData.bIsUse = true;
        assayBtnData.assayCode = spAssayInfo->assayCode;
        assayBtnData.strAssayName = QString::fromStdString(spAssayInfo->assayName);

        // 根据页面类型来配置页面
        if (m_pageKeyinfo[assayType].pageType == ASSAYTYPE1_WORKPAGE)
        {
            auto assayStatus = QSystemMonitor::GetInstance().GetAssayStatus(assayBtnData.assayCode);
            if (assayStatus)
            {
                assayBtnData.bIsCaliAbn = assayStatus->abnormalCalibrate;
                assayBtnData.bIsMask = assayStatus->maskAssay;
                assayBtnData.bIsRegentAbn = assayStatus->abnormalReagent;
            }
        }
        // 检测position index的合法性
        if (spAssayInfo->positionIdx < 0 || spAssayInfo->positionIdx > (m_singleAssayRowAmount * AMOUNT_COL) - 1)
        {
            ULOG(LOG_WARN, "Invalid signle assay position, page:%d, pos:%d.", pageIndex, spAssayInfo->positionIdx);
            continue;
        }

        // 计算项目位置
        int index = pageIndex * UI_ASSAYPAGENUM + spAssayInfo->positionIdx;
        pageMap.emplace(make_pair(index, assayBtnData));
    }

    return boost::make_optional(pageMap);
}

///
/// @brief 获取组合项目显示列表
///
/// @param[in]  assayType  项目类型
/// @param[in]  pageIndex  页面索引
///
/// @return 返回组合项目卡片的列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月27日，新建函数
///
boost::optional<AssayButtons> QAssayTabWidgetContainer::GetCombAssayCardList(const tf::AssayClassify::type& assayType, int pageIndex)
{
    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
    std::vector<::tf::ProfileAssayPageItem> profileItems;
    if (assayType == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
    {
        assayMgr->GetAssayProfiles(tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY, pageIndex, profileItems);
        assayMgr->GetAssayProfiles(tf::AssayClassify::type::ASSAY_CLASSIFY_ISE, pageIndex, profileItems);
    }
    else
    {
        assayMgr->GetAssayProfiles(assayType, pageIndex, profileItems);
    }

    // 获取通用组合项目地图
    auto ProfileAssayMaps = assayMgr->GetCommonProfileMap();
    if (!ProfileAssayMaps)
    {
        // 查询失败
        return boost::none;
    }

    AssayButtons pageMap;
    // 将结果设置到界面
    for (const auto& assayItem : profileItems)
    {
        // 项目按钮数据
        if (ProfileAssayMaps.value().count(assayItem.id) == 0)
        {
            continue;
        }

        AssayButtonData assayBtnData;
        auto assayProfile = ProfileAssayMaps.value()[assayItem.id];
        assayBtnData.bIsUse = true;
        assayBtnData.assayCode = assayProfile->id + UI_PROFILESTART;
        assayBtnData.strAssayName = QString::fromStdString(assayProfile->profileName);

        // 填充组合项目的代码
        for_each(assayItem.subAssays.begin(), assayItem.subAssays.end(), [&](auto assayCode) {
            assayBtnData.bIsProfile = true;
            assayBtnData.strSubAssayList.push_back(assayCode); });
        
        // 检测position index的合法性
        if (assayItem.positionIdx < (m_singleAssayRowAmount * AMOUNT_COL) || assayItem.positionIdx >= UI_ASSAYPAGENUM)
        {
            ULOG(LOG_WARN, "Invalid profle assay position, page:%d, pos:%d.", pageIndex, assayItem.positionIdx);
            continue;
        }

        // 计算项目位置
        int index = pageIndex * UI_ASSAYPAGENUM + assayItem.positionIdx;
        pageMap.emplace(make_pair(index, assayBtnData));
    }

    return boost::make_optional(pageMap);
}

///
/// @brief 完全更新项目卡
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月7日，新建函数
///
void QAssayTabWidgetContainer::RefreshAssayComplete()
{
    for (auto&& page : m_pageKeyinfo)
    {
        page.second.assayTable->ClearComplete();
        RefreshPageAssayCards(page.first, page.second.assayTable);
    }
}

///
/// @brief 更新对应页面的稀释倍数状态
///
/// @param[in]  assayTable  项目页面
/// @param[in]  testItem  项目
///
/// @return true，更新成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
bool QAssayTabWidgetContainer::UpdateDulition(QAssayTabWidgets* assayTable, tf::TestItem& testItem)
{
    int indexPostion = -1;
    AssayButtonData attribute;
    if (!assayTable->GetAttributeByAssayCode(testItem.assayCode, attribute, indexPostion))
    {
        return false;
    }

    // 若稀释倍数等于1,则使用标准的，增量减量等
    if (testItem.dilutionFactor == 1)
    {
        switch (testItem.suckVolType)
        {
        case tf::SuckVolType::SUCK_VOL_TYPE_DEC:
            attribute.enSampleSize = DEC_SIZE;
            break;
        case tf::SuckVolType::SUCK_VOL_TYPE_INC:
            attribute.enSampleSize = INC_SIZE;
            break;
        case tf::SuckVolType::SUCK_VOL_TYPE_STD:
            attribute.enSampleSize = STD_SIZE;
            break;
        default:
            attribute.enSampleSize = STD_SIZE;
            break;
        }

    }
    // 需要使用稀释倍数
    else
    {
        attribute.enSampleSize = USER_SIZE;
        attribute.iDilution = testItem.dilutionFactor;
    }

    // 刷新项目选项卡
    if (!assayTable->SetAssayButtonAttribute(indexPostion, attribute))
    {
        return false;
    }

    return true;
}

///
/// @brief 更新项目的稀释信息
///
/// @param[in]  testItem  
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
bool QAssayTabWidgetContainer::UpdateDulition(tf::TestItem& testItem)
{
    for (const auto& page : m_pageKeyinfo)
    {
        if (UpdateDulition(page.second.assayTable, testItem))
        {
            return true;
        }
    }
    return false;
}

///
/// @brief 设置选中的项目
///
///
/// @return true:设置成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月15日，新建函数
///
bool QAssayTabWidgetContainer::SetSelectAssay(const std::vector<int>& assayCodes, int type)
{
    // 设置项目
    for (int assayCode : assayCodes)
    {
        for (const auto& page : m_pageKeyinfo)
        {
            if (type != page.second.pageType)
            {
                continue;
            }
            AssayButtonData attribute;
            int indexPostion;
            bool isGet = page.second.assayTable->GetAttributeByAssayCode(assayCode, attribute, indexPostion);
            if (isGet)
            {
                ProcSelectButton(page.second.assayTable, indexPostion);
            }
        }
    }

    return true;
}

///
/// @brief 更新项目测试次数
///
/// @param[in]  assayCode  项目编号
/// @param[in]  number	   次数
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月24日，新建函数
///
bool QAssayTabWidgetContainer::UpdateTestItemDbList(int assayCode, int number)
{
    AssayButtonData attribute;
    int indexPostion = -1;

    for (const auto& page : m_pageKeyinfo)
    {
        if (page.second.assayTable->GetAttributeByAssayCode(assayCode, attribute, indexPostion))
        {
            // 获取测试项目的id列表
            int dbsize = attribute.guidList.size();
            // 减少
            if (dbsize > number)
            {
                attribute.guidList.erase((attribute.guidList.begin() + number), attribute.guidList.end());
            }
            // 增加
            else if (dbsize < number)
            {
                // 增加-1表示新增
                attribute.guidList.insert(attribute.guidList.end(), (number - dbsize), -1);
            }
            // 等于的话不做任何变化
            else
            {
                return true;
            }

            // 回设置
            page.second.assayTable->SetAssayButtonAttribute(indexPostion, attribute);

            return true;
        }
    }

    return false;

}

///
/// @brief 更新项目的状态
///
/// @param[in]  testItem  测试项目信息
///
/// @return true，更新成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
bool QAssayTabWidgetContainer::UpdateTestItemStatus(QAssayTabWidgets* assayTable, tf::TestItem& testItem)
{
    ULOG(LOG_INFO, __FUNCTION__);

    AssayButtonData attribute;
    int indexPostion = -1;
    if (!assayTable->GetAttributeByAssayCode(testItem.assayCode, attribute, indexPostion))
    {
        ULOG(LOG_WARN, "Failed to get attribute.");
        return false;
    }

    // 设置禁能标志
    attribute.bIsForbidden = ffbidenAssay(testItem.assayCode);

    // 存储数据库的id
    attribute.guidList.push_back(testItem.id);

    // 单测试项目
    if (attribute.strSubAssayList.empty())
    {
        // 复查状态
        if (testItem.__isset.rerun && testItem.rerun
            && !(testItem.__isset.status && testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_TESTED))
        {
            attribute.bIsRequest = false;
            attribute.bIsAnalyzed = false;
            attribute.bIsRecheck = true;
        }
        // 待测试状态
        else if (testItem.__isset.status && ((testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_PENDING)
            || (testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_TESTING)))
        {
            attribute.bIsRequest = true;
            attribute.bIsAnalyzed = false;
            attribute.bIsRecheck = false;
        }
        // 测试完毕状态
        else if (testItem.__isset.status && testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_TESTED)
        {
            attribute.bIsRequest = false;
            attribute.bIsAnalyzed = true;
            attribute.bIsRecheck = false;
        }
        else
        {
            attribute.bIsRequest = false;
            attribute.bIsAnalyzed = false;
            attribute.bIsRecheck = false;
        }
		
		// 测试属性转化为选中属性
		SetAttributeFromTestitem(attribute, testItem);
    }
    // ISE或者其他项目的时候，直接设置即可
    else
    {
        // 设置为请求状态
        attribute.bIsRequest = true;
        attribute.bIsAnalyzed = false;
        attribute.bIsRecheck = false;
    }

    // 修改状态
    assayTable->SetAssayButtonAttribute(indexPostion, attribute);
    return true;
}

///
/// @brief 更新测试项目信息
///
/// @param[in]  testItem  测试项目详细信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
bool QAssayTabWidgetContainer::UpdateTestItemStatus(tf::TestItem& testItem)
{
    for (const auto& page : m_pageKeyinfo)
    {
        if (UpdateTestItemStatus(page.second.assayTable, testItem))
        {
            return true;
        }
    }

    return false;
}

bool QAssayTabWidgetContainer::UpdateAssayProfileStatus(QAssayTabWidgets* assayTable, int assayCode)
{
    AssayButtonData attribute;
    int indexPostion = -1;
    if (!assayTable->GetAttributeByAssayCode(assayCode, attribute, indexPostion))
    {
        return false;
    }

    // 设置为请求状态
    attribute.bIsRequest = true;
    // 修改状态
    assayTable->SetAssayButtonAttribute(indexPostion, attribute);
    return true;
}

///
/// @brief 更新组合项目信息
///
/// @param[in]  assayCode  组合项目编号
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
bool QAssayTabWidgetContainer::UpdateAssayProfileStatus(int assayCode)
{
    for (const auto& page : m_pageKeyinfo)
    {
        if (UpdateAssayProfileStatus(page.second.assayTable, assayCode))
        {
            return true;
        }
    }

    return false;
}

bool QAssayTabWidgetContainer::GetSampleTestItemInfo(QAssayTabWidgets* assayTable, std::vector<::tf::TestItem>& testItems, std::vector<int64_t>& profiles)
{
    // 遍历所有选中项目信息
    for (int i = 0; i < assayTable->GetAssayButtonCount(); i++)
    {
        // 获取选中项目按钮的信息失败则跳过
        AssayButtonData stuAssayBtnData;
        if (!assayTable->GetAttributeByIndex(i, stuAssayBtnData))
        {
            continue;
        }

        // 仅仅是展示，未使用
        if (!stuAssayBtnData.bIsUse)
        {
            continue;
        }

        // 构造测试项目信息
        if (stuAssayBtnData.bIsProfile && stuAssayBtnData.bIsRequest)
        {
            // 组合项目编号
            profiles.push_back(stuAssayBtnData.assayCode);
        }
        else
        {
            // 非组合项目
            ::tf::TestItem testItem;
            std::shared_ptr<::tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuAssayBtnData.assayCode);
            if (nullptr == spAssayInfo)
            {
                ULOG(LOG_WARN, "Invalid assay name,");
                continue;
            }

			// 1：首先设置稀释类型和项目基础信息
            testItem.__set_assayCode(spAssayInfo->assayCode);
            testItem.__set_suckVolType((::tf::SuckVolType::type)stuAssayBtnData.enSampleSize);

			// 2： 再次根据情况设置稀释的具体信息
            // 测试项目请求状态
            if (stuAssayBtnData.bIsRequest && !stuAssayBtnData.bIsAnalyzed && !stuAssayBtnData.bIsRecheck)
            {
                testItem.__set_status(::tf::TestItemStatus::TEST_ITEM_STATUS_PENDING);

                // 手工稀释
                if (stuAssayBtnData.enSampleSize == MANUAL_SIZE)
                {
                    testItem.__set_suckVolType(::tf::SuckVolType::type::SUCK_VOL_TYPE_STD);
					testItem.__set_preDilutionFactor(stuAssayBtnData.iDilution);
                    m_storeDilution = stuAssayBtnData.iDilution;
                }
                else
                {
					// 机内稀释首先将机外稀释置为1
					testItem.__set_preDilutionFactor(1);
                    testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
                    m_storeDilution = 1;
                }

                m_requestAssays++;
            }
            // 已完成
            else if (!stuAssayBtnData.bIsRequest && stuAssayBtnData.bIsAnalyzed && !stuAssayBtnData.bIsRecheck)
            {
				// 当增加项目的手工稀释倍数以后，已经完成的稀释倍数可能变化（复查->已完成）
				// 手工稀释
				if (stuAssayBtnData.enSampleSize == MANUAL_SIZE)
				{
					testItem.__set_suckVolType(::tf::SuckVolType::type::SUCK_VOL_TYPE_STD);
					testItem.__set_preDilutionFactor(stuAssayBtnData.iDilution);
				}
				else
				{
					// 机内稀释首先将机外稀释置为1
					testItem.__set_preDilutionFactor(1);
					testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
				}

                testItem.__set_rerun(false);
            }
            // 复查
            else if (!stuAssayBtnData.bIsRequest && !stuAssayBtnData.bIsAnalyzed && stuAssayBtnData.bIsRecheck)
            {
                testItem.__set_rerun(true);
                //	复查的时候设置状态为项目请求状态
                testItem.__set_status(::tf::TestItemStatus::TEST_ITEM_STATUS_PENDING);
                // 手工稀释
                if (stuAssayBtnData.enSampleSize == MANUAL_SIZE)
                {
                    testItem.__set_suckVolType(::tf::SuckVolType::type::SUCK_VOL_TYPE_STD);
					testItem.__set_preDilutionFactor(stuAssayBtnData.iDilution);
                    m_storeDilution = stuAssayBtnData.iDilution;
                }
                else
                {
					// 机内稀释首先将机外稀释置为1
					testItem.__set_preDilutionFactor(1);
                    testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
                    m_storeDilution = 1;
                }

                m_requestAssays++;
            }
            // 未在范围内
            else
            {
                testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
                continue;
            }

            // 当拥有id的时候表示非新增项目
            int i = 0;
            for (auto guid : stuAssayBtnData.guidList)
            {
                if (guid > 0)
                {
                    testItem.__set_id(guid);
                }
                else
                {
                    testItem.__isset.id = false;
                    testItem.id = -1;
                }

                // 重复索引
                testItem.__set_repeatIdx(i);
                i++;

                testItems.push_back(testItem);
            }
        }
    }

    return true;
}

///
/// @brief 获取所有页面的已选择的测试项目
///
/// @param[in]  testItems  单一测试项目
/// @param[in]  profiles   组合项目
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月18日，新建函数
///

bool QAssayTabWidgetContainer::GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems, std::vector<int64_t>& profiles)
{
    for (const auto& page : m_pageKeyinfo)
    {
        std::vector<::tf::TestItem> tmpTestItems;
        std::vector<int64_t> tmpProfiles;
        if (!GetSampleTestItemInfo(page.second.assayTable, tmpTestItems, tmpProfiles))
        {
            return false;
        }

        testItems.insert(testItems.end(), tmpTestItems.begin(), tmpTestItems.end());
        profiles.insert(profiles.end(), tmpProfiles.begin(), tmpProfiles.end());
    }

    return true;
}

///
/// @brief 设置图例状态
///
/// @param[in]  status  状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QAssayTabWidgetContainer::SetLengendStatus(bool status)
{
    // 获取图例控件
    QList<QLabel*> labelMember = findChildren<QLabel*>(QString(), Qt::FindDirectChildrenOnly);
    for_each(labelMember.begin(), labelMember.end(), [&](auto label) {label->setVisible(status); });
}

///
/// @brief 设置按钮属性
///
/// @param[in]  attribute  按钮属性
/// @param[in]  testItem   测试项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月7日，新建函数
///
void QAssayTabWidgetContainer::SetAttributeFromTestitem(AssayButtonData& attribute, const tf::TestItem& testItem)
{
	if (!testItem.__isset.preDilutionFactor)
	{
		ULOG(LOG_WARN, "The preDilutionFactor is not set ,please check it ,testItem id : %d", testItem.id);
		return;
	}

	// 机内稀释的时候preDilutionFacto==1
	if (testItem.preDilutionFactor == 1)
	{
		if (testItem.dilutionFactor == 1)
		{
			switch (testItem.suckVolType)
			{
				// 增量
				case tf::SuckVolType::type::SUCK_VOL_TYPE_INC:
					attribute.enSampleSize = INC_SIZE;
					break;
					// 减量
				case tf::SuckVolType::type::SUCK_VOL_TYPE_DEC:
					attribute.enSampleSize = DEC_SIZE;
					break;
					// 标准--用户自定义
				case tf::SuckVolType::type::SUCK_VOL_TYPE_STD:
					attribute.enSampleSize = STD_SIZE;
					break;
				default:
					attribute.enSampleSize = USER_SIZE;
					break;
			}
		}
		else
		{
			attribute.enSampleSize = USER_SIZE;
		}

		attribute.iDilution = testItem.dilutionFactor;
	}
	// 机外稀释
	else
	{
		attribute.iDilution = testItem.preDilutionFactor;
		attribute.enSampleSize = MANUAL_SIZE;
	}
}

///
/// @brief 稀释策略是否规范（1：不能手工稀释和自动稀释混合，2：不能多次手工稀释）
///
///
/// @return true,表示pass
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月8日，新建函数
///
bool QAssayTabWidgetContainer::IsDiluTionPolicy(AssayButtonData& data)
{
    // 请求测试(取消请求测试)
    if (data.bIsRequest && !data.bIsAnalyzed && !data.bIsRecheck)
    {
        return true;
    }

    // 复查（取消复查）
    if (!data.bIsRequest && !data.bIsAnalyzed && data.bIsRecheck)
    {
        return true;
    }

    std::vector<::tf::TestItem> testItems;
    std::vector<int64_t> profiles;
    // 请求项目的个数
    m_requestAssays = 0;
    m_storeDilution = 0;

    if (m_audit)
    {
        ReportErr(true, tr("样本已审核，不可修改！"));
        return false;
    }

    // 是否适配ISE和LHI(ISE和LHI只能使用手工稀释和常量稀释)
    auto isFitForSINandISE = [&]()->bool
    {
        if (m_duliton.first == STD_SIZE || m_duliton.first == MANUAL_SIZE)
        {
            return true;
        }
        return false;
    };

    // ISE,SIND,HBA1c(糖化血红蛋白),都属于特殊项目，只能做手工稀释和常量稀释
    auto isSpecialAssay = [&](int assayCode)->bool
    {
        if (IsSIND(data.assayCode)
            || IsISE(data.assayCode)
            || (data.assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_HBA1c))
        {
            return true;
        }

        return false;
    };

    if (isSpecialAssay(data.assayCode) && !isFitForSINandISE())
    {
        ReportErr(true, tr("当前项目不能使用此稀释方式！"));
        return false;
    }

    if (!GetSampleTestItemInfo(testItems, profiles))
    {
        return false;
    }

    // 没有请求项目，返回true
    if (m_requestAssays == 0)
    {
        if (m_duliton.first == MANUAL_SIZE || m_duliton.first == USER_SIZE)
        {
            if (m_duliton.second <= 0)
            {
                ReportErr(true, tr("稀释倍数不能为空！"));
                return false;
            }
            // 如果是免疫 	0018196: [工作] 能成功添加稀释因子为1的样本 modify by chenjianlin 20230913
            if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
            {
                // 检测免疫项目稀释倍数设置是否超过要求的范围
                if (!IsImDiluTionPolicy(data))
                {
                    return false;
                }
            }
            else
            {
                // 手工稀释不能是1
                if (m_duliton.first == MANUAL_SIZE && m_duliton.second == 1)
                {
                    ReportErr(true, tr("手工稀释不能是1！"));
                    return false;
                }
                if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
                    && m_duliton.first == USER_SIZE
                    && m_duliton.second < 3)
                {
                    ReportErr(true, tr("自动稀释倍数超出范围！"));
                    return false;
                }
            }
        }
        return true;
    }

    if (m_duliton.first == MANUAL_SIZE || m_duliton.first == USER_SIZE)
    {
        if (m_duliton.second <= 0)
        {
            ReportErr(true, tr("稀释倍数不能为空！"));
            return false;
        }

        if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
            && m_duliton.first == USER_SIZE
            && m_duliton.second < 3)
        {
            ReportErr(true, tr("自动稀释倍数超出范围！"));
            return false;
        }
        // 如果是免疫 	0018196: [工作] 能成功添加稀释因子为1的样本 modify by chenjianlin 20230913
        if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            // 检测免疫项目稀释倍数设置是否超过要求的范围
            if (!IsImDiluTionPolicy(data))
            {
                return false;
            }
        }
    }

    // 手工稀释的情况下（手工稀释的参数不能改变）
    if (m_duliton.first == MANUAL_SIZE && m_storeDilution != 1)
    {
        if (m_duliton.second != m_storeDilution)
        {
            ReportErr(true, tr("同一样本添加手工稀释项目，稀释倍数应保持一致！"));
            return false;
        }
        return true;
    }

    // 自动稀释
    if (m_duliton.first != MANUAL_SIZE && m_storeDilution == 1)
    {
        return true;
    }

    ReportErr(true, tr("同一样本不能同时添加手工稀释与自动稀释项目！"));
    return false;
}

///
/// @brief 检测免疫项目稀释倍数设置是否超过要求的范围
///
/// @param[in]  data  项目选择按钮属性
///
/// @return true：通过， false：不通过
///
/// @par History:
/// @li 1556/Chenjianlin，2023年9月14日，新建函数 
///
bool QAssayTabWidgetContainer::IsImDiluTionPolicy(const AssayButtonData& data)
{
    // 只判断免疫项目
    if (m_currentDevType != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        ULOG(LOG_ERROR, "%s(Is not immune assyaCode : %d)", __FUNCTION__, data.assayCode);
        return true;
    }
    // 如果方式手工稀释
    if (m_duliton.first == MANUAL_SIZE)
    {
        // 手工稀释稀释倍数超出范围 // 0018196: [工作] 能成功添加稀释因子为1的样本 modify by chenjianlin 20230913
        if (m_duliton.second < 2 || m_duliton.second > 640000000)
        {
            ReportErr(true, tr("手工稀释倍数超过允许范围（2-640000000），请重新设置！"));
            return false;
        }
    }
    else if (USER_SIZE == m_duliton.first) // 如果时自动稀释
    {
        // 获取免疫项目参数
        auto spAssay = CommonInformationManager::GetInstance()->GetImmuneAssayInfo(data.assayCode, tf::DeviceType::DEVICE_TYPE_I6000);
        if (nullptr == spAssay)
        {
            ULOG(LOG_ERROR, "%s(can not find assyaCode : %d)", __FUNCTION__, data.assayCode);
            return true;
        }
        // bug修改 0018596: [工作] 稀释倍数可设置超出项目稀释范围的值
        // 不能大于本项目的最大稀释倍数
        if (m_duliton.second > spAssay->reactInfo.maxDilutionRatio)
        {
			QString strMsg = tr("稀释倍数超过允许范围（%1-%2），请重新设置！").arg(spAssay->reactInfo.dilutionRatio).arg(spAssay->reactInfo.maxDilutionRatio);
			ReportErr(true, strMsg);
            return false;
        }
        // 不能小于默认稀释倍数
        if (m_duliton.second < spAssay->reactInfo.dilutionRatio)
        {
			QString strMsg = tr("稀释倍数超过允许范围（%1-%2），请重新设置！").arg(spAssay->reactInfo.dilutionRatio).arg(spAssay->reactInfo.maxDilutionRatio);
            ReportErr(true, strMsg);
            return false;
        }
    }
    return true;
}

///
/// @brief 处理对应的小空间选择卡
///
/// @param[in]  indexPostion  对应位置的按钮被选中
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
void QAssayTabWidgetContainer::OnAssaySelected(int indexPostion)
{
    if (indexPostion < 0)
    {
        return;
    }
    QAssayTabWidgets* assayTable = qobject_cast<QAssayTabWidgets*>(sender());
    if (assayTable == nullptr)
    {
        return;
    }

    ProcSelectButton(assayTable, indexPostion);
}

///
/// @brief 切换页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月18日，新建函数
///
void QAssayTabWidgetContainer::OnChangeTabIndex()
{
    QRadioButton* btn = qobject_cast<QRadioButton*>(sender());
    if (btn == nullptr)
    {
        return;
    }

    for (const auto& page : m_pageKeyinfo)
    {
        if (page.second.devButton == btn)
        {
            ui->stacked_assay_table->setCurrentWidget(page.second.assayTable);

            // 生化设备
            if (page.first == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
            {
                m_currentDevType = tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY;
                emit buttonPress(0);
            }
            // 免疫设备
            else if (page.first == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
            {
                m_currentDevType = tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE;
                emit buttonPress(1);
            }
        }
    }
}

///
/// @bref
///		处理按钮选择动作
///
/// @param[in] assayTable 目标 页
/// @param[in] indexPostion 目标按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年10月30日，新建函数（来自对OnAssaySelected函数的提取
///
void QAssayTabWidgetContainer::ProcSelectButton(QAssayTabWidgets* assayTable, int indexPostion)
{
    if (assayTable == nullptr)
    {
        return;
    }

    // 页面使用类型为类型3
    if (!m_pageKeyinfo.empty() && AssayType::ASSAYTYPE3_POSITION_SET == m_pageKeyinfo.begin()->second.pageType)
    {
        // 若不是-1，则首先清除上一次位置的显示
        if (m_currentIndex != -1)
        {
            AssayButtonData attribute;
            if (assayTable->GetAttributeByIndex(m_currentIndex, attribute))
            {
                attribute.bIsRequest = false;
                assayTable->SetAssayButtonAttribute(m_currentIndex, attribute);
            }
        }

        // 设置本次显示
        AssayButtonData attribute;
        if (assayTable->GetAttributeByIndex(indexPostion, attribute))
        {
            attribute.bIsRequest = true;
            assayTable->SetAssayButtonAttribute(indexPostion, attribute);
            m_currentIndex = indexPostion;
        }

        return;
    }

    AssayButtonData attribute;
    assayTable->GetAttributeByIndex(indexPostion, attribute);

    // 若禁能，直接返回
    if (attribute.bIsForbidden)
    {
        return;
    }

    // 是否符合稀释逻辑
    if (!IsDiluTionPolicy(attribute))
    {
        return;
    }

    // 复合项目特殊处理
    if (!attribute.strSubAssayList.empty())
    {
        if (attribute.bIsRequest)
        {
            attribute.bIsRequest = false;
        }
        else
        {
            attribute.bIsRequest = true;
        }

        // 设置父项目的状态
        assayTable->SetAssayButtonAttribute(indexPostion, attribute);

        // 点击组合项目相当于点击所有的子项目
        for (auto subAssayCode : attribute.strSubAssayList)
        {
            // 获取子项目的位置
            AssayButtonData subAttribute;
            int subIndex = -1;
            if (assayTable->GetAttributeByAssayCode(subAssayCode, subAttribute, subIndex))
            {
                // 父项目选中
                if (attribute.bIsRequest)
                {
                    // 是否符合稀释逻辑
                    if (!IsDiluTionPolicy(subAttribute))
                    {
                        continue;
                    }

                    if (subAttribute.bIsAnalyzed)
                    {
                        // 复查
                        subAttribute.bIsRecheck = true;
                        subAttribute.bIsAnalyzed = false;

                        // 免疫项目的特殊处理
                        if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE && m_duliton.first == STD_SIZE)
                        {
                            m_preDilutionFactor = 1;
                            subAttribute.enSampleSize = USER_SIZE;
                            subAttribute.iDilution = GetDefaultDuliTon(subAssayCode);
                        }
                        else
                        {
                            subAttribute.enSampleSize = m_duliton.first;
                            subAttribute.iDilution = m_duliton.second;
                        }
                    }
                    else if (!subAttribute.bIsRecheck)
                    {
                        // 必须增加一个新增
                        if (subAttribute.guidList.empty())
                        {
                            subAttribute.guidList.push_back(-1);
                        }

                        // 免疫项目的特殊处理
                        if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE && m_duliton.first == STD_SIZE)
                        {
                            m_preDilutionFactor = 1;
                            subAttribute.enSampleSize = USER_SIZE;
                            subAttribute.iDilution = GetDefaultDuliTon(subAssayCode);
                        }
                        else
                        {
                            subAttribute.enSampleSize = m_duliton.first;
                            subAttribute.iDilution = m_duliton.second;
                        }

                        subAttribute.bIsRequest = true;
                        subAttribute.bIsAnalyzed = false;
                        subAttribute.bIsRecheck = false;
                    }
                }
                // 父项目未选中
                else
                {
                    if (subAttribute.bIsRecheck)
                    {
                        subAttribute.bIsAnalyzed = true;
                        subAttribute.bIsRecheck = false;
                    }
                    else if (!subAttribute.bIsAnalyzed)
                    {
                        subAttribute.enSampleSize = SampleSize::STD_SIZE;
                        subAttribute.bIsRequest = false;
                        subAttribute.bIsAnalyzed = false;
                        subAttribute.bIsRecheck = false;
                    }
                }

                assayTable->SetAssayButtonAttribute(subIndex, subAttribute);
            }
        }
    }
    else
    {
        // 请求测试状态->取消测试
        if (attribute.bIsRequest)
        {
            // 取消测试
            attribute.enSampleSize = SampleSize::STD_SIZE;
            attribute.bIsRequest = false;
        }
        // 已完成状态->复查
        else if (attribute.bIsAnalyzed)
        {
            // 复查--已经审核的情况下不允许复查
            if (!m_audit)
            {
                attribute.preiDilution = attribute.iDilution;
                attribute.preSampleSize = attribute.enSampleSize;

                // 免疫项目的特殊处理
                if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE && m_duliton.first == STD_SIZE)
                {
                    m_preDilutionFactor = 1;
                    attribute.enSampleSize = USER_SIZE;
                    attribute.iDilution = GetDefaultDuliTon(attribute.assayCode);
                }
                else
                {
                    attribute.enSampleSize = m_duliton.first;
                    attribute.iDilution = m_duliton.second;
                }
                attribute.bIsRecheck = true;
                attribute.bIsAnalyzed = false;
            }
        }
        // 复查状态->已完成（复查转往已完成的稀释倍数必须从结果信息中去获取）
        else if (attribute.bIsRecheck)
        {
			if (attribute.guidList.empty())
			{
				ULOG(LOG_WARN, "The attribute's testInfo id is empty ,please check it; assayCode is :%d", attribute.assayCode);
			}
			else
			{
				auto testItem = DataPrivate::Instance().GetTestItemInfoByDb(attribute.guidList[0]);
				if (testItem == Q_NULLPTR)
				{
					ULOG(LOG_WARN, "There is no testitem :%d", attribute.guidList[0]);
				}
				else
				{
					tf::TestItem testItemInfo = *testItem;
					DataPrivate::Instance().ResetTestItemDulition(testItemInfo);
					// 转化对应关系
					SetAttributeFromTestitem(attribute, testItemInfo);
					attribute.preSampleSize = attribute.enSampleSize;
					attribute.preiDilution = attribute.iDilution;
				}
			}

            attribute.enSampleSize = attribute.preSampleSize;
            attribute.iDilution = attribute.preiDilution;
            attribute.bIsAnalyzed = true;
            attribute.bIsRecheck = false;
        }
        else//->请求测试
        {
            // 必须增加一个新增
            if (attribute.guidList.empty())
            {
                attribute.guidList.push_back(-1);
            }

            // 免疫项目的特殊处理
            if (m_currentDevType == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE && m_duliton.first == STD_SIZE)
            {
                m_preDilutionFactor = 1;
                attribute.enSampleSize = USER_SIZE;
                attribute.iDilution = GetDefaultDuliTon(attribute.assayCode);
            }
            else
            {
                attribute.enSampleSize = m_duliton.first;
                attribute.iDilution = m_duliton.second;
            }
            attribute.bIsRequest = true;
            attribute.bIsAnalyzed = false;
            attribute.bIsRecheck = false;
        }

        assayTable->SetAssayButtonAttribute(indexPostion, attribute);
    }
}
