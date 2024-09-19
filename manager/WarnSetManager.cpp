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
/// @file     WarnSetManager.cpp
/// @brief    系统总览-报警设置管理器源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年8月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年8月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "WarnSetManager.h"
#include <QDateTime>
#include <QTimer>
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "shared/ReagentCommon.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_constants.h"

#define INTERVAL			(2 * 60 * 1000)							// 系统总览显示的时候2分钟检测一次时间参数

// ise耗材报警设置
#define		ISE_OPEN_EXPIRE								{ 0,  EWL_WARNING }		// 开瓶过期
#define		ISE_NOT_CALI								{ 1,  EWL_WARNING }		// 未校准
#define		ISE_SUPPLY_EXPIRE							{ 2,  EWL_WARNING }		// 耗材过期
#define		ISE_CALI_FAILED								{ 3,  EWL_WARNING }		// 校准失败
#define		ISE_BOTTLE_MARGIN_NOT_ENOUGH				{ 4,  EWL_NOTICE  }		// 余量不足
#define		ISE_CALI_RESULT_EXPIRE						{ 5,  EWL_NOTICE  }		// 校准结果过期
#define		ISE_BOTTLE_MARGIN_0							{ 6,  EWL_WARNING }		// 余量为0
#define		ISE_SUPPLY_OUT_OF_CONTROL					{ 7,  EWL_WARNING }		// 耗材失控

// ch、im试剂报警设置
#define		CHIMR_OPEN_EXPIRE							{ 0,  EWL_WARNING }		// 开瓶过期
#define		CHIMR_CALI_FAILED							{ 1,  EWL_WARNING }		// 校准失败
#define		CHIMR_REAGENT_EXPIRE                        { 2,  EWL_WARNING }		// 试剂过期
#define		CHIMR_BOTTLE_CALI_CURVE_EXPIRE              { 3,  EWL_NOTICE  }		// 瓶校准曲线过期
#define		CHIMR_PRO_MARGIN_NOT_ENOUGH					{ 4,  EWL_NOTICE  }		// 项目余量不足
#define		CHIMR_LOT_CALI_CURVE_EXPIRE					{ 5,  EWL_NOTICE  }		// 批校准曲线过期
#define		CHIMR_PRO_MARGIN_0							{ 6,  EWL_WARNING }		// 项目余量为0
#define		CHIMR_QC_OUT_OF_CONTROL						{ 7,  EWL_WARNING }		// 试剂失控
#define		CHIMR_NOT_CALI								{ 8,  EWL_WARNING }		// 未校准

// ch、im耗材报警设置
#define		CHIMS_OPEN_EXPIRE							{ 0,  EWL_WARNING }		// 开瓶过期
#define		CHIMS_SUPPLY_MARGIN_NOT_ENOUGH              { 1,  EWL_NOTICE  }		// 耗材余量不足
#define		CHIMS_SUPPLY_EXPIRE 						{ 2,  EWL_WARNING }		// 耗材过期
#define		CHIMS_SUPPLY_MARGIN_0                       { 3,  EWL_WARNING }		// 耗材余量为0

// ch、im废液桶报警设置
#define		CHIM_LIQUID_WASTE_NOTICE					{ 0, EWL_NOTICE  }		// 废液桶可用空间小于设定值
#define		CHIM_LIQUID_WASTE_FULL						{ 1, EWL_WARNING }		// 废液桶满

// 免疫独有的垃圾桶
#define		IM_TRASH_CAN_NOTICE							{ 0, EWL_NOTICE  }		// 垃圾桶可用空间小于设定值
#define		IM_TRASH_CAN_FULL							{ 1, EWL_WARNING }		// 垃圾桶满

///														 
/// @brief 初始化单例对象
///
std::shared_ptr<WarnSetManager>    WarnSetManager::s_instance;
std::recursive_mutex               WarnSetManager::s_mtx;

WarnSetManager::WarnSetManager()
{
	// 监听关注报警设置列表
	REGISTER_HANDLER(MSG_ID_SYSTEM_OVERVIEW_WARNSET_UPDATE, this, slotUpdateCareWarns);
	// 监听生化试剂/耗材更新
	REGISTER_HANDLER(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, this, slotUpdateChReagentSupply);
	// 监听免疫试剂更新
	REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, slotUpdateImReagent);
	// 监听免疫耗材更新
	REGISTER_HANDLER(MSG_ID_IM_SUPPLY_INFO_UPDATE, this, slotUpdateImSupply);
	// 监听免疫稀释液更新
	REGISTER_HANDLER(MSG_ID_IM_DILUENT_INFO_UPDATE, this, slotUpdateImDiluent);
	// 监听ISE耗材更新
	REGISTER_HANDLER(MSG_ID_ISE_SUPPLY_INFO_UPDATE, this, slotUpdateIseSupply);
	// 监听ISE校准申请更新
	REGISTER_HANDLER(MSG_ID_ISE_CALI_APP_UPDATE, this, slotUpdateCaliAppSatus);

    // 试剂报警值更新
    REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_REAGENT, this, slotUpdateAlarmVol);

    // 耗材报警值更新
    REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_SUPPLY, this, slotUpdateAlarmVol);

	// 废液桶状态、余量更新
	REGISTER_HANDLER(MSG_ID_CH_WASTE_CNT_UPDATE, this, slotUpdateBucketStatus);

	// 监听校准过期提示开关改变
	REGISTER_HANDLER(MSG_ID_CALI_LINE_EXPIRE_CONFIG, this, OnCaliExpireNoticeChanged);

	// 监听质控结果更新
	REGISTER_HANDLER(MSG_ID_QC_RESULT_UPDATE, this, slotQcResultUpdate);

	// 监听项目编码管理器信息更新(可能是校准周期改变)
	REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayInfoUpdate);

	// 初始化定时器
	m_timer = new QTimer(this);
	m_timer->setInterval(INTERVAL);
	connect(m_timer, &QTimer::timeout, this, &WarnSetManager::OnUpdateTimeDependParam);

    m_cacheImSupplyAttri.ReloadAttributes();
}

std::shared_ptr<WarnSetManager> WarnSetManager::GetInstance()
{
    // 检查是否存在单例对象
    if (nullptr == s_instance)
    {
        std::unique_lock<std::recursive_mutex> autoLock(s_mtx);

        // 检查单例对象是否存在
        if (nullptr == s_instance)
        {
            s_instance.reset(new WarnSetManager);
        }
    }

    return s_instance;
}

bool WarnSetManager::init()
{
	// 初始化关注的报警设置列表
	if (!initCareWarns())
	{
		ULOG(LOG_ERROR, "Failed to execute initCareWarns()");
		return false;
	}

	// 初始化缓存报警列表
	if(!initTempWarns())
	{
		ULOG(LOG_ERROR, "Failed to execute initTempWarns()");
		return false;
	}

	return true;
}

QMap<QString, QMap<EM_BP, EM_WL>> WarnSetManager::filterGenerateWarns()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	std::lock_guard<std::recursive_mutex> autoLock(m_careWarnsMtx);
	std::lock_guard<std::recursive_mutex> autoLock1(m_setWarnsMtx);

	// 返回的结果
	QMap<QString, QMap<EM_BP, EM_WL>> resultMap;

	// 遍历报警列表
	for (const auto& tempWarn : m_setTempWarns)
	{
		// 关注列表中 包含该报警类型且包含对应报警代码
		if (m_mapCareWarns.count(tempWarn.m_type) &&
			m_mapCareWarns[tempWarn.m_type].find(tempWarn.m_codeLev.first) != m_mapCareWarns[tempWarn.m_type].end())
		{
			QString devSN = QString::fromStdString(tempWarn.m_devSn);
			// 没有直接放入，已有如果是“提示”继续放入，直到放入更高级别的“警告”
			if (!resultMap.contains(devSN) ||
				!resultMap[devSN].contains(tempWarn.m_bottlePos) ||
				resultMap[devSN][tempWarn.m_bottlePos] == EWL_NOTICE)
			{
				resultMap[devSN][tempWarn.m_bottlePos] = tempWarn.m_codeLev.second;
			}
		}
	}

	return resultMap;
}

///
/// @brief
///     忽略免疫耗材更新
///
/// @param[in]  bIgnore  忽略
///
/// @par History:
/// @li 4170/TangChuXian，2024年6月21日，新建函数
///
void WarnSetManager::SetIgnoreImSplUpdate(bool bIgnore)
{
    m_bIgnoreImSplUpdate = bIgnore;
    if (m_bIgnoreImSplUpdate)
    {
        return;
    }

    // 如果是开始监听，则加载
    clearWarn(EWT_IM_REAGENT);
    clearWarn(EWT_IM_SUPPLY);
    clearWarn(EWT_IM_LIQUID_WASTE);
    clearWarn(EWT_IM_TRASH_CAN);

    // 重新加载
    // 初始化所有免疫设备对应报警
    initImWarns(m_deviceSnListIm);

    // 更新系统总览报警瓶子
    updateSysWarnBottles();
}

void WarnSetManager::SetSysVisble(const bool bShow)
{
	if (bShow)
	{
		// 系统总览显示时，定时器检测时间参数
		m_timer->start();
	}
	else
	{
		// 隐藏时停止计时
		m_timer->stop();
	}
}

void WarnSetManager::updateWarn(const string &devSn, const EM_WT type,const EM_BP bottlePos,
	const string& index, const pair<int, EM_WL>& codeLev,const bool resolve)
{
	std::lock_guard<std::recursive_mutex> autoLock(m_setWarnsMtx);

	// 构造报警信息
	ST_WDI temp(devSn, type, bottlePos, index, codeLev);

	// 如果已经解决，从缓存中删掉
	if (resolve)
	{
		auto iter = m_setTempWarns.find(temp);
		if (iter != m_setTempWarns.end())
		{
			m_setTempWarns.erase(iter);
		}
	}
	// 否则插入报警信息
	else
	{
		m_setTempWarns.insert(temp);
	}
}

void WarnSetManager::deleteWarn(const string &devSn, const EM_WT type, const string& index)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	std::lock_guard<std::recursive_mutex> autoLock(m_setWarnsMtx);

	// 找到对应的报警信息并删除
	for (auto it = m_setTempWarns.begin(); it != m_setTempWarns.end();)
	{
		if (it->m_devSn == devSn && it->m_type == type && it->m_index == index)
		{
			it = m_setTempWarns.erase(it);
		}
		else
		{
			++it;
		}
	}
}

///
/// @brief
///     清空某个设备某个类型的告警
///
/// @param[in]  type    告警类型
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月22日，新建函数
///
void WarnSetManager::clearWarn(const EM_WT type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::lock_guard<std::recursive_mutex> autoLock(m_setWarnsMtx);

    // 找到对应的报警信息并删除
    for (auto it = m_setTempWarns.begin(); it != m_setTempWarns.end();)
    {
        if (it->m_type == type)
        {
            it = m_setTempWarns.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool WarnSetManager::initCareWarns()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	SystemOverviewWarnSet warnSet;
	if (!DictionaryQueryManager::GetWarnsSet(warnSet))
	{
		ULOG(LOG_ERROR, "Failed to get warnsets from DB.");
		return false;
	}

	// 更新关心的报警码
	slotUpdateCareWarns(warnSet);

	return true;
}

bool WarnSetManager::initTempWarns()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取指定类型设备
	auto devices = CIM_INSTANCE->GetDeviceFromType(
	{ tf::DeviceType::DEVICE_TYPE_C1000,
		tf::DeviceType::DEVICE_TYPE_I6000,
		tf::DeviceType::DEVICE_TYPE_ISE1005 });

	// 遍历设备
	for (const auto& device : devices)
	{
		// 生化
		if (device->deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
		{
			m_deviceSnListCh.push_back(device->deviceSN);
		}
		// 免疫
		else if (device->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
		{
			m_deviceSnListIm.push_back(device->deviceSN);
		}
		// ise
		else
		{
			m_deviceSnListIse.push_back(device->deviceSN);
		}
	}

	// 初始化所有生化设备对应报警
	if (!initChWarns(m_deviceSnListCh))
	{
		return false;
	}

	// 初始化所有免疫设备对应报警
	if (!initImWarns(m_deviceSnListIm))
	{
		return false;
	}

	// 初始化所有ISE设备对应报警
	if (!initIseWarns(m_deviceSnListIse))
	{
		return false;
	}

	return true;
}

bool WarnSetManager::initChWarns(const std::vector<std::string>& devices)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (devices.empty())
	{
		ULOG(LOG_INFO, "CH devices is empty");
		return true;
	}

	// 遍历设备进行刷新
	for (const auto &devSN : devices)
	{
		// 依次更新报警的仓内试剂和仓内耗材
		auto mapReagent = CIM_INSTANCE->GetChDiskReagentSupplies(devSN);
		for (const auto &rgt : mapReagent)
		{
			updateChReagent(rgt.second);
		}

		// 依次更新报警的仓外耗材
		auto mapSupplies = CIM_INSTANCE->GetChCabinetSupplies(devSN);
		for (const auto &supply : mapSupplies)
		{
			// 排除非反应杯酸碱清洗液
			if (supply.second.type != ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ACIDITY
				&& supply.second.type != ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ALKALINITY)
			{
				continue;
			}

			updateChSupply(EWT_CH_SUPPLY, supply.second);
		}

		// 废液桶
		updateChWasteLiquid(devSN);
	}

	return true;
}

bool WarnSetManager::initImWarns(const std::vector<std::string>& devices)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (devices.empty())
	{
		ULOG(LOG_INFO, "IM devices is empty");
		return true;
	}

	// 试剂
	{
		// 构造查询条件和查询结果-查询所有试剂信息
		::im::tf::ReagentInfoTableQueryResp qryResp;
		::im::tf::ReagTableUIQueryCond qryCond;
		qryCond.__set_deviceSNs(devices);
		if (!::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond)
			|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "im::LogicControlProxy::QueryReagentInfoForUI() failed.");
			return false;
		}

		// 依次更新报警的试剂
		for (const auto &reagent : qryResp.lstReagentInfos)
		{
            // 扫描失败的试剂忽略
            if (reagent.__isset.assayCode && reagent.assayCode <= 0)
            {
                continue;
            }

			updateImReagent(reagent);
		}
	}

	// 稀释液
	{
		// 构造查询条件和查询结果-查询所有稀释液
		::im::tf::DiluentInfoTableQueryResp qryResp;
		::im::tf::ReagTableUIQueryCond qryCond;
		qryCond.__set_deviceSNs(devices);

		// 查询所有稀释液信息
		if (!::im::LogicControlProxy::QueryDiluentInfoForUI(qryResp, qryCond)
			|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "im::LogicControlProxy::QueryDiluentInfoForUI() failed.");
			return false;
		}

		// 依次更新报警的稀释液
		for (const auto& diluent : qryResp.lstDiluentInfos)
		{
			updateImDiluent(diluent);
		}
	}

	// 耗材
	{
		for (auto& dev : devices)
		{
			// 构造查询条件和查询结果-查询所有耗材信息
			::im::tf::SuppliesInfoTableQueryResp qryResp;
			::im::tf::SuppliesInfoTableQueryCond qryCond;
			qryCond.__set_deviceSN(dev);
			qryCond.__set_isLoad(true);
			if (!::im::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond) 
				|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_ERROR, "im::LogicControlProxy::QuerySuppliesInfo() failed.");
				continue;
			}

			// 依次更新报警的耗材
			for (const auto& supply : qryResp.lstSuppliesInfos)
			{
				updateImSupply(supply);
			}
		}
	}

	return true;
}

bool WarnSetManager::initIseWarns(const std::vector<std::string>& devices)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (devices.empty())
	{
		ULOG(LOG_INFO, "ISE devices is empty");
		return true;
	}

	// 遍历设备进行刷新
	for (const auto &devSN : devices)
	{
		// 查询ISE的耗材, 查询在机的耗材
		auto mapSupplies = CIM_INSTANCE->GetIseSupplies(devSN);

		// 依次更新报警的耗材
		for (const auto& si : mapSupplies)
		{
			updateIseSupply(si.second);
		}

		// 更新各个设备校准
		updateIseModule(devSN);
	}

	return true;
}

void WarnSetManager::updateIseModule(const string& devSn)
{
	// 遍历ise校准模块列表
	auto moduleInfos = getIseModuleInfos(devSn);
	bool bCaliExpire = false, bNotCali = false, bCaliFail = false, bOutQc = false;
	for (const auto &mi : moduleInfos)
	{
		if (!judgeISeCaliCurveExpire(mi.caliCurveId))
		{
			bCaliExpire = true;
		}

		if(mi.caliStatus == tf::CaliStatus::CALI_STATUS_NOT && mi.caliCurveId < 0)
		{
			bNotCali = true;
		}

		if (mi.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL)
		{
			bCaliFail = true;
		}

		// qcStat 使用3个二进制位来标识Cl,K,Na的质控状态, 1: 表示失控， 0: 表示在控，001:NA , 010:K, 100:CL,
		// 001: 表示NA失控，K,CL在控， 011:K,NA 失控，CL 在控， 111: 三者均失控， 000:三者均在控
		// 只要不等于0就表示失控
		if (mi.qcStat != 0)
		{
			bOutQc = true;
		}
	}

	// 校准结果过期
	updateWarn(devSn, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, devSn, ISE_CALI_RESULT_EXPIRE, !bCaliExpire);
	// 未校准
	updateWarn(devSn, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, devSn, ISE_NOT_CALI, !bNotCali);
	// 校准失败
	updateWarn(devSn, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, devSn, ISE_CALI_FAILED, !bCaliFail);
	// 质控失控 0表示在控 非0表示失控
	updateWarn(devSn, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, devSn, ISE_SUPPLY_OUT_OF_CONTROL, !bOutQc);
}

void WarnSetManager::updateChReagent(const ChReagentInfo& cri)
{
	const auto &si = cri.supplyInfo;
	const auto &rgts = cri.reagentInfos;

	// 无效试剂 删除
	if (si.suppliesCode <= 0)
	{
		deleteWarn(si.deviceSN, EWT_CH_REAGENT, to_string(si.pos));
	}
	// 试剂
	else if (si.type == ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
	{
		// 试剂组不存在认为无效试剂 删除报警
		if (rgts.empty())
		{
			deleteWarn(si.deviceSN, EWT_CH_REAGENT, to_string(si.pos));
			return;
		}

		// 遍历试剂组 判断各个报警(双向同测时，可能会覆盖掉原来的状态，有一个有问题，都该有问题)
		std::vector<bool> vecResolveFlag;
		vecResolveFlag.resize(9, true);

		for (const auto ri : rgts)
		{
			// 开瓶过期
			if (GetOpenRestdays(ri.openBottleExpiryTime) <= 0)
			{
				vecResolveFlag[0] = false;
			}

			// 试剂过期
			if (ChSuppliesExpire(ri.expiryTime))
			{
				vecResolveFlag[1] = false;
			}

			// 统计可用测试数
			int canUseTimes = GetAvailableReagentTimes(ri.assayCode, ri.deviceSN);
			if (canUseTimes <= 0)
			{
				// 项目余量为0
				vecResolveFlag[2] = false;
			}

			// 获取项目配置信息
			auto spAssayInfo = CIM_INSTANCE->GetAssayInfo(ri.assayCode);
			if (spAssayInfo != nullptr && spAssayInfo->assayAlarmThreshold > 0 
				&& canUseTimes <= spAssayInfo->assayAlarmThreshold)
			{
				// 项目余量不足
				vecResolveFlag[3] = false;
			}

			auto pair = judgeChCaliCurveExpire(ri);
			// 瓶校准曲线过期
			if (pair.first)
			{
				vecResolveFlag[4] = false;
			}

			// 批校准曲线过期
			if (pair.second)
			{
				vecResolveFlag[5] = false;
			}

			// 校准失败
			if (ri.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL)
			{
				vecResolveFlag[6] = false;
			}

			// 未校准
			if (ri.caliStatus == tf::CaliStatus::CALI_STATUS_NOT && ri.caliCurveId < 0)
			{
				vecResolveFlag[7] = false;
			}

			// 质控失控
			if (ri.qcStatus == tf::QcStat::QC_STAT_UNCONTROLLABLE)
			{
				vecResolveFlag[8] = false;
			}
		}

		// 开瓶过期
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_OPEN_EXPIRE, vecResolveFlag[0]);

		// 试剂过期
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_REAGENT_EXPIRE, vecResolveFlag[1]);

		// 模块余量为0
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_PRO_MARGIN_0, vecResolveFlag[2]);

		// 模块余量不足
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_PRO_MARGIN_NOT_ENOUGH, vecResolveFlag[3]);

		// 瓶校准曲线过期
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_BOTTLE_CALI_CURVE_EXPIRE, vecResolveFlag[4]);

		// 批校准曲线过期
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_LOT_CALI_CURVE_EXPIRE, vecResolveFlag[5]);

		// 校准失败
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_CALI_FAILED, vecResolveFlag[6]);

		// 未校准
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_NOT_CALI, vecResolveFlag[7]);

		// 质控失控
		updateWarn(si.deviceSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_QC_OUT_OF_CONTROL, vecResolveFlag[8]);
	}
	// 仓内耗材（稀释液、抑菌剂等等）
	else
	{
		updateChSupply(EWT_CH_REAGENT, si);
	}
}

void WarnSetManager::updateChSupply(const EM_WT type, const ch::tf::SuppliesInfo& si)
{
	// 无效耗材 删除
	if (si.suppliesCode <= 0)
	{
		deleteWarn(si.deviceSN, type, to_string(si.pos));
		return;
	}

	// 瓶子位置（默认仓内耗材）
	EM_BP bPos = EBP_CH_REAGENT;

	// 是否样本针酸碱清洗液（默认不是）
	bool isSample = false;

	// 反应杯酸碱清洗液
	if (si.type == ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ACIDITY
		|| si.type == ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ALKALINITY)
	{
		bPos = EBP_CH_SUPPLY;
	}

	// 开瓶过期
	bool openExpire = (GetOpenRestdays(si.openBottleExpiryTime) > 0);
	bPos == EBP_CH_REAGENT ? updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMR_OPEN_EXPIRE, openExpire)
		: updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMS_OPEN_EXPIRE, openExpire);

	// 失效过期
	bool expire = !ChSuppliesExpire(si.expiryTime);
	bPos == EBP_CH_REAGENT ? updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMR_REAGENT_EXPIRE, expire)
		: updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMS_SUPPLY_EXPIRE, expire);

	// 仓内耗材表示可用测试数,仓外耗材表示剩余测试数 单位: 0.1ul
	int residual = 0;

	// 样本针的耗材属性信息
	int warnValue = -1; // 耗材的提醒值
	auto supplyAttribute = CIM_INSTANCE->GetSupplyAttributeByTypeCode(si.type, isSample);
	if (!supplyAttribute.has_value())
	{
		ULOG(LOG_ERROR, "GetSupplyAttributeByTypeCode failed, si.type=%d, isSample=%d", si.type, (int)isSample);
		return;
	}

	// 仓内耗材
	if (type == EWT_CH_REAGENT)
	{
		residual = GetAvailableSupplyTimes(si.suppliesCode, si.deviceSN);
		warnValue = supplyAttribute.value()->assayAlarmThreshold;
	}
	// 仓外耗材
	else
	{
		if (!si.cavityInfos.empty())
		{
			residual = GetSupplyResidual(si);
		}

		warnValue = supplyAttribute.value()->bottleAlarmThreshold;
	}

	// 模块余量为0
	bPos == EBP_CH_REAGENT ? updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMR_PRO_MARGIN_0, residual > 0)
		: updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMS_SUPPLY_MARGIN_0, residual > 0);

	// 模块余量不足(如果未启用报警门限，无需报警)
	bool resolve = (warnValue <= 0 || residual > warnValue);
	bPos == EBP_CH_REAGENT ? updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMR_PRO_MARGIN_NOT_ENOUGH, resolve)
		: updateWarn(si.deviceSN, type, bPos, to_string(si.pos), CHIMS_SUPPLY_MARGIN_NOT_ENOUGH, resolve);
}

void WarnSetManager::updateChWasteLiquid(const std::string& devSN)
{
	auto cnt = CIM_INSTANCE->GetChWasteCnt(devSN);

	// 废液满
	updateWarn(devSN, EWT_CH_LIQUID_WASTE, EBP_CH_LIQUID_WASTE, devSN, CHIM_LIQUID_WASTE_FULL, cnt < ::ch::c1005::tf::g_c1005_constants.MAX_WASTE);

	// 废液提醒值
	auto warnValue = DictionaryQueryManager::GetInstance()->GetChWasteAlarmVolumn();
	// 废液告警
	updateWarn(devSN, EWT_CH_LIQUID_WASTE, EBP_CH_LIQUID_WASTE, devSN, CHIM_LIQUID_WASTE_NOTICE, cnt < warnValue);

	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

void WarnSetManager::updateImReagent(const im::tf::ReagentInfoTable& rit)
{
	// 开瓶过期
	updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_OPEN_EXPIRE, GetOpenBottleValidDays(rit) > 0);

	// 试剂过期
	updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_REAGENT_EXPIRE, judgeExprie(rit.reagentExpiry));

	// 项目余量为0 / 项目余量不足 (项目报警)
	judgeProModuleMargin(rit.deviceSN, rit.reagentPos, rit.assayCode, rit.avaTestNum, false);

	// 校准曲线有效期
	QStringList strCalCurveExpLst = QString::fromStdString(rit.caliCurveExpInfo).split(":");
	// 瓶校准曲线过期、批校准曲线过期标志
	bool bottleFlag = true, lotFlag = true;
	if ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0))
	{
		if (strCalCurveExpLst[1] == "B")
		{
			bottleFlag = false;
		}
		else if (strCalCurveExpLst[1] == "L")
		{
			lotFlag = false;
		}
	}
	// 瓶校准曲线过期
	updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_BOTTLE_CALI_CURVE_EXPIRE, bottleFlag);
	// 批校准曲线过期
	updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_LOT_CALI_CURVE_EXPIRE, lotFlag);

	// 校准失败
	updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_CALI_FAILED, rit.caliStatus != tf::CaliStatus::CALI_STATUS_FAIL);

	// 未校准
	updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_NOT_CALI, rit.caliStatus != tf::CaliStatus::CALI_STATUS_NOT);

	// 质控失控 0-未失控 1-失控
	updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_QC_OUT_OF_CONTROL, rit.qCOut <= 0);
}

void WarnSetManager::updateImDiluent(const im::tf::DiluentInfoTable& dit)
{
	// 开瓶过期
	updateWarn(dit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(dit.pos), CHIMR_OPEN_EXPIRE, GetOpenBottleValidDays(dit) > 0);

	// 耗材过期
	updateWarn(dit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(dit.pos), CHIMR_REAGENT_EXPIRE, judgeExprie(dit.diluentExpiry));

	// 耗材类型
	im::tf::SuppliesType::type st;
	if (IsSpecialWashing(dit.diluentNumber))
	{
		st = im::tf::SuppliesType::SUPPLIES_TYPE_SPECIAL_WASH;
	}
	else
	{
		st = im::tf::SuppliesType::SUPPLIES_TYPE_DILUENT;
	}

	// 项目余量为0 / 项目余量不足（项目报警）
	judgeImModuleMargin(dit.deviceSN, to_string(dit.pos), st, EWT_IM_REAGENT, EBP_IM_REAGENT, (dit.avaVol + 999.9) / 1000.0);
}

void WarnSetManager::updateImSupply(const im::tf::SuppliesInfoTable& sit)
{
    // 如果不是I6000界面要显示的耗材，则返回
    if (sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_SOLID
        && sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID
        && sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A
        && sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B
        && sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID
        && sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
    {
        return;
    }

	// 用类型来当作位置判断
	string index = to_string(sit.supType);
    if (sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A ||
        sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B ||
        sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID)
    {
        // 加上组号
        index.append(to_string(sit.groupNum));
    }

	// 垃圾桶/废液桶特殊处理
	if (sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID
		|| sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID)
	{
        // 触发光电可能导致后台将规格改为0的情况，此处不再判断
		bool bRemainVolAlarm = true;
		if (/*(sit.volumeAmount > 0) && */(sit.remainVol >= sit.volumeAmount))
		{
			bRemainVolAlarm = false;
		}

		// 垃圾桶/废液桶满
		(sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID) ? 
		updateWarn(sit.deviceSN, EWT_IM_TRASH_CAN, EBP_IM_TRASH_CAN, index, IM_TRASH_CAN_FULL, bRemainVolAlarm)
			: updateWarn(sit.deviceSN, EWT_IM_LIQUID_WASTE, EBP_IM_LIQUID_WASTE, index, CHIM_LIQUID_WASTE_FULL, bRemainVolAlarm);

		// 垃圾桶/废液桶可用空间小于设定值
		(sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID) ?
		judgeImModuleMargin(sit.deviceSN, index, sit.supType, EWT_IM_TRASH_CAN, EBP_IM_TRASH_CAN, sit.remainVol)
			: judgeImModuleMargin(sit.deviceSN, index, sit.supType, EWT_IM_LIQUID_WASTE, EBP_IM_LIQUID_WASTE, sit.remainVol);

		return;
	}

    double dResidual = sit.remainVol;

	// 底物液 位置特殊判断
	if (sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A
		|| sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B)
	{
		// A1:11 B1:12 A2:21 B2:22
		index = index + to_string(sit.groupNum);
        dResidual = sit.available == 0 ? 0.0 : sit.remainVol;
	}

	// 开瓶过期(反应杯不判断开瓶过期)
    if (sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
    {
        updateWarn(sit.deviceSN, EWT_IM_SUPPLY, EBP_IM_SUPPLY, index, CHIMS_OPEN_EXPIRE, GetOpenBottleValidDays(sit) > 0);
    }

	// 耗材过期
	updateWarn(sit.deviceSN, EWT_IM_SUPPLY, EBP_IM_SUPPLY, index, CHIMS_SUPPLY_EXPIRE, judgeExprie(sit.expiryDate));

	// 耗材余量为0 / 耗材余量不足
	judgeImModuleMargin(sit.deviceSN, index, sit.supType, EWT_IM_SUPPLY, EBP_IM_SUPPLY, dResidual);
}

void WarnSetManager::updateIseSupply(const ise::tf::SuppliesInfo& si)
{
	auto index = to_string(si.moduleIndex) + "-" + to_string(si.pos);

	// 开瓶过期
	updateWarn(si.deviceSN, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, index, ISE_OPEN_EXPIRE, GetOpenRestdays(si.openBottleExpiryTime) > 0);

	// 耗材过期
	updateWarn(si.deviceSN, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, index, ISE_SUPPLY_EXPIRE, !ChSuppliesExpire(si.expiryTime));

	// 耗材的瓶提醒
	int warnValue = -1; 
	auto supplyAttribute = CIM_INSTANCE->GetISESupplyAttributeByTypeCode(si.suppliesCode);
	if (supplyAttribute)
	{
		warnValue = supplyAttribute.value()->bottleAlarmThreshold;
	}
	// 瓶余量为0
	updateWarn(si.deviceSN, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, index, ISE_BOTTLE_MARGIN_0, si.residual > 0);

	// 瓶余量不足(如果未启用报警门限，无需报警)
	bool resolve = (warnValue <= 0 || si.residual > warnValue);
	updateWarn(si.deviceSN, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, index, ISE_BOTTLE_MARGIN_NOT_ENOUGH, resolve);
}

void WarnSetManager::updateSysWarnBottles()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 整合过滤并生成报警信息
	auto sendWarns = filterGenerateWarns();

	// 发送报警更新到系统总览
	POST_MESSAGE(MSG_ID_WARN_UPDATE_TO_SYS_OVERVIEW, sendWarns);
}

bool WarnSetManager::judgeExprie(const std::string& exprieTime)
{
	// 过期时间
	auto expTime = QDateTime::fromString(QString::fromStdString(exprieTime), UI_DATE_TIME_FORMAT);

	return expTime > QDateTime::currentDateTime();
}

void WarnSetManager::judgeProModuleMargin(const std::string& devSn, const int pos, const int code, const int canUseTimes, const bool isCh)
{
    // 余量0
	if (isCh)
	{			
		// 模块余量为0
		updateWarn(devSn, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(pos), CHIMR_PRO_MARGIN_0, canUseTimes > 0);
	}
	else
	{
		// 模块余量为0
		updateWarn(devSn, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(pos), CHIMR_PRO_MARGIN_0, canUseTimes > 0);
	}

    // 获取项目配置信息
    auto spAssayInfo = CIM_INSTANCE->GetAssayInfo(code);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "spAssayInfo for code: %d is nullptr", code);
        return;
    }

	bool resolve = (spAssayInfo->assayAlarmThreshold <= 0 || canUseTimes > spAssayInfo->assayAlarmThreshold);
    // 项目报警数
    if (isCh)
    {
        // 模块余量不足
        updateWarn(devSn, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(pos), CHIMR_PRO_MARGIN_NOT_ENOUGH, resolve);
    }
    else
    {
        // 模块余量不足
        updateWarn(devSn, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(pos), CHIMR_PRO_MARGIN_NOT_ENOUGH, resolve);
    }
}

void WarnSetManager::judgeImModuleMargin(const std::string& devSn, const string& pos, const im::tf::SuppliesType::type type,
	const EM_WT tType, const EM_BP bPos, const int remainCount)
{
	// 查询耗材属性
    std::vector<::im::tf::SupplyAttribute> lstSupplyAttributes;
    m_cacheImSupplyAttri.QueryAttributes(lstSupplyAttributes, type);
	if (lstSupplyAttributes.empty())
    {
        ULOG(LOG_WARN, "Empty supplyattributes, type:%d.", type);
		return;
	}

	// 单位转换（0.1ul -> ml）
	int iAlarmVolML = lstSupplyAttributes[0].bottleAlarmThreshold / 10000;

	// 垃圾桶，只判断垃圾桶是否满了
	if (type == im::tf::SuppliesType::SUPPLIES_TYPE_SOLID)
	{
		// 垃圾桶可用空间小于设定值
		updateWarn(devSn, tType, bPos, pos, IM_TRASH_CAN_NOTICE, (iAlarmVolML <= 0 || remainCount < iAlarmVolML));
	}
	// 废液桶，只判断废液桶是否满了
	else if (type == im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID)
	{
		// 废液桶可用空间小于设定值
		updateWarn(devSn, tType, bPos, pos, CHIM_LIQUID_WASTE_NOTICE, (iAlarmVolML <= 0 || remainCount < iAlarmVolML));
	}
	// 稀释液放在仓内的 可用测试数
	else if (type == im::tf::SuppliesType::SUPPLIES_TYPE_DILUENT || type == im::tf::SuppliesType::SUPPLIES_TYPE_SPECIAL_WASH)
	{
		iAlarmVolML = lstSupplyAttributes[0].assayAlarmThreshold / 10000;

		// 模块余量为0
		updateWarn(devSn, tType, bPos, pos, CHIMR_PRO_MARGIN_0, remainCount > 0);

		// 模块余量不足
		updateWarn(devSn, tType, bPos, pos, CHIMR_PRO_MARGIN_NOT_ENOUGH, (iAlarmVolML <= 0 || remainCount > iAlarmVolML));
	}
	else
	{
		// 模块余量为0
		updateWarn(devSn, tType, bPos, pos, CHIMS_SUPPLY_MARGIN_0, remainCount > 0);

		// 模块余量不足
		updateWarn(devSn, tType, bPos, pos, CHIMS_SUPPLY_MARGIN_NOT_ENOUGH, (iAlarmVolML <= 0 || remainCount > iAlarmVolML));
	}
}

std::pair<bool, bool> WarnSetManager::judgeChCaliCurveExpire(const ch::tf::ReagentGroup& rg)
{
	// 瓶校准曲线过期、批校准曲线过期标志
	std::pair<bool, bool> retPair{ false, false };

	// sind排除 默认不过期
	if (rg.suppliesCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		return retPair;
	}

	// 获取校准曲线有效期
	RowCellInfo rci;
	MakeCaliCurveExpirateDate(rg, rci);
	QStringList strCalCurveExpLst = rci.m_text.split(":");

	// 校准曲线存在过期
	if ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0)
		&& DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
	{
		// 最新需求，校准过期显示黄色
		if (strCalCurveExpLst[0] == "B")
		{
			retPair.first = true;
		}
		else if (strCalCurveExpLst[0] == "L")
		{
			retPair.second = true;
		}
	}

	return retPair;
}

bool WarnSetManager::judgeISeCaliCurveExpire(const int caliCurveId)
{
	// 不存在表示已解决
	if (caliCurveId < 0)
	{
		return true;
	}

	// 未启用校准过期提示表示已解决
	if (!DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
	{
		return true;
	}

	//查询曲线详细数据
	::ise::tf::IseCaliCurveQueryResp qryResp;
	::ise::tf::IseCaliCurveQueryCond qryCond;
	qryCond.__set_id(caliCurveId);

	// 失败已解决
	ise::ise1005::LogicControlProxy::QueryIseCaliCurve(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QueryIseCaliCurve failed.");
		return true;
	}

	auto spIseAssayMap = CIM_INSTANCE->GetIseAssayIndexCodeMaps();

	// 如果取不到，则直接为空
	if (spIseAssayMap.empty() || spIseAssayMap.begin()->second == nullptr)
	{
		return true;
	}

	// 校准曲线为空说明已解决
	if (qryResp.lstIseCaliCurves.empty())
	{
		ULOG(LOG_ERROR, "qryResp.lstIseCaliCurves is empty.");
		return true;
	}

	// 获得校准时间
	auto caliTime = QDateTime::fromString(QString::fromStdString(qryResp.lstIseCaliCurves[0].caliTime), UI_DATE_TIME_FORMAT);

	return caliTime.addDays(spIseAssayMap.begin()->second->caliSuggest.timeoutCali.timeout) > QDateTime::currentDateTime();
}

std::vector<ise::tf::IseModuleInfo> WarnSetManager::getIseModuleInfos(const std::string& devSn)
{
	::ise::tf::IseModuleInfoQueryCond qryCond;
	::ise::tf::IseModuleInfoQueryResp qryResp;

	qryCond.__set_deviceSn({ devSn });
	//qryCond.__set_moduleIndex(module);

	ise::LogicControlProxy::QueryIseModuleInfo(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QueryIseModuleInfo failed.");
	}

	return qryResp.lstIseModuleInfos;
}

void WarnSetManager::slotUpdateCareWarns(const SystemOverviewWarnSet &sows)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	std::lock_guard<std::recursive_mutex> autoLock(m_careWarnsMtx);

	// 清空列表
	m_mapCareWarns.clear();

	// ise耗材
	for (auto &warn : sows.iseSupplyCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_ISE_SUPPLY].insert(warn.first);
		}
	}

	// ch试剂
	for (auto &warn : sows.chReagentCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_CH_REAGENT].insert(warn.first);
		}
	}

	// ch耗材
	for (auto &warn : sows.chSupplyCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_CH_SUPPLY].insert(warn.first);
		}
	}

	// ch废液桶
	for (auto &warn : sows.chLiquidWasteCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_CH_LIQUID_WASTE].insert(warn.first);
		}
	}

	// im试剂
	for (auto &warn : sows.imReagentCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_IM_REAGENT].insert(warn.first);
		}
	}

	// im耗材
	for (auto &warn : sows.imSupplyCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_IM_SUPPLY].insert(warn.first);
		}
	}

	// im废液桶
	for (auto &warn : sows.imLiquidWasteCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_IM_LIQUID_WASTE].insert(warn.first);
		}
	}

	// im垃圾桶
	for (auto &warn : sows.imTrashCanCheckBoxs)
	{
		if (warn.second)
		{
			m_mapCareWarns[EWT_IM_TRASH_CAN].insert(warn.first);
		}
	}
	
	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

void WarnSetManager::slotUpdateChReagentSupply(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates)
{
	// 遍历试剂/耗材
	for (const auto& supply : supplyUpdates)
	{
		int pos = supply.posInfo.pos;

		// 仓内试剂还是仓外耗材
		EM_WT type = (supply.posInfo.area == ch::tf::SuppliesArea::SUPPLIES_AREA_CABINET) ? EWT_CH_SUPPLY : EWT_CH_REAGENT;

		// 删除
		if (supply.updateType == tf::UpdateType::UPDATE_TYPE_DELETE)
		{
			deleteWarn(supply.deviceSN, type, to_string(pos));
			continue;
		}

		// 仓内试剂、耗材
		if (type == EWT_CH_REAGENT)
		{
			auto mapReagent = CIM_INSTANCE->GetChDiskReagentSupplies(supply.deviceSN, { pos });
			if (!mapReagent.count(pos))
			{
				// 没查到删除
				deleteWarn(supply.deviceSN, type, to_string(pos));
				continue;
			}

			updateChReagent(mapReagent[pos]);
		}
		// 仓外耗材
		else
		{
			auto mapSupply = CIM_INSTANCE->GetChCabinetSupplies(supply.deviceSN, { pos });
			if (!mapSupply.count(pos))
			{
				// 没查到删除
				deleteWarn(supply.deviceSN, type, to_string(pos));
				continue;
			}

			updateChSupply(EWT_CH_SUPPLY, mapSupply[pos]);
		}
	}

	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

void WarnSetManager::slotUpdateImReagent(const im::tf::ReagentInfoTable& ri, im::tf::ChangeType::type type)
{
    // 如果忽略则跳过
    if (m_bIgnoreImSplUpdate)
    {
        return;
    }

    // 参数判断
    if (!ri.__isset.deviceSN)
    {
        ULOG(LOG_WARN, "slotUpdateImReagent:no device sn.");
        return;
    }

    // 更新试剂信息，构造查询条件和查询结果,查询所有试剂信息
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // delete时后台参数位置始终为-1，UI无法判断具体卸载位置，此处特殊处理
	if (type == im::tf::ChangeType::Delete)
	{
        // 清空该类型告警
        deleteWarn(ri.deviceSN, EWT_IM_REAGENT, std::to_string(ri.reagentPos));
	}

    // 构造查询条件
    if (ri.__isset.assayCode && (ri.assayCode > 0))
    {
        std::vector<std::string> vecDevSns;
        vecDevSns.push_back(ri.deviceSN);
        qryCond.__set_deviceSNs(vecDevSns);
        qryCond.__set_assayCode(ri.assayCode);
    }
    else
    {
        ULOG(LOG_INFO, "slotUpdateImReagent:scan failed ignore.");
        return;
    }

    // 执行查询
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        updateSysWarnBottles();
        return;
    }

    // 后台设计缺陷无法监听试剂使用状态改变，暂由UI特殊处理：UI收到试剂更新通知时，更新该设备所有同项目试剂
    for (const auto& stuRgntIf : qryResp.lstReagentInfos)
    {
        updateImReagent(stuRgntIf);
    }

	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

void WarnSetManager::slotUpdateImSupply(const im::tf::SuppliesInfoTable& si, im::tf::ChangeType::type type)
{
    // 如果忽略则跳过
    if (m_bIgnoreImSplUpdate)
    {
        return;
    }

    // 清空
    clearWarn(EWT_IM_SUPPLY);
    clearWarn(EWT_IM_LIQUID_WASTE);
    clearWarn(EWT_IM_TRASH_CAN);

    // 构造查询条件和查询结果
    ::im::tf::SuppliesInfoTableQueryResp qryResp;
    ::im::tf::SuppliesInfoTableQueryCond qryCond;
    qryCond.__set_isLoad(true);

    // 查询所有耗材信息
    bool bRet = ::im::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Query suppliesInfo failed.");
        updateSysWarnBottles();
        return;
    }

    // 更新耗材信息
    m_cacheImSupplyAttri.ReloadAttributes();
    for (const auto& stuSplInfo : qryResp.lstSuppliesInfos)
    {
        updateImSupply(stuSplInfo);
    }

	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

void WarnSetManager::slotUpdateImDiluent(const im::tf::DiluentInfoTable& di, im::tf::ChangeType::type type)
{
    // 如果忽略则跳过
    if (m_bIgnoreImSplUpdate)
    {
        return;
    }

    // 参数判断
    if (!di.__isset.deviceSN)
    {
        ULOG(LOG_WARN, "slotUpdateImDiluent:no device sn.");
        return;
    }

    // delete时后台参数位置始终为-1，UI无法判断具体卸载位置，此处特殊处理
    if (type == im::tf::ChangeType::Delete)
    {
        // 清空该类型告警
        deleteWarn(di.deviceSN, EWT_IM_REAGENT, std::to_string(di.pos));
    }

    // 构造查询条件和查询结果
    ::im::tf::DiluentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 筛选条件
    std::vector<std::string> vecDevSns;
    vecDevSns.push_back(di.deviceSN);
    qryCond.__set_deviceSNs(vecDevSns);

    // 查询所有试剂信息
    bool bRet = ::im::LogicControlProxy::QueryDiluentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDiluentInfoForUI failed.");
        updateSysWarnBottles();
        return;
    }

    // 后台设计缺陷无法监听试剂使用状态改变，暂由UI特殊处理：UI收到试剂更新通知时，更新该设备所有同项目试剂
    m_cacheImSupplyAttri.ReloadAttributes();
    for (const auto& stuDltIf : qryResp.lstDiluentInfos)
    {
        updateImDiluent(stuDltIf);
    }

	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

void WarnSetManager::slotUpdateIseSupply(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates)
{
	if (supplyUpdates.empty())
	{
		return;
	}

	// 遍历耗材
	for (const auto& supply : supplyUpdates)
	{
		// 删除
		if (supply.updateType == tf::UpdateType::UPDATE_TYPE_DELETE)
		{
			deleteWarn(supply.suppliesInfo.deviceSN, EWT_ISE_SUPPLY, 
				to_string(supply.suppliesInfo.moduleIndex) + "-" + to_string(supply.suppliesInfo.pos));
		}
		else// 更新、增加
		{
			updateIseSupply(supply.suppliesInfo);
		}
	}

	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

void WarnSetManager::slotUpdateCaliAppSatus(std::vector<ise::tf::IseModuleInfo, std::allocator<ise::tf::IseModuleInfo>> iseAppUpdates)
{
	if (iseAppUpdates.empty())
	{
		return;
	}

	// 遍历更新的校准
	for (const auto& imi : iseAppUpdates)
	{
		updateIseModule(imi.deviceSn);
	}

	// 更新系统总览报警瓶子
	updateSysWarnBottles();
}

///
/// @brief
///     试剂耗材的提醒值更新
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月22日，新建函数
///
void WarnSetManager::slotUpdateAlarmVol()
{
    // 重新加载免疫告警
    ReloadWarns();

    // 更新系统总览报警瓶子
    updateSysWarnBottles();
}

void WarnSetManager::ReloadWarns()
{
    // 清空对应告警
	std::lock_guard<std::recursive_mutex> autoLock(m_setWarnsMtx);
	m_setTempWarns.clear();

    m_cacheImSupplyAttri.ReloadAttributes();

	// 初始化生化告警
	initChWarns(m_deviceSnListCh);

    // 初始化免疫告警
    initImWarns(m_deviceSnListIm);

	// 初始化ise告警
	initIseWarns(m_deviceSnListIse);
}

void WarnSetManager::slotUpdateBucketStatus(const QString& devSN)
{
	updateChWasteLiquid(devSN.toStdString());
}

void WarnSetManager::OnCaliExpireNoticeChanged(bool bOn)
{
	Q_UNUSED(bOn)

	OnAssayInfoUpdate();
}

void WarnSetManager::slotQcResultUpdate(QString devSn, int64_t qcDocId, QVector<int64_t> changedId)
{
	Q_UNUSED(qcDocId)
	Q_UNUSED(changedId)

	string strSN = devSn.toStdString();
	ULOG(LOG_INFO, "%s(devSn=%s)", __FUNCTION__, strSN);

	// 暂时只关注ise设备的，比色的已经通过上报试剂更新消息进行更新
	if (std::find(m_deviceSnListIse.begin(), m_deviceSnListIse.end(), strSN) == m_deviceSnListIse.end())
	{
		return;
	}

	// 更新质控信息
	updateIseModule(strSN);
}

void WarnSetManager::OnAssayInfoUpdate()
{
	// 只需更新试剂校准曲线有效期相关的报警
	clearWarn(EWT_CH_REAGENT);
	clearWarn(EWT_IM_REAGENT);

	// 生化试剂
	for (const auto &devSN : m_deviceSnListCh)
	{
		// 依次更新报警的仓内试剂和仓内耗材
		auto mapReagent = CIM_INSTANCE->GetChDiskReagentSupplies(devSN);
		for (const auto &rgt : mapReagent)
		{
			updateChReagent(rgt.second);
		}
	}

	// 免疫试剂
	{
		// 构造查询条件和查询结果-查询所有试剂信息
		::im::tf::ReagentInfoTableQueryResp qryResp;
		::im::tf::ReagTableUIQueryCond qryCond;
		qryCond.__set_deviceSNs(m_deviceSnListIm);
		if (!::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond)
			|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "im::LogicControlProxy::QueryReagentInfoForUI() failed.");
			return;
		}

		// 依次更新报警的试剂
		for (const auto &reagent : qryResp.lstReagentInfos)
		{
			// 扫描失败的试剂忽略
			if (reagent.__isset.assayCode && reagent.assayCode <= 0)
			{
				continue;
			}

			updateImReagent(reagent);
		}
	}

	updateSysWarnBottles();
}

void WarnSetManager::OnUpdateTimeDependParam()
{
	// 遍历生化设备
	for (const auto &devSN : m_deviceSnListCh)
	{
		// 依次更新仓内试剂和仓内耗材
		auto mapReagent = CIM_INSTANCE->GetChDiskReagentSupplies(devSN);
		for (const auto &pair : mapReagent)
		{
			auto si = pair.second.supplyInfo;

			// 扫描失败的忽略
			if (si.suppliesCode <= 0)
			{
				continue;
			}

			// 开瓶过期
			updateWarn(devSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_OPEN_EXPIRE, GetOpenRestdays(si.openBottleExpiryTime) > 0);

			// 失效过期
			updateWarn(devSN, EWT_CH_REAGENT, EBP_CH_REAGENT, to_string(si.pos), CHIMR_REAGENT_EXPIRE, !ChSuppliesExpire(si.expiryTime));
		}

		// 依次更新仓外耗材
		auto mapSupplies = CIM_INSTANCE->GetChCabinetSupplies(devSN);
		for (const auto &supply : mapSupplies)
		{
			auto si = supply.second;

			// 扫描失败的忽略
			if (si.suppliesCode <= 0)
			{
				continue;
			}

			// 排除非反应杯酸碱清洗液
			if (si.type != ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ACIDITY
				&& si.type != ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ALKALINITY)
			{
				continue;
			}

			// 开瓶过期
			updateWarn(devSN, EWT_CH_SUPPLY, EBP_CH_SUPPLY, to_string(si.pos), CHIMS_OPEN_EXPIRE, GetOpenRestdays(si.openBottleExpiryTime) > 0);

			// 失效过期
			updateWarn(devSN, EWT_CH_SUPPLY, EBP_CH_SUPPLY, to_string(si.pos), CHIMS_SUPPLY_EXPIRE, !ChSuppliesExpire(si.expiryTime));
		}
	}

	// 遍历免疫设备
	if(!m_deviceSnListIm.empty())
	{
		// 试剂
		{
			// 构造查询条件和查询结果-查询所有试剂信息
			::im::tf::ReagentInfoTableQueryResp qryResp;
			::im::tf::ReagTableUIQueryCond qryCond;
			qryCond.__set_deviceSNs(m_deviceSnListIm);
			if (!::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond)
				|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_WARN, "im::LogicControlProxy::QueryReagentInfoForUI() failed.");
			}
			for (const auto &rit : qryResp.lstReagentInfos)
			{
				// 扫描失败的试剂忽略
				if (rit.__isset.assayCode && rit.assayCode <= 0)
				{
					continue;
				}

				// 开瓶过期
				updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_OPEN_EXPIRE, GetOpenBottleValidDays(rit) > 0);

				// 试剂过期
				updateWarn(rit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(rit.reagentPos), CHIMR_REAGENT_EXPIRE, judgeExprie(rit.reagentExpiry));
			}
		}

		// 稀释液
		{
			// 构造查询条件和查询结果-查询所有稀释液
			::im::tf::DiluentInfoTableQueryResp qryResp;
			::im::tf::ReagTableUIQueryCond qryCond;
			qryCond.__set_deviceSNs(m_deviceSnListIm);

			// 查询所有稀释液信息
			if (!::im::LogicControlProxy::QueryDiluentInfoForUI(qryResp, qryCond)
				|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_WARN, "im::LogicControlProxy::QueryDiluentInfoForUI() failed.");
			}
			for (const auto& dit : qryResp.lstDiluentInfos)
			{
				// 开瓶过期
				updateWarn(dit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(dit.pos), CHIMR_OPEN_EXPIRE, GetOpenBottleValidDays(dit) > 0);

				// 耗材过期
				updateWarn(dit.deviceSN, EWT_IM_REAGENT, EBP_IM_REAGENT, to_string(dit.pos), CHIMR_REAGENT_EXPIRE, judgeExprie(dit.diluentExpiry));
			}
		}

		// 耗材
		for (const auto& dev : m_deviceSnListIm)
		{
			// 构造查询条件和查询结果-查询所有耗材信息
			::im::tf::SuppliesInfoTableQueryResp qryResp;
			::im::tf::SuppliesInfoTableQueryCond qryCond;
			qryCond.__set_deviceSN(dev);
			qryCond.__set_isLoad(true);
			if (!::im::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond)
				|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_WARN, "im::LogicControlProxy::QuerySuppliesInfo() failed.");
				continue;
			}

			// 依次更新报警的耗材
			for (const auto& sit : qryResp.lstSuppliesInfos)
			{
				// 如果不是I6000界面要显示的耗材，则返回
				if (sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_SOLID
					&& sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_WASTE_LIQUID
					&& sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A
					&& sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B
					&& sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID
					&& sit.supType != im::tf::SuppliesType::SUPPLIES_TYPE_NEW_CUPS)
				{
					continue;
				}

				// 用类型来当作位置判断
				string index = to_string(sit.supType);
				if (sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A ||
					sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B ||
					sit.supType == im::tf::SuppliesType::SUPPLIES_TYPE_CONCENTRATE_LIQUID)
				{
					// 加上组号
					index.append(to_string(sit.groupNum));
				}

				// 开瓶过期
				updateWarn(sit.deviceSN, EWT_IM_SUPPLY, EBP_IM_SUPPLY, index, CHIMS_OPEN_EXPIRE, GetOpenBottleValidDays(sit) > 0);

				// 耗材过期
				updateWarn(sit.deviceSN, EWT_IM_SUPPLY, EBP_IM_SUPPLY, index, CHIMS_SUPPLY_EXPIRE, judgeExprie(sit.expiryDate));
			}
		}
	}

	// 遍历ise设备
	for (const auto &devSN : m_deviceSnListIse)
	{
		// 查询ISE的耗材, 查询在机的耗材
		auto mapSupplies = CIM_INSTANCE->GetIseSupplies(devSN);

		// 依次更新报警的耗材
		for (const auto& pair : mapSupplies)
		{
			auto si = pair.second;
			auto index = to_string(si.moduleIndex) + "-" + to_string(si.pos);

			// 开瓶过期
			updateWarn(si.deviceSN, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, index, ISE_OPEN_EXPIRE, GetOpenRestdays(si.openBottleExpiryTime) > 0);

			// 耗材过期
			updateWarn(si.deviceSN, EWT_ISE_SUPPLY, EBP_ISE_SUPPLY, index, ISE_SUPPLY_EXPIRE, !ChSuppliesExpire(si.expiryTime));
		}

		// 更新校准时间等信息
		updateIseModule(devSN);
	}

	// 通知系统总览界面
	updateSysWarnBottles();
}

bool WarnSetManager::WarnDetailInfo::operator==(const WarnDetailInfo& other) const
{
    return this->m_devSn == other.m_devSn
        && this->m_type == other.m_type
        && this->m_bottlePos == other.m_bottlePos
        && this->m_index == other.m_index
        && this->m_codeLev == other.m_codeLev;
}

bool WarnSetManager::WarnDetailInfo::operator<(const WarnDetailInfo& other) const
{
    if (this->m_devSn != other.m_devSn)
    {
        return this->m_devSn < other.m_devSn;
    }

    if (this->m_type != other.m_type)
    {
        return this->m_type < other.m_type;
    }

    if (this->m_bottlePos != other.m_bottlePos)
    {
        return this->m_bottlePos < other.m_bottlePos;
    }

    if (this->m_index != other.m_index)
    {
        return this->m_index < other.m_index;
    }

    if (this->m_codeLev != other.m_codeLev)
    {
        return this->m_codeLev < other.m_codeLev;
    }

    return false;
}

void CacheImSupplyattributes::ReloadAttributes()
{
    ::im::tf::SupplyAttributeQueryResp splAttrQryResp;
    ::im::tf::SupplyAttributeQueryCond splAttrQryCond;

    if (!im::LogicControlProxy::QuerySupplyAttribute(splAttrQryResp, splAttrQryCond)
        || splAttrQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Failed to query all im supplyattributes.");
        return;
    }

    allAttributes = splAttrQryResp.lstSupplyAttributes;
}

void CacheImSupplyattributes::QueryAttributes(std::vector<::im::tf::SupplyAttribute>& spplyAttributes)
{
    spplyAttributes = allAttributes;
}

void CacheImSupplyattributes::QueryAttributes(std::vector<::im::tf::SupplyAttribute>& spplyAttributes, im::tf::SuppliesType::type attrType)
{
    spplyAttributes.clear();
    for (const ::im::tf::SupplyAttribute& attr : allAttributes)
    {
        if (attr.type == attrType)
        {
            spplyAttributes.push_back(attr);
        }
    }
}
