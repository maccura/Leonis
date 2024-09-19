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
/// @file     ImAnalysisparamwidget.cpp
/// @brief    项目设置-分析
///
/// @author   1226/ZhangJing
/// @date     2023年2月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年2月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ImAnalysisparamwidget.h"
#include <float.h>
#include "ui_ImAnalysisparamwidget.h"
#include "thrift/DcsControlProxy.h"

#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"

#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "manager/UserInfoManager.h"

#define REF_TABLE_DEFAULT_ROW_CNT (5)	 // 参考区间表默认行数
#define REF_TABLE_DEFAULT_COLUMN_CNT (5) // 参考区间表默认列数

#define MAX_ASSAY_NAME_LENGTH 32

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

///
/// @brief 范围判断
///
#define OutOfRange(dValue) ((dValue)<-99999 || (dValue)>999999)
#define OutOfRangePositive(dValue) ((dValue)<0 || (dValue)>999999)


ImAnalysisParamWidget::ImAnalysisParamWidget(QWidget *parent)
    : QWidget(parent),
    m_bInit(false),
    m_bSampleTypeFlag(false),
    m_bInitVerCom(false)
{
    ui = new Ui::ImAnalysisParamWidget();
    ui->setupUi(this);

    InitCtrErrorDescription();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

ImAnalysisParamWidget::~ImAnalysisParamWidget()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
void ImAnalysisParamWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
void ImAnalysisParamWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设置只读控件
    auto readOnlyCtrls = { ui->fullNameEdit, ui->AssayCodeEdit, ui->DefaultDiluEdit };
    for (auto ctrl : readOnlyCtrls)
    {
        ctrl->setReadOnly(true);
    }

    // 初始化保留小数位数下拉框
    for (auto i : { 0, 1, 2, 3, 4 })
    {
        ui->DecimalCombo->addItem(QString::number(i), i);
    }

    // 初始化符号下拉
    QStringList opList = { "<", ">", "<=", ">=" };
    ui->PosOperatorCombo->addItems(opList);
    ui->PosOperatorCombo->setCurrentIndex(0);
    ui->NegOperatorCombo->addItems(opList);
    ui->NegOperatorCombo->setCurrentIndex(0);

    // 输入框限制
    SetCtrlsRegExp();

    // 版本切换
    connect(ui->AssayVerCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnAssayVerChanged(const QString&)));
    // 单位切换
    connect(ui->ResUnitCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnUnitChanged(const QString&)));
    // 精度切换
    connect(ui->DecimalCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnDecimalChanged(const QString&)));
}

///
/// @brief  输入框样式设置
///
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
void ImAnalysisParamWidget::SetCtrlsRegExp()
{
    for (auto edit : findChildren<QLineEdit*>())
    {
        // 限制输入正浮点数
        if (edit->objectName().startsWith("TechRange") || edit->objectName().startsWith("AutoRerun")
            || edit->objectName().startsWith("Pos") || edit->objectName().startsWith("Neg") || edit->objectName().startsWith("Remaind")
            || edit->objectName().startsWith("AlarmRange"))
        {
            edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_FLOAT)));
        }

        // 限制输入正整数
        if (edit->objectName().startsWith("DefaultDiluEdit"))
        {
            edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
        }
    }
}

///
/// @brief 窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
void ImAnalysisParamWidget::showEvent(QShowEvent *event)
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
}

///
/// @brief
///     复位控件
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
void ImAnalysisParamWidget::Reset()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    for (auto edit : QWidget::findChildren<QLineEdit*>())
    {
        edit->clear();
    }

    for (auto box : QWidget::findChildren<QComboBox*>())
    {
        if (box->objectName().contains("AutoRerun"))
        {
            box->setCurrentIndex(0);
        }
        else
        {
            box->setCurrentIndex(-1);
        }
    }
}

int ImAnalysisParamWidget::GetPrecion(int iAssayCode)
{
    std::shared_ptr<tf::GeneralAssayInfo> pAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(iAssayCode);
    if (pAssayInfo == nullptr)
    {
        return 4; // default precion.
    }

    return pAssayInfo->decimalPlace;
}

///
/// @brief  当前版本切换
///
/// @param[in]  text  新版本号
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
void ImAnalysisParamWidget::OnAssayVerChanged(const QString &text)
{
    // 正在初始化过程中不处理 bug22524
    if (!m_bInitVerCom)
    {
        ULOG(LOG_WARN, "%s, Skip for in init ver comb.", __FUNCTION__);
        return;
    }
    std::shared_ptr<CommonInformationManager> commMgr = CommonInformationManager::GetInstance();

    // 项目精度
    int iPrecision = GetPrecion(m_rowItem.assayCode);

    // 查询免疫通用项目信息
    auto imGAI = commMgr->GetImmuneAssayInfo(m_rowItem.assayCode, (::tf::DeviceType::type)m_rowItem.model, text.toStdString());
    if (imGAI == nullptr)
    {
        ULOG(LOG_ERROR, "GetImmuneAssayInfo failed. assayCode=%d", m_rowItem.assayCode);
        return;
    }
    m_funSetVersion(text);

    // 仅更新版本不同内容
    // 获取当前单位与主单位的转化倍率
    double factor = 1;
    if (!commMgr->GetUnitFactor(m_rowItem.assayCode, factor))
    {
        ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", m_rowItem.assayCode);
        return;
    }
    // 默认稀释倍率
    int32_t defaultDilu = imGAI->reactInfo.dilutionRatio;

    // 定性项目要显示默认稀释倍数但不反乘
    ui->DefaultDiluEdit->setText(QString::number(defaultDilu));
    if (imGAI->caliType == 0)
    {
        defaultDilu = 1;
    }

    // 报告范围
    ui->RepRangeLowerEdit->setText(QString::number(imGAI->techRangeMin * factor * defaultDilu, 'f', iPrecision));
    ui->RepRangeUpperEdit->setText(QString::number(imGAI->techRangeMax * factor * defaultDilu, 'f', iPrecision));
    // 开瓶有效期
    ui->OpenDaysEdit->setText(QString::number(imGAI->daysOnBoard));
	ui->BotCaliWeekEdit->setText(imGAI->kitCaliPeriod > 0 ? QString::number(imGAI->kitCaliPeriod) : "");
	ui->LotCaliWeekEdit->setText(imGAI->lotCaliPeriod > 0 ? QString::number(imGAI->lotCaliPeriod) : "");
}

ImAnalysisParamWidget::UiCtrlValidError ImAnalysisParamWidget::CheckCtrlValue(int assayCode, ::tf::DeviceType::type devType, const std::string& ver)
{
    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
    double factor = 0;
    if (!assayMgr->GetUnitFactor(assayCode, factor))
    {
        return UV_ERROR_NO_UNITFACTOR;
    }
    auto imAssay = assayMgr->GetImmuneAssayInfo(assayCode, devType, ver);
    if (imAssay == nullptr)
    {
        return UV_ERROR_NO_IMGENERALASSAY;
    }

    // 基本参数
    UiCtrlValidError errStat = CheckCtrlValueBaseParameter(assayCode, factor);
    if (errStat != UV_NO_ERROR)
    {
        return errStat;
    }

    // 阴阳性判断
    UiCtrlValidError errStatPosNeg = CheckCtrlValuePosNeg(factor);
    if (errStatPosNeg != UV_NO_ERROR)
    {
        return errStatPosNeg;
    }  
    
    // 默认稀释倍数不能大于最大稀释倍数
    int defaultDilu = ui->DefaultDiluEdit->text().toInt();
    if (defaultDilu > imAssay->reactInfo.maxDilutionRatio)
    {
        return UV_DILU_DEFAULT_OVER_MAX;
    }

    return UV_NO_ERROR;
}

ImAnalysisParamWidget::UiCtrlValidError ImAnalysisParamWidget::CheckCtrlValueBaseParameter(int assayCode, double factor)
{
    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();

    // 检测英文简称
    std::string strEnAbreviation = ui->AssayNameEdit->text().toUtf8().data();
    if (strEnAbreviation.empty())
    {
        return UV_BASE_BRIF_NAME_EMPTY;
    }
    if (strEnAbreviation.length() > MAX_ASSAY_NAME_LENGTH)
    {
        return UV_BASE_BRIF_NAME_TOOLENGTH;
    }
    if (assayMgr->IsExistGeneralAssayName(assayCode, strEnAbreviation))
    {
        return UV_BASE_BRIF_NAME_EXIST;
    }

    // 小数位数
    bool bOk = false;
    int iDecimal = ui->DecimalCombo->currentText().toInt(&bOk);
    if (!bOk || ui->DecimalCombo->currentIndex() == -1)
    {
        return UV_BASE_DECIMAL_VALUE_ERROR;
    }

    double alarmLower = -1;
    double alarmUpper = -1;
    // 线性范围的低值
    if (!ui->AlarmRangeLowerEdit->text().isEmpty())
    {
        alarmLower = ui->AlarmRangeLowerEdit->text().toDouble(&bOk);
        if (!bOk || OutOfRange(alarmLower))
        {
            return UV_BASE_VALUE_LINE_LOWER;
        }
        alarmLower /= factor;
    }
    // 线性范围的高值
    if (!ui->AlarmRangeUpperEdit->text().isEmpty())
    {
        alarmUpper = ui->AlarmRangeUpperEdit->text().toDouble(&bOk);
        if (!bOk || OutOfRange(alarmUpper))
        {
            return UV_BASE_VALUE_LINE_UPPER;
        }
        alarmUpper /= factor;
    }
    if (alarmLower > alarmUpper)
    {
        return UV_BASE_LINE_RANGE;
    }

    return UV_NO_ERROR;
}

ImAnalysisParamWidget::UiCtrlValidError ImAnalysisParamWidget::CheckCtrlValuePosNeg(double factor)
{
    bool bOk = false;

    // 定量项目阴阳性阈值
    if (ui->PosNegGrpBox->isChecked())
    {
        // 阳值符号
        int operType = ui->PosOperatorCombo->currentIndex();
        if (operType == -1 || (im::tf::operatorType::type)operType == im::tf::operatorType::OPERATOR_OTHER)
        {
            return UV_PN_SYMBO_POS;
        }

        double posNum = ui->PosNumEdit->text().toDouble(&bOk);
        if (!bOk || OutOfRange(posNum))
        {
            return UV_PN_VALUE_POS;
        }

        int operTypeNeg = (im::tf::operatorType::type)ui->NegOperatorCombo->currentIndex();
        if (operTypeNeg == -1 || (im::tf::operatorType::type)operTypeNeg == im::tf::operatorType::OPERATOR_OTHER)
        {
            return UV_PN_SYMBO_NEG;
        }

        double negNum = ui->NegNumEdit->text().toDouble(&bOk);
        if (!bOk || OutOfRange(negNum))
        {
            return UV_PN_VALUE_NEG;
        }

        posNum /= factor;
        negNum /= factor;
        if (operType == im::tf::operatorType::OPERATOR_LESS || operType == im::tf::operatorType::OPERATOR_LESS_EQUAL)
        {
            if ((operTypeNeg == im::tf::operatorType::OPERATOR_LESS || operTypeNeg == im::tf::operatorType::OPERATOR_LESS_EQUAL) || \
                (negNum < posNum) || \
                (negNum == posNum && operType == im::tf::operatorType::OPERATOR_LESS_EQUAL && operTypeNeg == im::tf::operatorType::OPERATOR_MORE_EQUAL))
            {
                return UV_PN_REPEAT_RANGE;
            }
        }
        else if (operType == im::tf::operatorType::OPERATOR_MORE || operType == im::tf::operatorType::OPERATOR_MORE_EQUAL)
        {
            if ((operTypeNeg == im::tf::operatorType::OPERATOR_MORE || operTypeNeg == im::tf::operatorType::OPERATOR_MORE_EQUAL) || \
                (negNum > posNum) || \
                (negNum == posNum && operType == im::tf::operatorType::OPERATOR_MORE_EQUAL && operTypeNeg == im::tf::operatorType::OPERATOR_LESS_EQUAL))
            {
                return UV_PN_REPEAT_RANGE;
            }
        }
    }

    return UV_NO_ERROR;
}

ImAnalysisParamWidget::UiCtrlValidError ImAnalysisParamWidget::CheckCtrlValueUnitChanged(\
    const std::shared_ptr<im::tf::GeneralAssayInfo>& pImAssayInfo, double factor)
{
    if (pImAssayInfo == nullptr)
    {
        ULOG(LOG_WARN, "Not checked.");
        return UV_NO_ERROR;
    }
    // 线性范围
    if (OutOfRange(pImAssayInfo->alarmRangeMin * factor))
    {
        return UV_BASE_VALUE_LINE_LOWER;
    }
    if (OutOfRange(pImAssayInfo->alarmRangeMax * factor))
    {
        return UV_BASE_VALUE_LINE_UPPER;
    }

    // 定量项目阴阳性阈值
    if (OutOfRange(pImAssayInfo->threshold.posCompare.num * factor))
    {
        return UV_PN_VALUE_POS;
    }
    if (OutOfRange(pImAssayInfo->threshold.negCompare.num * factor))
    {
        return UV_PN_VALUE_NEG;
    }

    // 自定义提醒设置
    std::vector<im::tf::remaindSet> &vecRemainder = pImAssayInfo->reminder;
    if (vecRemainder.size() > 0 && \
        vecRemainder[0].remaindCompare.num >= 0 && \
        OutOfRange(vecRemainder[0].remaindCompare.num * factor))
    {
        return UV_CUSTOM_RANGE1;
    }
    if (vecRemainder.size() > 1 && \
        vecRemainder[1].remaindCompare.num >= 0 && \
        OutOfRange(vecRemainder[1].remaindCompare.num * factor))
    {
        return UV_CUSTOM_RANGE2;
    }

    if (vecRemainder.size() == 4) // 第3组是区间,即第3,4组成的一个区间
    {
        if (OutOfRange(vecRemainder[2].remaindCompare.num * factor) || \
            OutOfRange(vecRemainder[3].remaindCompare.num * factor))
        {
            return UV_CUSTOM_RANGE3;
        }
    }

    // 自动复查范围检查
    int valueIdx = 0;
    for (const im::tf::autoRerunByRange& rItem : pImAssayInfo->rerunSet.ranges)
    {
        if (rItem.range.lower != std::numeric_limits<double>::lowest() && OutOfRangePositive(rItem.range.lower * factor))
        {
            return valueIdx == 0 ? UV_AUTORETEST_VALUE_LOWER : UV_AUTORETEST_VALUE2_LOWER;
        }
        if (rItem.range.upper != DBL_MAX && OutOfRangePositive(rItem.range.upper * factor))
        {
            return valueIdx == 0 ? UV_AUTORETEST_VALUE_UPPER : UV_AUTORETEST_VALUE2_UPPER;
        }
        valueIdx++;
    }

    return UV_NO_ERROR;
}

void ImAnalysisParamWidget::InitCtrErrorDescription()
{
    m_ctrlErrorDescription.clear();

    m_ctrlErrorDescription[UV_BASE_BRIF_NAME_EMPTY] = tr("简称不能为空。");
    m_ctrlErrorDescription[UV_BASE_BRIF_NAME_TOOLENGTH] = tr("简称过长。");
    m_ctrlErrorDescription[UV_BASE_BRIF_NAME_EXIST] = tr("简称已经存在。");
    m_ctrlErrorDescription[UV_BASE_DECIMAL_VALUE_ERROR] = tr("值错误：小数位。");
    m_ctrlErrorDescription[UV_BASE_VALUE_LINE_LOWER] = tr("值错误：线性范围设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_BASE_VALUE_LINE_UPPER] = tr("值错误：线性范围设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_BASE_LINE_RANGE] = tr("范围错误：线性范围低值不能大于高值。");

    m_ctrlErrorDescription[UV_PN_SYMBO_POS] = tr("值错误：阳值符号设置。");
    m_ctrlErrorDescription[UV_PN_VALUE_POS] = tr("值错误：阳值设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_PN_SYMBO_NEG] = tr("值错误：阴值符号设置。");
    m_ctrlErrorDescription[UV_PN_VALUE_NEG] = tr("值错误：阴值设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_PN_REPEAT_RANGE] = tr("值错误：阳值和阴值存在重复区间。");

    m_ctrlErrorDescription[UV_CUSTOM_SYMBO1] = tr("值错误：自定义提醒1符号设置。");
    m_ctrlErrorDescription[UV_CUSTOM_RANGE1] = tr("值错误：自定义提醒1值设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_CUSTOM_SYMBO2] = tr("值错误：自定义提醒2符号设置。");
    m_ctrlErrorDescription[UV_CUSTOM_RANGE2] = tr("值错误：自定义提醒2值设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_CUSTOM_RANGE3] = tr("值错误：自定义提醒3值设置范围为[0,999999]。");

    m_ctrlErrorDescription[UV_AUTORETEST_VALUE_LOWER] = tr("值错误：自动复查范围1设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_AUTORETEST_VALUE_UPPER] = tr("值错误：自动复查范围1设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_AUTORETEST_VALUE2_LOWER] = tr("值错误：自动复查范围2设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_AUTORETEST_VALUE2_UPPER] = tr("值错误：自动复查范围2设置范围为[0,999999]。");


    m_ctrlErrorDescription[UV_DILU_DEFAULT_OVER_MAX] = tr("值错误：默认稀释倍数不能大于项目的最大稀释倍数。");
}

std::shared_ptr<im::tf::GeneralAssayInfo> ImAnalysisParamWidget::GetImAssayInfoAndInitVerComb(\
    int iAssayCode, ::tf::DeviceType::type devType, const QString& strVer)
{
    ULOG(LOG_INFO, "%s(%d, %d)", __FUNCTION__, iAssayCode, devType);

    // 查询免疫通用项目信息
    std::shared_ptr<CommonInformationManager> commMgr = CommonInformationManager::GetInstance();

    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> imGAIs;
    commMgr->GetImmuneAssayInfos(imGAIs, iAssayCode, devType);
    if (imGAIs.size() < 1)
    {
        ULOG(LOG_ERROR, "Failed to get immune assayinfos!");
        return nullptr;
    }
    m_bInitVerCom = false;
    ui->AssayVerCombo->clear();
    for (std::shared_ptr<im::tf::GeneralAssayInfo>& pt : imGAIs)
    {
        if (pt == nullptr)
        {
            continue;
        }
        QString strCurParamVersion = QString::fromStdString(pt->version);
        if (ui->AssayVerCombo->findText(strCurParamVersion) < 0)
        {
            ui->AssayVerCombo->addItem(strCurParamVersion);
        }
    }

    std::shared_ptr<im::tf::GeneralAssayInfo> imGAI = commMgr->GetImmuneAssayInfo(iAssayCode, devType, strVer.toStdString());
    if (imGAI != nullptr)
    {
        ui->AssayVerCombo->setCurrentText(QString::fromStdString(imGAI->version));
    }
    m_bInitVerCom = true; 

    return imGAI;
}

void ImAnalysisParamWidget::AssignFactorAboutEdit(const std::shared_ptr<im::tf::GeneralAssayInfo>& pImAssayInfo, double factor, int iPrecision)
{
    if (pImAssayInfo == nullptr)
    {
        return;
    }

    // 报告范围
    int32_t defaultDilu = pImAssayInfo->reactInfo.dilutionRatio;
    if (pImAssayInfo->caliType == 0)
    {
        defaultDilu = 1;
    }
    ui->RepRangeLowerEdit->setText(QString::number(pImAssayInfo->techRangeMin * factor * defaultDilu, 'f', iPrecision));
    ui->RepRangeUpperEdit->setText(QString::number(pImAssayInfo->techRangeMax * factor * defaultDilu, 'f', iPrecision));

    // 线性范围 (-1显示为空)
    double aMin = pImAssayInfo->alarmRangeMin * factor;
    ui->AlarmRangeLowerEdit->setText(aMin < 0 ? "" : QString::number(aMin, 'f', iPrecision));
    double aMax = pImAssayInfo->alarmRangeMax * factor;
    ui->AlarmRangeUpperEdit->setText(aMax < 0 ? "" : QString::number(aMax, 'f', iPrecision));

    //----------------------------
    // 定量项目阴阳性阈值; 定量项目转换为定性项目，顾对定量项目进行阴阳阈值设置（0：定量，1：定性
    //----------------------------
    if (pImAssayInfo->caliType == 0)
    {
        ui->PosNegGrpBox->setChecked(false);
        ui->PosNegGrpBox->setEnabled(false);
    }
    else
    {
        if (UserInfoManager::GetInstance()->IsPermisson(PMS_SETTINGS_SHRESHOLD))
        {
            ui->PosNegGrpBox->setEnabled(true);
        }
        ui->PosNegGrpBox->setChecked(pImAssayInfo->thresholdActive);
        ui->PosOperatorCombo->setCurrentIndex((int)pImAssayInfo->threshold.posCompare.opera);
        ui->PosNumEdit->setText(QString::number(pImAssayInfo->threshold.posCompare.num*factor, 'f', iPrecision));
        ui->NegOperatorCombo->setCurrentIndex((int)pImAssayInfo->threshold.negCompare.opera);
        ui->NegNumEdit->setText(QString::number(pImAssayInfo->threshold.negCompare.num*factor, 'f', iPrecision));
    }
}

///
/// @brief  加载选中项目信息
///
/// @param[in]  item  项目信息
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
void ImAnalysisParamWidget::LoadAnalysisParam(const AssayListModel::StAssayListRowItem &item)
{
    ULOG(LOG_INFO, "%s(assayCode:%d)", __FUNCTION__, item.assayCode);

    m_bSampleTypeFlag = false;

    // 复位界面
    Reset();
    m_rowItem = item;

    // 判断待加载项有效
    if (item.assayCode == INVALID_ASSAY_CODE)
    {
        ULOG(LOG_ERROR, "Invalid assayCode.");
        return;
    }

    // 查询免疫通用项目信息
    std::shared_ptr<im::tf::GeneralAssayInfo> pImAssayInfo = GetImAssayInfoAndInitVerComb(item.assayCode, (::tf::DeviceType::type)item.model, item.version);
    if (pImAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Find none im assayinfo by assayCode:%d, model:%d, ver:%s.", item.assayCode, item.model, item.version.toStdString().c_str());
        return;
    }

    // 将信息刷入到控件

    //----------------------------
    // 基本参数
    //----------------------------
    std::shared_ptr<tf::GeneralAssayInfo> pAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item.assayCode);
    if (pAssayInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null assayInfo.");
        return;
    }
    ui->AssayNameEdit->setText(QString::fromStdString(pAssayInfo->assayName));
    ui->fullNameEdit->setText(QString::fromStdString(pAssayInfo->assayFullName));
    ui->PrintNameEdit->setText(pAssayInfo->printName.c_str());
    ui->AssayCodeEdit->setText(QString::number(pAssayInfo->assayCode));

    // 结果单位下拉框
    UpdateUnitCombbox();

    // 小数位
    int iPrecision = GetPrecion(item.assayCode);
    ui->DecimalCombo->setCurrentIndex(iPrecision);

    // 默认稀释倍率
    int32_t defaultDilu = pImAssayInfo->reactInfo.dilutionRatio;
    ui->DefaultDiluEdit->setText(QString::number(defaultDilu));

    // 获取当前单位与主单位的转化倍率
    double factor;
    if (!CommonInformationManager::GetInstance()->GetUnitFactor(item.assayCode, factor))
    {
        ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", m_rowItem.assayCode);
        return;
    }

    // 曲线延伸
    ui->l_extend_ckbox->setChecked(pImAssayInfo->extenL);
    ui->h_extend_ckbox->setChecked(pImAssayInfo->extenH);

    // 开瓶有效期
    ui->OpenDaysEdit->setText(QString::number(pImAssayInfo->daysOnBoard));
	ui->BotCaliWeekEdit->setText(pImAssayInfo->kitCaliPeriod > 0 ? QString::number(pImAssayInfo->kitCaliPeriod) : "");
	ui->LotCaliWeekEdit->setText(pImAssayInfo->lotCaliPeriod > 0 ? QString::number(pImAssayInfo->lotCaliPeriod) : "");

    // 单位系数相关的编辑框的赋值
    AssignFactorAboutEdit(pImAssayInfo, factor, iPrecision);
}

bool ImAnalysisParamWidget::GetGeneralAssayInfo(std::shared_ptr<tf::GeneralAssayInfo>& spAssayInfo, std::vector<std::shared_ptr<tf::QcDoc>>& changedQc)
{
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null assayInfo.");
        return false;
    }
    std::string strEnAbreviation = ui->AssayNameEdit->text().toUtf8().data();
    spAssayInfo->__set_assayName(strEnAbreviation);
    spAssayInfo->__set_printName(ui->PrintNameEdit->text().toUtf8().data());

    // 拿到原来的单位倍率
    double origFactor = -1;
    for (const ::tf::AssayUnit& item : spAssayInfo->units)
    {
        if (item.isCurrent)
        {
            origFactor = item.factor;
            break;
        }
    }
    if (origFactor > 0)
    {
        // 更新当前单位
        std::string currUnitName = ui->ResUnitCombo->currentText().toStdString();
        for (::tf::AssayUnit& item : spAssayInfo->units)
        {
            item.__set_isCurrent(item.name == currUnitName);
        }
    }

    // 更新小数位数
    spAssayInfo->__set_decimalPlace(ui->DecimalCombo->currentText().toInt());

    return true;
}

bool ImAnalysisParamWidget::UpdateParameteShow(const QString& strUnit)
{
    std::shared_ptr<CommonInformationManager> commMgr = CommonInformationManager::GetInstance();

    // 查询通用项目信息//  采用界面显示的精度
    int iPrecision = ui->DecimalCombo->currentText().toInt(); //GetPrecion(m_rowItem.assayCode);

                                                              // 获取当前单位与主单位的转化倍率
    double curfactor = 0;
    if (!commMgr->GetNewUnitFactor(m_rowItem.assayCode, strUnit, curfactor))
    {
        ULOG(LOG_ERROR, "GetNewUnitFactor By AssayCode[%d] Failed !", m_rowItem.assayCode);
        return false;
    }

    // 默认稀释倍率
    std::string curVer = ui->AssayVerCombo->currentText().toStdString();
    auto imGAI = commMgr->GetImmuneAssayInfo(m_rowItem.assayCode, (::tf::DeviceType::type)m_rowItem.model, curVer);
    if (imGAI == nullptr)
    {
        ULOG(LOG_ERROR, "GetImmuneAssayInfo failed. assayCode=%d", m_rowItem.assayCode);
        return false;
    }
    int32_t defaultDilu = imGAI->reactInfo.dilutionRatio;
    if (imGAI->caliType == 0)
    {
        defaultDilu = 1;
    }
    // 报告范围
    ui->RepRangeLowerEdit->setText(QString::number(imGAI->techRangeMin * curfactor * defaultDilu, 'f', iPrecision));
    ui->RepRangeUpperEdit->setText(QString::number(imGAI->techRangeMax * curfactor * defaultDilu, 'f', iPrecision));

    // 检查切换单位后的值合法性
    UiCtrlValidError errStat = CheckCtrlValueUnitChanged(imGAI, curfactor);
    if (errStat != UV_NO_ERROR)
    {
        std::map<UiCtrlValidError, QString>::iterator it = m_ctrlErrorDescription.find(errStat);
        if (it == m_ctrlErrorDescription.end())
        {
            ULOG(LOG_ERROR, "Unkonw ctrl value error description.");
            return false;
        }

        TipDlg(it->second).exec();
        ui->ResUnitCombo->setCurrentText(m_curUnit);

        return false;
    }
    m_curUnit = strUnit;

    // 根据选择的单位倍率赋值界面
    AssignFactorAboutEdit(imGAI, curfactor, iPrecision);

    return true;
}

///
/// @brief
///     功能说明
///
///@param[in]   被保存项目的定位信息
///
/// @return     成功返回true
///
/// @par History:
/// @li 1226/zhangjing，2023年2月16日，新建函数
///
bool ImAnalysisParamWidget::GetAnalysisParam(const AssayListModel::StAssayListRowItem &item, \
    std::shared_ptr<tf::GeneralAssayInfo>& pGen, std::vector<std::shared_ptr<::im::tf::GeneralAssayInfo>>& vecImAssayInfo, std::vector<std::shared_ptr<tf::QcDoc>>& changedQc)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (pGen == nullptr)
    {
        ULOG(LOG_WARN, "Null general assayinfo.");
        return false;
    }
    std::shared_ptr<CommonInformationManager> &assayMgr = CommonInformationManager::GetInstance();

    // 只要有一个符合类型的仪器在运行或者加样停，就不能保存，来自BUG单Mts18556
	if (assayMgr->IsExistDeviceRuning(item.assayClassify))
	{
		// 消息提示
		TipDlg(tr("保存失败"), tr("仪器处于运行状态或加样停状态不能进行保存操作！")).exec();
		return false;
	}
    // 获取当前单位与主单位的转化倍率
    double factor=1.0;
    if (!assayMgr->GetNewUnitFactor(item.assayCode, m_curUnit, factor))
    {
        ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", m_rowItem.assayCode);
        return false;
    }

    // 检查参数的有效性
    UiCtrlValidError errStat = CheckCtrlValue(item.assayCode, (::tf::DeviceType::type)item.model, item.version.toStdString());
    if (errStat != UV_NO_ERROR)
    {
        std::map<UiCtrlValidError, QString>::iterator it = m_ctrlErrorDescription.find(errStat);
        if (it == m_ctrlErrorDescription.end())
        {
            ULOG(LOG_ERROR, "Unknown ctrl error value description.");
            return false;
        }
        TipDlg(it->second).exec();
        return false;
    }

    // 获取基本参数
    if (!GetGeneralAssayInfo(pGen, changedQc))
    {
        ULOG(LOG_ERROR, "Failed to collect generalassayinfo from ui.");
        return false;
    }

    // 定量项目阴阳性阈值
    bool bPosNegActive = ui->PosNegGrpBox->isChecked();
    im::tf::threSholdForQuali threShold;
    if (bPosNegActive)
    {
        im::tf::comparePair pos;
        pos.__set_opera((im::tf::operatorType::type)ui->PosOperatorCombo->currentIndex());
        double dPos = ui->PosNumEdit->text().toDouble();
        dPos /= factor;
        pos.__set_num(dPos);

        im::tf::comparePair neg;
        neg.__set_opera((im::tf::operatorType::type)ui->NegOperatorCombo->currentIndex());
        double dNeg = ui->NegNumEdit->text().toDouble();
        dNeg /= factor;
        neg.__set_num(dNeg);

        threShold.__set_posCompare(pos);
        threShold.__set_negCompare(neg);
    }

    // 线性范围
    double alarmLower = -1;
    double alarmUpper = -1;
    if (!ui->AlarmRangeLowerEdit->text().isEmpty())
    {
        alarmLower = ui->AlarmRangeLowerEdit->text().toDouble();
        alarmLower /= factor;
    }
    if (!ui->AlarmRangeUpperEdit->text().isEmpty())
    {
        alarmUpper = ui->AlarmRangeUpperEdit->text().toDouble();
        alarmUpper /= factor;
    }

    // 判断变化是否是由于精度改变产生
    auto funcDoubleEque = [](int deci, double da, double db) -> bool{
        QString nmb1 = QString::number(da, 'f', deci + 2);
        QString nmb2 = QString::number(db, 'f', deci + 2);
        return (nmb1.mid(0, nmb1.indexOf('.') + 1 + deci) == nmb2.mid(0, nmb2.indexOf('.') + 1 + deci));
    };

    // 查询免疫通用项目信息
    std::string currentVer = ui->AssayVerCombo->currentText().toStdString();
    for (std::shared_ptr<im::tf::GeneralAssayInfo>& imAssay : vecImAssayInfo)
    {
        // 特定需求：当前页面的修改对所有版本同步
        imAssay->usingFlag = (imAssay->version == currentVer);
        // 如果值的变化是小数位数导致的，则不进行覆盖保存（来自 BUG:0021631的修改)
        if (!funcDoubleEque(pGen->decimalPlace, imAssay->alarmRangeMin, alarmLower))
        {
            imAssay->__set_alarmRangeMin(alarmLower);
        }
        if (!funcDoubleEque(pGen->decimalPlace, imAssay->alarmRangeMax, alarmUpper))
        {
            imAssay->__set_alarmRangeMax(alarmUpper);
        }
        imAssay->__set_extenL(ui->l_extend_ckbox->isChecked());
        imAssay->__set_extenH(ui->h_extend_ckbox->isChecked());
        imAssay->__set_thresholdActive(bPosNegActive);

        if (bPosNegActive)
        {
			// 增加对操作符的判断 add by Chenjianlin 20240311
			if (imAssay->threshold.posCompare.opera != threShold.posCompare.opera)
			{
                imAssay->__set_threshold(threShold);
				//imAssay->threshold.posCompare.__set_opera(threShold.posCompare.opera);
			}
			if (imAssay->threshold.negCompare.opera != threShold.negCompare.opera)
			{
                imAssay->__set_threshold(threShold);
				//imAssay->threshold.negCompare.__set_opera(threShold.negCompare.opera);
			}
			if (!funcDoubleEque(pGen->decimalPlace, imAssay->threshold.posCompare.num*factor, threShold.posCompare.num*factor))
			{
                imAssay->__set_threshold(threShold);
				//imAssay->threshold.posCompare.__set_num(threShold.posCompare.num);
			}
			if (!funcDoubleEque(pGen->decimalPlace, imAssay->threshold.negCompare.num*factor, threShold.negCompare.num*factor))
            {
                imAssay->__set_threshold(threShold);
				//imAssay->threshold.negCompare.__set_num(threShold.negCompare.num);
            }
        }
    }

    return true;
}

///
/// @brief 获取更改操作细节信息
///
/// @param[in]  strOptLog  返回的日志记录信息
/// @param[in]  spAssayInfoUi  界面修改的项目信息
/// @param[in]  vecImAssayInfoUi  界面修改的免疫项目信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月7日，新建函数
///
void ImAnalysisParamWidget::GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfoUi)
{
	auto MakeStr = [](QString& strOptLog, const QString strName, const QString strValue)
	{
		strOptLog += " ";
		strOptLog += strName;
		strOptLog += ":";
		strOptLog += strValue;
	};
	// 参数检查
	if (nullptr == spAssayInfoUi)
	{
		ULOG(LOG_WARN, "The input param is nullptr.");
		return;
	}
	// 从CommMgr中获取免疫项目信息
	std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
	std::shared_ptr<CommonInformationManager> &assayMgr = CommonInformationManager::GetInstance();
	assayMgr->GetImmuneAssayInfos(vecImAssayInfo, spAssayInfoUi->assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
	auto spAssayInfo = assayMgr->GetAssayInfo(spAssayInfoUi->assayCode);
	if (spAssayInfo == nullptr)
	{
		ULOG(LOG_WARN, "Get Assay parameter infor error.");
		return;
	}
	// 如果修改了简称
	if (spAssayInfo->assayName != spAssayInfoUi->assayName)
	{
		MakeStr(strOptLog, ui->AssayNaneLab->text(), QString::fromStdString(spAssayInfoUi->assayName));
	}
	// 如果修改了结果单位
	QString strUnitOld("");
	for (auto& atUnit : spAssayInfo->units)
	{
		if (atUnit.isCurrent)
		{
			strUnitOld = QString::fromStdString(atUnit.name);
			break;
		}
	}
	double dFactor = 1;
	for (auto& atUnit : spAssayInfoUi->units)
	{
		if (atUnit.isCurrent)
		{
			dFactor = atUnit.factor;
			if (strUnitOld != QString::fromStdString(atUnit.name))
			{
				MakeStr(strOptLog, ui->ResUnitLab->text(), QString::fromStdString(atUnit.name));
			}
			break;
		}
	}	
	// 如果修改了打印名称
	if (spAssayInfoUi->printName != spAssayInfo->printName)
	{
		MakeStr(strOptLog, ui->PrintNameLab->text(), QString::fromStdString(spAssayInfoUi->printName));
	}
	// 如果修改了小数位数
	if (spAssayInfoUi->decimalPlace != spAssayInfo->decimalPlace)
	{
		MakeStr(strOptLog, ui->DecimalLab->text(), QString::number(spAssayInfoUi->decimalPlace));
	}	
	QString strOpen(tr("打开")), strClose(tr("关闭")), strCheck(tr("勾选")), strUncheck(tr("取消勾选"));
	bool alarmRangeMin = false, extenL = false, extenH = false, thresholdActive = false, posCompare = false, negCompare=false;
	for (auto& atValUi : vecImAssayInfoUi)
	{
		for (auto& atVal : vecImAssayInfo)
		{
			if (atValUi->version != atVal->version)
			{
				continue;
			}
			// 如果修改了线性范围
			if ((!IsEqual(atValUi->alarmRangeMin, atVal->alarmRangeMin) || !IsEqual(atValUi->alarmRangeMax, atVal->alarmRangeMax)) && !alarmRangeMin)
			{
				MakeStr(strOptLog, ui->Lab_7->text(), QString::number(atValUi->alarmRangeMin * dFactor, 'f', spAssayInfoUi->decimalPlace) + "-" + QString::number(atValUi->alarmRangeMax * dFactor, 'f', spAssayInfoUi->decimalPlace));
				alarmRangeMin = true;
			}
			// 如果修改了曲线延伸
			if (atValUi->extenL != atVal->extenL && !extenL)
			{
				MakeStr(strOptLog, ui->Lab_8->text() + ui->l_extend_ckbox->text(), atValUi->extenL ? strCheck : strUncheck);
				extenL = true;
			}
			if (atValUi->extenH != atVal->extenH && !extenH)
			{
				MakeStr(strOptLog, ui->Lab_8->text() + ui->h_extend_ckbox->text(), atValUi->extenH ? strCheck : strUncheck);
				extenH = true;
			}
			// 如果修改了阴阳性阈值
			QString strGroupText = ui->PosNegGrpBox->title();
			// 检查开关
			if (atValUi->thresholdActive != atVal->thresholdActive && !thresholdActive)
			{
				MakeStr(strOptLog, strGroupText, atValUi->thresholdActive ? strOpen : strClose);
				thresholdActive = true;
			}
			// 若启动，检查参数修改
			if (atValUi->thresholdActive)
			{
				// 阳性
				if ((atValUi->threshold.posCompare.opera != atVal->threshold.posCompare.opera
					|| !IsEqual(atValUi->threshold.posCompare.num, atVal->threshold.posCompare.num)) && !posCompare)
				{
					MakeStr(strOptLog, strGroupText + ui->Lab_13->text(),
						ui->PosOperatorCombo->currentText() + QString::number(atValUi->threshold.posCompare.num * dFactor, 'f', spAssayInfoUi->decimalPlace));
					posCompare = true;
				}
				// 阴性
				if ((atValUi->threshold.negCompare.opera != atVal->threshold.negCompare.opera
					|| !IsEqual(atValUi->threshold.negCompare.num, atVal->threshold.negCompare.num)) && !negCompare)
				{
					MakeStr(strOptLog, strGroupText + ui->Lab_14->text(),
						ui->NegOperatorCombo->currentText() + QString::number(atValUi->threshold.negCompare.num * dFactor, 'f', spAssayInfoUi->decimalPlace));
					negCompare = true;
				}
			}
		}
	}
}

///
/// @brief  单位切换
///
/// @param[in]  text  切换的新单位
///
/// @par History:
/// @li 1226/zhangjing，2023年3月28日，新建函数
///
bool ImAnalysisParamWidget::OnUnitChanged(const QString &text)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 忽略加载参数时引发的非当前项目变化
    QString strCurrentAssayCode = ui->AssayCodeEdit->text();
    static QString st_strAssayCode = strCurrentAssayCode;
    if (st_strAssayCode != strCurrentAssayCode)
    {
        st_strAssayCode = strCurrentAssayCode;
        return true;
    }

    // 单位未变更
    if (text == m_curUnit)
    {
        return true;
    }

    return UpdateParameteShow(text);
}

bool ImAnalysisParamWidget::OnDecimalChanged(const QString& text)
{
    // 忽略加载参数时引发的非当前项目变化
    QString strCurrentAssayCode = ui->AssayCodeEdit->text();
    static QString st_strAssayCode = strCurrentAssayCode;
    if (st_strAssayCode != strCurrentAssayCode)
    {
        st_strAssayCode = strCurrentAssayCode;
        return true;
    }

    return UpdateParameteShow(ui->ResUnitCombo->currentText());
}

void ImAnalysisParamWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 项目名称
    ui->fullNameEdit->setEnabled(userPms->IsPermisson(PSM_ASSAY_ASSAYNAME));

    // 项目通道号
    ui->AssayCodeEdit->setEnabled(userPms->IsPermisson(PSM_ASSAY_ASSAYCODE));

    // 开瓶有效期
    ui->OpenDaysEdit->setEnabled(userPms->IsPermisson(PSM_ASSAY_OPENBOTTLEEXP));

    // 简称、打印名称
    ui->AssayNameEdit->setEnabled(userPms->IsPermisson(PMS_ASSAY_SET_SHORTNAME));
    ui->PrintNameEdit->setEnabled(userPms->IsPermisson(PMS_ASSAY_SET_PRINTNAME));

    // 结果单位
    ui->ResUnitCombo->setEnabled(userPms->IsPermisson(PMS_ASSAY_RESULT_UNIT));

    // 小数位数
    ui->DecimalCombo->setEnabled(userPms->IsPermisson(PMS_ASSAY_RESULT_PRICISION));

    // 参数版本； bug 0027739；注：参数版本不可修改，切换版本只是查看不同版本的信息
    //ui->AssayVerCombo->setEnabled(userPms->IsPermisson(PSM_ASSAY_VERSION));

    // 曲线延伸
    bool isAllowCurestrs = userPms->IsPermisson(PMS_SET_CURVE_EXTEND);
    ui->l_extend_ckbox->setEnabled(isAllowCurestrs);
    ui->h_extend_ckbox->setEnabled(isAllowCurestrs);

    // 阴阳阈值
    ui->PosNegGrpBox->setEnabled(userPms->IsPermisson(PMS_SETTINGS_SHRESHOLD));

    // 线性范围
    bool bRef = userPms->IsPermisson(PSM_IM_ASSAYSET_LINARSET);
    ui->AlarmRangeLowerEdit->setEnabled(bRef);
    ui->AlarmRangeUpperEdit->setEnabled(bRef);

    // 默认稀释倍数
    ui->DefaultDiluEdit->setEnabled(userPms->IsPermisson(PSM_IM_ASSAYSET_DEFAULT_DILURATE));
}

void ImAnalysisParamWidget::UpdateUnitCombbox()
{
    std::shared_ptr<tf::GeneralAssayInfo> pAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_rowItem.assayCode);
    if (pAssayInfo == nullptr)
    {
        return;
    }

    // 结果单位下拉框
    ui->ResUnitCombo->clear();
    QString currentText;
    for (const tf::AssayUnit& unit : pAssayInfo->units)
    {
        ui->ResUnitCombo->addItem(QString::fromStdString(unit.name));
        if (unit.isCurrent)
        {
            currentText = QString::fromStdString(unit.name);
        }
    }
    ui->ResUnitCombo->setCurrentText(currentText);
    m_curUnit = currentText;
}
