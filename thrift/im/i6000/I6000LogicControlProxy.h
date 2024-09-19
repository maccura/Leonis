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
/// @file     I6000LogicControlProxy.h
/// @brief    发送I6000业务逻辑控制命令的代理
///
/// @author   3558/ZhouGuangMing
/// @date     2021年4月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年4月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/thrift/im/i6000/gen-cpp/I6000LogicControl.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

namespace im
{
namespace i6000
{

///
/// @brief
///     发送I6000业务逻辑控制命令的代理
///
class LogicControlProxy : public boost::noncopyable
{
public:
    ///
    /// @brief
    ///     获取thift客户端
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月29日，新建函数
    ///
    static std::shared_ptr<im::i6000::tf::I6000LogicControlClient> GetI6000LogicControlClient();

    ///
    /// @brief
    ///     查询样本结果
    ///
    /// @param[in]  qryResp  项目测试结果
    /// @param[in]  qryCond  样本查找条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年8月5日，新建函数
    ///
    static bool QueryAssayTestResult(im::tf::AssayTestResultQueryResp& qryResp, im::tf::AssayTestResultQueryCond& qryCond);

	///
	/// @brief 加载试剂前的判断
	///
	/// @param[in]  _return		返回值
	/// @param[in]  deviceSN	设备编号
	/// @param[in]  pos			加载位置 
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年8月10日，新建函数
	///
	static bool CheckLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos = -1);

    ///
    /// @brief  判断试剂仓盖打开后是否执行试剂扫描
    ///
    /// @param[in]    deviceSN:设备序列号 HasReagentScan:是否执行试剂扫描
    ///
    /// @return true:执行过试剂扫描 false:未执行试剂扫描
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年5月29日，新建函数
    ///
    static bool CheckReagentCoverReagentScan(const std::string& deviceSN, bool& HasReagentScan);

	///
	/// @brief 试剂自动加载
	///
	/// @param[in]  _return		返回值
	/// @param[in]  deviceSN	设备编号
	/// @param[in]  pos			加载位置
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年8月10日，新建函数
	///
	static bool LoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos);

	///
	/// @brief 取消加载试剂
	///
	/// @param[in]  _return		返回值
	/// @param[in]  deviceSN	设备编号
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年8月11日，新建函数
	///
	static bool CancelLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN);

	///
	/// @brief 卸载试剂前的判断
	///
	/// @param[in]  _return		返回值
	/// @param[in]  deviceSN	设备编号
	/// @param[in]  pos			加载位置 
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月16日，新建函数
	///
	static bool CheckUnLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos);

	///
	/// @brief 卸载试剂
	///
	/// @param[in]  _return		返回值
	/// @param[in]  deviceSN	设备编号
	/// @param[in]  pos			载位置 
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年9月9日，新建函数
	///
	static bool UnLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos);

	///
	/// @brief 取消卸载试剂
	///
	/// @param[in]  _return		返回值
	/// @param[in]  deviceSN	设备编号
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年9月9日，新建函数
	///
	static bool CancelUnLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN);

	///
	/// @brief 设置试剂屏蔽
	///
	/// @param[in]  _return	返回结果
	/// @param[in]  id		试剂数据库ID
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年9月9日，新建函数
	///
	static bool SetReagentMask(::im::i6000::tf::ResultReagent& _return, const  ::im::tf::ReagentInfoTable& reag);

	///
	/// @brief 设置试剂优先使用
	///
	/// @param[in]  _return  
	/// @param[in]  reag  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年9月9日，新建函数
	///
	static bool SetReagentPreUse(::tf::ResultLong& _return, const  ::im::tf::ReagentInfoTable& reag);

    ///
    /// @brief
    ///     稀释液屏蔽
    ///
    /// @param[in]  _return   返回值
    /// @param[in]  dilu      稀释液信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月28日，新建函数
    ///
    static bool SetDiluentMask(::tf::ResultLong& _return, const  ::im::tf::DiluentInfoTable& dilu);

    ///
    /// @brief
    ///     稀释液优先使用
    ///
    /// @param[in]  _return   返回值
    /// @param[in]  dilu      稀释液信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月28日，新建函数
    ///
    static bool SetDiluentPreUse(::tf::ResultLong& _return, const  ::im::tf::DiluentInfoTable& dilu);

	///
	/// @brief 切换试剂校准设置
	///
	/// @param[in]  _return  返回值
	/// @param[in]  reag     试剂信息
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年10月21日，新建函数
	///
	static bool SetReagentCaliSel(::tf::ResultLong& _return, const  ::im::tf::ReagentInfoTable& reag);

	///
	/// @brief 设置试剂扫描
	///
	/// @param[in]  _return  
	/// @param[in]  deviceSN  
	/// @param[in]  pos  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年9月9日，新建函数
	///
	static bool SetReagentScan(::tf::ResultLong& _return, const std::string& deviceSN, const std::vector<int32_t>& pos);

    ///
    /// @brief
    ///     试剂磁珠混匀
    ///
    /// @param[in]  _return   返回值
    /// @param[in]  deviceSN  设备序列号
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月28日，新建函数
    ///
    static bool SetVortexReagent(::tf::ResultLong& _return, const std::string& deviceSN);

    ///
    /// @brief 解除校准屏蔽
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  reag  试剂信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    static bool ClearReagentCaliMask(::tf::ResultLong& _return, const  ::im::tf::ReagentInfoTable& reag);

    ///
    /// @brief 查询校准曲线
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  reag  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    static bool QueryCaliCurve(::im::tf::CaliCurveQueryResp& _return, const  ::im::tf::CaliCurveQueryCond& ccqc);

    ///
    /// @bref
    ///		修改校准曲线
    ///
    /// @param[in] caliCurv 最新的校准曲线
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月11日，新建函数
    ///
    static bool ModifyCaliCurve(const ::im::tf::CaliCurve& caliCurv);

    ///
    /// @brief 查询满足该试剂条件的校准曲线（瓶曲线和批曲线）
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  reag  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    static bool GetReagentCurveInfo(::im::tf::CaliCurveQueryResp& _return, const  ::im::tf::ReagentInfoTable& reag);

    ///
    /// @brief 查询该仪器该项目的历史校准曲线
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  deviceSN  设备序列号
    /// @param[in]  assayCode  项目编号
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    static bool GetHistoryCurveInfo(::im::tf::CaliCurveQueryResp& _return, const std::string& deviceSN, const int32_t assayCode);

    ///
    /// @brief 更新试剂当前曲线
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  reag  试剂信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    static bool UpdateCurentCurve(const  ::im::tf::CaliCurve& curve, const  ::im::tf::ReagentInfoTable& reag);

    ///
    /// @brief  查询耗材状态
    ///
    /// @param[in]    DeviceSn:设备序列号
    /// @param[in]    msgType:耗材类型
    ///
    /// @return bool
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年1月3日，新建函数
    ///
    static bool QueryMaterialStatus(const std::string& DeviceSn ,const im::i6000::tf::MonitorMessageType::type  msgType);

	///
	/// @brief  重新计算结果
	///
	/// @param[in]  _return  计算完成结果
	/// @param[in]  lsResult  需要计算结果
	/// @param[in]  reag      配套试剂
	///
	/// @return true成功
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年1月17日，新建函数
	///
	static bool ReCalcResult(::im::tf::AssayTestResultQueryResp& _return, const std::vector< ::im::tf::AssayTestResult> & lsResult, const::im::tf::ReagentInfoTable& reag);

	///
	/// @brief  获取该试剂中未计算的结果
	///
	/// @param[in]  _return  结果列表
	/// @param[in]  reag     试剂
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年1月18日，新建函数
	///
	static bool QueryResultsUnCalc(::im::tf::AssayTestResultQueryResp& _return, const  ::im::tf::ReagentInfoTable& reag);

	///
	/// @brief  修改结果
	///
	/// @param[in]  rt  结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月1日，新建函数
	///
	static bool ModifyAssayTestResult(const ::im::tf::AssayTestResult& rt);

	///
	/// @brief 设置耗材优先使用
	///
	/// @param[in]  _return		是否成功
	/// @param[in]  supply		耗材信息	
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年5月4日，新建函数
	///
	static bool SetSupplyPreUse(::tf::ResultLong& _return, const  ::im::tf::SuppliesInfoTable& supply);

	///
	/// @brief 检查在线加载是否已经被屏蔽
	///
	/// @param[in]  _return  
	/// @param[in]  deviceSN  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月11日，新建函数
	///
	static bool CheckLoadUnloadIsMask(::tf::ResultLong& _return, const std::string& deviceSN);

	///
	/// @brief 用户申请在线加载
	///
	/// @param[in]  _return     执行结果
	/// @param[in]  deviceSN    涉笔序列号
    /// @param[in]  bChangeReag true-申请更换 false-取消申请
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月11日，新建函数
    /// @li 4170/TangChuXian，2024年5月16日，增加参数bChangeReag
	///
	static bool ManualHandlReag(::tf::ResultBool& _return, const std::string& deviceSN, bool bChangeReag = true);

    ///
    /// @brief 查询满足该试剂的当前曲线
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  reag  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 1226/zhangjing，2024年1月25日，新建函数
    ///
    static bool GetReagentCurrentCurveInfo(::im::tf::CaliCurveQueryResp& _return, const  ::im::tf::ReagentInfoTable& reag);

    ///
    /// @brief 装载装置复位
    ///
    /// @param[in]  _return		返回值
    /// @param[in]  deviceSN	设备编号
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月17日，新建函数
    ///
    static bool ResetReagentLoader(::tf::ResultLong& _return, const std::string& deviceSN);
public:
    static unsigned short           m_usPort;                   ///< 端口号
};

}
}
