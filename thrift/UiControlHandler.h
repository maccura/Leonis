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
/// @file     UiControlHandler.h
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
#pragma once

#include "src/thrift/gen-cpp/UiControl.h"

///
/// @brief
///     处理UI控制命令的类
///
class UiControlHandler : public tf::UiControlIf
{
public:
    ///
    /// @brief
    ///     处理设备状态信息上报
    ///
    /// @param[in]  deviceInfo  设备状态信息列表
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
    ///
    virtual void NotifyDeviceStatus(const ::tf::DeviceInfo& deviceInfo) override;

    ///
    /// @brief  通知是否可以追加测试样本
    ///
    /// @param[in]  devSN   设备序列号
    /// @param[in]  faats   影响开始（追加）测试样本的异常
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2023年3月21日，新建函数
    ///
    virtual void NotifyCanAppendTest(const std::string& devSN, const std::set<::tf::FaultAffectAppendTest::type>& faats) override;

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
    virtual void NotifySampleUpdate(const ::tf::UpdateType::type enUpdateType, const std::vector<::tf::SampleInfo> & lstSIs) override;

    ///
    /// @brief  处理测试项目更新
    ///
    /// @param[in]  enUpdateType    更新类型
    /// @param[in]  lstTestItems    更新的测试项目信息列表
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2024年1月7日，新建函数
    ///
    virtual void NotifyTestItemUpdate(const ::tf::UpdateType::type enUpdateType, const std::vector<::tf::TestItem>& lstTestItems) override;

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
	virtual void NotifySampleLisUpdate(const int64_t sampleID) override;

	///
	/// @brief
	///     处理告警信息更新
	///
	/// @param[in]  alarmDesc  告警描述信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月15日，新建函数
	///
	virtual void NotifyDeviceFaultInfo(const  ::tf::AlarmDesc& alarmDesc) override;

    ///
    /// @brief
    ///     当前告警消失
    ///
    /// @param[in]  alarmDesc  告警描述信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月18日，新建函数
    ///
    virtual void NotifyDisCurrentAlarm(const  ::tf::AlarmDesc& alarmDesc) override;

    ///
    /// @brief
    ///     已有的告警信息详情更新
    ///
    /// @param[in]  alarmDesc  告警描述信息
    /// @param[in]  increaseParams  是否是详情中的参数增加
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年08月07日，新建函数
    ///
    virtual void NotifyUpdateCurrentAlarm(const  ::tf::AlarmDesc& alarmDesc, const bool increaseParams)override;

    ///
    /// @brief
    ///     通知UI已读的告警信息
    ///
    /// @param[in]  alarmDescs  已读的告警信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年2月24日，新建函数
    ///
    virtual void NotifyAlarmReaded(const std::vector< ::tf::AlarmDesc> & alarmDescs) override;

    ///
    /// @brief 质控文档信息变更通知
    ///
    /// @param[in]  infos  更新的质控文档信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年10月20日，新建函数
    ///
    virtual void NotifyQcDocInfoUpdate(const std::vector<::tf::QcDocUpdate>& infos) override;

    ///
    /// @brief 处理设备状态信息更新
    ///
    /// @param[in]    statusInfo:设备状态信息
    /// 
    /// @par History:
    /// @li 7951/LuoXin，2023年2月13日，新建函数
    ///
	virtual void NotifyUnitStatusInfo(const ::tf::UnitStatusInfo& statusInfo) override;

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
	virtual void ReportMaintainItemInfo(const std::string& devSN, const int64_t groupId, const ::tf::MaintainItemExeResult& mier) override;

	///
	/// @brief	维护组执行信息更新
	///     
	///
	/// @param[in]  lmi  维护组执行信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年3月9日，新建函数
	///
	virtual void ReportMaintainGroupInfo(const  ::tf::LatestMaintainInfo& lmi) override;

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
    virtual void ReportMaintainDataInit(const std::string& devSN) override;

    ///
    /// @brief  通知UI报警屏蔽已清空
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年9月5日，新建函数
    ///
    virtual void NotifyAlarmShieldClearAll() override;

	///
	/// @brief  处理Lis连接状态更新
	///
	/// @param[in]  status  true表示正常连接
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月21日，新建函数
	///
	virtual void NotifyLisConnectionStatus(const bool status) override;

	///
	/// @brief 通知样本架回收消息变更
	///
	/// @param[in]  rack			样本架架号信息
	/// @param[in]  recycleRackInfo 样本架回收信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月24日，新建函数
	///
	//virtual void NotifyRecycleRackUpdate(const std::string& rack, const std::map<int32_t, int64_t> & recycleRackInfo) override;
	virtual void NotifyRecycleRackUpdate(const int32_t trayNo, const std::vector< ::tf::RecycleRackInfo> & rackInfo) override;

    ///
    /// @bref
    ///		通知界面，质控结果发生变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月13日，新建函数
    ///
    virtual void NotifyQcResultUpdate(const std::string& devceSn, const int64_t qcDocId, const std::vector<int64_t> & updatedId) override;

	///
	/// @brief 通知轨道状态更新
	///
	/// @param[in] statusInfo: 轨道状态信息
	///
	/// @par History:
	/// @li 6950/ChenFei，2023/12/26，create the function
	///
	virtual void NotifyTrackStatusUpdate(const  ::track::tf::LowerStatusInfo& statusInfo) override;

	///
	/// @brief 通知UI更新传感器状态
	///     
	/// @param[in]  deviceInfo 设备信息
	/// @param[out]  mapStatus mapStatus key-索引， value-状态
	///
	/// @par History:
	/// @li 5220/SunChangYan，2024年8月2日，新建函数
	///
	virtual void NotifySensorStatusUpdate(const ::tf::SubDeviceInfo& deviceInfo, const std::map<int32_t, ::tf::EmSensorResultType::type> & mapStatus) override;

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
	virtual void NotifyConsumableChangeLog(const  ::tf::ConsumableChangeLog& ccl) override;


	///
	/// @brief 通知UI设备当前任务(测试或维护)剩余时间
	///
	/// @param[in] totalSeconds: 整个流水线或单机的事务完成剩余秒数
	/// @param[in] sn2seconds key:设备SN, val:剩余时间(单位秒)
	///
	/// @return void: true means success 
	///
	/// @par History:
	/// @li 6950/chenfei，2024/02/28，create the function
	///
	virtual void NotifyDevServiceLefTime(const int32_t totalSeconds, const std::map<std::string, int32_t> & sn2seconds) override;

    ///
    /// @bref
    ///		通知关机管理类，更新指定设备的关机维护类型
    ///
    /// @param[in] devSN 指定设备
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年7月8日，新建函数
    ///
    virtual void NotifyAutoWeekMaintain(const std::string& devSN);
	
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
	virtual void GetMaintainGroupName(std::string& _return, const int64_t groupId) override;
	
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
	virtual void GetMaintainItemName(std::string& _return, const  ::tf::MaintainItemType::type itemType) override;

	///
	/// @brief  通知UI试剂耗材余量不足报警已读标志更新
	///
	/// @param[in]  devSn    设备序列号
	/// @param[in]  readed   true表示已读
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年08月01日，新建函数
	///
	virtual void NotifyRgtAlarmReadFlagChange(const std::string& devSn, const bool readed) override;
};
