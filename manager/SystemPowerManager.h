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
/// @file     SystemPowerManager.h
/// @brief    系统开关机管理
///
/// @author   8580/GongZhiQiang
/// @date     2024年3月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <mutex>
#include <set>
#include <QObject>
#include <QTimer>
#include <boost/noncopyable.hpp>
#include "src/leonis/thrift/DcsControlProxy.h"
using namespace std;

// 关机模式
enum PowerOffMode
{
	INVALID_MODE,			// 不合法模式
	LOGIN_MODE,				// 登录界面退出模式
	MAIN_MODE			    // 主界面退出模式
};

struct MaintainAbout
{
    tf::MaintainGroupType::type doType = tf::MaintainGroupType::MAINTAIN_GROUP_STOP;
    bool alreadyRecord = false;
};

// 设备监听记录
struct DevRecordInfo
{
	std::map<std::string, MaintainAbout>	mapDevsRecord;			// 运行记录<devSN, 是否记录过（防止多次记录）>
	int										finishedCount = 0;		// 完成项记录
	bool									finishedFlag = false;   // 任务完成标识	
	std::vector<std::string>				vecFailedDevSnList;		// 失败仪器sn

	// 初始化
    void init();

    // 通过设备SN获取对应的维护组
    inline tf::MaintainGroupType::type GetMaintainGroupType(const std::string& devSN)
    {
        std::map<std::string, MaintainAbout>::iterator it = mapDevsRecord.find(devSN);
        return (it == mapDevsRecord.end() ? tf::MaintainGroupType::MAINTAIN_GROUP_STOP : it->second.doType);
    }

    // 修改指定设备的关机维护类型
    void UpdateMaintainType(const std::string& devSN, ::tf::MaintainGroupType::type maintainType);

    // 如果记录为空，则更新其标记
    inline void UpdateFinishedFlag()
    {
        if (mapDevsRecord.empty()){ finishedFlag = true; }
    }

    inline bool IsAllFinished() { return (finishedCount == mapDevsRecord.size()); }

	// 添加设备
	inline void addDev(const std::string& devSn) { mapDevsRecord.insert({ devSn, MaintainAbout() }); }

    // 是否包含目标设备
    inline bool isContains(const std::string& devSn){ return mapDevsRecord.find(devSn) != mapDevsRecord.end(); }

    // 目标设备是否已经被记录
    inline bool isRecord(const std::string& devSn)
    {
        return (mapDevsRecord.find(devSn) == mapDevsRecord.end()) ? false : mapDevsRecord[devSn].alreadyRecord;
    }

    // 更新对应设备记录的标记并finishedCount++
    void UpdateDeviceFlag(const std::string& devSn);

	// 获取设备关键信息
    std::vector< ::tf::DevicekeyInfo> getDevKeyInfoList();

	// 获取设备snList
    std::vector<std::string> getDevSnList();

    // 获取设备以及对应的关机维护组
    void GetDeviceSnAndMaintainId(std::map<tf::MaintainGroupType::type, std::vector<std::string>>& classfiedDev);
};

///
/// @brief   系统初始化信息管理器
///
class SystemPowerManager : public QObject, public boost::noncopyable
{
	Q_OBJECT

public:
    ///
    /// @brief  获得单例对象
    ///
    /// @return 系统开关机管理单例对象
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年1月31日，新建函数
    ///
    static std::shared_ptr<SystemPowerManager> GetInstance();

	///
	/// @brief 开始退出
	///
	/// @param[in]  mode  退出模式
	///
	/// @return true:启动成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	void StartExit(PowerOffMode mode);

	///
	/// @brief 取消退出
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	void CancelExit();

Q_SIGNALS:

	void ReadyToExit();

	void ExitFailed();

protected:

	///
	/// @brief  构造函数
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年1月31日，新建函数
	///
	SystemPowerManager();

protected Q_SLOTS :
	///
	/// @brief 设备状态更新
	///
	/// @param[in]  deviceInfo  设备状态信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年7月14日，新建函数
	///
	void OnDevOtherStateChange(tf::DeviceInfo deviceInfo);

	///
	/// @brief 监听执行维护的设备状态，防止命令发送不成功的情况
	///
	/// @param[in]  deviceInfo  设备状态信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月5日，新建函数
	///
	void OnDevStandbyStateChange(tf::DeviceInfo deviceInfo);

	///
	/// @brief
	///     维护组阶段更新
	///
	/// @param[in]  lmi        维护结果
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年7月14日，新建函数
	///
	void OnUpdateMaintainGroup(tf::LatestMaintainInfo lmi);

    ///
    /// @bref
    ///		更新指定设备的关机维护类型
    ///
    /// @param[in] devSN 设备SN
    /// @param[in] mantianGroupType 指定的关机维护类型，如：tf::MaintainGroupType::MAINTAIN_GROUP_WEEK
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年7月8日，新建函数
    ///
    void OnUpdateStopMaintainType(QString devSN, int mantianGroupType);

private:
	
	///
	/// @brief 获取全部设备状态
	///
	///
	/// @return true:获取成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
	///
	bool GetAllDevStatus();

	///
	/// @brief 处理非停机或待机仪器
	///
	///
	/// @return true:处理成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
	///
	bool HandOtherStatusDev();

	///
	/// @brief 处理待机仪器
	///
	///
	/// @return true：处理成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
	///
	bool HandStandbyStatusDev();

	///
	/// @brief 处理停机仪器
	///
	///
	/// @return true:处理成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
	///
	bool HandHaltStatusDev();

	///
	/// @brief 退出过程检测
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	void ProcessCheck();

	///
	/// @brief 登陆界面模式处理
	///
	///
	/// @return true:处理成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	bool LoginModeHander();

	///
	/// @brief 主界面模式处理
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	bool MainModeHander();

	///
	/// @brief 维护失败退出确认
	///
	///
	/// @return true：用户同意退出
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	bool MaintainFailedCheck();

	///
	/// @brief 退出程序接口
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	void ExitProgramInterface();

	///
	/// @brief 退出前清理
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	void CleanBeforeQuit();

	///
	/// @brief 退出系统
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
	///
	void QuitSystem(PowerOffMode mode);

	///
	/// @brief 获取关机维护
	///			
    /// @param[in]  maintainType  待获取的维护类型
    /// @param[out] curMaint  获取到的维护组
    ///
	/// @return true：检查成功
	///
	/// @par History:
    /// @li 8580/GongZhiQiang，2024年3月27日，新建函数
	///
	bool GetStopMaintainGroup(tf::MaintainGroupType::type maintainType, ::tf::MaintainGroup& curMaint);

	///
	/// @brief 延迟下电
	///
	/// @param[in]  lstDevSN 设备sn 
	/// @param[in]  sec  延迟时间
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年6月21日，新建函数
	///
	void DelayPowerOff(const std::vector<std::string>& lstDevSN, const int sec);

    // 通过类型获取维护组
    bool GetMaintainGroup(tf::MaintainGroupType::type maintainType, ::tf::MaintainGroup& mGroup);

	///
	/// @brief 等待延迟下电线程
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年7月18日，新建函数
	///
	void WaitstdDelayPowerOffThread();

private:
    static std::shared_ptr<SystemPowerManager>    s_instance;			///< 单例对象
    static std::recursive_mutex					  s_mtx;				///< 保护单例对象的互斥量

	PowerOffMode								  m_powerOffMode;		///< 关机模式
	QTimer*										  m_pWaitTimer;			///< 等待延时定时器
										  
	DevRecordInfo								  m_vecStandbyDevRecord;///< 待机状态仪器
	DevRecordInfo								  m_vecHaltDevRecord;	///< 停机仪器记录
	DevRecordInfo								  m_vecOtherDevRecord;	///< 其他状态仪器

    std::map<std::string, ::tf::MaintainGroupType::type> m_stopMaintainTypes; ///< 记录当前退出时需要执行的维护类型
    std::map<::tf::MaintainGroupType::type, ::tf::MaintainGroup> m_cacheMaintgroup; ///< 缓存对应的维护组

	std::thread*								 m_pDelayPowerOffThread;	///< 延迟下电线程
};
