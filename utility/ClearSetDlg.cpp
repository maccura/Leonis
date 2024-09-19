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
/// @file     ClearSetDlg.cpp
/// @brief    应用--系统--清空设置
///
/// @author   7951/LuoXin
/// @date     2022年12月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年12月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ClearSetDlg.h"
#include "ui_ClearSetDlg.h" 
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

ClearSetDlg::ClearSetDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::ClearSetDlg)
{
	ui->setupUi(this);

	InitCtrls();
}

ClearSetDlg::~ClearSetDlg()
{
}

void ClearSetDlg::LoadDataToCtrls()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	Reset();

    ClearSetData data;
    if (!DictionaryQueryManager::GetClearSet(data))
    {
        ULOG(LOG_ERROR, "Failed to get clearset config.");
        return;
    }

	switch (data.iClearModel)
	{
	case 0:
		ui->startup_clear_rdbtn->setChecked(true);
		break;
	case 1:
		ui->timing_clear_rdbtn->setChecked(true);
		break;
	default:
		break;
	}

	// 数据清空
	ui->clear_data_groupbox->setChecked(data.bEnableClearData);
	// 时间
	ui->timeEdit->setTime(QTime::fromString(QString::fromStdString( data.strTime)));
	// 清空数据类型
	ui->sample_ckbox->setChecked(data.bClearDataTypeSample);
	ui->qc_ckbox->setChecked(data.bClearDataTypeQc);
	ui->cali_ckbox->setChecked(data.bClearDataTypeCali);

    // 清空状态
    ui->pending_ckbox->setChecked(data.bClearDataTypeSample4Pending);
    ui->complete_ckbox->setChecked(data.bClearDataTypeSample4Tested);

	// 位置设置
	ui->clear_seat_groupbox->setChecked(data.bEnableClearSeat);
	// 校准完成清空校准架位置
	ui->cali_complete_ckbox->setChecked(data.bCaliCompleteClearRack);
	// 质控完成清空质控架位置
	ui->qc_complete_ckbox->setChecked(data.bQcCompleteClearRack);
}

void ClearSetDlg::InitCtrls()
{
	SetTitleName(tr("清空设置"));

	// 定时清空
	connect(ui->timing_clear_rdbtn, &QRadioButton::toggled, this,[&](bool checked)
	{ ui->timeEdit->setEnabled(checked && ui->clear_data_groupbox->isChecked()); });

    connect(ui->clear_data_groupbox, &QGroupBox::toggled, this, [&](bool checked) 
    { ui->timeEdit->setEnabled(checked && ui->timing_clear_rdbtn->isChecked()); });

	// 确认按钮
	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void ClearSetDlg::Reset()
{
	ui->startup_clear_rdbtn->setChecked(true);
	ui->timeEdit->clear();
	ui->clear_data_groupbox->setChecked(false);
	ui->clear_seat_groupbox->setChecked(false);

	for (auto box : QWidget::findChildren<QCheckBox*>())
	{
		box->setChecked(false);
	}
}

bool ClearSetDlg::CheckUserData()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (ui->clear_data_groupbox->isChecked() 
		&& !( ui->sample_ckbox->isChecked()
			|| ui->qc_ckbox->isChecked()
			|| ui->cali_ckbox->isChecked()))
	{
		TipDlg(tr("保存失败"), tr("至少选择一种清空的数据类型")).exec();
		return false;
	}

	if (ui->clear_seat_groupbox->isChecked()
		&& !(ui->cali_complete_ckbox->isChecked()
			|| ui->qc_complete_ckbox->isChecked()))
	{
		TipDlg(tr("保存失败"), tr("至少选择一种清空的位置类型")).exec();
		return false;
	}

    if (ui->clear_data_groupbox->isChecked()
        && !(ui->pending_ckbox->isChecked()
            || ui->complete_ckbox->isChecked()))
    {
        TipDlg(tr("保存失败"), tr("至少选择一种清空的状态类型")).exec();
        return false;
    }

	return true;
}

void ClearSetDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (!CheckUserData())
	{
		ULOG(LOG_INFO, "User Input Params Invalid");
		return;
	}

	ClearSetData data;
	// 数据清空
	data.bEnableClearData = ui->clear_data_groupbox->isChecked();
	// 开机后清空
	data.iClearModel = ui->startup_clear_rdbtn->isChecked()? 0 : 1;
	// 时间
	data.strTime = ui->timeEdit->text().toStdString();
	// 清空模式不为定时清空时间置空
	if (data.iClearModel != 1)
	{
		data.strTime.clear();
	}

	// 清空数据类型
	data.bClearDataTypeSample = ui->sample_ckbox->isChecked();
	data.bClearDataTypeQc = ui->qc_ckbox->isChecked();
	data.bClearDataTypeCali = ui->cali_ckbox->isChecked();

    // 清空状态
    data.bClearDataTypeSample4Pending = ui->pending_ckbox->isChecked();
    data.bClearDataTypeSample4Tested = ui->complete_ckbox->isChecked();

	// 位置设置
	data.bEnableClearSeat = ui->clear_seat_groupbox->isChecked();
	// 校准完成清空校准架位置
	data.bCaliCompleteClearRack = ui->cali_complete_ckbox->isChecked();
	// 质控完成清空质控架位置
	data.bQcCompleteClearRack = ui->qc_complete_ckbox->isChecked();

    if (!DictionaryQueryManager::SaveClearSet(data))
    {
        ULOG(LOG_ERROR, "Failed to save clearset config!");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

	close();
}

void ClearSetDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    // 清空设置
    bool isPermitSet = UserInfoManager::GetInstance()->IsPermisson(PSM_IM_CLEARSET_CHECKDATA);
    ui->clear_data_groupbox->setEnabled(isPermitSet);
    ui->clear_seat_groupbox->setEnabled(isPermitSet);
}
