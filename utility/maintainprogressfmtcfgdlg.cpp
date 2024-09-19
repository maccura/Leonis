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
/// @file     maintainprogressfmtcfgdlg.cpp
/// @brief    维护进度格式设置对话框
///
/// @author   4170/TangChuXian
/// @date     2023年7月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintainprogressfmtcfgdlg.h"
#include "ui_maintainprogressfmtcfgdlg.h"
#include "src/common/Mlog/mlog.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/DictionaryQueryManager.h"

MaintainProgressFmtCfgDlg::MaintainProgressFmtCfgDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    // 初始化ui对象
    ui = new Ui::MaintainProgressFmtCfgDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

MaintainProgressFmtCfgDlg::~MaintainProgressFmtCfgDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void MaintainProgressFmtCfgDlg::InitBeforeShow()
{
    // 设置标题
    SetTitleName(tr("显示信息设置"));
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void MaintainProgressFmtCfgDlg::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void MaintainProgressFmtCfgDlg::InitStrResource()
{
	
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void MaintainProgressFmtCfgDlg::InitConnect()
{
    // 连接信号槽
    // 确定按钮被点击
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnOkBtnClicked()));

    // 取消按钮被点击
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void MaintainProgressFmtCfgDlg::InitChildCtrl()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    MaintainShowSet mss;
    if (!DictionaryQueryManager::GetMaintainShowConfig(mss))
    {
        ULOG(LOG_ERROR, "Failed to get maintain show config.");
        return;
    }

	// 更新界面(true:项目数，false:百分比)
	m_mssTemp = mss;
	ui->ItemCntRBtn->setChecked(mss.bEnableShowInfoProNum);
	ui->PercentRBtn->setChecked(!mss.bEnableShowInfoProNum);
}

///
/// @brief
///     确定按钮槽函数
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月18日，新建函数
///
void MaintainProgressFmtCfgDlg::OnOkBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 是否更新
	if (m_mssTemp.bEnableShowInfoProNum == ui->ItemCntRBtn->isChecked())
	{
		this->accept();
	}

	// 保存配置
	MaintainShowSet mss;
	mss.bEnableShowInfoProNum = ui->ItemCntRBtn->isChecked();
    DictionaryQueryManager::SaveMaintainShowConfig(mss);

	// 通知更新
	POST_MESSAGE(MSG_ID_MAINTAIN_SHOW_SET_UPDATE, mss.bEnableShowInfoProNum);

	// 退出
	this->accept();
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void MaintainProgressFmtCfgDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}
