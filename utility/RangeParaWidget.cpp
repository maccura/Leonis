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
/// @file     RangeParaWidget.h
/// @brief    项目范围
///
/// @author   5774/WuHongTao
/// @date     2021年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ui_Rangeparamwidget.h"
#include "RangeParaWidget.h"
#include <QStandardItemModel>

#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"

#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "RangParaSetWidget.h"
#include "RecheckConditionsDlg.h"
#include "UtilityCommonFunctions.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

#define DATA_ALARM_GT_TEST				(">Test")		// 浓度高于线性范围上限
#define DATA_ALARM_LT_TEST				("<Test")		// 浓度低于线性范围下限
#define DATA_ALARM_GT_REPT				(">Rept")		// 浓度高于重测范围上限
#define DATA_ALARM_LT_REPT				("<Rept")		// 浓度低于重测范围下限

///
/// @brief
///     构造函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
CRangeParaWidget::CRangeParaWidget(QWidget *parent)
    : QWidget(parent),
    m_bInit(false),
    m_currentRow(0),
    m_RefModel(new QStandardItemModel),
    m_bSampleTypeFlag(true),
    m_RecheckConditionsDlg(nullptr)
{
    ui = new Ui::CRangeParaWidget();
    ui->setupUi(this);

    m_RangeParaSetWidget = new CRangParaSetWidget(RPD_Ch, this);
    // 显示之前初始化
    InitBeforeShow();
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
CRangeParaWidget::~CRangeParaWidget()
{
}

///
/// @brief
///     显示之前初始化
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void CRangeParaWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化参考区间表
    InitRefRangeTable();

    connect(ui->recheck_btn, SIGNAL(clicked()), this, SLOT(OnRecheckConditionBtnClicked()));

    connect(ui->sampleSourceCobox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnSampleSourceChanged(const QString&)));

    SetCtrlsRegExp();

    // 注册新增项目处理函数
    REGISTER_HANDLER(MSG_ID_UTILITY_ADD_ASSAY, this, OnAddAssay);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlsEnabled);

    // 联机版启用复查模块,TODO:暂时屏蔽此代码，待dcs功能开发完成后放开
//     bool isPipe = DictionaryQueryManager::GetInstance()->GetPipeLine();
//     ui->GenderLab_8->setEnabled(isPipe);
//     ui->auto_recheck_commbox->setEnabled(isPipe);
//     ui->GenderLab_10->setEnabled(isPipe);
//     ui->recheck_combox->setEnabled(isPipe);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void CRangeParaWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 参考区间添加按钮被点击
    connect(ui->RefAddBtn, SIGNAL(clicked()), this, SLOT(OnRefAddBtnClicked()));

    // 参考区间修改按钮被点击
    connect(ui->RefModBtn, SIGNAL(clicked()), this, SLOT(OnRefModBtnClicked()));

    // 参考区间删除按钮被点击
    connect(ui->RefDelBtn, SIGNAL(clicked()), this, SLOT(OnRefDelBtnClicked()));

    // 参考区间保存按钮被点击
    connect(m_RangeParaSetWidget->getSaveBtn(), SIGNAL(clicked()), this, SLOT(OnRefSaveBtnClicked()));

    // 选中空行时禁用编辑、删除按钮
    connect(ui->RefRangeTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=]
    {
        UpdateCtrlsEnabled();
    });
}

void CRangeParaWidget::SetCtrlsRegExp()
{
    // 血清信息检查值
    ui->LValue->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->HValue->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
    ui->IValue->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));

    // 自动复查
    ui->lower_range_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));
    ui->high_range_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));
    ui->lower_recheck_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));
    ui->upper_recheck_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_FLOAT_RANG)));
}

bool CRangeParaWidget::CheckUserInputData(int code)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 血清指数不可为0
    OnErr(ui->LValue->text() == "0" || ui->HValue->text() == "0" || ui->HValue->text() == "0", tr("血清指数检查值不能为0"));

    // 检查线性范围
    OnErr((!ui->lower_range_edit->text().isEmpty() && !ui->high_range_edit->text().isEmpty())
          &&(ui->lower_range_edit->text().toDouble() > ui->high_range_edit->text().toDouble()),
            tr("线性范围低值必须小于线性范围最高值"));
    
    // 复查界限
    OnErr((!ui->lower_recheck_edit->text().isEmpty() && !ui->upper_recheck_edit->text().isEmpty())
        && (ui->lower_recheck_edit->text().toDouble() > ui->upper_recheck_edit->text().toDouble()),
        tr("复查界限低值必须小于复查界限高值"));
    
    bool isIse = code >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE && code <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL;

    std::vector<std::string> shieldStatusCodes;
    const auto& CIM = CommonInformationManager::GetInstance();
    if (isIse)
    {
        auto& ptrAI = CIM->GetIseAssayInfo(code, ::tf::DeviceType::DEVICE_TYPE_ISE1005, m_rowItem.version.toStdString());
        if (ptrAI != nullptr)
        {
            shieldStatusCodes.assign(ptrAI->lstShieldStatusCodes.begin(), ptrAI->lstShieldStatusCodes.end());
        }
    }
    else
    {
        const auto& ptrAI = CIM->GetChemistryAssayInfo(code, ::tf::DeviceType::DEVICE_TYPE_C1000, m_rowItem.version.toStdString());
        if (ptrAI != nullptr)
        {
            shieldStatusCodes.assign(ptrAI->lstShieldStatusCodes.begin(), ptrAI->lstShieldStatusCodes.end());
        }
    }

    if (ui->auto_grbox->isChecked())
    {
        // 线性范围上限
        bool rangUp = std::count(shieldStatusCodes.begin(), shieldStatusCodes.end(), DATA_ALARM_GT_TEST) == 0;
        // 线性范围下限
        bool rangLow = std::count(shieldStatusCodes.begin(), shieldStatusCodes.end(), DATA_ALARM_LT_TEST) == 0;
        // 复查范围上限
        bool reCheckUp = std::count(shieldStatusCodes.begin(), shieldStatusCodes.end(), DATA_ALARM_GT_REPT) == 0;
        // 复查范围下限
        bool reCheckLow = std::count(shieldStatusCodes.begin(), shieldStatusCodes.end(), DATA_ALARM_LT_REPT) == 0;

        OnErr(!isIse && rangUp && ui->high_range_edit->text().isEmpty(),
            tr("已在数据报警复查中启用线性范围上限检查，请输入线性范围高值"));
        OnErr(!isIse && rangLow && ui->lower_range_edit->text().isEmpty(),
            tr("已在数据报警复查中启用线性范围下限检查，请输入线性范围低值"));
        OnErr(reCheckUp && ui->upper_recheck_edit->text().isEmpty(),
            tr("已在数据报警复查中启用重测范围上限检查，请输入复查界限高值"));
        OnErr(reCheckLow && ui->lower_recheck_edit->text().isEmpty(),
            tr("已在数据报警复查中启用重测范围下限检查，请输入复查界限低值"));
    }

    // 线性范围包含复查界限
    OnErr(!isIse && !ui->lower_recheck_edit->text().isEmpty() && !ui->lower_range_edit->text().isEmpty()
        && ui->lower_recheck_edit->text().toDouble() < ui->lower_range_edit->text().toDouble(),
        tr("线性范围低值必须小于等于复查界限低值"));

    OnErr(!isIse && !ui->upper_recheck_edit->text().isEmpty() && !ui->high_range_edit->text().isEmpty()
        && ui->upper_recheck_edit->text().toDouble() > ui->high_range_edit->text().toDouble(),
        tr("线性范围高值必须大于等于复查界限高值"));

    return true;
}

///
/// @brief
///     初始化参考区间表
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void CRangeParaWidget::InitRefRangeTable()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设置表头
    QStringList strSelSampHeaderList;
    strSelSampHeaderList << tr("序号") << tr("是否缺省") << tr("样本类型") << tr("低值")
        << tr("高值") << tr("参考值范围") << tr("危急低值") << tr("危急高值") << tr("性别")
        << tr("年龄下限") << tr("年龄上限") << tr("单位");
    m_RefModel->setHorizontalHeaderLabels(strSelSampHeaderList);

    ui->RefRangeTable->setModel(m_RefModel);
	ui->RefRangeTable->setColumnWidth(0, 90);
	ui->RefRangeTable->setColumnWidth(1, 100);
	ui->RefRangeTable->setColumnWidth(2, 179);
	ui->RefRangeTable->setColumnWidth(5, 130);
	ui->RefRangeTable->horizontalHeader()->setMinimumSectionSize(90);
}

void CRangeParaWidget::Reset(bool updateSampleType)
{
    for (auto edit : QWidget::findChildren<QLineEdit*>())
    {
        edit->clear();
    }

    ui->auto_grbox->setChecked(false);
    ui->linear_extension_ckbox->setChecked(false);
    m_RefModel->removeRows(0, m_RefModel->rowCount());

    if (updateSampleType)
    {
       ui->sampleSourceCobox->clear();
    }

    ui->recheck_combox->clear();
    ui->auto_recheck_commbox->clear();
}

void CRangeParaWidget::SetCtrlsVisible(bool isIse)
{
    ui->linear_extension_ckbox->setVisible(!isIse);

    ui->GenderLab_3->setVisible(!isIse);
    ui->lower_range_edit->setVisible(!isIse);
    ui->high_range_edit->setVisible(!isIse);
}

///
/// @brief
///     初始化范围界面数据
///
/// @param[in]  db  生化数据库主键
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月20日，新建函数
///
void CRangeParaWidget::LoadRangeParameter(const AssayListModel::StAssayListRowItem	&item, bool updateSampleType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_bSampleTypeFlag = false;

    Reset(updateSampleType);
    // 缓存当前选中行信息
    m_rowItem = item;
    if (item.assayCode == INVALID_ASSAY_CODE)
    {
        return;
    }

    UpdateCtrlsEnabled();

    // 查询选中项目配置信息
    auto CIM = CommonInformationManager::GetInstance();
    if (CIM == nullptr)
    {
        ULOG(LOG_ERROR, "CommonInformationManager::GetInstance() Failed!");
        m_bSampleTypeFlag = true;
        return;
    }

    if (item.assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE && item.assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        auto spIseAI = CIM->GetIseAssayInfo(item.assayCode, (tf::DeviceType::type)item.model, item.version.toStdString());

        // 判断结果是否为空
        if (spIseAI == nullptr)
        {
            ULOG(LOG_ERROR, "QueryAssayConfigInfo(), the result empty");
            m_bSampleTypeFlag = true;
            return;
        }

        // 查询选中项目特殊配置信息
        auto spIseSAIs = CIM->GetIseSpecialAssayInfo(item.assayCode, (tf::DeviceType::type)item.model);

        // 判断结果是否为空
        if (spIseSAIs.empty())
        {
            ULOG(LOG_ERROR, "QuerySpecialAssayInfo(), the result empty");
            m_bSampleTypeFlag = true;
            return;
        }

        std::shared_ptr<ise::tf::SpecialAssayInfo> spIseSAI;
        for (auto ai : spIseSAIs)
        {
            if (updateSampleType)
            {
               AddTfEnumItemToComBoBox(ui->sampleSourceCobox, (::tf::SampleSourceType::type)ai->sampleSourceType);
            }

            if (ai->sampleSourceType == item.sampleType)
            {
                spIseSAI = ai;
            }
        }

        // 样本类型
        ui->sampleSourceCobox->setCurrentText(ConvertTfEnumToQString((::tf::SampleSourceType::type)item.sampleType));

        if (spIseSAI == nullptr)
        {
            ULOG(LOG_ERROR, "Get iseSpecialAssayInfo Failed!");
            m_bSampleTypeFlag = true;
            return;
        }

        SetCtrlsVisible(true);
        UpdateRecheckComboBox(false);
        LoadDataToCtrls(spIseAI, spIseSAI);
    }
    else
    {
        auto spCAI = CIM->GetChemistryAssayInfo(item.assayCode, (tf::DeviceType::type)item.model, item.version.toStdString());
        // 判断结果是否为空
        if (spCAI == nullptr)
        {
            ULOG(LOG_INFO, "QueryAssayConfigInfo(), the result empty");
            m_bSampleTypeFlag = true;
            return;
        }

        // 查询选中项目特殊配置信息
        auto spSAIs = CIM->GetChemistrySpecialAssayInfo(item.assayCode,
            (tf::DeviceType::type)item.model, -1, item.version.toStdString());
        if (spSAIs.empty())
        {
            ULOG(LOG_ERROR, "QuerySpecialAssayInfo() failed");
            m_bSampleTypeFlag = true;
            return;
        }

        QList<int32_t> sampleSourceList;
        std::shared_ptr<ch::tf::SpecialAssayInfo> spSAI;
        for (auto ai : spSAIs)
        {
            if (updateSampleType)
            {
                sampleSourceList.push_back(ai->sampleSourceType);
            }

            if (ai->sampleSourceType == item.sampleType)
            {
                spSAI = ai;
            }
        }

        // 排序
        std::sort(sampleSourceList.begin(), sampleSourceList.end());
        for (int ss : sampleSourceList)
        {
            if (ss == tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER)
            {
                continue;
            }

            AddTfEnumItemToComBoBox(ui->sampleSourceCobox, (::tf::SampleSourceType::type)ss);
        }

        if (sampleSourceList.contains(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER))
        {
            AddTfEnumItemToComBoBox(ui->sampleSourceCobox, tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);
        }

        // 样本类型
        ui->sampleSourceCobox->setCurrentText(ConvertTfEnumToQString((::tf::SampleSourceType::type)item.sampleType));

        if (spSAI == nullptr)
        {
            ULOG(LOG_ERROR, "Get chSpecialAssayInfo Failed!");
            m_bSampleTypeFlag = true;
            return;
        }

        ui->linear_extension_ckbox->setChecked(spCAI->linearExtension);

        SetCtrlsVisible(false);
        UpdateRecheckComboBox(true);
        LoadDataToCtrls(spCAI, spSAI);
    }

    if (m_RefModel->rowCount() >= m_currentRow)
    {
        ui->RefRangeTable->selectRow(m_currentRow);
    }
    else if (m_RefModel->rowCount() > 0)
    {
        ui->RefRangeTable->selectRow(0);
    }

    m_bSampleTypeFlag = true;
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月20日，新建函数
///
void CRangeParaWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

    bool enable = ui->RefRangeTable->selectionModel()->hasSelection();
    ui->RefDelBtn->setEnabled(enable);
    ui->RefModBtn->setEnabled(enable);
}

bool CRangeParaWidget::GetUnitFactor(int assayCode, double& factor)
{
    auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_INFO, "GetAssayInfo By assayCode[%d] Failed! ", assayCode);
        factor = 1.0;
        return true;
    }

    for (auto unit : spAssayInfo->units)
    {
        if (unit.isCurrent)
        {
            factor = unit.factor;
            return true;
        }
    }

    ULOG(LOG_ERROR, "Not Find Current Unit Factor !");
    return false;
}

::tf::AssayReferenceItem CRangeParaWidget::GetReferrenceByRow(int row)
{
    ::tf::AssayReferenceItem referrence;
    referrence.bGenderRelevent = true;
    referrence.bAgeRelevant = true;

    // 样本源
    referrence.__set_sampleSourceType(ui->sampleSourceCobox->currentData().toInt());

    // 是否缺省
    auto item = m_RefModel->item(row, REF_TABLE_DEFAULT_COLUMN);
    referrence.bAutoDefault = item->text() == tr(STR_AUTO_DEFAULT_Y);

    // 参考区间
    QString tempText = m_RefModel->item(row, REF_TABLE_REF_COLUMN)->text();
    referrence.__set_fLowerRefRang(tempText.isEmpty() ? std::numeric_limits<double>::lowest() : tempText.split("-")[0].toDouble());
    referrence.__set_fUpperRefRang(tempText.isEmpty() ? DBL_MAX : tempText.split("-")[1].toDouble());

    // 危急区间
    tempText = m_RefModel->item(row, REF_TABLE_F_LOW_COLUMN)->text();
    referrence.__set_fLowerCrisisRang(tempText.isEmpty() ? std::numeric_limits<double>::lowest() : tempText.toDouble());
    tempText = m_RefModel->item(row, REF_TABLE_F_UP_COLUMN)->text();
    referrence.__set_fUpperCrisisRang(tempText.isEmpty() ? DBL_MAX : tempText.toDouble());

    // 性别
    item = m_RefModel->item(row, REF_TABLE_GENDER_COLUMN);
    int iGender = ThriftEnumTrans::TransGender(item->text());
    if (iGender != -1)
    {
        referrence.__set_enGender((tf::Gender::type)iGender);
    }

    // 年龄下限
    tempText = m_RefModel->item(row, REF_TABLE_AGE_LOW_COLUMN)->text();
    referrence.__set_iLowerAge(tempText.isEmpty() ? INT_MIN : tempText.toInt());

    // 年龄上限
    tempText = m_RefModel->item(row, REF_TABLE_AGE_UP_COLUMN)->text();
    referrence.__set_iUpperAge(tempText.isEmpty() ? INT_MAX : tempText.toInt());

    // 年龄单位
    item = m_RefModel->item(row, REF_TABLE_AGEUNIT_COLUMN);
    int iAgeUnit = ThriftEnumTrans::TransAgeUnit(item->text());
    if (iAgeUnit != -1)
    {
        referrence.__set_enAgeUnit((tf::AgeUnit::type)iAgeUnit);

    }

    return referrence;
}

void CRangeParaWidget::ShowReferrence(int row, ::tf::AssayReferenceItem item, double factor)
{
    // 将参考区间参数添加到表格中
    // 序号
    AddTextToTableView(m_RefModel, row, REF_TABLE_NUM_COLUMN, QString::number(row + 1));
    // 是否缺省
    AddTextToTableView(m_RefModel, row, REF_TABLE_DEFAULT_COLUMN, 
            item.bAutoDefault ? STR_AUTO_DEFAULT_Y : STR_AUTO_DEFAULT_N);

    // 样本源
    AddTextToTableView(m_RefModel, row, REF_TABLE_SAMPLE_SOURCE, 
            ConvertTfEnumToQString((::tf::SampleSourceType::type)item.sampleSourceType));

    // 低值
    AddTextToTableView(m_RefModel, row, REF_TABLE_LOW_COLUMN, item.fLowerRefRang == std::numeric_limits<double>::lowest() 
            ? "" : UiCommon::DoubleToQString(item.fLowerRefRang * factor));

    // 高值
    AddTextToTableView(m_RefModel, row, REF_TABLE_UP_COLUMN, item.fUpperRefRang == DBL_MAX ? 
        "" : UiCommon::DoubleToQString(item.fUpperRefRang * factor));

    // 参考范围
    QString temp = item.fLowerRefRang == std::numeric_limits<double>::lowest() ? "" : 
        UiCommon::DoubleToQString(item.fLowerRefRang * factor) + "-" +
        UiCommon::DoubleToQString(item.fUpperRefRang * factor);

    AddTextToTableView(m_RefModel, row, REF_TABLE_REF_COLUMN, temp);
    
    // 危急范围
    AddTextToTableView(m_RefModel, row, REF_TABLE_F_LOW_COLUMN, item.fLowerCrisisRang == std::numeric_limits<double>::lowest() 
        ? "" : UiCommon::DoubleToQString(item.fLowerCrisisRang * factor));
    AddTextToTableView(m_RefModel, row, REF_TABLE_F_UP_COLUMN, item.fUpperCrisisRang == DBL_MAX 
        ? "" : UiCommon::DoubleToQString(item.fUpperCrisisRang * factor));

    // 性别
    AddTextToTableView(m_RefModel, row, REF_TABLE_GENDER_COLUMN, ConvertTfEnumToQString(item.enGender));
    // 年龄下限
    AddTextToTableView(m_RefModel, row, REF_TABLE_AGE_LOW_COLUMN, item.iLowerAge == INT_MIN ? "" : QString::number(item.iLowerAge));
    // 年龄上限
    AddTextToTableView(m_RefModel, row, REF_TABLE_AGE_UP_COLUMN, item.iUpperAge == INT_MAX ? "" : QString::number(item.iUpperAge));
    // 年龄单位
    AddTextToTableView(m_RefModel, row, REF_TABLE_AGEUNIT_COLUMN, item.iLowerAge == INT_MIN ? "" : ConvertTfEnumToQString(item.enAgeUnit));
}

bool CRangeParaWidget::GetLinearExtension()
{
    return ui->linear_extension_ckbox->isChecked();
}

bool CRangeParaWidget::HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item)
{
    const auto& CIM = CommonInformationManager::GetInstance();

    // 获取当前单位与主单位的转化倍率
    double factor;
    if (!GetUnitFactor(item.assayCode, factor))
    {
        ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", item.assayCode);
        return true;
    }

    // ise
    if (item.assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE
        && item.assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        auto& spIseAi = CIM->GetIseAssayInfo(item.assayCode, (tf::DeviceType::type)item.model);
        if (nullptr == spIseAi)
        {
            ULOG(LOG_INFO, "Get ise assay info by code[%d] failed", item.assayCode);
            return true;
        }

        // 自动复查
        if (spIseAi->autoRetest != ui->auto_grbox->isChecked())
        {
            ULOG(LOG_INFO, "Please save autoRetest");
            return true;
        }

        // 血清信息检查值
        if (spIseAi->serumIndexCheckValues != GetSerumIndexCheckValue())
        {
            ULOG(LOG_INFO, "Please save serumIndexCheckValues");
            return true;
        }

        auto& spIseSais = CIM->GetIseSpecialAssayInfo(m_rowItem.assayCode, (tf::DeviceType::type)item.model);
        for (const auto& sai : spIseSais)
        {
            // 复查界限
            if (sai->sampleSourceType == item.sampleType
                && spIseSais[0]->repeatLimit != GetTechnicalOrRepeatLimit(true, factor))
            {
                ULOG(LOG_INFO, "Please save autoRetest");
                return true;
            }
        }
    }
    // 比色
    else
    {
        const auto& spChAi = CIM->GetChemistryAssayInfo(m_rowItem.assayCode, (tf::DeviceType::type)item.model);
        if (nullptr == spChAi)
        {
            ULOG(LOG_INFO, "Get ch assay info by code[%d] failed", m_rowItem.assayCode);
            return true;
        }

        // 线性拓展
        if (spChAi->linearExtension != ui->linear_extension_ckbox->isChecked())
        {
            ULOG(LOG_INFO, "Please save linearExtension");
            return true;
        }

        // 自动复查
        if (spChAi->autoRetest != ui->auto_grbox->isChecked())
        {
            ULOG(LOG_INFO, "Please save autoRetest");
            return true;
        }

        // 血清信息检查值
        if (spChAi->serumIndexCheckValues != GetSerumIndexCheckValue())
        {
            ULOG(LOG_INFO, "Please save serumIndexCheckValues");
            return true;
        }

        const auto& spChSais = CIM->GetChemistrySpecialAssayInfo(
            m_rowItem.assayCode, (tf::DeviceType::type)item.model, item.sampleType, item.version.toStdString());
        if (spChSais.empty())
        {
            ULOG(LOG_INFO, "Get ise special assay info by code[%d] failed", m_rowItem.assayCode);
            return true;
        }

        // 复查界限
        if (spChSais[0]->repeatLimit != GetTechnicalOrRepeatLimit(true, factor))
        {
            ULOG(LOG_INFO, "Please save repeatLimit");
            return true;
        }

        // 线性范围
        if (spChSais[0]->technicalLimit != GetTechnicalOrRepeatLimit(false, factor))
        {
            ULOG(LOG_INFO, "Please save technicalLimit");
            return true;
        }
    }

    return false;
}

std::vector<int32_t> CRangeParaWidget::GetSerumIndexCheckValue()
{
    std::vector<int32_t>  serumIndexCheckValues;

    serumIndexCheckValues.push_back(ui->LValue->text().toInt());
    serumIndexCheckValues.push_back(ui->HValue->text().toInt());
    serumIndexCheckValues.push_back(ui->IValue->text().toInt());

    return serumIndexCheckValues;
}

::tf::DoubleRange CRangeParaWidget::GetTechnicalOrRepeatLimit(bool isRepeat, double factor)
{
    ::tf::DoubleRange limit;
    QString temp = isRepeat ? ui->upper_recheck_edit->text(): ui->high_range_edit->text();
    limit.__set_upper(temp.isEmpty() ? DBL_MAX : temp.toDouble() / factor);

    temp = isRepeat ? ui->lower_recheck_edit->text() : ui->lower_range_edit->text();
    limit.__set_lower(temp.isEmpty() ? std::numeric_limits<double>::lowest() : temp.toDouble() / factor);

    return limit;
}

void CRangeParaWidget::SetRerunDispatchModule(QComboBox* box, const ::tf::RerunDispatchModule& rdm)
{
    if (rdm.rtDispatchType == ::tf::RerunDispatchType::RtModuleAppoint)
    {
        const auto& CIM = CommonInformationManager::GetInstance();
        for (const auto& dk : rdm.rtModules)
        {
            QString name = "-" + QString::fromStdString(CIM->GetDeviceName(dk.sn));
            if (dk.modelIndex >= 0)
            {
                name += "-" + ('A' + dk.modelIndex - 1);
            }

            box->setCurrentText(ConvertTfEnumToQString(rdm.rtDispatchType) + name);
        }
    }
    else
    {
        box->setCurrentText(ConvertTfEnumToQString(rdm.rtDispatchType));
    }
}

::tf::RerunDispatchModule CRangeParaWidget::GetRerunDispatchModule(QComboBox* box)
{
    ::tf::RerunDispatchModule rdm;
    QString text = box->currentText();
    if (text.contains(ConvertTfEnumToQString(::tf::RerunDispatchType::RtModuleAppoint)))
    {
        rdm.rtDispatchType = ::tf::RerunDispatchType::RtModuleAppoint;
        const auto& CIM = CommonInformationManager::GetInstance();
        auto tempList = text.split("-");

        ::tf::DevicekeyInfo di;
        di.sn = box->currentData().toString().toStdString();

        // ISE处理模块信息
        if (tempList.size() == 3)
        {
            // ISE-A表示模块1，ISE-B表示模块2
            di.modelIndex = tempList[2] == 'A' ? 1 : 2;
        } 

        rdm.__set_rtModules({ di });
    }
    else
    {
        rdm.__set_rtDispatchType((::tf::RerunDispatchType::type)box->currentData().toInt());
    }

    return rdm;
}

void CRangeParaWidget::UpdateRecheckComboBox(bool isChemistry)
{
    for (auto each : ::tf::_RerunDispatchType_VALUES_TO_NAMES)
    {
        auto type = (::tf::RerunDispatchType::type)each.first;
        if (type == ::tf::RerunDispatchType::RtModuleAppoint)
        {
            continue;
        }

        AddTfEnumItemToComBoBox(ui->recheck_combox, type);
        AddTfEnumItemToComBoBox(ui->auto_recheck_commbox, type);
    }

    QString text = ConvertTfEnumToQString(::tf::RerunDispatchType::RtModuleAppoint);
    const auto& CIM = CommonInformationManager::GetInstance();
    if (isChemistry)
    {
        for (const auto& dev : CIM->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000 }))
        {
            QString tempText = text + "-" + QString::fromStdString(dev->groupName + dev->name);
            ui->recheck_combox->addItem(tempText, QString::fromStdString(dev->deviceSN));
            ui->auto_recheck_commbox->addItem(tempText, QString::fromStdString(dev->deviceSN));
        }
    }
    else
    {
        for (const auto& dev : CIM->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_ISE1005 }))
        {
            QString tempText = text + "-" + QString::fromStdString(dev->groupName + dev->name);
            if (dev->moduleCount == 1)
            {
                ui->recheck_combox->addItem(tempText, QString::fromStdString(dev->deviceSN));
                ui->auto_recheck_commbox->addItem(tempText, QString::fromStdString(dev->deviceSN));
            } 
            else
            {
                tempText += "-";
                for (int i=0; i<2; i++)
                {
                    ui->recheck_combox->addItem(tempText + ('A' + i), QString::fromStdString(dev->deviceSN));
                    ui->auto_recheck_commbox->addItem(tempText + ('A' + i), QString::fromStdString(dev->deviceSN));
                }
            }
        }
    }
}

///
/// @brief
///     参考区间添加按钮被点击
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void CRangeParaWidget::OnRefAddBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_RangeParaSetWidget->ShowDlg(Et_Add);
    m_RangeParaSetWidget->SetSampleTypeCommbox(ui->sampleSourceCobox->currentData().toInt());
}

///
/// @brief
///     参考区间修改按钮被点击
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void CRangeParaWidget::OnRefModBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_RangeParaSetWidget->ShowDlg(Et_Modify);
    int optRow = ui->RefRangeTable->currentIndex().row();
    m_RangeParaSetWidget->loadRangParaSetWidget(GetReferrenceByRow(optRow));
    m_RangeParaSetWidget->SetSampleTypeCommbox(ui->sampleSourceCobox->currentData().toInt());
}

///
/// @brief
///     参考区间删除按钮被点击
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void CRangeParaWidget::OnRefDelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (TipDlg(tr("删除"), tr("即将删除选中的参考区间"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    // 刷新行号并移除当前行
    auto rowIndexs = ui->RefRangeTable->selectionModel()->selectedRows();
    QStringList rowList;
    for (auto& index : rowIndexs)
    {
        rowList.push_back(QString::number(index.row() + 1));
    }

    // 按序号删除选中行
    for (QString row : rowList)
    {
        for (int i = 0; i < m_RefModel->rowCount(); i++)
        {
            auto rowSeqItem = m_RefModel->item(i, REF_TABLE_NUM_COLUMN);
            if (rowSeqItem ==nullptr)
            {
                continue;
            }

            if (rowSeqItem->text() == row)
            {
                m_RefModel->removeRow(i);
                continue;
            }
        }
    }

    // 保存参考区间配置信
    SaveAssayReferrence();
}

///
/// @brief
///     新增/编辑弹窗保存按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 7951/LuoXin，2022年6月14日，新建函数
///
void CRangeParaWidget::OnRefSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (CommonInformationManager::GetInstance()->IsExistDeviceRuning())
    {
        TipDlg(tr("提示"), tr("项目测试中不可保存")).exec();
        return;
    }

    tf::AssayReferenceItem setRangData;
    if (!m_RangeParaSetWidget->getSetParaData(setRangData))
    {
        ULOG(LOG_ERROR, "Failed to get setparadata.");
        return;
    }

    // 检查参考区间
    int row = 0;
    ::tf::AssayReference referrence;

    while (row < m_RefModel->rowCount())
    {
        auto item = GetReferrenceByRow(row);
        referrence.Items.push_back(std::move(item));
        row++;
    }

    m_currentRow = m_RefModel->rowCount();

    // 编辑时，先移除编辑前的行数据
    if (m_RangeParaSetWidget->GetEditType() == Et_Modify)
    {
        m_currentRow = ui->RefRangeTable->currentIndex().row();
        referrence.Items.erase(referrence.Items.begin() + m_currentRow);
    }

    referrence.Items.push_back(setRangData);
    if (!UiCommon::CheckAssayReferenceIsValid(referrence))
    {
        return;
    }

    // 新输入的参考范围单位倍率为1
    ShowReferrence(m_currentRow, setRangData, 1);

    // 保存参考区间配置信
    SaveAssayReferrence();

    // 启用修改、删除按钮
    ui->RefRangeTable->selectRow(m_currentRow);
    ui->RefModBtn->setEnabled(true);
    ui->RefDelBtn->setEnabled(true);

    m_RangeParaSetWidget->close();
}

void CRangeParaWidget::OnRecheckConditionBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_RecheckConditionsDlg == nullptr)
    {
        m_RecheckConditionsDlg = new RecheckConditionsDlg(this);
        connect(m_RecheckConditionsDlg, &RecheckConditionsDlg::sigSaveBtnClicked, this, &CRangeParaWidget::OnSaveRecheckConditions);
    }

    const auto& CIM = CommonInformationManager::GetInstance();

    if (m_rowItem.assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE && m_rowItem.assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        // 查询ise通用项目信息
        auto spGai = CIM->GetIseAssayInfo(m_rowItem.assayCode, tf::DeviceType::DEVICE_TYPE_ISE1005, m_rowItem.version.toStdString());
        if (spGai == nullptr)
        {
            ULOG(LOG_ERROR, "GetIseSpecialAssayInfo() failed!");
            return;
        }

        m_RecheckConditionsDlg->show();
        m_RecheckConditionsDlg->UpdateSelectRows(spGai->lstShieldStatusCodes, false);
    }
    else
    {
        // 查询ch特殊项目信息
        auto spGai = CIM->GetChemistryAssayInfo(m_rowItem.assayCode, tf::DeviceType::DEVICE_TYPE_C1000, m_rowItem.version.toStdString());
        if (nullptr == spGai)
        {
            ULOG(LOG_ERROR, "GetIseSpecialAssayInfo() failed!");
            return;
        }

        m_RecheckConditionsDlg->show();
        m_RecheckConditionsDlg->UpdateSelectRows(spGai->lstShieldStatusCodes, true);
    }
}

void CRangeParaWidget::OnSaveRecheckConditions(std::vector<std::string> shieldStatusCodes)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!CommonInformationManager::GetInstance()->ModifyAssayShieldStatusCodes(
        m_rowItem.assayCode, m_rowItem.version.toStdString(), shieldStatusCodes))
    {
        ULOG(LOG_ERROR, "ModifySpecialAssayInfo() failed!");
        TipDlg(tr("保存自动复查条件失败!")).exec();
    }
}

void CRangeParaWidget::OnSampleSourceChanged(const QString &text)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    if (m_bSampleTypeFlag)
    {
        POST_MESSAGE(MSG_ID_ASSAY_SAMPLE_UPDATE, ui->sampleSourceCobox->currentData().toInt());
    }
}

bool CRangeParaWidget::SaveAssayReferrence()
{
    // 获取当前单位与主单位的转化倍率
    double factor;
    if (!GetUnitFactor(m_rowItem.assayCode, factor))
    {
        ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", m_rowItem.assayCode);
        return false;
    }

    // 参考区间
    int row = 0;
    ::tf::AssayReference referrence;
    while (row < m_RefModel->rowCount())
    {
        auto item = GetReferrenceByRow(row);

        // 单位倍率转换
        if (std::numeric_limits<double>::lowest() != item.fLowerRefRang)
        {
            item.__set_fLowerRefRang(item.fLowerRefRang / factor);
        }

        if (DBL_MAX != item.fUpperRefRang)
        {
            item.__set_fUpperRefRang(item.fUpperRefRang / factor);
        }

        if (std::numeric_limits<double>::lowest() != item.fLowerCrisisRang)
        {
            item.__set_fLowerCrisisRang(item.fLowerCrisisRang / factor);
        }

        if (DBL_MAX != item.fUpperCrisisRang)
        {
            item.__set_fUpperCrisisRang(item.fUpperCrisisRang / factor);
        }

        referrence.Items.push_back(item);
        row++;
    }

    const auto& CIM = CommonInformationManager::GetInstance();
    if (m_rowItem.assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE 
        && m_rowItem.assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        // ISE
        ::ise::tf::SpecialAssayInfo sai;
        auto& spIseSai = CIM->GetIseSpecialAssayInfo(m_rowItem.assayCode,
            (tf::SampleSourceType::type)m_rowItem.sampleType, (tf::DeviceType::type)m_rowItem.model);
        OnErr(spIseSai == nullptr, tr("保存参考区间失败"));

        referrence.bUsing = true;
        sai.__set_referenceRanges(referrence);
        sai.__set_id(spIseSai->id);
        OnErr(!ise::LogicControlProxy::ModifySpecialAssayInfo(sai), tr("保存参考区间失败"));
    }
    else
    {
        // 比色
        ::ch::tf::SpecialAssayInfo sai;
        const auto& spSais = CIM->GetChemistrySpecialAssayInfo(m_rowItem.assayCode,
            (tf::DeviceType::type)m_rowItem.model, m_rowItem.sampleType, m_rowItem.version.toStdString());
        OnErr(spSais.empty(), tr("保存参考区间失败"));

        referrence.bUsing = true;
        sai.__set_referenceRanges(referrence);
        sai.__set_id(spSais[0]->id);
        OnErr(ch::LogicControlProxy::ModifySpecialAssayInfo(sai) 
                != tf::ThriftResult::THRIFT_RESULT_SUCCESS, tr("保存参考区间失败"));
    }

    // 通知UI信息管理器更新
    return CIM->updateReferenceRanges(m_rowItem.assayCode, m_rowItem.sampleType, m_rowItem.version.toStdString(), referrence);
}

void CRangeParaWidget::OnAddAssay(QString name, QString unit, int code, QString version, QSet<int> sampleSources)
{
    m_bSampleTypeFlag = false;

    const auto& CIM = CommonInformationManager::GetInstance();

    // 新增项目
    if (CIM->GetAssayInfo(code) == nullptr)
    {
        Reset();
    }

    // 设置样本类型下拉框
    ui->sampleSourceCobox->clear();

    // 排序
    auto sampleSourceList = sampleSources.toList();
    std::sort(sampleSourceList.begin(), sampleSourceList.end());

    for (int ss : sampleSourceList)
    {
        if (ss == tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER)
        {
            continue;
        }

        AddTfEnumItemToComBoBox(ui->sampleSourceCobox, (tf::SampleSourceType::type)ss);
    }

    if (sampleSources.contains(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER))
    {
        AddTfEnumItemToComBoBox(ui->sampleSourceCobox, tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);
    }

    ui->sampleSourceCobox->setCurrentIndex(0);

    m_bSampleTypeFlag = true;
}

void CRangeParaWidget::UpdateCtrlsEnabled()
{
    const auto& spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(m_rowItem.assayCode);
    bool isOpenAssay = true;
    if (spAssay != nullptr)
    {
        isOpenAssay = spAssay->openAssay;
    }

    bool hasCanDel = ui->RefRangeTable->selectionModel()->hasSelection();
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 线性拓展
    ui->linear_extension_ckbox->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_LINEAR_EXTENSION));
    // 自动复查
    ui->auto_grbox->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_IM_ASSAYSET_AUTO_RERUNSET));
    // 血清指数检查
    ui->sind_grbox->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SIND_CHECK));
    // 新增参考范围
    ui->RefAddBtn->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_IM_ASSAYSET_ADD_REF_RANGE));
    // 修改参考范围
    ui->RefModBtn->setEnabled(hasCanDel && (isOpenAssay || userPms->IsPermisson(PSM_IM_ASSAYSET_MODLFY_REF_RANGE)));
    // 删除参考范围
    ui->RefDelBtn->setEnabled(hasCanDel && (isOpenAssay || userPms->IsPermisson(PSM_IM_ASSAYSET_DEL_REF_RANGE)));
}
