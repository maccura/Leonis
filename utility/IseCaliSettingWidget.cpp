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
/// @file     IseCaliSettingWidget.h
/// @brief    ISE项目分析参数配置
///
/// @author   7951/LuoXin
/// @date     2022年6月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年6月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "IseCaliSettingWidget.h"
#include "ui_IseCaliSettingWidget.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/common/Mlog/mlog.h"

///
/// @brief
///     构造函数
///
/// @param[in]  parent  
/// @par History:
/// @li 7951/LuoXin，2022年6月28日，新建函数
///
IseCaliSettingWidget::IseCaliSettingWidget(QWidget *parent /*= Q_NULLPTR*/)
	: QWidget(parent)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ui = new Ui::IseCaliSettingWidget();
	ui->setupUi(this);

    ui->compensated_edit->setValidator(new QDoubleValidator(0.0, 99999999.0, 2, ui->compensated_edit));
    ui->time_edit->setValidator(new QIntValidator(0, 99999999, ui->time_edit));
}


///
/// @brief
///     析构函数
///
/// @par History:
/// @li 7951/LuoXin，2022年6月28日，新建函数
///
IseCaliSettingWidget::~IseCaliSettingWidget()
{

}

void IseCaliSettingWidget::Reset()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	for (auto edit : QWidget::findChildren<QLineEdit*>())
	{
		edit->clear();
	}

	for (auto box : QWidget::findChildren<QCheckBox*>())
	{
		box->setChecked(false);
	}
}

///
/// @brief
///     初始化范围界面数据
///
/// @param[in]  item  项目信息
///
/// @par History:
/// @li 7951/LuoXin，2022年6月28日，新建函数
///
bool IseCaliSettingWidget::LoadRangeParameter(const AssayListModel::StAssayListRowItem& item)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	Reset();

	// 判断待加载项有效
	if (item.assayCode == INVALID_ASSAY_CODE)
	{
		return true;
	}

	// 查询通用项目信息
	auto &CIM = CommonInformationManager::GetInstance();
	auto GAI = CIM->GetAssayInfo(item.assayCode);
	if (GAI == nullptr)
	{
		ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
		return false;
	}

	// 查询ISE通用项目信息
	auto IseGAI = CIM->GetIseAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model, item.version.toStdString());
	if (IseGAI == nullptr)
	{
		ULOG(LOG_ERROR, "GetIseAssayInfo() Failed!");
		return false;
	}

	// 刷入控件
	// 校准失败是否自动屏蔽
	ui->cali_auto_mask_ckbox->setChecked(IseGAI->autoMaskWhenCaliFail);
	// 校准周期
    int tempData = IseGAI->caliSuggest.timeoutCali.timeout;
	ui->time_edit->setText(tempData == 0 ? "" : QString::number(tempData));

	// 耗材更换
    ui->consumables_change_ckbox->setChecked(IseGAI->caliSuggest.enableChangeSupply);
	// 校准结果过期
	ui->timeout_cali_type_ckbox->setChecked(IseGAI->caliSuggest.timeoutCali.enable);
	// 质控失败
	ui->single_qc_cali_mode_ckbox->setChecked(IseGAI->caliSuggest.qcViolation.enableSingleCaliMode);
	ui->union_qc_cali_mode_ckbox->setChecked(IseGAI->caliSuggest.qcViolation.enableUnionCaliMode);

    // 斜率变化允许值
    ui->compensated_edit->setText(IseGAI->compensatedLimit > 0 ? QString::number(IseGAI->compensatedLimit) : "");
	return true;
}

bool IseCaliSettingWidget::GetAnalysisParam(ise::tf::GeneralAssayInfo& iseGAI, ise::tf::SpecialAssayInfo& iseSAI, int assayCode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 校准失败是否自动屏蔽
	iseGAI.__set_autoMaskWhenCaliFail(ui->cali_auto_mask_ckbox->isChecked());
	// 校准建议
	ise::tf::CaliSuggest CS;

	// 校准周期
    if (ui->time_edit->text() == "0")
    {
        TipDlg(tr("提示"), tr("值错误：校准周期不能设置为0")).exec();
        return false;
    }

	CS.timeoutCali.timeout = ui->time_edit->text().toInt();
    CS.timeoutCali.timeUnit = tf::TimeUnit::TIME_UNIT_DAY;

    CS.timeoutCali.caliMode = ::tf::CaliMode::CALI_MODE_FULLDOT;

    // 耗材更换
    CS.enableChangeSupply = ui->consumables_change_ckbox->isChecked();
	// 校准结果过期
	CS.timeoutCali.enable = ui->timeout_cali_type_ckbox->isChecked();

	// 单质控品失控
	CS.qcViolation.enableSingleCaliMode = ui->single_qc_cali_mode_ckbox->isChecked();
	// 联合质控品失控
	CS.qcViolation.enableUnionCaliMode = ui->union_qc_cali_mode_ckbox->isChecked();

	iseGAI.__set_caliSuggest(CS);

    QString tempStr = ui->compensated_edit->text();
    QRegExp re("^[0-9]+(\\.[0-9]+)?$");

    // 斜率变换允许值为空则不检查,bug2896
    if (!re.exactMatch(tempStr) && !tempStr.isEmpty())
    {
        TipDlg(tr("提示"), tr("值错误：斜率变化允许值只能是数字")).exec();
        return false;
    }


    iseGAI.__set_compensatedLimit(tempStr.toDouble());
   
	return true;
}

bool IseCaliSettingWidget::HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item)
{
    // 查询ISE通用项目信息
    auto& CIM = CommonInformationManager::GetInstance();
    auto& IseGAI = CIM->GetIseAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model);
    if (IseGAI == nullptr)
    {
        ULOG(LOG_ERROR, "GetIseAssayInfo() Failed!");
        return false;
    }

    // 校准周期
    if (IseGAI->caliSuggest.timeoutCali.timeout != ui->time_edit->text().toInt())
    {
        return true;
    }

    // 校准屏蔽
    if (IseGAI->autoMaskWhenCaliFail != ui->cali_auto_mask_ckbox->isChecked()
        || IseGAI->caliSuggest.enableChangeSupply != ui->consumables_change_ckbox->isChecked()
        || IseGAI->caliSuggest.timeoutCali.enable != ui->timeout_cali_type_ckbox->isChecked()
        || IseGAI->caliSuggest.qcViolation.enableSingleCaliMode != ui->single_qc_cali_mode_ckbox->isChecked()
        || IseGAI->caliSuggest.qcViolation.enableUnionCaliMode != ui->union_qc_cali_mode_ckbox->isChecked())
    {
        return true;
    }

    // 斜率变化允许值
    QString tempStr = IseGAI->compensatedLimit > 0 ? QString::number(IseGAI->compensatedLimit) : "";
    return ui->compensated_edit->text() != tempStr;
}
