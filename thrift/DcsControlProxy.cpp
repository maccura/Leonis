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
/// @file     DcsControlProxy.cpp
/// @brief    发送DCS控制命令的代理
///
/// @author   3558/ZhouGuangMing
/// @date     2021年3月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "DcsControlProxy.h"
#include <stdexcept>
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>

#include "shared/uidef.h"
#include "src/thrift/gen-cpp/dcs_control_constants.h"
#include "src/public/DictionaryKeyName.h"
#include "src/dcs/interface/DcsControlHandler.h"
#include "src/common/StringUtil.h"
#include "src/common/Mlog/mlog.h"
#include <boost/algorithm/string.hpp>

///
/// @brief 初始化单例对象
///
std::shared_ptr<DcsControlProxy>    DcsControlProxy::s_dcp(new DcsControlProxy);

///
/// @brief
///     获得单例对象
///
/// @return 单例对象
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
std::shared_ptr<DcsControlProxy> DcsControlProxy::GetInstance()
{
    return s_dcp;
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
DcsControlProxy::~DcsControlProxy()
{
    UnInit();
}

///
/// @brief
///     初始化
///
/// @param[in]  strIP   服务器IP
/// @param[in]  usPort  服务器端口号
///
/// @return true表示成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
bool DcsControlProxy::Init(const std::string& strIP, unsigned short usPort)
{
    ULOG(LOG_INFO, "%s(%s, %d)", __FUNCTION__, strIP.c_str(), usPort);

	if (nullptr == m_spDcs)
	{
		m_spDcs.reset(new DcsControlHandler);
	}

    return true;
}

///
/// @brief
///     反初始化
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
void DcsControlProxy::UnInit()
{
    // 关闭连接
    if (m_spClient != nullptr)
    {
        m_spClient.reset();
    }
}

///
/// @brief
///     记录开机事件
///
/// @param[in]   
///
/// @return true表示成功
///
/// @par History:
/// @li 7685/likai，2023年11月23日，新建函数
bool DcsControlProxy::RecordStartEvent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->RecordStartEvent();
}

///
/// @brief
///     记录关机事件
///
/// @param[in]   
///
/// @return true表示成功
///
/// @par History:
/// @li 7685/likai，2023年11月23日，新建函数
bool DcsControlProxy::RecordExitEvent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->RecordExitEvent();
}

///
/// @brief 用户登录
///     
/// @param[in]  userInfo  用户信息
///
/// @return true表示成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
bool DcsControlProxy::Login(const tf::UserInfo& userInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	
	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->Login(userInfo);
}

///
/// @brief
///     自动登录
///
/// @return 自动登录成功的用户，空表示未能自动登录
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月8日，新建函数
///
std::string DcsControlProxy::AutoLogin()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 构造返回值
    std::string strAutoLoginUser("");

    // 返回自动登录的用户，空表示未能自动登录
	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return "";
	}

	m_spDcs->AutoLogin(strAutoLoginUser);
    return strAutoLoginUser;
}

///
/// @brief
///     用户身份验证
///
/// @param[in]  userName        用户名
/// @param[in]  password        密码
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月9日，新建函数
///
bool DcsControlProxy::UserIdentityVerify(const std::string& userName, const std::string& password)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->UserIdentityVerify(userName, password);
}

bool DcsControlProxy::StartTest(const ::tf::StartTestParams& params)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->StartTest(params);
}

///
/// @brief
///     停止测试
///
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月13日，新建函数
///
bool DcsControlProxy::StopTest()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->StopTest();
}

///
/// @brief 暂停测试
/// 
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月13日，新建函数
///
bool DcsControlProxy::PauseTest()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->PauseTest();
}

///
///  @brief 执行维护
///
///  @param[in]   exeParams	    维护执行参数
///
///  @return true 表示发送维护任务成功
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月9日，新建函数
///  @li 8580/GongZhiQiang，2024年5月13日，修改参数为结构体，并增加维护项参数
///
bool DcsControlProxy::Maintain(const tf::MaintainExeParams& exeParams)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->Maintain(exeParams);
}

///
///  @brief 停止维护
///
///
///  @param[in]   lstDev  设备列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月19日，新建函数
///
bool DcsControlProxy::StopMaintain(const std::vector< ::tf::DevicekeyInfo> & lstDev)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->StopMaintain(lstDev);
}

///
/// @brief 查询测试项目表
///
/// @param[in]  qryCond  查询项目条件
/// @param[in]  qryResp  查询结果
///
/// @return true返回成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月27日，新建函数
///
bool DcsControlProxy::QueryTestItems(const ::tf::TestItemQueryCond& qryCond, ::tf::TestItemQueryResp& qryResp)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->QueryTestItems(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Failed to execute QueryTestItems()");
		return false;
	}

    return true;
}

///
/// @brief新增样本信息
///     
///
/// @param[in]  sampInfo  样本信息
/// @param[in]  result  执行结果
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::AddSampleInfo(::tf::ResultListInt64& _return, const ::tf::SampleInfo& sampleInfo, const int32_t count)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddSampleInfo(_return, sampleInfo, count);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Failed to execute AddSampleInfo()");
		return false;
	}

    return true;
}

///
/// @brief 修改样本信息
///     
/// @param[in]  sampInfo  样本信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::ModifySampleInfo(const ::tf::SampleInfo& sampInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->ModifySampleInfo(sampInfo);
}

///
/// @brief 修改项目信息
///
/// @param[in]  vecTestItem  项目信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月16日，新建函数
///
bool DcsControlProxy::ModifyTestItemInfo(const ::tf::TestItem& ti)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyTestItemInfo(ti);
}

///
/// @brief
///     移除满足条件的样本信息
///
/// @param[in]  queryCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::RemoveSampleInfo(const ::tf::SampleInfoQueryCond& queryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->RemoveSampleInfo(queryCond);
}

///
/// @brief
///     批量移除样本信息
///
/// @param[in]  dbNos      移除样本信息的数据库主键
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::RemoveSampleInfos(const std::vector<int64_t>& dbNos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->RemoveSampleInfos(dbNos);
}

///
/// @brief
///     样本序号是否已经被使用
///
/// @param[in]  _return     返回结果
/// @param[in]  startSeqNo  起始序号
/// @param[in]  endSeqNo    终止序号
///
/// @return true表示成功
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::WhetherSeqNoUsed(::tf::ResultBool& _return, const int32_t startSeqNo, const int32_t endSeqNo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->WhetherSampleSeqnoUsed(_return, startSeqNo, endSeqNo);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief 查询样本信息
///     
/// @param[in]   qryCond  查询条件
/// @param[out]  qryResp  查询结果
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::QuerySampleInfo(const ::tf::SampleInfoQueryCond& qryCond, ::tf::SampleInfoQueryResp& qryResp)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->QuerySampleInfo(qryResp, qryCond);
    if (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
		ULOG(LOG_ERROR, "Failed to execute QuerySampleInfo()");
        return false;
    }

    return true;
}

///
/// @brief 传送样本到LIS
///
/// @param[in]  vecSampleKeys  样本数据库主键列表
/// @param[in]  transType      上传数据类型（参照 tf::LisTransferType ）
/// @param[in]  history        是否传输历史数据(false: 数据浏览样本, true: 历史数据样本，两个页面的数据来源不一样)
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月2日，新建函数
///
bool DcsControlProxy::PushSampleToLIS(const std::vector<int64_t> & vecSampleKeys, const int32_t transType, const bool history)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->PushSampleToLIS(vecSampleKeys, transType, history);
}

///
/// @brief 获取满足条件的最大样本号
///
/// @param[out] _return     满足条件的最大样本号
/// @param[out] stat        是否是急诊样本
/// @param[out] mode        模式（0：序号 1：条码 2：样本架）
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::GetMaxSampleSeqno(::tf::ResultStr& _return, bool stat, int mode)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->GetMaxSampleSeqno(_return, stat, mode);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     添加病人信息
///
/// @param[out] _return     执行结果
/// @param[in]  pi          病人信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::AddPatientInfo(::tf::ResultLong& _return, const  ::tf::PatientInfo& pi, const int64_t sampleId)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddPatientInfo(_return, pi, sampleId);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     删除满足条件的病人信息
///
/// @param[in]  piqc  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::DeletePatientInfo(const ::tf::PatientInfoQueryCond& piqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeletePatientInfo(piqc);
}

///
/// @brief
///     修改病人信息
///
/// @param[in]  pi  病人信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::ModifyPatientInfo(const ::tf::PatientInfo& pi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyPatientInfo(pi);
}

///
/// @brief
///     查询满足条件的病人信息
///
/// @param[out] _return 保存查询结果的容器
/// @param[in]  piqc    查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月9日，新建函数
///
bool DcsControlProxy::QueryPatientInfo(::tf::PatientInfoQueryResp& _return, const ::tf::PatientInfoQueryCond& piqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryPatientInfo(_return, piqc);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

bool DcsControlProxy::QueryAiRecognizeResult(::tf::AiRecognizeResultQueryResp& _return, const ::tf::AiRecognizeResultQueryCond& piqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryAiRecognizeResult(_return, piqc);
	if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

bool DcsControlProxy::ModifyDeviceInfo(const ::tf::DeviceInfo& di)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyDeviceInfo(di);
}

///
/// @brief
///     查询满足条件的设备信息
///
/// @param[out]  _return  保存查询结果的容器
/// @param[in]  diqc    查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年9月7日，新建函数
///
bool DcsControlProxy::QueryDeviceInfo(::tf::DeviceInfoQueryResp& _return, const ::tf::DeviceInfoQueryCond& diqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryDeviceInfo(_return, diqc);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

bool DcsControlProxy::QueryDeviceVersion(std::map<std::string, std::map<std::string, std::string> > & _return)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryDeviceVersion(_return);
    return true;
}

void DcsControlProxy::GetTemperautureConfigInfo(
    std::map<::tf::DeviceType::type, std::vector< ::tf::TemperautureInfo>>& _return, 
    const std::vector< ::tf::DeviceType::type>& types)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (nullptr == m_spDcs)
    {
        ULOG(LOG_ERROR, "m_spDcs is nullptr");
        return;
    }

    m_spDcs->GetTemperautureConfigInfo(_return, types);
}

void DcsControlProxy::GetLiquidPathConfigInfo(
    std::map<::tf::DeviceType::type, std::vector<std::string>>& _return, 
    const std::vector< ::tf::DeviceType::type>& types)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (nullptr == m_spDcs)
    {
        ULOG(LOG_ERROR, "m_spDcs is nullptr");
        return;
    }

    m_spDcs->GetLiquidPathConfigInfo(_return, types);
}

void DcsControlProxy::GetUnitConfigInfo(
    std::map<::tf::DeviceType::type, std::vector<std::string>>& _return, 
    const std::vector< ::tf::DeviceType::type>& types)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (nullptr == m_spDcs)
    {
        ULOG(LOG_ERROR, "m_spDcs is nullptr");
        return;
    }

    m_spDcs->GetUnitConfigInfo(_return, types);
}

void DcsControlProxy::GetCountConfigInfo(
    std::map<::tf::DeviceType::type, std::vector< ::tf::CountInfo>>& _return, 
    const std::vector< ::tf::DeviceType::type>& types)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (nullptr == m_spDcs)
    {
        ULOG(LOG_ERROR, "m_spDcs is nullptr");
        return;
    }

    m_spDcs->GetCountConfigInfo(_return, types);
}

bool DcsControlProxy::SetLowerComponentCount(const bool isReset, const ::tf::CountInfo& ci, const std::string& devSn, const int32_t moduleIndex /*= 1*/)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->SetLowerComponentCount(isReset, ci, devSn, moduleIndex);
}

///
/// @brief 设备管理-启用
///
/// @param[in]  lstDeviceSN  设备编号列表
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月23日，新建函数
///
bool DcsControlProxy::DevicePowerOn(const std::vector<std::string> & lstDeviceSN)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DevicePowerOn(lstDeviceSN);
}

bool DcsControlProxy::DevicePowerOff(const std::vector<std::string>& lstDevSN)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->DevicePowerOff(lstDevSN);
}

///
/// @brief 更新上电计划表
///
/// @param[in]  mapDevice2Pos   设备序列号----设备上电计划
///
/// @return true表示成功
///
/// @par History:
/// @li 8090/YeHuaNing，2023年3月24日，新建函数
///
bool DcsControlProxy::UpdetePowerOnSchedule(const std::map<std::string, ::tf::PowerOnSchedule>& mapDevice2Pos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->UpdetePowerOnSchedule(mapDevice2Pos);
}

bool DcsControlProxy::DeviceSleep()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeviceSleep();
}

bool DcsControlProxy::DeviceAwake()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeviceAwake();
}

///
/// @brief
///     查询指定项目选择页面的元素
///
/// @param[out] _return 保存查询结果的容器
/// @param[in]  ac      项目分类
/// @param[in]  pageIdx 页面索引，从0开始计数
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年4月13日，新建函数
///
bool DcsControlProxy::QueryGeneralAssayPage(::tf::GeneralAssayPageQueryResp& _return, ::tf::AssayClassify::type ac, int32_t pageIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->QueryGeneralAssayPage(_return, ac, pageIdx);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief 查询指定项目选择页面的组合项目元素
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  ac		 项目分类
/// @param[in]  pageIdx  页面索引，从0开始计数
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月27日，新建函数
///
bool DcsControlProxy::QueryProfileAssayPage(::tf::ProfileAssayPageQueryResp& _return, const ::tf::AssayClassify::type ac, const int32_t pageIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->QueryProfileAssayPage(_return, ac, pageIdx);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief 查询通用项目信息
///
/// @param[in]  _return 保存查询结果的容器
/// @param[in]  gaiq    查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月7日，新建函数
///
bool DcsControlProxy::QueryGeneralAssayInfo(::tf::GeneralAssayInfoQueryResp& _return, const ::tf::GeneralAssayInfoQueryCond& gaiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->QueryGeneralAssayInfo(_return, gaiq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Failed to execute QueryTestItems()");
		return false;
	}

    return true;
}

///
/// @brief 查询组合项目信息
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  piqc     查询条件
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月27日，新建函数
///
bool DcsControlProxy::QueryProfileInfo(::tf::ProfileInfoQueryResp& _return, const ::tf::ProfileInfoQueryCond& piqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryProfileInfo(_return, piqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Failed to execute QueryProfileInfo()");
		return false;
	}

    return true;
}

///
/// @brief  修改组合项目信息
///
/// @param[in]  pi      新的组合项目信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月28日，新建函数
///
bool DcsControlProxy::ModifyProfileInfo(const ::tf::ProfileInfo& pi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyProfileInfo(pi);
}

bool DcsControlProxy::DeleteProfileInfo(const ::tf::ProfileInfoQueryCond& pi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->DeleteProfileInfo(pi);
}

bool DcsControlProxy::AddProfileInfo(tf::ResultLong& _return, const ::tf::ProfileInfo& pi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddProfileInfo(_return, pi);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     修改通用项目信息
///
/// @param[in]  gai     通用项目信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月7日，新建函数
///
bool DcsControlProxy::ModifyGeneralAssayInfo(const ::tf::GeneralAssayInfo& gai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyGeneralAssayInfo(gai);
}

///
/// @brief
///     删除满足条件的通用项目信息
///
/// @param[in]  gaiq    查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月7日，新建函数
///
bool DcsControlProxy::DeleteGeneralAssayInfo(const ::tf::GeneralAssayInfoQueryCond& gaiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteGeneralAssayInfo(gaiq);
}

///
/// @brief
///     添加通用项目信息
///
/// @param[in]  _return 执行结果
/// @param[in]  gai     通用项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月7日，新建函数
///
void DcsControlProxy::AddGeneralAssayInfo(::tf::ResultLong& _return, const ::tf::GeneralAssayInfo& gai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return;
	}

	m_spDcs->AddGeneralAssayInfo(_return, gai);
}

///
/// @brief
///     添加开放项目记录
///
/// @param[in]  _return 执行结果
/// @param[in]  oar     开放项目记录
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
///
void DcsControlProxy::AddOpenAssayRecord(::tf::ResultLong& _return, const ::tf::OpenAssayRecord& oar)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return;
	}

	m_spDcs->AddOpenAssayRecord(_return, oar);
}

///
/// @brief
///     删除开放项目记录
///
/// @param[in]  oarqc    查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
///
bool DcsControlProxy::DeleteOpenAssayRecord(const ::tf::OpenAssayRecordQueryCond& oarqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteOpenAssayRecord(oarqc);
}

///
/// @brief
///     修改开放项目记录
///
/// @param[in]  oar     开放项目记录
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
///
bool DcsControlProxy::ModifyOpenAssayRecord(const ::tf::OpenAssayRecord& oar)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyOpenAssayRecord(oar);
}

///
/// @brief	查找开放项目信息
///     
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  oarqc  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
///
bool DcsControlProxy::QueryOpenAssayRecord(::tf::OpenAssayRecordQueryResp&  _return, ::tf::OpenAssayRecordQueryCond& oarqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryOpenAssayRecord(_return, oarqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}
	
	return true;
}

bool DcsControlProxy::DeleteDictionaryInfo(const::tf::DictionaryInfoQueryCond & dictionaryInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteDictionaryInfo(dictionaryInfo);
}

///
/// @brief
///     修改字典信息
///
/// @param[in]  dictionaryInfo  更新的字典信息
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年9月3日，新建函数
///
bool DcsControlProxy::ModifyDictionaryInfo(const ::tf::DictionaryInfo& dictionaryInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyDictionaryInfo(dictionaryInfo);
}

///
/// @brief
///     添加字典信息
///
/// @param[in]  dictionaryInfo  添加的字典信息
/// @param[out] lRet            成功返回对应的数据库主键
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年9月3日，新建函数
///
bool DcsControlProxy::AddDictionaryInfo(const ::tf::DictionaryInfo& dictionaryInfo, ::tf::ResultLong& lRet)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddDictionaryInfo(lRet, dictionaryInfo);
	if (lRet.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  gaiq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年9月3日，新建函数
///
bool DcsControlProxy::QueryDictionaryInfo(::tf::DictionaryInfoQueryResp& _return, ::tf::DictionaryInfoQueryCond& gaiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->QueryDictionaryInfo(_return, gaiq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
///  @brief
///
///
///  @param[in]   _return   执行结果
///  @param[in]   mi		维护项目信息
///
///  @return	true 表示成功
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::AddMaintainItem(::tf::ResultLong & _return, const::tf::MaintainItem & mi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->AddMaintainItem(_return, mi);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     删除满足条件的维护项目
///
/// @param[in]  miqc    查询条件
///
/// @return true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::DeleteMaintainItem(const::tf::MaintainItemQueryCond & miqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->DeleteMaintainItem(miqc);
}

///
/// @brief
///     修改维护项目
///
/// @param[in]  mi          维护项目信息
///
/// @return true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::ModifyMaintainItem(const::tf::MaintainItem & mi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyMaintainItem(mi);
}

///
///  @brief
///     查询满足条件的维护项目信息
///
///  @param[in]   _return     保存查询结果的容器  
///  @param[in]   miqc        查询条件  
///
///  @return	true 表示成功
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::QueryMaintainItem(::tf::MaintainItemQueryResp & _return, const::tf::MaintainItemQueryCond & miqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryMaintainItem(_return, miqc);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     添加维护组
///
/// @param[out] _return     执行结果
/// @param[in]  mg          维护组信息
///
///  @return	true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::AddMaintainGroup(::tf::ResultLong & _return, const::tf::MaintainGroup & mg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->AddMaintainGroup(_return, mg);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     删除满足条件的维护组
///
/// @param[in]  mgqc    查询条件
///
/// @return true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::DeleteMaintainGroup(const::tf::MaintainGroupQueryCond & mgqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteMaintainGroup(mgqc);
}

///
///  @brief
///		修改维护组
///
///  @param[in]   lstMg  维护组信息列表
///
///  @return	true 表示成功
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年6月16日，新建函数
///
bool DcsControlProxy::ModifyMaintainGroups(const std::vector<::tf::MaintainGroup>& lstMg)
{
	ULOG(LOG_INFO, "%s() param size:%d.", __FUNCTION__, lstMg.size());

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyMaintainGroups(lstMg);
}

///
/// @brief
///     查询满足条件的维护组信息
///
/// @param[out] _return     保存查询结果的容器
/// @param[in]  mgqc        查询条件
///
/// @return true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::QueryMaintainGroup(::tf::MaintainGroupQueryResp & _return, const::tf::MaintainGroupQueryCond & mgqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryMaintainGroup(_return, mgqc);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     添加维护日志
///
/// @param[out] _return     执行结果
/// @param[in]  ml          维护日志信息
///
/// @return true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::AddMaintainLog(::tf::ResultLong & _return, const::tf::MaintainLog & ml)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddMaintainLog(_return, ml);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     删除满足条件的维护日志
///
/// @param[in]  mlqc     查询条件
///
/// @return true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::DeleteMaintainLog(const::tf::MaintainLogQueryCond & mlqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->DeleteMaintainLog(mlqc);
}

///
/// @brief
///     查询满足条件的维护日志
///
/// @param[out] _return     保存查询结果的容器
/// @param[in]  mlqc        查询条件
///
/// @return true 表示成功
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
bool DcsControlProxy::QueryMaintainLog(::tf::MaintainLogQueryResp & _return, const::tf::MaintainLogQueryCond & mlqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryMaintainLog(_return, mlqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}
    
    return true;
}

///
/// @brief
///     记录维护失败原因字符串资源（用于维护失败报警）
///
/// @param[in]  mapStrRc  字符串资源映射
///
/// @par History:
/// @li 4170/TangChuXian，2024年9月6日，新建函数
///
bool DcsControlProxy::RecordMaintainFailReasonStringRc(const std::map<int, string>& mapStrRc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (nullptr == m_spDcs)
    {
        ULOG(LOG_ERROR, "m_spDcs is nullptr");
        return false;
    }

    return m_spDcs->RecordMaintainFailReasonStringRc(mapStrRc);
}

///
/// @brief
///     记录维护项名称字符串资源（用于维护失败报警）
///
/// @param[in]  mapStrRc  字符串资源映射
///
/// @par History:
/// @li 4170/TangChuXian，2024年9月9日，新建函数
///
bool DcsControlProxy::RecordMaintainItemNameStringRc(const std::map<int, std::string>& mapStrRc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (nullptr == m_spDcs)
    {
        ULOG(LOG_ERROR, "m_spDcs is nullptr");
        return false;
    }

    return m_spDcs->RecordMaintainItemNameStringRc(mapStrRc);
}

///
///  @brief
///
///
///  @param[in]   _return  执行结果
///  @param[in]   lmi      最近一次维护情况
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月24日，新建函数
///
bool DcsControlProxy::AddLatestMaintainInfo(::tf::ResultLong & _return, const::tf::LatestMaintainInfo & lmi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    m_spDcs->AddLatestMaintainInfo(_return, lmi);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
///  @brief 
///			查询满足条件的最近一次维护情况
///
///  @param[in]   _return  保存查询结果的容器
///  @param[in]   lmiqc    查询条件
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月23日，新建函数
///
bool DcsControlProxy::QueryLatestMaintainInfo(::tf::LatestMaintainInfoQueryResp & _return, const::tf::LatestMaintainInfoQueryCond & lmiqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryLatestMaintainInfo(_return, lmiqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     修改维护详情
///
/// @param[in]  lmi  维护详情
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月24日，新建函数
///
bool DcsControlProxy::ModifyLatestMaintainInfo(const ::tf::LatestMaintainInfo& lmi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->ModifyLatestMaintainInfo(lmi);
}

///
///  @brief 添加操作日志
///
///
///  @param[in]   _return   执行结果
///  @param[in]   ol		操作日志
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
///
bool DcsControlProxy::AddOperationLog(::tf::ResultLong & _return, const::tf::OperationLog & ol)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddOperationLog(_return, ol);
	if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
///  @brief 删除满足条件的操作日志（仅用于测试）
///
///
///  @param[in]   olqc  操作日志查询条件
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
///
bool DcsControlProxy::DeleteOperationLog(const::tf::OperationLogQueryCond & olqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteOperationLog(olqc);
}

///
///  @brief 查询满足条件的操作日志
///
///
///  @param[in]   _return  返回查询结果
///  @param[in]   olqc     操作日志查询条件
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
///
bool DcsControlProxy::QueryOperationLog(::tf::OperationLogQueryResp & _return, const::tf::OperationLogQueryCond & olqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryOperationLog(_return, olqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
///  @brief 查询满足条件的操作日志数量
///
///
///  @param[in]   olqc  操作日志查询条件
///
///  @return	满足条件的操作日志数量
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月17日，新建函数
///
int64_t DcsControlProxy::QueryOperationLogCount(const::tf::OperationLogQueryCond & olqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return 0;
	}

	return m_spDcs->QueryOperationLogCount(olqc);
}

///
///  @brief 添加试剂/耗材更换日志
///
///
///  @param[in]   _return  执行结果
///  @param[in]   ccl      试剂/耗材更换日志
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
///
bool DcsControlProxy::AddConsumableChangeLog(::tf::ResultLong & _return, const::tf::ConsumableChangeLog & ccl)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddConsumableChangeLog(_return, ccl);
	if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
///  @brief 删除满足条件的试剂/耗材更换日志（仅用于测试）
///
///
///  @param[in]   cclqc  试剂/耗材更换日志查询条件
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
///
bool DcsControlProxy::DeleteConsumableChangeLog(const::tf::ConsumableChangeLogQueryCond & cclqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteConsumableChangeLog(cclqc);
}

///
///  @brief 查询满足条件的试剂/耗材更换日志
///
///
///  @param[in]   _return  返回查询结果
///  @param[in]   cclqc    试剂/耗材更换日志查询条件
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
///
bool DcsControlProxy::QueryConsumableChangeLog(::tf::ConsumableChangeLogQueryResp & _return, const::tf::ConsumableChangeLogQueryCond & cclqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryConsumableChangeLog(_return, cclqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief 查询试剂/耗材更换日志名称
///
/// @param[in]  _return  
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年2月1日，新建函数
///
void DcsControlProxy::QueryConsumableChangeLogNames(std::vector<std::string> & _return)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return;
	}

	m_spDcs->QueryConsumableChangeLogNames(_return);
}

///
///  @brief 查询满足条件的试剂/耗材更换日志的数量
///
///
///  @param[in]   cclqc  试剂/耗材更换日志查询条件
///
///  @return	满足条件的试剂/耗材更换日志的数量
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月17日，新建函数
///
int64_t DcsControlProxy::QueryConsumableChangeLogCount(const::tf::ConsumableChangeLogQueryCond & cclqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return 0;
	}

	return m_spDcs->QueryConsumableChangeLogCount(cclqc);
}

bool DcsControlProxy::AddDisableAlarmCodes(const std::vector< ::tf::DisableAlarmCode> & dacs)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->AddDisableAlarmCodes(dacs);
}

bool DcsControlProxy::DeleteDisableAlarmCodes(const std::vector< ::tf::DisableAlarmCode> & dacs)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->DeleteDisableAlarmCodes(dacs);
}

bool DcsControlProxy::DeviceAlarmReaded(const std::vector<std::string> & deviceSns, const bool fromLeonis)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeviceAlarmReaded(deviceSns, fromLeonis);
}

bool DcsControlProxy::QueryDisableAlarmCode(::tf::DisableAlarmCodeQueryResp& _return, const ::tf::DisableAlarmCode& dac)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryDisableAlarmCode(_return, dac);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

void DcsControlProxy::GetAllAlarmPrompt(std::vector< ::tf::AlarmPrompt> & _return)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return;
	}

	m_spDcs->GetAllAlarmPrompt(_return);
}

///
/// @brief
///     删除指定的告警记录
///
/// @param[in]  delAll  是否删除所有告警
/// @param[in]  guids   待删除告警的guid列表
///
/// @return true表示成功
///
/// @par History:
/// @li 7951/LuoXin，2022年7月26日，新建函数
///
bool DcsControlProxy::DeleteCurAlarm(bool delAll, const std::vector<std::string> & devSns)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->DeleteCurAlarm(delAll, devSns);
}

std::vector<::tf::AlarmDesc> DcsControlProxy::QueryCurAlarm()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return {};
	}

	std::vector<::tf::AlarmDesc> result;
	m_spDcs->QueryCurAlarm(result);
    return std::move(result);
}

///
/// @brief
///     查询指定的告警日志
///
/// @param[in]  _return  查询结果
/// @param[in]  aiq  查询条件
///
/// @par History:
/// @li 7951/LuoXin，2022年7月27日，新建函数
///
bool DcsControlProxy::QueryAlarmInfo(::tf::AlarmInfoQueryResp& _return, const ::tf::AlarmInfoQueryCond& aiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryAlarmInfo(_return, aiq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief	查询指定的告警日志总数
///     
/// @param[in]  aiq  查询条件
///
/// @return 指定的告警日志总数
///
/// @par History:
/// @li 7951/LuoXin，2022年8月25日，新建函数
///
int64_t DcsControlProxy::QueryAlarmInfoCount(const ::tf::AlarmInfoQueryCond& aiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return 0;
	}

	return m_spDcs->QueryAlarmInfoCount(aiq);
}

std::string DcsControlProxy::GetAlarmTitleByCode(const  ::tf::DeviceType::type type, const std::string& code)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return "";
	}

	std::string title;
	m_spDcs->GetAlarmTitleByCode(title, type, code);
	return std::move(title);
}

void DcsControlProxy::ClearAllAlarmShield()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return;
	}

	m_spDcs->ClearAllAlarmShield();
}

bool DcsControlProxy::QueryDataAlarm(std::vector<::tf::DataAlarmItem>& _return, const std::string& code)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryDataAlarm(_return, code);
	return true;
}

///
/// @brief 添加质控文档
///
/// @param[in]  _return  执行结果
/// @param[in]  qcDoc	 质控文档
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月17日，新建函数
///
bool DcsControlProxy::AddQcDoc(::tf::ResultLong& _return, const ::tf::QcDoc& qcDoc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddQcDoc(_return, qcDoc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

bool DcsControlProxy::AddUserInfo(tf::ResultLong& ret, const ::tf::UserInfo& userInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddUserInfo(ret, userInfo);
	return (ret.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS);
}

bool DcsControlProxy::DeleteUserInfo(const std::string& userName)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	::tf::UserInfoQueryCond cond;
	cond.__set_username(userName);
	return m_spDcs->DeleteUserInfo(cond);
}

bool DcsControlProxy::ModefyUserInfo(const ::tf::UserInfo& userInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyUserInfo(userInfo);
}

bool DcsControlProxy::QueryUserInfo(const std::string& userName, std::shared_ptr<tf::UserInfo>& userInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	tf::UserInfoQueryCond cond;
	cond.__set_username(userName);
	tf::UserInfoQueryResp resp;
	m_spDcs->QueryUserInfo(resp, cond);
	if (resp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || resp.lstUserInfos.empty())
	{
		return false;
	}

	userInfo = std::shared_ptr<tf::UserInfo>(new tf::UserInfo(resp.lstUserInfos[0]));
	return true;
}

bool DcsControlProxy::QueryAllUserInfo(tf::UserInfoQueryResp& _return, tf::UserInfoQueryCond& uiqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryUserInfo(_return, uiqc);
	return (_return.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS);
}

///
/// @brief
///     添加耗材需求信息
///
/// @param[in]  _return     执行结果
/// @param[in]  splReqInfo  耗材需求信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
bool DcsControlProxy::AddSplReqInfo(tf::ResultLong& _return, const ::tf::SplRequireInfo& splReqInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddSplRequireInfo(_return, splReqInfo);
	return (_return.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS);
}

///
/// @brief
///     删除耗材需求信息
///
/// @param[in]  sriqc  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
bool DcsControlProxy::DeleteSplReqInfo(const ::tf::SplRequireInfoQueryCond& sriqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteSplRequireInfo(sriqc);
}

///
/// @brief
///     修改耗材需求信息
///
/// @param[in]  splReqInfo  耗材需求信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
bool DcsControlProxy::ModifySplReqInfo(const ::tf::SplRequireInfo& splReqInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifySplRequireInfo(splReqInfo);
}

///
/// @brief
///     查询耗材需求信息
///
/// @param[in]  _return     查询结构
/// @param[in]  sriqc       查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月26日，新建函数
///
bool DcsControlProxy::QuerySplReqInfo(::tf::SplRequireInfoQueryResp& _return, const ::tf::SplRequireInfoQueryCond& sriqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QuerySplRequireInfo(_return, sriqc);
	return (_return.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS);
}

///
/// @brief 修改质控文档
///
/// @param[in]  qcDoc  质控文档
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月17日，新建函数
///
bool DcsControlProxy::ModifyQcDoc(tf::ResultLong& ret, const ::tf::QcDoc& qcDoc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->ModifyQcDoc(ret, qcDoc);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief 查询满足条件的质控文档
///
/// @param[in]  _return  质控文档列表
/// @param[in]  cdqc	 查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月17日，新建函数
///
bool DcsControlProxy::QueryQcDoc(::tf::QcDocQueryResp& _return, const ::tf::QcDocQueryCond& cdqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryQcDoc(_return, cdqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @bref
///		查询获取所有质控品
///
/// @param[in] _return 反馈的质控品列表
///
/// @par History:
/// @li 8276/huchunli, 2022年11月11日，新建函数
///
bool DcsControlProxy::QueryQcDocAll(::tf::QcDocQueryResp& _return)
{
    ::tf::QcDocQueryCond tempCond;
    return QueryQcDoc(_return, tempCond);
}

///
/// @brief 查询包含指定项目的质控文档
///
/// @param[in]  _return  质控文档列表
/// @param[in]  assayCode 项目编号
///
/// @return true表示成功
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
bool DcsControlProxy::QueryQcDocByAssayCode(::tf::QcDocQueryResp & _return, int32_t assayCode)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryQcDocByAssayCode(_return, assayCode);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief 删除满足条件的质控文档
///
/// @param[in]  cdgq  删除条件
/// @param[out]  failedDelete  删除失败的质控品ID
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月17日，新建函数
///
bool DcsControlProxy::DeleteQcDoc(const ::tf::QcDocQueryCond& cdgq, std::vector<int64_t>& failedDelete)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	tf::ResultListInt64 resp;
	m_spDcs->DeleteQcDoc(resp, cdgq);
	if (resp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief  解析质控品条码
///
/// @param[in]  _return  质控品信息
/// @param[in]  strBarcode  条码字符串
///
/// @return 是否成功解析
///
/// @par History:
/// @li 1226/zhangjing，2022年8月2日，新建函数
///
bool DcsControlProxy::ParseQCBarcodeInfo(::tf::QcBarCodeInfo& _return, const std::string& strBarcode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->ParseQCBarcodeInfo(_return, strBarcode);
	// 编号和批号未设置解析失败
	if (!_return.__isset.docInfo || !_return.docInfo.__isset.sn || !_return.docInfo.__isset.lot)
	{
		return false;
	}

	return true;
}


///
/// @brief 添加仪器系数
///
/// @param[in]  _return  执行结果
/// @param[in]  mf  项目
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
bool DcsControlProxy::AddMachineFactor(::tf::ResultLong & _return, const::tf::MachineFactor & mf)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddMachineFactor(_return, mf);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief 修改仪器系数
///
/// @param[in]  mf  需要修改的项目
///
/// @return 是否成功
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
bool DcsControlProxy::ModifyMachineFactor(const std::vector< ::tf::MachineFactor> & mf)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyMachineFactors(mf);
}

///
/// @brief 查询仪器系数
///
/// @param[in]  _return  返回结果
/// @param[in]  mfqc  查询条件
///
/// @return 是否成功
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
bool DcsControlProxy::QueryMachineFactors(::tf::MachineFactorQueryResp & _return, const::tf::MachineFactorQueryCond mfqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryMachineFactor(_return, mfqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

bool DcsControlProxy::AddCalcAssayInfo(::tf::ResultLong& _return, const ::tf::CalcAssayInfo& cai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddCalcAssayInfo(_return, cai);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

bool DcsControlProxy::DeleteCalcAssayInfo(const ::tf::CalcAssayInfoQueryCond& caiqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->DeleteCalcAssayInfo(caiqc);
}

bool DcsControlProxy::ModifyCalcAssayInfo(const ::tf::CalcAssayInfo& cais)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->ModifyCalcAssayInfo(cais);
}

bool DcsControlProxy::QueryCalcAssayInfo(::tf::CalcAssayInfoQueryResp& _return, const ::tf::CalcAssayInfoQueryCond& caiqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryCalcAssayInfo(_return, caiqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

bool DcsControlProxy::AddCalcAssayTestResult(::tf::ResultLong& _return, const ::tf::CalcAssayTestResult& catr)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddCalcAssayTestResult(_return, catr);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

bool DcsControlProxy::DeleteCalcAssayTestResult(const ::tf::CalcAssayTestResultQueryCond& catrqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->DeleteCalcAssayTestResult(catrqc);
}

bool DcsControlProxy::ModifyCalcAssayTestResult(const ::tf::CalcAssayTestResult& catr)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->ModifyCalcAssayTestResult(catr);
}

bool DcsControlProxy::QueryCalcAssayTestResult(::tf::CalcAssayTestResultQueryResp& _return, const ::tf::CalcAssayTestResultQueryCond& catrqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryCalcAssayTestResult(_return, catrqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @bref
///		更新通用项目信息中的质控规则设定
///
/// @param[in] assayCode 指定的项目编号
/// @param[in] param 质控规则设定
///
/// @par History:
/// @li 8276/huchunli, 2022年12月27日，新建函数
///
bool DcsControlProxy::SetOutQcRules(int assayCode, const tf::QcJudgeParam& param)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->SetOutQcRules(assayCode, param);
}

///
/// @brief 
///     更新HistoryBaseData表的用户设定复查信息
///
/// @param[in]  _return  
/// @param[in]  ri  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月3日，新建函数
///
bool DcsControlProxy::UpdateHistoryBaseDataUserUseRetestInfo(::tf::ResultLong& _return, const ::tf::HistoryBaseDataUserUseRetestUpdate& ri)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->UpdateHistoryBaseDataUserUseRetestInfo(_return, ri);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     更新HistoryBaseData表的项目审核信息
///
/// @param[in]   
///
/// @return 
///
/// @par History:
/// @li 7685/likai，2023年1月3日，新建函数
///
bool DcsControlProxy::UpdateHistoryBaseDataCheckStatus(::tf::ResultLong& _return, const ::tf::HistoryBaseDataCheckUpdate & cs)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->UpdateHistoryBaseDataCheckStatus(_return, cs);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

bool DcsControlProxy::UpdateHistoryBaseDataPrint(const std::vector<int64_t>& samples, bool isPrinted)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (nullptr == m_spDcs)
    {
        ULOG(LOG_ERROR, "m_spDcs is nullptr");
        return false;
    }

    return m_spDcs->UpdateHistoryBaseDataPrinted(samples, isPrinted);
}

bool DcsControlProxy::QueryDictionary(const std::string& keyName, tf::DictionaryInfo& outVaule)
{
	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	tf::DictionaryInfoQueryCond dQuery;
	tf::DictionaryInfoQueryResp dRersp;
	dQuery.__set_keyName(keyName);
    m_spDcs->QueryDictionaryInfo(dRersp, dQuery);
    if (dRersp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
        dRersp.lstDictionaryInfos.empty())
    {
		return false;
    }

	outVaule = dRersp.lstDictionaryInfos[0];
	return true;
}

int DcsControlProxy::GetDictionaryValueInt(const std::string& keyName, int defaultValue)
{
    tf::DictionaryInfo dicInfo;
    if (!QueryDictionary(keyName, dicInfo))
    {
        ULOG(LOG_WARN, "Failed to query %s.", keyName);
        return defaultValue;
    }

    stringutil::IsInteger(dicInfo.value, defaultValue);
    return defaultValue;
}

bool DcsControlProxy::SetDictionaryValueInt(const std::string& keyName, int iValue)
{  
	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	tf::DictionaryInfo dicInfo;
	dicInfo.__set_keyName(keyName);
    if (!QueryDictionary(keyName, dicInfo))
    {
        dicInfo.__set_value(std::to_string(iValue));

        tf::ResultLong retStat;
        m_spDcs->AddDictionaryInfo(retStat, dicInfo);
        if (retStat.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_WARN, "Failed to add dictionary int value.");
            return false;
        }

		return true;
    }

    // 如果有该字段，则进行修改操作
    dicInfo.__set_value(std::to_string(iValue));
    return m_spDcs->ModifyDictionaryInfo(dicInfo);
}

bool DcsControlProxy::GetQcLjNoCalculateFlag()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 不为零则代表设置了显示不计算标记
    return GetDictionaryValueInt(DKN_UI_QC_LJ_SHOWNOCALCULATE, 1);
}

void DcsControlProxy::SetQcLjNoCalculateFlag(bool showNoCalculateFlag)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    SetDictionaryValueInt(DKN_UI_QC_LJ_SHOWNOCALCULATE, showNoCalculateFlag);
}

bool DcsControlProxy::GetQcLjSubPointFlag()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 不为零则代表设置了显示子点标记
    return GetDictionaryValueInt(DKN_UI_QC_LJ_SHOWSUBPOINT, 1);
}

void DcsControlProxy::SetQcLjSubPointFlag(bool showSubPoint)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    SetDictionaryValueInt(DKN_UI_QC_LJ_SHOWSUBPOINT, showSubPoint);
}

///
/// @brief
///     获取质控Tp图不计算点显示标志
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月8日，新建函数
///
bool DcsControlProxy::GetQcTpNoCalculateFlag()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 不为零则代表设置了显示子点标记
    return GetDictionaryValueInt(DKN_UI_QC_TP_SHOWNOCALCULATE, 1);
}

///
/// @brief
///     设置质控Tp图不计算点显示标志
///
/// @param[in]  showNoCalculateFlag  显示不计算点标志
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月8日，新建函数
///
void DcsControlProxy::SetQcTpNoCalculateFlag(bool showNoCalculateFlag)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    SetDictionaryValueInt(DKN_UI_QC_TP_SHOWNOCALCULATE, showNoCalculateFlag);
}

///
/// @bref
///		获取应用项目位置设置界面的搜索框是否显示
///
/// @par History:
/// @li 8276/huchunli, 2023年8月23日，新建函数
///
bool DcsControlProxy::GetAppAssayPositionSearchEditShow()
{
    return GetDictionaryValueInt(DKN_ENGRETAIN_ASSAYPOSSEARCHSHOW, false);
}

int DcsControlProxy::GetQcConfigLengthSn()
{
    ULOG(LOG_INFO, __FUNCTION__);
    return GetDictionaryValueInt(DKN_UI_QC_LENGTH_SN, 3);
}

int DcsControlProxy::GetQcConfigLengthLot()
{
    ULOG(LOG_INFO, __FUNCTION__);
    return GetDictionaryValueInt(DKN_UI_QC_LENGTH_LOT, 7);
}

int DcsControlProxy::GetQcLevelLimit()
{
    ULOG(LOG_INFO, __FUNCTION__);
    return GetDictionaryValueInt(DKN_UI_QC_LEVEL_LIMIT, 6);
}

///
/// @bref
///		获取质控品二维码长度
///
/// @par History:
/// @li 8276/huchunli, 2023年10月9日，新建函数
///
int DcsControlProxy::GetQcBarcodeLength()
{
    ULOG(LOG_INFO, __FUNCTION__);
    return GetDictionaryValueInt(DKN_UI_QC_BARCODE_LENGTH, 498); // 如果获取失败，默认质控二维码长度498
}

///
/// @bref
///		打印设置接口
///
/// @par History:
/// @li 6889/ChenWei, 2023年4月12日，新建函数
///
bool DcsControlProxy::SavePrintSetInfo(const std::vector< ::tf::PrintSetInfo> & pi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->SavePrintSetInfo(pi);
}

void DcsControlProxy::GetPrintSetInfo(std::vector< ::tf::PrintSetInfo> & _return)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return;
	}

	m_spDcs->GetPrintSetInfo(_return);
}

std::string DcsControlProxy::GetSeqStartSampleNumber(const std::string& lastSeqNo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return "";
	}

    std::string seqNo;
	m_spDcs->GetSeqStartSampleNumber(seqNo, lastSeqNo);
    return seqNo;
}

///
/// @bref
///		通过user type 获取对应权限
///
/// @param[in] userType 角色类型
///
/// @par History:
/// @li 8276/huchunli, 2023年6月26日，新建函数
///
bool DcsControlProxy::GetRolePermision(tf::UserType::type userType, tf::PermissionType::type perType, std::set<int>& vecPermission)
{
    ULOG(LOG_INFO, "%s() userType:%d.", __FUNCTION__, userType);
    vecPermission.clear();

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	tf::RolePermissionResp permisResp;
	m_spDcs->GetRolePermission(permisResp, userType, perType);
	if (permisResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Faild to query role permission.");
		return false;
	}

	if (permisResp.lstRolePermissions.size() == 0)
	{
		ULOG(LOG_ERROR, "Result is empty.");
		return false;
	}

	if (!permisResp.lstRolePermissions[0].__isset.permissions || permisResp.lstRolePermissions[0].permissions.empty())
	{
		ULOG(LOG_WARN, "Not exist any permission.");
		return false;
	}

	std::vector<std::string> tempPerm;
	boost::algorithm::split(tempPerm, permisResp.lstRolePermissions[0].permissions, boost::is_any_of(","));
	for (const std::string& strPer : tempPerm)
	{
		int iTarg = -1;
		if (!stringutil::IsInteger(strPer, iTarg))
		{
			ULOG(LOG_ERROR, "Invalid perssion string:%s.", strPer.c_str());
			continue;
		}

		vecPermission.insert(iTarg);
	}

	return true;
}

bool DcsControlProxy::QueryLisConnectionStatus(bool& status)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	status = m_spDcs->QueryLisConnectionStatus();
	return true;
}

///
/// @bref
///		获取免疫的稀释倍数列表
///
/// @param[out] lstDuli 返回的稀释倍数列表
///
/// @par History:
/// @li 8276/huchunli, 2023年8月31日，新建函数
///
bool DcsControlProxy::GetDilutionRatioListIm(std::vector<int>& lstDuli)
{
    lstDuli.clear();

    tf::DictionaryInfo dicInfo;
    if (!QueryDictionary(DKN_IM_DILUTION_TARIO_LIST, dicInfo) && dicInfo.__isset.value)
    {
        ULOG(LOG_WARN, "Failed to query %s.", DKN_IM_DILUTION_TARIO_LIST);
        return false;
    }

    std::vector<std::string> tempElems;
    boost::split(tempElems, dicInfo.value, boost::is_any_of(";"));
    for (int i = 0; i < tempElems.size(); ++i)
    {
        int iDuli = 0;
        if (stringutil::IsInteger(tempElems[i], iDuli))
        {
            lstDuli.push_back(iDuli);
        }
    }

    return true;
}

///
/// @bref
///		设置/获取 项目位置或项目选择界面的单项目行数
///
/// @par History:
/// @li 8276/huchunli, 2023年6月14日，新建函数
///
void DcsControlProxy::SetAssayTabSignleRowAmount(int rowAmount)
{
    ULOG(LOG_INFO, __FUNCTION__);
    SetDictionaryValueInt(DKN_UI_SETTINGS_SINGLEASSAY_ROW_AMOUNT, rowAmount);
}

int DcsControlProxy::GetAssayTabSignleRowAmount()
{
    ULOG(LOG_INFO, __FUNCTION__);
    return GetDictionaryValueInt(DKN_UI_SETTINGS_SINGLEASSAY_ROW_AMOUNT, UI_DEFAULT_ASSAYSELECTTB_SINGLEROW);
}

///
/// @brief  查询满足条件的样本量统计表格
///
/// @param[out]  _return  查询结果
/// @param[in]   sstdqc   查询条件
///
/// @return true 成功 false 失败
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月1日，新建函数
///
bool DcsControlProxy::QuerySampleStatisTab(::tf::SampleStatisTabQueryResp & _return, const::tf::SampleStatisTabDataQueryCond & sstdqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QuerySampleStatisTab(_return, sstdqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询样本量统计饼状图数据
///
/// @param[out]  _return  查询结果
/// @param[in]   sspqc    查询条件
///
/// @return true 成功 false 失败
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月1日，新建函数
///
bool DcsControlProxy::QuerySampleStatisPie(::tf::SampleStatisPieQueryResp & _return, const  ::tf::SampleStatisPieQueryCond& sspqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QuerySampleStatisPie(_return, sspqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询结果统计数据
///
/// @param[out]  _return  返回值
/// @param[in]  rscqc     查询条件
///
/// @return  true 成功 false 失败
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月4日，新建函数
///
bool DcsControlProxy::QueryResultStatisCurve(::tf::ResultStatisCurveQueryResp & _return, const::tf::ResultStatisCurveQueryCond & rscqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryResultStatisCurve(_return, rscqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询校准统计列表
///
/// @param[in]  _return  查询结果集
/// @param[in]  sctqc    查询条件
///
/// @return  true 成功 false 失败 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月8日，新建函数
///
bool DcsControlProxy::QueryStatisCaliTab(::tf::StatisCaliTabQueryResp & _return, const::tf::StatisCaliTabQueryCond & sctqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryStatisCaliTab(_return, sctqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询校准失败原因统计表
///
/// @param[in]  _return  查询结果集
/// @param[in]  scfctqc  查询条件
///
/// @return  true 成功 false 失败 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月8日，新建函数
///
bool DcsControlProxy::QueryStatisCaliFailCauseTab(::tf::StatisCaliFailCauseTabQueryResp & _return, const::tf::StatisCaliFailCauseTabQueryCond & scfctqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryStatisCaliFailCauseTab(_return, scfctqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  新增一条校准统计信息
///
/// @param[in]  _return  返回值
/// @param[in]  sc       校准统计信息
///
/// @return  true 成功 false 失败 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月8日，新建函数
///
bool DcsControlProxy::AddStatisCaliInfo(::tf::ResultLong & _return, const::tf::StatisCali & sc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->AddStatisCaliInfo(_return, sc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询试剂统计曲线
///
/// @param[in]  _return  返回值
/// @param[in]  srcqc    查询条件
///
/// @return  true 成功 false 失败  
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月10日，新建函数
///
bool DcsControlProxy::QueryStatisReagentCurve(::tf::StatisReagentCurveQueryResp & _return, const::tf::StatisReagentCurveQueryCond & srcqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryStatisReagentCurve(_return, srcqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     查询动态计算值
///
/// @param[in]  _return     查询结果
/// @param[in]  isReagent   是否是试剂
/// @param[in]  dcqc        查询条件
///
/// @return true表示查询成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月8日，新建函数
///
bool DcsControlProxy::QueryDynamicCalVal(::tf::DynamicCalcQueryResp& _return, const bool isReagent, const ::tf::DynamicCalcQueryCond& dcqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryDynamicCalVal(_return, isReagent, dcqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief  查询耗材统计曲线
///
/// @param[in]  _return  返回值
/// @param[in]  sscqc    查询条件
///
/// @return  true 成功 false 失败  
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月10日，新建函数
///
bool DcsControlProxy::QueryStatisSupCurve(::tf::StatisSupCurveQueryResp & _return, const::tf::StatisSupCurveQueryCond & sscqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryStatisSupCurve(_return, sscqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询对应统计表中所有的设备序列号（去重）
///
/// @param[out]  _return     返回值
/// @param[in]   scbqc       查询条件
///
/// @return  true 成功 false 失败 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月6日，新建函数
///
bool DcsControlProxy::QueryDeviceSN(::tf::DeviceSNQueryResp & _return, const ::tf::StatisComboBoxQueryCond &scbqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryDeviceSN(_return, scbqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询对应统计表中所有的试剂批号（去重）
///
/// @param[out]  _return     返回值
/// @param[in]   scbqc       查询条件
///
/// @return  true 成功 false 失败 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月6日，新建函数
///
bool DcsControlProxy::QueryReagentLot(::tf::ReagentLotQueryResp & _return, const ::tf::StatisComboBoxQueryCond &scbqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryReagentLot(_return, scbqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询对应统计表中所有的项目通道号（去重）
///
/// @param[out]  _return     返回值
/// @param[in]   scbqc       查询条件
///
/// @return  true 成功 false 失败 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月6日，新建函数
///
bool DcsControlProxy::QueryAssayCode(::tf::AssayCodeQueryResp & _return, const ::tf::StatisComboBoxQueryCond &scbqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryAssayCode(_return, scbqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  查询对应统计表中所有的耗材（去重）
///
/// @param[in]  _return     返回值
/// @param[in]   scbqc       查询条件
///
/// @return  true 成功 false 失败  
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
///
bool DcsControlProxy::QueryConsumables(::tf::ConsumablesQueryResp & _return, const ::tf::StatisComboBoxQueryCond &scbqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->QueryConsumables(_return, scbqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

bool DcsControlProxy::UnlockFlap(const std::vector<std::string> & deviceSNs, const  ::tf::EnumFlapType::type type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

    return m_spDcs->UnlockFlap(deviceSNs, type);
}

///
/// @brief  下载质控数据
///
/// @param[in]  _return  返回值
/// @param[in]  qqc  筛选条件
///
/// @return  true 成功 false 失败  
///
/// @par History:
/// @li 1226/zhangjing，2023年11月28日，新建函数
///
bool DcsControlProxy::DownloadQcDocs(::adapter::tf::CloudQcDocResp& _return, const  ::adapter::tf::QueryCloudQcCond& qqc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->DownloadQcDocs(_return, qqc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief 
///     获取所有的子设备信息（包含轨道模块）
/// @param[in]  vecDevice 子设备信息
///
/// @return true 成功 false 失败 
///
/// @par History:
/// @li 5220/SunChangYan，2024年7月26日，新建函数
///
bool DcsControlProxy::LoadAllSubDeviceInfo(std::vector<::tf::SubDeviceInfo>& vecDevice)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->LoadAllSubDeviceInfo(vecDevice);
	return true;
}

///
/// @brief 
///     按机型获取传感器配置信息
/// @param[in]  _return 传感器配置信息
/// @param[in]  deviceInfo 设备信息
///
/// @return true 成功 false 失败 
///
/// @par History:
/// @li 5220/SunChangYan，2024年7月26日，新建函数
///
bool DcsControlProxy::LoadSensorInfo(std::vector< ::tf::ModuleSensorInfo> & _return, const ::tf::SubDeviceInfo& deviceInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->LoadSensorInfo(_return, deviceInfo);
	return true;
}

///
/// @brief 
///     开启传感器监控
/// @param[in]  deviceInfo 设备信息
///
/// @return true 成功 false 失败 
///
/// @par History:
/// @li 5220/SunChangYan，2024年7月26日，新建函数
///
bool DcsControlProxy::StartSensorMonitor(const ::tf::SubDeviceInfo& deviceInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->StartSensorMonitor(deviceInfo);
}

///
/// @brief 
///     停止传感器监控
/// @param[in]  deviceInfo 设备信息
///
/// @return true 成功 false 失败 
///
/// @par History:
/// @li 5220/SunChangYan，2024年7月26日，新建函数
///
bool DcsControlProxy::StopSensorMonitor(const ::tf::SubDeviceInfo& deviceInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	return m_spDcs->StopSensorMonitor(deviceInfo);
}

///
/// @brief  项目参数URL下载
///
/// @param[in]  _return  返回值
/// @param[in]  qcapc  筛选条件
///
/// @return  true 成功 false 失败  
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月26日，新建函数
///
bool DcsControlProxy::GetCloudAssayParamUrls(::adapter::tf::CloudAssayParamResp& _return, const  ::adapter::tf::QueryCloudAssayParamCond& qcapc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->GetCloudAssayParamUrls(_return, qcapc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  项目参数信息下载
///
/// @param[in]  _return  返回值
/// @param[in]  lscp  通过原来的URL获取
///
/// @return  true 成功 false 失败  
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月26日，新建函数
///
bool DcsControlProxy::GetCloudAssayParamInfos(::adapter::tf::CloudAssayParamResp& _return, const std::vector< ::adapter::tf::CloudAssayParam> & lscp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "m_spDcs is nullptr");
		return false;
	}

	m_spDcs->GetCloudAssayParamInfos(_return, lscp);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

::tf::ResultBool DcsControlProxy::QueryReagentAlarmReaded(const std::string& devSn)
{
	ULOG(LOG_INFO, "%s(devSn=%s)", __FUNCTION__, devSn);

	::tf::ResultBool ret;

	// 获取客户端
	if (nullptr == m_spDcs)
	{
		ULOG(LOG_ERROR, "thrift error: Can not get the client");
		ret.result = ::tf::ThriftResult::THRIFT_RESULT_NO_CONTENT;
		return ret;
	}

	try
	{
		m_spDcs->QueryReagentAlarmReaded(ret, devSn);
	}
	catch (std::exception& ex)
	{
		ULOG(LOG_ERROR, "thrift error: %s", ex.what());
		ret.result = ::tf::ThriftResult::THRIFT_RESULT_EXCEPTION;
		return ret;
	}
	catch (...)
	{
		ULOG(LOG_ERROR, "thrift error: catch unknown exception");
		ret.result = ::tf::ThriftResult::THRIFT_RESULT_UNKNOWN_ERROR;
		return ret;
	}

	ret.result = ::tf::ThriftResult::THRIFT_RESULT_SUCCESS;
	return ret;
}