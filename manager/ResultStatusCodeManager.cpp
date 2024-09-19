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
/// @file     ResultStatusCodeManager.cpp
/// @brief    结果状态码的映射与处理
///
/// @author   8276/huchunli
/// @date     2023年9月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年9月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ResultStatusCodeManager.h"
#include <vector>
#include <boost/algorithm/string.hpp>
#include "thrift/DcsControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"


#define CONST_STR_QC_EXCLUDE      "QC.Exclude"


///
/// @brief 初始化单例对象
///
std::shared_ptr<ResultStatusCodeManager> ResultStatusCodeManager::s_instance = nullptr;

///
/// @brief 初始化单例锁
///
std::mutex ResultStatusCodeManager::m_userMgrMutx;


std::shared_ptr<ResultStatusCodeManager> ResultStatusCodeManager::GetInstance()
{
    if (s_instance == nullptr)
    {
        std::unique_lock<std::mutex> autoLock(m_userMgrMutx);
        if (s_instance == nullptr)
        {
            s_instance.reset(new ResultStatusCodeManager());
        }
    }

    return s_instance;
}

///
/// @bref
///		初始化加载结果状态码设置信息
///
/// @par History:
/// @li 8276/huchunli, 2023年9月20日，新建函数
///
void ResultStatusCodeManager::InitResultCodeSet()
{
    ULOG(LOG_INFO, __FUNCTION__);

    m_mapImResCodeSet.clear();
    std::vector<im::tf::ResultCodeSet> vt;
    if (!im::LogicControlProxy::GetResultCodeSet(vt))
    {
        ULOG(LOG_ERROR, "Failed to get im result codeset.");
        return;
    }
    // 依次保存
    for (im::tf::ResultCodeSet& codeSet : vt)
    {
        std::map<std::string, tf::DataAlarmItem>::iterator fItr = m_allDataAlarmsMap.find(codeSet.name);
        if (fItr != m_allDataAlarmsMap.end())
        {
            codeSet.__set_strDiscribe(fItr->second.name);
        }
        m_mapImResCodeSet.emplace(codeSet.name, codeSet);
    }
}

ResultStatusCodeManager::ResultStatusCodeManager()
{
    m_showCaliM = true;
    m_showCaliE = true;
    DictionaryQueryManager::GetCaliLineExpire(m_showCaliE);

    // 从配置文件获取报警码描述
    DcsControlProxy::GetInstance()->QueryDataAlarm(m_allDataAlarms, "ALL");
    m_allDataAlarmsMap.clear();
    for (const tf::DataAlarmItem& item : m_allDataAlarms)
    {
        m_allDataAlarmsMap.insert(std::pair<std::string, tf::DataAlarmItem>(item.code, item));
    }

    InitResultCodeSet();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

ResultStatusCodeManager::~ResultStatusCodeManager()
{

}

///
/// @bref
///		获取免疫状态码与其描述的映射
///
/// @param[out]  imDataAlarms  免疫的数据状态码
///
/// @par History:
/// @li 8276/huchunli, 2023年9月20日，新建函数
///
void ResultStatusCodeManager::GetImResultStatus(std::vector<tf::DataAlarmItem>& imDataAlarms)
{
    imDataAlarms.clear();

    for (const tf::DataAlarmItem& di : m_allDataAlarms)
    {
        if (di.enableIm)
        {
            imDataAlarms.push_back(di);
        }
    }
}

///
/// @brief  修改后需要更新map中的记录
///
/// @param[in]  name  名称
/// @param[in]  rcs   结果状态设置信息
///
/// @return 是否成功
///
/// @par History:
/// @li 1226/zhangjing，2023年4月19日，新建函数
///
bool ResultStatusCodeManager::UpdateResCode(const std::string& name, const im::tf::ResultCodeSet& rcs)
{
    auto iter = m_mapImResCodeSet.find(name);
    if (iter == m_mapImResCodeSet.end())
    {
        return false;
    }

    m_mapImResCodeSet[name] = rcs;
    return true;
}

///
/// @brief  获取显示的结果状态码
///
/// @param[in]  oldCodes  原状态码
/// @param[in]  newCodes  显示状态码
/// @param[in]  result    结果显示
/// @param[in]  refJud    参考范围标志
/// @param[in]  criJud    危机范围标志
///
/// @return 是否成功
///
/// @par History:
/// @li 1226/zhangjing，2023年4月19日，新建函数 
///
bool ResultStatusCodeManager::GetDisplayResCode(const std::string& oldCodes, 
    std::string& newCodes, const std::string& result, int refJud, int criJud, int sampleType)
{
    std::string mediString = oldCodes;

    // 保证结尾最后一个是分号分割
    if ((!mediString.empty()) && (mediString[mediString.length() - 1] != ';'))
    {
        mediString += ";";
    }

    // 根据结果修改结果状态
    if (result.find("<") != std::string::npos && mediString.find("Test.L") == std::string::npos)
    {
        mediString += "Test.L;";
    }
    if (result.find(">") != std::string::npos && mediString.find("Test.H") == std::string::npos)
    {
        mediString += "Test.H;";
    }
    // 参考范围按标识显示
    if (refJud == ::tf::RangeJudge::LESS_LOWER  && mediString.find("Refe.L") == std::string::npos)
    {
        mediString += "Refe.L;";
    }
    if (refJud == ::tf::RangeJudge::ABOVE_UPPER && mediString.find("Refe.H") == std::string::npos)
    {
        mediString += "Refe.H;";
    }
    // 危急范围按标识显示
    if (criJud == ::tf::RangeJudge::LESS_LOWER  && mediString.find("LL") == std::string::npos)
    {
        mediString += "LL;";
    }
    if (criJud == ::tf::RangeJudge::ABOVE_UPPER && mediString.find("HH") == std::string::npos)
    {
        mediString += "HH;";
    }

    // 需要显示忽略
    newCodes = mediString;
    for (const auto& iter : m_mapImResCodeSet)
    {
        if (iter.second.isDisplay)
        {
            continue;
        }
        boost::erase_all(newCodes, iter.second.name + ";");
    }

    // Cali.M显示的权限限制
    if (!m_showCaliM)
    {
        boost::erase_all(newCodes, "Cali.M;");
    }
    // Cali.E显示权限设置
    if (!m_showCaliE)
    {
        boost::erase_all(newCodes, "Cali.E;");
    }

    // 如果是质控记录，则不包含范围报警-bug27510
    if (sampleType == (int)tf::SampleType::type::SAMPLE_TYPE_QC)
    {
        static std::vector<std::string> qcExcludeAlarm = { "Refe.L;", "Refe.H;", "LL;", "HH;" ,"Test.H;" ,"Test.L;" };
        for (const std::string& strx : qcExcludeAlarm)
        {
            boost::erase_all(newCodes, strx);
        }
    }

    // 删除末尾的分号，bug[0022443]
    boost::trim_if(newCodes, boost::is_any_of(";"));

    return true;
}

QString ResultStatusCodeManager::GetChemistryDisplayResCode(const tf::TestItem& dataItem, bool isRetest)
{
	QStringList showStatus;
	QString result;

	// 删除结果的开始的分号和结束分号
	// modify bug0012085 by wuht
	auto formatResult = [](std::string inputData)->QString
	{
		boost::trim_right_if(inputData, boost::is_any_of(";"));
		boost::trim_left_if(inputData, boost::is_any_of(";"));
		return QString::fromStdString(inputData);
	};

	// 是否是血清指数项目
	// 根据与周晓峰最新需求，+++放在结果位置，不需要放置在状态位置
	/*
	bool isSind = CommonInformationManager::GetInstance()->IsAssaySIND(dataItem.assayCode);
	if (isSind)
	{
		// 复查
		if (isRetest)
		{
			if (dataItem.__isset.retestQualitativeJudge && tf::QualJudge::type::FuzzyRegion < dataItem.retestQualitativeJudge)
			{
				int start = dataItem.retestQualitativeJudge - tf::QualJudge::CH_SIND_LEVEL1;
				auto status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(dataItem.assayCode, start);
				showStatus.push_back(status);
			}
		}
		// 初测
		else
		{
			if (dataItem.__isset.firstQualitativeJudge && tf::QualJudge::type::FuzzyRegion < dataItem.firstQualitativeJudge)
			{
				int start = dataItem.firstQualitativeJudge - tf::QualJudge::CH_SIND_LEVEL1;
				auto status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(dataItem.assayCode, start);
				showStatus.push_back(status);
			}
		}
	}*/

	std::string status = isRetest ? \
		(dataItem.__isset.retestResultStatusCodes ? dataItem.retestResultStatusCodes : "") : \
		(dataItem.__isset.resultStatusCodes ? dataItem.resultStatusCodes : "");

	// Cali.M显示的权限限制
	// modify bug0011337 by wuht
	bool showCaliM = UserInfoManager::GetInstance()->IsPermissionShow(PMS_DATAREVIEW_RESULTCODE_CALIM);
	if (!showCaliM)
	{
		boost::erase_all(status, "Cali.M;");
		boost::erase_all(status, "Cali.M");
	}

	// 若禁用码为空，全部显示
	auto shileCodemap = DictionaryQueryManager::GetInstance()->GetShileDataAlarm();
	if (shileCodemap.empty())
	{
		// modify bug0012373 by wuht
		result = (QString::fromStdString(status) + ";" + showStatus.join(";"));
		return formatResult(result.toStdString());
	}

	auto statusmap = QString::fromStdString(status).split(";");
	for (auto stat : statusmap)
	{
		// 若不在禁用码中，则允许显示
		auto iter = shileCodemap.find(stat);
		if (iter == shileCodemap.end())
		{
			showStatus.push_back(stat);
		}
	}

	// 组合返回
	result = showStatus.join(";");
	return formatResult(result.toStdString());
}

QString ResultStatusCodeManager::GetResCode(std::string& status)
{
	QStringList showStatus;
	QString result;

	auto formatResult = [](std::string inputData)->QString
	{
		boost::trim_right_if(inputData, boost::is_any_of(";"));
		boost::trim_left_if(inputData, boost::is_any_of(";"));
		return QString::fromStdString(inputData);
	};

	// Cali.M显示的权限限制
	// modify bug0011337 by wuht
	bool showCaliM = UserInfoManager::GetInstance()->IsPermissionShow(PMS_DATAREVIEW_RESULTCODE_CALIM);
	if (!showCaliM)
	{
		boost::erase_all(status, "Cali.M;");
		boost::erase_all(status, "Cali.M");
	}

	// 若禁用码为空，全部显示
	auto shileCodemap = DictionaryQueryManager::GetInstance()->GetShileDataAlarm();
	if (shileCodemap.empty())
	{
		// modify bug0012373 by wuht
		result = (QString::fromStdString(status) + ";" + showStatus.join(";"));
		return formatResult(result.toStdString());
	}

	auto statusmap = QString::fromStdString(status).split(";");
	for (auto stat : statusmap)
	{
		// 若不在禁用码中，则允许显示
		auto iter = shileCodemap.find(stat);
		if (iter == shileCodemap.end())
		{
			showStatus.push_back(stat);
		}
	}

	// 组合返回
	result = showStatus.join(";");
	return formatResult(result.toStdString());
}

///
///  @brief 获取显示结果状态码的详情
///
///
///  @param[in]   resCodes  显示状态码
///
///  @return	显示结果状态码的详情
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月31日，新建函数
///
std::string ResultStatusCodeManager::GetResCodesDiscrbe(std::string & resCodes)
{
    std::vector<std::string> resCodeVec;
    std::string strResCode;
    boost::split(resCodeVec, resCodes, boost::is_any_of(";"), boost::token_compress_on);
    for (const std::string& strCode : resCodeVec)
    {
        if (strCode.empty())
        {
            continue;
        }

        // 查找描述
        auto fIter = m_allDataAlarmsMap.find(strCode);
		if (fIter == m_allDataAlarmsMap.end())
		{
			ULOG(LOG_WARN, "GetResCodesDiscrbe(%s) failed", strCode.c_str());
		}

        std::string strDiscrip = (fIter == m_allDataAlarmsMap.end()) ? "null" : fIter->second.name;
        strResCode += strCode + ": " + strDiscrip + ";\n";
    }

    return strResCode;
}

///
/// @bref
///		移除状态字符串中的质控排除状态码，返回移除后的状态列表
///
/// @param[in] strStatus 原始状态字符串
///
/// @par History:
/// @li 8276/huchunli, 2023年9月20日，新建函数
///
QStringList ResultStatusCodeManager::EraseQcExcludeSymble(const QString& strStatus)
{
    QStringList strRltStateList = strStatus.split("\n");
    for (auto it = strRltStateList.begin(); it != strRltStateList.end(); it++)
    {
        if ((*it).contains(CONST_STR_QC_EXCLUDE))
        {
            strRltStateList.erase(it);
            break;
        }
    }

    return strRltStateList;
}

///
/// @brief
///     获取质控排除计算结果状态描述
///
/// @return 结果状态描述
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月29日，新建函数
///
QString ResultStatusCodeManager::GetQCExcludeCodeDiscrbe()
{
    auto fIter = m_allDataAlarmsMap.find(CONST_STR_QC_EXCLUDE);
	if (fIter == m_allDataAlarmsMap.end())
	{
		ULOG(LOG_WARN, "GetQCExcludeCodeDiscrbe(%s) failed", CONST_STR_QC_EXCLUDE);
	}
    std::string strDisc = (fIter == m_allDataAlarmsMap.end()) ? "null" : fIter->second.name;
    std::string strShow = std::string(CONST_STR_QC_EXCLUDE) + ": " + strDisc + ";";

    return QString::fromStdString(strShow);
}


///
/// @brief  获取报警码设置
///
/// @param[in]  name  名称
/// @param[in]  rcs   结果状态设置信息
///
/// @return 是否成功
///
/// @par History:
/// @li 1226/zhangjing，2023年12月25日，新建函数
///
bool ResultStatusCodeManager::GetResStatusCodeSet(const std::string& name, im::tf::ResultCodeSet& rcs)
{
    auto iter = m_mapImResCodeSet.find(name);
    if (iter == m_mapImResCodeSet.end())
    {
        return false;
    }

    rcs = m_mapImResCodeSet[name];
    return true;
}

void ResultStatusCodeManager::OnPermisionChanged()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    m_showCaliM = UserInfoManager::GetInstance()->IsPermissionShow(PSM_IM_HISTORY_CALIM);
}
