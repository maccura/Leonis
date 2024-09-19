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
/// @file     I6000LogicControlProxy.cpp
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
#include "I6000LogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"
#include "src/dcs/interface/im/i6000/I6000LogicControlHandler.h"
#include "src/thrift/im/i6000/gen-cpp/i6000_logic_control_constants.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <stdexcept>

///
/// @brief
///     端口号
///
unsigned short                      im::i6000::LogicControlProxy::m_usPort = DCS_CONTROL_PORT;

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
std::shared_ptr<im::i6000::tf::I6000LogicControlClient> im::i6000::LogicControlProxy::GetI6000LogicControlClient()
{
	auto pTSocket = new apache::thrift::transport::TSocket("localhost", m_usPort);
	// 设置连接、发送，接收超时时间(单位ms) add by Chenjianlin 20240713
	pTSocket->setConnTimeout(20000);
	pTSocket->setRecvTimeout(20000);
	pTSocket->setSendTimeout(20000);
	std::shared_ptr<apache::thrift::transport::TTransport> spSocket(pTSocket);
    std::shared_ptr<apache::thrift::transport::TTransport> spTransport(new apache::thrift::transport::TFramedTransport(spSocket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> spProtocol(new apache::thrift::protocol::TCompactProtocol(spTransport));
    std::shared_ptr<apache::thrift::protocol::TMultiplexedProtocol> spMprotocol(new apache::thrift::protocol::TMultiplexedProtocol(spProtocol, im::i6000::tf::g_i6000_logic_control_constants.SERVICE_NAME));
    std::shared_ptr<im::i6000::tf::I6000LogicControlClient> spClient(new im::i6000::tf::I6000LogicControlClient(spMprotocol));

    // 连接服务器
    try
    {
        spTransport->open();
    }
    catch (std::exception& ex)
    {
        ULOG(LOG_ERROR, "thrift error: %s", ex.what());
        return nullptr;
    }
    catch (...)
    {
        ULOG(LOG_ERROR, "thrift error: catch unknown exception");
        return nullptr;
    }

    return spClient;
}

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
bool im::i6000::LogicControlProxy::QueryAssayTestResult(im::tf::AssayTestResultQueryResp& qryResp, im::tf::AssayTestResultQueryCond& qryCond)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.QueryAssayTestResult(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

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
bool im::i6000::LogicControlProxy::CheckLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.CheckLoadReagent(_return, deviceSN, pos);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  判断试剂仓盖打开后是否执行试剂扫描
///
/// @param[in]    deviceSN:设备序列号
///
/// @return true:执行过试剂扫描 false:未执行试剂扫描
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年5月29日，新建函数
///
bool im::i6000::LogicControlProxy::CheckReagentCoverReagentScan(const std::string& deviceSN, bool& HasReagentScan)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	HasReagentScan = logicControl.QueryReagentCoverReagentScan(deviceSN);

    return HasReagentScan;
}

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
bool im::i6000::LogicControlProxy::LoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.LoadReagent(_return, deviceSN, pos);

	return true;
}

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
bool im::i6000::LogicControlProxy::CancelLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.CancelLoadReagent(_return, deviceSN);

	return true;
}

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
bool im::i6000::LogicControlProxy::CheckUnLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos /*= -1*/)
{
	ULOG(LOG_INFO, "%s(, %s, %d)", __FUNCTION__, deviceSN, pos);

	im::i6000::LogicControlHandler logicControl;
	logicControl.CheckUnLoadReagent(_return, deviceSN, pos);

	return true;
}

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
bool im::i6000::LogicControlProxy::UnLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN, const int32_t pos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.UnLoadReagent(_return, deviceSN, pos);

	return true;
}

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
bool im::i6000::LogicControlProxy::CancelUnLoadReagent(::tf::ResultLong& _return, const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.CancelUnLoadReagent(_return, deviceSN);

	return true;
}

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
bool im::i6000::LogicControlProxy::SetReagentMask(::im::i6000::tf::ResultReagent& _return, const  ::im::tf::ReagentInfoTable& reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetReagentMask(_return, reag);

	return true;
}

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
bool im::i6000::LogicControlProxy::SetReagentPreUse(::tf::ResultLong& _return, const  ::im::tf::ReagentInfoTable& reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetReagentPreUse(_return, reag);

	return true;
}

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
bool im::i6000::LogicControlProxy::SetDiluentMask(::tf::ResultLong& _return, const ::im::tf::DiluentInfoTable& dilu)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetDiluentMask(_return, dilu);

	return true;
}

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
bool im::i6000::LogicControlProxy::SetDiluentPreUse(::tf::ResultLong& _return, const ::im::tf::DiluentInfoTable& dilu)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetDiluentPreUse(_return, dilu);

	return true;
}

///
/// @brief 切换试剂校准设置
///
/// @param[in]  _return  返回值
/// @param[in]  reag     试剂信息
///
/// @par History:
/// @li 1226/zhangjing，2022年10月21日，新建函数
///
bool im::i6000::LogicControlProxy::SetReagentCaliSel(::tf::ResultLong& _return, const  ::im::tf::ReagentInfoTable& reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetReagentCaliSel(_return, reag);

	return true;
}

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
bool im::i6000::LogicControlProxy::SetReagentScan(::tf::ResultLong& _return, const std::string& deviceSN, const std::vector<int32_t>& pos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetReagentScan(_return, deviceSN, pos);

	return true;
}

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
bool im::i6000::LogicControlProxy::SetVortexReagent(::tf::ResultLong& _return, const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetVortexReagent(_return, deviceSN);

	return true;
}

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
bool im::i6000::LogicControlProxy::ClearReagentCaliMask(::tf::ResultLong& _return, const  ::im::tf::ReagentInfoTable& reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.ClearReagentCaliMask(_return, reag);

	return true;
}

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
bool im::i6000::LogicControlProxy::QueryCaliCurve(::im::tf::CaliCurveQueryResp& _return, const ::im::tf::CaliCurveQueryCond& ccqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.QueryCaliCurve(_return, ccqc);

	return true;
}

bool im::i6000::LogicControlProxy::ModifyCaliCurve(const ::im::tf::CaliCurve& caliCurv)
{
    im::i6000::LogicControlHandler logicControl;
    return logicControl.ModifyCaliCurve(caliCurv);
}

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
bool im::i6000::LogicControlProxy::GetReagentCurveInfo(::im::tf::CaliCurveQueryResp & _return, const::im::tf::ReagentInfoTable & reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.GetReagentCurveInfo(_return, reag);

	return true;
}

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
bool im::i6000::LogicControlProxy::GetHistoryCurveInfo(::im::tf::CaliCurveQueryResp& _return, const std::string& deviceSN, const int32_t assayCode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.GetHistoryCurveInfo(_return, deviceSN, assayCode);

	return true;
}

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
bool im::i6000::LogicControlProxy::UpdateCurentCurve(const  ::im::tf::CaliCurve& curve, const  ::im::tf::ReagentInfoTable& reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.UpdateCurentCurve(curve, reag);

	return true;
}


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
bool im::i6000::LogicControlProxy::QueryMaterialStatus(const std::string& DeviceSn, const im::i6000::tf::MonitorMessageType::type msgType)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	return logicControl.QueryMaterialStatus(DeviceSn, msgType);
}


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
bool im::i6000::LogicControlProxy::ReCalcResult(::im::tf::AssayTestResultQueryResp& _return, const std::vector< ::im::tf::AssayTestResult> & lsResult, const::im::tf::ReagentInfoTable& reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.ReCalcResult(_return, lsResult, reag);

	return true;
}


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
bool im::i6000::LogicControlProxy::QueryResultsUnCalc(::im::tf::AssayTestResultQueryResp& _return, const  ::im::tf::ReagentInfoTable& reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.QueryResultsUnCalc(_return, reag);

	return true;
}

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
bool im::i6000::LogicControlProxy::ModifyAssayTestResult(const ::im::tf::AssayTestResult& rt)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	return logicControl.ModifyAssayTestResult(rt);
}

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
bool im::i6000::LogicControlProxy::SetSupplyPreUse(::tf::ResultLong& _return, const ::im::tf::SuppliesInfoTable& supply)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.SetSupplyPreUse(_return, supply);

	return true;
}

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
bool im::i6000::LogicControlProxy::CheckLoadUnloadIsMask(::tf::ResultLong& _return, const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.CheckLoadUnloadIsMask(_return, deviceSN);

	return true;
}

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
bool im::i6000::LogicControlProxy::ManualHandlReag(::tf::ResultBool& _return, const std::string& deviceSN, bool bChangeReag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.ManualHandlReag(_return, deviceSN, bChangeReag);

	return true;
}

///
/// @brief 查询该试剂的工作曲线
///
/// @param[in]  _return  返回值
/// @param[in]  reag  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2024年1月25日，新建函数
///
bool im::i6000::LogicControlProxy::GetReagentCurrentCurveInfo(::im::tf::CaliCurveQueryResp & _return, const::im::tf::ReagentInfoTable & reag)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::i6000::LogicControlHandler logicControl;
	logicControl.GetReagentCurrentCurveInfo(_return, reag);

	return true;
}

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
bool im::i6000::LogicControlProxy::ResetReagentLoader(::tf::ResultLong& _return, const std::string& deviceSN)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    im::i6000::LogicControlHandler logicControl;
    logicControl.ResetReagentLoader(_return, deviceSN);

    return true;
}
