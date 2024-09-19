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
/// @file     ChLogicControlProxy.cpp
/// @brief    发送生化通用业务逻辑控制命令的代理
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
#include "ChLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/dcs/interface/ch/ChLogicControlHandler.h"
#include "src/thrift/ch/gen-cpp/ch_logic_control_constants.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <stdexcept>

///
/// @brief
///     端口号
///
unsigned short                      ch::LogicControlProxy::m_usPort;

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
std::shared_ptr<ch::tf::ChLogicControlClient> ch::LogicControlProxy::GetChLogicControlClient()
{
    std::shared_ptr<apache::thrift::transport::TTransport> spSocket(new apache::thrift::transport::TSocket("localhost", m_usPort));
    std::shared_ptr<apache::thrift::transport::TTransport> spTransport(new apache::thrift::transport::TFramedTransport(spSocket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> spProtocol(new apache::thrift::protocol::TCompactProtocol(spTransport));
    std::shared_ptr<apache::thrift::protocol::TMultiplexedProtocol> spMprotocol(new apache::thrift::protocol::TMultiplexedProtocol(spProtocol, ch::tf::g_ch_logic_control_constants.SERVICE_NAME));
    std::shared_ptr<ch::tf::ChLogicControlClient> spClient(new ch::tf::ChLogicControlClient(spMprotocol));

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
///     添加生化通用项目参数
///
/// @param[out]  _return    执行结果
/// @param[in]  gai         生化通用项目参数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ch::LogicControlProxy::AddGeneralAssayInfo(::tf::ResultLong& _return, const ::ch::tf::GeneralAssayInfo& gai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddGeneralAssayInfo(_return, gai);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     删除生化通用项目参数
///
/// @param[in]  gaiq       查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ch::LogicControlProxy::DeleteGeneralAssayInfo(const ::ch::tf::GeneralAssayInfoQueryCond& gaiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    return logicControl.DeleteGeneralAssayInfo(gaiq);
}

///
/// @brief
///     修改生化通用项目参数
///
/// @param[in]  aci     新的生化通用项目参数
///
/// @return true表示成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年4月7日，新建函数
///
bool ch::LogicControlProxy::ModifyAssayConfigInfo(const ch::tf::GeneralAssayInfo& aci)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    return logicControl.ModifyGeneralAssayInfo(aci);
}

///
/// @brief    
///     查询项目信息
///
/// @param[in]  qryCond  查询条件
/// @param[out] qryResp  查询结果
///
/// @return true表示成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年4月7日，新建函数
///
bool ch::LogicControlProxy::QueryAssayConfigInfo(const ::ch::tf::GeneralAssayInfoQueryCond& qryCond, ::ch::tf::GeneralAssayInfoQueryResp& qryResp)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QueryGeneralAssayInfo(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief 添加耗材属性信息
///
/// @param[in]  _return  执行结果
/// @param[in]  sa		 耗材属性信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月4日，新建函数
///
void ch::LogicControlProxy::AddSupplyAttribute(::tf::ResultLong& _return, const ::ch::tf::SupplyAttribute& sa)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddSupplyAttribute(_return, sa);
}

///
/// @brief 删除条件指定的耗材属性
///
/// @param[in]  saq		指定条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月4日，新建函数
///
bool ch::LogicControlProxy::DeleteSupplyAttribute(const ::ch::tf::SupplyAttributeQueryCond& saq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.DeleteSupplyAttribute(saq);
}

///
/// @brief 修改指定耗材属性信息
///
/// @param[in]  sa		耗材属性
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月4日，新建函数
///
bool ch::LogicControlProxy::ModifySupplyAttribute(const ::ch::tf::SupplyAttribute& sa)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.ModifySupplyAttribute(sa);
}

///
/// @brief 查询条件指定的耗材属性
///
/// @param[in]  _return  执行结果
/// @param[in]  saq		 耗材查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月4日，新建函数
///
bool ch::LogicControlProxy::QuerySupplyAttribute(::ch::tf::SupplyAttributeQueryResp& _return, const ::ch::tf::SupplyAttributeQueryCond& saq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QuerySupplyAttribute(_return, saq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     添加生化特殊项目参数
///
/// @param[out]  _return 执行结果
/// @param[in]  sai      生化特殊项目参数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
void ch::LogicControlProxy::AddSpecialAssayInfo(::tf::ResultLong& _return, const ::ch::tf::SpecialAssayInfo& sai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddSpecialAssayInfo(_return, sai);
}

///
/// @brief
///     删除生化特殊项目参数
///
/// @param[in]  saiq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ch::LogicControlProxy::DeleteSpecialAssayInfo(const ::ch::tf::SpecialAssayInfoQueryCond& saiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.DeleteSpecialAssayInfo(saiq);
}

///
/// @brief
///     修改生化特殊项目参数
///
/// @param[in]  sai   新的生化特殊项目参数
///
/// @return 结果
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
::tf::ThriftResult::type ch::LogicControlProxy::ModifySpecialAssayInfo(const ::ch::tf::SpecialAssayInfo& sai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.ModifySpecialAssayInfo(sai);
}

bool ch::LogicControlProxy::AddAssayInfo(const ::tf::GeneralAssayInfo& gai, const ::ch::tf::GeneralAssayInfo& chGai, const ::ch::tf::SpecialAssayInfo& sai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.AddAssayInfo(gai, chGai, sai);
}

::tf::ThriftResult::type ch::LogicControlProxy::ModifyAssayInfo(const ::tf::GeneralAssayInfo& gai, const ::ch::tf::GeneralAssayInfo& chGai, const ::ch::tf::SpecialAssayInfo& sai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.ModifyAssayInfo(gai, chGai, sai);
}

///
/// @brief
///     查询满足条件的生化特殊项目参数
///
/// @param[out] _return 执行结果
/// @param[in]  saiq    生化特殊项目参数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ch::LogicControlProxy::QuerySpecialAssayInfo(::ch::tf::SpecialAssayInfoQueryResp& _return, const ::ch::tf::SpecialAssayInfoQueryCond& saiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QuerySpecialAssayInfo(_return, saiq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     生化项目信息编码
///
/// @param[in]  strAssayCode  编码后的字符串
/// @param[in]  caiList		  需要编码的项目信息
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月18日，新建函数
///
bool ch::LogicControlProxy::EncodeChAssayInfo(std::string& strAssayCode, const ::ch::tf::ChAssayInfoList& caiList, bool isEncryption)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	try
	{
		logicControl.EncodeChAssayInfo(strAssayCode, caiList, isEncryption);
	}
	catch (::tf::ThriftException& ex)
	{
		ULOG(LOG_ERROR, "exception: %s", ex.exceptInfo);
		return false;
	}
	catch (...)
	{
		ULOG(LOG_ERROR, "exception: unknown thrift exception");
		return false;
	}
	
	return true;
}

///
/// @brief
///     生化项目信息解码
///
/// @param[in]  strAssayCode  需要解码的字符串
/// @param[in]  caiList		  解码后的项目信息
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月18日，新建函数
///
bool ch::LogicControlProxy::DecodeChAssayInfo(const std::string& strAssayCode, ::ch::tf::ChAssayInfoList& caiList)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;

	try
	{
		logicControl.DecodeChAssayInfo(caiList, strAssayCode);
	}
	catch (::tf::ThriftException& ex)
	{
		ULOG(LOG_ERROR, "exception: %s", ex.exceptInfo);
		return false;
	}
	catch (...)
	{
		ULOG(LOG_ERROR, "exception: unknown thrift exception");
		return false;
	}

	return true;
}

///
/// @brief
///     查询试剂总览
///
/// @param[out]  _return  执行结果
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
void ch::LogicControlProxy::QueryReagentOverviews(std::vector< ::ch::tf::ReagentOverview> & _return)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryReagentOverviews(_return);
}

///
/// @brief
///     查询指定设备的仓内仓外耗材总览
///
/// @param[out]  _return  执行结果
/// @param[in]  deviceSN  查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
void ch::LogicControlProxy::QuerySuppliesOverviews(std::vector< ::ch::tf::SuppliesOverview> & _return, const std::string& deviceSN)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QuerySuppliesOverviews(_return, deviceSN);
}

///
/// @brief 添加质控申请信息
///
/// @param[in]  _return  执行结果
/// @param[in]  qcApp    质控申请信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月30日，新建函数
///
void ch::LogicControlProxy::AddQcApply(::tf::ResultLong& _return, const ::ch::tf::QcApply& qcApp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddQcApply(_return, qcApp);
}

///
/// @brief 删除满足条件的质控申请信息
///
/// @param[in]  qcapcd  质控申请信息查询条件
///
/// @return true表示删除成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月30日，新建函数
///
bool ch::LogicControlProxy::DeleteQcApply(const ::ch::tf::QcApplyQueryCond& qcapcd)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.DeleteQcApply(qcapcd);
}

///
/// @brief 修改质控申请信息
///
/// @param[in]  qcApp  质控申请信息
///
/// @return true修改成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月30日，新建函数
///
bool ch::LogicControlProxy::ModifyQcApply(const ::ch::tf::QcApply& qcApp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.ModifyQcApply(qcApp);
}

///
/// @brief 查询满足条件的质控申请信息
///
/// @param[in]  _return  质控申请信息
/// @param[in]  qcapcd   查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月30日，新建函数
///
void ch::LogicControlProxy::QueryQcApply(::ch::tf::QcApplyQueryResp& _return, const ::ch::tf::QcApplyQueryCond& qcapcd)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QueryQcApply(_return, qcapcd);
}

///
/// @brief
///     查询质控计算结果
///
/// @param[in]  _return   返回值
/// @param[in]  qccalccd  查询条件
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月14日，新建函数
///
void ch::LogicControlProxy::QueryQcCalculateResult(::ch::tf::QcCalculateResultQueryResp& _return, const ::ch::tf::QcCalculateResultQueryCond& qccalccd)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryQcCalculateResult(_return, qccalccd);
}

///
/// @brief
///     查询质控结果
///
/// @param[in]  _return  返回值
/// @param[in]  qcrltcd  查询条件
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月15日，新建函数
///
void ch::LogicControlProxy::QueryQcResult(::ch::tf::QcResultQueryResp& _return, const ::ch::tf::QcResultQueryCond& qcrltcd)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryQcResult(_return, qcrltcd);
}

///
/// @brief
///     修改质控结果信息
///
/// @param[in]  qcRlt  质控结果信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月16日，新建函数
///
bool ch::LogicControlProxy::ModifyQcResult(const ::ch::tf::QcResult& qcRlt)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    return logicControl.ModifyQcResult(qcRlt);
}

///
/// @brief
///     查询联合质控结果
///
/// @param[in]  _return  返回值
/// @param[in]  qcrltcd  查询条件
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月16日，新建函数
///
void ch::LogicControlProxy::QueryUnionQcResult(::ch::tf::UnionQcResultQueryResp& _return, const ::ch::tf::UnionQcResultQueryCond& qcrltcd)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryUnionQcResult(_return, qcrltcd);
}

///
/// @brief
///     修改联合质控结果信息
///
/// @param[in]  qcRlt  质控结果信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月16日，新建函数
///
bool ch::LogicControlProxy::ModifyUnionQcResult(const ::ch::tf::UnionQcResult& qcRlt)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    return logicControl.ModifyUnionQcResult(qcRlt);
}

///
/// @brief
///     添加试剂需求信息
///
/// @param[out]  _return  执行结果
/// @param[in]   rn       添加数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
void ch::LogicControlProxy::AddReagentNeed(::tf::ResultLong& _return, const ::ch::tf::ReagentNeed& rn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.AddReagentNeed(_return, rn);
}

///
/// @brief
///     删除满足条件的试剂需求信息
///
/// @param[in]  rnqc  需求信息的条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
bool ch::LogicControlProxy::DeleteReagentNeed(const ::ch::tf::ReagentNeedQueryCond& rnqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    return logicControl.DeleteReagentNeed(rnqc);
}

///
/// @brief
///     修改试剂需求信息
///
/// @param[in]  rn  需求信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
bool ch::LogicControlProxy::ModifyReagentNeed(const std::vector< ::ch::tf::ReagentNeed>& rn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    return logicControl.ModifyReagentNeed(rn);
}

///
/// @brief
///     查询满足条件的试剂需求信息
///
/// @param[out]  _return  执行结果
/// @param[in]   rnqc     需求信息的条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
void ch::LogicControlProxy::QueryReagentNeed(::ch::tf::ReagentNeedQueryResp& _return, const ::ch::tf::ReagentNeedQueryCond& rnqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryReagentNeed(_return, rnqc);
}

///
/// @brief
///     查询满足条件的校准品文档
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  cdqc  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
bool ch::LogicControlProxy::QueryCaliDoc(::ch::tf::CaliDocQueryResp& _return, const ::ch::tf::CaliDocQueryCond& cdqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryCaliDoc(_return, cdqc);
    if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     
///
/// @param[in]  cali  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
bool ch::LogicControlProxy::ModifyCaliDoc(const ::ch::tf::CaliDoc& cali)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	ch::LogicControlHandler logicControl;
    return logicControl.ModifyCaliDoc(cali);
}

///
/// @brief
///     添加校准品文档
///
/// @param[in]  _return  执行结果
/// @param[in]  cali  校准品文档
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
void ch::LogicControlProxy::AddCaliDoc(::tf::ResultLong& _return, const ::ch::tf::CaliDoc& cali)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.AddCaliDoc(_return, cali);
}

///
/// @brief
///     删除校准品文档
///
/// @param[in]  cdgq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 8090/YeHuaning，2022年8月26日，新建函数
///
bool ch::LogicControlProxy::DeleteCaliDoc(const::ch::tf::CaliDocQueryCond & cdgq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	ch::LogicControlHandler logicControl;
	return logicControl.DeleteCaliDoc(cdgq);
}

///
/// @brief
///     添加校准品组文档
///
/// @param[in]  _return  执行结果
/// @param[in]  cdg  校准品组文档
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
void ch::LogicControlProxy::AddCaliDocGroup(::tf::ResultLong& _return, const ::ch::tf::CaliDocGroup& cdg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.AddCaliDocGroup(_return, cdg);
}

///
/// @brief
///     修改校准品组文档
///
/// @param[in]  cdg  校准品组文档
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
bool ch::LogicControlProxy::ModifyCaliDocGroup(const ::ch::tf::CaliDocGroup& cdg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	ch::LogicControlHandler logicControl;
    return logicControl.ModifyCaliDocGroup(cdg);
}


///
/// @brief
///     删除校准品组文档
///
/// @param[in]  cdgq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
bool ch::LogicControlProxy::DeleteCaliDocGroup(const ::ch::tf::CaliDocGroupQueryCond& cdgq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    return logicControl.DeleteCaliDocGroup(cdgq);
}

///
/// @brief
///     查询满足条件的校准品组文档
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  cdgq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
bool ch::LogicControlProxy::QueryCaliDocGroup(::ch::tf::CaliDocGroupQueryResp& _return, const ::ch::tf::CaliDocGroupQueryCond& cdgq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryCaliDocGroup(_return, cdgq);
    if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     解析校准品二维码
///
/// @param[in]  _return   解析出的二维码校准信息
/// @param[in]  strQRcode  需要解析的字符串
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月16日，新建函数
///
bool ch::LogicControlProxy::ParseCaliQRcode(::ch::tf::CaliGroupQRcode& _return, const std::string& strQRcode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;

	try
	{
		logicControl.ParseCaliQRcode(_return, strQRcode);
	}
	catch (::tf::ThriftException& ex)
	{
		ULOG(LOG_ERROR, "thrift error: %s", ex.exceptInfo);
		return false;
	}
	catch (...)
	{
		ULOG(LOG_ERROR, "thrift error: catch unknown exception");
		return false;
	}

	return true;

}

///
/// @brief 下载校准品
///     
///
/// @param[out] _return	 下载的校准品文档组信息
/// @param[in]  qqcdgc   查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月17日，新建函数
///
bool ch::LogicControlProxy::DownloadCaliDocGroup(::adapter::chCloudDef::tf::CloudCaliDocGroupResp& _return, const  ::adapter::chCloudDef::tf::QueryCloudCaliDocGroupCond& qqcdgc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.DownloadCaliDocGroup(_return, qqcdgc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief 新增校准文档组
///     
///
/// @param[out] cdg		 校准品文档组信息
/// @param[in]  lstdoc   对应的文档列表
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月17日，新建函数
///
bool ch::LogicControlProxy::AddNewCaliDocGroup(const ::ch::tf::CaliDocGroup& cdg, const  std::vector< ::ch::tf::CaliDoc> & lstdoc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.AddNewCaliDocGroup(cdg, lstdoc);
}

void ch::LogicControlProxy::AddSpecialWashReagentProbe(::tf::ResultLong& _return, const ::ch::tf::SpecialWashReagentProbe& swrp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddSpecialWashReagentProbe(_return, swrp);
}

bool ch::LogicControlProxy::DeleteSpecialWashReagentProbe(const ::ch::tf::SpecialWashReagentProbeQueryCond& swrpq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	bool bRet = false;

	ch::LogicControlHandler logicControl;
	return logicControl.DeleteSpecialWashReagentProbe(swrpq);
}

bool ch::LogicControlProxy::ModifySpecialWashReagentProbe(const ::ch::tf::SpecialWashReagentProbe& swrp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	bool bRet = false;

	ch::LogicControlHandler logicControl;
	return logicControl.ModifySpecialWashReagentProbe(swrp);
}

void ch::LogicControlProxy::QuerySpecialWashReagentProbe(::ch::tf::SpecialWashReagentProbeQueryResp& _return
	, const ::ch::tf::SpecialWashReagentProbeQueryCond& swrpq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QuerySpecialWashReagentProbe(_return, swrpq);
}

int32_t ch::LogicControlProxy::QuerySpecialWashReagentProbeCount(const ::ch::tf::SpecialWashReagentProbeQueryCond& swrpq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.QuerySpecialWashReagentProbeCount(swrpq);
}

void ch::LogicControlProxy::AddSpecialWashSampleProbe(::tf::ResultLong& _return, const ::ch::tf::SpecialWashSampleProbe& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddSpecialWashSampleProbe(_return, swsp);
}

bool ch::LogicControlProxy::DeleteSpecialWashSampleProbe(const ::ch::tf::SpecialWashSampleProbeQueryCond& swspq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	bool bRet = false;

	ch::LogicControlHandler logicControl;
	return logicControl.DeleteSpecialWashSampleProbe(swspq);
}

bool ch::LogicControlProxy::ModifySpecialWashSampleProbe(const ::ch::tf::SpecialWashSampleProbe& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	bool bRet = false;

	ch::LogicControlHandler logicControl;
	return logicControl.ModifySpecialWashSampleProbe(swsp);
}

void ch::LogicControlProxy::QuerySpecialWashSampleProbe(::ch::tf::SpecialWashSampleProbeQueryResp& _return, const ::ch::tf::SpecialWashSampleProbeQueryCond& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QuerySpecialWashSampleProbe(_return, swsp);
}

int32_t ch::LogicControlProxy::QuerySpecialWashSampleProbeCount(const ::ch::tf::SpecialWashSampleProbeQueryCond& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.QuerySpecialWashSampleProbeCount(swsp);
}

void ch::LogicControlProxy::AddSpecialWashCup(::tf::ResultLong& _return, const ::ch::tf::SpecialWashCup& swc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddSpecialWashCup(_return, swc);
}

bool ch::LogicControlProxy::DeleteSpecialWashCup(const ::ch::tf::SpecialWashCupQueryCond& swcq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	bool bRet = false;

	ch::LogicControlHandler logicControl;
	return logicControl.DeleteSpecialWashCup(swcq);
}

bool ch::LogicControlProxy::ModifySpecialWashCup(const ::ch::tf::SpecialWashCup& swc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	bool bRet = false;

	ch::LogicControlHandler logicControl;
	return logicControl.ModifySpecialWashCup(swc);
}

void ch::LogicControlProxy::QuerySpecialWashCup(::ch::tf::SpecialWashCupQueryResp& _return, const ::ch::tf::SpecialWashCupQueryCond& swcq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QuerySpecialWashCup(_return, swcq);
}

int32_t ch::LogicControlProxy::QuerySpecialWashCupCount(const ::ch::tf::SpecialWashCupQueryCond& swcq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.QuerySpecialWashCupCount(swcq);
}

///
/// @brief
///     添加维护项结果信息
///
/// @param[in]  _return  执行结果
/// @param[in]  miri	 维护项结果信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void ch::LogicControlProxy::AddMaintainItemResultInfo(::tf::ResultLong& _return, const  ::ch::tf::MaintainItemResultInfo& miri)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.AddMaintainItemResultInfo(_return, miri);
}

///
/// @brief
///     查找维护项结果信息
///
/// @param[in]  _return  执行结果
/// @param[in]  miriqc	 查询条件
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void ch::LogicControlProxy::QueryMaintainItemResultInfo(::ch::tf::MaintainItemResultInfoQueryResp& _return, const  ::ch::tf::MaintainItemResultInfoQueryCond& miriqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	logicControl.QueryMaintainItemResultInfo(_return, miriqc);
}

void ch::LogicControlProxy::QueryReactionCupHistoryInfo(std::vector< ::ch::tf::ReactionCupHistoryInfo> & _return, const std::string& sn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
    logicControl.QueryReactionCupHistoryInfo(_return, sn);
}

bool ch::LogicControlProxy::ResetReactionCupHistoryInfo(const std::string& sn, const std::vector<int32_t> & cupNums)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ch::LogicControlHandler logicControl;
	return logicControl.ResetReactionCupHistoryInfo(sn, cupNums);
}
