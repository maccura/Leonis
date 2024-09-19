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

#include "WorkpageCommon.h"
#include <QPixmap>
#include <QString>
#include <QPushButton>
#include <QGridLayout>
#include "QSampleAssayModel.h"
#include "QHistorySampleAssayModel.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/tipdlg.h"
#include "shared/FileExporter.h"
#include "shared/uicommon.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "src/common/StringUtil.h"
#include "src/public/ch/ChCommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"

#define MAXWIDTH 500


// 默认状态
const QString pngStatusDefault(":/Leonis/resource/image/status-default.png");
// 复查状态
const QString pngStatusRecheck(":/Leonis/resource/image/status-recheck.png");
// 上传状态
const QString pngStatusUpload(":/Leonis/resource/image/status-uploaded.png");
// 待测
const QString pngStatusPending(":/Leonis/resource/image/status-pending.png");
// 测试中
const QString pngStatusTesting(":/Leonis/resource/image/status-testing.png");
// 已测试
const QString pngStatusTested(":/Leonis/resource/image/status-tested.png");
// 打印
const QString pngStatusVerifPrint(":/Leonis/resource/image/status-default-verify-print.png");
const QString pngPrint(":/Leonis/resource/image/status-print.png");
// 审核
const QString pngStatusVersify(":/Leonis/resource/image/status-verify.png");

// 上传AI
const QString pngUploadAi(":/Leonis/resource/image/status-uploaded_ai.png");

// 1970-1-1 0:0:0为未设值的时间
const boost::posix_time::ptime origTime(boost::gregorian::date(1970, 1, 1), boost::posix_time::time_duration(0, 0, 0));

template <typename T>
boost::optional<std::pair<int, int>> GetAttributePair(const QMap<int, T>& srcData, int iType)
{
	for (const auto& item : srcData)
	{
		if (iType == int(item.type))
		{
			return std::make_pair(item.index, item.width);
		}
	}

	return boost::none;
}

bool ConstructExportSampleInfo::BuildExportDataInfo(SampleExportInfo& exportData)
{
	// 获取样本信息
	auto spSample = DataPrivate::Instance().GetSampleInfoByDb(m_key);
	if (!spSample)
	{
		return false;
	}

	auto isGet = BuildExportSampleInfo(spSample, exportData);
	if (!isGet)
	{
		return false;
	}

	return true;
}

bool ConstructExportSampleInfo::BuildExportSampleInfo(const std::shared_ptr<tf::SampleInfo>& spSample, SampleExportInfo& exportData)
{
	return true;
}

bool ConstructExportItemInfo::BuildExportDataInfo(SampleExportInfo& exportData)
{
	auto dataItem = DataPrivate::Instance().GetTestItemInfoByDb(m_key);
	if (nullptr == dataItem)
	{
		return false;
	}

	bool isExport = BuildExportItemInfo(dataItem, exportData);
	if (!isExport)
	{
		return false;
	}

	return true;
}

SampleExportInfo ConstructExportItemInfo::m_exportSampleBase;
bool ConstructExportItemInfo::BuildExportItemInfo(const std::shared_ptr<tf::TestItem>& spTestIetm, SampleExportInfo& exportData)
{
	// 当样本相同的时候，不需要再次去生成样本导出信息
	if (m_sampleKey == spTestIetm->sampleInfoId)
	{
		exportData = m_exportSampleBase;
	}
	else
	{
		// 生成样本导出信息
		ConstructExportSampleInfo sampleExport;
		sampleExport.SetDataKey(spTestIetm->sampleInfoId);
		auto isExport = sampleExport.BuildExportDataInfo(exportData);
		if (!isExport)
		{
			return false;
		}

		m_exportSampleBase = exportData;
	}

	// 生成项目导出信息
	// 待完善.....
	// 生成初测结果导出信息
	if (spTestIetm->firstTestResultKey.assayTestResultId > 0)
	{
		SampleExportInfo exportDataItem = exportData;
		ConstructExportResultInfo resultExport;
		resultExport.SetDataKey(spTestIetm->firstTestResultKey.assayTestResultId);
		resultExport.BuildExportDataInfo(exportDataItem);
		// 添加进入
		m_exportDatas.push_back(exportDataItem);
		exportData = exportDataItem;
	}

	// 生成复查结果
	if (spTestIetm->lastTestResultKey.assayTestResultId > 0)
	{
		SampleExportInfo exportDataItem = exportData;
		ConstructExportResultInfo resultExport;
		resultExport.SetDataKey(spTestIetm->lastTestResultKey.assayTestResultId);
		resultExport.BuildExportDataInfo(exportDataItem);
		// 添加进入
		m_exportDatas.push_back(exportDataItem);
		exportData = exportDataItem;
	}

	return true;
}


bool ConstructExportResultInfo::BuildExportDataInfo(SampleExportInfo& exportData)
{
	return true;
}

std::map<int64_t, std::map<int, int>> ConstructHistoryExportItemInfo::m_caliSampleIdMap;
void ConstructHistoryExportItemInfo::SetData(const std::shared_ptr<HistoryBaseDataByItem>& spHisTestIetm)
{
	m_spHisTestIetm = spHisTestIetm;
}

bool ConstructHistoryExportItemInfo::BuildExportDataInfo(SampleExportInfo& exportData)
{
	return BuildExportItemInfo(m_spHisTestIetm, exportData);
}

// 导出历史数据
bool ConstructHistoryExportItemInfo::BuildExportItemInfo(const std::shared_ptr<HistoryBaseDataByItem>& spHisTestIetm, SampleExportInfo& exportData)
{
	if (nullptr == spHisTestIetm)
	{
		return false;
	}

	if (!BuildExportSampleInfo(spHisTestIetm, exportData))
	{
		return false;
	}

	// 试剂批号
	//exportData.strReagentBatchNo = QString::fromStdString(spHisTestIetm->m_reagentLot);
	// 模块
	auto testModule = QString::fromStdString(CommonInformationManager::GetDeviceName((spHisTestIetm->m_deviceSN)));
	exportData.strTestModule = testModule;
	int assayCode = spHisTestIetm->m_assayCode;

	// 计算项目的处理
	if (CommonInformationManager::GetInstance()->IsCalcAssay(assayCode))
	{
		auto spCalcAssayInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
		if (nullptr == spCalcAssayInfo)
		{
			return false;
		}

		// 项目名称
		exportData.strItemName = QString::fromStdString(spCalcAssayInfo->name);
		// 重复次数
		exportData.strFirstTestCounts = " ";
		auto resultVec = WorkpageCommon::GetCalcAssayTestResult(spHisTestIetm->m_itemId);
		for (const auto result : resultVec)
		{
			if (!WorkpageCommon::GetCalcResultExport(result, exportData))
			{
				continue;
			}

			m_exportDatas.push_back(exportData);
		}	
	}
	else
	{
		auto spGeneralAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
		if (spGeneralAssayInfo == nullptr)
		{
			return false;
		}

		// 校准次数
		int caliTimes = WorkpageCommon::GetcaliTimes(m_caliSampleIdMap, spHisTestIetm->m_sampleId, assayCode);
		exportData.strCaliCount = QString::number(caliTimes);

		// 项目名称
		exportData.strItemName = QString::fromStdString(spGeneralAssayInfo->assayName);
		// 重复次数
		exportData.strFirstTestCounts = " ";
		if (CommonInformationManager::GetInstance()->IsAssayISE(assayCode))
		{
			// 导出ISE结果信息
			auto resultVec = WorkpageCommon::GetIseAssayTestResult(spHisTestIetm->m_itemId);
			for (const auto result : resultVec)
			{
				if (!WorkpageCommon::GetIseResultExport(result, exportData))
				{
					continue;
				}

				m_exportDatas.push_back(exportData);
			}

		}
		else
		{
			// 导出CH结果信息
			auto resultVec = WorkpageCommon::GetChAssayTestResult(spHisTestIetm->m_itemId);
			for (const auto result : resultVec)
			{
				if (!WorkpageCommon::GetChResultExport(result, exportData))
				{
					continue;
				}

				if (!WorkpageCommon::GetResultAbs(result, exportData, m_titleExtend))
				{
					continue;
				}

				m_exportDatas.push_back(exportData);

			}
		}
	}

	return true;
}

bool ConstructHistoryExportItemInfo::BuildExportSampleInfo(const std::shared_ptr<HistoryBaseDataByItem>& spHisTestIetm, SampleExportInfo& exportData)
{
	if (spHisTestIetm == nullptr)
	{
		return false;
	}

	// 获取样本信息
	std::shared_ptr<HistoryBaseDataSample> spSample = nullptr;
	QHistorySampleAssayModel::Instance().GetSamplesInSelectedItems(spHisTestIetm, spSample);
	if (spSample != nullptr)
	{
		// 样本管类型
		exportData.strTubeType = ThriftEnumTrans::GetTubeType(tf::TubeType::type(spSample->m_tubeType), " ");
		// 样本申请模式（序号模式，样本架模式，条码模式）
		exportData.strSampleSendModle = ThriftEnumTrans::GetTestMode(tf::TestMode::type(spSample->m_testMode));
	}

	// 样本类型
	exportData.strSampleSourceType = ThriftEnumTrans::GetSourceTypeName(spHisTestIetm->m_sampleSourceType, " ");
	// 样本条码号
	exportData.strBarcode = QString::fromStdString(spHisTestIetm->m_barcode);
	// 样本类别
	exportData.strSampleTypeStat = WorkpageCommon::GetSampleTypeStat(spHisTestIetm->m_sampleType);
	// 样本号
	exportData.strSampleNo = QString::fromStdString(spHisTestIetm->m_seqNo);
	// 架号
	QString rackPos = QString::fromStdString(spHisTestIetm->m_pos);
	auto pos = rackPos.split("-");
	if (pos.empty())
	{
		return true;
	}
	exportData.strRack = pos[0];

	if (pos.size() < 2)
	{
		return true;
	}
	// 位置号
	exportData.strPos = pos[1];

	return true;
}

bool ConstructExportInfo::ProcessOnebyOne(const int64_t testKey)
{
	 std::shared_ptr<tf::TestItem> spTestItemInfo = DataPrivate::Instance().GetTestItemInfoByDb(testKey);
	if (nullptr == spTestItemInfo)
	{
		return false;
	}

	ConstructExportItemInfo testItemDataExport;
	testItemDataExport.SetDataKey(spTestItemInfo->id);
	SampleExportInfo sampleExportData;
	if (!testItemDataExport.BuildExportDataInfo(sampleExportData))
	{
		return false;
	}

	auto exportType = GetExportType(testKey);
	auto& exportData = testItemDataExport.GetExportData();
	if (exportData.empty())
	{
		return false;
	}

	if (m_sampleExportDataVec.count(exportType))
	{
		if (exportData.size() >= 2)
		{
			m_sampleExportDataVec[exportType].emplace_back(exportData[0]);
			m_sampleExportDataVec[exportType].emplace_back(exportData[1]);
		}
		else
		{
			m_sampleExportDataVec[exportType].emplace_back(exportData[0]);
		}
	}
	else
	{
		m_sampleExportDataVec[exportType] = exportData;
	}

	return true;
}

const ExportType ConstructExportInfo::GetExportType(const int64_t testKey)
{
	ExportType returnType = ExportType::CH_NORMAL_AND_QC_TYPE;
	std::shared_ptr<tf::TestItem> spTestItemInfo = DataPrivate::Instance().GetTestItemInfoByDb(testKey);
	if (nullptr == spTestItemInfo)
	{
		return returnType;
	}

	if (nullptr == spTestItemInfo)
	{
		return returnType;
	}

	auto spSample = DataPrivate::Instance().GetSampleInfoByDb(spTestItemInfo->sampleInfoId);
	if (!spSample)
	{
		return returnType;
	}

	// 是否校准样本
	bool isCali = (spSample->sampleType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR);
	// 是否ISE项目
	bool isIse = CommonInformationManager::GetInstance()->IsAssayISE(spTestItemInfo->assayCode);
	if (isIse)
	{
		// ise项目的常规、急诊、质控的导出
		if (!isCali)
		{
			returnType = ExportType::CH_ISE_SAM_AND_QC_TYPE;
		}
		else
		{
			returnType = ExportType::CH_ISE_CALI_TYPE;
		}
	}
	else
	{
		// 普通项目的常规、急诊、质控的导出
		if (!isCali)
		{
			returnType = ExportType::CH_NORMAL_AND_QC_TYPE;
		}
		else
		{
			returnType = ExportType::CH_CALI_TYPE;
		}
	}

	return returnType;
}

void ConstructHisExportInfo::ProcessData(std::vector<std::shared_ptr<HistoryBaseDataByItem>>& dataItemVec)
{
	if (dataItemVec.empty())
	{
		return;
	}

	ConstructHistoryExportItemInfo::ClearCaliMap();
	for (const auto& dataItem : dataItemVec)
	{
        bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(dataItem->m_assayCode);
		// 是否含有生化项目，对于联机版生化页面导出，当含有生化项目的时候，才导出对应的计算项目
		if (isCalc)
		{
			bool isHaveCh = CommonInformationManager::GetInstance()->IsCaClHaveChorIM(dataItem->m_assayCode);
			if (!isHaveCh)
			{
				continue;
			}
		}

		auto spGenneral = CommonInformationManager::GetInstance()->GetAssayInfo(dataItem->m_assayCode);
        // 不是计算项目，通用项目中也找不到
		if (spGenneral == nullptr && !isCalc)
		{
			continue;
		}

		// 只导出生化项目(0013739)
		if (!(isCalc || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
            || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE))
		{
			continue;
		}

		ProcessOnebyOne(dataItem);
	}
}

bool ConstructHisExportInfo::ProcessOnebyOne(const std::shared_ptr<HistoryBaseDataByItem>& hisTestItem)
{
	auto  spTestItemInfo = hisTestItem;
	if (nullptr == spTestItemInfo)
	{
		return false;
	}

	ConstructHistoryExportItemInfo testItemDataExport;
	testItemDataExport.SetData(spTestItemInfo);
	SampleExportInfo sampleExportData;
	if (!testItemDataExport.BuildExportDataInfo(sampleExportData))
	{
		return false;
	}

	auto exportType = GetExportType(spTestItemInfo);
	auto& exportData = testItemDataExport.GetExportData();
	if (exportData.empty())
	{
		return false;
	}

	// 增加吸光度的标题
	if (m_titleExtendMap.count(exportType) <= 0)
	{
		m_titleExtendMap[exportType] = testItemDataExport.GetExtendTitle();
	}

	if (m_sampleExportDataVec.count(exportType))
	{
		if (exportData.size() >= 2)
		{
			m_sampleExportDataVec[exportType].emplace_back(exportData[0]);
			m_sampleExportDataVec[exportType].emplace_back(exportData[1]);
		}
		else
		{
			m_sampleExportDataVec[exportType].emplace_back(exportData[0]);
		}
	}
	else
	{
		m_sampleExportDataVec[exportType] = exportData;
	}

	return true;
}

const ExportType ConstructHisExportInfo::GetExportType(const std::shared_ptr<HistoryBaseDataByItem>& hisTestItem)
{
	ExportType returnType = ExportType::CH_NORMAL_AND_QC_TYPE;
	auto spTestItemInfo = hisTestItem;
	if (nullptr == spTestItemInfo)
	{
		return returnType;
	}

	// 是否校准样本
	bool isCali = (spTestItemInfo->m_sampleType == tf::HisSampleType::SAMPLE_SOURCE_CL);
	// 是否ISE项目
	bool isIse = CommonInformationManager::GetInstance()->IsAssayISE(spTestItemInfo->m_assayCode);
	if (isIse)
	{
		// ise项目的常规、急诊、质控的导出
		if (!isCali)
		{
			returnType = ExportType::CH_ISE_SAM_AND_QC_TYPE;
		}
		else
		{
			returnType = ExportType::CH_ISE_CALI_TYPE;
		}
	}
	else
	{
		// 普通项目的常规、急诊、质控的导出
		if (!isCali)
		{
			returnType = ExportType::CH_NORMAL_AND_QC_TYPE;
		}
		else
		{
			returnType = ExportType::CH_CALI_TYPE;
		}
	}

	return returnType;
}

bool ConvertDataToExportString::GenData(const bool isXlsx, const std::map<ExportType, SampleExportVec>& sourceData, const std::map<ExportType, QVariantList>& titleMap)
{
	if (!DictionaryQueryManager::GetExportConfig(m_saveConfigdata))
	{
		ULOG(LOG_ERROR, "Failed to get export config.");
		return false;
	}

	std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (pLoginUserInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't get UserInfo.");
		return false;
	}

	if (pLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)   // 普通用户不能导出
	{
		return false;
	}

	// 创建Head
	CreatTitle(pLoginUserInfo->type,  m_saveConfigdata, titleMap);
	// 创建内容
	for (const auto& data : sourceData)
	{
		auto type = data.first;
		if (m_saveConfigdata.count(type) <= 0)
		{
			continue;
		}

		for (const auto& exportData : data.second)
		{
			auto dataVariantList = CreatContent(isXlsx, pLoginUserInfo->type, exportData, m_saveConfigdata.at(type));
			if (m_exportContent.count(type))
			{
				m_exportContent.at(type).push_back(dataVariantList);
			}
			else
			{
				QVariantList tmpData;
				tmpData.push_back(dataVariantList);
				m_exportContent[type] = tmpData;
			}
		}
	}

	return true;
}

QVariantList ConvertDataToExportString::GetTitle(const ExportType& titletype)
{
	if (m_exportTitle.count(titletype))
	{
		return m_exportTitle.at(titletype);
	}

	return QVariantList();
}

QVariantList ConvertDataToExportString::GetContent(const ExportType& titletype)
{
	if (m_exportContent.count(titletype))
	{
		return m_exportContent.at(titletype);
	}

	return QVariantList();
}

ConvertDataToExportString::ConvertDataToExportString()
{
	// 导出设置
	m_exportDataMap[EIE_RESULT] = DATATYPE::DOUBLETYPE;
	m_exportDataMap[EIE_ORIGINAL_RESULT] = DATATYPE::DOUBLETYPE;
	m_exportDataMap[EIE_CONC] = DATATYPE::DOUBLETYPE;
	m_exportDataMap[EIE_ABS] = DATATYPE::INTTYPE;
	m_exportDataMap[EIE_SUCK_VOL] = DATATYPE::INTTYPE;
	m_exportDataMap[EIE_PRIMARY_WAVE] = DATATYPE::INTTYPE;
	m_exportDataMap[EIE_DEPUTY_WAVE] = DATATYPE::INTTYPE;
}

bool ConvertDataToExportString::CreatTitle(const tf::UserType::type UType, const std::map<ExportType, std::set<ExportInfoEn>>& savedata, const std::map<ExportType, QVariantList>& titleMap)
{
	QVariantList title_string;
	for (const auto& data : savedata)
	{
		title_string.clear();
		if (data.first == ExportType::CH_NORMAL_AND_QC_TYPE)
		{
			GenTitleList(UType, title_string, data.second);
			if (titleMap.count(data.first))
			{
				// 生化需要新增导出条目
				auto rawTitle = titleMap.at(data.first);
				QVariantList extendTitles;
				if (!WorkpageCommon::GetExportAbsTitle(rawTitle, extendTitles, data.second))
				{
					m_exportTitle[data.first] = title_string;
					continue;
				}

				for (const auto& title : extendTitles)
				{
					for (const auto& data : title.toList())
					{
						title_string << (data.toString() + "\t");
					}
				}
			}

			m_exportTitle[data.first] = title_string;
		}

		if (data.first == ExportType::CH_CALI_TYPE)
		{
			GenTitleList(UType, title_string, data.second);
			// 生化需要新增导出条目
			if (titleMap.count(data.first))
			{
				auto rawTitle = titleMap.at(data.first);
				QVariantList extendTitles;
				if (!WorkpageCommon::GetExportAbsTitle(rawTitle, extendTitles, data.second))
				{
					m_exportTitle[data.first] = title_string;
					continue;
				}

				for (const auto& title : extendTitles)
				{
					for (const auto& data : title.toList())
					{
						title_string << (data.toString() + "\t");
					}
				}
			}
			m_exportTitle[data.first] = title_string;
		}

		if (data.first == ExportType::CH_ISE_SAM_AND_QC_TYPE)
		{
			GenTitleList(UType, title_string, data.second);
			m_exportTitle[data.first] = title_string;
		}

		if (data.first == ExportType::CH_ISE_CALI_TYPE)
		{
			GenTitleList(UType, title_string, data.second);
			m_exportTitle[data.first] = title_string;
		}
	}

	return true;
}

QVariantList ConvertDataToExportString::CreatContent(const bool isXlsx, const tf::UserType::type UType, const SampleExportInfo& srcData, const std::set<ExportInfoEn>& configData)
{
	QString flag = "\t";
	if (isXlsx)
	{
		flag.clear();
	}

	// 记录导出文本
	QVariantList strExportTextList;
	// 内容
	for (auto iter : configData)
	{
		// 需要排除吸光度，放在数据最后
		if (iter == ExportInfoEn::EIE_PRIMARY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_DEPUTY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_ALL_WAVE_ABS)
		{
			continue;
		}

		auto pSEInfo = SEMetaInfos::GetInstance()->GetMetaInfo(iter);
		if (pSEInfo != nullptr && pSEInfo->m_UserType <= UType)
		{
			auto data = srcData.*(pSEInfo->m_Field);
			if (isXlsx)
			{
				auto iterExport = m_exportDataMap.find(iter);
				if (iterExport != m_exportDataMap.end())
				{
					QVariant convertData;
					if (ConvertData(iterExport->second, data, convertData))
					{
						strExportTextList << convertData;
						continue;
					}
				}
			}

			strExportTextList << (data + flag);
		}
	}

	auto convertToabs = [&](const bool isXlsx, const QString& waveData, const QString& flag, QVariantList& exportList)
	{
		auto waveAbs = waveData.split("\t");
		for (const auto& abs : waveAbs)
		{
			QVariant convertData;
			if (isXlsx && ConvertData(DATATYPE::INTTYPE, abs, convertData))
			{
				exportList << convertData;
				continue;
			}

			exportList << (abs + flag);
		}
	};

	if (configData.count(EIE_PRIMARY_WAVE_ABS) > 0)
	{
		convertToabs(isXlsx, srcData.strPrimaryWaveAbs, flag, strExportTextList);
	}

	if (configData.count(EIE_DEPUTY_WAVE_ABS) > 0)
	{
		convertToabs(isXlsx, srcData.strDeputyWaveAbs, flag, strExportTextList);
	}

	if (configData.count(EIE_ALL_WAVE_ABS) > 0)
	{
		convertToabs(isXlsx, srcData.strAllWaveAbs, flag, strExportTextList);
	}

	return strExportTextList;
}

bool ConvertDataToExportString::GenTitleList(const tf::UserType::type UType, QVariantList& updateTitle, const std::set<ExportInfoEn>& configData)
{
	updateTitle.clear();
	for (auto iter : configData)
	{
		// 需要排除吸光度，放在数据最后
		if (iter == ExportInfoEn::EIE_PRIMARY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_DEPUTY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_ALL_WAVE_ABS)
		{
			continue;
		}

		auto pSEInfo = SEMetaInfos::GetInstance()->GetMetaInfo(iter);
		if (pSEInfo != nullptr && pSEInfo->m_UserType <= UType)
		{
			updateTitle << (pSEInfo->m_strName + "\t");
		}
	}

	return true;
}

bool ConvertDataToExportString::ConvertData(const DATATYPE dstType, const QString& input, QVariant& dst)
{
	bool isOk = false;
	if (dstType == DATATYPE::INTTYPE)
	{
		dst = input.toInt(&isOk);
	}
	else if (dstType == DATATYPE::DOUBLETYPE)
	{
		dst = input.toDouble(&isOk);
	}

	return isOk;
}

SampleColumn& SampleColumn::Instance()
{
	static SampleColumn instance;
	return instance;
}

SampleColumn::COL SampleColumn::IndexSampleToStatus(SAMPLEMODE mode, int column)
{
	// 样本申请页面的列，不允许设置
	if (mode == SAMPLEMODE::SAMPLEAPP)
	{
		return COL(column + 1);
	}

	// 数据浏览按样本展示的列
	if (m_sampleKeymap.contains(column))
	{
		return m_sampleKeymap[column].type;
	}

	return COL::INVALID;
}

QStringList SampleColumn::GetHeaders(SAMPLEMODE mode, bool IsHistory)
{
	QStringList headers;
	// 按样本
	if (mode == SAMPLEMODE::SAMPLE)
	{
		for (const auto& data : m_sampleKeymap)
		{
			headers << data.name;
		}
	}
	// 按项目
	else if (mode == SAMPLEMODE::ASSAYLIST)
	{
		for (const auto& data : m_assayKeymap)
		{
			headers << data.name;
		}
	}
	// 样本的项目
	else
	{
		for (const auto& data : m_sampleAssayKeymap)
		{
			if (IsHistory && data.type == SAMPLEASSAY::RECHECKOPTION)
			{
				continue;
			}

			headers << data.name;
		}
	}

	return headers;
}

bool SampleColumn::SetAttributeOfType(SAMPLEMODE mode, int index, int width)
{
	ULOG(LOG_INFO, "%s(modify the width index:%d - width:%d,)", __FUNCTION__, index, width);
	// 超过500认为是错误设置(不允许超过500)
	if (index < 0 || width <= 0 || width >= MAXWIDTH)
	{
		return false;
	}

	// 按样本展示
	if (mode == SAMPLEMODE::SAMPLE)
	{
		if (m_sampleKeymap.count(index) <= 0)
		{
			return false;
		}

		auto& item = m_sampleKeymap[index];
		item.width = width;

		// 映射到按项目展示(按样本展示对应的项目变化以后，按项目展示其中相同的列也要随同变化)
		for (auto& assayItem : m_assayKeymap)
		{
			if (int(item.type) != int(assayItem.type))
			{
				continue;
			}

			assayItem.width = width;
		}

		// 正常情况下不可能为空
		if (m_displaySet.empty())
		{
			return false;
		}

		// 依次遍历，修改样本列的宽度
		AssignDisplaySet(SAMPLETYPE, int(item.type), width);
	}
	// 按项目展示
	else if (mode == SAMPLEMODE::ASSAYLIST)
	{
		if (m_assayKeymap.count(index) <= 0)
		{
			return false;
		}

		auto& item = m_assayKeymap[index];
		item.width = width;

		int type = int(item.type) - int(COL::PRINT) + 1;
		// 小于0表明是属于样本信息段
		if (type < 0)
		{
			for (auto& sampleItem : m_sampleKeymap)
			{
				if (int(item.type) != int(sampleItem.type))
				{
					continue;
				}

				sampleItem.width = width;
			}

			// 正常情况下不可能为空
			if (m_displaySet.empty())
			{
				return false;
			}

			// 依次遍历，修改样本列的宽度
			AssignDisplaySet(SAMPLETYPE, (int(item.type) - 1), width);
		}
		else
		{
			for (auto& sanpleAssayItem : m_sampleAssayKeymap)
			{
				if (int(type) != int(sanpleAssayItem.type))
				{
					continue;
				}

				sanpleAssayItem.width = width;

				// 正常情况下不可能为空
				if (m_displaySet.empty())
				{
					return false;
				}

				// 依次遍历，修改样本列的宽度
				AssignDisplaySet(SAMPLETYPE, (int(item.type) - int(COL::PRINT)), width);
			}

			// 正常情况下不可能为空
			if (m_displaySet.empty())
			{
				return false;
			}

			// 依次遍历，修改样本列的宽度
			AssignDisplaySet(TESTITEMTYPE, (int(item.type) - int(COL::PRINT)), width);
		}
	}
	// 按样本所属项目
	else if (mode == SAMPLEMODE::SAMPLEASSAY)
	{
		if (m_sampleAssayKeymap.count(index) <= 0)
		{
			return false;
		}

		auto& item = m_sampleAssayKeymap[index];
		item.width = width;

		// 映射到按项目展示(按样本的项目展示对应的项目变化以后，按项目展示其中相同的列也要随同变化)
		for (auto& assayItem : m_assayKeymap)
		{
			if ((int(item.type) + int(COL::PRINT) - 1) != int(assayItem.type))
			{
				continue;
			}

			assayItem.width = width;
		}

		// 正常情况下不可能为空
		if (m_displaySet.empty())
		{
			return false;
		}

		// 依次遍历，修改样本列的宽度
		AssignDisplaySet(TESTITEMTYPE, (int(item.type) - 1), width);
	}

	std::vector<DisplaySet> alldata;
	if (!DictionaryQueryManager::GetUiDispalySet(alldata))
	{
		ULOG(LOG_ERROR, "Failed to get DisplaySet.");
		return false;
	}

	DisplaySet patientData;
	for (auto data : alldata)
	{
		if (data.type == PATIENTTYPE)
		{
			patientData = data;
		}
	}

	for (auto& data : m_displaySet)
	{
		if (data.type == PATIENTTYPE)
		{
			data = patientData;
		}
	}

	// 保存更改
	if (!DictionaryQueryManager::SaveUiDisplaySet(m_displaySet))
	{
		ULOG(LOG_ERROR, "Failed to save display set.");
		return false;
	}

	return true;
}

void SampleColumn::AssignDisplaySet(DisplayType displayType, int colType, int width)
{
	// 依次遍历，修改样本列的宽度
	for (auto& display : m_displaySet)
	{
		if (display.type == displayType)
		{
			// 依次查询
			for (auto& dataItem : display.displayItems)
			{
				if (!dataItem.enable)
				{
					continue;
				}

				if (colType != int(dataItem.type))
				{
					continue;
				}

				dataItem.width = width;
			}
		}
	}
}

boost::optional<std::pair<int, int>> SampleColumn::GetAttributeByType(SAMPLEMODE mode, int type)
{
	switch (mode)
	{
		case SAMPLEMODE::SAMPLE:
			// 按样本展示
			return GetAttributePair(m_sampleKeymap, type);
		case SAMPLEMODE::ASSAYLIST:
			// 按项目展示
			return GetAttributePair(m_assayKeymap, type);
		case SAMPLEMODE::SAMPLEASSAY:
			// 按样本所属项目
			return GetAttributePair(m_sampleAssayKeymap, type);
		default:
			break;
	}

	return boost::none;
}

SampleColumn::SampleColumn()
	: QObject()
{
	REGISTER_HANDLER(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE, this, OnUpdateColumn);
	OnUpdateColumn();
}

SampleColumn::SAMPLEASSAY SampleColumn::IndexAssayToStatus(int column)
{
	/// 参数检查（范围超限）
	if (m_sampleAssayKeymap.size() <= column || column < 0)
	{
		return SAMPLEASSAY::INVALID;
	}

	if (m_sampleAssayKeymap.contains(column))
	{
		return m_sampleAssayKeymap[column].type;
	}

	return SAMPLEASSAY::INVALID;
}

void SampleColumn::OnUpdateColumn()
{
	std::vector<DisplaySet> data;
	if (!DictionaryQueryManager::GetUiDispalySet(data))
	{
		ULOG(LOG_ERROR, "Failed to get DisplaySet.");
		return;
	}

	m_displaySet = data;
	std::shared_ptr<CommonInformationManager> assayMrg = CommonInformationManager::GetInstance();
	// 位置从2开始的原因是因为 1：对于样本展示和项目展示，前两列是选中和状态列，不允许调整和删除所有首先设置好
	int offset = 2;
	m_assayKeymap.clear();
	m_sampleKeymap.clear();
	m_sampleAssayKeymap.clear();
	for (const auto& dataSet : data)
	{
		if (dataSet.type == SAMPLETYPE)
		{
			for (const auto& sampleSet : dataSet.displayItems)
			{
				if (sampleSet.enable && sampleSet.enSelect)
				{
					ItemInfo<COL> itemData;
					itemData.index = sampleSet.postion + offset;
					itemData.name = assayMrg->GetNameOfSampleOrAssay(int(sampleSet.type));
					itemData.type = COL(sampleSet.type);
					itemData.width = sampleSet.width;
					m_sampleKeymap[itemData.index] = itemData;

					ItemInfo<COLASSAY> itemAssayData;
					itemAssayData.index = sampleSet.postion + offset;
					itemAssayData.name = itemData.name;
					itemAssayData.width = sampleSet.width;
					itemAssayData.type = COLASSAY(sampleSet.type);
					m_assayKeymap[itemAssayData.index] = itemAssayData;
				}
			}
		}
	}

	ItemInfo<SAMPLEASSAY> itemDataNew;
	itemDataNew.index = 0;
	itemDataNew.width = 45;
	itemDataNew.name = tr("复查");
	itemDataNew.type = SAMPLEASSAY::RECHECKOPTION;
	m_sampleAssayKeymap[itemDataNew.index] = itemDataNew;

	int startsize = m_assayKeymap.size();
	for (const DisplaySet& dataSet : data)
	{
		if (dataSet.type == TESTITEMTYPE)
		{
			for (const DisplaySetItem& assaySet : dataSet.displayItems)
			{
				if (assaySet.enable && assaySet.enSelect)
				{
					ItemInfo<COLASSAY> itemData;
					ItemInfo<SAMPLEASSAY> itemDataNew;
					itemData.index = assaySet.postion + startsize + 2;
					itemData.width = assaySet.width;
					itemData.type = COLASSAY(assaySet.type + int(COL::PRINT));
					itemData.name = assayMrg->GetNameOfSampleOrAssay(int(itemData.type));
					m_assayKeymap[itemData.index] = itemData;

					itemDataNew.index = assaySet.postion + 1;
					itemDataNew.type = SAMPLEASSAY(assaySet.type + 1);
					itemDataNew.width = assaySet.width;
					itemDataNew.name = itemData.name;
					m_sampleAssayKeymap[itemDataNew.index] = itemDataNew;
				}
			}
		}
	}

	emit ColumnChanged();
}

///
/// @brief 复查状态
///
/// @param[in]  item  
///
/// @return 复查状态按钮的Pic路径
///
/// @par History:
/// @li 5774/WuHongTao，2023年5月30日，新建函数
///
QVariant WorkpageCommon::GetItemReCheckPixmap(const tf::TestItem& item)
{
    // 复查(设置了rerun或者有了复查Key)
    if ((item.__isset.rerun && item.rerun)
        || (item.__isset.lastTestResultKey && item.lastTestResultKey.assayTestResultId > 0))
    {
        return QPixmap(pngStatusRecheck);
    }

    return QPixmap(pngStatusDefault);
}

QVariant WorkpageCommon::GetItemReCheckPixmap(const std::vector<std::shared_ptr<tf::TestItem>>& vecItem)
{
    for (const auto& pItem : vecItem)
    {
        if (pItem == nullptr)
        {
            continue;
        }
        // 复查(设置了rerun或者有了复查Key)
        if ((pItem->__isset.rerun && pItem->rerun)
            || (pItem->__isset.lastTestResultKey && pItem->lastTestResultKey.assayTestResultId > 0))
        {
            return QPixmap(pngStatusRecheck);
        }
    }

    return QPixmap(pngStatusDefault);
}

QVariant WorkpageCommon::GetRecheckPixmap(const boost::posix_time::ptime& retestEndtime, const boost::posix_time::ptime& endTestTime)
{
    // "如果复查结束时间和首查时间一致，则没有做复查 // retestEndtime == endTestTime ||" 被去除， 以解决bug：0012482，
    // 1970-1-1 0:0:0为未设值的时间，这里处理一下显示
    if (!retestEndtime.is_not_a_date_time() && retestEndtime != origTime)
    {
        return QPixmap(pngStatusRecheck);
    }

    // 默认
    return QPixmap(pngStatusDefault);
}

QVariant WorkpageCommon::GetSendLisPixmap()
{
	bool isPipel = DictionaryQueryManager::GetInstance()->GetPipeLine();
	if (isPipel)
	{
		return QPixmap(pngStatusUpload);
	}
	return QPixmap(pngUploadAi);
}

QVariant WorkpageCommon::GetDefaultPixmap()
{
	bool isPipel = DictionaryQueryManager::GetInstance()->GetPipeLine();
	if (isPipel && QDataItemInfo::Instance().IsAiRecognition())
	{
		return QPixmap(pngStatusDefault);
	}

	return QPixmap(pngStatusVerifPrint);
}

QVariant WorkpageCommon::GetTestStatusPixmap(int iStatus)
{
    switch (iStatus)
    {
    case ::tf::TestItemStatus::TEST_ITEM_STATUS_PENDING:
        // 待测
        return QPixmap(pngStatusPending);
    case ::tf::TestItemStatus::TEST_ITEM_STATUS_TESTING:
        // 测试中
        return QPixmap(pngStatusTesting);
    case ::tf::TestItemStatus::TEST_ITEM_STATUS_TESTED:
        // 测试完成
        return QPixmap(pngStatusTested);
    default:
        return QPixmap(pngStatusDefault);
    }

    // 默认状态
    return QPixmap(pngStatusDefault);
}

QVariant WorkpageCommon::GetItemAduitPrintPixmap(const std::shared_ptr<tf::SampleInfo>& pSample)
{
    if (pSample == nullptr)
    {
        return QPixmap(pngStatusVerifPrint);
    }
    // 审核
    if (pSample->__isset.audit && pSample->audit)
    {
        return QPixmap(pngStatusVersify);
    }
    // 打印
    if (pSample->__isset.printed && pSample->printed)
    {
        return QPixmap(pngPrint);
    }

    return QPixmap(pngStatusVerifPrint);
}

///
/// @brief 样本是否被上传
///
/// @param[in]  sample  样本
///
/// @return 显示上传的pic
///
/// @par History:
/// @li 5774/WuHongTao，2023年10月11日，新建函数
///
QVariant WorkpageCommon::GetUploadPixmap(const tf::SampleInfo& sample, bool bAiRec)
{
    // 是否联机
    bool isPipel = DictionaryQueryManager::GetInstance()->GetPipeLine();
    // 样本已上传
    if (sample.__isset.uploaded && sample.uploaded)
    {
        return (isPipel && bAiRec) ? QPixmap(pngStatusUpload) : QPixmap(pngUploadAi);
    }

    if (isPipel && bAiRec)
    {
        return QPixmap(pngStatusDefault);
    }

    // 默认显示// TODO:Confirm 此处默认使用verifPrint图标在单机模式下和原型图不符
    // (Confirmed:单机默认显示一个长方块，联机默认显示两个方块)
    return QPixmap(pngStatusVerifPrint);
}

QVariant WorkpageCommon::GetSampleAiStatus(const tf::SampleInfo& sample)
{
    // 默认显示
    return QPixmap(pngStatusDefault);
}

std::vector<std::shared_ptr<ch::tf::AssayTestResult>> WorkpageCommon::GetChAssayTestResult(int64_t itemId)
{
	std::vector<std::shared_ptr<ch::tf::AssayTestResult>> results;
	ch::tf::AssayTestResultQueryCond queryAssaycond;
	ch::tf::AssayTestResultQueryResp assayTestResult;
	// 根据测试项目的ID来查询结果
	queryAssaycond.__set_testItemId(itemId);

	if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
		|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| assayTestResult.lstAssayTestResult.empty())
	{
		ULOG(LOG_ERROR, "%s()", __FUNCTION__);
	}
	else
	{
		results.resize(assayTestResult.lstAssayTestResult.size());
		std::transform(assayTestResult.lstAssayTestResult.begin(), assayTestResult.lstAssayTestResult.end(),
			results.begin(),
		[](const ch::tf::AssayTestResult& result)->std::shared_ptr<ch::tf::AssayTestResult>
		{
			return std::make_shared<ch::tf::AssayTestResult>(result);
		});
	}

	return std::move(results);
}

std::vector<std::shared_ptr<ise::tf::AssayTestResult>> WorkpageCommon::GetIseAssayTestResult(int64_t itemId)
{
	std::vector<std::shared_ptr<ise::tf::AssayTestResult>> results;
	ise::tf::AssayTestResultQueryCond queryAssaycond;
	ise::tf::AssayTestResultQueryResp assayTestResult;
	// 根据测试项目的ID来查询结果
	queryAssaycond.__set_testItemId(itemId);

	if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
		|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| assayTestResult.lstAssayTestResult.empty())
	{
		ULOG(LOG_ERROR, "%s()", __FUNCTION__);
	}
	else
	{
		results.resize(assayTestResult.lstAssayTestResult.size());
		std::transform(assayTestResult.lstAssayTestResult.begin(), assayTestResult.lstAssayTestResult.end(),
			results.begin(),
			[](const ise::tf::AssayTestResult& result)->std::shared_ptr<ise::tf::AssayTestResult>
		{
			return std::make_shared<ise::tf::AssayTestResult>(result);
		});
	}

	return std::move(results);
}

std::vector<std::shared_ptr<tf::CalcAssayTestResult>> WorkpageCommon::GetCalcAssayTestResult(int64_t itemId)
{
	std::vector<std::shared_ptr<tf::CalcAssayTestResult>> results;
	::tf::CalcAssayTestResultQueryCond queryAssaycond;
	queryAssaycond.__set_testItemId(itemId);
	::tf::CalcAssayTestResultQueryResp assayTestResult;

	if (!DcsControlProxy::GetInstance()->QueryCalcAssayTestResult(assayTestResult, queryAssaycond)
		|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| assayTestResult.lstCalcAssayTestResult.empty())
	{
		ULOG(LOG_ERROR, "%s()", __FUNCTION__);
	}
	else
	{
		results.resize(assayTestResult.lstCalcAssayTestResult.size());
		std::transform(assayTestResult.lstCalcAssayTestResult.begin(), assayTestResult.lstCalcAssayTestResult.end(),
			results.begin(),
			[](const tf::CalcAssayTestResult& result)->std::shared_ptr<tf::CalcAssayTestResult>
		{
			return std::make_shared<tf::CalcAssayTestResult>(result);
		});
	}

	return std::move(results);

}

std::shared_ptr<ch::tf::AssayTestResult> WorkpageCommon::GetAssayResultByid(int64_t id)
{
	ch::tf::AssayTestResultQueryCond queryAssaycond;
	queryAssaycond.__set_id(id);
	ch::tf::AssayTestResultQueryResp assayTestResult;

	if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
		|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| assayTestResult.lstAssayTestResult.empty())
	{
		return nullptr;
	}

	return std::make_shared<ch::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);
}

int WorkpageCommon::GetcaliTimes(std::map<int64_t, std::map<int, int>>& caliMap, const int64_t sampleId, const int assayCode)
{
	// 没有导出过的校准样本 或者
	// 如果这个样本的这个项目没有导出过，那么就是第一次
	if (caliMap.find(sampleId) == caliMap.end() 
		|| caliMap[sampleId].find(assayCode) == caliMap[sampleId].end())
	{
		caliMap[sampleId][assayCode] = 1;
	}
	else
	{
		caliMap[sampleId][assayCode] += 1;
	}

	return caliMap[sampleId][assayCode];
}

bool WorkpageCommon::GetChResultExport(const std::shared_ptr<ch::tf::AssayTestResult>& spAssayTestResult, SampleExportInfo& exportData)
{
	if (nullptr == spAssayTestResult)
	{
		return false;
	}

	int assayCode = spAssayTestResult->assayCode;
	std::shared_ptr<ch::tf::GeneralAssayInfo>  spChGeneralAssayInfo = nullptr;
	if (CommonInformationManager::GetInstance()->IsAssaySIND(assayCode))
	{
		spChGeneralAssayInfo = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(ch::tf::g_ch_constants.ASSAY_CODE_SIND, tf::DeviceType::DEVICE_TYPE_C1000);
	}
	else
	{
		spChGeneralAssayInfo = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(assayCode, tf::DeviceType::DEVICE_TYPE_C1000);
	}

	// 检查项目参数信息
	if (nullptr == spChGeneralAssayInfo)
	{
		return false;
	}

	// 校准点数就是校准品数目
	exportData.strCaliPoints = QString::number(spChGeneralAssayInfo->caliQuantity);

	auto spGeneralAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
	if (nullptr == spGeneralAssayInfo)
	{
		return false;
	}

	int decimalPlace = spGeneralAssayInfo->decimalPlace;
	// 项目检测完成时间
	exportData.strEndTestTime = QString::fromStdString(spAssayTestResult->endTime);
	// 原始结果
	auto result = CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, spAssayTestResult->conc);
	exportData.strOrignialTestResult = QString::number(result, 'f', decimalPlace);
	// 结果(有可能是被修改过的)
	double showResult;
	if (spAssayTestResult->__isset.concEdit)
	{
		// 转化修改结果为double
		std::stringstream editConc(spAssayTestResult->concEdit);
		double tempResult;
		if ((editConc >> tempResult))
		{
			showResult = CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, tempResult);
		}
		else
		{
			showResult = result;
		}
	}
	else
	{
		showResult = result;
	}

	if (spAssayTestResult->__isset.concEdit)
	{
		exportData.strTestResult = "*" + QString::number(showResult, 'f', decimalPlace);
		// 浓度
		exportData.strConc = "*" + QString::number(showResult, 'f', decimalPlace);
	}
	else
	{
		exportData.strTestResult = QString::number(showResult, 'f', decimalPlace);
		// 浓度
		exportData.strConc = QString::number(showResult, 'f', decimalPlace);
	}

	// 计算吸光度
	exportData.strCalculateAbs = QString::number(spAssayTestResult->abs);
	// 吸光度
	exportData.strAbs = QString::number(spAssayTestResult->abs);
	// 结果状态(数据报警)
	exportData.strResultStatus = QString::fromStdString(spAssayTestResult->resultStatusCodes);
	// 单位
	exportData.strUnit = CommonInformationManager::GetInstance()->GetCurrentUnit(assayCode);
	// 样本量
	//exportData.strSuckVol = QString::number(double(spAssayTestResult->sampleVol) / 10, 'f', 1);
	if (spAssayTestResult->dilutionFactor == 1)
	{
		exportData.strSuckVol = CommonInformationManager::GetInstance()->GetDilutionFactor(spAssayTestResult);
	}

	// 手工稀释倍数(机外稀释)20240715最新需求,只有当稀释倍数大于1的时候，才显示倍数
	if (spAssayTestResult->preDilutionFactor > 1)
	{
		exportData.strPreDilutionFactor = QString::number(spAssayTestResult->preDilutionFactor);
	}

	// 机内稀释倍数20240715最新需求,只有当稀释倍数大于1的时候，才显示倍数
	if (spAssayTestResult->dilutionFactor > 1)
	{
		exportData.strDilutionFactor = CommonInformationManager::GetInstance()->GetDilutionFactor(spAssayTestResult);
	}
	// 视觉识别结果
	exportData.strAIResult = " "/*QString::number((::ch::tf::AIResult::type)(spAssayTestResult->aiResult))*/;
	// 试剂盘
	//exportData.strReagentDisk = " ";
	// 反应杯号
	exportData.strAssayCupBatchNo = QString::number(spAssayTestResult->cupSN);

	// 试剂上机时间
	exportData.strRgtRegisterT = (spAssayTestResult->__isset.rgtRegisterTime ? QString::fromStdString(spAssayTestResult->rgtRegisterTime) : " ");
	if (spAssayTestResult->__isset.reagentKeyInfos)
	{
		// 试剂批号
		exportData.strReagentBatchNo = ((spAssayTestResult->reagentKeyInfos.size() > 0) ? QString::fromStdString(spAssayTestResult->reagentKeyInfos[0].lot) : " ");
		// 试剂瓶号
		exportData.strReagentSerialNo = ((spAssayTestResult->reagentKeyInfos.size() > 0) ? QString::fromStdString(spAssayTestResult->reagentKeyInfos[0].sn) : " ");
		// 试剂失效日期
		exportData.strReagentExpTime = QString::fromStdString(spAssayTestResult->resultSupplyInfo.reagentexpiryDate);
		// 开瓶有效期
		exportData.strOpenBottleExpiryTime = (spAssayTestResult->resultSupplyInfo.reagentopenBottleExpiryTime >= 0 ? QString::number(spAssayTestResult->resultSupplyInfo.reagentopenBottleExpiryTime) : "0");
	}

	if (spAssayTestResult->__isset.resultCaliInfo)
	{
		// 校准品名称
		exportData.strCaliName = QString::fromStdString(spAssayTestResult->resultCaliInfo.calibratorName);
		// 校准品批号
		exportData.strCaliLot = QString::fromStdString(spAssayTestResult->resultCaliInfo.calibratorLot);
		// 校准时间
		exportData.strCaliTime = QString::fromStdString(spAssayTestResult->resultCaliInfo.caliDate);
	}

	// 校准方法
	exportData.strCaliType = ThriftEnumTrans::GetTestItemMethod(spChGeneralAssayInfo->calibrationType);
	// 校准水平
	exportData.strCaliLevel = QString::number(spAssayTestResult->caliLevel);
	// 稀释样本量
	exportData.strDilutionSampleVol = QString::number(double(spAssayTestResult->caliDilutSampleVol)/10, 'f', 1);
	// 稀释液量
	exportData.strDiluentVol = QString::number(double(spAssayTestResult->caliDilutVol) / 10, 'f', 1);
	// 分析方法
	exportData.strAnalysisMethod = ThriftEnumTrans::GetAnalysisMethodName(spChGeneralAssayInfo->analysisMethod);
	// 主波长
	exportData.strPrimaryWave = (!spAssayTestResult->primarySubWaves.empty() ? QString::number(spAssayTestResult->primarySubWaves[0]) : " ");
	// 次波长
	exportData.strDeputyWave = ((spAssayTestResult->primarySubWaves.size() > 1) ? QString::number(spAssayTestResult->primarySubWaves[1]) : " ");
	// 反应时间
	exportData.strReactionTime = QString::number(((spAssayTestResult->detectPoints.size() == 34) ? 10 : 5));
	// 测光点(总数，非m,n)
	exportData.strDetectPoint = QString::number(static_cast<long>(spAssayTestResult->detectPoints.size()));
	return true;
}

bool WorkpageCommon::GetIseResultExport(const std::shared_ptr<ise::tf::AssayTestResult>& spAssayTestResult, SampleExportInfo& exportData)
{
	if (nullptr == spAssayTestResult)
	{
		return false;
	}

	int assayCode = spAssayTestResult->assayCode;
	auto spGeneralAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
	if (spGeneralAssayInfo == nullptr)
	{
		return false;
	}

	int decimalPlace = spGeneralAssayInfo->decimalPlace;
	// 项目检测完成时间
	exportData.strEndTestTime = QString::fromStdString(spAssayTestResult->endTime);
	// 原始结果
	auto orignData = CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, spAssayTestResult->conc);
	exportData.strOrignialTestResult = QString::number(orignData, 'f', decimalPlace);

	// 结果
	double showResult;
	if (spAssayTestResult->__isset.concEdit)
	{
		// 转化修改结果为double
		std::stringstream editConc(spAssayTestResult->concEdit);
		double tempResult;
		if ((editConc >> tempResult))
		{
			showResult = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayTestResult->assayCode, tempResult);
		}
		else
		{
			showResult = orignData;
		}
	}
	else
	{
		showResult = orignData;
	}

	if (spAssayTestResult->__isset.concEdit)
	{
		exportData.strTestResult = "*" + QString::number(showResult, 'f', decimalPlace);
		// 浓度
		exportData.strConc = "*" + QString::number(showResult, 'f', decimalPlace);
	}
	else
	{
		exportData.strTestResult = QString::number(showResult, 'f', decimalPlace);
		// 浓度
		exportData.strConc = QString::number(showResult, 'f', decimalPlace);
	}

	// 计算电动势
	exportData.strCalcuLateEmf = QString::number(spAssayTestResult->testEMF.sampleEMF, 'f', 3);
	// 数据报警
	exportData.strResultStatus = QString::fromStdString(spAssayTestResult->resultStatusCodes);
	// 视觉识别结果
	exportData.strAIResult = " ";/*QString::number((::ch::tf::AIResult::type)(spAssayTestResult->aiResult))*/;
	// 手工稀释倍数(机外稀释)
	exportData.strPreDilutionFactor = QString::number(spAssayTestResult->preDilutionFactor);
	// 机内稀释
	exportData.strDilutionFactor = QString::number(spAssayTestResult->dilutionFactor);
	// 单位
	auto strUnit = CommonInformationManager::GetInstance()->GetCurrentUnit(spAssayTestResult->assayCode);
	exportData.strUnit = strUnit;

	if (spAssayTestResult->__isset.resultSupplyInfo)
	{
		// 内部标准液批号
		exportData.strIsLOt = QString::fromStdString(spAssayTestResult->resultSupplyInfo.IS_Lot);
		// 内部标准液瓶号
		exportData.strIsSn = QString::fromStdString(spAssayTestResult->resultSupplyInfo.IS_Sn);
		// 缓冲液批号
		exportData.strDiluentLot = QString::fromStdString(spAssayTestResult->resultSupplyInfo.diluent_Lot);
		// 缓冲液瓶号
		exportData.strDiluentSn = QString::fromStdString(spAssayTestResult->resultSupplyInfo.diluent_Sn);
	}

	if (spAssayTestResult->__isset.resultCaliInfo)
	{
		// 校准品名称
		exportData.strCaliName = QString::fromStdString(spAssayTestResult->resultCaliInfo.calibratorName);
		// 校准品批号
		exportData.strCaliLot = QString::fromStdString(spAssayTestResult->resultCaliInfo.calibratorLot);
		// 校准时间
		exportData.strCaliTime = QString::fromStdString(spAssayTestResult->resultCaliInfo.caliDate);
	}

	return true;
}

bool WorkpageCommon::GetResultAbs(const std::shared_ptr<ch::tf::AssayTestResult> spAssayTestResult, SampleExportInfo& exportData, QVariantList& dynamicTitle)
{
	if (nullptr == spAssayTestResult)
	{
		return false;
	}

	dynamicTitle.clear();
	// 水空白波长
	std::vector<std::map<int32_t, int32_t>> waterBlanks;
	QVariantList dyTitles;
	QVariantList rowDatas;
	if (DecodeWaterBlankOds(spAssayTestResult->waterBlanks, waterBlanks) && !waterBlanks.empty())
	{
		QVariantList rowBlankDatas;
		QVariantList titleBlank;
		for (auto& wb : waterBlanks[0])
		{
			rowBlankDatas << wb.second;
			titleBlank << (QString::number(wb.first) + QObject::tr("波长水空白"));
		}
		dyTitles.push_back(titleBlank);
		rowDatas.push_back(rowBlankDatas);
	}

	// 测光点的吸光度
	{
		std::map<int, std::vector<int>> wlClassify;
		// 获取各个测光点的吸光度的值列表
		for (const auto& detectData : spAssayTestResult->detectPoints)
		{
			// 将每个波长的吸光度都分类出来
			auto ods = detectData.ods;
			for (auto& od : ods)
			{
				wlClassify[od.first].push_back(od.second);
			}
		}

		// 导出主副波长
		int loop = 0;
		do
		{
			QVariantList rowInnerDatas;
			QVariantList titleMain;
			QString waveName;
			if (loop == 0)
				waveName = QObject::tr("主波长吸光度");
			else
				waveName = QObject::tr("副波长吸光度");

			for (size_t o_i = 0; o_i < 34; )
			{
				if (loop >= spAssayTestResult->primarySubWaves.size() || wlClassify.find(spAssayTestResult->primarySubWaves[loop]) == wlClassify.end())
				{
					rowInnerDatas << " ";
				}
				else
				{
					auto& odsVec = wlClassify[spAssayTestResult->primarySubWaves[loop]];
					// 按照最多34个点导出，不足的，导出为" ";
					if (o_i < odsVec.size())
					{
						rowInnerDatas << odsVec[o_i];
					}
					else
					{
						rowInnerDatas << " ";
					}
				}

				titleMain << (waveName + QString::number(++o_i));
			}
			dyTitles.push_back(titleMain);
			rowDatas.push_back(rowInnerDatas);

		} while (++loop < 2);

		// 重新组织波长和吸光度数据
		for (auto& wl : wlClassify)
		{
			QVariantList rowReInnerDatas;
			QVariantList titleAll;
			for (size_t o_i = 0; o_i < 34; )
			{
				// 按照最多34个点导出，不足的，导出为" ";
				if (o_i < wl.second.size())
				{
					rowReInnerDatas << wl.second[o_i];
					titleAll << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
				}
				else
				{
					rowReInnerDatas << " ";
					titleAll << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
				}
			}
			dyTitles.push_back(titleAll);
			rowDatas.push_back(rowReInnerDatas);
		}
	}

	auto fjoin = [](const QVariantList& rawData, const QString flag)->QString
	{
		QStringList strWaveAbs;
		for (const auto& data : rawData)
		{
			strWaveAbs.push_back(data.toString());
		}

		return strWaveAbs.join(flag);
	};

	// 保存标题
	for (const auto& dataTitle : dyTitles)
	{
		auto waveTitle = fjoin(dataTitle.toList(), "\t");
		dynamicTitle.push_back(dataTitle);
	}

	exportData.strPrimaryWaveAbs.clear();
	exportData.strDeputyWaveAbs.clear();
	exportData.strAllWaveAbs.clear();
	// 保存数据
	int i = 0;
	for (const auto& data : rowDatas)
	{
		auto waveData = fjoin(data.toList(), "\t");
		if (i == 0 || i == 1)
		{
			exportData.strPrimaryWaveAbs += (waveData + "\t");
		}

		if (i == 2)
		{
			exportData.strDeputyWaveAbs = (waveData);
		}

		exportData.strAllWaveAbs += (waveData + "\t");
		i++;
	}

	exportData.strPrimaryWaveAbs.chop(1);
	exportData.strAllWaveAbs.chop(1);
	return true;
}

bool WorkpageCommon::GetExportAbsTitle(const QVariantList& rawtitle, QVariantList& outputTitle, const std::set<ExportInfoEn>& configData)
{
	if (rawtitle.empty())
	{
		return false;
	}

	// 主次吸光度只要显示其中一个就要求显示空白
	if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
		|| configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0)
	{
		outputTitle << rawtitle[0];
	}

	// 要求显示主吸光度
	if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
		&& rawtitle.size() >= 2)
	{
		outputTitle << rawtitle[1];
	}

	// 要求显示次吸光度
	if (configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0
		&& rawtitle.size() >= 3)
	{
		outputTitle << rawtitle[2];
	}

	// 要求显示所有吸光度
	if (configData.count(ExportInfoEn::EIE_ALL_WAVE_ABS) > 0)
	{
		for (const auto& titles : rawtitle)
		{
			outputTitle << titles;
		}
	}

	return true;
}

bool WorkpageCommon::GetCalcResultExport(const std::shared_ptr<tf::CalcAssayTestResult>& spAssayTestResult, SampleExportInfo& exportData)
{
	if (nullptr == spAssayTestResult)
	{
		return false;
	}

	int assayCode = spAssayTestResult->assayCode;
	auto spCalcAssayInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
	if (spCalcAssayInfo == nullptr)
	{
		return false;
	}

	// 原始结果
	if (spAssayTestResult->__isset.result)
	{
		exportData.strOrignialTestResult = QString::fromStdString(spAssayTestResult->result);
	}

	// 结果
	if (spAssayTestResult->__isset.editResult && !spAssayTestResult->editResult.empty())
	{
		// modify bug12502 by wuht
		exportData.strTestResult = ("*" + QString::fromStdString(spAssayTestResult->editResult));
	}
	else
	{
		exportData.strTestResult = QString::fromStdString(spAssayTestResult->result);
	}

	// 结果生成时间
	if (spAssayTestResult->__isset.dateTime)
	{
		exportData.strEndTestTime = QString::fromStdString((spAssayTestResult->dateTime));
	}

	return true;
}

std::pair<std::string, std::string> WorkpageCommon::GetItemTestTime(int64_t db, int assayCode)
{
	// 将测试完成时间排序
	std::set<std::string> finishTimeMap;
	bool isISE = CommonInformationManager::GetInstance()->IsAssayISE(assayCode);
	if (isISE)
	{
		ise::tf::AssayTestResultQueryCond queryAssaycond;
		ise::tf::AssayTestResultQueryResp assayTestResult;
		// 根据测试项目的ID来查询结果
		queryAssaycond.__set_testItemId(db);
		if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
			|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			return std::make_pair<std::string, std::string>("", "");
		}

		for (const auto& result : assayTestResult.lstAssayTestResult)
		{
			if (!result.__isset.endTime)
			{
				continue;
			}

			if (result.__isset.resultStatusCodes && result.resultStatusCodes.find(u8"Calc.?") != std::string::npos)
			{
				continue;
			}

			finishTimeMap.insert(result.endTime);
		}
	}
	else
	{
		ch::tf::AssayTestResultQueryCond queryAssaycond;
		ch::tf::AssayTestResultQueryResp assayTestResult;
		// 根据测试项目的ID来查询结果
		queryAssaycond.__set_testItemId(db);

		if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
			|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			return std::make_pair<std::string, std::string>("", "");
		}

		for (const auto& result : assayTestResult.lstAssayTestResult)
		{
			if (!result.__isset.endTime)
			{
				continue;
			}

			if (result.__isset.resultStatusCodes && result.resultStatusCodes.find(u8"Calc.?") != std::string::npos)
			{
				continue;
			}

			finishTimeMap.insert(result.endTime);
		}

        im::tf::AssayTestResultQueryCond queryimAssaycond;
        im::tf::AssayTestResultQueryResp assayimTestResult;
        // 根据测试项目的ID来查询结果
        queryimAssaycond.__set_testItemId(db);

        if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayimTestResult, queryimAssaycond)
            || assayimTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            return std::make_pair<std::string, std::string>("", "");
        }

        for (const auto& result : assayimTestResult.lstAssayTestResult)
        {
            if (!result.__isset.endTime)
            {
                continue;
            }

            if (!result.__isset.RLU || result.RLU < 0)
            {
                continue;
            }

            finishTimeMap.insert(result.endTime);
        }
	}

	if (finishTimeMap.empty())
	{
		return { "", "" };
	}
	else if (finishTimeMap.size() == 1)
	{
		std::string firstTestTime = *(finishTimeMap.begin());
		return { firstTestTime, "" };
	}
	else 
	{
		 std::string firstTestTime = *(finishTimeMap.begin());
		 std::string lastTestTime = *(finishTimeMap.rbegin());
		 return { firstTestTime, lastTestTime };
	}
}

QString WorkpageCommon::GetSampleTypeStat(const int sampleType)
{
	if (sampleType == int(tf::HisSampleType::SAMPLE_SOURCE_EM))
	{
		return QObject::tr("急诊");
	}
	else if (sampleType == int(tf::HisSampleType::SAMPLE_SOURCE_NM))
	{
		return QObject::tr("常规");
	}
	else if (sampleType == int(tf::HisSampleType::SAMPLE_SOURCE_CL))
	{
		return QObject::tr("校准");
	}
	else if (sampleType == int(tf::HisSampleType::SAMPLE_SOURCE_QC))
	{
		return QObject::tr("质控");
	}
	else
	{
		return QObject::tr("未知");
	}
}

QVariant WorkpageCommon::GetSampleAiStatus()
{
    // TODO:
    return QPixmap(pngStatusDefault);
}

QVariant WorkpageCommon::GetItemAduitPrint(int iCheck)
{
    // 审核
    if (iCheck == 1)
    {
        return QPixmap(pngStatusVersify);
    }

    // 打印 TODO
    //if (spSampleInfo->__isset.printed && spSampleInfo->printed)
    //{
    //	auditPath = ":/Leonis/resource/image/status-print.png";
    //}

    // 默认
    return QPixmap(pngStatusVerifPrint);
}

///
/// @brief 记录删除样本的工作日志
///
/// @param[in]  sample  样本信息
///
/// @return true:删除成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月14日，新建函数
///
bool WorkpageCommon::DeleteSampleOptLog(const tf::SampleInfo& sample)
{
	// 免疫测试建议删除申请时间，描述修改为：样本号：XX或样本条码：XX----32872
	QString sampleOptLog = QObject::tr("样本号: %1或样本条码: %2")
		.arg(QString::fromStdString(sample.seqNo))
		.arg(QString::fromStdString(sample.barcode));
	AddOptLog(::tf::OperationType::type::DEL, sampleOptLog);
	return true;
}

///
/// @brief 是否显示浓度
///
/// @param[in]  sampleSet  设置信息
/// @param[in]  sampleType 样本类型
///
/// @return true:显示浓度，反之显示吸光度
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月12日，新建函数
///
bool WorkpageCommon::IsShowConc(const SampleShowSet& sampleSet, tf::SampleType::type sampleType)
{
    // 若是样本/质控（根据配置返回显示）
    if (sampleType == tf::SampleType::type::SAMPLE_TYPE_QC
        || sampleType == tf::SampleType::type::SAMPLE_TYPE_PATIENT)
    {
        return sampleSet.QCShowConc;
    }

    // 若是校准
    if (sampleType == tf::SampleType::type::SAMPLE_TYPE_CALIBRATOR)
    {
        return sampleSet.CaliShowConc;
    }

    return true;
}

///
/// @brief 根据项目通道号获取项目的小数位、和主单位倍率、单位名称
///
/// @param[in]		assayCode	项目通道号
/// @param[out]		unitName	单位名称
/// @param[out]		iPrecision	小数位
/// @param[out]		factor		和主单位倍率
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月21日，新建函数
///
void WorkpageCommon::GetPrecFactorUnit(int assayCode, int& iPrecision, double& factor, std::string& unitName)
{
    // 获取当前单位，按倍率转换并显示
    if (CommonInformationManager::IsCalcAssay(assayCode))
    {
        std::shared_ptr<::tf::CalcAssayInfo> spCalcInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
        if (spCalcInfo != Q_NULLPTR)
        {
            iPrecision = spCalcInfo->decimalDigit;
            unitName = spCalcInfo->resultUnit;
        }
    }
    else
    {
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            iPrecision = spAssayInfo->decimalPlace;
            for (const tf::AssayUnit& unit : spAssayInfo->units)
            {
                if (unit.isCurrent)
                {
                    factor = unit.factor;
                    unitName = unit.name;
                    break;
                }
            }
        }
    }
}

bool WorkpageCommon::ConfirmVerify(QPushButton* verfiyBtn)
{
    bool bStatus = (verfiyBtn->text() == QObject::tr("审核"));
    QString strNotice = bStatus ? QObject::tr("确定需要审核选中数据?") : QObject::tr("确定需要取消审核选中数据?");

    // 询问是否确定要审核
    TipDlg noticeDlg(QObject::tr("审核"), strNotice, TipDlgType::TWO_BUTTON);

    return (noticeDlg.exec() == QDialog::Accepted);
}

void WorkpageCommon::SetSampleTypeGridBtnsVisble(std::vector<QPushButton*>& sampleTypeBtns, 
    QGridLayout* gridLay, int iTargPos, bool bShow)
{
    for (QPushButton* btn : sampleTypeBtns)
    {
        btn->hide();
        gridLay->removeWidget(btn);
    }

    int iRow = 0;
    int iCol = 0;
    for (int i = 0; i < sampleTypeBtns.size(); ++i)
    {
        if (i == iTargPos && !bShow) // 包膜腔积液在列表的第3个
        {
            sampleTypeBtns[i]->setChecked(false);
            sampleTypeBtns[i]->hide();
            continue;
        }
        sampleTypeBtns[i]->show();
        gridLay->addWidget(sampleTypeBtns[i], iRow, iCol);
        if (iCol == 2) // 一行3个按钮，共2行；换行
        {
            iRow++;
            iCol = 0;
        }
        else
        {
            iCol++;
        }
    }
}

bool WorkpageCommon::IsShowConc(const SampleShowSet& sampleSet, tf::HisSampleType::type sampleType)
{
    auto funcTransHistorySampleType = [](tf::HisSampleType::type sampleType) -> tf::SampleType::type {
        switch (sampleType)
        {
        case tf::HisSampleType::SAMPLE_SOURCE_QC:
            return tf::SampleType::SAMPLE_TYPE_QC;
        case tf::HisSampleType::SAMPLE_SOURCE_CL:
            return tf::SampleType::SAMPLE_TYPE_CALIBRATOR;
        case tf::HisSampleType::SAMPLE_SOURCE_EM:
        case tf::HisSampleType::SAMPLE_SOURCE_NM:
            return tf::SampleType::SAMPLE_TYPE_PATIENT;
        default:
            break;
        }
        return tf::SampleType::SAMPLE_TYPE_PATIENT;
    };

    return IsShowConc(sampleSet, funcTransHistorySampleType(sampleType));
}

///
/// @brief 更新患者信息中的审核者信息
///
/// @param[in]  patientId  病人信息数据库主键
/// @param[in]  sampleId   样本信息数据库主键
/// @param[in]  auditName  审核者信息
///
/// @return true:修改成功
///
/// @par History:
/// @li 5774/WuHongTao，2024年6月11日，新建函数
///
bool WorkpageCommon::UpdateAduitInfo(const int64_t patientId, const int64_t sampleId, const std::string& auditName)
{
	ULOG(LOG_INFO, "%s(update patient id: %d and sample id: %d , audit name %s) ", __FUNCTION__, patientId, sampleId, auditName);

	// 添加病人信息,当病人信息主键是-1的时候
	bool ret = false;
	auto auditTimes = GetCurrentLocalTimeString();
	if (patientId <= 0)
	{
		::tf::ResultLong retLong;
		tf::PatientInfo patientInfoSave;
		patientInfoSave.__set_auditor(auditName);	
		patientInfoSave.__set_auditorTime(auditTimes);
		if (!DcsControlProxy::GetInstance()->AddPatientInfo(retLong, patientInfoSave, sampleId) || retLong.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "AddPatientInfo(%s) failed, sampelId: %d !", __FUNCTION__, sampleId);
			return ret;
		}
		return true;
	}

	::tf::PatientInfoQueryCond patienQryCond;
	::tf::PatientInfoQueryResp patienQryResp;
	patienQryCond.__set_id(patientId);
	// 执行查询条件
	if (!DcsControlProxy::GetInstance()->QueryPatientInfo(patienQryResp, patienQryCond) || patienQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "%s(error) query patient id: %d Info failed!", __FUNCTION__, patientId);
		return ret;
	}

	// 查询患者信息失败
	if (patienQryResp.lstPatientInfos.empty())
	{
		ULOG(LOG_ERROR, "%s(empty) query patient id: %d!", __FUNCTION__, patientId);
		return ret;
	}

	// 更新患者信息中的审核者信息
	tf::PatientInfo patientInfoSave;
	patientInfoSave.__set_auditor(auditName);
	patientInfoSave.__set_auditorTime(auditTimes);
	// 更新病人信息
	patientInfoSave.__set_id(patientId);
	if (!DcsControlProxy::GetInstance()->ModifyPatientInfo(patientInfoSave))
	{
		ULOG(LOG_ERROR, "ModifyPatientInfo(%s) failed, sampelId: %d !", __FUNCTION__, patientId);
		return ret;
	}

	return true;
}

bool WorkpageCommon::IsConcValid(const bool isCalc, const std::string& statusCode, const double conc)
{
	// 若无法计算，则不显示浓度
	if (statusCode.find(u8"Calc.?") != std::string::npos)
	{
		return false;
	}

	// 计算项目可以显示负数
	if (isCalc)
	{
		return true;
	}

    return (conc >= 0);
}

bool WorkpageCommon::IsConcValid(const bool isCalc, const std::string& statusCode, const std::string& concData)
{
	double conc = 0;
	if (concData.empty() || !stringutil::IsDouble(concData, conc))
	{
		return false;
	}

	return IsConcValid(isCalc, statusCode, conc);
}

bool WorkpageCommon::IsRLUValid(const int absValue)
{
	int maxAbs = DictionaryQueryManager::GetInstance()->GetChMaxAbs();
	int minAbs = maxAbs * (-1);
	if (absValue > maxAbs|| absValue < minAbs)
	{
		return false;
	}

	return true;
}

bool WorkpageCommon::IsRLUValid(const string& rluString)
{
    // 由于某些手动修改换算后可能产生小数，顾以更宽松的合法性检查
	double rlu = 0;
	if (rluString.empty() || !stringutil::IsDouble(rluString, rlu))
	{
		return false;
	}

	// 吸光度是否符合显示要求
	if (!IsRLUValid(rlu))
	{
		return false;
	}

	return true;
}
