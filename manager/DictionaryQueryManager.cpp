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

#include "DictionaryQueryManager.h"
#include "thrift/DcsControlProxy.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/track/TrackConfigSerialize.h"
#include "src/public/DictionaryDecode.h"
#include "src/public/ConfigSerialize.h"
#include "src/public/DictionaryKeyName.h"
#include "src/public/ch/ChConfigDefine.h"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ise/IseConfigDefine.h"
#include "src/public/ise/IseConfigSerialize.h"
#include "src/public/im/ImConfigDefine.h"
#include "src/public/im/ImConfigSerialize.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"

#define INVALID_VALUE -1 // 正整数值类型的未赋值默认值，用于判断未赋值

template<typename T>
bool QueryDictionaryValue(T& tValue, const std::string& keyName)
{
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(keyName);

    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "Failed to query dictionary by keyname:%s.", keyName);
        return false;
    }

    if (!DecodeJson<T>(tValue, qryResp.lstDictionaryInfos[0].value))
    {
        ULOG(LOG_ERROR, "DecodeJson inJectionModelConfig Failed, %s.", qryResp.lstDictionaryInfos[0].value.c_str());
        return false;
    }

    return true;
}

template<typename T>
bool SaveToDictinary(const T& tValue, const std::string& keyName)
{
    // 编码
    tf::DictionaryInfo dicObj;
    dicObj.__set_keyName(keyName);

    std::string strVal;
    if (!Encode2Json<T>(strVal, tValue))
    {
        ULOG(LOG_ERROR, "Failed to encode2json, at %s.", keyName.c_str());
        return false;
    }
    dicObj.__set_value(strVal);

    // 保存到字典
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(dicObj))
    {
        ULOG(LOG_ERROR, "Failed to modify dictionaryinfo.");
        return false;
    }

    return true;
}

DictionaryQueryManager::DictionaryQueryManager() : m_bCalExpireNotice(true)
{
    m_rnr = nullptr;
    m_qcDocSnLength = INVALID_VALUE; // not set.
    m_qcDocLotLength = INVALID_VALUE; // not set.
    m_qcDocBarcodeLength = INVALID_VALUE; // not set.
    m_qcDocLevel = INVALID_VALUE; // not set.
	m_chMaxAbs = DEFAULT_CH_MAX_ABS; // not set.

    REGISTER_HANDLER(MSG_ID_DICTIONARY_UPDATE, this, UpdateDictionaryReflection);
    REGISTER_HANDLER(MSG_ID_SAMPLE_SHOWSET_UPDATE, this, UpdateDictionaryReflection);
    UpdateDictionaryReflection();
}

DictionaryQueryManager::~DictionaryQueryManager()
{
}

DictionaryQueryManager* DictionaryQueryManager::GetInstance()
{
    static DictionaryQueryManager ins;
    return &ins;
}

bool DictionaryQueryManager::UpdateDictionaryReflection()
{    
    m_mapUnitType.clear(); // 清空单位映射    
    m_mapSouceType.clear(); // 清空样本源映射

    // 查询默认样本类型
    ::tf::DictionaryInfoQueryCond qryCond;
    ::tf::DictionaryInfoQueryResp qryResp;
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_WARN, "Failed to query dictionaryinfo all.");
        return false;
    }
    if (qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_WARN, "Empty dictinaryinfos.");
        return false;
    }

    // 将结果保存到项目映射中
    for (const tf::DictionaryInfo& dictionaryInfo : qryResp.lstDictionaryInfos)
    {
        // 数据字典--单位
        if (dictionaryInfo.keyName == DKN_UNIT_TYPE)
        {
            if (!DecodeJson(m_mapUnitType, dictionaryInfo.value))
            {
                ULOG(LOG_WARN, "Failed to decodejson:%s.", dictionaryInfo.value.c_str());
            }
        }
        // 样本源类型
        else if (dictionaryInfo.keyName == DKN_SAMPLE_SOURCE_TYPE)
        {
            if (!DecodeJson(m_mapSouceType, dictionaryInfo.value))
            {
                ULOG(LOG_WARN, "Failed to decodejson:%s.", dictionaryInfo.value.c_str());
            }
        }
        // 架号范围
        else if (dictionaryInfo.keyName == DKN_RACK_NUM_RANGE)
        {
            RackNumRange tempRack;
            if (!DecodeRackNumReange(dictionaryInfo.value, tempRack))
            {
                ULOG(LOG_ERROR, "Failed to execute DecodeRackNumReange()%s.", dictionaryInfo.value.c_str());
                return false;
            }
            m_rnr = std::make_shared<RackNumRange>(tempRack);
        }
        else if (dictionaryInfo.keyName == DKN_CH_BUCKET_WARNING_SET)
        {
            if (!DecodeJson(m_chWasteWarning, dictionaryInfo.value))
            {
                ULOG(LOG_ERROR, "Failed to execute DecodeJson(), %s.", dictionaryInfo.value.c_str());
                return false;
            }
        }
        // 显示设置中样本信息
        else if (dictionaryInfo.keyName == DKN_SAMPLE_DEFAULT_SHOWTYPE)
        {
            if (!DecodeJson(m_sampleShowSet, dictionaryInfo.value))
            {
                ULOG(LOG_ERROR, "Failed to execute DecodeJson(), %s.", dictionaryInfo.value.c_str());
                return false;
            }
        }
        // 显示设置中结果显示
        else if (dictionaryInfo.keyName == DKN_SAMPLE_RESULT_STATUS)
        {
            if (!DecodeJson(m_sampleResultShowSet, dictionaryInfo.value))
            {
                ULOG(LOG_ERROR, "Failed to execute DecodeJson(), %s.", dictionaryInfo.value.c_str());
                return false;
            }
        }
        // 结果提示
        else if (dictionaryInfo.keyName == DKN_RESULT_PROMPT)
        {
            if (!DecodeJson(m_resultTips, dictionaryInfo.value))
            {
                ULOG(LOG_ERROR, "Failed to execute DecodeJson(), %s.", dictionaryInfo.value.c_str());
                return false;
            }
        }
        // 备用订单
        else if (dictionaryInfo.keyName == DKN_BACKUP_ORDER)
        {
            if (!DecodeJson(m_backupOrder, dictionaryInfo.value))
            {
                ULOG(LOG_ERROR, "Failed to execute DecodeJson(), %s.", dictionaryInfo.value.c_str());
                return false;
            }
        }
        else if (dictionaryInfo.keyName == DKN_IS_PIPELINE)
        {
            if (dictionaryInfo.value == STR_VALUE_TRUE)
            {
                m_isPipeLine = true;
            }
            else if (dictionaryInfo.value == STR_VALUE_FALSE)
            {
                m_isPipeLine = false;
            }
            else
            {
                ULOG(LOG_ERROR, "invalid dictionary value:%s", dictionaryInfo.value);
                return false;
            }
        }
        else if (dictionaryInfo.keyName == DKN_CALI_LINE_EXPIRE)
        {
            m_bCalExpireNotice = (dictionaryInfo.value == STR_VALUE_TRUE);
        }
		else if (dictionaryInfo.keyName == DKN_BCYIME_TRACKCONFIG)
		{
			if (!DecodeJson(m_trackConfig, dictionaryInfo.value))
			{
				ULOG(LOG_ERROR, "Failed to execute DecodeJson(), %s.", dictionaryInfo.value.c_str());
				return false;
			}
		}
		// 生化最大吸光度
		else if (dictionaryInfo.keyName == DKN_CH_MAX_ABS)
		{
			if (!DecodeJson(m_chMaxAbs, dictionaryInfo.value))
			{
				ULOG(LOG_ERROR, "Failed to execute DecodeJson(), %s.", dictionaryInfo.value.c_str());
				return false;
			}
		}
    }

    // 更新数据地图屏蔽地图
    m_dataAlarmShiledMap.clear();
    auto alarmList = DictionaryQueryManager::GetChResultStatusCodes();
    for (const auto& alarm : alarmList)
    {
        m_dataAlarmShiledMap.insert(alarm);
    }

    return true;
}

bool DictionaryQueryManager::QueryAutoRecheckConfig()
{
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_START_TEST_AUTO_RETEST);

    // 查询自动复查设置
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "Failed query dictionary by keyname %s.", DKN_START_TEST_AUTO_RETEST);
        return false;
    }

    return qryResp.lstDictionaryInfos[0].value == STR_VALUE_TRUE;
}

int DictionaryQueryManager::GetInjectionModel()
{
    DetectionSetting detectionSetting;
    if (!GetDetectionConfig(detectionSetting))
    {
        ULOG(LOG_ERROR, "Failed to get injection model.");
        return ::tf::TestMode::INVALID_MODE;
    }

    return detectionSetting.testMode;
}

bool DictionaryQueryManager::GetDetectionConfig(DetectionSetting& detecConfg)
{
    return QueryDictionaryValue(detecConfg, DKN_DETECTION_SET_CONFIG);
}

bool DictionaryQueryManager::GetDynCalcConfig(RgntReqCalcCfg& dynCalcCfg)
{
    return QueryDictionaryValue(dynCalcCfg, DKN_UI_REQ_DYN_CALC_CFG);
}

bool DictionaryQueryManager::SaveDynCalcConfig(const RgntReqCalcCfg& dynCalcCfg)
{
    return SaveToDictinary(dynCalcCfg, DKN_UI_REQ_DYN_CALC_CFG);
}

bool DictionaryQueryManager::GetExportConfig(std::map<ExportType, std::set<ExportInfoEn>>& exportCfg)
{
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_EXPORT_SET);
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "Failed query dictionary by keyname:%s.", DKN_EXPORT_SET);
        return false;
    }

    exportCfg.clear();
    if (!DecodeJson(exportCfg, qryResp.lstDictionaryInfos[0].value))
    {
        ULOG(LOG_ERROR, "DecodeJson Exportset Failed,%s.", qryResp.lstDictionaryInfos[0].value.c_str());
        return false;
    }

    // 视觉识别设置需要屏蔽，此处特殊处理（后续如果实现则放开）
    auto it = exportCfg.find(IM_ALL_SAMPLE);
    if (it != exportCfg.end())
    {
        it->second.erase(EIE_IM_AI_RESULT);
        it->second.erase(EIE_IM_RE_AI_RESULT);
    }

    return true;
}

bool DictionaryQueryManager::SaveExportConfig(const std::map<ExportType, std::set<ExportInfoEn>>& exportCfg)
{
    return SaveToDictinary(exportCfg, DKN_EXPORT_SET);
}

bool DictionaryQueryManager::GetSystemUUID(std::string& strUUID)
{
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_SYSTEM_UUID);

    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "Failed query dictionary by kename:%s.", DKN_SYSTEM_UUID);
        return false;
    }

    strUUID = qryResp.lstDictionaryInfos[0].value;
    return true;
}

bool DictionaryQueryManager::GetChSerumIndexFactor(std::vector<int>& seriFactor)
{
    ::tf::DictionaryInfoQueryCond qryCond;
    ::tf::DictionaryInfoQueryResp qryResp;
    qryCond.__set_keyName(DKN_CH_SERUM_INDEX_FACTOR);

    // 查询默认样本类型
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond) || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Failed query dictionary by kename:%s.", DKN_CH_SERUM_INDEX_FACTOR);
        return false;
    }
    // 判断结果是否为空
    if (qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_WARN, "Empty query result.");
        return false;
    }

    seriFactor.clear();
    return DecodeSerumIndexFactors(seriFactor, qryResp.lstDictionaryInfos[0].value);
}

bool DictionaryQueryManager::SaveChSerumIndexFactor(const std::vector<int>& seriFactor)
{
    // 系数
    tf::DictionaryInfo dicTemp;
    dicTemp.__set_keyName(DKN_CH_SERUM_INDEX_FACTOR);

    std::string ss;
    EncodedSerumIndexFactors(ss, seriFactor);
    dicTemp.__set_value(ss);

    // 修改数据字典
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(dicTemp))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo() Failed!");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::GetItemTestOrder(OrderAssay& assayOrder, bool isCh)
{
    std::string strKey = (isCh ? DKN_CH_ITEM_TEST_ORDER : DKN_IM_ITEM_TEST_ORDER);
    return QueryDictionaryValue(assayOrder, strKey);
}

bool DictionaryQueryManager::SaveItemTestOrder(const OrderAssay& assyaOrder, bool isCh)
{
    std::string strKey = isCh ? DKN_CH_ITEM_TEST_ORDER : DKN_IM_ITEM_TEST_ORDER;
    return SaveToDictinary(assyaOrder, strKey);
}

bool DictionaryQueryManager::GetAssayShowOrder(OrderAssay& assayOrder)
{
    return QueryDictionaryValue(assayOrder, DKN_DISPLAY_ITEM_ORDER);
}

bool DictionaryQueryManager::SaveAssayShowOrder(const OrderAssay& assayOrder)
{
    return SaveToDictinary(assayOrder, DKN_DISPLAY_ITEM_ORDER);
}

bool DictionaryQueryManager::GetWarnsSet(SystemOverviewWarnSet& warnSet)
{
    return QueryDictionaryValue(warnSet, DKN_SYSTEM_OVERVIEW_WARN_SET);
}

bool DictionaryQueryManager::SaveWarnsSet(const SystemOverviewWarnSet& warnSet)
{
    return SaveToDictinary(warnSet, DKN_SYSTEM_OVERVIEW_WARN_SET);
}

bool DictionaryQueryManager::GetSampleProbeWashConfigCh(SampleProbeWashConfig& washConfig)
{
    return QueryDictionaryValue(washConfig, DKN_SAMPLE_PROBE_WASH);
}

bool DictionaryQueryManager::GetSampleProbeWashConfigIm(ImSampleProbeWashConfig& washConfig)
{
    return QueryDictionaryValue(washConfig, DKN_IM_SAMPLE_PROBE_WASH);
}

bool DictionaryQueryManager::SaveSampleProbeWashConfigCh(const SampleProbeWashConfig& washConfig)
{
    return SaveToDictinary(washConfig, DKN_SAMPLE_PROBE_WASH);
}

bool DictionaryQueryManager::SaveSampleProbeWashConfigIm(const ImSampleProbeWashConfig& washConfig)
{
    return SaveToDictinary(washConfig, DKN_IM_SAMPLE_PROBE_WASH);
}

bool DictionaryQueryManager::SaveDetectionConfig(const DetectionSetting& detecConfg)
{
    return SaveToDictinary(detecConfg, DKN_DETECTION_SET_CONFIG);
}

bool DictionaryQueryManager::GetSampleRackAllocationConfig(SampleRackAllocation& rackConfig)
{
    return QueryDictionaryValue(rackConfig, DKN_SAMPLE_RACK_ALLOCATION);
}

bool DictionaryQueryManager::SaveSampleRackAllocationConfig(const SampleRackAllocation& rackConfig)
{
    return SaveToDictinary(rackConfig, DKN_SAMPLE_RACK_ALLOCATION);
}

bool DictionaryQueryManager::GetSampleRecvModelConfig(ConfigSampleRecvMode& sampleRecv)
{
    return QueryDictionaryValue(sampleRecv, DKN_ConfigSampleRecvMode);
}

bool DictionaryQueryManager::SaveSampleRecvModelConfig(const ConfigSampleRecvMode& sampleRecv)
{
    // 保存样本架分配
    return SaveToDictinary(sampleRecv, DKN_ConfigSampleRecvMode);
}

bool DictionaryQueryManager::GetMaintainShowConfig(MaintainShowSet& maintShowConfig)
{
    return QueryDictionaryValue(maintShowConfig, DKN_MAINTAIN_SHOW_SET);
}

bool DictionaryQueryManager::SaveMaintainShowConfig(const MaintainShowSet& maintShowConfig)
{
    // 保存显示信息设置
    return SaveToDictinary(maintShowConfig, DKN_MAINTAIN_SHOW_SET);
}

bool DictionaryQueryManager::GetElectrodeCleanSet(ElectrodeCleanSet& electrodeCleanSet)
{
    return QueryDictionaryValue(electrodeCleanSet, DKN_ELECTRODE_CLEAN_SET);
}

bool DictionaryQueryManager::SaveElectrodeCleanSet(const ElectrodeCleanSet& electrodeCleanSet)
{
    // 保存电极清洗设置
    return SaveToDictinary(electrodeCleanSet, DKN_ELECTRODE_CLEAN_SET);
}

bool DictionaryQueryManager::GetFunctionManageConfig(std::map<std::string, std::vector<FunctionManageItem>>& functionManage)
{
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_FUNCTIONC_MANAGE_INFO);

    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "Failed to query dictionary by %s.", DKN_FUNCTIONC_MANAGE_INFO);
        return false;
    }

    if (!qryResp.lstDictionaryInfos[0].__isset.value)
    {
        ULOG(LOG_INFO, "ClearSetData Xml String Is Empty !");
        return false;
    }

    functionManage.clear();
    if (!DecodeJson(functionManage, qryResp.lstDictionaryInfos[0].value))
    {
        ULOG(LOG_ERROR, "DecodeJson ClearSetData Failed, %s.", qryResp.lstDictionaryInfos[0].value.c_str());
        return false;
    }

    return true;
}

bool DictionaryQueryManager::SaveFunctionManageConfig(const std::map<std::string, std::vector<FunctionManageItem>>& functionManage)
{
    return SaveToDictinary(functionManage, DKN_FUNCTIONC_MANAGE_INFO);
}

bool DictionaryQueryManager::GetWasteBucketWarningValueCh(int& iChWasteWarnValue)
{
    return QueryDictionaryValue(iChWasteWarnValue, DKN_CH_BUCKET_WARNING_SET);
}

bool DictionaryQueryManager::SaveWasteBucketWarningValueCh(int iChWasteWarnValue)
{
	if (iChWasteWarnValue == m_chWasteWarning)
	{
		ULOG(LOG_INFO, "old chWasteWarning is equal to new one, no need update!");
		return true;
	}

    if (!SaveToDictinary(iChWasteWarnValue, DKN_CH_BUCKET_WARNING_SET))
    {
        return false;
    }
	m_chWasteWarning = iChWasteWarnValue;

	POST_MESSAGE(MSG_ID_BUCKET_SET_UPDATE);

    return true;
}

bool DictionaryQueryManager::GetAiAnalysisConfig(AiAnalysisCfg& aiAnaysis)
{
    return QueryDictionaryValue(aiAnaysis, DKN_AI_ANALYSIS_SET);
}

bool DictionaryQueryManager::SaveAiAnalysisConfig(const AiAnalysisCfg& aiAnaysis)
{
    return SaveToDictinary(aiAnaysis, DKN_AI_ANALYSIS_SET);
}

bool DictionaryQueryManager::GetCfgToolImParamConfig(CfgToolImParam& cfgToolImParam)
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_I6000_CONFIG_PARAM);

    // 查询
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return false;
    }

    // 解析
    if (!DecodeJson(cfgToolImParam, qryResp.lstDictionaryInfos[0].value))
    {
        ULOG(LOG_ERROR, "DecodeJson ExportSet Failed");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::SaveCfgToolImParamConfig(const CfgToolImParam& cfgToolImParam)
{
    // 编码并保存
    tf::DictionaryInfo di;
    di.__set_keyName(DKN_I6000_CONFIG_PARAM);

    // 设置数据
    std::string xml;
    if (!Encode2Json(xml, cfgToolImParam))
    {
        return false;
    }
    di.__set_value(xml);

    // 添加数据字典信息
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        ULOG(LOG_WARN, "ModifyDictionaryInfo Failed");
        tf::ResultLong ret;
        return DcsControlProxy::GetInstance()->AddDictionaryInfo(di, ret);
    }

    return true;
}

bool DictionaryQueryManager::GetClearSet(ClearSetData& clearConfig)
{
    return QueryDictionaryValue(clearConfig, DKN_CLEAR_SET);
}

bool DictionaryQueryManager::SaveClearSet(const ClearSetData& clearConfig)
{
    return SaveToDictinary(clearConfig, DKN_CLEAR_SET);
}

bool DictionaryQueryManager::GetSoftLanguageConfig(std::vector<SoftLanguage>& softLanguages)
{
    return QueryDictionaryValue(softLanguages, DKN_SOFT_LANGUAGES);
}

bool DictionaryQueryManager::GetAuditConfig(bool& bEnableAudit)
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_ENABLE_AUDIT);

    // 查询显示设置
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return false;
    }
    bEnableAudit = (qryResp.lstDictionaryInfos[0].value == STR_VALUE_TRUE);

    return true;
}

bool DictionaryQueryManager::SaveAuditConfig(bool bEnableAudit)
{
    // 组装修改的数据
    tf::DictionaryInfo di;
    di.__set_keyName(DKN_ENABLE_AUDIT);

    std::string value = bEnableAudit ? STR_VALUE_TRUE : STR_VALUE_FALSE;
    di.__set_value(value);

    // 保存样本架分配
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::GetAutoLoginUser(std::string& strUser)
{
    // 获取自动登录用户
    ::tf::DictionaryInfoQueryResp dicResp;;
    ::tf::DictionaryInfoQueryCond dicCond;
    dicCond.__set_keyName(DKN_AUTO_LOGIN_USER);
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(dicResp, dicCond))
    {
        ULOG(LOG_ERROR, "%s(), QueryDictionaryInfo() failed", __FUNCTION__);
        return false;
    }

    // 结果为空则返回
    if (dicResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_WARN, "%s(), dicResp.lstDictionaryInfos.empty()", __FUNCTION__);
        return false;
    }
    strUser = dicResp.lstDictionaryInfos[0].value;

    return true;
}

bool DictionaryQueryManager::SaveAutoLoginUser(const std::string& strUser)
{
    tf::DictionaryInfo dicInfo;
    dicInfo.__set_keyName(DKN_AUTO_LOGIN_USER);
    dicInfo.__set_value(strUser);

    // 修改自动登录用户
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(dicInfo))
    {
        ULOG(LOG_ERROR, "%s(), ModifyDictionaryInfo() failed", __FUNCTION__);
        return false;
    }

    return true;
}

bool DictionaryQueryManager::GetBatchAddSampleConfig(AddSampleAction& batchConfig)
{
    return QueryDictionaryValue(batchConfig, DKN_BATCH_ADD_SAMPLE);
}

bool DictionaryQueryManager::SaveBatchAddSampleConfig(const AddSampleAction& batchConfig)
{
    return SaveToDictinary(batchConfig, DKN_BATCH_ADD_SAMPLE);
}

bool DictionaryQueryManager::GetCustomSetData(int dataType, std::vector<CustomSetRowData>& cutomData)
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;

    std::string key = QString("DataDict_%1").arg(dataType).toStdString();
    qryCond.__set_keyName(key);

    // 查询显示设置
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return false;
    }
    if (qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_INFO, "QueryDictionaryInfo Result Is Empty");
        return false;
    }

    // 解析字符串
    cutomData.clear();
    if (!DecodeJson(cutomData, qryResp.lstDictionaryInfos[0].value))
    {
        ULOG(LOG_ERROR, "DecodeJson CustomSetData Failed");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::GetUiBaseSet(BaseSet& baseSet)
{
    // 初始化
    baseSet.Clear();
    return QueryDictionaryValue(baseSet, DKN_BASE_SET);
}

bool DictionaryQueryManager::SaveUiBaseSet(const BaseSet& baseSet)
{
    return SaveToDictinary(baseSet, DKN_BASE_SET);
}

bool DictionaryQueryManager::GetUiDispalySet(std::vector<DisplaySet>& vecDisplays)
{
    return QueryDictionaryValue(vecDisplays, DKN_DISPALY_SET);
}

bool DictionaryQueryManager::SaveUiDisplaySet(const std::vector<DisplaySet>& vecDisplays)
{
    return SaveToDictinary(vecDisplays, DKN_DISPALY_SET);
}

std::string DictionaryQueryManager::GetCurrentLanuageType()
{
    BaseSet ds;
    if (GetUiBaseSet(ds))
    {
        return ds.languageType;
    }
    return "";
}

int DictionaryQueryManager::GetTimeDisplayMode()
{
    BaseSet ds;
    if (!GetUiBaseSet(ds))
    {
        return -1;
    }
    return ds.iTimeForm;
}

QString DictionaryQueryManager::GetUIDateFormat()
{
    BaseSet ds;
    if (GetUiBaseSet(ds))
    {
        return QString::fromStdString(ds.strDateForm);
    }

    // 默认值
    return UI_DATE_FORMAT;
}

QStringList DictionaryQueryManager::GetChResultStatusCodes()
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_CH_SHIELD_RESULT_STATUS_CODE);

    // 查询显示设置
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.size() == 0
        || qryResp.lstDictionaryInfos[0].value.empty())
    {
        return QStringList();
    }

    return QString::fromStdString(qryResp.lstDictionaryInfos[0].value).split(",");
}

bool DictionaryQueryManager::TakeBackupOrderConfig(BackupOrderConfig& bkOrder)
{
    // 查询默认设置
    return QueryDictionaryValue(bkOrder, DKN_BACKUP_ORDER);
}

bool DictionaryQueryManager::SaveBackupOrderConfigAct(BackupOrderConfig& bkOrder)
{
    // 保存样本架分配
    return SaveToDictinary(bkOrder, DKN_BACKUP_ORDER);
}

bool DictionaryQueryManager::SaveChResultStatusCodes(const QStringList& codes)
{
    QString strCodes = codes.join(',');

    ::tf::DictionaryInfo dic;
    dic.__set_keyName(DKN_CH_SHIELD_RESULT_STATUS_CODE);
    dic.__set_value(strCodes.toStdString());

    bool isSucced = DcsControlProxy::GetInstance()->ModifyDictionaryInfo(dic);
    if (isSucced)
    {
        POST_MESSAGE(MSG_ID_DICTIONARY_UPDATE);
    }

    return isSucced;
}

bool DictionaryQueryManager::GetCommParamSet(CommParamSet& communicateParamSet)
{
    // 查询显示设置
    return QueryDictionaryValue(communicateParamSet, DKN_COMM_PARAM_SET);
}

bool DictionaryQueryManager::SaveCommParamSet(const CommParamSet& communicateParamSet)
{
    return SaveToDictinary(communicateParamSet, DKN_COMM_PARAM_SET);
}

bool DictionaryQueryManager::SaveAutoRecheckConfig(bool bAutoRetest)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    tf::DictionaryInfo di;
    di.__set_keyName(DKN_START_TEST_AUTO_RETEST);
    di.__set_value(bAutoRetest ? STR_VALUE_TRUE : STR_VALUE_FALSE);

    // 修改自动复查设置
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo AutoRecheckConfig Failed");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::SaveStarttestMaintainConfig(bool bBMainte, const std::string& strData)
{
    tf::DictionaryInfo di;
    di.__set_keyName(DKN_START_TEST_MAINTAIN);

    std::string strChecked = bBMainte ? STR_VALUE_TRUE : STR_VALUE_FALSE;
    di.__set_value(strChecked + "," + strData);

    // 修改自动复查设置
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo maintain config failed");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::GetStarttestMaintainConfig(bool &bSMaintain, std::string& strdata)
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_START_TEST_MAINTAIN);

    // 查询自动复查设置
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return false;
    }

    auto valList = QString::fromStdString(qryResp.lstDictionaryInfos[0].value).split(",");
    if (valList.size() < 2)
    {
        ULOG(LOG_WARN, "Invalid starttest maintain config string.");
        return false;
    }
    bSMaintain = (valList[0] != STR_VALUE_FALSE);
    strdata = valList[1].toStdString();

    return true;
}

bool DictionaryQueryManager::GetShowLowerAlarmConfig(bool& isShowLowerAlarm)
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_LOWER_ALARM);

    // 查询
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty()
        || !qryResp.lstDictionaryInfos[0].__isset.value)
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return false;
    }

    isShowLowerAlarm = (qryResp.lstDictionaryInfos[0].value == STR_VALUE_TRUE);
    return true;
}

bool DictionaryQueryManager::UpdateLowerAlarmStatus(bool bCheck)
{
    tf::DictionaryInfo di;
    di.__set_keyName(DKN_LOWER_ALARM);
    di.__set_value(bCheck ? STR_VALUE_TRUE : STR_VALUE_FALSE);

    return DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di);
}

bool DictionaryQueryManager::GetAlarmMusicConfig(AlarmMusic& alarmMus)
{
    // 查询报警条件
    return QueryDictionaryValue(alarmMus, DKN_ALARM_MUSIC);
}

bool DictionaryQueryManager::SaveAlarmMusicConfig(const AlarmMusic& alarmMus)
{
    // 保存报警条件
    return SaveToDictinary(alarmMus, DKN_ALARM_MUSIC);
}

bool DictionaryQueryManager::GetCaliLineExpire(bool& caliLineExpire)
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_CALI_LINE_EXPIRE);

    // 查询报警条件设置
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return false;
    }

    caliLineExpire = qryResp.lstDictionaryInfos[0].value == STR_VALUE_TRUE;

    return true;
}

bool DictionaryQueryManager::SaveCaliLineExpire(bool caliLineExpire)
{
    // 组装修改的数据
    tf::DictionaryInfo di;
    di.__set_keyName(DKN_CALI_LINE_EXPIRE);
    di.__set_value(caliLineExpire ? STR_VALUE_TRUE : STR_VALUE_FALSE);

    // 保存报警条件
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::GetPageset(PageSet& pgSet)
{
    return QueryDictionaryValue(pgSet, DKN_WORKPAGE_SET);
}

bool DictionaryQueryManager::SavePageset(const PageSet& pgSet)
{
    return SaveToDictinary(pgSet, DKN_WORKPAGE_SET);
}

bool DictionaryQueryManager::SaveAlarmShieldConfig(const AlarmShieldInfo& shiledCfg)
{
    return SaveToDictinary(shiledCfg, DKN_ALARM_SHIELD_SET);
}

bool DictionaryQueryManager::GetAlarmShieldConfig(AlarmShieldInfo& alarmShiledCfg)
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_ALARM_SHIELD_SET);

    // 查询
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty()
        || !qryResp.lstDictionaryInfos[0].__isset.value)
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return false;
    }

    alarmShiledCfg.mapCodeAndDev.clear();
    if (!DecodeJson(alarmShiledCfg, qryResp.lstDictionaryInfos[0].value))
    {
        ULOG(LOG_ERROR, "DecodeJson AlarmShield Failed");
        return false;
    }

    return true;
}

bool DictionaryQueryManager::SaveSampleDefaultShowType(const std::vector<int>& defautShow)
{
    if (!SaveToDictinary(defautShow, DKN_SAMPLE_DEFAULT_SHOWTYPE))
    {
        return false;
    }
    DictionaryQueryManager::GetInstance()->m_sampleShowSet = defautShow;

    return true;
}

bool DictionaryQueryManager::SaveSampleShowSet(const SampleShowSet& showSet)
{
    if (!SaveToDictinary(showSet, DKN_SAMPLE_RESULT_STATUS))
    {
        return false;
    }
    DictionaryQueryManager::GetInstance()->m_sampleResultShowSet = showSet;

    return true;
}

bool DictionaryQueryManager::SaveResultTips(const ResultPrompt& resTip)
{
    if (!SaveToDictinary(resTip, DKN_RESULT_PROMPT))
    {
        return false;
    }
    DictionaryQueryManager::GetInstance()->m_resultTips = resTip;

    return true;
}

int DictionaryQueryManager::GetQcSnLengthConfig()
{
    if (m_qcDocSnLength == INVALID_VALUE)
    {
        m_qcDocSnLength = DcsControlProxy::GetInstance()->GetQcConfigLengthSn();
    }

    return m_qcDocSnLength;
}

int DictionaryQueryManager::GetQcLotLengthConfig()
{
    if (m_qcDocLotLength == INVALID_VALUE)
    {
        m_qcDocLotLength = DcsControlProxy::GetInstance()->GetQcConfigLengthLot();
    }

    return m_qcDocLotLength;
}

int DictionaryQueryManager::GetQcBarcodeLengthConfig()
{
    if (m_qcDocBarcodeLength == INVALID_VALUE)
    {
        m_qcDocBarcodeLength = DcsControlProxy::GetInstance()->GetQcBarcodeLength();
    }

    return m_qcDocBarcodeLength;
}

int DictionaryQueryManager::GetQcLevelConfig()
{
    if (m_qcDocLevel == INVALID_VALUE)
    {
        m_qcDocLevel = DcsControlProxy::GetInstance()->GetQcLevelLimit();
    }

    return m_qcDocLevel;
}

std::string  DictionaryQueryManager::GetSoftwareVersion()
{
    tf::DictionaryInfoQueryCond queryCond;
    tf::DictionaryInfoQueryResp queryResp;
    queryCond.__set_keyName(DKN_SYSTEM_SOFTWARE_VERSION);

    if (DcsControlProxy::GetInstance()->QueryDictionaryInfo(queryResp, queryCond)
        && queryResp.__isset.result
        && queryResp.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS
        && queryResp.lstDictionaryInfos.size() > 0)
    {
        return queryResp.lstDictionaryInfos[0].value;
    }

    return std::string("");
}

const std::set<std::string>& DictionaryQueryManager::GetImAlaramDataCodeExcept()
{
    if (m_imResultAlarmCodeExcept.size() > 0)
    {
        return m_imResultAlarmCodeExcept;
    }

    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_UI_IMEXCEPT_RERUNALARMDATACODE);

    // 如果存在配置，则使用配置的值
    if (DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        && qryResp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        && qryResp.lstDictionaryInfos.size() > 0)
    {
        std::string strConfig = qryResp.lstDictionaryInfos[0].value;
        std::vector<std::string> vecCurStatu;
        boost::split(vecCurStatu, strConfig, boost::is_any_of(";"));

        if (vecCurStatu.size() > 0)
        {
            m_imResultAlarmCodeExcept.clear();
            for (const std::string& strCode : vecCurStatu)
            {
                m_imResultAlarmCodeExcept.insert(strCode);
            }
        }
    }
    
    // 如果不存在配置，则使用默认值
    if (m_imResultAlarmCodeExcept.size() == 0)
    {
        std::set<std::string> exceptCode = { "QC.Exclude", "Ctrl.E", "Cali.F", "Cali.I",
            "Reag.I", "Reag.DF", "Reag.F", "Reag.B", "Reag.S",
            "Dilu.S", "Dilu.DF", "Dilu.F", "Cupload.F", "Samp.I",
            "Substrate.F", "Substrate.B", "Samp.CE", "Samp.F", "Samp.DF", "MIXStp" };
        m_imResultAlarmCodeExcept = exceptCode;
    }

    return m_imResultAlarmCodeExcept;
}
