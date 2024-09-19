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
/// @file     ShutDownDlg.h
/// @brief 	  关机对话框
///
/// @author   7656/zhang.changjiang
/// @date      2023年5月6日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年5月6日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ShutDownDlg.h"
#include "ui_ShutDownDlg.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "SwitchUsersDlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "utility/maintaindatamng.h"
#include "manager/UserInfoManager.h"
#include "manager/SystemPowerManager.h"
#include "thrift/DcsControlProxy.h"

#include <QProcess>
#include <QThread>

ShutDownDlg::ShutDownDlg(QWidget *parent)
	: BaseDlg(parent)
{
	ui = new Ui::ShutDownDlg();
	ui->setupUi(this);
	Init();
}

ShutDownDlg::~ShutDownDlg()
{
	delete ui;
}

void ShutDownDlg::Init()
{
	SetTitleName(tr(u8"关机"));

	connect(ui->okBtn, &QPushButton::clicked, this, &ShutDownDlg::OnOkBtnClicked);
	connect(ui->cancerBtn, &QPushButton::clicked, this, &ShutDownDlg::OnCancerBtnClicked);
	// 默认选择退出程序
	ui->rBtn_exitProgram->setChecked(true);
}

///
///  @brief 确定按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年7月12日，新建函数
///
void ShutDownDlg::OnOkBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 隐藏当前窗口
	this->hide();

	if (ui->rBtn_exitProgram->isChecked())
	{
		// 仿误触
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("请确认是否退出程序？"), TipDlgType::TWO_BUTTON));
		if (pTipDlg->exec() == QDialog::Accepted)
		{
			SystemPowerManager::GetInstance()->StartExit(PowerOffMode::MAIN_MODE);
		}
	}
	else if (ui->rBtn_switchUsers->isChecked())
	{
		if (!SwitchUsers())
		{
			ULOG(LOG_ERROR, "SwitchUsers Failed !");
		}
	}

	// 关闭当前窗口
	this->close();
}

///
///  @brief 取消按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月6日，新建函数
///
void ShutDownDlg::OnCancerBtnClicked()
{
	this->close();
}

///
///  @brief 切换用户
///
///
///
///  @return	true:执行成功
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年7月10日，新建函数
///
bool ShutDownDlg::SwitchUsers()
{
	std::shared_ptr<SwitchUsersDlg>  m_SwitchUsersDlg(new SwitchUsersDlg());
	m_SwitchUsersDlg->exec();

	return true;
}

