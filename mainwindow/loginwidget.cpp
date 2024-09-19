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
/// @file     loginwidget.cpp
/// @brief    用户登录界面
///
/// @author   4170/TangChuXian
/// @date     2020年5月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QTimer>

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"

#include "manager/SystemConfigManager.h"
#include "manager/UserInfoManager.h"
#include "manager/SystemPowerManager.h"
#include "src/leonis/thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"

#include "utility/maintaindatamng.h"
#include "uidcsadapter/uidcsadapter.h"



#define DELAY_MSEC_ON_CLOSE                         (10)                // 延迟关闭毫秒数


LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent), 
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);
	setWindowIcon(QIcon(":/Leonis/resource/image/Mc64.ico"));

    // 注册开机动画播放结束处理函数
	//if (SystemConfigManager::GetInstance()->IsShowFullScreen())
	//{
	//	REGISTER_HANDLER(MSG_ID_STARTUP_VIDEO_FINISHED, this, showFullScreen);
	//}
	//else
	//{
	//	REGISTER_HANDLER(MSG_ID_STARTUP_VIDEO_FINISHED, this, showMaximized);
	//}

    // 隐藏错误提示
    ui->errorTips->setVisible(false);

    // 连接信号槽
    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(OnLoginBtnClicked()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(OnShutdownBtnClicked()));
	connect(ui->userNameEdit, SIGNAL(returnPressed()), this, SLOT(OnUserNameEntern()));
	connect(ui->passwordEdit, SIGNAL(returnPressed()), this, SLOT(OnLoginBtnClicked()));
    connect(ui->userNameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnInputContent()));
    connect(ui->passwordEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnInputContent()));

    // 初始化电源管理类，用于监听登录后的关机维护变更事件
    SystemPowerManager::GetInstance();
}

LoginWidget::~LoginWidget()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月8日，新建函数
///
void LoginWidget::showEvent(QShowEvent *event)
{
    // 获取设备类型判断是生化单机版、免疫单机版、生免联机
    if (gUiAdapterPtr()->WhetherOnlyImDev())
    {
        // 只有免疫设备
        ui->logo_main->setProperty("devtype", "im");
    }
    else if (gUiAdapterPtr()->WhetherContainImDev())
    {
        // 生免联机
        ui->logo_main->setProperty("devtype", "ch-im");
    }
    else
    {
        // 否则则为生化单机版
        ui->logo_main->setProperty("devtype", "ch");
    }

    // 刷新样式
    ui->logo_main->style()->unpolish(ui->logo_main);
    ui->logo_main->style()->polish(ui->logo_main);
    ui->logo_main->update();

    // 登录界面一开始显示时尝试自动登录
    std::string strUserName = DcsControlProxy::GetInstance()->AutoLogin();
    if (strUserName.empty())
    {
        // 未能自动登录，正常显示
        QWidget::showEvent(event);
        return;
    }

    // 自动登录成功，直接进入登录成功流程
    // 核对用户名密码，成功后发送登录成功信号
    POST_MESSAGE(MSG_ID_LOGIN_SUCCESSED);

    // 记录当前登录用户
    UserInfoManager::GetInstance()->InitUserInfo(strUserName);

    // 延迟10毫秒后关闭窗口
    QTimer::singleShot(DELAY_MSEC_ON_CLOSE, this, SLOT(close()));
}

///
/// @brief 通过界面按钮退出软件，过滤Alt+F4关闭窗口事件
///
/// @param[in]  eventType  
/// @param[in]  message  
/// @param[in]  result  
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月9日，新建函数
///
bool LoginWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef _WIN32
	// 查看事件类型
	if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
	{
		MSG* pMsg = static_cast<MSG *>(message);
		if (nullptr != pMsg)
		{
			switch (pMsg->message)
			{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if ((VK_F4 == pMsg->wParam) && (::GetKeyState(VK_MENU) & 0xF000))
				{
					ULOG(LOG_INFO, "alt + f4 %s()", __FUNCTION__);
					return TRUE;
				}
				break;
			default:
				break;
			}
		}
	}
#endif
	return QWidget::nativeEvent(eventType, message, result);
}

///
/// @brief 登录按钮被点击
///     
/// @par History:
/// @li 4170/TangChuXian，2020年4月29日，新建函数
///
void LoginWidget::OnLoginBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	::tf::UserInfo user;
	user.__set_username(ui->userNameEdit->text().toStdString());
	user.__set_password(ui->passwordEdit->text().toStdString());

	if (DcsControlProxy::GetInstance()->Login(user))
	{
		// 核对用户名密码，成功后发送登录成功信号
		POST_MESSAGE(MSG_ID_LOGIN_SUCCESSED);

        // 记录当前登录用户
        UserInfoManager::GetInstance()->InitUserInfo(user.username);

		// 延迟10毫秒后关闭窗口
		QTimer::singleShot(DELAY_MSEC_ON_CLOSE, this, SLOT(close()));
	}
	else
	{
        // 隐藏错误提示
        ui->errorTips->setVisible(true);
	}

}

///
/// @brief 关机按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月29日，新建函数
/// @li 8580/GongZhiQiang，2023年7月14日，修改退出逻辑
///
void LoginWidget::OnShutdownBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 仿误触
	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("请确认是否退出程序？"), TipDlgType::TWO_BUTTON));
	if (pTipDlg->exec() == QDialog::Rejected)
	{
		return ;
	}

	// 执行退出
	SystemPowerManager::GetInstance()->StartExit(PowerOffMode::LOGIN_MODE);
}

///
/// @brief
///     用户输入内容
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月8日，新建函数
///
void LoginWidget::OnInputContent()
{
    // 隐藏错误提示
    ui->errorTips->setVisible(false);
}

///
/// @brief  用户名称回车
///
/// @par History:
/// @li 1226/zhangjing，2023年3月20日，新建函数
///
void  LoginWidget::OnUserNameEntern()
{
	ui->passwordEdit->setFocus();
}

