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
/// @file     GlobalEventFilter.cpp
/// @brief    全局事件过滤器源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include <QTimer>
#include <QEvent>
#include "GlobalEventFilter.h"
#include "mainwidget.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/DictionaryQueryManager.h"
#include "src/public/ConfigDefine.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"

GlobalEventFilter::GlobalEventFilter(MainWidget *mainwindow,QObject *parent /*= nullptr*/)
	: m_mainWindow(mainwindow)
{
	// 监控页面定时呼出初始化
	m_timer = new QTimer(this);

	// 监控页面定时呼出
	connect(m_timer, &QTimer::timeout, this, &GlobalEventFilter::slotTimeout);

	// 监控页面定时呼出设置更新
	REGISTER_HANDLER(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE, this, slotUpdateMonitorPageTime);

	// 登录成功才进行初始化
	REGISTER_HANDLER(MSG_ID_LOGIN_SUCCESSED, this, slotUpdateMonitorPageTime);
}

GlobalEventFilter::~GlobalEventFilter()
{

}

bool GlobalEventFilter::eventFilter(QObject *watched, QEvent *event)
{
	// 键盘按下、鼠标按下
	if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress)
	{
		if (m_timer && m_bIsShow)
		{
			// 重新开始计时
			m_timer->start();
		}
	}

	return QObject::eventFilter(watched, event);
}

void GlobalEventFilter::slotTimeout()
{
	if (m_bIsShow == false || m_mainWindow == nullptr)
	{
		ULOG(LOG_ERROR, "m_bIsShow == false || m_mainWindow == nullptr");
		return;
	}
	m_mainWindow->showSystemOverviewWidget();
	m_timer->stop();
}

void GlobalEventFilter::slotUpdateMonitorPageTime()
{
	if (m_timer == nullptr)
	{
		ULOG(LOG_ERROR, "m_timer is nullptr");
		return;
	}

    PageSet workSet;
    if (!DictionaryQueryManager::GetPageset(workSet))
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return;
    }

	m_timer->stop();
	m_bIsShow = workSet.callTime.first;

	// 如果时间无效 那么不显示
	if (workSet.callTime.second <= 0)
	{
		m_bIsShow = false;
	}

	// 定时呼出并且时间有效
	if (m_bIsShow)
	{
		m_timer->setInterval(workSet.callTime.second * 60 * 1000);
		m_timer->start();
	}
}
