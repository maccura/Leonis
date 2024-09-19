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
/// @file     QBaseDeviceWgt.h
/// @brief    各仪器基类头文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QMoveEvent>
#include <QTimer>
#include "src/thrift/gen-cpp/defs_types.h"
#include "manager/WarnSetManager.h"
#include "MacroDefine.h"

using namespace std;

// 设备类型
enum EnumDeviceType
{
	EDT_UNKNOWN = 0,	//< 未知
	EDT_TRACK_IOM,		//< 轨道IOM
	EDT_TRACK_DQI,		//< 轨道DQI
	EDT_TRACK_RWC,		//< 轨道RWC
	EDT_C1005_1000,		//< c1005 1000速
	EDT_C1005_2000,		//< c1005 2000速
	EDT_C1005_SINGLE,	//< c1005 单机版
	EDT_I6000,			//< i6000
	EDT_I6000_SINGLE,	//< i6000 单机版
	EDT_ISE1005			//< ise1005
};

// 含有2台设备的设备信息(生化单机含ISE、C2000)
struct TWO_DEV_INFO
{
	string						devSN;					//< 设备序列号
	int							status = 0;				//< 设备状态
	bool						mask = false;			//< 设备屏蔽状态
	bool						rgtAlarmReaded = true;	//< 试剂相关报警已读
	int							time = 0;				//< 上一次倒计时时间
	QTimer						timer;					//< 倒计时定时器
};

class QBaseDeviceWgt : public QWidget
{
	Q_OBJECT

public:
	
	///
	/// @brief  构造函数
	///
	/// @param[in]  sn  设备序列号
	/// @param[in]  name  设备名称
	/// @param[in]  type  设备类型
	/// @param[in]  status  设备状态
	/// @param[in]  haveWasteLiquid  是否含有废液桶
	/// @param[in]  mask  是否屏蔽
	/// @param[in]  rgtAlarmReaded  试剂耗材余量不足的报警已读
	/// @param[in]  parent  父类指针
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	QBaseDeviceWgt(const string &sn, const string &name, const EnumDeviceType &type, const int status,
		const bool haveWasteLiquid, const bool mask, const bool rgtAlarmReaded, QWidget *parent = nullptr);

	///
	/// @brief  构造函数
	///
	/// @param[in]  type  设备类型
	/// @param[in]  parent  父类指针
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	QBaseDeviceWgt(const EnumDeviceType &type, QWidget *parent = nullptr);

	///
	/// @brief  析构函数
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	~QBaseDeviceWgt();

	///
	/// @brief  初始化定时器
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月30日，新建函数
	///
	void initTimer();

	///
	/// @brief  改变设备状态
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月16日，新建函数
	///
	virtual void changeStatus();

	///
	/// @brief  更新检测剩余时间
	///
	/// @param[in]  time  时间（秒）
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月16日，新建函数
	///
	virtual void updateTime(const unsigned int time);

	///
	/// @brief  更新报警的瓶子
	///
	/// @param[in]  data  报警位置及列表信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	virtual void upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data);

	///
	/// @brief  定时器时间到
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月30日，新建函数
	///
	virtual void OnTimerOut();

	///
	/// @brief  获取设备状态信息
	///
	/// @param[in]  longLight true表示长灯类型
	///
	/// @return (设备状态文字，设备灯图片路径，是否显示倒计时)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月31日，新建函数
	///
	tuple<QString, QString, bool> GetDeviceStatusInfo(const bool longLight);

	///
	/// @brief  获取IOM设备状态文字
	///
	/// @param[in]  status  iom设备状态
	///
	/// @return 状态文字
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月31日，新建函数
	///
	QString GetIomStatusText(const int status);

	///
	/// @brief  获取状态文字
	///
	/// @param[in]  status  设备状态
	/// @param[in]  mask  是否屏蔽
	///
	/// @return (是否显示时间，状态文字)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	pair<bool, QString> GetStatusText(const int status, const bool mask);

	///
	/// @brief  获取时间文字
	///
	/// @param[in]  time  时间（单位：秒）
	///
	/// @return 显示时间字符串
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	QString GetTimeText(const int timeS);

Q_SIGNALS:
	///
	/// @brief  显示试剂/耗材界面信号
	///
	/// @param[in]  data  设备序列号
	///
	void signalShowReagentPage(const QString &devSn);

	/// @brief 显示样本架监视界面信号
	void signalSampleRackMonitor();

	/// @brief 显示维护界面
	void signalMaintain();

public:
	string							m_devSN;					//< 设备序列号
	string							m_devName;					//< 设备名称
	EnumDeviceType					m_devType;					//< 设备类型
	int								m_devStatus;				//< 设备状态（参见::tf::DeviceWorkState）
	bool							m_haveWasteLiquid = false;	//< 是否包含废液桶
	bool							m_mask = false;				//< 设备是否屏蔽
	QTimer*							m_timer = nullptr;			//< 用于倒计时按秒刷新的定时器
	int								m_currTime = -1;			//< 设备当前用于显示的时间（单位：秒）
	bool							m_rgtAlarmReaded = true;	//< 试剂余量不足相关的报警已读标志（灯要显示为黄色）
};
