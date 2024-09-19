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
/// @file     SystemPowerManager.cpp
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
#include "SystemPowerManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "src/public/DictionaryKeyName.h"
#include "src/leonis/thrift/DcsControlProxy.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "utility/maintaindatamng.h"

#include <QProcess>
#include <QApplication>
#include <QRegularExpression>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#define  SHUT_DOWN_MAX_SECOND                               (30)          // 关机倒计时
#define  POWER_OFF_MAX_SECOND                               (60)          // 下电倒计时


void TransToDevKeyInfo(const std::vector<std::string>& devSn, std::vector<::tf::DevicekeyInfo>& devKeys)
{
    for (const std::string& strSn : devSn)
    {
        ::tf::DevicekeyInfo devTemp;
        devTemp.__set_sn(strSn);
        devKeys.push_back(devTemp);
    }
}

///														 
/// @brief 初始化单例对象
///
std::shared_ptr<SystemPowerManager>			SystemPowerManager::s_instance = nullptr;
std::recursive_mutex						SystemPowerManager::s_mtx;

SystemPowerManager::SystemPowerManager():
	m_powerOffMode(PowerOffMode::INVALID_MODE)
	, m_pWaitTimer(nullptr)
	, m_pDelayPowerOffThread{nullptr}
{
    // 监听关机维护类型变化
    REGISTER_HANDLER(MSG_ID_UPDATE_STOP_MAINTAIN_TYPE, this, OnUpdateStopMaintainType);
}

std::shared_ptr<SystemPowerManager> SystemPowerManager::GetInstance()
{
    if (nullptr == s_instance)
    {
        std::unique_lock<std::recursive_mutex> autoLock(s_mtx);
        if (nullptr == s_instance)
        {
            s_instance.reset(new SystemPowerManager);
        }
    }

    return s_instance;
}

///
/// @brief 取消退出
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
void SystemPowerManager::CancelExit()
{
	CleanBeforeQuit();
}

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
void SystemPowerManager::StartExit(PowerOffMode mode)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, mode);

	// 正在运行模式判断
	if (m_powerOffMode != PowerOffMode::INVALID_MODE)
	{
		TipDlg(tr("退出程序"), tr("正在退出，请稍后!")).exec();
		return;
	}

	// 设置模式
	m_powerOffMode = mode;

	// 获取全部设备状态
	if(!GetAllDevStatus())
	{
		ULOG(LOG_ERROR, "GetAllDevStatus Failed !");
		TipDlg(tr("退出程序"), tr("退出程序失败！")).exec();
		return;
	}
    m_cacheMaintgroup.clear();

	// 各自模式去处理
	bool ret = false;
	switch (m_powerOffMode)
	{
	case LOGIN_MODE:
		ret = LoginModeHander();
		break;
	case MAIN_MODE:
		ret = MainModeHander();
		break;
	default:
		break;
	}

	// 退出失败提示
	if (!ret)
	{
		TipDlg(tr("退出程序"), tr("退出程序失败！")).exec();
	}
}


///
/// @brief 获取全部设备状态
///
///
/// @return true:获取成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
///
bool SystemPowerManager::GetAllDevStatus()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空缓存
	m_vecHaltDevRecord.init();
	m_vecStandbyDevRecord.init();
	m_vecOtherDevRecord.init();

	// 获取全部设备信息
	const auto& devs = CommonInformationManager::GetInstance()->GetDeviceMaps();
	for (const auto& dev : devs)
	{
		// 过滤不合法设备（轨道会在dcs自动退出，轨道需要执行关机维护）
		// 过滤掉单机版本的虚拟轨道
		// 过滤断开连接状态
		if ((dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_INVALID) || 
			(dev.second->status == tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT) || 
			(dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK && !DictionaryQueryManager::GetInstance()->GetPipeLine()))
		{
			continue;
		}

		// 待机状态
		if (dev.second->status == tf::DeviceWorkState::DEVICE_STATUS_STANDBY)
		{
			m_vecStandbyDevRecord.addDev(dev.first);
		}
		// 停机
		else if (dev.second->status == tf::DeviceWorkState::DEVICE_STATUS_HALT)
		{
			// 对于主界面退出，停机状态也需要做关机维护（第一项必须是仪器复位）
			// 登录界面退出则不作关机维护，直接退出
			if (m_powerOffMode == MAIN_MODE)
			{
				m_vecStandbyDevRecord.addDev(dev.first);
			}
			else
			{
				m_vecHaltDevRecord.addDev(dev.first);
			}		
		}
		// 其他状态
		else
		{
			m_vecOtherDevRecord.addDev(dev.first);
		}		
	}

	// 设置标识，为空则完成
    m_vecStandbyDevRecord.UpdateFinishedFlag();
    m_vecHaltDevRecord.UpdateFinishedFlag();
    m_vecOtherDevRecord.UpdateFinishedFlag();

    // 更新结束维护类型
    for (auto& smt : m_stopMaintainTypes)
    {
        m_vecStandbyDevRecord.UpdateMaintainType(smt.first, smt.second);
        m_vecHaltDevRecord.UpdateMaintainType(smt.first, smt.second);
        m_vecOtherDevRecord.UpdateMaintainType(smt.first, smt.second);
    }
	return true;
}

///
/// @brief 登陆界面模式处理
///
///
/// @return true:处理成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
bool SystemPowerManager::LoginModeHander()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 有其他状态的仪器再次确认
	if (!m_vecOtherDevRecord.mapDevsRecord.empty())
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("有仪器正在运行，是否退出?"), TipDlgType::TWO_BUTTON));
		if (pTipDlg->exec() == QDialog::Rejected)
		{
			// 取消退出，不执行后续
			return true;
		}
	}

	// 停机仪器：下电
	if (!HandHaltStatusDev())
	{
		ULOG(LOG_ERROR, "Execute HandHaltStatusDev Failed!");
		return false;
	}

	// 运行仪器：停机->下电
	if (!HandOtherStatusDev())
	{
		ULOG(LOG_ERROR, "Execute HandOtherStatusDev Failed!");
		return false;
	}

	// 待机仪器：关机维护->下电
	if (!HandStandbyStatusDev())
	{
		ULOG(LOG_ERROR, "Execute HandStandbyStatusDev Failed!");
		return false;
	}

	ProcessCheck();
	return true;
}

///
/// @brief 主界面模式处理
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
bool SystemPowerManager::MainModeHander()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 有其他状态的仪器不能退出
	if (!m_vecOtherDevRecord.mapDevsRecord.empty())
	{
		TipDlg(tr("退出程序"), tr("只能全部仪器在待机或者停止状态下才可退出软件!")).exec();
		CleanBeforeQuit();
		return true;
	}
	
	//// 停机仪器：下电
	//if (!HandHaltStatusDev())
	//{
	//	ULOG(LOG_ERROR, "Execute HandHaltStatusDev Failed!");
	//	return false;
	//}

	// 待机仪器：关机维护
	if (!HandStandbyStatusDev())
	{
		ULOG(LOG_ERROR, "Execute HandStandbyStatusDev Failed!");
		return false;
	}

	ProcessCheck();
	return true;
}

///
/// @brief 处理非停机或待机仪器
///
///
/// @return true:处理成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
///
bool SystemPowerManager::HandOtherStatusDev()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_vecOtherDevRecord.finishedFlag)
	{
		return true;
	}

	// 监听设备状态改变
	REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevOtherStateChange);
	if (!DcsControlProxy::GetInstance()->StopTest())
	{
		ULOG(LOG_ERROR, "Execute Stop Instrument Failed!");
		return false;
	}

	return true;
}

///
/// @brief 处理待机仪器
///
///
/// @return true：处理成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
///
bool SystemPowerManager::HandStandbyStatusDev()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_vecStandbyDevRecord.finishedFlag)
	{
		return true;
	}

    std::map<tf::MaintainGroupType::type, std::vector<std::string>> classfiedDev;
    m_vecStandbyDevRecord.GetDeviceSnAndMaintainId(classfiedDev);

    // 监听关机维护项状态更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_UPDATE, this, OnUpdateMaintainGroup);

	// 监听维护设备状态
	REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStandbyStateChange);

    for (auto it = classfiedDev.begin(); it != classfiedDev.end(); ++it)
    {
        // 获取关机维护
        ::tf::MaintainGroup curMaintain;
        if (!GetStopMaintainGroup(it->first, curMaintain))
        {
            continue;
        }

        tf::MaintainExeParams exeParams;
        exeParams.__set_groupId(curMaintain.id);

        std::vector<::tf::DevicekeyInfo> devKeys;
        TransToDevKeyInfo(it->second, devKeys);
        exeParams.__set_lstDev(devKeys);
        if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
        {
            ULOG(LOG_ERROR, "Execute Shutdown Maintain Failed!");
            return false;
        }
    }

	return true;
}

///
/// @brief 处理停机仪器
///
///
/// @return true:处理成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
///
bool SystemPowerManager::HandHaltStatusDev()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	
	// 完成标识
	if (m_vecHaltDevRecord.finishedFlag)
	{
		return true;
	}

	// 这里只是消息发送的结果，并不是真正开机或关机的结果。
	if (!DcsControlProxy::GetInstance()->DevicePowerOff(m_vecHaltDevRecord.getDevSnList()))
	{
		return false;
	}

	// 完成标识
	m_vecHaltDevRecord.finishedFlag = true;

	return true;
}

///
/// @brief 退出过程检测
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
void SystemPowerManager::ProcessCheck()
{
	// 有未完成的任务则返回
	if (!m_vecOtherDevRecord.finishedFlag ||
		!m_vecStandbyDevRecord.finishedFlag ||
		!m_vecHaltDevRecord.finishedFlag)
	{
		return;
	}

	// 兼顾免疫设置等待定时器
	if (m_pWaitTimer == nullptr)
	{
		m_pWaitTimer = new QTimer(this);
	}

	connect(m_pWaitTimer, &QTimer::timeout, this, [&] {
		// 退出接口直接退出
		ExitProgramInterface();
		m_pWaitTimer->stop();
	});
	m_pWaitTimer->start(3000);
}

///
/// @brief 维护失败退出确认
///
///
/// @return true：用户同意退出
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
bool SystemPowerManager::MaintainFailedCheck()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_powerOffMode == LOGIN_MODE)
	{
		TipDlg(tr("提示"), tr("存在维护失败的仪器!")).exec();
	}
	else if (m_powerOffMode == MAIN_MODE)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("存在维护失败的仪器，是否继续退出？"), TipDlgType::TWO_BUTTON));
		if (pTipDlg->exec() == QDialog::Rejected)
		{
			return false;
		}
	}
	
	return true;
}

///
/// @brief 退出前清理
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
void SystemPowerManager::CleanBeforeQuit()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 退出前清理
    m_powerOffMode = INVALID_MODE;
    //m_stopMaintainTypes.clear(); // 周维护失败，用户选中没有退出时，再次退出，不能清空周维护

	// 清空缓存
	m_vecHaltDevRecord.init();
	m_vecStandbyDevRecord.init();
    m_vecOtherDevRecord.init();

	// 等待延时定时器停止
	if (m_pWaitTimer != nullptr)
	{
		m_pWaitTimer->stop();
    }
	
	// 取消监听
	UNREGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_UPDATE, this, OnUpdateMaintainGroup);
	UNREGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStandbyStateChange);
	UNREGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevOtherStateChange);
}

///
/// @brief 退出程序接口
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
void SystemPowerManager::ExitProgramInterface()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 临时记录模式
	auto tempMode = m_powerOffMode;

	// 等待线程结束
	WaitstdDelayPowerOffThread();

	// 退出前清理
	CleanBeforeQuit();

	// 退出系统
	QuitSystem(tempMode);

}

///
/// @brief 退出系统
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月28日，新建函数
///
void SystemPowerManager::QuitSystem(PowerOffMode mode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 准备退出
	emit ReadyToExit();

	// 主界面退出需要判断权限
	if (mode == MAIN_MODE)
	{
		auto currentLoginUser = UserInfoManager::GetInstance()->GetLoginUserInfo();
		if (currentLoginUser == nullptr)
		{
			ULOG(LOG_ERROR, "GetLoginUserInfo Failed !");
			return;
		}

		// 如果是权限小于工程师用户，需要关闭操作系统
		if (currentLoginUser != nullptr && currentLoginUser->type < tf::UserType::USER_TYPE_ENGINEER)
		{
			QProcess::startDetached("cmd.exe", QStringList() << "/c" << "shutdown -s -t 60");
		}
	}
	
	//退出程序
	QApplication::quit();
}

bool SystemPowerManager::GetMaintainGroup(tf::MaintainGroupType::type maintainType, ::tf::MaintainGroup& mGroup)
{
    std::map<::tf::MaintainGroupType::type, ::tf::MaintainGroup>::iterator it = m_cacheMaintgroup.find(maintainType);
    if (it != m_cacheMaintgroup.end())
    {
        mGroup = it->second;
        return true;
    }

    auto DocInstance = DcsControlProxy::GetInstance();
    if (DocInstance == nullptr)
    {
        ULOG(LOG_ERROR, "Null dcscontrolproxy.");
        return false;
    }

    // 查找关机维护
    tf::MaintainGroupQueryCond grpQryCond;
    grpQryCond.__set_groupType(maintainType);

    tf::MaintainGroupQueryResp grpQryResp;
    if (!DocInstance->QueryMaintainGroup(grpQryResp, grpQryCond) ||
        (grpQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS) || grpQryResp.lstMaintainGroups.size() != 1)
    {
        ULOG(LOG_ERROR, "QueryMaintainGroup failed.");
        return false;
    }

    mGroup = grpQryResp.lstMaintainGroups[0];
    m_cacheMaintgroup.insert(std::pair<::tf::MaintainGroupType::type, ::tf::MaintainGroup>(maintainType, mGroup));

    return true;
}

///
/// @brief 等待延迟下电线程
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年7月18日，新建函数
///
void SystemPowerManager::WaitstdDelayPowerOffThread()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (nullptr != m_pDelayPowerOffThread && m_pDelayPowerOffThread->joinable())
	{
		ULOG(LOG_INFO, "Waiting delay power off thread.");
		m_pDelayPowerOffThread->join();
	}
	ULOG(LOG_INFO, "Exit Wait delay power off thread.");
}

///
/// @brief 获取关机维护
///			
///
/// @return true：检查成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月27日，新建函数
///
bool SystemPowerManager::GetStopMaintainGroup(tf::MaintainGroupType::type maintainType, ::tf::MaintainGroup& curMaint)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, maintainType);

    ::tf::MaintainGroup stopGroup;
    if (!GetMaintainGroup(maintainType, stopGroup))
    {
        return false;
    }
    if (stopGroup.items.empty())
    {
        ULOG(LOG_WARN, "Maintain group items is empty.");
        return true;
    }
    // 第一项必须是仪器复位
    if (stopGroup.items.front().itemType != tf::MaintainItemType::MAINTAIN_ITEM_RESET)
    {
        ULOG(LOG_ERROR, "The first item of group:%d is not MAINTAIN_ITEM_RESET!", maintainType);
        return false;
    }

    curMaint = stopGroup;

	return true;
}

///
/// @brief 设备状态更新
///
/// @param[in]  deviceInfo  设备状态信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月14日，新建函数
///
void SystemPowerManager::OnDevOtherStateChange(tf::DeviceInfo deviceInfo)
{
	// 避免多次进入
	if (m_vecOtherDevRecord.finishedFlag)
	{
		return;
	}

	// 包含该仪器，并且没有记录过
	if (m_vecOtherDevRecord.isContains(deviceInfo.deviceSN) && !m_vecOtherDevRecord.isRecord(deviceInfo.deviceSN))
	{
		// 正常停止为待机状态，异常停止为停止状态
		if (deviceInfo.__isset.status && 
			(deviceInfo.status == tf::DeviceWorkState::DEVICE_STATUS_HALT || deviceInfo.status == tf::DeviceWorkState::DEVICE_STATUS_STANDBY))
		{
            m_vecOtherDevRecord.UpdateDeviceFlag(deviceInfo.deviceSN);
		}
	}

	// 完成判断
	if (m_vecOtherDevRecord.IsAllFinished())
	{
		// 取消监听
		UNREGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevOtherStateChange);

		// 完成标识
		m_vecOtherDevRecord.finishedFlag = true;

		// 强下电
		DcsControlProxy::GetInstance()->DevicePowerOff(m_vecOtherDevRecord.getDevSnList());

		// 执行下一步
		ProcessCheck();
	}

}

///
/// @brief 监听执行维护的设备状态，防止命令发送不成功的情况
///
/// @param[in]  deviceInfo  设备状态信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年9月5日，新建函数
///
void SystemPowerManager::OnDevStandbyStateChange(tf::DeviceInfo deviceInfo)
{
	// 避免多次进入
	if (m_vecStandbyDevRecord.finishedFlag)
	{
		return;
	}

	// 包含该仪器，并且没有记录过
	if (m_vecStandbyDevRecord.isContains(deviceInfo.deviceSN) && !m_vecStandbyDevRecord.isRecord(deviceInfo.deviceSN))
	{
		// 如果该设备断开连接，则记录
		if (deviceInfo.__isset.status &&
			(deviceInfo.status == tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT))
		{
			m_vecStandbyDevRecord.UpdateDeviceFlag(deviceInfo.deviceSN);
			m_vecStandbyDevRecord.vecFailedDevSnList.push_back(deviceInfo.deviceSN);
		}
	}

	// 完成判断
	if (m_vecStandbyDevRecord.IsAllFinished())
	{
		// 取消监听
		UNREGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStandbyStateChange);

		// 完成标识
		m_vecStandbyDevRecord.finishedFlag = true;

		// 有维护失败询问
		if (!m_vecStandbyDevRecord.vecFailedDevSnList.empty())
		{
			// 返回true就继续，返回true就退出
			if (!MaintainFailedCheck())
			{
				CleanBeforeQuit();
				return;
			}
			// 强下电
			// 兼顾免疫，在维护完成后不能及时切换状态，延时3s下电
			DelayPowerOff(m_vecStandbyDevRecord.vecFailedDevSnList, 3);
		}

		// 执行下一步
		ProcessCheck();

	}
}

///
/// @brief
///     维护组阶段更新
///
/// @param[in]  lmi        维护结果
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月14日，新建函数
///
void SystemPowerManager::OnUpdateMaintainGroup(tf::LatestMaintainInfo lmi)
{
	ULOG(LOG_INFO, "%s(%lld, %d)", __FUNCTION__, lmi.groupId, lmi.exeResult);

	// 避免多次进入
	if (m_vecStandbyDevRecord.finishedFlag)
	{
		return;
	}
    ::tf::MaintainGroupType::type maintainType = m_vecStandbyDevRecord.GetMaintainGroupType(lmi.deviceSN);
    ::tf::MaintainGroup stopGroup;
    if (!GetMaintainGroup(maintainType, stopGroup))
    {
        ULOG(LOG_WARN, "Failed to query maintain group by type:%d.", maintainType);
        return;
    }
    if (stopGroup.id != lmi.groupId)
    {
        ULOG(LOG_WARN, "Invalid groupid, %d != %d.", stopGroup.id, lmi.groupId);
        return;
    }

	// 包含该仪器，并且没有记录过
	if (m_vecStandbyDevRecord.isContains(lmi.deviceSN) && !m_vecStandbyDevRecord.isRecord(lmi.deviceSN))
	{
		if (lmi.exeResult == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
		{
			m_vecStandbyDevRecord.UpdateDeviceFlag(lmi.deviceSN);

			// 判断最后一项是否是自动关机,如果不是则下电
			if (stopGroup.items.back().itemType != tf::MaintainItemType::MAINTAIN_ITEM_AUTO_SHUTDOWN)
			{
				// 兼顾免疫，在维护完成后不能及时切换状态，延时3s下电
				DelayPowerOff({ lmi.deviceSN }, 3);
			}
		}
		else if (lmi.exeResult == tf::MaintainResult::MAINTAIN_RESULT_FAIL)
		{
			m_vecStandbyDevRecord.UpdateDeviceFlag(lmi.deviceSN);
			m_vecStandbyDevRecord.vecFailedDevSnList.push_back(lmi.deviceSN);
		}
	}

	// 完成判断
	if (m_vecStandbyDevRecord.IsAllFinished())
	{
		// 取消监听
        UNREGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_UPDATE, this, OnUpdateMaintainGroup);

		// 完成标识
		m_vecStandbyDevRecord.finishedFlag = true;

		// 有维护失败询问
		if (!m_vecStandbyDevRecord.vecFailedDevSnList.empty())
		{
			// 返回true就继续，返回true就退出
			if (!MaintainFailedCheck())
			{
				CleanBeforeQuit();
				return;
			}
			// 强下电
			// 兼顾免疫，在维护完成后不能及时切换状态，延时3s下电
			DelayPowerOff(m_vecStandbyDevRecord.vecFailedDevSnList, 3);
		}

		// 执行下一步
		ProcessCheck();
	}
}

void SystemPowerManager::OnUpdateStopMaintainType(QString devSN, int mantianGroupType)
{
    ULOG(LOG_INFO, "%s(devSN:%s)", __FUNCTION__, devSN.toStdString());

    std::unique_lock<std::recursive_mutex> autoLock(s_mtx);
    m_stopMaintainTypes[devSN.toStdString()] = (tf::MaintainGroupType::type)mantianGroupType; //tf::MaintainGroupType::MAINTAIN_GROUP_WEEK;
}

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
void SystemPowerManager::DelayPowerOff(const std::vector<std::string>& lstDevSN, const int sec)
{
	ULOG(LOG_INFO, "Delay power off, dev size:%d sec:%d.", lstDevSN.size(), sec);

	// 参数判断
	if (lstDevSN.empty() || sec < 0)
	{
		return;
	}

	// 等待延迟线程结束
	WaitstdDelayPowerOffThread();

	// 兼顾免疫，延迟3s下电
	auto delayPowerOffFunc = [&](std::vector<std::string> offDevSN, int delSec) {
		// 等待时间
		boost::this_thread::sleep_for(boost::chrono::seconds(delSec));
		// 调用DCS层，仪器下电
		ULOG(LOG_INFO, "Power off to dcs, dev size:%d sec:%d.", lstDevSN.size(), delSec);
		DcsControlProxy::GetInstance()->DevicePowerOff(offDevSN);
		// 等待2秒时间
		boost::this_thread::sleep_for(boost::chrono::seconds(2));
	};
	// 启动延迟下电线程
	ULOG(LOG_INFO, "Start delay power off thread, dev size:%d sec:%d.", lstDevSN.size(), sec);
	m_pDelayPowerOffThread = new std::thread(delayPowerOffFunc, lstDevSN, sec);
}

void DevRecordInfo::init()
{
    mapDevsRecord.clear();
    finishedCount = 0;
    finishedFlag = false;
    vecFailedDevSnList.clear();
}

void DevRecordInfo::UpdateMaintainType(const std::string& devSN, ::tf::MaintainGroupType::type maintainType)
{
    std::map<std::string, MaintainAbout>::iterator it = mapDevsRecord.find(devSN);
    if (it != mapDevsRecord.end())
    {
        it->second.doType = maintainType;
    }
}

void DevRecordInfo::UpdateDeviceFlag(const std::string& devSn)
{
    finishedCount++;
    std::map<std::string, MaintainAbout>::iterator it = mapDevsRecord.find(devSn);
    if (it != mapDevsRecord.end())
    {
        it->second.alreadyRecord = true;
    }
    else
    {
        addDev(devSn);
        mapDevsRecord[devSn].alreadyRecord = true;
    }
}

std::vector< tf::DevicekeyInfo> DevRecordInfo::getDevKeyInfoList()
{
    std::vector< ::tf::DevicekeyInfo> vecList;
    for (auto item : mapDevsRecord)
    {
        ::tf::DevicekeyInfo devTemp;
        devTemp.__set_sn(item.first);

        vecList.push_back(devTemp);
    }

    return vecList;
}

std::vector<std::string> DevRecordInfo::getDevSnList()
{
    std::vector<std::string> vecSnList;
    for (auto item : mapDevsRecord)
    {
        vecSnList.push_back(item.first);
    }

    return vecSnList;
}

void DevRecordInfo::GetDeviceSnAndMaintainId(std::map<tf::MaintainGroupType::type, std::vector<std::string>>& classfiedDev)
{
    std::map<std::string, MaintainAbout>::iterator it = mapDevsRecord.begin();
    for (; it != mapDevsRecord.end(); ++it)
    {
        std::map<tf::MaintainGroupType::type, std::vector<std::string>>::iterator cdIter = classfiedDev.find(it->second.doType);
        if (cdIter != classfiedDev.end())
        {
            cdIter->second.push_back(it->first);
        }
        else
        {
            classfiedDev.insert(std::pair<tf::MaintainGroupType::type, std::vector<std::string>>(it->second.doType, { it->first }));
        }
    }
}
