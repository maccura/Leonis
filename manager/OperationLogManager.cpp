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
/// @file     OperationLogManager.cpp
/// @brief    操作日志管理类，单例对象
///
/// @author   1556/Chenjianlin
/// @date     2023年7月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "src/leonis/manager/OperationLogManager.h"
#include "UserInfoManager.h"
#include "CommonInformationManager.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "shared/uicommon.h"

///
/// @brief 初始化单例对象
///
std::shared_ptr<COperationLogManager> COperationLogManager::s_instance = nullptr;

///
/// @brief 初始化单例锁
///
std::mutex COperationLogManager::m_mutxMgrMutx;


///
/// @brief 
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
COperationLogManager::COperationLogManager(QObject *parent/* = Q_NULLPTR*/)
	: QObject(parent)
{

}

///
/// @brief  日志接口中需要设备编号，随便取一个设备
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年12月14日，新建函数
///
std::string COperationLogManager::GetDeviceSn() const
{
	// 日志接口中需要设备编号，随便取一个设备先
	auto mapDev = CommonInformationManager::GetInstance()->GetDeviceMaps();
	std::string strDevSn(u8"0");
	if (mapDev.size() > 0)
	{
		auto atDev = mapDev.rbegin();
		if (nullptr != atDev->second)
		{
			strDevSn = atDev->second->deviceSN;
		}
	}
	return strDevSn;
}

///
/// @brief 
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
COperationLogManager::~COperationLogManager()
{

}

///
/// @brief 
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
std::shared_ptr<COperationLogManager> COperationLogManager::GetInstance()
{
	if (s_instance == nullptr)
	{
		std::unique_lock<std::mutex> autoLock(m_mutxMgrMutx);
		if (s_instance == nullptr)
		{
			s_instance.reset(new COperationLogManager());
		}
	}
	return s_instance;
}

///
/// @brief 添加操作日志到数据库
///
/// @param[in]  eOptType  操作类型（添加、修改）
/// @param[in]  newDoc   新质控文档
/// @param[in]  spQcDoc  旧质控文档
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
bool COperationLogManager::AddQCOperationLog(::tf::OperationType::type eOptType, const tf::QcDoc& newDoc, const std::shared_ptr<tf::QcDoc> spQcDoc/* = nullptr*/)
{
	// 操作日志信息
	tf::OperationLog log;
	// 获取当前用户名（不是登录名）
	auto atUseInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (nullptr == atUseInfo)
	{
		ULOG(LOG_ERROR, "Has not use info, add operator log failed.");
		return false;
	}
	log.__set_user(atUseInfo->nickname);
	// 日志接口中需要设备编号，随便取一个设备先
	log.__set_deviceSN(GetDeviceSn());
	// 类型为其他
	log.__set_deviceClassify(::tf::AssayClassify::type::ASSAY_CLASSIFY_OTHER);
	// 操作类型
	log.__set_operationType(eOptType);
	// 设置时间 获取当前时间
	auto now = boost::posix_time::microsec_clock::local_time();
	log.__set_operationTime(PosixTimeToTimeString(now));
	// 操作
	QString strRecord("");
	if (::tf::OperationType::type::ADD == eOptType)
	{
		strRecord = tr("新增质控品");
		strRecord += tr("(");
		strRecord += tr("名称:");
		strRecord += tr(newDoc.name.c_str());
		strRecord += tr(";");
		strRecord += tr("编号:");
		strRecord += tr(newDoc.sn.c_str());
		strRecord += tr(";");
		strRecord += tr("批号:");
		strRecord += tr(newDoc.lot.c_str());
		strRecord += tr(")");
	}
	else if (::tf::OperationType::type::MOD == eOptType)
	{
		QString strChange = QString::fromStdString(GetSdChangeOptLogstr(newDoc, spQcDoc));
		if (strChange.isEmpty())
		{
			ULOG(LOG_WARN, "Not change, don't add operator log.");
			return false;
		}
		strRecord = tr("修改质控品");
		strRecord += tr("(");
		strRecord += tr("质控品名称:");
		strRecord += tr(spQcDoc == nullptr ? newDoc.name.c_str() : spQcDoc->name.c_str());
		strRecord += tr(";");
		strRecord += tr("修改内容:");
		strRecord += strChange;
		strRecord += tr(")");
	}
	else if (::tf::OperationType::type::DEL == eOptType)
	{
		strRecord = tr("删除质控品");
		strRecord +=tr("(");
		strRecord +=tr("名称:");
		strRecord +=tr(newDoc.name.c_str());
		strRecord +=tr(";");
		strRecord +=tr("编号:");
		strRecord +=tr(newDoc.sn.c_str());
		strRecord +=tr(";");
		strRecord +=tr("批号:");
		strRecord +=tr(newDoc.lot.c_str());
		strRecord +=tr(")");
	}
	else
	{
		// 未知
		ULOG(LOG_ERROR, "Unknown type, Add qc document operator log failed.");
		return false;
	}
	log.__set_operationRecord(strRecord.toStdString());

	// 调用接口添加操作日志
	tf::ResultLong ret;
	DcsControlProxy::GetInstance()->AddOperationLog(ret, log);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Add qc document operator log failed.");
		return false;
	}
	return true;
}

///
/// @brief 添加免疫校准操作日志
///
/// @param[in]  eOptType  操作类型（添加、删除）
/// @param[in]  caliDocGroup  校准文档
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年12月14日，新建函数
///
bool COperationLogManager::AddCaliOperationLog(::tf::OperationType::type eOptType, const im::tf::CaliDocGroup& caliDocGroup)
{
	// 操作日志信息
	tf::OperationLog log;
	// 获取当前用户名（不是登录名）
	auto atUseInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (nullptr == atUseInfo)
	{
		ULOG(LOG_ERROR, "Has not use info, add operator log failed.");
		return false;
	}
	log.__set_user(atUseInfo->nickname);
	// 日志接口中需要设备编号，随便取一个设备先
	log.__set_deviceSN(GetDeviceSn());
	// 类型为其他
	log.__set_deviceClassify(::tf::AssayClassify::type::ASSAY_CLASSIFY_OTHER);
	// 操作类型
	log.__set_operationType(eOptType);
	// 设置时间 获取当前时间
	auto now = boost::posix_time::microsec_clock::local_time();
	log.__set_operationTime(PosixTimeToTimeString(now));
	// 操作
	QString strRecord("");
	if (::tf::OperationType::type::ADD == eOptType)
	{
		strRecord = tr("新增校准品");
		strRecord += tr("(");
		strRecord += tr("名称:");
		strRecord += tr(caliDocGroup.name.c_str());
		strRecord += tr(";");
		strRecord += tr("批号:");
		strRecord += tr(caliDocGroup.lot.c_str());
		strRecord += tr(")");
	}
	else if (::tf::OperationType::type::DEL == eOptType)
	{
		strRecord =  tr("删除校准品");
		strRecord += tr("(");
		strRecord += tr("名称:");
		strRecord += tr(caliDocGroup.name.c_str());
		strRecord += tr(";");
		strRecord += tr("批号:");
		strRecord += tr(caliDocGroup.lot.c_str());
		strRecord += tr(")");
	}
	else
	{
		// 未知
		ULOG(LOG_ERROR, "Unknown type, Add calibrate document operator log failed.");
		return false;
	}
	log.__set_operationRecord(strRecord.toStdString());

	// 调用接口添加操作日志
	tf::ResultLong ret;
	DcsControlProxy::GetInstance()->AddOperationLog(ret, log);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Add calibrate document operator log failed.");
		return false;
	}
	return true;
}

///
/// @brief 添加参考范围的操作日志
///
/// @param[in]  eOptType  操作类型
/// @param[in]  strAssay  项目名称
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年12月14日，新建函数
///
bool COperationLogManager::AddRangeParamOperationLog(::tf::OperationType::type eOptType, const std::string strAssay)
{
	// 操作日志信息
	tf::OperationLog log;
	// 获取当前用户名（不是登录名）
	auto atUseInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (nullptr == atUseInfo)
	{
		ULOG(LOG_ERROR, "Has not use info, add operator log failed.");
		return false;
	}
	log.__set_user(atUseInfo->nickname);
	// 日志接口中需要设备编号，随便取一个设备先
	log.__set_deviceSN(GetDeviceSn());
	// 类型为其他
	log.__set_deviceClassify(::tf::AssayClassify::type::ASSAY_CLASSIFY_OTHER);
	// 操作类型
	log.__set_operationType(eOptType);
	// 设置时间 获取当前时间
	auto now = boost::posix_time::microsec_clock::local_time();
	log.__set_operationTime(PosixTimeToTimeString(now));
	// 操作
	QString strRecord("");
	if (::tf::OperationType::type::ADD == eOptType)
	{
		strRecord = tr("新增参考范围");
		strRecord += tr("(");
		strRecord += tr("项目:");
		strRecord += tr(strAssay.c_str());
		strRecord += tr(")");
	}
	else if (::tf::OperationType::type::MOD == eOptType)
	{
		strRecord =  tr("修改参考范围");
		strRecord += tr("(");
		strRecord += tr("项目:");
		strRecord += tr(strAssay.c_str());
		strRecord += tr(")");
	}
	else if (::tf::OperationType::type::DEL == eOptType)
	{
		strRecord = tr("删除参考范围");
		strRecord += tr("(");
		strRecord += tr("项目:");
		strRecord += tr(strAssay.c_str());
		strRecord += tr(")");
	}
	else
	{
		// 未知
		ULOG(LOG_ERROR, "Unknown type, Add range parame operator log failed.");
		return false;
	}
	log.__set_operationRecord(strRecord.toStdString());

	// 调用接口添加操作日志
	tf::ResultLong ret;
	DcsControlProxy::GetInstance()->AddOperationLog(ret, log);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Add range parame operator log failed.");
		return false;
	}
	return true;
}

///
/// @brief 添加免疫测试项目的操作日志
///
/// @param[in]  eOptType  操作类型
/// @param[in]  seqNo  样本号
/// @param[in]  barcode  样本条码
/// @param[in]  assayCode  测试项目
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年12月15日，新建函数
///
bool COperationLogManager::AddTestItemOperationLog(::tf::OperationType::type eOptType, const std::string& seqNo, const std::string& barcode, int assayCode)
{
	// 操作日志信息
	tf::OperationLog log;
	// 获取当前用户名（不是登录名）
	auto atUseInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (nullptr == atUseInfo)
	{
		ULOG(LOG_ERROR, "Has not use info, add operator log failed.");
		return false;
	}
	log.__set_user(atUseInfo->nickname);
	// 日志接口中需要设备编号，随便取一个设备先
	log.__set_deviceSN(GetDeviceSn());
	// 类型为其他
	log.__set_deviceClassify(::tf::AssayClassify::type::ASSAY_CLASSIFY_OTHER);
	// 操作类型
	log.__set_operationType(eOptType);
	// 设置时间 获取当前时间
	auto now = boost::posix_time::microsec_clock::local_time();
	log.__set_operationTime(PosixTimeToTimeString(now));
	std::string strAssayName("");
	// 如果是计算项目
	if (tf::AssayCodeRange::CALC_RANGE_MIN <= assayCode && assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
	{
		auto pCalcAssayInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
		if (nullptr != pCalcAssayInfo)
		{
			strAssayName = pCalcAssayInfo->name;
		}
	}
	else
	{
		// 如果是常规项目
		auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
		if (nullptr != spAssayInfo)
		{
			strAssayName = spAssayInfo->assayName;
		}
	}
	// 操作
	QString strRecord("");
	if (::tf::OperationType::type::MOD == eOptType)
	{
		strRecord = QString(tr("修改项目结果, 样本号: %1, 条码: %2, 项目: %3"))
			.arg(QString::fromStdString(seqNo)).arg(QString::fromStdString(barcode)).arg(QString::fromStdString(strAssayName));
	}
	else if (::tf::OperationType::type::DEL == eOptType)
	{
		strRecord = QString(tr("删除项目结果, 样本号: %1, 条码: %2, 项目: %3"))
			.arg(QString::fromStdString(seqNo)).arg(QString::fromStdString(barcode)).arg(QString::fromStdString(strAssayName));
	}
	else
	{
		// 未知
		ULOG(LOG_ERROR, "Unknown type, Add test item operator log failed.");
		return false;
	}
	log.__set_operationRecord(strRecord.toStdString());

	// 调用接口添加操作日志
	tf::ResultLong ret;
	DcsControlProxy::GetInstance()->AddOperationLog(ret, log);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Add test item operator log failed.");
		return false;
	}
	return true;
}

bool COperationLogManager::AddGeneralAssayInfoLog(std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoBefore, std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoAfter)
{
	// 操作日志信息
	tf::OperationLog log;
	std::string strAssayName("");	
	// 类型 (此接口只实现添加和删除，修改在各自的接口取实现)
	::tf::OperationType::type eOptType;
	if (nullptr == spAssayInfoBefore && nullptr!=spAssayInfoAfter)
	{
		eOptType = ::tf::OperationType::type::ADD;
		if (nullptr != spAssayInfoAfter && spAssayInfoAfter->__isset.assayFullName)
		{
			strAssayName = spAssayInfoAfter->assayFullName;
		}
	}
	else
	{
		ULOG(LOG_ERROR, "Just add assay operator log.");
		return false;
	}
	// 获取当前用户名（不是登录名）
	auto atUseInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (nullptr == atUseInfo)
	{
		ULOG(LOG_ERROR, "Has not use info, add operator log failed.");
		return false;
	}
	log.__set_user(atUseInfo->nickname);
	// 日志接口中需要设备编号，随便取一个设备先
	log.__set_deviceSN(GetDeviceSn());
	// 类型为其他
	log.__set_deviceClassify(::tf::AssayClassify::type::ASSAY_CLASSIFY_OTHER);
	// 操作类型
	log.__set_operationType(eOptType);
	// 设置时间 获取当前时间
	auto now = boost::posix_time::microsec_clock::local_time();
	log.__set_operationTime(PosixTimeToTimeString(now));
	
	// 操作
	QString strRecord("");
	if (::tf::OperationType::type::ADD == eOptType)
	{
		strRecord = QString(tr("新增项目参数（项目：%1）")).arg(QString::fromStdString(strAssayName));
	}
	else
	{
		// 未知
		ULOG(LOG_ERROR, "Unknown type, Add assay info operator log failed.");
		return false;
	}
	log.__set_operationRecord(strRecord.toStdString());

	// 调用接口添加操作日志
	tf::ResultLong ret;
	DcsControlProxy::GetInstance()->AddOperationLog(ret, log);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Add assay info operator log failed.");
		return false;
	}
	return true;
}

///
/// @brief 获取靶值、标准差变更内容
///
/// @param[in]  newDocUi 界面设置的数据  
/// @param[in]  spQcDoc 之前的数据  
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月15日，新建函数
///
std::string COperationLogManager::GetSdChangeOptLogstr(const ::tf::QcDoc& newOcDoc, const std::shared_ptr<::tf::QcDoc> spQcDoc)
{
	QString strOptLog("");
	auto FunTvSd = [&strOptLog](::tf::OperationType::type eOptType, const ::tf::QcComposition& qcValNew)
	{
		// 如果是常规项目
		auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(qcValNew.assayCode);
		if (nullptr == spAssayInfo)
		{
			return;
		}
		strOptLog += "{";
		strOptLog += tr("项目名称");
		strOptLog += ":";
		strOptLog += QString::fromStdString(spAssayInfo->assayName);
		strOptLog += (" ");
		strOptLog += tr("靶值");
		strOptLog += ":";
		strOptLog += eOptType == tf::OperationType::DEL ? tr("空") : QString::number(qcValNew.targetValue, 'f', spAssayInfo->decimalPlace);
		strOptLog += (" ");
		strOptLog += tr("标准差");
		strOptLog += ":";
		strOptLog += eOptType == tf::OperationType::DEL ? tr("空") : QString::number(qcValNew.sd, 'f', spAssayInfo->decimalPlace);
		if (eOptType == tf::OperationType::DEL)
		{
			strOptLog += (" ");
			strOptLog += tr("CV%") + ":" + tr("空");
		}
		else
		{
			if (abs(qcValNew.sd) > FLT_EPSILON && abs(qcValNew.targetValue) > FLT_EPSILON)
			{
				double dCv = double(qcValNew.sd) / qcValNew.targetValue * 100;
				if (dCv > FLT_EPSILON)
				{
					strOptLog += (" ");
					strOptLog += tr("CV%");
					strOptLog += ":";
					strOptLog += QString::number(dCv, 'f', spAssayInfo->decimalPlace);
				}
			}
		}
		strOptLog += "}";
	};
	// 如果是新添加
	if (nullptr == spQcDoc)
	{
		for (const auto& atValNew : newOcDoc.compositions)
		{
			FunTvSd(::tf::OperationType::ADD, atValNew);
		}
		return strOptLog.toStdString();
	}
	// 修改之前
	auto vecComp = spQcDoc->compositions;
	// 位置
	if (newOcDoc.rack != spQcDoc->rack || newOcDoc.pos != spQcDoc->pos)
	{
		strOptLog += "{";
		strOptLog += tr("位置");
		strOptLog += ":";
		if (!newOcDoc.rack.empty())
		{
			strOptLog += QString::fromStdString(newOcDoc.rack);
			strOptLog += "-";
			strOptLog += QString::number(newOcDoc.pos);
		}
		else
		{
			strOptLog += tr("空");
		}
		strOptLog += "}";
	}
	// 质控品编号
	QString snNew = newOcDoc.sn.length() > 3 ? QString::fromStdString(newOcDoc.sn.substr(0, 3)) : QString::fromStdString(newOcDoc.sn);
	QString snOld = spQcDoc->sn.length() > 3 ? QString::fromStdString(spQcDoc->sn.substr(0, 3)) : QString::fromStdString(spQcDoc->sn);
	if (snNew != snOld)
	{
		strOptLog += "{";
		strOptLog += tr("质控品编号");
		strOptLog += ":";
		strOptLog += snNew;
		strOptLog += "}";
	}
	// 质控品名称
	if (newOcDoc.name != spQcDoc->name)
	{
		strOptLog += "{";
		strOptLog += tr("质控品名称");
		strOptLog += ":";
		strOptLog += QString::fromStdString(newOcDoc.name);
		strOptLog += "}";
	}
    // 质控品简称
	if (newOcDoc.shortName != spQcDoc->shortName)
	{
		strOptLog += "{";
		strOptLog += tr("质控品简称");
		strOptLog += ":";
		strOptLog += newOcDoc.shortName.empty() ? tr("空") : QString::fromStdString(newOcDoc.shortName);
		strOptLog += "}";
	}
	// 质控品类型
	if (newOcDoc.sampleSourceType != spQcDoc->sampleSourceType)
	{
		strOptLog += "{";
		strOptLog += tr("质控品类型");
		strOptLog += ":";
		strOptLog += ConvertTfEnumToQString(tf::SampleSourceType::type(newOcDoc.sampleSourceType));
		strOptLog += "}";
	}
	// 质控品水平
	if (newOcDoc.level != spQcDoc->level)
	{
		strOptLog += "{";
		strOptLog += tr("质控品水平");
		strOptLog += ":";
		strOptLog += QString::number(newOcDoc.level);
		strOptLog += "}";
	}
	// 质控品批号
	if (newOcDoc.lot != spQcDoc->lot)
	{
		strOptLog += "{";
		strOptLog += tr("质控品批号");
		strOptLog += ":";
		strOptLog += QString::fromStdString(newOcDoc.lot);
		strOptLog += "}";
	}
	// 失效日期
	if (newOcDoc.expireTime != spQcDoc->expireTime)
	{
		strOptLog += "{";
		strOptLog += tr("失效日期");
		strOptLog += ":";
		strOptLog += QString::fromStdString(newOcDoc.expireTime);
		strOptLog += "}";
	}
	// 如果是修改
	for (const auto& atValNew : newOcDoc.compositions)
	{
		bool bFind = false;
		for (auto atValOld = vecComp.begin(); atValOld != vecComp.end();)
		{
			if (atValNew.assayCode != atValOld->assayCode)
			{
				++atValOld;
				continue;
			}
			bFind = true;
			// 靶值、标准差、CV无变化
			if (fabs(atValNew.targetValue - atValOld->targetValue) > FLT_EPSILON ||
				fabs(atValNew.sd - atValOld->sd) > FLT_EPSILON)
			{
				FunTvSd(::tf::OperationType::MOD, atValNew);
			}
			atValOld = vecComp.erase(atValOld);
			break;
		}
		if (!bFind)
		{
			FunTvSd(::tf::OperationType::ADD, atValNew);
		}
	}
	// 如果以前有剩余，说明有删除
	for (const auto atValOld : vecComp)
	{
		FunTvSd(::tf::OperationType::DEL, atValOld);
	}
	return strOptLog.toStdString();
}
