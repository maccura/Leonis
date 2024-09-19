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
/// @file     I6000UiControlHandler.h
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

#include "src/thrift/im/i6000/gen-cpp/i6000UiControl.h"

namespace im
{
namespace i6000
{
///
/// @brief
///     处理UI控制命令的类
///
class I6000UiControlHandler : public tf::i6000UiControlIf
{
public:
	
	///
	/// @brief 通知UI试剂发生变化
	///
	/// @param[in]  reag	试剂信息
	/// @param[in]  type	变化类型
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月7日，新建函数
	///
	virtual void ReportReagentChanged(const im::tf::ReagentInfoTable& reag, const im::tf::ChangeType::type type) override;

	///
	/// @brief 通知UI稀释液发生变化
	///
	/// @param[in]  dilu	稀释液信息
	/// @param[in]  type	变化类型
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月7日，新建函数
	///
	virtual void ReportDiluentChanged(const im::tf::DiluentInfoTable& dilu, const im::tf::ChangeType::type type) override;

	///
	/// @brief 通知UI耗材发生变化
	///
	/// @param[in]  supply	耗材信息
	/// @param[in]  type	变化类型
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月7日，新建函数
	///
	virtual void ReportSupplyChanged(const im::tf::SuppliesInfoTable& supply, const im::tf::ChangeType::type type) override;

	///
	/// @brief 通知UI项目发生变化
	///
	/// @param[in]  assayCode	项目通道号
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月5日，新建函数
	///
	virtual void ReportAssayChanged(const int32_t assayCode) override;

	///
	///  @brief:	上报检测信号值
	///
	///  @param[in]   sigValue  信号值
	///  @param[in]   mit  维护类型
	///
	///  @return	:
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年7月27日，新建函数
	///
	virtual void ReportDetectSignal(const  ::tf::MaintainItemType::type mit, const double sigValue) override;

	///
	/// @brief  上报质控申请结束
	///
	/// @param[in]  qa  质控申请
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年11月4日，新建函数
	///
	virtual void ReportQcApplyDel(const ::im::tf::QcApply& qa) override;

    ///
    /// @brief  上报校准完成
    ///
    /// @param[in]  curve  生成曲线
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年8月28日，新建函数
    ///
    virtual void ReportCaliEnd(const  ::im::tf::CaliCurve& curve) override;

    ///
    /// @brief
    ///     通知温度异常
    ///
    /// @param[in]  deviceSN    设备序列号
    /// @param[in]  bErr        温度是否异常
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月30日，新建函数
    ///
    virtual void ReportTemperatureErr(const std::string& deviceSN, const bool isErr) override;

	///
	/// @brief 通知UI校准组发生变化 
	///
	/// @param[in]  cdg  校准组
	/// @param[in]  type  类型
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月23日，新建函数
	///
	virtual void ReportCaliGrpChanged(const im::tf::CaliDocGroup& cdg, const im::tf::ChangeType::type type) override;

	///
	/// @brief 上报试剂在线加载机构是否正在工作
	///
	/// @param[in]  deviceSN	设备编号
	/// @param[in]  isRunning	是否正在工作
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月24日，新建函数
	///
	virtual void ReportReagentLoadStatus(const std::string& deviceSN, const bool isRunning) override;

    ///
    /// @brief 上报装载装置复位执行结果
    ///
    /// @param[in]  deviceSN	设备编号
    /// @param[in]  rlt	        执行结果
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2024年7月17日，新建函数
    ///
    virtual void ReportReagentLoaderResetRlt(const std::string& deviceSN, const  ::im::i6000::tf::ReagentLoaderResetRlt::type rlt) override;

	///
	/// @brief 上报拿走试剂在线加载机构试剂槽上的试剂盒
	///
	/// @param[in]  deviceSN	设备编号
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年9月4日，新建函数
	///
	virtual void ReportTakeReagentMessage(const std::string& deviceSN) override;

	///
	/// @brief 通知UI继续测试(加样停、待机下)
	///
	/// @param[in]  deviceSN  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月10日，新建函数
	///
	virtual void NotifyUIContinueRun(const std::string& deviceSN) override;

	///
	/// @brief 通知UI申请更换试剂结果
	///
	/// @param[in]  deviceSN	设备编号
	/// @param[in]  result		申请更换试剂结果，0-取消，1-接受，2-结束
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月11日，新建函数
	///
	virtual void ReportManualHandlReagResult(const std::string& deviceSN, const int result) override;

	///
	/// @brief 获取免疫耗材的名称
	///
	/// @param[in]  _return  
	/// @param[in]  supType  
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年1月25日，新建函数
	///
	virtual void GetSuppliesTypeName(std::string& _return, const  ::im::tf::SuppliesType::type supType) override;

    ///
    /// @brief 返回调试流程执行结果
    ///
    /// @param[in]  _return  
    /// @param[in]  Process  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年3月28日，新建函数
    ///
    virtual bool ReportExecuteDebugProResult(const  ::im::tf::DebugProcess& Process) override;
};
}
}
