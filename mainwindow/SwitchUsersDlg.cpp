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
/// @file     SwitchUsersDlg.cpp
/// @brief 	  切换用户对话框
///
/// @author   8580/GongZhiQiang
/// @date      2023年7月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SwitchUsersDlg.h"
#include "ui_SwitchUsersDlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"

#include "manager/SystemConfigManager.h"
#include "manager/UserInfoManager.h"
#include "src/leonis/thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"

SwitchUsersDlg::SwitchUsersDlg(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::SwitchUsersDlg();
	ui->setupUi(this);
	Init();
}

SwitchUsersDlg::~SwitchUsersDlg()
{
}

///
///  @brief 初始化
///
///
///  @return	
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年7月10日，新建函数
///
void SwitchUsersDlg::Init()
{
	SetTitleName(tr("切换用户"));

	connect(ui->okBtn, &QPushButton::clicked, this, &SwitchUsersDlg::OnOkBtnClicked);
	connect(ui->cancerBtn, &QPushButton::clicked, this, &SwitchUsersDlg::OnCancerBtnClicked);
    connect(ui->edit_userName, SIGNAL(returnPressed()), this, SLOT(OnUserNameEntern()));
    connect(ui->edit_passWord, SIGNAL(returnPressed()), this, SLOT(OnOkBtnClicked()));

	// 密码框输入隐藏
	ui->edit_passWord->setEchoMode(QLineEdit::Password);

    // 设置焦点
    ui->edit_userName->setFocus();
    ui->okBtn->setFocusPolicy(Qt::NoFocus);
    ui->cancerBtn->setFocusPolicy(Qt::NoFocus);
}

///
///  @brief 确定按钮
///
///
///  @return	
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年7月10日，新建函数
///
void SwitchUsersDlg::OnOkBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	::tf::UserInfo user;
	user.__set_username(ui->edit_userName->text().toStdString());
	user.__set_password(ui->edit_passWord->text().toStdString());

	if (DcsControlProxy::GetInstance()->Login(user))
	{
		// 核对登录名密码，成功后发送登录成功信号
		POST_MESSAGE(MSG_ID_LOGIN_SUCCESSED);

		// 记录当前登录用户
		UserInfoManager::GetInstance()->InitUserInfo(user.username);
		this->close();

	}
	else
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("登录失败，登录名或密码错误!")));
		pTipDlg->exec();
	}
}

///
///  @brief 取消按钮
///
///
///  @return	
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年7月10日，新建函数
///
void SwitchUsersDlg::OnCancerBtnClicked()
{
	this->close();
}

///
/// @brief  用户名称回车
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月1日，新建函数
///
void SwitchUsersDlg::OnUserNameEntern()
{
    ui->edit_passWord->setFocus();
}
