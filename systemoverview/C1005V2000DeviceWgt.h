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
/// @file     C1005V2000DeviceWgt.h
/// @brief    C1005 2000速设备界面头文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "QBaseDeviceWgt.h"

namespace Ui {
    class C1005V2000DeviceWgt;
};
class QLabel;

class C1005V2000DeviceWgt : public QBaseDeviceWgt
{
	Q_OBJECT

public:

	///
	/// @brief  构造函数
	///
	/// @param[in]  sn		设备序列号
	/// @param[in]  name	设备名称
	/// @param[in]  type	设备类型
	/// @param[in]  wasteLiquidEnable  是否含有废液桶
	/// @param[in]  parent  父窗口
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月23日，新建函数
	///
	C1005V2000DeviceWgt(const string &sn, const string &name, const EnumDeviceType &type, const bool wasteLiquidEnable, QWidget *parent = Q_NULLPTR);
	~C1005V2000DeviceWgt();

	///
	/// @brief  更新设备信息
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  devName  设备名称
	/// @param[in]  status  设备状态
	/// @param[in]  mask  是否屏蔽
	/// @param[in]  readed  试剂耗材余量不足的报警已读
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	void UpdateDevInfo(const string &devSn, const string &devName, const int status, const bool mask, const bool readed);

	///
	/// @brief  改变设备状态
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月15日，新建函数
	///
	void changeStatus() override;

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
	/// @brief  更新生化2000速报警的瓶子
	///
	/// @param[in]  data   报警位置及其报警等级
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	void upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data) override;

	///
	/// @brief  根据第二个设备和第一个设备判断是否包含废液桶
	///
	/// @param[in]  enable  第二个设备是否包含废液桶
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月23日，新建函数
	///
	void setWasteLiquidVisible(const bool enable);

	///
	/// @brief  设置2000速设备试剂相关的报警已读标志
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年12月13日，新建函数
	///
	void setDeviceReagentAlarmReadFlag();

private:

	///
	/// @brief  获取指示灯的颜色(根据CHA和CHB的组合状态)
	///
	/// @return 灯颜色字符串
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	QString GetLightColorStr() const;

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
	/// @brief  设备A倒计时自减
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	void OnChATimerOut();

	///
	/// @brief  设备B倒计时自减
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	void OnChBTimerOut();

private:
	Ui::C1005V2000DeviceWgt* ui;

public:
	TWO_DEV_INFO						m_devA;						//< 设备A信息
	TWO_DEV_INFO						m_devB;						//< 设备B信息
};
