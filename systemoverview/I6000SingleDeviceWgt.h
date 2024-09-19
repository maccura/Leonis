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
/// @file     I6000SingleDeviceWgt.h
/// @brief    I6000 单机版设备界面头文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年8月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年8月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "QBaseDeviceWgt.h"

namespace Ui {
    class I6000SingleDeviceWgt;
};


class I6000SingleDeviceWgt : public QBaseDeviceWgt
{
	Q_OBJECT

public:

	///
	/// @brief  构造函数
	///
	/// @param[in]  sn		设备序列号
	/// @param[in]  name	设备名称
	/// @param[in]  type	设备类型
	/// @param[in]  status	设备状态
	/// @param[in]  wasteLiquidEnable  是否含有废液桶
	/// @param[in]  mask	设备是否屏蔽
	/// @param[in]  rgtAlarmReaded  true表示试剂余量不足相关报警已读
	/// @param[in]  parent  父窗口
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月23日，新建函数
	///
	I6000SingleDeviceWgt(const string &sn, const string &name, const EnumDeviceType &type, 
		const int status, const bool wasteLiquidEnable, const bool mask, const bool rgtAlarmReaded, QWidget *parent = Q_NULLPTR);
	~I6000SingleDeviceWgt();

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
	/// @brief  更新维护剩余时间
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
	Ui::I6000SingleDeviceWgt* ui;

public:
	int								m_iomStatus;				//< 免疫单机时iom状态
};
