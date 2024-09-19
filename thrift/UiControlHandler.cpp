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
/// @file     UiControlHandler.cpp
/// @brief    处理UI控制命令的类
///
/// @author   3558/ZhouGuangMing
/// @date     2021年3月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QMap>
#include <QVector>
#include <algorithm>
#include "UiControlHandler.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "manager/SystemPowerManager.h"
#include "DcsControlProxy.h"

///
/// @brief
///     处理设备状态信息上报
///
/// @param[in]  deviceInfo  设备状态信息列表
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
///
void UiControlHandler::NotifyDeviceStatus(const ::tf::DeviceInfo& deviceInfo)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(deviceInfo).c_str());
    // 发送设备状态信息,使通用信息管理器更新
    POST_MESSAGE(MSG_ID_DCS_REPORT_STATUS_CHANGED, deviceInfo);
}

void UiControlHandler::NotifyCanAppendTest(const std::string& devSN, const std::set<::tf::FaultAffectAppendTest::type>& faats)
{
    ULOG(LOG_INFO, "%s(%s, %s)", __FUNCTION__, devSN, ToString(faats));

	POST_MESSAGE(MSG_ID_DEVS_CAN_APPEND_TEST, QString::fromStdString(devSN), faats);
}

///
/// @brief
///     处理样本信息更新
///
/// @param[in]  enUpdateType    更新类型
/// @param[in]  lstSIs          更新的样本信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月12日，新建函数
///
void UiControlHandler::NotifySampleUpdate(const ::tf::UpdateType::type enUpdateType, const std::vector<::tf::SampleInfo> & lstSIs)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(lstSIs).c_str());
    // 将样本信息更新消息发送到UI消息总线
    POST_MESSAGE(MSG_ID_SAMPLE_INFO_UPDATE, enUpdateType, const_cast<std::vector<tf::SampleInfo>&>(lstSIs));
}

void UiControlHandler::NotifyTestItemUpdate(const ::tf::UpdateType::type enUpdateType, const std::vector<::tf::TestItem>& lstTestItems)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(lstTestItems).c_str());
	// 将样本项目信息更新消息发送到UI消息总线
	POST_MESSAGE(MSG_ID_SAMPLE_TEST_ITEM_UPDATE, enUpdateType, const_cast<std::vector<::tf::TestItem>&>(lstTestItems));
}

///
/// @brief 处理样本上传LIS信息更新
///
/// @param[in]  sampleID  样本ID
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年5月30日，新建函数
///
void UiControlHandler::NotifySampleLisUpdate(const int64_t sampleID)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 发送到UI消息总线
	POST_MESSAGE(MSG_ID_SAMPLE_LIS_UPDATE, sampleID);
}

void UiControlHandler::NotifyDeviceFaultInfo(const ::tf::AlarmDesc& alarmDesc)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(alarmDesc).c_str());
	// 发送告警日志更新信息到UI消息总线
	POST_MESSAGE(MSG_ID_DEVS_FAULT_INFO, alarmDesc);
}

void UiControlHandler::NotifyDisCurrentAlarm(const ::tf::AlarmDesc& alarmDesc)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(alarmDesc).c_str());
    // 发送告警日志更新信息到UI消息总线
    POST_MESSAGE(MSG_ID_DEVS_DIS_ALARM, alarmDesc);
}

void UiControlHandler::NotifyUpdateCurrentAlarm(const ::tf::AlarmDesc& alarmDesc, const bool increaseParams)
{
    ULOG(LOG_INFO, "%s(%s, %d)", __FUNCTION__, ToString(alarmDesc), increaseParams);
    
    // 将报警信息详情更新消息发送到UI消息总线
    POST_MESSAGE(MSG_ID_ALARM_DETAIL_UPDATE, alarmDesc, increaseParams);
}

void UiControlHandler::NotifyAlarmReaded(const std::vector< ::tf::AlarmDesc> & alarmDescs)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 将报警信息更新消息发送到UI消息总线
    POST_MESSAGE(MSG_ID_ALARM_READED, const_cast<std::vector<tf::AlarmDesc>&>(alarmDescs));
}

///
/// @brief 质控文档信息变更通知
///
/// @param[in]  infos  更新的质控文档信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月20日，新建函数
///
void UiControlHandler::NotifyQcDocInfoUpdate(const std::vector<::tf::QcDocUpdate>& infos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 将质控文档信息更新消息发送到UI消息总线
    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, const_cast<std::vector<tf::QcDocUpdate>&>(infos));
}

///
/// @brief 处理设备状态信息更新
///
/// @param[in]    statusInfo:设备状态信息
/// 
/// @par History:
/// @li 7951/LuoXin，2023年2月13日，新建函数
///
void UiControlHandler::NotifyUnitStatusInfo(const ::tf::UnitStatusInfo& statusInfo)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
	// 发送温度更新信息到UI消息总线

	POST_MESSAGE(MSG_ID_DEVICE_STATUS_INFO, statusInfo);
}

///
/// @brief	维护项执行信息更新
///     
/// @param[in]  devSN   设备序列号
/// @param[in]  groupId 维护组id
/// @param[in]  mier    维护项执行信息
///
/// @par History:
/// @li 7951/LuoXin，2023年3月9日，新建函数
///
void UiControlHandler::ReportMaintainItemInfo(const std::string& devSN, const int64_t groupId, const ::tf::MaintainItemExeResult& mier)
{
	ULOG(LOG_INFO, "%s : %s", __FUNCTION__, ToString(mier));

	POST_MESSAGE(MSG_ID_MAINTAIN_ITEM_UPDATE, QString::fromStdString(devSN), groupId, mier);
}

///
/// @brief	维护组执行信息更新
///     
///
/// @param[in]  lmi  维护组执行信息
///
/// @par History:
/// @li 7951/LuoXin，2023年3月9日，新建函数
///
void UiControlHandler::ReportMaintainGroupInfo(const ::tf::LatestMaintainInfo& lmi)
{
	ULOG(LOG_INFO, "%s : %s", __FUNCTION__, ToString(lmi));
	POST_MESSAGE(MSG_ID_MAINTAIN_GROUP_UPDATE, lmi);
}

///
/// @brief
///     通知维护数据初始化
///
/// @param[in]  devSN  设备序列号
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月15日，新建函数
///
void UiControlHandler::ReportMaintainDataInit(const std::string& devSN)
{
    ULOG(LOG_INFO, "%s : %s", __FUNCTION__, devSN);
    QUEUE_SEND_MESSAGE(MSG_ID_MAINTAIN_DATA_INIT, QString::fromStdString(devSN));
}

void UiControlHandler::NotifyAlarmShieldClearAll()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    POST_MESSAGE(MSG_ID_ALARM_SHIELD_CLEAR_ALL);
}

void UiControlHandler::NotifyLisConnectionStatus(const bool status)
{
	ULOG(LOG_INFO, "%s : status:%d", __FUNCTION__,status);

	POST_MESSAGE(MSG_ID_LIS_CONNECTION_STATUS, status);
}

///
/// @brief 通知样本架回收消息变更
///
/// @param[in]  rack			样本架架号信息
/// @param[in]  recycleRackInfo 样本架回收信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月24日，新建函数
///
void UiControlHandler::NotifyRecycleRackUpdate(const int32_t trayNo, const std::vector< ::tf::RecycleRackInfo> & rackInfo)
{
	ULOG(LOG_INFO, "%s(tray: %d { %s })", __FUNCTION__, trayNo, ToString(rackInfo).c_str());
	POST_MESSAGE(MSG_ID_RACK_RECYCLE_UPDATE, trayNo, const_cast<std::vector<tf::RecycleRackInfo>&>(rackInfo));
}

///
/// @bref
///		通知界面，质控结果发生变化
///
/// @par History:
/// @li 8276/huchunli, 2023年9月13日，新建函数
///
void UiControlHandler::NotifyQcResultUpdate(const std::string& devceSn, const int64_t qcDocId, const std::vector<int64_t> & updatedId)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    QVector<int64_t> vUpdateId;
    std::copy(std::begin(updatedId), std::end(updatedId), std::back_inserter(vUpdateId));
    POST_MESSAGE(MSG_ID_QC_RESULT_UPDATE, QString::fromStdString(devceSn), qcDocId, vUpdateId);
}

void UiControlHandler::NotifyTrackStatusUpdate(const ::track::tf::LowerStatusInfo& statusInfo)
{
	POST_MESSAGE(MSG_ID_TRACT_STATE_UPDATE, const_cast<::track::tf::LowerStatusInfo&>(statusInfo));
}

///
/// @brief 通知UI更新传感器状态
///     
/// @param[in]  deviceInfo 设备信息
/// @param[out]  mapStatus mapStatus key-索引， value-状态
///
/// @par History:
/// @li 5220/SunChangYan，2024年8月2日，新建函数
///
void UiControlHandler::NotifySensorStatusUpdate(const ::tf::SubDeviceInfo& deviceInfo, const std::map<int32_t, ::tf::EmSensorResultType::type> & mapStatus)
{
    QMap<int, tf::EmSensorResultType::type> qtMapStatuss;
    for (auto& p : mapStatus)
    {
        qtMapStatuss.insert(p.first, p.second);
    }

	POST_MESSAGE(MSG_ID_SENSOR_STATUS_UPDATE, deviceInfo, qtMapStatuss);
}

///
/// @brief 通知UI试剂/耗材更换日志更新
///
/// @param[in]  ccl  日志信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年2月1日，新建函数
///
void UiControlHandler::NotifyConsumableChangeLog(const ::tf::ConsumableChangeLog& ccl)
{
	POST_MESSAGE(MSG_ID_CONSUMABLE_CHANGE_LOG_UPDATE, const_cast<::tf::ConsumableChangeLog&>(ccl));
}

void UiControlHandler::NotifyDevServiceLefTime(const int32_t totalSeconds, const std::map<std::string, int32_t> & sn2seconds)
{
	QMap<QString, int> qtSn2seconds;
	for (auto& p : sn2seconds)
	{
		qtSn2seconds[p.first.c_str()] = p.second;
	}

	POST_MESSAGE(MSG_ID_DEV_COUNT_DOWN_UPDATE, totalSeconds, qtSn2seconds);
}

void UiControlHandler::NotifyAutoWeekMaintain(const std::string& devSN)
{
    POST_MESSAGE(MSG_ID_UPDATE_STOP_MAINTAIN_TYPE, QString::fromStdString(devSN), (int)tf::MaintainGroupType::MAINTAIN_GROUP_WEEK);
}

///
/// @brief  获取维护组名称
///
/// @param[in]  _return  维护组名称
/// @param[in]  groupId  维护组id
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
///
void UiControlHandler::GetMaintainGroupName(std::string& _return, const int64_t groupId)
{
	::tf::MaintainGroupQueryResp ret;
	::tf::MaintainGroupQueryCond mgqc;
	mgqc.__set_ids({ groupId });
	if (!DcsControlProxy::GetInstance()->QueryMaintainGroup(ret, mgqc))
	{
		return;
	}

	if (ret.lstMaintainGroups.empty())
	{
		return;
	}

	::tf::MaintainGroup group = ret.lstMaintainGroups.front();
	QString groupName;
	switch (group.groupType)
	{
	case ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM:
		groupName = QString::fromStdString(group.groupName);
		break;
	case ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE:
		groupName = (!group.__isset.items || group.items.empty()) ? "" : ConvertTfEnumToQString(group.items.front().itemType);
		break;
	default:
		groupName = ConvertTfEnumToQString(group.groupType);
		break;
	}

	_return = groupName.toStdString();
}

///
/// @brief  获取维护项目名称
///
/// @param[in]  _return   维护项目名称
/// @param[in]  itemType  维护项目类型
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
///
void UiControlHandler::GetMaintainItemName(std::string& _return, const::tf::MaintainItemType::type itemType)
{
	_return = ConvertTfEnumToQString(itemType).toStdString();
}

void UiControlHandler::NotifyRgtAlarmReadFlagChange(const std::string& devSn, const bool readed)
{
	ULOG(LOG_INFO, "%s(devSn=%s,readed=%d)", __FUNCTION__, devSn, readed);

	POST_MESSAGE(MSG_ID_RGT_ALARM_READ_FLAG_UPDATE, QString::fromStdString(devSn), readed);
}
