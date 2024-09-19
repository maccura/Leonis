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
/// @file     TrackDqiRwcIseDeviceWgt.h
/// @brief    轨道DQI RWC 和ISE 设备界面头文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "QBaseDeviceWgt.h"

namespace Ui {
    class TrackDqiRwcIseDeviceWgt;
};


class TrackDqiRwcIseDeviceWgt : public QBaseDeviceWgt
{
	Q_OBJECT

public:
	TrackDqiRwcIseDeviceWgt(const string &sn, const string &name, const EnumDeviceType &type, const int status, 
		const bool wasteLiquidEnable, const bool mask, const bool rgtAlarmReaded, QWidget *parent = Q_NULLPTR);
	~TrackDqiRwcIseDeviceWgt();

	///
	/// @brief  改变设备状态
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月15日，新建函数
	///
	void changeStatus() override;

	///
	/// @brief  更新报警的瓶子
	///
	/// @param[in]  data  报警位置及列表信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	void upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data) override;

	///
	/// @brief  更新剩余时间
	///
	/// @param[in]  time  时间（秒）
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月14日，新建函数
	///
	void updateTime(const unsigned int time) override;

	///
	/// @brief  定时器时间到
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月30日，新建函数
	///
	void OnTimerOut() override;

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
	/// @li 7997/XuXiaoLong，2023年8月11日，新建函数
	///
	bool eventFilter(QObject *watched, QEvent *event) override;

private:

	///
	/// @brief  设置状态+时间文字
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	void SetStatusTimeText();

private:
	Ui::TrackDqiRwcIseDeviceWgt* ui;
};
