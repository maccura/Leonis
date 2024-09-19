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
/// @file     C1005SingleDeviceWgt.h
/// @brief    C1005 单机版设备界面头文件
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
    class C1005SingleDeviceWgt;
};

class C1005SingleDeviceWgt : public QBaseDeviceWgt
{
	Q_OBJECT

public:

	///
	/// @brief  构造函数
	///
	/// @param[in]  type	设备类型
	/// @param[in]  parent  父窗口
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月23日，新建函数
	///
	C1005SingleDeviceWgt(const EnumDeviceType &type, QWidget *parent = Q_NULLPTR);

	~C1005SingleDeviceWgt();

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
	/// @param[in]  devSn  设备序列号
	/// @param[in]  time   时间
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月14日，新建函数
	///
	void updateTime(const string& devSn, const int time);

	///
	/// @brief  设置生化单机设备试剂相关的报警已读标志
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年12月13日，新建函数
	///
	void setDeviceReagentAlarmReadFlag();

	///
	/// @brief  更新设备信息(更新生化设备)
	///
	/// @param[in]  sn		设备序列号
	/// @param[in]  name	设备名称
	/// @param[in]  status	CH设备状态
	/// @param[in]  iomStatus	iom设备状态
	/// @param[in]  wasteLiquidEnable  是否含有废液桶
	/// @param[in]  mask	设备是否屏蔽
	/// @param[in]  readed	试剂报警是否已读
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月23日，新建函数
	///
	void updateChDeviceInfo(const string &sn, const string &name, const int status, const int iomStatus,
		const bool wasteLiquidEnable, const bool mask, const bool readed);

	///
	/// @brief  更新设备信息(更新ISE设备)
	///
	/// @param[in]  sn		设备序列号
	/// @param[in]  status	设备状态
	/// @param[in]  mask	设备是否屏蔽
	/// @param[in]  readed	试剂报警是否已读
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月23日，新建函数
	///
	void updateIseDeviceInfo(const string &sn, const int status, const bool mask, const bool readed);

private:

	///
	/// @brief  获取指示灯的颜色(根据CH和ISE设备的组合状态)
	///
	/// @return 灯颜色字符串
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	QString GetLightColorStr() const;

	///
	/// @brief  设置状态+时间文字
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	void SetStatusTimeText();
	
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

protected slots:

	///
	/// @brief  生化设备倒计时自减
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	void OnChTimerOut();

	///
	/// @brief  ISE设备倒计时自减
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	void OnIseTimerOut();

private:
	Ui::C1005SingleDeviceWgt* ui;

public:
	int								m_iomStatus;				//< 生化单机时iom状态
	TWO_DEV_INFO					m_devCh;					//< 设备CH信息
	TWO_DEV_INFO					m_devIse;					//< 设备ISE信息
};
