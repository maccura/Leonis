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
/// @file     QHistorySampleAssayModel.h
/// @brief    样本模块module
///
/// @author   5774/WuHongTao
/// @date     2022年7月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QHistorySampleAssayModel.h"
#include <QPixmap>
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "thrift/DcsControlProxy.h"

#include "src/public/ConfigDefine.h"
#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/StringUtil.h"
#include "src/dcs/dcs.h"
#include "src/dcs/interface/DcsControlHandler.h"
#include "WorkpageCommon.h"
#include "QSampleAssayModel.h"
#include "manager/ResultStatusCodeManager.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include <cctype>
#include <QRegularExpression>
#define  INVALID_VALUE									(-1)

// 搜索的目标字段类型
enum SearchField { SF_SEQ, SF_BARCODE };

// 常用的全局对象
std::shared_ptr<CommonInformationManager> gCommMgr = CommonInformationManager::GetInstance();

// 判断字符串是否是纯数字
bool ContainsNonDigit(const std::string& str)
{
	for(char ch : str)
	{
		if(!std::isdigit(ch))
		{
			return true;
		}
	}
	return false;
}

///
/// @brief
///     结果列的背景色
///
template <typename T>
QVariant AlarmBackGroudColor(const T& dataItem, bool isRecheck)
{
    const std::string& strResult = isRecheck ? dataItem.m_reTestResultStatu : dataItem.m_resultStatu;
    return strResult.empty() ? QVariant() : QVariant("#FFFF99");
};

///
/// @bref
///		删除容器中不显示的计算项目
///
/// @param[in] vecHistoryBaseDataItems 项目容器
///
/// @par History:
/// @li 8276/huchunli, 2024年5月16日，新建函数
///
template<typename T>
void DelateNoDisplayCalcAssays(std::vector<T>& vecHistoryBaseDataItems)
{
    std::vector<T>::iterator it = vecHistoryBaseDataItems.begin();
    for (; it != vecHistoryBaseDataItems.end(); )
    {
        int calcAssayCode = (*it)->m_assayCode;
        if (!CommonInformationManager::IsCalcAssay(calcAssayCode))
        {
            it++;
            continue;
        }
        std::shared_ptr<tf::CalcAssayInfo> spCalcAssay = gCommMgr->GetCalcAssayInfo(calcAssayCode);
        if (spCalcAssay == nullptr || spCalcAssay->display)
        {
            it++;
            continue;
        }
        it = vecHistoryBaseDataItems.erase(it);
        if (it == vecHistoryBaseDataItems.end())
        {
            break;
        }
    }
}

template<typename T>
QString TransDisplayStatuCode(const T& data, bool isReTest)
{
    std::string status = isReTest ? (data.m_reTestResultStatu) : (data.m_resultStatu);
    auto spAssayInfo = gCommMgr->GetAssayInfo(data.m_assayCode);
    if (nullptr == spAssayInfo)
    {
        return QString::fromStdString(status);
    }

    if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        if (isReTest)
        {
            stAssayResult retestResult;
            QHistorySampleAssayModel::GetAssayResult(data, true, retestResult);
            std::string retestResultStatusCodes;
            ResultStatusCodeManager::GetInstance()->GetDisplayResCode(data.m_reTestResultStatu, retestResultStatusCodes, \
                retestResult.m_result.toStdString(), data.m_reTestRefRangeJudge, data.m_reTestCrisisRangeJudge, data.m_sampleType);
            return QString::fromStdString(retestResultStatusCodes);
        }
        else
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(data, false, firstResult);
            std::string resultStatusCodes;
            ResultStatusCodeManager::GetInstance()->GetDisplayResCode(data.m_resultStatu, resultStatusCodes, \
                firstResult.m_result.toStdString(), data.m_refRangeJudge, data.m_crisisRangeJudge, data.m_sampleType);
            return QString::fromStdString(resultStatusCodes);
        }
    }
    // 生化，或者ISE等
    else
    {
        return ResultStatusCodeManager::GetInstance()->GetResCode(status);
    }
}

template<typename T>
QString GetHistoryAssayName(const T& data)
{
    // 项目名称可能变化，因此不使用历史数据中的项目名称，使用内存中保存的项目表的名称
    std::string strAssayName;
    if (CommonInformationManager::IsCalcAssay(data.m_assayCode))
    {
        std::shared_ptr<::tf::CalcAssayInfo> spCalcInfo = gCommMgr->GetCalcAssayInfo(data.m_assayCode);
        strAssayName = (spCalcInfo == nullptr ? data.m_itemName : spCalcInfo->name);
    }
    else
    {
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = gCommMgr->GetAssayInfo(data.m_assayCode);
        strAssayName = (spAssayInfo == nullptr ? data.m_itemName : spAssayInfo->assayName);
    }

    return QString::fromStdString(strAssayName);
}

QString GetSeqNo(::tf::HisSampleType::type sampleType, ::tf::TestMode::type testType, const std::string& seqNo)
{
    // 条码模式下样本号为空，则UI显示为空
    if (seqNo.empty())
    {
        return QString();
    }
    // 若样本在条码模式下，序号只是作为备注，所以不需要加前缀，返回样本的原始信息即可
    if (testType == tf::TestMode::BARCODE_MODE &&
        (sampleType == ::tf::HisSampleType::SAMPLE_SOURCE_NM || sampleType == ::tf::HisSampleType::SAMPLE_SOURCE_EM))
    {
        return QString::fromStdString(seqNo);
    }

    // 构造样本类别
    switch (sampleType)
    {
    case ::tf::HisSampleType::SAMPLE_SOURCE_NM:
        // 常规样本号
        return QString::fromStdString(UiCommon::CombinSeqName("N", seqNo));
    case ::tf::HisSampleType::SAMPLE_SOURCE_EM:
        // 急诊样本号
        return QString::fromStdString(UiCommon::CombinSeqName("E", seqNo));
    case ::tf::HisSampleType::SAMPLE_SOURCE_QC:
        // 质控
        return ("QC" + QString::fromStdString(seqNo));
    case ::tf::HisSampleType::SAMPLE_SOURCE_CL:
        // 校准
        return QString::fromStdString(seqNo);
    default:
        ULOG(LOG_WARN, "Unkown history sample type.");
        break;
    }

    return QString::fromStdString(seqNo);
}

///
/// @brief
///     构建Sql语句,oder by 子句
///
/// @param[in]  cond，筛选条件 
/// @param[out]  sql，根据查询条件构建出的SQL语句 
///
/// @return 
///
/// @par History:
/// @li 7685/likai，2023年8月15日，新建函数
void QHistorySampleAssayModel::BuildOrderByForHistoryBaseData(std::string& Sql, const ::tf::HistoryBaseDataQueryCond & cond)
{
	//排序
    std::string OrderByStr = "";
	if (cond.__isset.orderBy)
	{
		switch (cond.orderBy)
		{
		case ::tf::HistoryDataOrderBy::ORDER_BY_BARCORDE_ASC:
			OrderByStr = " order by barcode ASC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_BARCORDE_DESC:
			OrderByStr = " order by barcode DESC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_SEQNO_ASC:
			OrderByStr = " order by seqNo ASC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_SEQNO_DESC:
			OrderByStr = " order by seqNo DESC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_SAMPLESOURCETYPE_ASC:
			OrderByStr = " order by sampleSourceType ASC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_SAMPLESOURCETYPE_DESC:
			OrderByStr = " order by sampleSourceType DESC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_POS_ASC:
			OrderByStr = " order by pos ASC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_POS_DESC:
			OrderByStr = " order by pos DESC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_ENDTESTTIME_ASC:
			OrderByStr = " order by endTestTime ASC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_ENDTESTTIME_DESC:
			OrderByStr = " order by endTestTime DESC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_RETESTENDTIMEMAX_ASC:
			OrderByStr = " order by reTestEndTime ASC";
			break;
		case ::tf::HistoryDataOrderBy::ORDER_BY_RETESTENDTIMEMAX_DESC:
			OrderByStr = " order by reTestEndTime DESC";
			break;
		default:
			break;
		}
	}

	Sql = OrderByStr;
}

///
/// @brief
///     构建Sql语句,where子句除时间条件之外的语句
///
/// @param[in]  bQuerySample，true表示用于构建按样本时查询样本信息的条件
/// @param[in]  cond，筛选条件 
/// @param[out]  sql，根据查询条件构建出的SQL语句 
///
/// @return 
///
/// @par History:
/// @li 7685/likai，2023年8月15日，新建函数
void QHistorySampleAssayModel::BuildSQLForHistoryBaseData(std::string & Sql, const ::tf::HistoryBaseDataQueryCond & cond, bool bQuerySample/* = false*/)
{
	//and筛选条件
    std::string andQueryStr = "";

	auto ContainsNonDigitFunc = [](const std::string& str)->bool {
		QRegularExpression regex("[^0-9]");
		return QString::fromStdString(str).contains(regex);
	};

	// 样本号固定传两个字符串，都为空时不查询，一个为空时模糊匹配，都不为空且都为数字时范围匹配，1为低值，2为高值，含非数字字符时精准查询两个样本号
	if (cond.__isset.seqNoList && cond.seqNoList.size() == 2)
	{
		if (cond.seqNoList[0] == "" && cond.seqNoList[1] == "")
		{
		}
		// 范围匹配
		else if (cond.seqNoList[0] != "" && cond.seqNoList[1] != "")
		{
			// 包含非数字字符时精准查询
			if (ContainsNonDigitFunc(cond.seqNoList[0]) || ContainsNonDigitFunc(cond.seqNoList[1]))
			{
				andQueryStr += " and (";
				andQueryStr += "seqNo = '";
				andQueryStr += cond.seqNoList[0];
				andQueryStr += "' or ";
				andQueryStr += "seqNo = '";
				andQueryStr += cond.seqNoList[1];
				andQueryStr += "')";
			}
			else
			{
                // 纯数字样本号才适应该筛选
				andQueryStr += " and seqNo REGEXP '^[0-9]+$' and ";
				andQueryStr += "seqNo >= ";
				andQueryStr += cond.seqNoList[0];
				andQueryStr += " and ";
				andQueryStr += "seqNo <= ";
				andQueryStr += cond.seqNoList[1];
			}
		}
		// 模糊匹配
		else
		{
			std::string seqNo = cond.seqNoList[0] != "" ? cond.seqNoList[0] : cond.seqNoList[1];
            boost::replace_all(seqNo, "%", "/%");
			andQueryStr += " and ";
			andQueryStr += "seqNo like ";
			andQueryStr += "'%";
			andQueryStr += seqNo;
			andQueryStr += "%' escape '/' ";
		}
	}

	// 样本条码固定传两个字符串，都为空时不查询，一个为空时模糊匹配，都不为空且都为数字时范围匹配，1为低值，2为高值，含非数字字符时精准查询两个样本号
	if (cond.__isset.sampleBarcodeList && cond.sampleBarcodeList.size() == 2)
	{
		if (cond.sampleBarcodeList[0] == "" && cond.sampleBarcodeList[1] == "")
		{
		}
		// 范围匹配
		else if (cond.sampleBarcodeList[0] != "" && cond.sampleBarcodeList[1] != "")
		{
			// 包含非数字字符时精准查询
			if (ContainsNonDigitFunc(cond.sampleBarcodeList[0]) || ContainsNonDigitFunc(cond.sampleBarcodeList[1]))
			{
				andQueryStr += " and (";
				andQueryStr += "barcode = '";
				andQueryStr += cond.sampleBarcodeList[0];
				andQueryStr += "' or ";
				andQueryStr += "barcode = '";
				andQueryStr += cond.sampleBarcodeList[1];
				andQueryStr += "')";
			}
			else
			{
				andQueryStr += " and ";
				andQueryStr += "barcode >= ";
				andQueryStr += cond.sampleBarcodeList[0];
				andQueryStr += " and ";
				andQueryStr += "barcode <= ";
				andQueryStr += cond.sampleBarcodeList[1];
			}
		}
		// 模糊匹配
		else
		{
			auto barcode = cond.sampleBarcodeList[0] != "" ? cond.sampleBarcodeList[0] : cond.sampleBarcodeList[1];
			andQueryStr += " and ";
			andQueryStr += "barcode like ";
			andQueryStr += "'%";
			andQueryStr += barcode;
			andQueryStr += "%'";
		}
	}

	if (cond.__isset.moduleList)
	{
		//andQueryStr += " and ";
		//andQueryStr += "moduleNo = ";
		// 目前模块号按设备编号查询
		//andQueryStr += "deviceSN = ";
		//andQueryStr += "'";
		//andQueryStr += cond.moduleList[0];
		//andQueryStr += "'";

        // bugfix 0025817: [工作-两联机] 历史数据界面按模块1筛选后打印含两个模块均检测的记录，打印文件中需要显示包含的模块检测的项目
        andQueryStr += " and sampleId in (select distinct sampleId from t_historybasedata where deviceSN ='";
        andQueryStr += cond.moduleList[0];
        andQueryStr += "')";
	}

	if (cond.__isset.caseNo)
	{
		andQueryStr += " and ";
		andQueryStr += "caseNo like ";
		andQueryStr += "'%";
		andQueryStr += cond.caseNo;
		andQueryStr += "%'";
	}

	if (cond.__isset.patientName)
	{
		andQueryStr += " and ";
		andQueryStr += "patientName like ";
		andQueryStr += "'%";
		andQueryStr += cond.patientName;
		andQueryStr += "%'";
	}

	if (cond.__isset.reagentLot)
	{
		andQueryStr += " and ( ";
		andQueryStr += "reagentLot = ";
		andQueryStr += "'";
		andQueryStr += cond.reagentLot;
		andQueryStr += "'";
        // 复查试剂批号也要匹配-bug26508
        andQueryStr += " or ";
        andQueryStr += "reTestReagentLot = ";
        andQueryStr += "'";
        andQueryStr += cond.reagentLot;
        andQueryStr += "' ) ";
	}


    //需区分当前是按样本还是按项目
    //按项目：只要通道号是assayCodeList中的任意一个就查出来
    //按样本：查出来的左边的样本必须做了assayCodeList的所有项目，再把这些符合条件的样本得所有项目在右边展开
    // 按样本时查询同时SQL例子如下:
    /*
    SELECT seqNo,sampleId,barcode,pos
    FROM t_HistoryBaseData WHERE endTestTime >  "2015" and sampleId IN
    (
        SELECT sampleId from
        (
            SELECT DISTINCT assayCode, sampleId from t_HistoryBaseData  WHERE assayCode in ( 3001,3002,3003 )
        ) tmp
        GROUP BY sampleId HAVING count(sampleId) >= 3
    )
    GROUP BY sampleId
    */
	if (cond.__isset.assayCodeList && !cond.assayCodeList.empty())
	{
        int i = 0;
        if (bQuerySample)
        {
            andQueryStr += " and sampleId IN ( SELECT sampleId from ( ";
            andQueryStr += " SELECT DISTINCT assayCode, sampleId from t_HistoryBaseData  WHERE assayCode in ( ";
            for (auto assayCode : cond.assayCodeList)
            {
                i++;
                andQueryStr += to_string(assayCode);
                if (i != cond.assayCodeList.size())
                {
                    andQueryStr += ",";
                }
            }
            andQueryStr += " ) ) tmp GROUP BY sampleId HAVING count(sampleId) >= ";
            andQueryStr += to_string(cond.assayCodeList.size());
            andQueryStr += " ) ";
        }
        else
        {
            andQueryStr += " and assayCode in ( ";
            for (auto assayCode : cond.assayCodeList)
            {
                i++;
                andQueryStr += to_string(assayCode);
                if (i != cond.assayCodeList.size())
                {
                    andQueryStr += ",";
                }
            }
            andQueryStr += " ) ";
        }
    }
    else
    {
        // 只查询存在项目的
        andQueryStr += " and assayCode in ( ";
        AssayIndexCodeMaps allAssay = gCommMgr->GetAssayInfo();
        for (const auto& ait : allAssay)
        {
            andQueryStr += to_string(ait.first);
            andQueryStr += ",";
        }
        CalcAssayInfoMap allCalc = gCommMgr->GetCalcAssayInfoMap();
        for (const auto& calcIt : allCalc)
        {
            andQueryStr += to_string(calcIt.first);
            andQueryStr += ",";
        }

        if (andQueryStr.size() > 0 && andQueryStr[andQueryStr.size()-1] == ',')
        {
            andQueryStr.resize(andQueryStr.size() - 1);
        }
        andQueryStr += " ) ";
    }

	if (cond.__isset.sampleTypeList && cond.sampleTypeList.size() != 4)
	{
		int i = 0;
		andQueryStr += " and(";
		for (auto sampleType : cond.sampleTypeList)
		{
			i++;
			andQueryStr += "sampleType = ";
			andQueryStr += "'";
			andQueryStr += to_string((int)sampleType);
			andQueryStr += "'";
			if (i != cond.sampleTypeList.size())
			{
				andQueryStr += " or ";
			}
		}
		andQueryStr += " ) ";
	}

	if (cond.__isset.sampleSourceTypeList && cond.sampleSourceTypeList.size() != 6)
	{
		int i = 0;
		andQueryStr += " and( ";
		for (auto sampleSourceType : cond.sampleSourceTypeList)
		{
			i++;
			andQueryStr += "sampleSourceType = ";
			andQueryStr += "'";
			andQueryStr += to_string((int)sampleSourceType);
			andQueryStr += "'";
			if (i != cond.sampleSourceTypeList.size())
			{
				andQueryStr += " or ";
			}
		}
		andQueryStr += " ) ";
	}
	if (cond.__isset.retestList && cond.retestList.size() != 2 && cond.retestList.size() > 0)
	{
		int i = 0;
		andQueryStr += " and(";
		for (auto retest : cond.retestList)
		{
			i++;
			andQueryStr += "retest = ";
			andQueryStr += "'";
			andQueryStr += to_string(retest);
			andQueryStr += "'";
			if (i != cond.retestList.size())
			{
				andQueryStr += " or ";
			}
		}
		andQueryStr += " ) ";
	}
	if (cond.__isset.warnList && cond.warnList.size() == 1)
	{
		int i = 0;
        auto warn = cond.warnList[0];
        if (0 == warn)
        {
            if (bQuerySample)
            {
                andQueryStr += " and (resultStatu = '' and reTestResultStatu = '' ) and sampleId not in(select distinct sampleId from t_historybasedata where(resultStatu != '' or reTestResultStatu != '')) ";
            }
            else
            {
                andQueryStr += " and (resultStatu = '' and reTestResultStatu = '' )";
            }
        }
        else
        {
            andQueryStr += " and (resultStatu != '' or reTestResultStatu != '' )";
        }
	}
	if (cond.__isset.checkList && cond.checkList.size() != 2 && cond.checkList.size() > 0)
	{
		int i = 0;
		andQueryStr += " and(";
		for (auto check : cond.checkList)
		{
			i++;
			andQueryStr += "bCheck = ";
			andQueryStr += "'";
			andQueryStr += to_string(check);
			andQueryStr += "'";
			if (i != cond.checkList.size())
			{
				andQueryStr += " or ";
			}
		}
		andQueryStr += " ) ";
	}
	if (cond.__isset.diluteStatuList && cond.diluteStatuList.size() >=1)
	{
		int i = 0;
		andQueryStr += " and( ";
		for (auto diluteStatu : cond.diluteStatuList)
		{
			i++;
			andQueryStr += "diluteStatu = ";
			andQueryStr += "'";
			andQueryStr += to_string((int)diluteStatu);
			andQueryStr += "'";
			if (i != cond.diluteStatuList.size())
			{
				andQueryStr += " or ";
			}
		}
		andQueryStr += " ) ";
	}

	if (cond.__isset.qualitativeResultList)
	{
        if (1 == cond.qualitativeResultList.size())
        {
            andQueryStr += " and ";
            andQueryStr += "qualitativeResult = ";
            andQueryStr += "'";
            andQueryStr += to_string(cond.qualitativeResultList[0]);
            andQueryStr += "'";
        }
        else if (2 == cond.qualitativeResultList.size())
        {
            andQueryStr += " and ( ";
            andQueryStr += "qualitativeResult = ";
            andQueryStr += "'";
            andQueryStr += to_string(cond.qualitativeResultList[0]);
            andQueryStr += "'";
            andQueryStr += " or ";
            andQueryStr += "qualitativeResult = ";
            andQueryStr += "'";
            andQueryStr += to_string(cond.qualitativeResultList[1]);
            andQueryStr += "'";
            andQueryStr += " ) ";
        }

	}
	if (cond.__isset.sendLISList && cond.sendLISList.size() != 2 && cond.sendLISList.size() > 0)
	{
		int i = 0;
		andQueryStr += " and(";
		for (auto sendLis : cond.sendLISList)
		{
			i++;
			andQueryStr += "sendList = ";
			andQueryStr += "'";
			andQueryStr += to_string(sendLis);
			andQueryStr += "'";
			if (i != cond.sendLISList.size())
			{
				andQueryStr += " or ";
			}
		}
		andQueryStr += " ) ";
	}
	if (cond.__isset.printList && cond.printList.size() != 2 && cond.printList.size() > 0)
	{
		int i = 0;
		andQueryStr += " and(";
		for (auto print : cond.printList)
		{
			i++;
			andQueryStr += "print = ";
			andQueryStr += "'";
			andQueryStr += to_string(print);
			andQueryStr += "'";
			if (i != cond.printList.size())
			{
				andQueryStr += " or ";
			}
		}
		andQueryStr += " ) ";
	}
	Sql = andQueryStr;
}

///
/// @brief
///     构建Sql语句,按样本查询时的样本相关信息，对应HistoryData.hxx的HistoryBaseDataSample视图
///
/// @param[in]  cond，筛选条件 
/// @param[out]  sql，根据查询条件构建出的SQL语句 
///
/// @return 
///
/// @par History:
/// @li 7685/likai，2023年8月15日，新建函数
bool QHistorySampleAssayModel::CondToSql_Sample(const ::tf::HistoryBaseDataQueryCond& cond,std::string& sql)
{
	std::string sqlOrderBy = "";
	BuildOrderByForHistoryBaseData(sqlOrderBy, cond);
	{
		//一、输入参数合法性检查及拼接时间查询条件
		//当筛选条件勾选了有复查时，查询的时间范围指最后一次复查时间的范围，当勾选了无复查时，查询时间范围指首查的时间范围
		//当未选中有复查和无复查时，查询的开始时间是指首查时间，结束时间是指最后一次复查时间
        std::string timeQueryStr;
		if (cond.__isset.retestList && 1 == cond.retestList.size())
		{
			if (1 == cond.retestList[0])
			{
				//有复查
				if (!(cond.__isset.reTestStartTime && cond.__isset.reTestEndtime))
				{
                    ULOG(LOG_ERROR, "query param error，cond：%s", ToString(cond).c_str());
					return false;
				}
				timeQueryStr = " reTestEndTime >= '" + cond.reTestStartTime + "' and reTestEndTime <= '" + cond.reTestEndtime + "'";
			}
			else
			{
				//无复查
				if (!(cond.__isset.startTime&&cond.__isset.endtime))
				{
                    ULOG(LOG_ERROR, "query param error，cond：%s", ToString(cond).c_str());
					return false;
				}
				timeQueryStr = " endTestTime >= '" + cond.startTime + "' and endTestTime <= '" + cond.endtime + "'";
			}
		}
		else
		{
			//未选或全选（等效）
			if (!(cond.__isset.startTime&&cond.__isset.reTestEndtime))
			{
                ULOG(LOG_INFO, "query param error，cond：%s", ToString(cond).c_str());
				return false;
			}
			timeQueryStr = " ((endTestTime >= '" + cond.startTime + "' and endTestTime <= '" + cond.endtime + "') or" + \
				" (reTestEndTime >= '" + cond.startTime + "' and reTestEndTime <= '" + cond.endtime + "'))";
		}

		//二、拼装其他查询条件
        std::string sqlAnd = "";
		BuildSQLForHistoryBaseData(sqlAnd, cond, true);

		sql.clear();
		sql += "select sampleId,seqNo,barcode,sampleSourceType,pos,max(endTestTime),max(reTestEndTime),bCheck,\
				sampleType,testMode,tubeType,patientId,aiRecognizeResultId,comment,sendList,print,retest \
				from t_HistoryBaseData where ";
		sql += timeQueryStr;
		sql += sqlAnd;
		sql += " group by sampleId ";
		sql += sqlOrderBy;
		sql += ";";
	}

	return true;
}

///
/// @brief
///     构建Sql语句,按样本查询时的项目相关信息，对应HistoryData.hxx的HistoryBaseDataItem视图
///
/// @param[in]  sampleID，查询指定样本的项目 
/// @param[in]  cond，筛选条件 
/// @param[out]  sql，根据查询条件构建出的SQL语句 
///
/// @return 
///
/// @par History:
/// @li 7685/likai，2023年8月15日，新建函数
bool QHistorySampleAssayModel::CondToSql_Item(const ::tf::HistoryBaseDataQueryCond& cond, std::string& sql,const int64_t sampleID)
{
	if (sampleID < 0)
	{
        ULOG(LOG_INFO, "query param error，cond：%s,sampleID:%lld", ToString(cond).c_str(), sampleID);
		return false;
	}

	//二、拼装其他查询条件
	std::string sqlAnd = "";
	BuildSQLForHistoryBaseData(sqlAnd, cond);

	sql.clear();
	sql+= "select itemName,suckVolType,diluteStatu,dilutionFactor,testResult,resultStatu,reTestResult,reTestResultStatu,unit,moduleNo,itemId,deviceSN,sampleId,patientId,sampleType,seqNo,barcode,"\
        " assayCode,sendList,sampleSourceType,pos,userUseRetestResultID,bCheck,print,qualitativeResult,refRangeJudge,crisisRangeJudge,reTestQualitativeResult,reTestRefRangeJudge,reTestCrisisRangeJudge,RLU,reTestRLU,retest,reTestDilutionFactor,reSuckVolType,reTestDiluteStatu "\
		" from t_HistoryBaseData where sampleId = ";
	sql += std::to_string(sampleID);
    //sql += sqlAnd; // 按需求，按样本展示时显示的是满足筛选条件的样本，右侧为该样本的所有项目，因此不需要包含其他筛选条件 add by wzx-20231201
    sql += ";";

	return true;
}

///
/// @brief
///     构建Sql语句,按项目查询时的基本信息，对应HistoryData.hxx的HistoryBaseDataByItem视图
///
/// @param[in]  cond，筛选条件 
/// @param[out]  sql，根据查询条件构建出的SQL语句 
///
/// @return 
///
/// @par History:
/// @li 7685/likai，2023年8月15日，新建函数
bool QHistorySampleAssayModel::CondToSqlByItem(const ::tf::HistoryBaseDataQueryCond& cond, std::string& sql)
{
    std::string sqlOrderBy = "";
	BuildOrderByForHistoryBaseData(sqlOrderBy, cond);

	{
		//一、输入参数合法性检查及拼接时间查询条件
		//当筛选条件勾选了有复查时，查询的时间范围指最后一次复查时间的范围，当勾选了无复查时，查询时间范围指首查的时间范围
		//当未选中有复查和无复查时，查询的开始时间是指首查时间，结束时间是指最后一次复查时间
        std::string timeQueryStr;
		if (cond.__isset.retestList && 1 == cond.retestList.size())
		{
			if (1 == cond.retestList[0])
			{
				//有复查
				if (!(cond.__isset.reTestStartTime && cond.__isset.reTestEndtime))
				{
                    ULOG(LOG_ERROR, "query param error，cond：%s", ToString(cond).c_str());
					return false;
				}
				timeQueryStr = " reTestEndTime >= '" + cond.reTestStartTime + "' and reTestEndTime <= '" + cond.reTestEndtime + "'";
			}
			else
			{
				//无复查
				if (!(cond.__isset.startTime&&cond.__isset.endtime))
				{
                    ULOG(LOG_ERROR, "query param error，cond：%s", ToString(cond).c_str());
					return false;
				}
				timeQueryStr = " ((endTestTime >= '" + cond.startTime + "' and endTestTime <= '" + cond.endtime + "') or" + \
					" (reTestEndTime >= '" + cond.startTime + "' and reTestEndTime <= '" + cond.endtime + "'))";
			}
		}
		else
		{
			//未选或全选（等效）
			if (!(cond.__isset.startTime&&cond.__isset.reTestEndtime))
			{
                ULOG(LOG_INFO, "query param error，cond：%s", ToString(cond).c_str());
				return false;
			}
			timeQueryStr = " ((endTestTime >= '" + cond.startTime + "' and endTestTime <= '" + cond.endtime + "') or" + \
				" (reTestEndTime >= '" + cond.startTime + "' and reTestEndTime <= '" + cond.endtime + "'))";
		}

		//二、拼装其他查询条件
        std::string sqlAnd = "";
		BuildSQLForHistoryBaseData(sqlAnd, cond);

		// 查询
		sql.clear();
		sql += "select seqNo,barcode,sampleSourceType,pos,endTestTime,reTestEndTime,";
		sql += "itemName,suckVolType,diluteStatu,dilutionFactor,testResult,resultStatu,reTestResult,reTestResultStatu,unit,bCheck,moduleNo,itemId,deviceSN,sampleId,patientId,sampleType,assayCode,sendList,userUseRetestResultID,";
        sql += "print,qualitativeResult,refRangeJudge,crisisRangeJudge,reTestQualitativeResult,reTestRefRangeJudge,reTestCrisisRangeJudge,RLU,reTestRLU,retest,reTestDilutionFactor,reSuckVolType,reTestDiluteStatu ";
		sql += " from t_HistoryBaseData where ";
		sql += timeQueryStr;
		sql += sqlAnd;
		sql += sqlOrderBy;
		sql += ";";
	}

	return true;
}

///
/// @brief 更新病人信息
///
/// @param[in]  sampleId		样本ID
/// @param[in]  patientId		病人数据库主键ID
/// @param[in]  patientName		病人姓名
/// @param[in]  caseNo			病例号
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月25日，新建函数
///
void QHistorySampleAssayModel::UpdatePatienInfo(const int64_t& sampleId, const int64_t& patientId, const std::string& patientName, const std::string& caseNo)
{
	std::string sql;
	sql += "update t_HistoryBaseData set patientId='" + std::to_string(patientId) + "'";
	sql += ",patientName='" + patientName + "'";
	sql += ",caseNo='" + caseNo + "'";
	sql += "where sampleId='" + std::to_string(sampleId) + "'";
	sql += ";";

	DcsControlHandler dcs;
	dcs.UpdateHistoryPatientInfo(sql);
}

QHistorySampleAssayModel::QHistorySampleAssayModel()
{
	m_moudleType = VIEWMOUDLE::SAMPLEBROWSE;
	m_moduleStatus = MOUDLESTATUS::UNKONW;
	m_assayBrowseHeader << " " << tr("状态");
	m_sampleBrowseHeader = m_assayBrowseHeader;
	// 按照项目浏览
	m_assayBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::ASSAYLIST);
	// 按照样本浏览
	m_sampleBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLE);
	m_bCheckAll = false;

	// 数据更新
	connect(&SampleColumn::Instance(), &SampleColumn::ColumnChanged, this, [&]()
	{
		m_assayBrowseHeader.clear();
		m_sampleBrowseHeader.clear();
		m_assayBrowseHeader << " " << tr("状态");
		m_sampleBrowseHeader = m_assayBrowseHeader;
		// 按照项目浏览
		m_assayBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::ASSAYLIST);
		// 按照样本浏览
		m_sampleBrowseHeader += SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLE);
		this->beginResetModel();
		this->endResetModel();
	});
}

///
/// @brief 审核样本
///
/// @param[in]  indexs	选中要审核的样本索引
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月5日，新建函数
///
void QHistorySampleAssayModel::CheckSample(const QModelIndexList &indexs)
{
	if (indexs.isEmpty())
	{
		return;
	}

	if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
	{
		// 必须保证在范围以内
		int row = indexs.at(0).row();
		if (row >= m_vecHistoryBaseDataSamples.size() || row < 0)
		{
			return;
		}

		// 多个样本审核时取第一个样本的审核状态
		bool bCheck = m_vecHistoryBaseDataSamples.at(row)->m_bCheck;
		bCheck = !bCheck;// 已审核变为未审核，未审核变为已审核

		std::vector<int64_t> vecSampleID;
		for (auto index : indexs)
		{
			if (!index.isValid() || index.row() >= m_vecHistoryBaseDataSamples.size())
			{
				continue;
			}

			auto sampleID = QHistorySampleAssayModel::Instance().GetSampleID(index);
			if (sampleID == -1)
			{
				continue;
			}
			vecSampleID.push_back(sampleID);
		}
		CheckSampleBySampleID(vecSampleID, bCheck);
		
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)	// 按项目展示时无法审核
	{
		return;
	}
}

///
/// @brief 获取样本审核状态
///
/// @param[in]  index  
///
/// @return 1-已审核，0-未审核，-1-无法获取
///
/// @par History:
/// @li 7702/WangZhongXin，2023年2月17日，新建函数
///
int QHistorySampleAssayModel::GetCheck(const QModelIndex &index) const
{
	if (!index.isValid())
	{
		return -1;
	}

    int iCurrRow = index.row();
	if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
	{
        return (iCurrRow >= m_vecHistoryBaseDataSamples.size()) ? -1 : m_vecHistoryBaseDataSamples[iCurrRow]->m_bCheck;
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
        return (iCurrRow >= m_vecHistoryBaseDataItems.size()) ? -1 : m_vecHistoryBaseDataItems[iCurrRow]->m_bCheck;
	}
	return -1;
}

///
/// @brief 是否可以审核
///
/// @param[in]  index  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月19日，新建函数
///
bool QHistorySampleAssayModel::CanCheck(const QModelIndex &index)
{
	if (GetModule() == VIEWMOUDLE::ASSAYBROWSE)	// 项目展示模式无法审核
	{
		return false;
	}
	else if (GetModule() == VIEWMOUDLE::SAMPLEBROWSE)	// 样本展示时判断index是否合法
	{
		return index.row() >= m_vecHistoryBaseDataSamples.size() ? false : true;
	}
	return false;
}

///
/// @bref
///		执行搜索
///
/// @par History:
/// @li 8276/huchunli, 2023年12月25日，新建函数（提取自LikeSearch
///
template <typename T>
int ProcSearch(const std::vector<T>& vecData, const QString& strWorld, SearchField qType, int direction, int& curIdx)
{
    // 查找容器为空，退出
    if (vecData.empty())
    {
        return -1;
    }
    //bug29112 历史界面查找下一条有误修正
    int tempIndex = curIdx;
    int LastIndex = curIdx;

    if (direction == 0)// 匹配第一个时从头找
    {
        curIdx = 0;
        tempIndex = 0;
    }
    else if (direction == 1) // 向上找
    {
        tempIndex--;
    }
    else if (direction == 2) // 向下找
    {
        tempIndex++;
    }

    // 索引越界，无法查找
    if (tempIndex < 0)
    {
        curIdx = LastIndex;
        return -1;
    }
    else if (tempIndex >= vecData.size())
    {
        curIdx = LastIndex;
        return -2;
    }
    curIdx = tempIndex;
    for (; curIdx < vecData.size() && curIdx >= 0;)
    {
        const auto& d = vecData[curIdx];
        // 此处为字符串已全信息进行处理，test mode,暂时默认为seq-mode.
        QString strField = (qType == SF_SEQ) ? GetSeqNo(::tf::HisSampleType::type(d->m_sampleType), tf::TestMode::type::SEQNO_MODE, d->m_seqNo) : QString::fromStdString(d->m_barcode);
        if (strField.contains(strWorld))
        {
            return curIdx;
        }
        // 未找到时根据查找方向更改索引
        else
        {
            if (direction == 0)// 匹配第一个时从头找
            {
                curIdx++;
            }
            else if (direction == 1) // 向上找
            {
                curIdx--;
            }
            else if (direction == 2) // 向下找
            {
                curIdx++;
            }
        }
    }
    if (curIdx < 0)
    {
        return -1;
    }
    else if (curIdx >= vecData.size())
    {
        return -2;
    }

    return -2;
}

///
/// @brief 模糊查询
///
/// @param[in]  str		模糊查询匹配字符串
/// @param[in]  type	模糊查询类型字符串
/// @param[in]  direction	模糊查询方向(0-匹配第一个，1-向上，2-向下)
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月17日，新建函数
///
int QHistorySampleAssayModel::LikeSearch(const QString& str, const QString& type, const int& direction, int beginPos)
{
    int iFindIdx = -1;
	if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
	{
        SearchField sField = (m_sampleBrowseHeader.indexOf(type) == static_cast<int>(COL::SEQNO)) ? SF_SEQ : SF_BARCODE;
        iFindIdx = ProcSearch(m_vecHistoryBaseDataSamples, str, sField, direction, beginPos);
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
        SearchField sField = (m_assayBrowseHeader.indexOf(type) == static_cast<int>(COLASSAY::SEQNO)) ? SF_SEQ : SF_BARCODE;
        iFindIdx = ProcSearch(m_vecHistoryBaseDataItems, str, sField, direction, beginPos);
	}

    if (iFindIdx >= 0)
    {
        emit HightLightChanged(iFindIdx);
    }

    return iFindIdx;
}

///
///  @brief 获取选中的样本数据
///
///
///  @param[in]   selectedIndexs  选中的样本索引列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
///
void QHistorySampleAssayModel::GetSelectedSamples(const QModelIndexList & selectedIndexs, \
    std::vector<std::shared_ptr<HistoryBaseDataSample>>& selectedSamples) const
{
    selectedSamples.clear();
	if (m_vecHistoryBaseDataSamples.empty())
	{
		return;
    }
	for (auto it : selectedIndexs)
	{
		if (!it.isValid())
		{
			continue;
		}
		int row = it.row();
		if (row < 0 || row >= m_vecHistoryBaseDataSamples.size())
		{
			continue;
		}
		selectedSamples.push_back(m_vecHistoryBaseDataSamples[row]);
	}
}

///
///  @brief 获取选中的项目数据
///
///
///  @param[in]   selectedIndexs  选中的项目索引列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
///
std::vector<std::shared_ptr<HistoryBaseDataByItem>> QHistorySampleAssayModel::GetSelectedItems(const QModelIndexList & selectedIndexs)
{
	std::vector<std::shared_ptr<HistoryBaseDataByItem>> selectedItems;
	int row = -1;
	if (m_vecHistoryBaseDataItems.empty())
	{
		return std::move(selectedItems);
	}
	for (auto it : selectedIndexs)
	{
		if (!it.isValid())
		{
			continue;
		}
		row = it.row();
		if (row < 0 || row > m_vecHistoryBaseDataItems.size())
		{
			continue;
		}
		selectedItems.push_back(m_vecHistoryBaseDataItems[it.row()]);
	}
	return std::move(selectedItems);
}

///
///  @brief 获取选中样本数据包含的项目数据
///
///
///  @param[in]   selectedSamples  选中的样本数据列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
///
void QHistorySampleAssayModel::GetItemsInSelectedSamples(const std::vector<std::shared_ptr<HistoryBaseDataSample>>& selectedSamples, \
    std::vector<std::shared_ptr<HistoryBaseDataByItem>>& relatedAssays)
{
    relatedAssays.clear();
	for (const std::shared_ptr<HistoryBaseDataSample>& sample : selectedSamples)
	{
		for (const auto& item : m_vecHistoryBaseDataItems)
		{
			if (sample->m_sampleId == item->m_sampleId)
			{
                relatedAssays.push_back(item);
			}
		}
	}
}

std::shared_ptr<HistoryBaseDataByItem> QHistorySampleAssayModel::GetItemInSelectedSample(int64_t sampleId, int64_t itemId)
{
    for (const auto& item : m_vecHistoryBaseDataItems)
    {
        if (sampleId == item->m_sampleId && itemId == item->m_itemId)
        {
            return item;
        }
    }

    return nullptr;
}

///
///  @brief 获取选中项目数据的样本数据
///
///
///  @param[in]   pItem  选中的样本数据列表
///
///  @return	
///
///  @par History: 
///  @li 6889/ChenWei，2024年4月16日，新建函数
///
void QHistorySampleAssayModel::GetSamplesInSelectedItems(const std::shared_ptr<HistoryBaseDataByItem>& pItem, \
    std::shared_ptr<HistoryBaseDataSample>& pSample)
{
    for (std::shared_ptr<HistoryBaseDataSample> pS : m_vecHistoryBaseDataSamples)
    {
        if (pS->m_sampleId == pItem->m_sampleId)
        {
            pSample = pS;
            break;
        }
    }
}


///
/// @brief 设置全勾选还是全不勾选
///
/// @param[in]  bCheck  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月24日，新建函数
///
void QHistorySampleAssayModel::SetOrClearAllCheck(bool bCheck)
{
	auto& vecCheck = m_moudleType == VIEWMOUDLE::SAMPLEBROWSE ? m_setHistoryBaseDataSamplesCheck : m_setHistoryBaseDataItemsCheck;
    m_bCheckAll = bCheck;

	vecCheck.clear();

	auto changedIndex1 = this->index(0, static_cast<int>(COL::Check));
	auto changedIndex2 = this->index(vecCheck.size() - 1, static_cast<int>(COL::Check));
	emit dataChanged(changedIndex1, changedIndex2, { Qt::DisplayRole });
}

///
/// @brief 设置全勾选还是全不勾选
///
/// @param[in]  index  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月24日，新建函数
///
void QHistorySampleAssayModel::SetCheckBoxStatus(const QModelIndex& index, bool IscontrolKeyPress)
{
	if (!index.isValid())
	{
		return;
	}
	auto indexRow = index.row();
	auto& vecCheck = (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE) ? m_setHistoryBaseDataSamplesCheck : m_setHistoryBaseDataItemsCheck;
	auto size = m_moudleType == VIEWMOUDLE::SAMPLEBROWSE ? m_vecHistoryBaseDataSamples.size() : m_vecHistoryBaseDataItems.size();
	auto checkCol = m_moudleType == VIEWMOUDLE::SAMPLEBROWSE ? static_cast<int>(COL::Check) : static_cast<int>(COLASSAY::Check);
	// 添加有空白行，可能越界了，清空之前的勾选并刷新
	if (indexRow >= size)
	{
		m_bCheckAll = false;
		vecCheck.clear();
		emit dataChanged(this->index(0, checkCol), this->index(size, checkCol), { Qt::DisplayRole });
		return;
	}

	// 不是点击勾选列，则只选中一行，其他行设为不勾选
	if (checkCol != index.column())
	{
		if (m_bCheckAll)
		{
			m_bCheckAll = false;
			// 当没有按下controlkey的时候，不需要清除
			if (!IscontrolKeyPress)
			{
				vecCheck.clear();
			}

			vecCheck.insert(indexRow);

			auto changedIndex1 = this->index(0, checkCol);
			auto changedIndex2 = this->index(size - 1, checkCol);

			emit dataChanged(changedIndex1, changedIndex2, { Qt::DisplayRole });
		}
        else {
			// 当没有按下controlkey的时候，不需要清除
			if (!IscontrolKeyPress)
			{
				for (auto it = vecCheck.begin(); it != vecCheck.end();)
				{
					auto uncheckRow = *it;
					it = vecCheck.erase(it);

					auto changedIndex1 = this->index(uncheckRow, checkCol);
					auto changedIndex2 = this->index(uncheckRow, checkCol);

					emit dataChanged(changedIndex1, changedIndex2, { Qt::DisplayRole });
				}
			}

			vecCheck.insert(indexRow);

			auto changedIndex1 = this->index(indexRow, checkCol);
			auto changedIndex2 = this->index(indexRow, checkCol);

			emit dataChanged(changedIndex1, changedIndex2, { Qt::DisplayRole });
		}
	}
	// 是勾选行，则只修改该行
	else
	{
		// 全选时点击某一行，则其他行全是勾选
		if (m_bCheckAll)
		{
			m_bCheckAll = false;
			for (int i = 0; i < size; ++i)
			{
				if (i != indexRow)
				{
					vecCheck.insert(i);
				}
			}

			auto changedIndex1 = this->index(0, checkCol);
			auto changedIndex2 = this->index(size - 1, checkCol);

			emit dataChanged(changedIndex1, changedIndex2, { Qt::DisplayRole });
		}
		else
		{
            // 找到了则该行是勾选行，需要取消勾选
            auto it = vecCheck.find(indexRow);
			if (it != vecCheck.end())
			{
				vecCheck.erase(it);
			}
			// 未找到则该行不是勾选行，需要添加勾选
			else
			{
				vecCheck.insert(indexRow);
			}
			auto changedIndex = this->index(index.row(), checkCol);
			emit dataChanged(changedIndex, changedIndex, { Qt::DisplayRole });
		}
	}
}

///
/// @brief 获取勾选的索引
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月24日，新建函数
///
QModelIndexList QHistorySampleAssayModel::GetCheckedModelIndex() const
{
	int dataSize = (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE) ? m_vecHistoryBaseDataSamples.size() : m_vecHistoryBaseDataItems.size();

	QModelIndexList indexs;
	if (m_bCheckAll)
	{
		for (int i = 0; i < dataSize; ++i)
		{
			indexs.append(index(i, 0));
		}
	}
	else
	{
		auto& setCheck = m_moudleType == VIEWMOUDLE::SAMPLEBROWSE ? m_setHistoryBaseDataSamplesCheck : m_setHistoryBaseDataItemsCheck;
		for (std::set<int>::iterator it = setCheck.begin(); it != setCheck.end(); ++it)
		{
			indexs.append(index(*it, 0));
		}
	}
	return indexs;
}

bool QHistorySampleAssayModel::IsSelectAll()
{
	int dataSize = (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE) ? m_vecHistoryBaseDataSamples.size() : m_vecHistoryBaseDataItems.size();
	int selectSize = (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE) ? m_setHistoryBaseDataSamplesCheck.size() : m_setHistoryBaseDataItemsCheck.size();

	if (dataSize == 0)
	{
		return false;
	}

    return (selectSize >= dataSize);
}

///
///  @brief 获取审核字符串
///
///
///  @param[in]   iCheck  审核枚举值
///
///  @return	审核字符串
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月19日，新建函数
///
QString QHistorySampleAssayModel::GetCheck(const int iCheck)
{
	switch (iCheck)
	{
	case 1:
		return tr("已审核");
	case 0:
		return tr("未审核");
	default:
		return tr("unknown");
	}
}

///
/// @brief 获取样本的显示数据
///
/// @param[in]  index  位置
/// @param[in]  role   角色
///
/// @return 显示数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月3日，新建函数
///
QVariant QHistorySampleAssayModel::DataSample(const QModelIndex &index, int role) const
{
	const int row = index.row();
	const int column = index.column();
	if (row >= m_vecHistoryBaseDataSamples.size())	// 超出数据显示范围，显示空白
	{
		return QVariant();
	}

	// 状态列编号是1，不允许改变
	int statusColumn = 1;

	// 获取数据
	auto& data = *m_vecHistoryBaseDataSamples[row];
	if (role == Qt::DisplayRole) 
	{
		switch (SampleColumn::Instance().IndexSampleToStatus(SampleColumn::SAMPLEMODE::SAMPLE, column))
		{
			// 状态改为代理绘制
			//case COL::STATUS:
			//	return GetStatus(data.m_sampleStatus, data.m_sendLis);
			case SampleColumn::COL::SEQNO:
				return GetSeqNo((::tf::HisSampleType::type)data.m_sampleType, (::tf::TestMode::type)data.m_testMode, data.m_seqNo);
			case SampleColumn::COL::BARCODE:
				return QString::fromStdString(data.m_barcode);
			case SampleColumn::COL::TYPE:
				return ThriftEnumTrans::GetSourceTypeName(data.m_sampleSourceType);
			case SampleColumn::COL::POS:
				return QString::fromStdString(data.m_pos);
			case SampleColumn::COL::ENDTIME:
				return QString::fromStdString(PosixTimeToTimeString(data.m_endTestTimeMax));
			case SampleColumn::COL::RECHECKENDTIME:
				if (data.m_retest)
				{
					return QString::fromStdString(PosixTimeToTimeString(data.m_reTestEndTimeMax));
				}
				return QString(tr("-"));
			case SampleColumn::COL::AUDIT:
			{
				return (data.m_bCheck ? QString(tr("已审核")) : QString(tr("未审核")));
			}
			break;
			case SampleColumn::COL::PRINT:
			{
				return (data.m_bPrint ? QString(tr("已打印")) : QString(tr("未打印")));
			}
			default:
				return QVariant();
		}
	}
	// 对齐方式
	else if (role == Qt::TextAlignmentRole) 
	{
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
	}
	// 额外数据
	else if (role == (Qt::UserRole + 1))
	{
		// 勾选列
		if (column == 0)
		{
			if (m_bCheckAll)
			{
				return true;
			}
            return m_setHistoryBaseDataSamplesCheck.find(row) != m_setHistoryBaseDataSamplesCheck.end();
		}
		// 状态列复查
		else if (column == statusColumn)
		{
			// modify by likai 20230815 历史数据不再保存样本状态，均默认为已完成 
			return WorkpageCommon::GetTestStatusPixmap(::tf::TestItemStatus::TEST_ITEM_STATUS_TESTED);
		}
	}
	else if (role == (Qt::UserRole + 2))
	{
		// 状态列样本状态
		if (column == statusColumn)
		{
			return WorkpageCommon::GetRecheckPixmap(data.m_reTestEndTimeMax, data.m_endTestTimeMax);
		}
	}
	else if (role == (Qt::UserRole + 3))
	{
		// 状态列样本状态
		if (column == statusColumn)
		{
			tf::SampleInfo sampleInfo;
			sampleInfo.__set_uploaded(data.m_sendLis);
			return WorkpageCommon::GetUploadPixmap(sampleInfo, QDataItemInfo::Instance().IsAiRecognition());
		}
	}
    else if (role == (Qt::UserRole + 4) && column == statusColumn)
    {
        return WorkpageCommon::GetSampleAiStatus();
    }

	return QVariant();
}

///
/// @brief 获取样本的显示数据(按项目显示)
///
/// @param[in]  index  位置
/// @param[in]  role   角色
///
/// @return 显示数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月3日，新建函数
///
QVariant QHistorySampleAssayModel::DataAssay(const QModelIndex &index, int role) const
{
	const int row = index.row();
	const int column = index.column();
	if (row >= m_vecHistoryBaseDataItems.size())	// 超出数据显示范围，显示空白
	{
		return QVariant();
	}

    HistoryBaseDataByItem& data = *m_vecHistoryBaseDataItems[row];
	// 状态列编号是1，不允许改变
	int statusColumn = 1;
	// 获取当前列的类型（是项目展示，还是结果等）
	SampleColumn::COLASSAY columnType = SampleColumn::Instance().IndexToStatus(column);
	// 获取数据
	if (role == Qt::DisplayRole) 
	{
		switch (columnType)
		{
		//case COLASSAY::STATUS:
		//	return GetStatus(data.m_sampleStatus, data.m_sendLis);
		case SampleColumn::COLASSAY::SEQNO:
        {
            int curTestType = GetHistorySample(data.m_sampleId);
            return GetSeqNo((::tf::HisSampleType::type)data.m_sampleType, (::tf::TestMode::type)curTestType, data.m_seqNo);
        }
		case SampleColumn::COLASSAY::BARCODE:
			return QString::fromStdString(data.m_barcode);
		case SampleColumn::COLASSAY::TYPE:
			return ThriftEnumTrans::GetSourceTypeName(data.m_sampleSourceType);
		case SampleColumn::COLASSAY::POS:
			return QString::fromStdString(data.m_pos);
		case SampleColumn::COLASSAY::ENDTIME:
			return QString::fromStdString(PosixTimeToTimeString(data.m_endTestTime));
		case SampleColumn::COLASSAY::RECHECKENDTIME:
			if (data.m_retest)
			{
				return QString::fromStdString(PosixTimeToTimeString(data.m_reTestEndTimeMax));
			}
			return QString(tr("-"));
		//case COLASSAY::AUDIT:
		//	return GetCheck(data.m_bCheck);
		case SampleColumn::COLASSAY::ASSAY:
			return GetHistoryAssayName(data);
		case SampleColumn::COLASSAY::RESULT:
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(data, false, firstResult);
            return firstResult.m_result;
        }
		case SampleColumn::COLASSAY::RESULTSTATUS:
        {
			return TransDisplayStatuCode(data, false);
		}
		case SampleColumn::COLASSAY::RECHECKRESULT:
			if (data.m_retest)
            {
                stAssayResult retestResult;
                QHistorySampleAssayModel::GetAssayResult(data, true, retestResult);
				return retestResult.m_result;
			}
			break;
		case SampleColumn::COLASSAY::RECHECKSTATUS:

			if (data.m_retest)
            {
				return TransDisplayStatuCode(data, true);
			}
			break;
		case SampleColumn::COLASSAY::UNIT:
        {
            return GetResultUnit(data);
		}
		case SampleColumn::COLASSAY::MODULE:
		{
			// 计算项目不显示模块号
			if (CommonInformationManager::IsCalcAssay(data.m_assayCode))
			{
				return QVariant();
			}

			int moduleIndex = 1;
			if (!stringutil::IsInteger(data.m_moduleNo, moduleIndex))
			{
				return QString::fromStdString(data.m_moduleNo);
			}

			return QString::fromStdString(CommonInformationManager::GetDeviceName(data.m_deviceSN, moduleIndex));
		}
			break;
		case SampleColumn::COLASSAY::AUDIT:
		{
			return (data.m_bCheck ? QString(tr("已审核")) : QString(tr("未审核")));
		}
		break;
		case SampleColumn::COLASSAY::FIRSTABSORB:
		{
			return QString::fromStdString(data.m_RLU);
		}
		break;
		case SampleColumn::COLASSAY::REABSORB:
		{
			return QString::fromStdString(data.m_reTestRLU);
		}
		break;
		case SampleColumn::COLASSAY::PRINT:
		{
			return (data.m_bPrint ? QString(tr("已打印")) : QString(tr("未打印")));
		}
		break;
		default:
			return QVariant();
		}
	}
	// 对齐方式
	else if (role == Qt::TextAlignmentRole) 
	{
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
	}

	// 额外数据
	else if (role == (Qt::UserRole + 1)) 
	{
		// 勾选列
		if (column == 0)
		{
			if (m_bCheckAll)
			{
				return true;
            }
            else
            {
                return m_setHistoryBaseDataItemsCheck.find(row) != m_setHistoryBaseDataItemsCheck.end();
			}
		}
		// 状态列复查
		else if (column == statusColumn)
		{
			// modify by likai 20230815 历史数据不再保存样本状态，均默认为已完成 
			return WorkpageCommon::GetTestStatusPixmap(::tf::TestItemStatus::TEST_ITEM_STATUS_TESTED);
		}
        auto dilutionFactor = data.m_dilutionFactor;
        auto diluteStatu = data.m_diluteStatu;
        auto suckVolType = data.m_suckVolType;
        if (data.m_retest)
        {
            dilutionFactor = data.m_reTestDilutionFactor;
            diluteStatu = data.m_reTestDiluteStatu;
            suckVolType = data.m_reSuckVolType;
        }
		// 稀释结果
		if (columnType == SampleColumn::COLASSAY::ASSAY && (diluteStatu == ::tf::HisDataDiluteStatu::type::DILUTE_STATU_MANUAL \
				|| diluteStatu == ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_AUTO))
		{
			auto assayClass = gCommMgr->GetAssayClassify(data.m_assayCode);
			// 自动稀释，包括常量增量减量(针对生化的逻辑)
			if ((diluteStatu == ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_AUTO)
				&& assayClass == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
			{              
				if (dilutionFactor <= 1)
				{
					QPixmap pixmap;
					if (suckVolType == tf::SuckVolType::type::SUCK_VOL_TYPE_DEC)
					{
						pixmap.load(QString(":/Leonis/resource/image/icon-dec.png"));
					}
					else if (suckVolType == tf::SuckVolType::type::SUCK_VOL_TYPE_INC)
					{
						pixmap.load(QString(":/Leonis/resource/image/icon-inc.png"));
					}
					else
					{
						return QVariant();
					}
					return QVariant(pixmap);
				}

				return dilutionFactor;
			}
			else
			{
				return dilutionFactor;
			}
		}
		else if (columnType == SampleColumn::COLASSAY::RESULT)	// 设置结果字体
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(data, false, firstResult);
			return firstResult.m_fontColorType;
		}
		else if (columnType == SampleColumn::COLASSAY::RECHECKRESULT && data.m_retest)	// 设置结果字体
        {
            stAssayResult retestResult;
            QHistorySampleAssayModel::GetAssayResult(data, true, retestResult);
			return retestResult.m_fontColorType;
		}
		else
		{
			return QVariant();
		}
	}
	else if (role == (Qt::UserRole + 2))
	{
		// 状态列样本状态
		if (column == statusColumn)
		{
			return WorkpageCommon::GetRecheckPixmap(data.m_reTestEndTimeMax, data.m_endTestTime);
		}
	}
	else if (role == (Qt::UserRole + 3))
	{
		// 状态列样本状态
		if (column == statusColumn)
		{
			tf::SampleInfo sampleInfo;
			sampleInfo.__set_uploaded(data.m_sendLis);
			return WorkpageCommon::GetUploadPixmap(sampleInfo, QDataItemInfo::Instance().IsAiRecognition());
		}
	}
    else if (role == (Qt::UserRole + 4) && column == statusColumn)
    {
        return WorkpageCommon::GetSampleAiStatus();
    }
	else if (role == (Qt::UserRole + 5))
	{
		if (columnType == SampleColumn::COLASSAY::RESULT)
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(data, false, firstResult);
			return firstResult.m_backgroundColor;
		}

		// 复查结果
		if (columnType == SampleColumn::COLASSAY::RECHECKRESULT)
        {
            stAssayResult retestResult;
            QHistorySampleAssayModel::GetAssayResult(data, true, retestResult);
			return retestResult.m_backgroundColor;
		}
	}

	return QVariant();
}

///
/// @brief 获取结果单位
///
///
/// @return 
///
/// @par History:
/// @li 6889/ChenWei，2024年5月29日，新建函数
///
QString QHistorySampleAssayModel::GetResultUnit(const HistoryBaseDataByItem& data)
{
    // 样本显示的设置
    SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
    // 显示非浓度的情况下，不需要显示单位
    if (!WorkpageCommon::IsShowConc(sampleSet, (::tf::HisSampleType::type)data.m_sampleType)
		//0014371 modify by wuht
		|| CommonInformationManager::GetInstance()->IsAssaySIND(data.m_assayCode))
    {
        return "";
    }
    int iPrecision = 2; // 小数点后保留几位小数(默认2位)	
    double factor = 1.0; // 获取当前单位与主单位的转化倍率
    std::string unitName = data.m_unit;
    WorkpageCommon::GetPrecFactorUnit(data.m_assayCode, iPrecision, factor, unitName);
    return QString::fromStdString(unitName);
}

///
/// @brief 通过样本ID审核样本
///
/// @param[in]  vecSampleID	样本ID
/// @param[in]  bCheck		审核状态	
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年5月31日，新建函数
///
void QHistorySampleAssayModel::CheckSampleBySampleID(const std::vector<int64_t> &vecSampleID, bool bCheck, const bool bUpdate)
{
    ULOG(LOG_INFO, "%s()sampSize:%d, isCheck:%d, isUpdate:%d.", __FUNCTION__, vecSampleID.size(), bCheck, bUpdate);

	if (vecSampleID.empty())
	{
        ULOG(LOG_WARN, "Empty sample id vec.");
		return;
	}

	std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (pLoginUserInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't get UserInfo.");
		return;
	}

	::tf::ResultLong _return;
	::tf::HistoryBaseDataCheckUpdate cs;
	cs.__set_sampleIds(vecSampleID);
	cs.__set_bCheck(bCheck);
	DcsControlProxy::GetInstance()->UpdateHistoryBaseDataCheckStatus(_return, cs);
	if (_return.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_WARN, "Faild to query history basedata by sampleid and check flag.");
		return;
	}

	for (int sampleIndex = 0; sampleIndex < vecSampleID.size(); ++sampleIndex)
	{
		int64_t sampleID = vecSampleID[sampleIndex];
		auto sampleIt = m_mapSampleIndex.find(sampleID);
		if (sampleIt == m_mapSampleIndex.end())
		{
			ULOG(LOG_WARN, "Not find sampleID:%lld", sampleID);
			continue;
		}
		int64_t index = sampleIt->second;
		if (m_vecHistoryBaseDataSamples.size() <= index)
		{
            ULOG(LOG_WARN, "Sample index error:%lld, over range, historybasedata samples vecsize:%d.", index, m_vecHistoryBaseDataSamples.size());
			continue;
		}
		m_vecHistoryBaseDataSamples[index]->m_bCheck = bCheck;
		auto changedIndex = this->index(index, static_cast<int>(COL::STATUS));
		emit dataChanged(changedIndex, changedIndex);

		// 更新按项目展示审核状态
        auto range = m_mapSampleAssayIndex.equal_range(sampleID);
        for (auto it = range.first; it != range.second; ++it)
        {
            auto index = it->second;
            if (m_vecHistoryBaseDataItems.size() <= index)
            {
                ULOG(LOG_WARN, "Sample assay item index error:%lld, over range, historybase dataitem vecsize:%d.", index, m_vecHistoryBaseDataItems.size());
                continue;
            }
            m_vecHistoryBaseDataItems[index]->m_bCheck = bCheck;
        }

		// 更新审核者信息	
		auto patitentId = m_vecHistoryBaseDataSamples[index]->m_patientId;
		if (!WorkpageCommon::UpdateAduitInfo(patitentId, sampleID, pLoginUserInfo->username))
		{
			ULOG(LOG_ERROR, "audit name, patient id: %id, sample id: %id failed!", pLoginUserInfo->username, patitentId, sampleID);
			continue;
		}
	}

    // 启动审核后自动上次Lis
    CommParamSet comm;
    DictionaryQueryManager::GetCommParamSet(comm);
    if (comm.bUploadResults 
		&& comm.bUploadAfterAudit 
		&& bCheck
		&& bUpdate)
    {
        DcsControlProxy::GetInstance()->PushSampleToLIS(vecSampleID, tf::LisTransferType::TRANSFER_AUTO, true);
    }
}

QHistorySampleAssayModel::~QHistorySampleAssayModel()
{
	if (m_spThread != nullptr && m_spThread->joinable())
	{
		m_bThreadStopFlag = true;
		m_spThread->join();
	}
}

///
/// @brief 获取单例对象
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
QHistorySampleAssayModel& QHistorySampleAssayModel::Instance()
{
	static QHistorySampleAssayModel model;
	return model;
}

///
/// @brief 更新历史数据
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2022年12月29日，新建函数
///
void QHistorySampleAssayModel::Update(const ::tf::HistoryBaseDataQueryCond& cond)
{
	beginResetModel();

	m_vecHistoryBaseDataSamples.clear();
	m_setHistoryBaseDataSamplesCheck.clear();
	m_vecHistoryBaseDataItems.clear();
	m_setHistoryBaseDataItemsCheck.clear();
	m_cond = cond;
	m_bCheckAll = false;

	QHistorySampleModel_Assay::Instance().Clear();

	std::string sql;
	DcsControlHandler dcs;
	if (CondToSql_Sample(cond, sql))
	{
		dcs.QueryHistoryBaseDataBySample_Sample(m_vecHistoryBaseDataSamples, sql);
	}
	if (CondToSqlByItem(cond, sql))
	{
        dcs.QuertHistoryBaseDataByItem(m_vecHistoryBaseDataItems, sql);
        DelateNoDisplayCalcAssays(m_vecHistoryBaseDataItems);
	}

	endResetModel();
	emit sampleChanged();

	// 开一个临时线程去组织映射表
	if (m_spThread != nullptr && m_spThread->joinable())
	{
		m_bThreadStopFlag = true;
		m_spThread->join();
	}
	m_spThread = std::shared_ptr<std::thread>(new std::thread([&]() {
		std::lock_guard<std::mutex> lock(m_mutex);

		m_bThreadStopFlag = false;
		m_mapSampleIndex.clear();
		m_mapSampleAssayIndex.clear();
		for (int i = 0; i < m_vecHistoryBaseDataSamples.size(); ++i)
		{
			if (m_bThreadStopFlag)
			{
				return;
			}
			m_mapSampleIndex.insert(std::make_pair(m_vecHistoryBaseDataSamples[i]->m_sampleId, i));
		}
		for (int i = 0; i < m_vecHistoryBaseDataItems.size(); ++i)
		{
			if (m_bThreadStopFlag)
			{
				return;
			}
			m_mapSampleAssayIndex.insert(std::make_pair(m_vecHistoryBaseDataItems[i]->m_sampleId, i));
		}
	}));
}

void QHistorySampleAssayModel::GetHistoryDataBySampleId(int64_t sampleId, const ::tf::HistoryBaseDataQueryCond& cond, std::vector<std::shared_ptr<HistoryBaseDataByItem>>& dataItem)
{
    std::string sqlOrderBy;
    BuildOrderByForHistoryBaseData(sqlOrderBy, cond);

    //二、拼装其他查询条件
    std::string sqlAnd;
    BuildSQLForHistoryBaseData(sqlAnd, cond);

    // 查询
    std::string sql;
    sql += "select seqNo,barcode,sampleSourceType,pos,endTestTime,reTestEndTime,";
    sql += "itemName,suckVolType,diluteStatu,dilutionFactor,testResult,resultStatu,reTestResult,reTestResultStatu,unit,bCheck,moduleNo,itemId,deviceSN,sampleId,patientId,sampleType,assayCode,sendList,userUseRetestResultID,";
    sql += "print,qualitativeResult,refRangeJudge,crisisRangeJudge,reTestQualitativeResult,reTestRefRangeJudge,reTestCrisisRangeJudge,RLU,reTestRLU,retest,reTestDilutionFactor,reSuckVolType,reTestDiluteStatu ";
    sql += " from t_HistoryBaseData where sampleId = " + std::to_string(sampleId);
    sql += sqlAnd;
    sql += sqlOrderBy;
    sql += ";";

    DcsControlHandler dcs;
    dcs.QuertHistoryBaseDataByItem(dataItem, sql);
    DelateNoDisplayCalcAssays(dataItem);
}

QString QHistorySampleAssayModel::GetHistoryAssayNameByBaseItem(const std::shared_ptr<HistoryBaseDataByItem>& dataItem)
{
    return dataItem == nullptr ? "" : GetHistoryAssayName(*dataItem);
}

void QHistorySampleAssayModel::UpdatePrintFlag(const std::vector<int64_t>& sampleId, bool isPrinted)
{
    // 把vector转换成set，便于后续查询
    std::set<int64_t> sampIdSet;
    for (int64_t id : sampleId)
    {
        sampIdSet.insert(id);
    }

    beginResetModel();
    for (std::shared_ptr<HistoryBaseDataSample>& sIt : m_vecHistoryBaseDataSamples)
    {
        if (sampIdSet.find(sIt->m_sampleId) != sampIdSet.end())
        {
            sIt->m_bPrint = isPrinted;
        }
    }

    for (std::shared_ptr<HistoryBaseDataByItem>& sIt : m_vecHistoryBaseDataItems)
    {
        if (sampIdSet.find(sIt->m_sampleId) != sampIdSet.end())
        {
            sIt->m_bPrint = isPrinted;
        }
    }
    endResetModel();

    emit sampleChanged();
}

///
/// @brief 获取当前查询条件
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月17日，新建函数
///
const ::tf::HistoryBaseDataQueryCond& QHistorySampleAssayModel::GetQueryCond()
{
	return m_cond;
}

///
/// @brief 刷新
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月4日，新建函数
///
void QHistorySampleAssayModel::Refush()
{
	m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_NULL);
	Update(m_cond);
}

///
/// @brief 排序
///
/// @param[in]  col		排序列
/// @param[in]  order	排序方式，0-无，1-降序，2-升序
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月4日，新建函数
///
void QHistorySampleAssayModel::Sort(int col, int order)
{
	if (order == 0)
	{
		m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_NULL);
	}
	else
	{
		switch (m_moudleType)
		{
		case QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE:
		{
			switch (col)
			{
				case static_cast<int>(SampleColumn::COL::BARCODE) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_BARCORDE_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_BARCORDE_ASC);
				break;
			case static_cast<int>(SampleColumn::COL::SEQNO) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SEQNO_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SEQNO_ASC);
				break;
			case static_cast<int>(SampleColumn::COL::TYPE) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SAMPLESOURCETYPE_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SAMPLESOURCETYPE_ASC);
				break;
			case static_cast<int>(SampleColumn::COL::POS) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_POS_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_POS_ASC);
				break;
			case static_cast<int>(SampleColumn::COL::ENDTIME) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_ENDTESTTIME_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_ENDTESTTIME_ASC);
				break;
			case static_cast<int>(SampleColumn::COL::RECHECKENDTIME) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_RETESTENDTIMEMAX_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_RETESTENDTIMEMAX_ASC);
				break;
			default:
				break;
			}
		}
			break;
		case QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
			switch (col)
			{
			case static_cast<int>(SampleColumn::COLASSAY::BARCODE) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_BARCORDE_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_BARCORDE_ASC);
				break;
			case static_cast<int>(SampleColumn::COLASSAY::SEQNO) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SEQNO_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SEQNO_ASC);
				break;
			case static_cast<int>(SampleColumn::COLASSAY::TYPE) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SAMPLESOURCETYPE_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_SAMPLESOURCETYPE_ASC);
				break;
			case static_cast<int>(SampleColumn::COLASSAY::POS) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_POS_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_POS_ASC);
				break;
			case static_cast<int>(SampleColumn::COLASSAY::ENDTIME) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_ENDTESTTIME_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_ENDTESTTIME_ASC);
				break;
			case static_cast<int>(SampleColumn::COLASSAY::RECHECKENDTIME) :
				order == 1 ? m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_RETESTENDTIMEMAX_DESC) : \
				m_cond.__set_orderBy(::tf::HistoryDataOrderBy::ORDER_BY_RETESTENDTIMEMAX_ASC);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	Update(m_cond);
}

///
/// @brief 返回是否为空
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月28日，新建函数
///
bool QHistorySampleAssayModel::Empty() const
{
	return (m_vecHistoryBaseDataItems.empty() && m_vecHistoryBaseDataSamples.empty());
}

///
/// @brief 更新历史数据Lis状态
///
/// @param[in]  vecSampleID		发生改变的样本ID
/// @param[in]  vecStatus		发生改变的样本当前状态///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年5月24日，新建函数
///
void QHistorySampleAssayModel::UpdateLisStatus(const QVector<int64_t>& vecSampleID)
{
	if (vecSampleID.empty())
	{
		return;
	}

	std::lock_guard<std::mutex> lock(m_mutex);

	// 上传Lis只能在按样本展示时有效
	if (m_moudleType != VIEWMOUDLE::SAMPLEBROWSE)
	{
		return;
	}
	for (int sampleIndex = 0; sampleIndex < vecSampleID.size(); ++sampleIndex)
	{
		auto sampleID = vecSampleID[sampleIndex];
		auto it = m_mapSampleIndex.find(sampleID);
		if (it == m_mapSampleIndex.end())
		{
			continue;
		}
		auto index = it->second;
		if (m_vecHistoryBaseDataSamples.size() <= index)
		{
			continue;
		}
		m_vecHistoryBaseDataSamples[index]->m_sendLis = true;
		auto changedIndex = this->index(index, static_cast<int>(COL::STATUS));
		emit dataChanged(changedIndex, changedIndex);

		// 更新按项目展示Lis状态
		{
			auto it = m_mapSampleAssayIndex.find(sampleID);
			if (it == m_mapSampleAssayIndex.end())
			{
				continue;
			}
			auto index = it->second;
			if (m_vecHistoryBaseDataItems.size() <= index)
			{
				continue;
			}
			m_vecHistoryBaseDataItems[index]->m_sendLis = true;
		}
	}
}

///
/// @brief 更新复查结果
///
/// @param[in]  index	索引
/// @param[in]  vol		复查结果，0-复查结果，1-复查状态
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月4日，新建函数
///
void QHistorySampleAssayModel::UpdateRetestResult(const QModelIndex& index, std::tuple<std::string, std::string, int64_t> vol)
{
	if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		auto row = index.row();
		if (m_vecHistoryBaseDataItems.size() <= row)
		{
			return;
		}
		m_vecHistoryBaseDataItems[row]->m_reTestResult = std::get<0>(vol);
		m_vecHistoryBaseDataItems[row]->m_reTestResultStatu = std::get<1>(vol);
		m_vecHistoryBaseDataItems[row]->m_userUseRetestResultID = std::get<2>(vol);
		// 把一整行都更新
		emit dataChanged(this->index(row,0), this->index(row, static_cast<int>(COLASSAY::MODULE)));
	}
}

///
/// @brief 清除历史数据
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年3月16日，新建函数
///
void QHistorySampleAssayModel::Clear()
{
	beginResetModel();
	m_vecHistoryBaseDataSamples.clear();
	m_vecHistoryBaseDataItems.clear();

	endResetModel();

    // 重置默认查询条件BUG：http://192.168.39.67:86/view.php?id=12083
    m_cond = ::tf::HistoryBaseDataQueryCond();

	emit sampleChanged();
}

///
/// @brief 获取模式
///
///
/// @return 获取当前模式
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月26日，新建函数
///
QHistorySampleAssayModel::VIEWMOUDLE QHistorySampleAssayModel::GetModule() const
{
	return m_moudleType;
}

///
/// @brief 设置模式类型（项目选择，项目浏览-（按样本展示，按项目展示））
///
/// @param[in]  module  模式类型
///
/// @return 设置成功返回true
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月21日，新建函数
///
bool QHistorySampleAssayModel::SetSampleModuleType(VIEWMOUDLE module)
{
	beginResetModel();
	m_moudleType = module;
	endResetModel();
	return true;
}

///
/// @brief
///     获取单元格属性(可编辑、可选择)
///
/// @param[in]    index      当前单元格索引
///
/// @return       QVariant   包装的数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
Qt::ItemFlags QHistorySampleAssayModel::flags(const QModelIndex &index) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int QHistorySampleAssayModel::rowCount(const QModelIndex &parent) const
{
    // 统一为没有空行
    return ((m_moudleType == VIEWMOUDLE::SAMPLEBROWSE) ? m_vecHistoryBaseDataSamples.size() :
        (m_moudleType == VIEWMOUDLE::ASSAYBROWSE ? m_vecHistoryBaseDataItems.size() : 0));
}

int QHistorySampleAssayModel::columnCount(const QModelIndex &parent) const
{
	switch (m_moudleType)
	{
		case QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE:
			return m_sampleBrowseHeader.size();
		case QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
			return m_assayBrowseHeader.size();
		default:
			//return m_assaySelectHeader.size();
			return 0;
	}
}

///
/// @brief 获取当前选中样本ID
///
/// @param[in]  index  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2022年12月30日，新建函数
///
int64_t QHistorySampleAssayModel::GetSampleID(const QModelIndex &index)
{
	if (!index.isValid())
	{
		return -1;
	}
	if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
	{
		if (index.row() < m_vecHistoryBaseDataSamples.size())
		{
			return m_vecHistoryBaseDataSamples[index.row()]->m_sampleId;
		}
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		if (index.row() < m_vecHistoryBaseDataItems.size())
		{
			return m_vecHistoryBaseDataItems[index.row()]->m_sampleId;
		}
	}

	return  -1;
}

bool QHistorySampleAssayModel::IsRowChecked(int iRow) const
{
    if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
    {
        return m_setHistoryBaseDataSamplesCheck.find(iRow) != m_setHistoryBaseDataSamplesCheck.end();
    }
    else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
    {
        return m_setHistoryBaseDataItemsCheck.find(iRow) != m_setHistoryBaseDataItemsCheck.end();
    }

    return false;
}

int64_t QHistorySampleAssayModel::GetSelectedSampleID() const
{
    if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
    {
        if (m_setHistoryBaseDataSamplesCheck.size() > 0)
        {
            int curIdx = *(m_setHistoryBaseDataSamplesCheck.begin());
            if (curIdx < m_vecHistoryBaseDataSamples.size() && curIdx >= 0)
            {
                return m_vecHistoryBaseDataSamples[curIdx]->m_sampleId;
            }
        }
    }
    else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
    {
        if (m_setHistoryBaseDataItemsCheck.size() > 0)
        {
            int curIdx = *(m_setHistoryBaseDataItemsCheck.begin());
            if (curIdx < m_vecHistoryBaseDataItems.size() && curIdx >= 0)
            {
                return m_vecHistoryBaseDataItems[curIdx]->m_sampleId;
            }
        }
    }

    return -1;
}

int64_t QHistorySampleAssayModel::GetAiRecognizeResultId(const QModelIndex &index)
{
	if (!index.isValid())
	{
		return -1;
	}
	if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
	{
		if (index.row() < m_vecHistoryBaseDataSamples.size())
		{
			return m_vecHistoryBaseDataSamples[index.row()]->m_aiRecognizeResultId;
		}
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		if (index.row() < m_vecHistoryBaseDataItems.size())
		{
			// 先通过Item查找到sampleid,再通过sampleId查找ai信息
			auto iter = m_mapSampleIndex.find(m_vecHistoryBaseDataItems[index.row()]->m_sampleId);
			if (iter != m_mapSampleIndex.end() 
				&& iter->second < m_vecHistoryBaseDataSamples.size()
				&& iter->second >= 0)
			{
				return m_vecHistoryBaseDataSamples[iter->second]->m_aiRecognizeResultId;;
			}
		}
	}

	return  -1;
}

int QHistorySampleAssayModel::GetSampleType(const QModelIndex &index)
{
	if (!index.isValid())
	{
		return -1;
	}
	if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
	{
		if (index.row() < m_vecHistoryBaseDataSamples.size())
		{
			return m_vecHistoryBaseDataSamples[index.row()]->m_sampleType;
		}
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		if (index.row() < m_vecHistoryBaseDataItems.size())
		{
			return m_vecHistoryBaseDataItems[index.row()]->m_sampleType;
		}
	}

	return  -1;
}

///
/// @brief 获取按项目展示模式下的选中数据（供结果详情使用）
///
/// @param[in]  index  当前单元格索引
///
/// @return 组装数据
///
/// @par History:
/// @li 8090/YeHuaNing，2023年2月8日，新建函数
///
std::shared_ptr<HistoryBaseDataItem> QHistorySampleAssayModel::GetAssayItemData(const QModelIndex & index) const
{
	std::shared_ptr<HistoryBaseDataItem> itemData = std::make_shared<HistoryBaseDataItem>();
    if (index.isValid() && GetModule() == VIEWMOUDLE::ASSAYBROWSE)
    {
		if (index.row() < m_vecHistoryBaseDataItems.size())
		{
			auto& line = *m_vecHistoryBaseDataItems[index.row()];
			itemData->m_itemName = line.m_itemName;
			itemData->m_diluteStatu = line.m_diluteStatu;
			itemData->m_dilutionFactor = line.m_dilutionFactor;
			itemData->m_testResult = line.m_testResult;
			itemData->m_resultStatu = line.m_resultStatu;
			itemData->m_reTestResult = line.m_reTestResult;
			itemData->m_reTestResultStatu = line.m_reTestResultStatu;
			itemData->m_unit = line.m_unit;
			itemData->m_moduleNo = line.m_moduleNo;
			itemData->m_itemId = line.m_itemId;
			itemData->m_deviceSN = line.m_deviceSN;
			itemData->m_sampleId = line.m_sampleId;
			itemData->m_patientId = line.m_patientId;
			itemData->m_seqNo = line.m_seqNo;
			itemData->m_barcode = line.m_barcode;
			itemData->m_assayCode = line.m_assayCode;
			itemData->m_sendLis = line.m_sendLis;
			itemData->m_sampleSourceType = line.m_sampleSourceType;
			itemData->m_sampleType = line.m_sampleType;
			itemData->m_pos = line.m_pos;
			itemData->m_userUseRetestResultID = line.m_userUseRetestResultID;
			itemData->m_bCheck = line.m_bCheck;
			itemData->m_bPrint = line.m_bPrint;
			itemData->m_refRangeJudge = line.m_refRangeJudge;
			itemData->m_crisisRangeJudge = line.m_crisisRangeJudge;
			itemData->m_qualitativeResult = line.m_qualitativeResult;	
			itemData->m_reTestRefRangeJudge = line.m_reTestRefRangeJudge;
			itemData->m_reTestCrisisRangeJudge = line.m_reTestCrisisRangeJudge;
			itemData->m_reTestQualitativeResult = line.m_reTestQualitativeResult;
			itemData->m_RLU = line.m_RLU;
			itemData->m_reTestRLU = line.m_reTestRLU;
			itemData->m_retest = line.m_retest;
            itemData->m_reTestDilutionFactor = line.m_reTestDilutionFactor;
            itemData->m_reSuckVolType = line.m_reSuckVolType;
            itemData->m_reTestDiluteStatu = line.m_reTestDiluteStatu;
		}
    }

    return itemData;
}

bool QHistorySampleAssayModel::IsItemCalcAssay(const QModelIndex& idx) const
{
    if (idx.isValid() && GetModule() == VIEWMOUDLE::ASSAYBROWSE)
    {
        if (idx.row() < m_vecHistoryBaseDataItems.size())
        {
            const std::shared_ptr<HistoryBaseDataByItem> &rowItem = m_vecHistoryBaseDataItems[idx.row()];
            if (rowItem != nullptr)
            {
                return CommonInformationManager::IsCalcAssay(rowItem->m_assayCode);
            }
        }
    }
    
    return false;
}

///
/// @brief   根据结果和设置显示结果范围
///
/// @param[in]		hisItem			历史数据
/// @param[in]		resultProm		设置参数
/// @param[out]		firstResult		首查结果
/// @param[out]		retestResult	复查结果
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年12月22日，新建函数
///
void QHistorySampleAssayModel::ResultRangeFlag(int isRe, int cris, int ref, int reCris, int reRef, int sampType, \
	stAssayResult& firstResult, stAssayResult& retestResult)
{
	// 校准、质控无参考范围标记、危机值范围标记
	if (sampType == (int)::tf::HisSampleType::SAMPLE_SOURCE_CL || \
        sampType == (int)::tf::HisSampleType::SAMPLE_SOURCE_QC)
	{
		return;
    }
    // 结果提示的设置
    ResultPrompt resultProm = DictionaryQueryManager::GetInstance()->GetResultTips();
	QStringList refFlags;
	if (resultProm.enabledOutRange)
	{
		if (resultProm.outRangeMarkType == 1)
		{
			refFlags << "↑" << "↓";
		}
		else if (resultProm.outRangeMarkType == 2)
		{
			refFlags << "H" << "L";
		}
	}

	bool isRecheck = (isRe == 1) ? true : false;

	QString lowerdangeFlag, lowerdangeColor;
	QString upperdangeFlag, upperdangeColor;
	if (resultProm.enabledCritical)
	{
		lowerdangeColor = "#E5F2FF";
		upperdangeColor = "#FFDAD6";

		// modify bug0011817 by wuht
		if (resultProm.outRangeMarkType == 1)
		{
			lowerdangeFlag = "↓!";
			upperdangeFlag = "↑!";
		}
		else if (resultProm.outRangeMarkType == 2)
		{
			lowerdangeFlag = "L!";
			upperdangeFlag = "H!";
		}
	}

	// 初测--初测参考范围标志
	if (!isRecheck && !refFlags.empty())
	{
		if (ref == (int)::tf::RangeJudge::ABOVE_UPPER)
		{
			firstResult.m_resultFlag = refFlags[0];
			firstResult.m_fontColorType = ColorType::UPPERCOLOR;
		}
		else if (ref == (int)::tf::RangeJudge::LESS_LOWER)
		{
			firstResult.m_resultFlag = refFlags[1];
			firstResult.m_fontColorType = ColorType::LOWERCOLOR;
		}
	}
	// 复查--复查参考范围标志
	else if (isRecheck && !refFlags.empty())
	{
		if (reRef == (int)::tf::RangeJudge::ABOVE_UPPER)
		{
			retestResult.m_resultFlag = refFlags[0];
			retestResult.m_fontColorType = ColorType::UPPERCOLOR;
		}
		else if (reRef == (int)::tf::RangeJudge::LESS_LOWER)
		{
			retestResult.m_resultFlag = refFlags[1];
			retestResult.m_fontColorType = ColorType::LOWERCOLOR;
		}
	}
	if (resultProm.enabledCritical)
	{
		if (!isRecheck)
		{
			// 初测--初测危急范围标志
			if (cris == (int)::tf::RangeJudge::ABOVE_UPPER)
			{
				firstResult.m_resultFlag = upperdangeFlag;
				firstResult.m_backgroundColor = upperdangeColor;
				firstResult.m_fontColorType = ColorType::UPPERCOLOR;
			}
			// 初测--初测危急范围标志
			else if (cris == (int)::tf::RangeJudge::LESS_LOWER)
			{
				firstResult.m_resultFlag = lowerdangeFlag;
				firstResult.m_backgroundColor = lowerdangeColor;
				firstResult.m_fontColorType = ColorType::LOWERCOLOR;
			}
		}
		else
		{
			// 复查--复查危急范围标志
			if (reCris == (int)::tf::RangeJudge::ABOVE_UPPER)
			{
				retestResult.m_resultFlag = upperdangeFlag;
				retestResult.m_backgroundColor = upperdangeColor;
				retestResult.m_fontColorType = ColorType::UPPERCOLOR;
			}
			// 复查--复查危急范围标志
			else if (reCris == (int)::tf::RangeJudge::LESS_LOWER)
			{
				retestResult.m_resultFlag = lowerdangeFlag;
				retestResult.m_backgroundColor = lowerdangeColor;
				retestResult.m_fontColorType = ColorType::LOWERCOLOR;
			}
		}
	}
}

void QHistorySampleAssayModel::ResultRangeFlag(int assayCode, int sampType, int qualitative, int cris, int ref, stAssayResult& result)
{
	// 校准、质控无参考范围标记、危机值范围标记
	if (sampType == (int)::tf::HisSampleType::SAMPLE_SOURCE_CL || \
		sampType == (int)::tf::HisSampleType::SAMPLE_SOURCE_QC)
	{
		return;
	}

	// 结果提示的设置
	ResultPrompt resultProm = DictionaryQueryManager::GetInstance()->GetResultTips();
	QStringList refFlags;
	if (resultProm.enabledOutRange)
	{
		if (resultProm.outRangeMarkType == 1)
		{
			refFlags << "↑" << "↓";
		}
		else if (resultProm.outRangeMarkType == 2)
		{
			refFlags << "H" << "L";
		}
	}

	QString lowerdangeFlag, lowerdangeColor;
	QString upperdangeFlag, upperdangeColor;
	if (resultProm.enabledCritical)
	{
		lowerdangeColor = "#E5F2FF";
		upperdangeColor = "#FFDAD6";

		// modify bug0011817 by wuht
		if (resultProm.outRangeMarkType == 1)
		{
			lowerdangeFlag = "↓!";
			upperdangeFlag = "↑!";
		}
		else if (resultProm.outRangeMarkType == 2)
		{
			lowerdangeFlag = "L!";
			upperdangeFlag = "H!";
		}
	}

	// 参考范围标志
	if (!refFlags.empty())
	{
		if (ref == (int)::tf::RangeJudge::ABOVE_UPPER)
		{
			result.m_resultFlag = refFlags[0];
			result.m_fontColorType = ColorType::UPPERCOLOR;
		}
		else if (ref == (int)::tf::RangeJudge::LESS_LOWER)
		{
			result.m_resultFlag = refFlags[1];
			result.m_fontColorType = ColorType::LOWERCOLOR;
		}
	}

	if (resultProm.enabledCritical)
	{
		// 初测--初测危急范围标志
		if (cris == (int)::tf::RangeJudge::ABOVE_UPPER)
		{
			result.m_resultFlag = upperdangeFlag;
			result.m_backgroundColor = upperdangeColor;
			result.m_fontColorType = ColorType::UPPERCOLOR;
		}
		// 初测--初测危急范围标志
		else if (cris == (int)::tf::RangeJudge::LESS_LOWER)
		{
			result.m_resultFlag = lowerdangeFlag;
			result.m_backgroundColor = lowerdangeColor;
			result.m_fontColorType = ColorType::LOWERCOLOR;
		}
	}

	QString status;
	bool isSind = gCommMgr->IsAssaySIND(assayCode);
	if (isSind && tf::QualJudge::type::FuzzyRegion < qualitative)
	{
		int start = qualitative - tf::QualJudge::CH_SIND_LEVEL1;
		status = gCommMgr->GetResultStatusLHIbyCode(assayCode, start);
	}

	result.m_resultFlag = status + result.m_resultFlag;
}

///
/// @brief 获取项目结果
///
/// @param[in]		data	历史数据
/// @param[in]		bRetest	是否是复查列
/// @param[out]		result  测试结果
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年12月22日，新建函数
///
void QHistorySampleAssayModel::GetAssayResult(const HistoryBaseDataItem& data, bool bRetest, stAssayResult& result)
{
	// 小数点后保留几位小数(默认2位)
	int left = 2;
	// 单位倍率
	double factor = 1.0;
	// 项目类型
	tf::AssayClassify::type classi = tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

	bool isCalc = CommonInformationManager::IsCalcAssay(data.m_assayCode);
	if (isCalc)
	{
		auto spCalcInfo = gCommMgr->GetCalcAssayInfo(data.m_assayCode);
		if (spCalcInfo != nullptr)
		{
			left = spCalcInfo->decimalDigit;
		}
	}
	else
	{
		auto spAssayInfo = gCommMgr->GetAssayInfo(data.m_assayCode);
		if (spAssayInfo != nullptr)
		{
			left = spAssayInfo->decimalPlace;
			classi = spAssayInfo->assayClassify;
			for (const tf::AssayUnit& unit : spAssayInfo->units)
			{
				if (unit.isCurrent)
				{
					factor = unit.factor;
                    break;
				}
			}
		}
	}

	SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	if (!WorkpageCommon::IsShowConc(sampleSet, (tf::HisSampleType::type)data.m_sampleType))
    {
        std::string tmpRlu = bRetest ? data.m_reTestRLU : data.m_RLU;

        // 免疫没有DEFAULT_CH_MAX_ABS，暂时不使用生化设定的值规则
        if (classi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            result.m_result = QString::fromStdString(tmpRlu);
        }
        else
        {
            // 吸光度是否符合显示要求
            bool isIse = gCommMgr->IsAssayISE(data.m_assayCode);
            if (!isIse && !WorkpageCommon::IsRLUValid(tmpRlu))
            {
                return;
            }

			int leftvalue = 0;
			// 获取按照小数位数格式化的字符串
			if (isIse)
			{
				leftvalue = left;
			}
			result.m_result = QDataItemInfo::GetValueFromLeft(QString::fromStdString(tmpRlu), "/", leftvalue);
        }
		return;
	}

	// 当结果中含有无法计算的项目时候，不显示
    if (!isCalc)
    {
        if (bRetest)
        {
            // 复查浓度不符合显示标准
            if (!WorkpageCommon::IsConcValid(isCalc, data.m_reTestResultStatu, data.m_reTestResult))
            {
                return;
            }
        }
        else
        {
            // 初测结果不符合显示标准
            if (!WorkpageCommon::IsConcValid(isCalc, data.m_resultStatu, data.m_testResult))
            {
                return;
            }
        }
    }

	if (bRetest)
	{
		QHistorySampleAssayModel::ResultRangeFlag(data.m_assayCode, data.m_sampleType, data.m_reTestQualitativeResult, data.m_reTestCrisisRangeJudge, data.m_reTestRefRangeJudge, result);
	}
	else
	{
		QHistorySampleAssayModel::ResultRangeFlag(data.m_assayCode, data.m_sampleType, data.m_qualitativeResult, data.m_crisisRangeJudge, data.m_refRangeJudge, result);
	}

	double conc = -1;
	int diluFactor = data.m_dilutionFactor;

	// 复查
	if (bRetest)
	{
		if (data.m_retest)
		{
			conc = std::stod(data.m_reTestResult);
            diluFactor = data.m_reTestDilutionFactor;
		}
		else
		{
			// 当要求显示空白的时候，单元格还原
			result.m_fontColorType = ColorType::NORMALCOLOR;
			result.m_backgroundColor = "#ffffff";	// 默认背景白色
			result.m_result = QObject::tr(""); //所有未计算都要求显示空白
			return;
		}
	}
	// 初测
	else
	{
		conc = data.m_testResult;
	}

	// 免疫
	if (classi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
	{
		if (conc < 0)
		{
			result.m_result = QObject::tr(""); //所有未计算都要求显示空白
			return ;
		}

		result.m_result = QString::number(conc*factor, 'f', left) + result.m_resultFlag;
		// 校准品不显示线性范围
		if (data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_NM && 
            data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_EM &&
            data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_QC)
		{
			return;
		}

		// 查询通用项目信息
		std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> imGAIs;
		gCommMgr->GetImmuneAssayInfos(imGAIs, data.m_assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000));
		if (imGAIs.size() <= 0)
		{
			ULOG(LOG_ERROR, "Failed to get immune assayinfos!");
			return;
		}

		// 线性范围(<*或>*)加参考范围标志bug24912      
		if (!imGAIs[0]->extenL && conc < imGAIs[0]->techRangeMin*diluFactor)
		{
			result.m_result = "<" + QString::number(imGAIs[0]->techRangeMin*factor*diluFactor, 'f', left) + result.m_resultFlag;
			return;
		}
		else if (!imGAIs[0]->extenH && conc > imGAIs[0]->techRangeMax*diluFactor)
		{
			result.m_result = ">" + QString::number(imGAIs[0]->techRangeMax*factor*diluFactor, 'f', left) + result.m_resultFlag;
			return;
		}
	}
	else
	{
		result.m_result = QString::number(conc*factor, 'f', left) + result.m_resultFlag;
	}

	// 若是计算项目，则只有在（DBL_MAX）的时候才不显示
	if (isCalc)
	{
		if (DBL_MAX == conc)
		{
			// 当要求显示空白的时候，单元格还原
			result.m_fontColorType = ColorType::NORMALCOLOR;
			result.m_backgroundColor = "#ffffff";	// 默认背景白色
			result.m_result = "";
		}
	}
	else
	{
		// 普通项目，当是负值的时候，不显示
		if (conc < 0)
		{
			// 当要求显示空白的时候，单元格还原
			result.m_fontColorType = ColorType::NORMALCOLOR;
			result.m_backgroundColor = "#ffffff";	// 默认背景白色
			result.m_result = "";
		}
	}

	return;
}


///
/// @brief 获取项目结果
///
/// @param[in]		data	历史数据
/// @param[in]		bRetest	是否是复查列
/// @param[out]		result  测试结果
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年12月22日，新建函数
///
void QHistorySampleAssayModel::GetAssayResult(const HistoryBaseDataByItem& data, bool bRetest, stAssayResult& result)
{
	// 小数点后保留几位小数(默认2位)
	int left = 2;
	// 单位倍率
	double factor = 1.0;
	// 项目类型
	tf::AssayClassify::type classi = tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

	bool isCalc = CommonInformationManager::IsCalcAssay(data.m_assayCode);
	if (isCalc)
	{
		auto spCalcInfo = gCommMgr->GetCalcAssayInfo(data.m_assayCode);
		if (spCalcInfo != Q_NULLPTR)
		{
			left = spCalcInfo->decimalDigit;
		}
	}
	else
	{
		auto spAssayInfo = gCommMgr->GetAssayInfo(data.m_assayCode);
		if (spAssayInfo != Q_NULLPTR)
		{
			left = spAssayInfo->decimalPlace;
			classi = spAssayInfo->assayClassify;
			for (const tf::AssayUnit& unit : spAssayInfo->units)
			{
				if (unit.isCurrent)
				{
					factor = unit.factor;
                    break;
				}
			}
		}
	}

	SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	if (!WorkpageCommon::IsShowConc(sampleSet, (tf::HisSampleType::type)data.m_sampleType))
	{
        std::string tmpRLU = bRetest ? data.m_reTestRLU : data.m_RLU;
        if (classi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            result.m_result = QString::fromStdString(tmpRLU);
        }
        else
        {			
            // ISE显示的电动势，不需要做吸光度合法性的判断
            bool isIse = gCommMgr->IsAssayISE(data.m_assayCode);
            if (!isIse && !WorkpageCommon::IsRLUValid(tmpRLU))
            {
                return;
            }

			int leftvalue = 0;
			// 获取按照小数位数格式化的字符串
			if (isIse)
			{
				leftvalue = left;
			}
			result.m_result = QDataItemInfo::GetValueFromLeft(QString::fromStdString(tmpRLU), "/", leftvalue);
        }
		return;
	}

	// 当结果中含有无法计算的项目时候，不显示
    if (!isCalc)
    {
        if (bRetest)
        {
            // 复查浓度不符合显示标准
            if (!WorkpageCommon::IsConcValid(isCalc, data.m_reTestResultStatu, data.m_reTestResult))
            {
                return;
            }
        }
        else
        {
            // 初测结果不符合显示标准
            if (!WorkpageCommon::IsConcValid(isCalc, data.m_resultStatu, data.m_testResult))
            {
                return;
            }
        }
    }

	// 结果提示的设置
	stAssayResult temp;
	if (bRetest)
	{
        QHistorySampleAssayModel::ResultRangeFlag(bRetest, data.m_crisisRangeJudge,
            data.m_refRangeJudge, data.m_reTestCrisisRangeJudge, data.m_reTestRefRangeJudge, data.m_sampleType, temp, result);
	}
	else
	{
        QHistorySampleAssayModel::ResultRangeFlag(bRetest, data.m_crisisRangeJudge,
            data.m_refRangeJudge, data.m_reTestCrisisRangeJudge, data.m_reTestRefRangeJudge, data.m_sampleType, result, temp);
	}

	// modify bug0014180 by wuht
	QString status;
	bool isSind = gCommMgr->IsAssaySIND(data.m_assayCode);
	if (isSind && tf::QualJudge::type::FuzzyRegion < data.m_qualitativeResult)
	{
		int start = data.m_qualitativeResult - tf::QualJudge::CH_SIND_LEVEL1;
		status = gCommMgr->GetResultStatusLHIbyCode(data.m_assayCode, start);
	}

	QString flag = "", refFlag = "";
	refFlag = status + result.m_resultFlag;
	double conc = -1;
	int diluFactor = data.m_dilutionFactor;

	// 复查
	if (bRetest)
	{
		if (data.m_retest)
		{
			conc = std::stod(data.m_reTestResult);
            diluFactor = data.m_reTestDilutionFactor;
		}
		else
		{
			// 当要求显示空白的时候，单元格还原
			result.m_fontColorType = ColorType::NORMALCOLOR;
			result.m_backgroundColor = "#ffffff";	// 默认背景白色
			result.m_result = QObject::tr(""); //所有未计算都要求显示空白
			return;
		}
	}
	// 初测
	else
	{
		conc = data.m_testResult;
	}

	// 免疫
	if (classi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
	{
		if (conc < 0)
		{
			//result = tr("未计算");
			result.m_result = QObject::tr(""); //所有未计算都要求显示空白
			return;
		}

		result.m_result = flag + QString::number(conc*factor, 'f', left) + refFlag;
		// 校准品不显示线性范围
		if (data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_NM && 
            data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_EM &&
            data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_QC)
		{
			return;
		}

		// 查询通用项目信息
		std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> imGAIs;
		gCommMgr->GetImmuneAssayInfos(imGAIs, data.m_assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000));
		if (imGAIs.size() <= 0)
		{
			ULOG(LOG_ERROR, "Failed to get immune assayinfos!");
			return;
		}

		// 线性范围(<*或>*)也需要显示参考范围标志bug24912     
		if (!imGAIs[0]->extenL && conc < imGAIs[0]->techRangeMin*diluFactor)
		{
			result.m_result = "<" + QString::number(imGAIs[0]->techRangeMin*factor*diluFactor, 'f', left) + refFlag;
			return;
		}
		else if (!imGAIs[0]->extenH && conc > imGAIs[0]->techRangeMax*diluFactor)
		{
			result.m_result = ">" + QString::number(imGAIs[0]->techRangeMax*factor*diluFactor, 'f', left) + refFlag;
			return;
		}
	}
	else
	{
		result.m_result = flag + QString::number(conc*factor, 'f', left) + refFlag;
	}

	// 若是计算项目，则只有在（DBL_MAX）的时候才不显示
	if (isCalc)
	{
		if (DBL_MAX == conc)
		{
			// 当要求显示空白的时候，单元格还原
			result.m_fontColorType = ColorType::NORMALCOLOR;
			result.m_backgroundColor = "#ffffff";	// 默认背景白色
			result.m_result = "";
		}
	}
	else
	{
		// 普通项目，当是负值的时候，不显示
		if (conc < 0)
		{
			// 当要求显示空白的时候，单元格还原
			result.m_fontColorType = ColorType::NORMALCOLOR;
			result.m_backgroundColor = "#ffffff";	// 默认背景白色
			result.m_result = "";
		}
	}

	return;
}

QString QHistorySampleAssayModel::GetDisplayStatuCode(const HistoryBaseDataByItem& data, bool bRetest)
{
    return TransDisplayStatuCode(data, bRetest);
}

///
/// @brief 为给定的项目结果设置标识
///
/// @param[in]		dValue	结果
/// @param[in]		isOriginal	是否是原始结果
/// @param[in]		data	历史数据
/// @param[in]		bRetest	是否是复查列
/// @param[out]		result  测试结果
///
/// @return 
///
/// @par History:
/// @li 6889/ChenWei，2024年6月22日，新建函数
///
void QHistorySampleAssayModel::SetAssayResultFlag(double dValue, bool isOriginal, const HistoryBaseDataByItem& data, bool bRetest, stAssayResult& result)
{
    // 小数点后保留几位小数(默认2位)
    int left = 2;
    // 单位倍率
    double factor = 1.0;
    // 项目类型
    tf::AssayClassify::type classi = tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

    bool isCalc = CommonInformationManager::IsCalcAssay(data.m_assayCode);
    if (isCalc)
    {
        auto spCalcInfo = gCommMgr->GetCalcAssayInfo(data.m_assayCode);
        if (spCalcInfo != Q_NULLPTR)
        {
            left = spCalcInfo->decimalDigit;
        }
    }
    else
    {
        auto spAssayInfo = gCommMgr->GetAssayInfo(data.m_assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            left = spAssayInfo->decimalPlace;
            classi = spAssayInfo->assayClassify;
            for (const tf::AssayUnit& unit : spAssayInfo->units)
            {
                if (unit.isCurrent)
                {
                    factor = unit.factor;
                }
            }
        }
    }

    // 结果提示的设置
    stAssayResult temp;
    if (bRetest)
    {
        QHistorySampleAssayModel::ResultRangeFlag(1, data.m_crisisRangeJudge,
            data.m_refRangeJudge, data.m_reTestCrisisRangeJudge, data.m_reTestRefRangeJudge, data.m_sampleType, temp, result);
    }
    else
    {
        QHistorySampleAssayModel::ResultRangeFlag(0, data.m_crisisRangeJudge,
            data.m_refRangeJudge, data.m_reTestCrisisRangeJudge, data.m_reTestRefRangeJudge, data.m_sampleType, result, temp);
    }

    QString flag = "", refFlag = "";
    refFlag = result.m_resultFlag;
    double conc = dValue;
    int diluFactor = data.m_dilutionFactor;
    if (data.m_retest)
    {
        diluFactor = data.m_reTestDilutionFactor;
    }

    // 免疫
    if (classi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        if (conc < 0)
        {
            //result = tr("未计算");
            result.m_result = QObject::tr(""); //所有未计算都要求显示空白
            return;
        }

        if (isOriginal)
        {
            result.m_result = QString::number(conc*factor, 'f', left);
            return;
        }

        result.m_result = flag + QString::number(conc*factor, 'f', left) + refFlag;

        // 校准品不显示线性范围
        if (data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_NM &&
            data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_EM &&
            data.m_sampleType != tf::HisSampleType::type::SAMPLE_SOURCE_QC)
        {
            return;
        }

        // 查询通用项目信息
        std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> imGAIs;
        gCommMgr->GetImmuneAssayInfos(imGAIs, data.m_assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000));
        if (imGAIs.size() <= 0)
        {
            ULOG(LOG_ERROR, "Failed to get immune assayinfos!");
            return;
        }

        // 线性范围(<*或>*)也需要显示参考范围标志bug24912     
        if (!imGAIs[0]->extenL && conc < imGAIs[0]->techRangeMin*diluFactor)
        {
            result.m_result = "<" + QString::number(imGAIs[0]->techRangeMin*factor*diluFactor, 'f', left) + refFlag;
            return;
        }
        else if (!imGAIs[0]->extenH && conc > imGAIs[0]->techRangeMax*diluFactor)
        {
            result.m_result = ">" + QString::number(imGAIs[0]->techRangeMax*factor*diluFactor, 'f', left) + refFlag;
            return;
        }
    }
    else
    {
        result.m_result = flag + QString::number(conc*factor, 'f', left) + refFlag;
    }

    // 若是计算项目，则只有在（DBL_MAX）的时候才不显示
    if (isCalc)
    {
        if (DBL_MAX == conc)
        {
            result.m_result = "";
        }
    }
    else
    {
        // 普通项目，当是负值的时候，不显示
        if (conc < 0)
        {
            result.m_result = "";
        }
    }

    return;
}

QVariant QHistorySampleAssayModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (m_moudleType == VIEWMOUDLE::SAMPLEBROWSE)
	{
		return DataSample(index, role);
	}
	else if (m_moudleType == VIEWMOUDLE::ASSAYBROWSE)
	{
		return DataAssay(index, role);
	}

	return QVariant();
}

QVariant QHistorySampleAssayModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal) 
	{
		if (role == Qt::DisplayRole) 
		{
			switch (m_moudleType)
			{
				case QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE:
					return m_sampleBrowseHeader.at(section);
					break;
				case QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
					return m_assayBrowseHeader.at(section);
					break;
				default:
					//return m_assaySelectHeader.at(section);
					return QVariant();
					break;
			}
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

QHistorySampleModel_Assay::~QHistorySampleModel_Assay()
{

}

///
/// @brief 获取单例对象
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
QHistorySampleModel_Assay& QHistorySampleModel_Assay::Instance()
{
	static QHistorySampleModel_Assay model;
	return model;
}

///
/// @brief 更新历史数据
///
/// @param[in]  sampleID	样本ID
/// @param[in]  cond		查询条件
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2022年12月29日，新建函数
///
void QHistorySampleModel_Assay::Update(const int64_t& sampleID, const ::tf::HistoryBaseDataQueryCond& cond)
{
	m_iSampleID = sampleID;
	m_cond = cond;
	beginResetModel();
	m_vecHistoryBaseDataItems.clear();

	std::string sql;
	if (QHistorySampleAssayModel::CondToSql_Item(cond, sql, sampleID))
	{
		DcsControlHandler dcs;
		dcs.QuertHistoryBaseDataBySample_Item(m_vecHistoryBaseDataItems, sql);
        DelateNoDisplayCalcAssays(m_vecHistoryBaseDataItems);
	}
	
	// 查询结束后根据项目显示顺序排序
	RefushBySort(m_vecHistoryBaseDataItems);

	endResetModel();
}

void QHistorySampleModel_Assay::GetItems(int64_t sampleId, const ::tf::HistoryBaseDataQueryCond& cond, std::vector<std::shared_ptr<HistoryBaseDataItem>>& vecItems)
{
    vecItems.clear();
    std::string sql;
    if (QHistorySampleAssayModel::CondToSql_Item(cond, sql, sampleId))
    {
        DcsControlHandler dcs;
        dcs.QuertHistoryBaseDataBySample_Item(vecItems, sql);
        DelateNoDisplayCalcAssays(vecItems);
    }

    RefushBySort(vecItems);
}

///
/// @brief 刷新
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月17日，新建函数
///
void QHistorySampleModel_Assay::Refush()
{
	Update(m_iSampleID, m_cond);
}

///
/// @brief 排序规则变化时刷新数据
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月12日，新建函数
///
void QHistorySampleModel_Assay::RefushBySort(std::vector<std::shared_ptr<HistoryBaseDataItem>>& vecItems)
{
	auto cmp = [&](std::shared_ptr<HistoryBaseDataItem> a, std::shared_ptr<HistoryBaseDataItem> b) {
		// 特殊情况，都不在序列中的情况(按照通道号来排序，小的排在前面， Bug0011854 by wuht)
		if (m_showOrder.count(a->m_assayCode) <= 0 && m_showOrder.count(b->m_assayCode) <= 0)
		{
			return (a->m_assayCode < b->m_assayCode) ? true : false;
		}

		if (m_showOrder.count(a->m_assayCode) <= 0)
		{
			return false;
		}

		if (m_showOrder.count(b->m_assayCode) <= 0)
		{
			return true;
		}

		if (m_showOrder[a->m_assayCode] < m_showOrder[b->m_assayCode])
		{
			return true;
		}

		// 编号相同的时候，看id号，id号越小则优先级越高
		if (m_showOrder[a->m_assayCode] == m_showOrder[b->m_assayCode])
		{
			// 当计算项目的位置号跟非计算项目的位置号相同的时候，计算项目的优先级更高
			// 当计算项目位置号跟计算项目位置号相同的情况，未做规定(相同的时候，按照数据库主键来排, bug0012241 by wuht)
			bool isaCalc = CommonInformationManager::IsCalcAssay(a->m_assayCode);
			bool isbCalc = CommonInformationManager::IsCalcAssay(b->m_assayCode);
			if (!(isaCalc && isbCalc))
			{
				return isaCalc ? true : false;
			}

			return (a->m_itemId < b->m_itemId) ? true : false;
		}

		return false;
	};

	std::stable_sort(vecItems.begin(), vecItems.end(), cmp);
}

///
/// @brief 清除历史数据
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年3月16日，新建函数
///
void QHistorySampleModel_Assay::Clear()
{
	m_iSampleID = -1;
	beginResetModel();
	m_vecHistoryBaseDataItems.clear();

	endResetModel();

    // 恢复默认筛选 BUG:http://192.168.39.67:86/view.php?id=12083
    m_cond = ::tf::HistoryBaseDataQueryCond();
}

///
/// @brief
///     获取单元格属性(可编辑、可选择)
///
/// @param[in]    index      当前单元格索引
///
/// @return       QVariant   包装的数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
Qt::ItemFlags QHistorySampleModel_Assay::flags(const QModelIndex &index) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int QHistorySampleModel_Assay::rowCount(const QModelIndex &parent) const
{
	// 统一为没有空行
	auto size = m_vecHistoryBaseDataItems.size();
	// if (size < PAGE_LIMIT_ITEM)
	// {
	// 	return PAGE_LIMIT_ITEM;
	// }

	return m_vecHistoryBaseDataItems.size();
}

int QHistorySampleModel_Assay::columnCount(const QModelIndex &parent) const
{
	return m_assayBrowseHeader.size();
}

///
/// @brief 更新索引顺序
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年10月8日，新建函数
///
bool QHistorySampleModel_Assay::UpdateIndex()
{
    OrderAssay data;
    if (!DictionaryQueryManager::GetAssayShowOrder(data))
    {
        ULOG(LOG_ERROR, "Failed to get showorder.");
        return false;
    }

	m_showOrder.clear();
	// 使用自定义的排序
	if (data.enableCustom)
	{
		int postion = 0;
		for (auto order : data.orderVec)
		{
			m_showOrder.insert(std::make_pair(order, postion));
			postion++;
		}
	}
	else
	{
		m_showOrder = gCommMgr->GetAssayDefaultMap();
	}

	// 排序规则变化之后刷新UI
	beginResetModel();
	RefushBySort(m_vecHistoryBaseDataItems);
	endResetModel();

	return true;
}

QVariant QHistorySampleModel_Assay::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	const int row = index.row();
	const int column = index.column();
	if (row >= m_vecHistoryBaseDataItems.size())	// 超出数据显示范围，显示空白
	{
		return QVariant();
	}

	// 获取当前列的类型（是项目展示，还是结果等）
	SampleColumn::SAMPLEASSAY columnType = SampleColumn::Instance().IndexAssayToStatus(column + 1);
	// 获取数据
	auto& data = *m_vecHistoryBaseDataItems[row];
	if (role == Qt::DisplayRole) 
	{
		switch (columnType)
		{
		case SampleColumn::SAMPLEASSAY::ASSAY:
			return GetHistoryAssayName(data);
		case SampleColumn::SAMPLEASSAY::RESULT:
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(data, false, firstResult);
            return firstResult.m_result;
        }
		case SampleColumn::SAMPLEASSAY::RESULTSTATUS:
        {
			return TransDisplayStatuCode(data, false);
		}
		case SampleColumn::SAMPLEASSAY::RECHECKRESULT:
			if (data.m_retest)
            {
                stAssayResult retestResult;
                QHistorySampleAssayModel::GetAssayResult(data, true, retestResult);
				return retestResult.m_result;
			}
			break;
		case SampleColumn::SAMPLEASSAY::RECHECKSTATUS:
			if (data.m_retest)
            {
				return TransDisplayStatuCode(data, true);
			}
			break;
		case SampleColumn::SAMPLEASSAY::UNIT:
        {
            // 样本显示的设置
            SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
			// 显示非浓度的情况下，不需要显示单位
			if (!WorkpageCommon::IsShowConc(sampleSet, (::tf::HisSampleType::type)data.m_sampleType)
				//0014371 modify by wuht
				|| CommonInformationManager::GetInstance()->IsAssaySIND(data.m_assayCode))
			{
				return "";
            }
            int iPrecision = 2; // 小数点后保留几位小数(默认2位)	
            double factor = 1.0; // 获取当前单位与主单位的转化倍率	
            std::string unitName = data.m_unit; // 单位
            WorkpageCommon::GetPrecFactorUnit(data.m_assayCode, iPrecision, factor, unitName);
			return QString::fromStdString(unitName);
		}
		case SampleColumn::SAMPLEASSAY::FIRSTABSORB:
		{
			return QString::fromStdString(data.m_RLU);
		}
		break;
		case SampleColumn::SAMPLEASSAY::REABSORB:
		{
			return QString::fromStdString(data.m_reTestRLU);
		}
		break;
		case SampleColumn::SAMPLEASSAY::MODULE:
		{
			// 计算项目不显示模块号
			if (CommonInformationManager::IsCalcAssay(data.m_assayCode))
			{
				return QVariant();
			}

			int moduleIndex = 1;
			if (!stringutil::IsInteger(data.m_moduleNo, moduleIndex))
			{
				return QString::fromStdString(data.m_moduleNo);
			}

			return QString::fromStdString(CommonInformationManager::GetDeviceName(data.m_deviceSN, moduleIndex));

			auto spDevice = gCommMgr->GetDeviceInfo(data.m_deviceSN);
			if (spDevice == nullptr)
			{
				return QString::fromStdString(data.m_moduleNo);
			}
			else
			{
				return QString::fromStdString(spDevice->groupName + spDevice->name);
			}
		}
			break;
		default:
			return QVariant();
		}
	}
	// 对齐方式
	else if (role == Qt::TextAlignmentRole) 
	{
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
	}
	// 额外数据
	else if (role == (Qt::UserRole + 1)) 
	{
        auto dilutionFactor = data.m_dilutionFactor;
        auto diluteStatu = data.m_diluteStatu;
        auto suckVolType = data.m_suckVolType;
        if (data.m_retest)
        {
            dilutionFactor = data.m_reTestDilutionFactor;
            diluteStatu = data.m_reTestDiluteStatu;
            suckVolType = data.m_reSuckVolType;
        }
		// 稀释结果
		if (columnType == SampleColumn::SAMPLEASSAY::ASSAY && (diluteStatu == ::tf::HisDataDiluteStatu::type::DILUTE_STATU_MANUAL \
			|| diluteStatu == ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_AUTO))
		{
			auto assayClass = gCommMgr->GetAssayClassify(data.m_assayCode);
			// 自动稀释，包括常量增量减量(针对生化的逻辑)
			if ((diluteStatu == ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_AUTO)
				&& assayClass == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
			{
				// 是否计算项目(计算项目不显示稀释倍数)
				bool isCalc = CommonInformationManager::IsCalcAssay(data.m_assayCode);
				if (isCalc)
				{
					return QVariant();
				}
				if (dilutionFactor <= 1)
				{
					QPixmap pixmap;
					if (suckVolType == tf::SuckVolType::type::SUCK_VOL_TYPE_DEC)
					{
						pixmap.load(QString(":/Leonis/resource/image/icon-dec.png"));
					}
					else if (suckVolType == tf::SuckVolType::type::SUCK_VOL_TYPE_INC)
					{
						pixmap.load(QString(":/Leonis/resource/image/icon-inc.png"));
					}
					else
					{
						return QVariant();
					}
					return QVariant(pixmap);
				}

				return dilutionFactor;
			}
			else
			{
				// ISE的特殊处理
				if ((assayClass == tf::AssayClassify::type::ASSAY_CLASSIFY_ISE)
					&& dilutionFactor <= 1)
				{
					return QVariant();
				}

				// 设置稀释倍数
				return dilutionFactor;
			}
		}
		else if (columnType == SampleColumn::SAMPLEASSAY::RESULT)	// 设置结果字体
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(data, false, firstResult);
			return firstResult.m_fontColorType;
		}
		else if (columnType == SampleColumn::SAMPLEASSAY::RECHECKRESULT && data.m_retest)	// 设置结果字体
        {
            stAssayResult retestResult;
            QHistorySampleAssayModel::GetAssayResult(data, true, retestResult);
			return retestResult.m_fontColorType;
		}
		else
		{
			return QVariant();
		}
	}
	// modify for bug3312
	else if (role == (Qt::UserRole + 5))
	{
		// 初测结果
		if (columnType == SampleColumn::SAMPLEASSAY::RESULT)
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(data, false, firstResult);
			return firstResult.m_backgroundColor;
		}

		// 复查结果
		if (columnType == SampleColumn::SAMPLEASSAY::RECHECKRESULT)
        {
            stAssayResult retestResult;
            QHistorySampleAssayModel::GetAssayResult(data, true, retestResult);
			return retestResult.m_backgroundColor;
		}
	}

	return QVariant();
}

QVariant QHistorySampleModel_Assay::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			return m_assayBrowseHeader.at(section);
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

QHistorySampleModel_Assay::QHistorySampleModel_Assay()
{
	m_assayBrowseHeader << tr("项目名称") << tr("检测结果") << tr("数据报警") << tr("复查结果")
		<< tr("复查数据报警") << tr("单位") << tr("模块");

	m_iSampleID = -1;
	// 数据更新
	connect(&SampleColumn::Instance(), &SampleColumn::ColumnChanged, this, [&]()
	{
		// 更新信息
		m_assayBrowseHeader = SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLEASSAY, true);
		this->beginResetModel();
		this->endResetModel();
	});

	m_assayBrowseHeader = SampleColumn::Instance().GetHeaders(SampleColumn::SAMPLEMODE::SAMPLEASSAY, true);
	UpdateIndex();
	// 注册显示顺序更新信号处理函数
	REGISTER_HANDLER(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE, this, UpdateIndex);
}
