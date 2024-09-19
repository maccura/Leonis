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
/// @file     EnableAuditDlg.cpp
/// @brief    审核弹窗
///
/// @author   7951/LuoXin
/// @date     2023年1月6日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年1月6日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "EnableAuditDlg.h"
#include "ui_EnableAuditDlg.h" 
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

EnableAuditDlg::EnableAuditDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::EnableAuditDlg)
{
	ui->setupUi(this);

	// 设置标题
	SetTitleName(tr("审核"));

	// 保存按钮
	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
}

EnableAuditDlg::~EnableAuditDlg()
{

}

void EnableAuditDlg::LoadDataToCtrls()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    bool bEnableAudit = false;
    if (!DictionaryQueryManager::GetAuditConfig(bEnableAudit))
    {
        ui->enableAudit_checkBox->setChecked(bEnableAudit);
    }
}

void EnableAuditDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 保存样本架分配
    bool isEnableAudit = ui->enableAudit_checkBox->isChecked();
    if (!DictionaryQueryManager::SaveAuditConfig(isEnableAudit))
    {
        ULOG(LOG_ERROR, "Failed to save audit config.");
        return;
    }
}
