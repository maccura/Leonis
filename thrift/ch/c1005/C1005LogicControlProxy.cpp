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
/// @file     C1005LogicControlProxy.cpp
/// @brief    发送C1005业务逻辑控制命令的代理
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
#include "C1005LogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"
#include "src/dcs/interface/ch/c1005/C1005LogicControlHandler.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_logic_control_constants.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <stdexcept>

///
/// @brief
///     端口号
///
unsigned short                      ch::c1005::LogicControlProxy::m_usPort = DCS_CONTROL_PORT;

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
std::shared_ptr<ch::c1005::tf::C1005LogicControlClient> ch::c1005::LogicControlProxy::GetC1005LogicControlClient()
{
    std::shared_ptr<apache::thrift::transport::TTransport> spSocket(new apache::thrift::transport::TSocket("localhost", m_usPort));
    std::shared_ptr<apache::thrift::transport::TTransport> spTransport(new apache::thrift::transport::TFramedTransport(spSocket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> spProtocol(new apache::thrift::protocol::TCompactProtocol(spTransport));
    std::shared_ptr<apache::thrift::protocol::TMultiplexedProtocol> spMprotocol(new apache::thrift::protocol::TMultiplexedProtocol(spProtocol, ch::c1005::tf::g_c1005_logic_control_constants.SERVICE_NAME));
    std::shared_ptr<ch::c1005::tf::C1005LogicControlClient> spClient(new ch::c1005::tf::C1005LogicControlClient(spMprotocol));

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
///     查询项目测试结果
///
/// @param[in]  qryResp  项目测试结果
/// @param[in]  qryCond  样本查找条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月14日，新建函数
///
bool ch::c1005::LogicControlProxy::QueryAssayTestResult(ch::tf::AssayTestResultQueryResp& qryResp
	, ch::tf::AssayTestResultQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    logicControl.QueryAssayTestResult(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}
    
    return true;
}

///
/// @brief
///     修改项目测试结果
///
/// @param[in]  tr  项目测试结果
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月14日，新建函数
///
bool ch::c1005::LogicControlProxy::ModifyAssayTestResult(ch::tf::AssayTestResult& tr)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.ModifyAssayTestResult(tr);
}

///
/// @brief
///     查询满足条件的耗材信息
///
/// @param[in]  qryResp  耗材结果
/// @param[in]  qryCond  耗材查找条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年8月26日，新建函数
///
bool ch::c1005::LogicControlProxy::QuerySuppliesInfo(ch::tf::SuppliesInfoQueryResp& qryResp
	, ch::tf::SuppliesInfoQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	logicControl.QuerySuppliesInfo(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     屏蔽指定位置的耗材
///
/// @param[in]  siqc  指定耗材条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月18日，新建函数
///
bool ch::c1005::LogicControlProxy::MaskReagentGroups(const std::vector<::ch::tf::SuppliesInfoQueryCond>& siqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.MaskReagentGroups(siqc);
}

///
/// @brief
///     解除屏蔽指定位置的耗材
///
/// @param[in]  siqc  指定耗材条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月18日，新建函数
///
bool ch::c1005::LogicControlProxy::UnMaskReagentGroups(const std::vector< ::ch::tf::SuppliesInfoQueryCond>& siqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.UnMaskReagentGroups(siqc);
}

///
/// @brief
///     解除屏蔽校准
///
/// @param[in]  siqc  指定耗材条件
///
/// @return true表示成功
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月6日，新建函数
///
bool ch::c1005::LogicControlProxy::UnMaskCalibrate(const std::vector<::ch::tf::ReagentGroupQueryCond>& siqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	return logicControl.UnMaskCalibrate(siqc);
}

///
/// @brief
///     优先使用指定位置的试剂
///
/// @param[in]  siqc  指定试剂条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月11日，新建函数
///
bool ch::c1005::LogicControlProxy::PriorUsingReagents(const std::vector< ::ch::tf::SuppliesInfoQueryCond>& siqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.PriorUsingReagents(siqc);
}

///
/// @brief
///     优先使用指定位置的耗材
///
/// @param[in]  siqc  指定耗材条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月18日，新建函数
///
bool ch::c1005::LogicControlProxy::PriorUsingSupplies(const std::vector< ::ch::tf::SuppliesInfoQueryCond>& siqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.PriorUsingSupplies(siqc);
}

///
/// @brief
///     卸载指定位置的耗材
///
/// @param[in]  deviceSN    设备序列号
/// @param[in]  pos         待检测的位置列表
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月18日，新建函数
///
bool ch::c1005::LogicControlProxy::UnloadSupplies(const std::string& deviceSN, const std::vector<int32_t>& pos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.UnloadSupplies(deviceSN, pos);
}

///
/// @brief
///     扫描指定位置的耗材的信息
///
/// @param[in]  deviceSN        设备编号
/// @param[in]  pos             设备位置号
/// @param[in]  probeResidual   是否探测余量
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月18日，新建函数
///
bool ch::c1005::LogicControlProxy::ScanSuppliesInfo(const std::string& deviceSN
	, const std::vector<::ch::tf::SuppliesPosition>& pos,const bool probeResidual, const bool isFastMode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::tf::SuppliesScanParamInfo sspi;
	sspi.__set_posInfo(pos);
	sspi.__set_probeResidual(probeResidual);
	sspi.__set_isFastMode(isFastMode);

	ch::c1005::LogicControlHandler logicControl;
	return logicControl.ScanSuppliesInfo(deviceSN, sspi);
}

///
/// @brief
///     获得试剂加载器的耗材信息
///
/// @param[out]  _return  加载器状态信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月17日，新建函数
///
void ch::c1005::LogicControlProxy::QueryReagentLoader(const std::string& deviceSN, ::ch::tf::ReagentLoaderInfo& _return)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    logicControl.QueryReagentLoader(_return, deviceSN);
}

bool ch::c1005::LogicControlProxy::CancelReagentLoader(const std::string& deviceSN, const std::vector<int32_t>& pos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.CancelReagentLoader(deviceSN, pos);
}

///
/// @brief
///     查询校准曲线
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  ccqc  查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月16日，新建函数
///
void ch::c1005::LogicControlProxy::QueryCaliCurve(::ch::tf::CaliCurveQueryResp& _return
	, const ::ch::tf::CaliCurveQueryCond& ccqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	logicControl.QueryCaliCurve(_return, ccqc);
}

///
/// @brief
///     修改校准曲线列表
///
/// @param[in]  cc  校准曲线列表
///     1：曲线列表里只有1根曲线，批曲线标志位没有被置--修改对比曲线参数
///     2：曲线列表里只有1根曲线，并且批曲线标志位被置--修改批曲线的参数或者设置对比曲线为批曲线
///     3：曲线列表里只有2根曲线，批曲线标志位被置，数据库主键标志位被置--将对比曲线设置为当前批曲线
///     4：曲线列表里只有2根曲线，批曲线标志位未被置，数据库主键标志位未被置--生成两条新的校准曲线
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月16日，新建函数
///
bool ch::c1005::LogicControlProxy::ModifyCaliCurves(const std::vector<::ch::tf::CaliCurve>& cc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.ModifyCaliCurves(cc);
}


///
/// @brief
///     增加校准曲线
///
/// @param[in]  _return  状态返回
/// @param[in]  cc       校准曲线列表
/// 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月10日，新建函数
///
void ch::c1005::LogicControlProxy::AddCaliCurve(::tf::ResultLong& _return, const ::ch::tf::CaliCurve& cc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	logicControl.AddCaliCurve(_return,cc);
}

::tf::StCanResult ch::c1005::LogicControlProxy::SendCanCommand(const std::string& deviceSN, const int dstCanId
	, const ::tf::StCanSendData &data)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	::tf::StCanResult result;
	ch::c1005::LogicControlHandler logicControl;
	logicControl.SendCanCommand(result, deviceSN, dstCanId, data);

	return result;
}

///
/// @brief
///     查询试剂组信息
///
/// @param[in]  qryCond  查询条件
/// @param[in]  qryResp  查询结果
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年9月8日，新建函数
///
bool ch::c1005::LogicControlProxy::QueryReagentGroup(::ch::tf::ReagentGroupQueryResp& qryResp, const ::ch::tf::ReagentGroupQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	logicControl.QueryReagentGroup(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     添加试剂组
///
/// @param[in]  _return  试剂组的返回值
/// @param[in]  rg  试剂组
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建函数
///
void ch::c1005::LogicControlProxy::AddReagentGroup(::tf::ResultLong& _return, const ::ch::tf::ReagentGroup& rg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	logicControl.AddReagentGroup(_return, rg);
}

///
/// @brief
///     删除试剂组
///
/// @param[in]  rgqc  删除试剂组条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建函数
///
bool ch::c1005::LogicControlProxy::DeleteReagentGroup(const ::ch::tf::ReagentGroupQueryCond& rgqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.DeleteReagentGroup(rgqc);
}

///
/// @brief
///     修改试剂组
///
/// @param[in]  rg  试剂组信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建函数
///
bool ch::c1005::LogicControlProxy::ModifyReagentGroup(const ::ch::tf::ReagentGroup& rg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ch::c1005::LogicControlHandler logicControl;
    return logicControl.ModifyReagentGroup(rg);
}

///
/// @brief
///     添加耗材信息
///
/// @param[in]  _return  执行结果
/// @param[in]  si       耗材信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建函数
///
void ch::c1005::LogicControlProxy::AddSuppliesInfo(::tf::ResultLong& _return, const ::ch::tf::SuppliesInfo& si)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	logicControl.AddSuppliesInfo(_return, si);
}

///
/// @brief
///     手动添加耗材信息
///
/// @param[in]  si  耗材信息
///
/// @return 执行结果（成功，失败原因，见数据结构）
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月25日，新建函数
///
::ch::tf::ManualRegisterResult::type ch::c1005::LogicControlProxy::ManualRegisterSupplies(const ::ch::tf::SuppliesInfo& si)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
	return logicControl.ManualRegisterSupplies(si);
}

///
/// @brief
///     删除满足条件的耗材信息
///
/// @param[in]  siqc  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建函数
///
bool ch::c1005::LogicControlProxy::DeleteSuppliesInfo(const ::ch::tf::SuppliesInfoQueryCond& siqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.DeleteSuppliesInfo(siqc);
}

///
/// @brief
///     修改耗材信息
///
/// @param[in]  si  耗材信息
///
/// @return rue表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建函数
///
bool ch::c1005::LogicControlProxy::ModifySuppliesInfo(const ::ch::tf::SuppliesInfo& si)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::c1005::LogicControlHandler logicControl;
    return logicControl.ModifySuppliesInfo(si);
}
