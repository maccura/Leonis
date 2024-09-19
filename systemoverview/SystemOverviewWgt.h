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
/// @file     SystemOverviewWgt.h
/// @brief    系统总览界面头文件
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

#include <QMap>
#include <QDialog>
#include <QDateTime>
#include "src/thrift/gen-cpp/defs_types.h"
#include "manager/WarnSetManager.h"

using namespace std;

class QBaseDeviceWgt;
class WarningSetWgt;
class QLabel;
class C1005SingleDeviceWgt;
struct BaseSet;

namespace Ui {
    class SystemOverviewWgt;
};

class SystemOverviewWgt : public QDialog
{
	Q_OBJECT

public:
	SystemOverviewWgt(QWidget *parent = Q_NULLPTR);
	~SystemOverviewWgt();

	///
	/// @brief  更新当前系统显示时间
	///
	/// @param[in]  time  当前时间
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月20日，新建函数
	///
	void updateCurrentTime(const QDateTime &time);

	///
	/// @brief  更新报警按钮
	///
	/// @param[in]  flicker  true表示闪烁
	/// @param[in]  level	 报警等级
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	void updateWarningBtn(const bool flicker,const int level);

	///
	/// @brief  更新LIS连接状态
	///
	/// @param[in]  isConnect  是否连接
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年9月8日，新建函数
	///
	void updateLisConnection(const bool isConnect);

	///
	/// @brief  更新打印机连接状态
	///
	/// @param[in]  isConnect  是否连接
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月5日，新建函数
	///
	void updatePrintConnection(const bool isConnect);

private:
	
	///
	/// @brief  初始化UI界面
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月12日，新建函数
	///
	void initUI();

	///
	/// @brief  初始化信号与槽连接
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月12日，新建函数
	///
	void initConnect();

	///
	/// @brief  初始化所有设备
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月12日，新建函数
	///
	void initDevices();

	///
	/// @brief  初始化所有样本
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	void initSamples();

	///
	/// @brief  根据设备序列号获取指定设备
	///
	/// @param[in]  sn  设备序列号
	///
	/// @return 指定设备智能指针
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月18日，新建函数
	///
	shared_ptr<QBaseDeviceWgt> getDeviceBySn(const string &sn);

	///
	/// @brief  更新所有设备的显示位置
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年9月7日，新建函数
	///
	void updateDevicesPos();

	///
	/// @brief  设置左右按钮显隐
	///
	/// @param[in]  show  true表示显示
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月9日，新建函数
	///
	void setLeftRightVisible(const bool show);

	///
	/// @brief  获取试剂耗材余量不足已读标志
	///
	/// @param[in]  devSN  设备序列号
	///
	/// @return true表示已读
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月1日，新建函数
	///
	bool GetRgtAlarmReadFlag(const string& devSN);

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

    ///
    /// @brief
    ///     显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年6月21日，新建函数
    ///
    void showEvent(QShowEvent *event) override;

    ///
    /// @brief
    ///     隐藏事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年6月21日，新建函数
    ///
    void hideEvent(QHideEvent *event) override;

Q_SIGNALS:
	/// @brief 显示数据浏览界面信号
	void signalShowDataBrowse();

	/// @brief 开始信号
	void signalStart();

	/// @brief 暂停信号
	void signalSuspend();

	/// @brief 结束信号
	void signalStop();

	/// @brief 显示报警界面信号
	void signalWarning();

	/// @brief 显示样本架监视界面信号
	void signalSampleRackMonitor();

	///
	/// @brief  显示试剂/耗材界面信号
	///
	/// @param[in]  data  设备序列号
	///
	void signalShowReagentPage(const QString &devSn);

	/// @brief 显示维护界面信号
	void signalMaintain();

protected Q_SLOTS:

	///
	/// @brief  点击设置按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月13日，新建函数
	///
	void slotClickSet();

	///
	/// @brief  更新样本信息
	///
	/// @param[in]  enUpdateType  更新类型
	/// @param[in]  lstSIs		  更新样本列表 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	void OnSampleUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> lstSIs);

	///
	/// @brief  更新报警的瓶子
	///   
	/// @param[in]  infos  更新的信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	void slotUpadteBottles(const QMap<QString, QMap<EnumBottlePos, EnumWarnLevel>> &infos);

	///
	/// @brief  更新设备屏蔽状态
	///
	/// @param[in]  groupName	设备组母
	/// @param[in]  devSn		设备序列号
	/// @param[in]  masked		true表示屏蔽
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年12月12日，新建函数
	///
	void slotUpadteDeviceMaskStatus(const QString& groupName, const QString& devSn, bool masked);

	///
	/// @brief	更新时间显示格式
	///
	/// @param[in]  ds  显示设置信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年12月13日，新建函数
	///
	void OnDateTimeFormatUpdate(const BaseSet& ds);

	///
	/// @brief  设备倒计时刷新
	///
	/// @param[in]  totalSeconds  总的倒计时时间
	/// @param[in]  sn2seconds	其他设备的倒计时时间
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年2月28日，新建函数
	///
	void OnDeviceCountDownUpdate(int totalSeconds, const QMap<QString, int> &sn2seconds);

	///
	/// @brief  设备状态改变
	///
	/// @param[in]  deviceInfo  设备信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月12日，新建函数
	///
	void OnDevStateChange(tf::DeviceInfo deviceInfo);

    ///
    /// @bref	权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月30日，新建函数
    ///
    void OnPermisionChanged();

	///
	/// @brief  试剂耗材余量不足报警已读标志更新
	///
	/// @param[in]  devSN  设备序列号
	/// @param[in]  readed  true表示已读
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月1日，新建函数
	///
	void OnRgtAlarmReadFlagChange(QString devSN, bool readed);

	///
	/// @bref	定时器时间到
	///
	/// @par History:
	/// @li 8276/huchunli, 2024年5月30日，新建函数
	///
	void OnTimerOut();

private:
	Ui::SystemOverviewWgt* ui;

	vector<shared_ptr<QBaseDeviceWgt>>m_vecDevices;				//< 仪器列表（按数据库查出来的顺序排列）
	WarningSetWgt*					m_warnSetWgt = nullptr;		//< 报警设置弹出
	QMap<int64_t, tf::SampleInfo>	m_mapSamples;				//< 所有样本信息
	uint							m_toTestSampleCount = 0;	//< 待测样本数量
	QLabel*							m_computerDevice = nullptr;	//< 电脑设备
	QString							m_strDateTimeFormat;		//< 时间显示格式
	QDateTime						m_date;						//< 缓存当前时间
	shared_ptr<C1005SingleDeviceWgt>m_spChSingleDev = nullptr;	//< 生化单机版设备指针
	QTimer*							m_timer = nullptr;			//< 倒计时定时器
	int								m_currTotalTime = 0;		//< 当前倒计时总时长
};
