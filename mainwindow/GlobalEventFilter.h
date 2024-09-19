﻿/***************************************************************************
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
/// @file     GlobalEventFilter.h
/// @brief    全局事件过滤器头文件
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
#pragma once

#include <QObject>

class QTimer;
class QEvent;
class MainWidget;

class GlobalEventFilter : public QObject
{
	Q_OBJECT

public:
	explicit GlobalEventFilter(MainWidget *mainwindow, QObject *parent = nullptr);
	~GlobalEventFilter();

protected:

	///
	/// @brief  重写事件过滤
	///
	/// @param[in]  watched	事件对象
	/// @param[in]  event   事件
	///
	/// @return true表示处理该事件
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	bool eventFilter(QObject *watched, QEvent *event) override;

protected Q_SLOTS:
	
	///
	/// @brief  定时器时间到
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	void slotTimeout();

	///
	/// @brief  监控页面定时呼出时间更新
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	void slotUpdateMonitorPageTime();

private:
	QTimer*							m_timer = nullptr;			// 监控页面定时呼出定时器
	bool							m_bIsShow = false;			// 监控页面是否定时呼出
	MainWidget*						m_mainWindow = nullptr;		// 主窗口
};

