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
/// @file     CommonInformationManager.cpp
/// @brief    项目编号管理器
///
/// @author   4170/TangChuXian
/// @date     2020年10月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年10月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CommonInformationManager.h"
#include <algorithm>
#include <QStringList>
#include <QSet>
#include <QMessageBox>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/ReagentCommon.h"
#include "shared/ThriftEnumTransform.h"
#include "manager/DictionaryQueryManager.h"
#include "src/public/DictionaryDecode.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "uicommon.h"

#define UI_ASSAYPAGEMAX 3					/// 默认项目选择页面为3
#define UI_ASSAYMAXPOSTION 300				/// 默认最大位置号
Q_DECLARE_METATYPE(ch::tf::ReagentGroup);
Q_DECLARE_METATYPE(im::tf::ReagentInfoTable);
Q_DECLARE_METATYPE(ise::tf::SuppliesInfo);

bool KeyData::IsCaliBrateAbnormal()
{
    if (!m_reagent.isValid())
    {
        return false;
    }

    // 生化
    if (m_reagent.canConvert<ch::tf::ReagentGroup>())
    {
        auto chReagent = m_reagent.value <ch::tf::ReagentGroup>();
        return IsCaliStatusAbnormal(chReagent);
    }

    // 免疫
    if (m_reagent.canConvert<im::tf::ReagentInfoTable>())
    {
        auto imReagent = m_reagent.value <im::tf::ReagentInfoTable>();
        return IsCaliStatusAbnormal(imReagent);
    }

    return false;
}

bool KeyData::IsReagentAbnormal()
{
    if (!m_reagent.isValid())
    {
        return false;
    }

    // 生化
    if (m_reagent.canConvert<ch::tf::ReagentGroup>())
    {
        auto chReagent = m_reagent.value <ch::tf::ReagentGroup>();
        return IsReagentAbNormal(chReagent);
    }

    // 免疫
    if (m_reagent.canConvert<im::tf::ReagentInfoTable>())
    {
        auto imReagent = m_reagent.value <im::tf::ReagentInfoTable>();
        return IsReagentAbNormal(imReagent);
    }

    return false;
}

int64_t KeyData::Getid()
{
    if (!m_reagent.isValid())
    {
        return -1;
    }

    // 生化
    if (m_reagent.canConvert<ch::tf::ReagentGroup>())
    {
        auto chReagent = m_reagent.value <ch::tf::ReagentGroup>();
        return chReagent.id;
    }

    // 免疫
    if (m_reagent.canConvert<im::tf::ReagentInfoTable>())
    {
        auto imReagent = m_reagent.value <im::tf::ReagentInfoTable>();
        return imReagent.id;
    }

    // ISE
    if (m_reagent.canConvert<ise::tf::SuppliesInfo>())
    {
        auto supplyInfoIse = m_reagent.value <ise::tf::SuppliesInfo>();
        return supplyInfoIse.id;
    }

    return -1;
}

std::string KeyData::GetDeviceSn()
{
	if (!m_reagent.isValid())
	{
		return "";
	}

	// 生化
	if (m_reagent.canConvert<ch::tf::ReagentGroup>())
	{
		auto chReagent = m_reagent.value <ch::tf::ReagentGroup>();
		return chReagent.deviceSN;
	}

	// 免疫
	if (m_reagent.canConvert<im::tf::ReagentInfoTable>())
	{
		auto imReagent = m_reagent.value <im::tf::ReagentInfoTable>();
		return imReagent.deviceSN;
	}

	return "";
}

int64_t KeyData::GetCode()
{
    if (!m_reagent.isValid())
    {
        return -1;
    }

    // 生化
    if (m_reagent.canConvert<ch::tf::ReagentGroup>())
    {
        auto chReagent = m_reagent.value <ch::tf::ReagentGroup>();
        return chReagent.assayCode;
    }

    // 免疫
    if (m_reagent.canConvert<im::tf::ReagentInfoTable>())
    {
        auto imReagent = m_reagent.value <im::tf::ReagentInfoTable>();
        return imReagent.assayCode;
    }

    return -1;
}

bool KeyIseDate::IsCaliBrateAbnormal()
{
    if (m_reagent.canConvert<ise::tf::SuppliesInfo>())
    {
        auto supplyInfoIse = m_reagent.value <ise::tf::SuppliesInfo>();
        auto spModule = CommonInformationManager::GetInstance()->GetIseModuleMaps(supplyInfoIse);
        // 找不到耗材对于的模快的信息，肯定异常
        if (spModule == Q_NULLPTR)
        {
            return true;
        }

        // 获取当前模块状态
        bool isAbnormal = IsCaliStatusAbnormal(spModule);
        if (isAbnormal)
        {
            return true;
        }

        return false;
    }

    return true;
}


bool KeyIseDate::IsReagentAbnormal()
{
    if (m_reagent.canConvert<ise::tf::SuppliesInfo>())
    {
        auto supplyInfoIse = m_reagent.value <ise::tf::SuppliesInfo>();
        auto spModule = CommonInformationManager::GetInstance()->GetIseModuleMaps(supplyInfoIse);
        // 找不到耗材对于的模快的信息，肯定异常
        if (spModule == Q_NULLPTR)
        {
            return true;
        }

		// 是否被卸载
		bool isUnload = (supplyInfoIse.placeStatus == tf::PlaceStatus::type::PLACE_STATUS_UNLOAD);
		// 耗材被卸载的处理
		if (isUnload)
		{
			// 若之前未存储此耗材信息，说明耗材上报有误，记录日志,不影响之前状态
			auto iterSupplyInfo = m_statusMap.reagentMap.find(supplyInfoIse.id);
			if (iterSupplyInfo == m_statusMap.reagentMap.end())
			{
				ULOG(LOG_WARN, "%s(), ise supply info have not been recorded , the key id: %d", __FUNCTION__, supplyInfoIse.id);
				return false;
			}
			// 清除卸载的耗材信息
			else
			{
				m_statusMap.reagentMap.erase(iterSupplyInfo);
			}
		}

        // 获取当前耗材的状态
        bool isAbnormal = IsReagentAbNormal(supplyInfoIse, spModule);
        // ise耗材中（缓冲液和内标液只要有一瓶异常，这个组就一定异常，不需要继续判断）
        if (isAbnormal)
        {
            return true;
        }
        // 当其中一瓶正常的时候，需要找相对应的另外一瓶看是否正常，若正常，则这一组正常，否则异常
        else
        {
            for (auto& reagent : m_statusMap.reagentMap)
            {
                // 转换不成功，下一个
                if (!m_reagent.canConvert<ise::tf::SuppliesInfo>())
                {
                    continue;
                }

                auto supplyInfo = m_reagent.value <ise::tf::SuppliesInfo>();

                // 设备是否一致
                if (supplyInfoIse.deviceSN != supplyInfo.deviceSN)
                {
                    continue;
                }

                // 模块编号是否一致
                if (supplyInfoIse.moduleIndex != supplyInfo.moduleIndex)
                {
                    continue;
                }

                // 是否配对耗材
                if (supplyInfo.pos == GetOtherType(supplyInfoIse.pos))
                {
                    continue;
                }

                // 判断当前耗材是异常
                return IsReagentAbNormal(supplyInfo, spModule);
            }
        }
    }

    return true;
}

std::string KeyIseDate::GetDeviceSn()
{
	if (m_reagent.canConvert<ise::tf::SuppliesInfo>())
	{
		auto supplyInfoIse = m_reagent.value <ise::tf::SuppliesInfo>();
		return supplyInfoIse.deviceSN;
	}

	return "";
}

QSystemMonitor::QSystemMonitor() : m_bImRgntAbnUpdateRealTime(false)
{
    qRegisterMetaType<ch::tf::ReagentGroup>("ChReagentGroup");
    qRegisterMetaType<im::tf::ReagentInfoTable>("ImReagentGroup");
    qRegisterMetaType<ise::tf::SuppliesInfo>("IseSupplyInfo");

    // 耗材更新消息(此处必须使用此消息，而不能使用试剂盘处计算后消息的原因是因为，工作页面要统计所有设备的试剂消息，而试剂页面只统计当前设备)
    REGISTER_HANDLER(MSG_ID_REAGENT_INFO_UPDATE, this, OnUpdateSupplyForReagent);
    // 监听免疫试剂更新信息
    REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, OnUpdateIMSupplyForReagent);
    // ISE耗材更新消息
    REGISTER_HANDLER(MSG_ID_ISE_INFO_UPDATE, this, OnUpdateSupplyForISE);
    // 监听检测模式改变
    REGISTER_HANDLER(MSG_ID_DETECT_MODE_UPDATE, this, OnDetectModeChanged);
	// 项目遮蔽信息
	REGISTER_HANDLER(MSG_ID_ASSAY_MASK_UPDATE, this, OnShiledAssay);
	// 设备被屏蔽或者打开
	REGISTER_HANDLER(MSG_ID_DEVICE_MASK_UPDATE, this, OnUpdateDeviceAssayStatus);
}

std::vector<std::shared_ptr<KeyData>> QSystemMonitor::GetReagentGroup(int assayCode, int type)
{
    std::vector<std::shared_ptr<KeyData>> dataVec;
    // 免疫
    if (type == tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE)
    {
        for (const auto& reagent : GetImReagentGroupByAssayCode(assayCode))
        {
            QVariant innerData;
            innerData.setValue(reagent);
            dataVec.push_back(std::make_shared<KeyData>(innerData));
        }

    }
    // 生化
    else if (type == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
    {
        for (const auto& reagent : GetReagentGroupByAssayCode(assayCode))
        {
            QVariant innerData;
            innerData.setValue(reagent);
            dataVec.push_back(std::make_shared<KeyData>(innerData));
        }

    }
    // ise
    else if (type == tf::AssayClassify::type::ASSAY_CLASSIFY_ISE)
    {
        for (const auto& supply : GetISESupplyByAssayCode(assayCode))
        {
            QVariant innerData;
            innerData.setValue(supply);
            dataVec.push_back(std::make_shared<KeyIseDate>(innerData));
        }
    }

    return dataVec;
}

bool QSystemMonitor::ISAssayShiled(AssayStatus& status, bool checkReagent)
{
	int assayCode = status.assayCode;
    ULOG(LOG_INFO, "%s(assyaCode : %d)", __FUNCTION__, assayCode);

    // 计算是否包含项目编号的表达式
    auto isHaveCode = [](std::string assayCodeString, int assayCode)->bool{
        if (assayCodeString.empty() || assayCode < 0)
        {
            return false;
        }

        std::map<int, std::set<int>> assayCodes;
		DecodeJson(assayCodes, assayCodeString);
        if (assayCodes.empty())
        {
            return false;
        }

		int transferCode = assayCode;
		if (CommonInformationManager::GetInstance()->IsAssayISE(assayCode))
		{
			transferCode = ::ise::tf::g_ise_constants.ASSAY_CODE_ISE;
		}

		// 血清指数
		if (CommonInformationManager::GetInstance()->IsAssaySIND(assayCode))
		{
			transferCode = ::ch::tf::g_ch_constants.ASSAY_CODE_SIND;
		}

		for (const auto& subMap : assayCodes)
		{
			auto iter = subMap.second.find(transferCode);
			if (subMap.second.end() == iter)
			{
				return false;
			}
		}

		return true;
    };

    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (spAssay == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(can not find assyaCode : %d)", __FUNCTION__, assayCode);
        return false;
    }

	// 设备数目（项目类型），对应项目类型可能在多台设备上存在
	int deviceNumber = 0;
	int deviceReagentAbnormalNumber = 0;
	bool isPreRet = false;
    // 搜索所有设备查看项目是否被遮蔽
	auto deviceMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (const auto & deviceInfo : deviceMap)
    {
		// 只有同类型的设备能遮蔽相同类型的项目
		if (spAssay->assayClassify != deviceInfo.second->deviceClassify)
		{
			continue;
		}

		deviceNumber++;

		// 设备被屏蔽了
		if (deviceInfo.second->__isset.masked && deviceInfo.second->masked)
		{
			continue;
		}

		ULOG(LOG_INFO, "%s(devicesn: %s shiled message: %s)", __FUNCTION__, deviceInfo.second->deviceSN,
			deviceInfo.second->maskedAssayCode);
        // 是否包含项目对应的编号
        if (!isHaveCode(deviceInfo.second->maskedAssayCode, assayCode))
        {
			// 若设备未mask对应的项目，但是设备的试剂可测试数为0，那么依然属于不可用
			if (false)
			{
				// 查看当前设备是否有可用的试剂
				for (const auto& reagent : status.reagentMap)
				{
					if (reagent.second->GetDeviceSn() != deviceInfo.second->deviceSN)
					{
						continue;
					}

					// 若试剂正常则返回正常
					if (!reagent.second->IsReagentAbnormal())
					{
						return false;
					}
				}

				// 因为当前设备无可用试剂，虽然取消了设备屏蔽，但是依然需要根据其他设备来确定
				// 整个项目的屏蔽状态
				deviceReagentAbnormalNumber++;
				isPreRet = true;
				continue;
			}

            return false;
        }
    }

	if (false)
	{
		// 当仅有一台设备的时候
		if (deviceNumber == 1 && isPreRet)
		{
			return false;
		}

		// 当每台设备都因为试剂异常等原因导致假遮蔽
		if (deviceNumber != 0 && deviceReagentAbnormalNumber == deviceNumber)
		{
			return false;
		}
	}

    return true;
}

void QSystemMonitor::OnShiledAssay(class std::set<int, struct std::less<int>, class std::allocator<int>> assyaCodes)
{
	ULOG(LOG_INFO, "%s(assayCode size: %d)", __FUNCTION__, assyaCodes.size());
	// 存储需要更新的项目编号
	QStringList assayCodes;
	for (const auto& code : assyaCodes)
	{
		if (UpdateAssayMaskStatus(code))
		{
			assayCodes.push_back(QString::number(code));
		}
	}

	// 当需要更新的项目编号列表不为空的时候才更新
	if (!assayCodes.empty())
	{
		QString tempCodes = assayCodes.join(",");
		ULOG(LOG_INFO, "Post codes: %s.", tempCodes.toStdString().c_str());
		POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, tempCodes);
	}
}

void QSystemMonitor::OnUpdateDeviceAssayStatus(QString groupName, QString device, bool isMask)
{
	ULOG(LOG_INFO, "%s(groupName: %s device: %s)", __FUNCTION__, groupName, device);
	Q_UNUSED(groupName);
	Q_UNUSED(device);
	Q_UNUSED(isMask);

	tf::GeneralAssayInfoQueryCond qryCond;
	tf::GeneralAssayInfoQueryResp qryResp;
	if (!DcsControlProxy::GetInstance()->QueryGeneralAssayInfo(qryResp, qryCond)
		|| (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
	{
		ULOG(LOG_ERROR, "QueryGeneralAssayInfo() failed");
		return;
	}

	// 未查到项目信息，删除原来的项目信息
	if (qryResp.lstAssayInfos.empty())
	{
		return;
	}

	// 清除项目信息，全部重新计算
	m_AssayStatusMap.clear();
	QStringList assayCodes;
	// 将结果保存到项目映射中
	for (const tf::GeneralAssayInfo& assayInfo : qryResp.lstAssayInfos)
	{
		// 构造项目信息智能指针
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = std::make_shared<tf::GeneralAssayInfo>(assayInfo);
		if (spAssayInfo == Q_NULLPTR)
		{
			continue;
		}

		// 初始化项目状态
		QSystemMonitor::GetInstance().InitStatusMap(spAssayInfo);
		assayCodes.push_back(QString::number(spAssayInfo->assayCode));
	}

	// 当需要更新的项目编号列表不为空的时候才更新
	if (!assayCodes.empty())
	{
		ULOG(LOG_INFO, "%s(post codes: %s )", __FUNCTION__, assayCodes.join(","));
		POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, assayCodes.join(","));
	}
}

bool QSystemMonitor::UpdateAssayMaskStatus(int assayCode)
{
	bool ret = false;
	ULOG(LOG_INFO, "%s(shiled code: %d)", __FUNCTION__, assayCode);
	auto iter = m_AssayStatusMap.find(assayCode);
	if (iter == m_AssayStatusMap.end())
	{
		ULOG(LOG_INFO, "%s(can not find shiled code: %d)", __FUNCTION__, assayCode);
		return ret;
	}

	if (ISAssayShiled(iter->second, true))
	{
		if (!iter->second.maskAssay)
		{
			iter->second.maskAssay = true;
			ret = true;
		}
	}
	else
	{
		if (iter->second.maskAssay)
		{
			iter->second.maskAssay = false;
			ret = true;
		}
	}

	return ret;
}

void QSystemMonitor::OnUpdateSupplyForReagent(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates)
{
    ULOG(LOG_INFO, "%s(update supply size : %s)", __FUNCTION__, ToString(supplyUpdates));
    // 存储需要更新的项目编号
    QStringList assayCodes;
    // 依次处理每一瓶更新
    for (const auto& supplyUpdate : supplyUpdates)
    {
        // 工作页面不是处理仓外耗材信息
        if (supplyUpdate.posInfo.area == ch::tf::SuppliesArea::type::SUPPLIES_AREA_CABINET)
        {
            continue;
        }

        std::vector<ch::tf::ReagentGroup> vecReagentGroups;
        vecReagentGroups = GetReagentGroupsBySupply(supplyUpdate);
        if (vecReagentGroups.size() > 1)
        {
            ULOG(LOG_INFO, "%s(double assay : %d)", __FUNCTION__, vecReagentGroups.size());
        }

        // 依次处理试剂信息(更新工作页面的项目状态表)
        for (const auto& reagent : vecReagentGroups)
        {
            QVariant innerData;
            innerData.setValue(reagent);

			// 当卸载试剂的时候
			if (reagent.__isset.beDeleted && reagent.beDeleted)
			{
				if (!UnloadReagentProcess(std::make_shared<KeyData>(innerData)))
				{
					ULOG(LOG_INFO, "%s(unload assay: %d don't need changed)", __FUNCTION__, reagent.assayCode);
					continue;
				}
			}
			else
			{
				if (!UpdateAssayStatus(std::make_shared<KeyData>(innerData)))
				{
					ULOG(LOG_INFO, "%s(change assay: %d don't need changed)", __FUNCTION__, reagent.assayCode);
					continue;
				}
			}

            assayCodes.push_back(QString::number(reagent.assayCode));
        }
    }

    // 当需要更新的项目编号列表不为空的时候才更新
    if (!assayCodes.empty())
    {
        QString tempCodes = assayCodes.join(",");
        ULOG(LOG_INFO, "Post codes: %s.", tempCodes.toStdString().c_str());
        POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, tempCodes);
    }
}

void QSystemMonitor::OnUpdateIMSupplyForReagent(const im::tf::ReagentInfoTable& stuRgntInfo, const im::tf::ChangeType::type type)
{
    ULOG(LOG_INFO, "%s(update reagent assayCode : %d)", __FUNCTION__, stuRgntInfo.assayCode);
    // 存储需要更新的项目编号
    QString assayCodes;

    // 如果是卸载试剂，则按卸载试剂流程处理
    if (type == im::tf::ChangeType::type::Delete)
    {
        // 缓存数据
        QVariant innerData;
        innerData.setValue(stuRgntInfo);

        if (!UnloadReagentProcess(std::make_shared<KeyData>(innerData)))
        {
            ULOG(LOG_INFO, "%s(unload assay: %d don't need changed)", __FUNCTION__, stuRgntInfo.assayCode);
        }
    }
    // 如果需要实时刷新
    else if (m_bImRgntAbnUpdateRealTime)
    {
        // 更新试剂信息，构造查询条件和查询结果,查询所有试剂信息
        ::im::tf::ReagentInfoTableQueryResp qryResp;
        ::im::tf::ReagTableUIQueryCond qryCond;

        // 构造查询条件
        if (stuRgntInfo.__isset.assayCode && (stuRgntInfo.assayCode > 0))
        {
            std::vector<std::string> vecDevSns;
            vecDevSns.push_back(stuRgntInfo.deviceSN);
            qryCond.__set_deviceSNs(vecDevSns);
            qryCond.__set_assayCode(stuRgntInfo.assayCode);
        }
        else
        {
            qryCond.__set_reagentId(stuRgntInfo.id);
        }

        // 执行查询
        bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
        if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
            return;
        }

        // 后台设计缺陷无法监听试剂使用状态改变，暂由UI特殊处理：UI收到试剂更新通知时，更新该设备所有同项目试剂
        for (auto& stuRgntIf : qryResp.lstReagentInfos)
        {
            // 缓存数据
            QVariant innerData;
            innerData.setValue(stuRgntIf);

            // 更新
            if (!UpdateAssayStatus(std::make_shared<KeyData>(innerData)))
            {
                ULOG(LOG_INFO, "%s(assay: %d don't need changed)", __FUNCTION__, stuRgntIf.assayCode);
                continue;
            }
        }
    }

    assayCodes = QString::number(stuRgntInfo.assayCode);
    POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, assayCodes);
}

void QSystemMonitor::OnUpdateSupplyForISE(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates)
{
    ULOG(LOG_INFO, "%s(update supply size : %d)", __FUNCTION__, supplyUpdates.size());
    // 存储需要更新的项目编号
    QStringList assayCodes;
    // ISE的项目
    std::vector<int> iseSubCodes;
    iseSubCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_NA);
    iseSubCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_K);
    iseSubCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_CL);

    // 依次处理每一瓶更新
    for (const auto& supplyUpdate : supplyUpdates)
    {
        QVariant innerData;
        auto supplyInfo = supplyUpdate.suppliesInfo;
        innerData.setValue(supplyInfo);
        auto spIseData = std::make_shared<KeyIseDate>(innerData);
        bool hasDone = false;

        for (const auto& code : iseSubCodes)
        {
            auto iter = m_AssayStatusMap.find(code);
            if (iter == m_AssayStatusMap.end())
            {
                continue;
            }

            spIseData->SetStatus(iter->second);
			// 耗材卸载的处理
			if (supplyUpdate.__isset.updateType && supplyUpdate.updateType == tf::UpdateType::UPDATE_TYPE_DELETE)
			{
				if (!UnloadReagentProcess(spIseData))
				{
					ULOG(LOG_INFO, "%s(unload change ise code: %d don't need changed)", __FUNCTION__, supplyInfo.suppliesCode);
					continue;
				}
			}
			else
			{
				if (!UpdateAssayStatus(spIseData))
				{
					ULOG(LOG_INFO, "%s(change ise code: %d don't need changed)", __FUNCTION__, supplyInfo.suppliesCode);
					continue;
				}
			}

            if (hasDone)
            {
                assayCodes.push_back(QString::number(code));
                hasDone = true;
            }
        }
    }

    // 当需要更新的项目编号列表不为空的时候才更新
    if (!assayCodes.empty())
    {
        ULOG(LOG_INFO, "%s(post codes: %s )", __FUNCTION__, assayCodes.join(","));
        POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, assayCodes.join(","));
    }
}

///
/// @brief
///     刷新免疫试剂异常校准异常标志
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月25日，新建函数
///
void QSystemMonitor::RefreshIMSupplyForReagent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新所有试剂（已统一缓存处理）
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 执行查询
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        return;
    }

    // 逐个更新
    QStringList assayCodes;
    for (auto& stuRgntIf : qryResp.lstReagentInfos)
    {
        // 缓存数据
        QVariant innerData;
        innerData.setValue(stuRgntIf);

        // 更新
        if (!UpdateAssayStatus(std::make_shared<KeyData>(innerData)))
        {
            ULOG(LOG_INFO, "%s(assay: %d don't need changed)", __FUNCTION__, stuRgntIf.assayCode);
            continue;
        }

        // 添加项目编号
        assayCodes.push_back(QString::number(stuRgntIf.assayCode));
    }

    // 当需要更新的项目编号列表不为空的时候才更新
    if (!assayCodes.empty())
    {
        ULOG(LOG_INFO, "%s(post codes: %s )", __FUNCTION__, assayCodes.join(","));
        POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, assayCodes.join(","));
    }
}

///
/// @brief
///     检测模式更新处理
///
/// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月10日，新建函数
///
void QSystemMonitor::OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果参数未空则忽略
    if (mapUpdateInfo.isEmpty())
    {
        ULOG(LOG_WARN, "%s(), mapUpdateInfo.isEmpty()", __FUNCTION__);
        return;
    }

    // 更新试剂信息，构造查询条件和查询结果,查询所有试剂信息
    ::im::tf::ReagentInfoTableQueryResp qryResp;
    ::im::tf::ReagTableUIQueryCond qryCond;

    // 构造查询条件
    std::vector<std::string> vecDevSns;
    for (auto it = mapUpdateInfo.begin(); it != mapUpdateInfo.end(); it++)
    {
        vecDevSns.push_back(it.key().toStdString());
    }
    qryCond.__set_deviceSNs(vecDevSns);

    // 执行查询
    bool bRet = ::im::LogicControlProxy::QueryReagentInfoForUI(qryResp, qryCond);
    if (!bRet || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryReagentInfoForUI failed.");
        return;
    }

    // 后台设计缺陷无法监听试剂使用状态改变，暂由UI特殊处理：UI收到试剂更新通知时，更新该设备所有同项目试剂
    QSet<int> setAssayCode;
    for (auto& stuRgntIf : qryResp.lstReagentInfos)
    {
        // 缓存数据
        QVariant innerData;
        innerData.setValue(stuRgntIf);

        // 更新
        if (!UpdateAssayStatus(std::make_shared<KeyData>(innerData)))
        {
            ULOG(LOG_INFO, "%s(assay: %d don't need changed)", __FUNCTION__, stuRgntIf.assayCode);
            continue;
        }

        // 记录项目编号
        setAssayCode.insert(stuRgntIf.assayCode);
    }

    // 存储需要更新的项目编号
    QStringList strAssayCodeLst;
    for (const int iAssayCode : setAssayCode)
    {
        strAssayCodeLst.push_back(QString::number(iAssayCode));
    }
    ULOG(LOG_INFO, "%s(post codes: %s )", __FUNCTION__, strAssayCodeLst.join(","));
    POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, strAssayCodeLst.join(","));
}

void QSystemMonitor::OnUpdateSampleType()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 存储需要更新的项目编号
    QStringList assayCodes;
    // ISE的项目
    std::vector<int> iseSubCodes;
    iseSubCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_NA);
    iseSubCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_K);
    iseSubCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_CL);

    bool hasDone = false;
    for (auto code : iseSubCodes)
    {
        auto iter = m_AssayStatusMap.find(code);
        if (iter == m_AssayStatusMap.end())
        {
            continue;
        }

        // 重新计算每一瓶耗材
        for (const auto& reagent : iter->second.reagentMap)
        {
            auto spReagent = reagent.second;
            if (spReagent == Q_NULLPTR)
            {
                continue;
            }

            if (!UpdateAssayStatus(spReagent))
            {
                ULOG(LOG_INFO, "%s(ise code: %d don't need changed)", __FUNCTION__, spReagent->GetCode());
                continue;
            }
        }

        assayCodes.push_back(QString::number(code));
    }

    // 当需要更新的项目编号列表不为空的时候才更新
    if (!assayCodes.empty())
    {
        ULOG(LOG_INFO, "%s(post codes: %s )", __FUNCTION__, assayCodes.join(","));
        POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, assayCodes.join(","));
    }
}

void QSystemMonitor::UpdateIseCaliModuleSatus()
{
	std::vector<int> iseCodes;
	iseCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_ISE);
	iseCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_NA);
	iseCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_K);
	iseCodes.push_back(ise::tf::g_ise_constants.ASSAY_CODE_CL);
	for (const auto code : iseCodes)
	{
		// 本身没有包含此项目，则不处理
		if (m_AssayStatusMap.count(code) <= 0)
		{
			continue;
		}

		// 查询是否有对应的项目设置信息
		auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(code);
		if (spAssay == nullptr)
		{
			continue;
		}

		// 重新设置项目新状态
		InitStatusMap(spAssay);
	}
}

///
/// @brief
///     设置免疫试剂异常标志是否实时刷新（项目选择界面显示时置为true，否则置为false）
///
/// @param[in]  updateRealTime  是否实时刷新
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月25日，新建函数
///
void QSystemMonitor::SetImRgntAbnFlagUpdateRealTime(bool updateRealTime)
{
    // 如果从非实时刷新变为实时刷新，需要先刷新全部
    if (updateRealTime && !m_bImRgntAbnUpdateRealTime)
    {
        RefreshIMSupplyForReagent();
    }

    // 更新实时刷新标志
    m_bImRgntAbnUpdateRealTime = updateRealTime;
}

void QSystemMonitor::UpdateMaskDeviceInfo()
{
    QStringList assayCodes;
    // 项目状态
    for (auto& assayStatus : m_AssayStatusMap)
    {
        if (ISAssayShiled(assayStatus.second))
        {
            if (!assayStatus.second.maskAssay)
            {
                assayCodes.push_back(QString::number(assayStatus.first));
                assayStatus.second.maskAssay = true;
            }
        }
        else
        {
            if (assayStatus.second.maskAssay)
            {
                assayCodes.push_back(QString::number(assayStatus.first));
                assayStatus.second.maskAssay = false;
            }
        }
    }

    // 当需要更新的项目编号列表不为空的时候才更新
    if (!assayCodes.empty())
    {
        ULOG(LOG_INFO, "%s(post codes: %s )", __FUNCTION__, assayCodes.join(","));
        POST_MESSAGE(MSG_ID_WORK_ASSAY_STATUS_UPDATE, assayCodes.join(","));
    }
}

void QSystemMonitor::SetCurrentType(int sourceType)
{
    switch (sourceType)
    {
        // 血清/血浆
    case 1:
        m_sourceType = ise::tf::SampleType::Serum;
        break;
        // 尿液
    case 2:
        m_sourceType = ise::tf::SampleType::Urine;
        break;
    default:
        m_sourceType = ise::tf::SampleType::Unknown;
        break;
    }
}

QSystemMonitor::~QSystemMonitor()
{

}

///
/// @brief 初始化项目状态地图
///
/// @param[in]  spAssayInfo  项目信息
///
/// @return true:初始化成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月21日，新建函数
///
bool QSystemMonitor::InitStatusMap(std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo)
{
    if (spAssayInfo == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
        return false;
    }

    auto assayCode = spAssayInfo->assayCode;
    ULOG(LOG_INFO, "%s(assayCode : %d)", __FUNCTION__, assayCode);

    AssayStatus status;
    status.assayCode = assayCode;

    // 生化项目,免疫项目，ISE项目
    if (spAssayInfo->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY
        || spAssayInfo->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE
        || spAssayInfo->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_ISE)
    {
        // 试剂异常&&校准异常
        auto reagentGroups = GetReagentGroup(assayCode, spAssayInfo->assayClassify);
        do
        {
            // 若没有试剂信息，则未未加载属于试剂异常状态
            if (reagentGroups.empty())
            {
                status.abnormalReagent = true;
                break;
            }

            // 试剂异常，校准异常（首先假设存在异常，循环判断试剂，只要存在一个不异常的，表明不异常）
            status.abnormalReagent = true;
            status.abnormalCalibrate = true;
            for (auto& reagent : reagentGroups)
            {
                // 试剂异常(只要不是全部试剂都异常就不是异常)
                if (status.abnormalReagent)
                {
                    status.abnormalReagent = reagent->IsReagentAbnormal();
                }

                // 校准异常
                if (status.abnormalCalibrate)
                {
                    status.abnormalCalibrate = reagent->IsCaliBrateAbnormal();
                }

                // 构建试剂地图
                status.reagentMap[reagent->Getid()] = reagent;
                // 回设状态
                reagent->SetStatus(status);

            }

        } while (false);
    }
    // 未知项目
    else
    {
        return false;
    }

	// 项目遮蔽(因为项目遮蔽的判断会用到对应项目的试剂信息，故放到后面)
	status.maskAssay = true;
	if (ISAssayShiled(status))
	{
		status.maskAssay = true;
	}
	else
	{
		status.maskAssay = false;
	}

    m_AssayStatusMap[assayCode] = status;
    return true;
}

QSystemMonitor& QSystemMonitor::GetInstance()
{
    static QSystemMonitor instance;
    return instance;
}

bool QSystemMonitor::UpdateAssayStatus(spKeyData reagentGroup)
{
    if (reagentGroup == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(reagentGroup pointer is empty : %d)", __FUNCTION__);
        return false;
    }

	int reagentCode = reagentGroup->GetCode();
    ULOG(LOG_INFO, "%s(reagentGroup assayCode : %d)", __FUNCTION__, reagentCode);
	std::vector<int> subCodes;
	if (reagentCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		subCodes = { ::ch::tf::g_ch_constants.ASSAY_CODE_L,
			::ch::tf::g_ch_constants.ASSAY_CODE_H,
			::ch::tf::g_ch_constants.ASSAY_CODE_I,
			// 增加组合项目
			::ch::tf::g_ch_constants.ASSAY_CODE_SIND };
	}
	else
	{
		subCodes = { reagentCode };
	}

	// 状态是否变化（与之前状态相比较，当状态没有变化的时候，不需要更新工作页面的状态，避免多余更新）
	bool isStatusChanged = false;
	for (const auto& code : subCodes)
	{
		auto iterStatus = m_AssayStatusMap.find(code);
		if (iterStatus == m_AssayStatusMap.end())
		{
			ULOG(LOG_ERROR, "%s(can not find the reagentGroup assayCode : %d)", __FUNCTION__, code);
			continue;
		}

		// 计算本瓶试剂异常状态
		bool statusReagent = reagentGroup->IsReagentAbnormal();
		bool statusCaliBrate = reagentGroup->IsCaliBrateAbnormal();

		// 获取项目状态信息
		auto& assayStatus = iterStatus->second;
		// 若之前的项目是试剂未加载状态，那么当前项目状态就由新增的这瓶试剂决定
		if (assayStatus.reagentMap.empty())
		{
			// 状态有变化
			if (assayStatus.abnormalReagent != statusReagent
				|| assayStatus.abnormalCalibrate != statusCaliBrate)
			{
				assayStatus.abnormalReagent = statusReagent;
				assayStatus.abnormalCalibrate = statusCaliBrate;
				isStatusChanged = true;
			}

			assayStatus.reagentMap[reagentGroup->Getid()] = reagentGroup;
			continue;
		}

		// 更新试剂
		assayStatus.reagentMap[reagentGroup->Getid()] = reagentGroup;

		// 试剂的情况
		// 若之前项目状态是异常，当前试剂正常了，那么项目状态变为正常
		if (assayStatus.abnormalReagent && !statusReagent)
		{
			assayStatus.abnormalReagent = false;
			isStatusChanged = true;
		}

		// 之前正常，当前异常,需要重新整体检查
		if (!assayStatus.abnormalReagent && statusReagent)
		{
			// 假设全部为异常常
			bool isAbbormal = true;
			for (const auto & reagentIter : assayStatus.reagentMap)
			{
				// 若有正常直接退出
				if (!reagentIter.second->IsReagentAbnormal())
				{
					isAbbormal = false;
					break;
				}
			}

			// 状态不一致
			if (assayStatus.abnormalReagent != isAbbormal)
			{
				isStatusChanged = true;
			}

			assayStatus.abnormalReagent = isAbbormal;
		}

		//校准的情况
		// 若之前项目状态是异常，当前试剂正常了，那么项目状态变为正常
		if (assayStatus.abnormalCalibrate && !statusCaliBrate)
		{
			assayStatus.abnormalCalibrate = false;
			isStatusChanged = true;
		}

		// 之前正常，当前异常,需要重新整体检查
		if (!assayStatus.abnormalCalibrate && statusCaliBrate)
		{
			// 假设全部为异常常
			bool isAbbormal = true;
			for (const auto & reagentIter : assayStatus.reagentMap)
			{
				// 若有正常直接退出
				if (!reagentIter.second->IsCaliBrateAbnormal())
				{
					isAbbormal = false;
					break;
				}
			}

			// 状态不一致
			if (assayStatus.abnormalCalibrate != isAbbormal)
			{
				isStatusChanged = true;
			}

			assayStatus.abnormalCalibrate = isAbbormal;
		}

		// 试剂的状态变化会影响遮蔽的状态
		if (ISAssayShiled(assayStatus, true))
		{
			if (!assayStatus.maskAssay)
			{
				isStatusChanged = true;
				assayStatus.maskAssay = true;
			}
		}
		else
		{
			if (assayStatus.maskAssay)
			{
				isStatusChanged = true;
				assayStatus.maskAssay = false;
			}
		}

	}

    return isStatusChanged;
}

///
/// @brief 处理卸载试剂的流程
///
/// @param[in]  reagentGroup  卸载试剂
///
/// @return true 是否修改了项目状态
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月9日，新建函数
///
bool QSystemMonitor::UnloadReagentProcess(spKeyData reagentGroup)
{
	if (reagentGroup == Q_NULLPTR)
	{
		ULOG(LOG_ERROR, "%s(reagentGroup pointer is empty : %d)", __FUNCTION__);
		return false;
	}

	int reagentCode = reagentGroup->GetCode();
	ULOG(LOG_INFO, "%s(reagentGroup assayCode : %d)", __FUNCTION__, reagentCode);
	std::vector<int> subCodes;
	if (reagentCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		subCodes = { ::ch::tf::g_ch_constants.ASSAY_CODE_L,
			::ch::tf::g_ch_constants.ASSAY_CODE_H,
			::ch::tf::g_ch_constants.ASSAY_CODE_I };
	}
	else
	{
		subCodes = { reagentCode };
	}

	for (const auto code : subCodes)
	{
		auto iterStatus = m_AssayStatusMap.find(code);
		if (iterStatus == m_AssayStatusMap.end())
		{
			ULOG(LOG_ERROR, "%s(can not find the reagentGroup assayCode : %d)", __FUNCTION__, code);
			return false;
		}

		// 获取项目状态信息
		auto& assayStatus = iterStatus->second;
		// 获取试剂对应的记录的迭代器
		auto iterReagent = assayStatus.reagentMap.find(reagentGroup->Getid());
		// 当试剂卸载的时候，若未发现之前的试剂记录，说明软件误报(打印错误日志)
		if (iterReagent == assayStatus.reagentMap.end())
		{
			ULOG(LOG_ERROR, "%s(can not find the reagentGroup id: %d)", __FUNCTION__, reagentGroup->Getid());
			return false;
		}

		// 当卸载试剂的时候首先从试剂Map中删除试剂
		assayStatus.reagentMap.erase(iterReagent);
		// 删除试剂以后，在重新计算目前项目的试剂状态（所有的试剂参与）
		auto reagentMap = assayStatus.reagentMap;
		do
		{
			// 若卸载试剂以后，项目没有对应的试剂{试剂异常：未加载；校准正常：没有试剂}
			if (reagentMap.empty())
			{
				assayStatus.abnormalReagent = true;
				assayStatus.abnormalCalibrate = false;
				break;
			}

			// 试剂异常，校准异常（首先假设存在异常，循环判断试剂，只要存在一个不异常的，表明不异常）
			assayStatus.abnormalReagent = true;
			assayStatus.abnormalCalibrate = true;
			for (auto& reagentIter : reagentMap)
			{
				auto reagent = reagentIter.second;
				// 试剂异常(只要不是全部试剂都异常就不是异常)
				if (assayStatus.abnormalReagent)
				{
					assayStatus.abnormalReagent = reagent->IsReagentAbnormal();
				}

				// 校准异常
				if (assayStatus.abnormalCalibrate)
				{
					assayStatus.abnormalCalibrate = reagent->IsCaliBrateAbnormal();
				}
			}

		} while (false);
	}

	return true;
}

///< 静态单实例对象初始化
std::shared_ptr<CommonInformationManager> CommonInformationManager::sm_instance(new CommonInformationManager());

CommonInformationManager::CommonInformationManager()
{
    // 默认单项目显示五行
    m_singleAssayRowAmount = UI_DEFAULT_ASSAYSELECTTB_SINGLEROW;

    // 注册项目参数更新事件处理函数
    REGISTER_HANDLER(MSG_ID_PROFILE_PARAM_UPDATE, this, UpdateProfileInfo);
    REGISTER_HANDLER(MSG_ID_DCS_REPORT_STATUS_CHANGED, this, UpdateDeviceStatus);
    REGISTER_HANDLER(MSG_ID_DEVS_CAN_APPEND_TEST, this, UpdateDeviceCanAppendTest);
    //REGISTER_HANDLER(MSG_ID_BUCKET_SET_UPDATE, this, UpdateDeviceInfo); 废液桶报警值变化 为什么要关注该消息
    REGISTER_HANDLER(MSG_ID_CALC_ASSAY_UPDATE, this, UpdateCalcAssayInfo);
    REGISTER_HANDLER(MSG_ID_SPL_ALARM_VAL_UPDATE, this, UpdateSupplyAttribute);
    REGISTER_HANDLER(MSG_ID_TEMPERATURE_ERR, this, OnDevTemperatureErrChanged);
	// 更新电解质模块状态的变化
	REGISTER_HANDLER(MSG_ID_ISE_CALI_APP_UPDATE, this, OnUpdateCaliAppSatus);
	// 试剂报警值更新
	REGISTER_HANDLER(MSG_ID_RGNT_ALARM_VOL_UPDATE, this, OnReagentAlarmValueChanged);
	// 监听生化在线试剂加载器信息更新
	REGISTER_HANDLER(MSG_ID_REAGENT_LOADER_UPDATE, this, OnChReagentLoaderInfoUpdate);
	// 生化试剂&耗材更新消息
	REGISTER_HANDLER(MSG_ID_REAGENT_INFO_UPDATE, this, OnUpdateChReagentSupply);
	// ISE耗材更新消息
	REGISTER_HANDLER(MSG_ID_ISE_INFO_UPDATE, this, OnUpdateISeSupply);
	// 生化废液桶余量更新
	REGISTER_HANDLER(MSG_ID_DEVICE_STATUS_INFO, this, OnUpdateBucketStatus);
}

CommonInformationManager::~CommonInformationManager()
{
}

///
/// @brief
///     项目编号管理器初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月2日，新建函数
///
bool CommonInformationManager::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 类型名称的匹配
    {
        int type = 0;
        m_nameMap.insert(type++, tr(""));
        m_nameMap.insert(type++, tr("状态"));
        m_nameMap.insert(type++, tr("样本号"));
        m_nameMap.insert(type++, tr("样本条码"));
        m_nameMap.insert(type++, tr("样本类型"));
        m_nameMap.insert(type++, tr("位置"));
        m_nameMap.insert(type++, tr("检测完成时间"));
        m_nameMap.insert(type++, tr("复查完成时间"));
		m_nameMap.insert(type++, tr("审核"));
		m_nameMap.insert(type++, tr("打印"));

        m_nameMap.insert(type++, tr("项目名称"));
        m_nameMap.insert(type++, tr("检测结果"));
        m_nameMap.insert(type++, tr("数据报警"));
        m_nameMap.insert(type++, tr("复查结果"));
        m_nameMap.insert(type++, tr("复查数据报警"));
        m_nameMap.insert(type++, tr("单位"));
        m_nameMap.insert(type++, tr("模块"));
		m_nameMap.insert(type++, tr("检测信号值"));
		m_nameMap.insert(type++, tr("复查信号值"));
    }

    // 类型名称的匹配
    {
        int type = 1;
        m_rgntHeaderMap.insert(type++, tr("试剂位"));
        m_rgntHeaderMap.insert(type++, tr("名称"));
        m_rgntHeaderMap.insert(type++, tr("可用测试数"));
        m_rgntHeaderMap.insert(type++, tr("剩余测试数"));
        m_rgntHeaderMap.insert(type++, tr("使用状态"));
        m_rgntHeaderMap.insert(type++, tr("校准状态"));
        m_rgntHeaderMap.insert(type++, tr("试剂批号"));
        m_rgntHeaderMap.insert(type++, tr("瓶号"));
        m_rgntHeaderMap.insert(type++, tr("当前曲线有效期"));
        m_rgntHeaderMap.insert(type++, tr("开瓶有效期(天)"));
        m_rgntHeaderMap.insert(type++, tr("上机时间"));
        m_rgntHeaderMap.insert(type++, tr("失效日期"));
    }

	// 加载单项目行数(修改获取位置，方便项目排序使用)
	m_singleAssayRowAmount = DcsControlProxy::GetInstance()->GetAssayTabSignleRowAmount();

    // 初始化试剂管理器(需要放在前面注册消息，以先收到试剂项目相关的UI消息)
    InitRgntManager();

    // 初始化设备信息
    if (!InitDeviceReflection())
    {
        ULOG(LOG_ERROR, "InitDeviceReflection() failed");
        return false;
    }

    // 初始化项目映射
    if (!InitAssayReflection())
    {
        ULOG(LOG_ERROR, "InitAssayReflection failed");
        return false;
    }

    // 初始化耗材属性
    UpdateSupplyAttribute();

    // 初始化计算项目信息
    if (!UpdateCalcAssayInfo())
    {
        ULOG(LOG_ERROR, "UpdateCalcAssayInfo() failed");
        return false;
    }

    // 初始化数据报警Map
    UpdateDataAlarmMap();

	// 初始化生化设备信息
	InitChDeviceInfo();

	// 初始化ISE设备信息
    InitIseDeviceInfo();

    return true;
}

///
/// @brief
///     初始化项目映射
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月2日，新建函数
///
bool CommonInformationManager::InitAssayReflection()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 通用项目配置信息
    ReloadGeneralAssayInfo();
    // 生化通用项目映射表
    ReloadAssayInfoCh();

    // 生化特殊项目信息
    ReloadAssayInfoChSpecial();

    // 免疫通用项目映射表
    ReloadAssayInfoIm();

    // ISE通用项目映射表
    ReloadAssayInfoIse();

    // ISE的特殊项目
    ReloadAssayInfoIseSpecial();

    // 组合项目通用信息
    ReloadGeneralProfile();

    return true;
}

///
/// @brief
///     初始化设备信息映射表
///
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月11日，新建函数
///
bool CommonInformationManager::InitDeviceReflection()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ::tf::DeviceInfoQueryResp DeviceResp;
    ::tf::DeviceInfoQueryCond QueryDeviceCond;

    // 查询设备信息
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(DeviceResp, QueryDeviceCond)
        || DeviceResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s() : GET device information failed", __FUNCTION__);
        return false;
    }

	// 如果没有配置任何设备
	if (DeviceResp.lstDeviceInfos.empty())
	{
		QMessageBox::warning(nullptr, u8"提示", u8"未添加任何设备");
		ULOG(LOG_INFO, "DeviceResp.lstDeviceInfos is empty");
		return true;
	}

    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    // 依次更新设备信息; 
    UpdateDeviceBuff(DeviceResp.lstDeviceInfos);

    // 设备更新的时候，更新遮蔽项目的状态
    QSystemMonitor::GetInstance().UpdateMaskDeviceInfo();

    // 更新ise的模组信息
    m_iseModuleMaps.clear();
    ::ise::tf::IseModuleInfoQueryResp IseModuleResp;
    ::ise::tf::IseModuleInfoQueryCond queryIseModule;

    // 查询设备信息
    ise::LogicControlProxy::QueryIseModuleInfo(IseModuleResp, queryIseModule);
    if (IseModuleResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s() : GET ise module failed", __FUNCTION__);
        return false;
    }

    // 依次更新ise模块信息
    for (const auto& module : IseModuleResp.lstIseModuleInfos)
    {
        auto spDeviceInfo = std::make_shared<ise::tf::IseModuleInfo>(module);
        m_iseModuleMaps[spDeviceInfo->id] = spDeviceInfo;
    }

    return true;
}

///
/// @brief 获取生化指定项目编号的样本类型
/// 
void CommonInformationManager::GetSampleTypeCh(int assayCode, std::set<int>& sampleType)
{
    sampleType.clear();

    std::shared_ptr<ch::tf::GeneralAssayInfo> spCurrentAssay = nullptr;

    // 查找项目编号对应的项目信息
    auto range = m_mapChAssayCodeMaps.equal_range(assayCode);
    for (auto itr = range.first; itr != range.second; itr++)
    {
        std::shared_ptr<ch::tf::GeneralAssayInfo> spTempGai = itr->second;
        if (spTempGai == nullptr)
        {
            continue;
        }

        // 编号相同/设备类型/必须使能
        if (spTempGai->enable &&
            spTempGai->assayCode == assayCode && 
            spTempGai->deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
        {
            spCurrentAssay = spTempGai;
            break;
        }
    }

    if (spCurrentAssay == nullptr)
    {
        return;
    }

    auto specialRange = m_mapChSpecialAssayCodeMaps.equal_range(assayCode);
    for (auto itr = specialRange.first; itr != specialRange.second; itr++)
    {
        std::shared_ptr<ch::tf::SpecialAssayInfo> spTempSai = itr->second;

        // 编号相同/设备类型/版本类型
        if (spTempSai->assayCode == assayCode && 
            spTempSai->deviceType == tf::DeviceType::DEVICE_TYPE_C1000 &&
            spTempSai->version == spCurrentAssay->version)
        {
            sampleType.insert(spTempSai->sampleSourceType);
        }
    }
}

void CommonInformationManager::GetSampleTypeIm(int assayCode, std::set<int>& sampleType)
{
    sampleType.clear();

    ImAssayIndexCodeMaps::iterator it = m_mapImAssayCodeMaps.begin();
    for (; it != m_mapImAssayCodeMaps.end(); ++it)
    {
        if (it->second->assayCode == assayCode &&  it->second->usingFlag)
        {
            sampleType.insert(it->second->sampleSource);
        }
    }
}

void CommonInformationManager::ReloadGeneralAssayInfo(int assayCode /*= -1*/)
{
    ULOG(LOG_INFO, "%s, appoint assayCode:%d.", __FUNCTION__, assayCode);

    tf::GeneralAssayInfoQueryCond qryCond;
    if (assayCode > 0)
    {
        qryCond.__set_assayCode(assayCode);
    }

    tf::GeneralAssayInfoQueryResp qryResp;
    if (!DcsControlProxy::GetInstance()->QueryGeneralAssayInfo(qryResp, qryCond)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "QueryGeneralAssayInfo() failed");
        return;
    }

    // 删除项目编号索引的缓存
    if (assayCode == -1)
    {
        m_mapAssayCodeMaps.clear();
    } 
    else
    {
        m_mapAssayCodeMaps.erase(assayCode);
    }

    auto iter = find_if(m_mapAssayNameMaps.begin(), m_mapAssayNameMaps.end(), [&assayCode](auto& item)
    {
        return item.second->assayCode == assayCode;
    });
    if (iter != m_mapAssayNameMaps.end())
    {
        m_mapAssayNameMaps.erase(iter);
    }

    // 未查到项目信息，删除原来的项目信息
    if (qryResp.lstAssayInfos.empty())
    {
        ULOG(LOG_ERROR, "Empty qryResp.lstAssayInfos.");
        return;
    }

    // 将结果保存到项目映射中
    for (const tf::GeneralAssayInfo& assayInfo : qryResp.lstAssayInfos)
    {
        // 构造项目信息智能指针
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = std::make_shared<tf::GeneralAssayInfo>(assayInfo);

        // 更新项目编号映射
        if (assayInfo.__isset.assayCode)
        {
            // 通用项目信息--编号和项目的映射关系
            m_mapAssayCodeMaps[assayInfo.assayCode] = spAssayInfo;

			if (assayInfo.__isset.pageIdx 
				&& assayInfo.__isset.positionIdx
				&& assayInfo.pageIdx >= 0
				&& assayInfo.positionIdx >= 0)
			{

				// 每页项目数 = 行数 * 列数
				int pageNumber = m_singleAssayRowAmount * 7;
				// （项目编号，项目位置)；项目位置 = 页数*每页项目数 + 页内位置
				m_defaultPostionMap[assayInfo.assayCode] = assayInfo.pageIdx * pageNumber + assayInfo.positionIdx;
			}
        }

        if (assayInfo.__isset.assayName)
        {
            m_mapAssayNameMaps[assayInfo.assayName] = spAssayInfo;
        }

        // 初始化项目状态
        // InitAssayStatusMap(spAssayInfo);
        // 初始化项目状态
        QSystemMonitor::GetInstance().InitStatusMap(spAssayInfo);
    }
}

void CommonInformationManager::ReloadAssayInfoCh(int assayCode /*= -1*/)
{
    ULOG(LOG_INFO, __FUNCTION__);

    ::ch::tf::GeneralAssayInfoQueryCond qryCond;
    if (assayCode > 0)
    {
        qryCond.__set_assayCode(assayCode);
    }

    ::ch::tf::GeneralAssayInfoQueryResp qryResp;
    if (!ch::LogicControlProxy::QueryAssayConfigInfo(qryCond, qryResp)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "QueryAssayConfigInfo() failed");
        return;
    }

    // 删除原来的项目信息
    if (assayCode == -1)
    {
        m_mapChAssayCodeMaps.clear();
    } 
    else
    {
        m_mapChAssayCodeMaps.erase(assayCode);
    }

    // 将生化项目保存到项目映射中
    for (const auto& assayChInfo : qryResp.lstGeneralAssayInfos)
    {
        // 更新项目编号映射
        if (assayChInfo.__isset.assayCode)
        {
            // 生化通用项目信息--编号和项目的映射关系
            m_mapChAssayCodeMaps.insert(std::pair<int, std::shared_ptr<ch::tf::GeneralAssayInfo>>(assayChInfo.assayCode
                , std::make_shared<ch::tf::GeneralAssayInfo>(assayChInfo)));
        }
    }
}

void CommonInformationManager::ReloadAssayInfoChSpecial(int assayCode /*= -1*/)
{
    ULOG(LOG_INFO, __FUNCTION__);

    ::ch::tf::SpecialAssayInfoQueryCond qryCond;
    if (assayCode > 0)
    {
        qryCond.__set_assayCode(assayCode);
    }

    ::ch::tf::SpecialAssayInfoQueryResp qryResp;
    if (!ch::LogicControlProxy::QuerySpecialAssayInfo(qryResp, qryCond)
        || (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "QuerySpecialAssayInfo() failed");
        return;
    }

    // 删除原来的项目信息
    if (assayCode == -1)
    {
        m_mapChSpecialAssayCodeMaps.clear();
    } 
    else
    {
        m_mapChSpecialAssayCodeMaps.erase(assayCode);
    }

    // 将结果保存到生化特殊项目映射中
    for (const auto& assayChSpecialInfo : qryResp.lstSpecialAssayInfos)
    {
        // 更新项目编号映射
        if (assayChSpecialInfo.__isset.assayCode)
        {
            // 生化通用项目信息--编号和项目的映射关系
            m_mapChSpecialAssayCodeMaps.insert(std::pair<int, std::shared_ptr<ch::tf::SpecialAssayInfo>>(assayChSpecialInfo.assayCode
                , std::make_shared<ch::tf::SpecialAssayInfo>(assayChSpecialInfo)));
        }
    }
}

void CommonInformationManager::ReloadAssayInfoChSpecialAfterEditTwins(int firstCode, int secondCode, int editBeforeSecondCode /*= -1*/)
{
    ReloadAssayInfoChSpecial(firstCode);
    ReloadAssayInfoChSpecial(secondCode);

    if (editBeforeSecondCode > 0)
    {
        ReloadAssayInfoChSpecial(editBeforeSecondCode);
    }
}

void CommonInformationManager::ReloadAssayInfoIm(int assayCode /*= -1*/)
{
    ULOG(LOG_INFO, __FUNCTION__);

    ::im::tf::GeneralAssayInfoQueryCond qryCond;
    if (assayCode > 0)
    {
        qryCond.__set_assayCode(assayCode);
    }

    ::im::tf::GeneralAssayInfoQueryResp qryResp;
    if (!im::LogicControlProxy::QueryGeneralAssayInfo(qryCond, qryResp)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "QueryAssayConfigInfo() failed");
        return;
    }

    // 删除待更新的信息
    if (assayCode > 0)
    {
        auto range = m_mapImAssayCodeMaps.equal_range(assayCode);
        while (range.first != range.second)
        {
            range.first = m_mapImAssayCodeMaps.erase(range.first);
        }
    }
    else
    {
        m_mapImAssayCodeMaps.clear();
    }

    // 将免疫项目保存到项目映射中
    for (const auto& imAssayInfo : qryResp.lstAssayConfigInfos)
    {
        // 更新项目编号映射
        if (imAssayInfo.__isset.assayCode)
        {
            // 生化通用项目信息--编号和项目的映射关系
            m_mapImAssayCodeMaps.insert(std::pair<int, std::shared_ptr<im::tf::GeneralAssayInfo>>(imAssayInfo.assayCode
                , std::make_shared<im::tf::GeneralAssayInfo>(imAssayInfo)));
        }
    }
}

void CommonInformationManager::ReloadAssayInfoIse()
{
    ULOG(LOG_INFO, __FUNCTION__);

    ::ise::tf::GeneralAssayInfoQueryCond qryCond;
    ::ise::tf::GeneralAssayInfoQueryResp qryResp;
    if (!ise::LogicControlProxy::QueryAssayConfigInfo(qryCond, qryResp)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "Query ISE General Assay Info Failed!");
        return;
    }

    m_mapIseAssayCodeMaps.clear();

    // 将结果保存到ISE普通项目映射中
    for (const auto& each : qryResp.lstGeneralAssayInfos)
    {
        if (each.__isset.assayCode)
        {
            m_mapIseAssayCodeMaps.emplace(
                each.assayCode, std::make_shared<ise::tf::GeneralAssayInfo>(each));
        }
    }
}

void CommonInformationManager::ReloadAssayInfoIseSpecial()
{
    ULOG(LOG_INFO, __FUNCTION__);

    ::ise::tf::SpecialAssayInfoQueryCond qryCond;
    ::ise::tf::SpecialAssayInfoQueryResp qryResp;
    if (!ise::LogicControlProxy::QuerySpecialAssayInfo(qryResp, qryCond)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "Query ISE Special Assay Info Failed!");
        return;
    }

    m_mapIseSpecialAssayCodeMaps.clear();

    // 将结果保存到ISE特殊项目映射中
    for (const auto& each : qryResp.lstSpecialAssayInfos)
    {
        if (each.__isset.assayCode)
        {
            m_mapIseSpecialAssayCodeMaps.emplace(
                each.assayCode, std::make_shared<ise::tf::SpecialAssayInfo>(each));
        }
    }
}

void CommonInformationManager::ReloadGeneralProfile()
{
    ULOG(LOG_INFO, __FUNCTION__);

    m_mapProfileAssay.clear();

    ::tf::ProfileInfoQueryCond qryCond;
    ::tf::ProfileInfoQueryResp qryResp;
    if (!DcsControlProxy::GetInstance()->QueryProfileInfo(qryResp, qryCond)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "QueryProfileInfo() failed");
        return;
    }

    // 依次保存组合项目的信息到数组
    for (const auto& profile : qryResp.lstProfileInfos)
    {
        m_mapProfileAssay.emplace((int32_t)profile.id, std::make_shared<tf::ProfileInfo>(profile));
    }
}

void CommonInformationManager::ReloadSupplyAttributeCh()
{
    ULOG(LOG_INFO, __FUNCTION__);

    m_supplyAttributeMap.clear();

    ch::tf::SupplyAttributeQueryCond qryCond;
    ch::tf::SupplyAttributeQueryResp qryResp;
    if (!ch::LogicControlProxy::QuerySupplyAttribute(qryResp, qryCond)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "Failed to query all ch supplyattribute.");
        return;
    }

    // 依次保存耗材属性信息
    for (const auto& supplyAttribute : qryResp.lstSupplyAttributes)
    {
        m_supplyAttributeMap.emplace(supplyAttribute.type, std::make_shared<ch::tf::SupplyAttribute>(supplyAttribute));
    }
}

void CommonInformationManager::ReloadSupplyAttributeIse()
{
    ULOG(LOG_INFO, __FUNCTION__);

    m_iseSupplyAttributeMap.clear();

    ise::tf::SupplyAttributeQueryCond qryCond;
    ise::tf::SupplyAttributeQueryResp qryResp;
    if (!ise::LogicControlProxy::QuerySupplyAttribute(qryResp, qryCond)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "Failed to query all ise supplyattribute.");
        return;
    }

    // 依次保存耗材属性信息
    for (const auto& supplyAttribute : qryResp.lstSupplyAttributes)
    {
        m_iseSupplyAttributeMap.emplace(supplyAttribute.type, std::make_shared<ise::tf::SupplyAttribute>(supplyAttribute));
    }
}

///
/// @brief
///     初始化免疫耗材属性表（报警值）缓存
///
///
/// @return void
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月11日，新建函数
///
void CommonInformationManager::ReloadSupplyAttributeIm()
{
    ULOG(LOG_INFO, __FUNCTION__);

    m_ImSupplyAttributeMap.clear();

    im::tf::SupplyAttributeQueryCond qryCond;
    im::tf::SupplyAttributeQueryResp qryResp;
    if (!im::LogicControlProxy::QuerySupplyAttribute(qryResp, qryCond)
        || (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS))
    {
        ULOG(LOG_ERROR, "Failed to query all im supplyattribute.");
        return;
    }

    // 依次保存耗材属性信息
    for (const auto& supplyAttribute : qryResp.lstSupplyAttributes)
    {
        m_ImSupplyAttributeMap.emplace(supplyAttribute.type, std::make_shared<im::tf::SupplyAttribute>(supplyAttribute));
    }
}

void CommonInformationManager::UpdateDeviceBuff(const std::vector<tf::DeviceInfo>& vecDev)
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 依次更新设备信息; 
    // 此处暂时不直接clear后赋值，应为设备更新消息会调到这里，如果clear了，用到指针的地方会存在问题。
    // 所以m_mapDevices中保存的指针是不可靠的，此处进行赋值而不是替换不能完全消除其不可靠性

    std::set<std::string> unqSn;
    for (const tf::DeviceInfo& device : vecDev)
    {
        if (device.deviceType == ::tf::DeviceType::DEVICE_TYPE_C1000)
        {
            ChDeviceOtherInfo otherInfo;
            if (DecodeJson(otherInfo, device.otherInfo) && !otherInfo.bindIseSn.empty())
            {
                m_mapDevSnAndBindSn[device.deviceSN] = otherInfo.bindIseSn;
                m_mapDevSnAndBindSn[otherInfo.bindIseSn] = device.deviceSN;
            }
        }

        unqSn.insert(device.deviceSN);
        ULOG(LOG_INFO, "Device, name:%s, sn:%s, id:%lld.", device.name.c_str(), device.deviceSN.c_str(), device.id);

        DeviceMaps::iterator it = m_mapDevices.find(device.deviceSN);
        if (it == m_mapDevices.end())
        {
            m_mapDevices[device.deviceSN] = std::make_shared<tf::DeviceInfo>(device);
            ULOG(LOG_INFO, "New device info.");
        }
        else
        {
            if (it->second == nullptr)
            {
                it->second = std::make_shared<tf::DeviceInfo>(device);
            }
            else
            {
                *(it->second) = device;
            }
            ULOG(LOG_INFO, "Replace device info.");
        }
    }

    // 移除被删除了的设备信息
    DeviceMaps::iterator it = m_mapDevices.begin();
    do
    {
        if (it == m_mapDevices.end())
        {
            break;
        }
        if (unqSn.find(it->first) == unqSn.end())
        {
            std::string strname;
            std::string strsn;
            int64_t id = -1;
            if (it->second != nullptr)
            {
                strname = it->second->name;
                strsn = it->second->deviceSN;
                id = it->second->id;
            }
            ULOG(LOG_INFO, "Delete device, name:%s, sn:%s, id:%lld.", strname.c_str(), strsn.c_str(), id);

            it = m_mapDevices.erase(it);
            continue;
        }

        it++;

    } while (true);
}

void CommonInformationManager::InitChDeviceInfo()
{
	ULOG(LOG_INFO, __FUNCTION__);

	m_reagentLoaderMap.clear();
	m_chCabinetSuppliesInfoMap.clear();
	m_chReagentDiskInfoMap.clear();
	m_chWasteCntMap.clear();

	for (const auto& device : m_mapDevices)
	{
		// 生化设备
		if (device.second->deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
		{
			m_chReagentDiskInfoMap.insert({ device.first,{} });
			m_chCabinetSuppliesInfoMap.insert({ device.first,{} });
			
			// 查询该设备耗材信息
			ch::tf::SuppliesInfoQueryCond siQryCond;
			ch::tf::SuppliesInfoQueryResp siQryResp;
			siQryCond.__set_placeStatus({ ::tf::PlaceStatus::PLACE_STATUS_REGISTER
				, ::tf::PlaceStatus::PLACE_STATUS_LOAD, ::tf::PlaceStatus::PLACE_STATUS_UNLOADING });
			siQryCond.__set_deviceSN({ device.first });
			if (!ch::c1005::LogicControlProxy::QuerySuppliesInfo(siQryResp, siQryCond) 
				|| siQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_WARN, "ch::c1005::LogicControlProxy::QuerySuppliesInfo() failed");
				continue;
			}

			// 查询该设备的试剂组信息
			::ch::tf::ReagentGroupQueryCond rgtQryCond;
			::ch::tf::ReagentGroupQueryResp rgtQryResp;
			rgtQryCond.__set_deviceSN({ device.first });
			rgtQryCond.__set_beDeleted(false);
			if (!ch::c1005::LogicControlProxy::QueryReagentGroup(rgtQryResp, rgtQryCond) 
				|| rgtQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_WARN, "ch::c1005::LogicControlProxy::QueryReagentGroup() failed");
				continue;
			}

			// 遍历耗材信息
			for (const auto& si : siQryResp.lstSuppliesInfo)
			{
				// 仓内
				if (si.area == ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1)
				{
					std::vector<::ch::tf::ReagentGroup> vecReagentGroups;
					std::copy_if(rgtQryResp.lstReagentGroup.begin(), rgtQryResp.lstReagentGroup.end(), 
						std::back_inserter(vecReagentGroups), [=](::ch::tf::ReagentGroup group) { 
						return group.posInfo.pos == si.pos && group.posInfo.area == ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1
							&& group.suppliesCode == si.suppliesCode; });

					m_chReagentDiskInfoMap[device.first].insert({ si.pos, std::move(ChReagentInfo(vecReagentGroups, si)) });
				}
				// 仓外
				else if (si.area == ch::tf::SuppliesArea::SUPPLIES_AREA_CABINET)
				{
					m_chCabinetSuppliesInfoMap[device.first].insert({ si.pos, si });
				}
			}

			// 初始化生化设备加载器信息
			::ch::tf::ReagentLoaderInfo reagentLoader;
			::ch::c1005::LogicControlProxy::QueryReagentLoader(device.first, reagentLoader);
			m_reagentLoaderMap[QString::fromStdString(device.first)] = reagentLoader;

			// 初始化废液桶
			if (device.second->enableWasterContainer)
			{
				m_chWasteCntMap[device.first] = 0;
			}
		}
	}
}

void CommonInformationManager::InitIseDeviceInfo()
{
	ULOG(LOG_INFO, __FUNCTION__);

	for (const auto& device : m_mapDevices)
	{
		// 初始化ise设备耗材信息
		if (device.second->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
		{
			m_iseCabinetSuppliesInfoMap.insert({ device.first,{} });
						
			// 查询该设备耗材信息
			ise::tf::SuppliesInfoQueryCond siQryCond;
			ise::tf::SuppliesInfoQueryResp siQryResp;
			siQryCond.__set_placeStatus({ ::tf::PlaceStatus::PLACE_STATUS_REGISTER, ::tf::PlaceStatus::PLACE_STATUS_LOAD });
			siQryCond.__set_deviceSN({ device.first });
			ise::ise1005::LogicControlProxy::QuerySuppliesInfo(siQryResp, siQryCond);
			if (siQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_WARN, "ise::ise1005::LogicControlProxy::QuerySuppliesInfo() failed");
				continue;
			}

			// 遍历耗材信息
			for (const auto& si : siQryResp.lstSuppliesInfo)
			{
				m_iseCabinetSuppliesInfoMap[device.first].insert({ { si.moduleIndex, si.pos }, si });
			}
		}
	}
}

bool CommonInformationManager::GetDefaultDictData(int type, std::vector<CustomSetRowData>& csrData)
{
    bool ret = true;
    switch ((tf::PatientFields::type)type)
    {
        case tf::PatientFields::PATIENT_TYPE:
        {
            {
                CustomSetRowData data;
                data.strData = tr("常规").toUtf8().toStdString();
                csrData.push_back(data);
            }
            {
                CustomSetRowData data;
                data.strData = tr("急诊").toUtf8().toStdString();
                csrData.push_back(data);
            }
        }
        break;
        case tf::PatientFields::PATIENT_COST_TYPE:
        {
            {
                CustomSetRowData data;
                data.strData = tr("自费").toUtf8().toStdString();
                csrData.push_back(data);
            }
        }
        break;
        default:
            ret = false;
            break;
    }


    return ret;
}

///
/// @brief 根据告警码获取解释
///
/// @param[in]  codes  告警码
///
/// @return 告警详细解释
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月2日，新建函数
///
QString CommonInformationManager::GetResultStatusDetail(QString code)
{
	if (code.isEmpty())
	{
		return QString();
	}

	auto iter = m_dataAlarmMap.find(code.toStdString());
	if (m_dataAlarmMap.end() == iter)
	{
		return QString();
	}

	return  QString::fromStdString(iter->second.name);
}

///
/// @brief
///     设备是否温度异常
///
/// @param[in]  strDevSn  设备序列号
///
/// @return true表示温度异常
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月30日，新建函数
///
bool CommonInformationManager::IsDevTemperatureErr(const QString& strDevSn)
{
    //ULOG(LOG_INFO, "%s()", __FUNCTION__); // too frequency
    // 从缓存中查找
    auto it = m_deviceTemperatureErrMap.find(strDevSn);
    if (it != m_deviceTemperatureErrMap.end())
    {
        ULOG(LOG_INFO, "%s(), it != m_deviceTemperatureErrMap.end(), ret=%d", __FUNCTION__, int(it.value()));
        return it.value();
    }

    // 目前只支持i6000的判断
    auto devInfo = GetDeviceInfo(strDevSn.toStdString());
    if (devInfo == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), devInfo == Q_NULLPTR", __FUNCTION__);
        return false;
    }

    // 目前只支持i6000的判断
    if (devInfo->deviceType != tf::DeviceType::DEVICE_TYPE_I6000)
    {
        ULOG(LOG_INFO, "%s(), ignore, devInfo->deviceType != tf::DeviceType::DEVICE_TYPE_I6000", __FUNCTION__);
        return false;
    }

    // 如果缓存中没有找到，则查询对应状态
    ::tf::ResultBool qryResp;
    im::LogicControlProxy::QueryDeviceTemperatureErr(qryResp, strDevSn.toStdString());
    if (qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), QueryDeviceTemperatureErr failed!", __FUNCTION__);
        return false;
    }

    m_deviceTemperatureErrMap.insert(strDevSn, qryResp.value);
    return qryResp.value;
}

bool CommonInformationManager::UpdateDevicePoweronSchedul(const std::map<std::string, ::tf::PowerOnSchedule>& mapDevice2Pos)
{
    if (!DcsControlProxy::GetInstance()->UpdetePowerOnSchedule(mapDevice2Pos))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
        return false;
    }

    // 更新CommonAssay设备列表信息缓存中的计划
    for (const auto& it : mapDevice2Pos)
    {

        auto devIt = m_mapDevices.find(it.first);
        if (devIt != m_mapDevices.end())
        {
            devIt->second->__set_powerOnSchedule(it.second);
        }
    }
    return true;
}

bool CommonInformationManager::IsUnAllocatedAssay(const std::string &deviceSn, const int assayCode)
{
	std::unique_lock<std::mutex> buffLock(m_devMapMtx);

	auto iter = m_mapDevices.find(deviceSn);
	if (iter == m_mapDevices.end())
	{
		return false;
	}

	// 解码
	std::set<int> unAllocatedAssayCodes;
	if (!iter->second->unAllocatedAssayCodes.empty() && !DecodeJson(unAllocatedAssayCodes, iter->second->unAllocatedAssayCodes))
	{
		ULOG(LOG_ERROR, "DecodeJson() failed!");
		return false;
	}

	return unAllocatedAssayCodes.count(assayCode);
}

///
/// @brief 当前系统是否支持目标设备类型
///
/// @param[in]  devType  目标设备类型
///
/// @return true:支持该设备类型
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月22日，新建函数
///
bool CommonInformationManager::IsSupportAimDevType(const tf::DeviceType::type& devType)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	std::unique_lock<std::mutex> buffLock(m_devMapMtx);

	std::vector<std::shared_ptr<const tf::DeviceInfo>> devicelist;
	for (const auto& device : m_mapDevices)
	{
		if (device.second->deviceType == devType)
		{
			return true;
		}
	}

	return false;
}

std::vector<ch::tf::ReagentGroup> CommonInformationManager::GetAllChReagentGroupsByDevice(const std::vector<std::string>& devices)
{
    std::vector<ch::tf::ReagentGroup> result;

    for (const auto& chDev : devices)
    {
        auto iter = m_chReagentDiskInfoMap.find(chDev);
        if (iter == m_chReagentDiskInfoMap.end())
        {
            continue;
        }

        for (const auto &rgt : iter->second)
        {
            auto rgtGroups = rgt.second.reagentInfos;
            if (!rgtGroups.empty())
            {
                result.insert(result.end(), rgtGroups.begin(), rgtGroups.end());
            }
        }
    }

    return result;
}

std::vector<ch::tf::ReagentGroup> CommonInformationManager::GetChGroupReagents(const std::string& devSn, const int suppliesCode)
{
	std::vector<ch::tf::ReagentGroup> result;
	auto iter = m_chReagentDiskInfoMap.find(devSn);
	if (iter == m_chReagentDiskInfoMap.end())
	{
		return result;
	}

	for (const auto &rgt : iter->second)
	{
		auto rgtGroups = rgt.second.reagentInfos;
		if (!rgtGroups.empty() && rgtGroups[0].suppliesCode == suppliesCode)
		{
			result.insert(result.end(), rgtGroups.begin(), rgtGroups.end());
		}
	}

	return result;
}

std::vector<ch::tf::SuppliesInfo> CommonInformationManager::GetChGroupSupplies(const std::string& devSn, const int supplyCode)
{
	std::vector<ch::tf::SuppliesInfo> result;
	auto iter = m_chReagentDiskInfoMap.find(devSn);
	if (iter == m_chReagentDiskInfoMap.end())
	{
		return result;
	}

	for (const auto &supply : iter->second)
	{
		if (supply.second.supplyInfo.suppliesCode == supplyCode)
		{
			result.push_back(supply.second.supplyInfo);
		}
	}

	return result;
}

std::map<int, ChReagentInfo> CommonInformationManager::GetChDiskReagentSupplies(const std::string& devSn, const std::set<int> &setPos/* = {}*/)
{
	std::map<int, ChReagentInfo> result;
	auto iter = m_chReagentDiskInfoMap.find(devSn);
	if (iter == m_chReagentDiskInfoMap.end())
	{
		return result;
	}

	// 无位置参数返回所有
	if (setPos.empty())
	{
		return iter->second;
	}

	// 遍历位置
	for (const auto &pos : setPos)
	{
		auto it = iter->second.find(pos);
		if (it != iter->second.end())
		{
			result[pos] = it->second;
		}
	}

	return result;
}

bool CommonInformationManager::IsExistOnUsedReagentByCode(int code)
{
    for (auto& iter : m_chReagentDiskInfoMap)
    {
        for (auto& reagentItem : iter.second)
        {
            if (reagentItem.second.supplyInfo.type == ::ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
            {
                for (auto& reagent : reagentItem.second.reagentInfos)
                {
                    if (reagent.assayCode == code)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

std::map<int, ch::tf::SuppliesInfo> CommonInformationManager::GetChCabinetSupplies(const std::string& devSn, const std::set<int> &setPos/* = {}*/)
{
	std::map<int, ch::tf::SuppliesInfo> result;
	auto iter = m_chCabinetSuppliesInfoMap.find(devSn);
	if (iter == m_chCabinetSuppliesInfoMap.end())
	{
		return result;
	}

	// 无位置参数返回所有
	if (setPos.empty())
	{
		return iter->second;
	}

	// 遍历位置
	for (const auto &pos : setPos)
	{
		auto it = iter->second.find(pos);
		if (it != iter->second.end())
		{
			result[pos] = it->second;
		}
	}

	return result;
}

std::map<std::pair<int, int>, ise::tf::SuppliesInfo> CommonInformationManager::GetIseSupplies(const std::string& devSn,const std::set< std::pair<int, int>> &setPos/* = {}*/)
{
	std::map<std::pair<int, int>, ise::tf::SuppliesInfo> result;
	auto iter = m_iseCabinetSuppliesInfoMap.find(devSn);
	if (iter == m_iseCabinetSuppliesInfoMap.end())
	{
		return result;
	}

	// 无位置参数返回所有
	if (setPos.empty())
	{
		return iter->second;
	}

	// 遍历位置
	for (const auto &pos : setPos)
	{
		auto it = iter->second.find(pos);
		if (it != iter->second.end())
		{
			result[pos] = it->second;
		}
	}

	return result;
}

bool CommonInformationManager::CheckReagentRemainCount(const std::map<int, int>& assayAndTimes)
{
    // 遍历各设备上的试剂组信息，统计所有设备上的试剂可以测试数
    std::map<int, int> reagentAndRemainCount;
    for (const auto& iter: m_chReagentDiskInfoMap)
    {
        for (const auto& chriIter : iter.second)
        {
            for (const auto& rg : chriIter.second.reagentInfos)
            {
                if (reagentAndRemainCount.find(rg.assayCode) == reagentAndRemainCount.end())
                {
                    reagentAndRemainCount[rg.assayCode] = rg.remainCount > 0 ? rg.remainCount : 0;
                }
                else
                {
                    reagentAndRemainCount[rg.assayCode] += rg.remainCount > 0 ? rg.remainCount : 0;
                }
            }
        }
    }

    // 遍历项目，检查当前试剂的可用测试数是否满足项目的测试数
    for (const auto& iter : assayAndTimes)
    {
        const auto& reagentIter = reagentAndRemainCount.find(iter.first);
        if (reagentIter == reagentAndRemainCount.end()
            || reagentIter->second < iter.second)
        {
            return false;
        }
    }

    return true;
}

bool CommonInformationManager::updateReferenceRanges(int code, int sampleSourceType
	, const std::string& version, const ::tf::AssayReference& referrence)
{
    if (code >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE
        && code <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        for (const auto& iter : m_mapIseSpecialAssayCodeMaps)
        {
            auto& spAssayInfo = iter.second;

            // 编号相同/设备类型/版本类型/样本类型
            if (spAssayInfo->assayCode == code
                && spAssayInfo->version == version
                && spAssayInfo->sampleSourceType == sampleSourceType)
            {
                spAssayInfo->referenceRanges = referrence;
                return true;
            }
        }
    }
    else
    {
        for (const auto& iter : m_mapChSpecialAssayCodeMaps)
        {
            auto& spAssayInfo = iter.second;

            // 编号相同/设备类型/版本类型/样本类型
            if (spAssayInfo->assayCode == code
                && spAssayInfo->version == version
                && spAssayInfo->sampleSourceType == sampleSourceType)
            {
                spAssayInfo->referenceRanges = referrence;
                return true;
            }
        }
    }

    return false;
}

bool CommonInformationManager::CheckAssayNameExisted(int assayCode, const std::string& assayName) const
{
    // 项目的名称以及打印名称不能和常规项目、计算项目的项目名称以及打印名称重复
    for (const auto& iter : m_mapAssayNameMaps)
    {
        if ((iter.second->assayName == assayName || iter.second->assayFullName == assayName || iter.second->printName == assayName)
            && (iter.second->assayCode != assayCode))
        {
            return true;
        }
    }

    for (const auto& iter : m_calcAssayInfoMaps)
    {
        if ((iter.second->name == assayName || iter.second->printName == assayName || iter.second->printName == assayName)
            && (iter.second->assayCode != assayCode))
        {
            return true;
        }
    }

    return false;
}

void CommonInformationManager::UpdateQcRules(int64_t assayCode, const tf::QcJudgeParam& qcRule)
{
    // 更新远端的库
    if (DcsControlProxy::GetInstance()->SetOutQcRules(assayCode, qcRule))
    {
        // 更新本地缓存
        AssayIndexCodeMaps::iterator it = m_mapAssayCodeMaps.find(assayCode);
        if (it != m_mapAssayCodeMaps.end())
        {
            it->second->__set_qcRules(qcRule);
        }
    }
}

///
/// @brief
///     获取对应类型的设备列表
///
/// @param[in]  deviceType  设备类型
///
/// @return 设备信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月12日，新建函数
///
std::vector<std::shared_ptr<const tf::DeviceInfo>> CommonInformationManager::GetDeviceFromType(std::vector<DeviceType> deviceTypes)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    std::vector<std::shared_ptr<const tf::DeviceInfo>> devicelist;
    for (const auto& device : m_mapDevices)
    {
        if (Contains(deviceTypes, device.second->deviceType))
        {
            devicelist.push_back(device.second);
        }
    }

    return devicelist;
}

bool CommonInformationManager::IsDeviceExistedByAssayClassify(tf::AssayClassify::type deviceClassify)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    for (const auto& device : m_mapDevices)
    {
        if (deviceClassify == device.second->deviceClassify)
        {
            return true;
        }
    }

    return false;
}

std::vector<std::shared_ptr<const tf::DeviceInfo>> CommonInformationManager::GetDeviceByGroupName(const std::string& groupName)
{
    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    std::vector<std::shared_ptr<const tf::DeviceInfo>> devicelist;
    for (const auto& device : m_mapDevices)
    {
        if (device.second->groupName == groupName)
        {
            devicelist.push_back(device.second);
        }
    }

    return devicelist;
}

bool CommonInformationManager::HasIseDevice()
{
    std::unique_lock<std::mutex> buffLock(m_devMapMtx);
    auto& iter = std::find_if(m_mapDevices.begin(), m_mapDevices.end(), [&](auto& iter)
    {
        return iter.second->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005;
    });

    return iter != m_mapDevices.end();
}

///
/// @brief 获取当前项目对应的样本类型（此样本类型能够使用这个项目来做检测）
///
/// @param[in]  assayCode  项目编号
///
/// @return 样本类型列表
///
/// @par History:
/// @li 5774/WuHongTao，2023年7月19日，新建函数
///
void CommonInformationManager::GetSampleTypeFromCode(int32_t assayCode, std::set<int32_t>& outSampleTypes)
{
    std::shared_ptr<::tf::GeneralAssayInfo> pGeneralInfo = GetAssayInfo(assayCode);
    if (pGeneralInfo == nullptr)
    {
        return;
    }

    if (pGeneralInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        GetSampleTypeIm(assayCode, outSampleTypes);
    }
    else
    {
        GetSampleTypeCh(assayCode, outSampleTypes);
    }
}

///
/// @brief
///     更新项目配置信息
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月4日，新建函数
///
void CommonInformationManager::UpdateAssayCfgInfo(const QVector<int32_t>& assayCodes)
{
    ULOG(LOG_INFO, "%s(), parm size:%d.", __FUNCTION__, assayCodes.size());

    for (const int32_t assayCode : assayCodes)
    {
        // 通用项目配置信息
        ReloadGeneralAssayInfo(assayCode);

        // ise的自动质控设置同步
        if (assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE
            &&assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
        {
            // 遍历修改NA、K、CL
            for (int i = ::ise::tf::g_ise_constants.ASSAY_CODE_NA; i <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL; i++)
            {
                if (i == assayCode)
                {
                    continue;
                }

                // 通用项目配置信息
                ReloadGeneralAssayInfo(i);
            }
        }

        // 生化常规项目
        if ((assayCode >= ::tf::AssayCodeRange::CH_RANGE_MIN && assayCode < ::ch::tf::g_ch_constants.ASSAY_CODE_SIND)
            || assayCode == -1)
        {
            // 生化通用项目映射表
            ReloadAssayInfoCh(assayCode);

            // 生化特殊项目信息
            ReloadAssayInfoChSpecial(assayCode);
        }

        // SIND
        else if (assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND || assayCode == -1)
        {
            // 遍历修改L、H、I
            for (int i = ::ch::tf::g_ch_constants.ASSAY_CODE_SIND; i <= ::ch::tf::g_ch_constants.ASSAY_CODE_I; i++)
            {
                // 通用项目配置信息
                ReloadGeneralAssayInfo(i);

                // 生化通用项目映射表
                ReloadAssayInfoCh(i);

                // 生化特殊项目信息
                ReloadAssayInfoChSpecial(i);
            }

            // 生化通用项目映射表
            ReloadAssayInfoCh(::ch::tf::g_ch_constants.ASSAY_CODE_SIND);

            // 生化特殊项目信息
            ReloadAssayInfoChSpecial(::ch::tf::g_ch_constants.ASSAY_CODE_SIND);
        }

        // 免疫项目
        else if ((assayCode >= ::tf::AssayCodeRange::IM_RANGE_MIN && assayCode <= ::tf::AssayCodeRange::IM_RANGE_MAX)
            || assayCode == -1)
        {
            // 免疫通用项目映射表
            ReloadAssayInfoIm(assayCode);
        }

        // ISE
        else if ((assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE && assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
            || assayCode == -1)
        {
            // ISE通用项目映射表
            ReloadAssayInfoIse();

            // ISE的特殊项目
            ReloadAssayInfoIseSpecial();
        }
    }

    // 发送项目编码管理器更新消息
    POST_MESSAGE(MSG_ID_ASSAY_CODE_MANAGER_UPDATE);
}

///
/// @brief
///     更新组合项目信息
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月4日，新建函数
///
void CommonInformationManager::UpdateProfileInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 更新组合项目映射信息
/*    UpdateProfileReflection();*/

    // 发送项目编码管理器更新消息
    POST_MESSAGE(MSG_ID_ASSAY_CODE_MANAGER_UPDATE);
}

void CommonInformationManager::UpdateDeviceInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    InitDeviceReflection();
}

void CommonInformationManager::UpdateDeviceStatus(tf::DeviceInfo deviceInfo)
{
	ULOG(LOG_INFO, "%s(dev:%s, status:%d)", __FUNCTION__, deviceInfo.deviceSN, deviceInfo.__isset.status ? deviceInfo.status : -1);

	// 轨道:更新自身和子节点状态（在otherInfo内）
	if (deviceInfo.deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
	{
		if (deviceInfo.__isset.status || deviceInfo.__isset.otherInfo)
		{
			std::unique_lock<std::mutex> buffLock(m_devMapMtx);
			auto iter = m_mapDevices.find(deviceInfo.deviceSN);
			if (iter != m_mapDevices.end())
			{
				if (deviceInfo.__isset.status)
				{
					iter->second->status = deviceInfo.status;
				}
				if (deviceInfo.__isset.otherInfo)
				{
					iter->second->otherInfo = deviceInfo.otherInfo;
				}

				// 发送设备状态信息到UI消息总线
				POST_MESSAGE(MSG_ID_DEVS_STATUS_CHANGED, *(iter->second));
			}
		}
	}
	// 设备:更新设备或进样器状态
	else
	{
		if (deviceInfo.__isset.status || deviceInfo.__isset.iomStatus)
		{
			std::unique_lock<std::mutex> buffLock(m_devMapMtx);

			auto iter = m_mapDevices.find(deviceInfo.deviceSN);
			if (iter != m_mapDevices.end())
			{
				if (deviceInfo.__isset.status)
				{
					iter->second->status = deviceInfo.status;
				}
				if (deviceInfo.__isset.iomStatus)
				{
					iter->second->iomStatus = deviceInfo.iomStatus;
				}

				// 发送设备状态信息到UI消息总线
				POST_MESSAGE(MSG_ID_DEVS_STATUS_CHANGED, *(iter->second));
			}
		}
	}
}

void CommonInformationManager::UpdateDeviceCanAppendTest(QString devSN, std::set<enum tf::FaultAffectAppendTest::type,
	std::less<enum tf::FaultAffectAppendTest::type>, std::allocator<enum tf::FaultAffectAppendTest::type>> faats)
{
    std::string strDevSn = devSN.toStdString();
    ULOG(LOG_INFO, "%s() devSN:%s", __FUNCTION__, strDevSn);

    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    auto iter = m_mapDevices.find(strDevSn);
    if (iter != m_mapDevices.end())
    {
        iter->second->faats = faats;
    }

    POST_MESSAGE(MSG_ID_UPDATE_ASSAY_TEST_BTN);
}

bool CommonInformationManager::UpdateCalcAssayInfo(int calcAssayCode /*= -1*/)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, calcAssayCode);

    // 构造查询条件
    ::tf::CalcAssayInfoQueryCond qryCond;
    if (calcAssayCode > 0)
    {
        qryCond.__set_assayCode(calcAssayCode);
    }

    ::tf::CalcAssayInfoQueryResp qryResp;

    // 查询
    if (!DcsControlProxy::GetInstance()->QueryCalcAssayInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Failed to execute QueryCalcAssayInfo()");
        return false;
    }

    // 判断结果是否为空
    if (qryResp.lstCalcAssayInfo.empty())
    {
        m_calcAssayInfoMaps.erase(calcAssayCode);
		// 删除之前的项目位置信息
		m_mapAssayCodeMaps.erase(calcAssayCode);
        return true;
    }

	// 找到计算项目的显示位置（按照要求计算项目显示位于目前子项目的最后位置+1）
	auto findCalcShowPostion = [&](const tf::CalcAssayInfo& calcInfo)->int
	{
		int postion = -1;
		for (const auto& subCode : calcInfo.vecAssayCode)
		{
			// 只要有项目无法找到
			auto iter = m_defaultPostionMap.find(subCode);
			if (iter == m_defaultPostionMap.end())
			{
				// 最大项目位置
				return UI_ASSAYMAXPOSTION;
			}

			if (iter->second > postion)
			{
				postion = iter->second;
			}
		}

		// 项目位置+1
		return (postion + 1);
	};

    // 将结果保存到项目映射中
    for (const tf::CalcAssayInfo& calcAssayInfo : qryResp.lstCalcAssayInfo)
    {
        m_calcAssayInfoMaps[calcAssayInfo.assayCode] = std::make_shared<tf::CalcAssayInfo>(calcAssayInfo);
		m_defaultPostionMap[calcAssayInfo.assayCode] = findCalcShowPostion(calcAssayInfo);
    }

    POST_MESSAGE(MSG_ID_ASSAY_CODE_MANAGER_UPDATE);
    return true;
}

void CommonInformationManager::UpdateSupplyAttribute()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ReloadSupplyAttributeCh();
    ReloadSupplyAttributeIm();
    ReloadSupplyAttributeIse();

    // 耗材提醒更新
    POST_MESSAGE(MSG_ID_MANAGER_UPDATE_SUPPLY);
}

///
/// @brief 更新数据报警ｍａｐ
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月2日，新建函数
///
void CommonInformationManager::UpdateDataAlarmMap()
{
	std::vector<::tf::DataAlarmItem> dataAlarmVec;
	DcsControlProxy::GetInstance()->QueryDataAlarm(dataAlarmVec, "ALL");
	for (const auto& data : dataAlarmVec)
	{
		m_dataAlarmMap[data.code] = data;
	}

	QSystemMonitor::GetInstance().UpdateIseCaliModuleSatus();
}

///
/// @brief
///     设备温度异常状态改变
///
/// @param[in]  strDevSn    设备序列号
/// @param[in]  bErr        是否温度异常
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月30日，新建函数
///
void CommonInformationManager::OnDevTemperatureErrChanged(QString strDevSn, bool bErr)
{
    ULOG(LOG_INFO, "%s(%s, %d)", __FUNCTION__, strDevSn.toStdString(), int(bErr));
    // 更新map
    m_deviceTemperatureErrMap.insert(strDevSn, bErr);
}

void CommonInformationManager::OnUpdateCaliAppSatus(class std::vector<class ise::tf::IseModuleInfo, class std::allocator<class ise::tf::IseModuleInfo>> iseAppUpdates)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(iseAppUpdates));
	// 更新ISE设备的状态信息
	for (const auto& moduleInfo : iseAppUpdates)
	{
		auto spDeviceInfo = std::make_shared<ise::tf::IseModuleInfo>(moduleInfo);
		m_iseModuleMaps[spDeviceInfo->id] = spDeviceInfo;
	}

	// 更新ISE的校准状态信息（样本申请页面）
	QSystemMonitor::GetInstance().UpdateIseCaliModuleSatus();
}

void CommonInformationManager::OnReagentAlarmValueChanged()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 更新内存
	ReloadGeneralAssayInfo();
	// 抛出消息
	POST_MESSAGE(MSG_ID_MANAGER_UPDATE_REAGENT);
}

void CommonInformationManager::OnChReagentLoaderInfoUpdate(QString deviceSN, class ch::tf::ReagentLoaderInfo loaderUpdate)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 更新内存
	if (m_reagentLoaderMap.contains(deviceSN))
	{
		m_reagentLoaderMap[deviceSN] = loaderUpdate;
	}

	// 抛出消息
	POST_MESSAGE(MSG_ID_MANAGER_REAGENT_LOADER_UPDATE,deviceSN);
}

void CommonInformationManager::OnUpdateChReagentSupply(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates)
{
	ULOG(LOG_INFO, "%s(supplyUpdates=%s)", __FUNCTION__, ToString(supplyUpdates));

	// 依次更新生化试剂&耗材信息
	for (auto supplyUpdate : supplyUpdates)
	{
        std::string devSn = supplyUpdate.deviceSN;
		int pos = supplyUpdate.posInfo.pos;

		ch::tf::SuppliesInfoQueryCond qrySuppliesCond;
		ch::tf::SuppliesInfoQueryResp qrySuppliesResp;
		qrySuppliesCond.__set_placeStatus({ ::tf::PlaceStatus::PLACE_STATUS_REGISTER
			, ::tf::PlaceStatus::PLACE_STATUS_LOAD, ::tf::PlaceStatus::PLACE_STATUS_UNLOADING });
		qrySuppliesCond.__set_area(supplyUpdate.posInfo.area);
		qrySuppliesCond.__set_pos(pos);
		qrySuppliesCond.__set_deviceSN({ devSn });

		// 查找对应设备的耗材信息
		if (!ch::c1005::LogicControlProxy::QuerySuppliesInfo(qrySuppliesResp, qrySuppliesCond) 
			|| qrySuppliesResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_WARN, "ch::c1005::LogicControlProxy::QuerySuppliesInfo() failed");
			continue;
		}

		// 是否已卸载
		bool bUnload = qrySuppliesResp.lstSuppliesInfo.empty();

		// 该位置已卸载
		ULOG(LOG_INFO, u8"该位置是否卸载=%d, pos=%d.", bUnload, pos);

		// 仓内
		if (supplyUpdate.posInfo.area == ch::tf::SuppliesArea::type::SUPPLIES_AREA_REAGENT_DISK1)
		{
			auto iter = m_chReagentDiskInfoMap.find(devSn);
			if (iter == m_chReagentDiskInfoMap.end())
			{
				ULOG(LOG_INFO, "not care device:%s", devSn);
				continue;
			}

			if (bUnload)
			{
				if (iter->second.count(pos))
				{
					iter->second.erase(pos);
				}
			}
			else
			{
				std::vector<::ch::tf::ReagentGroup> vecReagentGroups;
				auto si = qrySuppliesResp.lstSuppliesInfo[0];

				// 如果是试剂那么需要查询试剂组
				if (si.type == ::ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
				{
					// 查询该设备的试剂组信息
					::ch::tf::ReagentGroupQueryCond rgtQryCond;
					::ch::tf::ReagentGroupQueryResp rgtQryResp;
					rgtQryCond.__set_suppliesCode(si.suppliesCode);
					rgtQryCond.__set_deviceSN({ devSn });
					rgtQryCond.__set_beDeleted(false);
					rgtQryCond.__set_posInfo(supplyUpdate.posInfo);
					if (!ch::c1005::LogicControlProxy::QueryReagentGroup(rgtQryResp, rgtQryCond)
						|| rgtQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
					{
						ULOG(LOG_WARN, "ch::c1005::LogicControlProxy::QueryReagentGroup() failed");
						continue;
					}

					vecReagentGroups = rgtQryResp.lstReagentGroup;

					// 打印日志
					for (const auto &reagentInfo : vecReagentGroups)
					{
						ULOG(LOG_INFO, u8"查询试剂组信息: id:%lld, deviceSn:%s, beDeleted:%d, pos:%d, caliStatus:%d,"\
							"caliReason:%d, caliMode:%d, caliSelect:%d, qcStatus:%d", reagentInfo.id, reagentInfo.deviceSN, 
							reagentInfo.beDeleted, reagentInfo.posInfo.pos, reagentInfo.caliStatus, reagentInfo.caliReason, 
							reagentInfo.caliMode, reagentInfo.caliSelect, reagentInfo.qcStatus);
					}
				}

				iter->second[pos] = std::move(ChReagentInfo(vecReagentGroups, si));
			}
		}
		// 仓外
		else if (supplyUpdate.posInfo.area == ch::tf::SuppliesArea::type::SUPPLIES_AREA_CABINET)
		{
			auto iter = m_chCabinetSuppliesInfoMap.find(devSn);
			if (iter == m_chCabinetSuppliesInfoMap.end())
			{
				ULOG(LOG_INFO, "not care device:%s", devSn);
				continue;
			}

			if (bUnload)
			{
				if (iter->second.count(pos))
				{
					iter->second.erase(pos);
				}
			}
			else
			{
				iter->second[pos] = qrySuppliesResp.lstSuppliesInfo[0];
			}
		}
	}

	// 发送生化设备试剂&耗材更新消息
	POST_MESSAGE(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, supplyUpdates);
}

void CommonInformationManager::OnUpdateISeSupply(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates)
{
	// 依次更新ise耗材信息
	for (const auto& supplyUpdate : supplyUpdates)
	{
        std::string devSn = supplyUpdate.suppliesInfo.deviceSN;
		int pos = supplyUpdate.suppliesInfo.pos;
		int idx = supplyUpdate.suppliesInfo.moduleIndex;
		ise::tf::SuppliesInfoQueryCond qrySuppliesCond;
		ise::tf::SuppliesInfoQueryResp qrySuppliesResp;
		qrySuppliesCond.__set_placeStatus({ ::tf::PlaceStatus::PLACE_STATUS_REGISTER, ::tf::PlaceStatus::PLACE_STATUS_LOAD });
		qrySuppliesCond.__set_moduleIndex(idx);
		qrySuppliesCond.__set_pos(pos);
		qrySuppliesCond.__set_deviceSN({ devSn });
		qrySuppliesCond.__set_suppliesCode(supplyUpdate.suppliesInfo.suppliesCode);
		ise::ise1005::LogicControlProxy::QuerySuppliesInfo(qrySuppliesResp, qrySuppliesCond);
		if (qrySuppliesResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_WARN, "ise::ise1005::LogicControlProxy::QuerySuppliesInfo() failed");
			continue;
		}

		// 是否已卸载
		bool bUnload = qrySuppliesResp.lstSuppliesInfo.empty();
		auto iter = m_iseCabinetSuppliesInfoMap.find(devSn);
		if (iter == m_iseCabinetSuppliesInfoMap.end())
		{
			ULOG(LOG_INFO, "not care device:%s", devSn);
			continue;
		}

        std::pair<int, int> key{ idx, pos };
		if (bUnload)
		{
			if (iter->second.count(key))
			{
				iter->second.erase(key);
			}
		}
		else
		{
			iter->second[key] = qrySuppliesResp.lstSuppliesInfo[0];
		}
	}

	// 发送ISE设备耗材更新消息
	POST_MESSAGE(MSG_ID_ISE_SUPPLY_INFO_UPDATE, supplyUpdates);
}

void CommonInformationManager::OnUpdateBucketStatus(class tf::UnitStatusInfo statusInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 设置了废液桶才更新
	if (statusInfo.__isset.wasteCnt)
	{
		ULOG(LOG_INFO, u8"废液桶状态变化: %s(%s)", __FUNCTION__, ToString(statusInfo));
		if (m_chWasteCntMap.count(statusInfo.devSn))
		{
			m_chWasteCntMap[statusInfo.devSn] = statusInfo.wasteCnt;

			// 发送废液桶更新消息
			POST_MESSAGE(MSG_ID_CH_WASTE_CNT_UPDATE, QString::fromStdString(statusInfo.devSn));
		}
	}
}

void CommonInformationManager::SetSingleAssayRowAmount(int assayRowAmount)
{
    m_singleAssayRowAmount = assayRowAmount;
    DcsControlProxy::GetInstance()->SetAssayTabSignleRowAmount(assayRowAmount);
}

QString CommonInformationManager::GetDilutionFactor(std::shared_ptr<ch::tf::AssayTestResult> spAssayTestResult)
{
    QString dilutionFactor = " ";
    if (spAssayTestResult == nullptr)
        return dilutionFactor;

    if (spAssayTestResult->dilutionFactor > 1)
    {
        dilutionFactor = (QString::number(spAssayTestResult->dilutionFactor));
    }
    else
    {
        return ThriftEnumTrans::GetSuckVolType((tf::SuckVolType::type)spAssayTestResult->suckVolType, " ");
    }

    return dilutionFactor;
}

///
/// @brief	通过计算项目编号,病人信息获取计算项目结果参考区间
///     
/// @param[in]  assayCode  计算项目编号
///
/// @return 计算项目信息
///
/// @par History:
/// @li 6889/ChenWei，2023年11月22日，新建函数
///
bool CommonInformationManager::GetCalcAssayResultReference(int assayCode, int sourceType, std::shared_ptr<tf::PatientInfo> pPatientInfo, tf::AssayReferenceItem& matchRef)
{
    std::shared_ptr<::tf::CalcAssayInfo> calcItem = GetCalcAssayInfo(assayCode);
    if (calcItem == nullptr)
        return false;

    // 初始化
    int iAge = -1;
    ::tf::Gender::type eGender = ::tf::Gender::GENDER_UNKNOWN;
    ::tf::AgeUnit::type ePatientAgeUnit = ::tf::AgeUnit::AGE_UNIT_YEAR;
    auto eSampleSource = (tf::SampleSourceType::type)sourceType;

    // 如果样本有病人信息，那么匹配病人信息
    if (pPatientInfo != nullptr)
    {
        eGender = (::tf::Gender::type)pPatientInfo->gender;
        iAge = pPatientInfo->age;
        ePatientAgeUnit = (::tf::AgeUnit::type)pPatientInfo->ageUnit;
    }

    // 遍历参考范围，依次与每一条参考范围对比
    bool bFind = false;
    for (const tf::AssayReferenceItem& tempRef : calcItem->vecRefRange.Items)
    {
        // 如果存在缺省参考范围
        if (tempRef.bAutoDefault && !bFind)
        {
            bFind = true;
            matchRef = tempRef;
            continue;
        }

        // 性别不符
        if ((tempRef.enGender != ::tf::Gender::GENDER_UNKNOWN) && (eGender != tempRef.enGender))
        {
            continue;
        }
        // 样本类型不符
        else if ((tempRef.sampleSourceType >= 0) && (tempRef.sampleSourceType != eSampleSource))
        {
            continue;
        }

        // 年龄不符合跳过
        int iStartAge = tempRef.iLowerAge;
        int iEndAge = tempRef.iUpperAge;
        // 参考区间未设置年龄，且病人信息也未设置年龄
        if (iStartAge <= 0 && iEndAge <= 0)
        {
            bFind = true;
        }
        else
        {
            double dStartAge = ConvertAge(iStartAge, tempRef.enAgeUnit, ePatientAgeUnit);// 年龄 统一单位
            double dEndAge = ConvertAge(iEndAge, tempRef.enAgeUnit, ePatientAgeUnit);
            if (iAge <= dEndAge && iAge >= dStartAge)
            {
                // 判定结果是否符合
                bFind = true;
            }
            else
            {
                // 此处增加循环跳过，否则若第一个参考范围为缺省值，
                // 只要后面的参考范围样本、性别和测试项目相符，就会被标记为新的参考范围
                continue;
            }
        }

        // 保存找到的数据(可能先找到了非缺省的未知
        // 性别，后面的参考范围可能还能精确匹配到性别)
        // PS:带来的问题是，如果非缺省未知性别如果有
        // 年龄区间重叠，可能会匹配最后一个，而非第一个
        if (bFind)
        {
            matchRef = tempRef;
        }

        //如果找到的数据性别能精确匹配，那么直接跳出
        if (bFind && (eGender == tempRef.enGender))
        {
            break;
        }
    }

    return bFind;
}

///
/// @brief  转换年龄单位
///
/// @param[in]  iAge        现有单位对应年龄
/// @param[in]  eAgeUnit    当前单位，参见 ::tf::AgeUnit::type 类型
/// @param[in]  eExpectedAgeUnit  目标单位，参见 ::tf::AgeUnit::type 类型
///
/// @return 转换后的年龄，小于0表示转换失败
///
/// @par History:
/// @li 6889/ChenWei，2023年11月22日，新建函数
///
double CommonInformationManager::ConvertAge(int iAge, int eAgeUnit, int eExpectedAgeUnit)
{
    switch (eAgeUnit)
    {
    case ::tf::AgeUnit::AGE_UNIT_YEAR:
        if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_MONTH)
        {
            return iAge * 12;
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_DAY)
        {
            return iAge * 365;
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_HOUR)
        {
            return iAge * 365 * 24;
        }
        break;

    case ::tf::AgeUnit::AGE_UNIT_MONTH:
        if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_YEAR)
        {
            return iAge / 12.0;
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_DAY)
        {
            return iAge * 30;
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_HOUR)
        {
            return iAge * 30 * 24;
        }
        break;

    case ::tf::AgeUnit::AGE_UNIT_DAY:
        if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_YEAR)
        {
            return iAge / 365.0;
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_MONTH)
        {
            return iAge / 30.0;
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_HOUR)
        {
            return iAge * 24;
        }
        break;

    case ::tf::AgeUnit::AGE_UNIT_HOUR:
        if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_YEAR)
        {
            return iAge / (365.0 * 24);
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_MONTH)
        {
            return iAge / (30.0 * 24);
        }
        else if (eExpectedAgeUnit == ::tf::AgeUnit::AGE_UNIT_DAY)
        {
            return iAge / 24.0;
        }
        break;

    default:
        // 如果有没匹配到年龄单位，则返回-1
        return -1;
        break;
    }

    // eAgeUnit和eExpectedAgeUnit相同时,
    // 不需要转换，直接返回自身
    return iAge;
}

void CommonInformationManager::GetCalcAssayInfo(std::vector<std::shared_ptr<tf::CalcAssayInfo>>& calcAssayInfo)
{
    calcAssayInfo.clear();
    for (CalcAssayInfoMap::iterator it = m_calcAssayInfoMaps.begin(); it != m_calcAssayInfoMaps.end(); ++it)
    {
        calcAssayInfo.push_back(it->second);
    }
}

std::shared_ptr<tf::CalcAssayInfo> CommonInformationManager::GetCalcAssayInfoById(int calcItemId)
{
    for (CalcAssayInfoMap::iterator it = m_calcAssayInfoMaps.begin(); it != m_calcAssayInfoMaps.end(); ++it)
    {
        if (it->second->id == calcItemId)
        {
            return it->second;
        }
    }

    return nullptr;
}

bool CommonInformationManager::ModifyCalcAssayInfo(const tf::CalcAssayInfo& calcItem)
{
    // 修改
    if (!DcsControlProxy::GetInstance()->ModifyCalcAssayInfo(calcItem))
    {
        ULOG(LOG_ERROR, "ModifyCalcAssayInfo Failed !");
        return false;
    }

    // 更新缓存
    UpdateCalcAssayInfo();

    return true;
}

bool CommonInformationManager::AddCalcAssayInfo(const tf::CalcAssayInfo& calcItem)
{
    // 保存到库
    ::tf::ResultLong ret;
    if (!DcsControlProxy::GetInstance()->AddCalcAssayInfo(ret, calcItem)
        || ret.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "AddCalcAssayInfo Failed !");
        return false;
    }

    // 更新缓存
    UpdateCalcAssayInfo(calcItem.assayCode);

    return true;
}

bool CommonInformationManager::DeleteCalcAssayInfoById(int calcId, int calcCode)
{
    // 删除当前行
    ::tf::CalcAssayInfoQueryCond caiqc;
    caiqc.__set_id(calcId);
    caiqc.__set_assayCode(calcCode);

    if (!DcsControlProxy::GetInstance()->DeleteCalcAssayInfo(caiqc))
    {
        return false;
    }

    // 更新缓存
    UpdateCalcAssayInfo(calcCode);

    return true;
}

bool CommonInformationManager::IsExistAssayCodeInCalcAssayInfo(int assayCode, std::vector<std::string>& calAssayName)
{
    for (CalcAssayInfoMap::iterator it = m_calcAssayInfoMaps.begin(); it != m_calcAssayInfoMaps.end(); ++it)
    {
        for (const auto& aItem : it->second->vecAssayCode)
        {
            if (aItem == assayCode)
            {
                calAssayName.push_back(it->second->name);
                break;
            }
        }
    }

    return calAssayName.size() > 0;
}

std::set<int> CommonInformationManager::GetDifferentMap(const std::map<int, std::set<int>>& firstSet, const std::map<int, std::set<int>>& secondSet)
{
	std::set<int> maskCodeMap;
	for (const auto& subMap : firstSet)
	{
		if (secondSet.count(subMap.first) > 0)
		{
			std::set_difference(subMap.second.begin(), subMap.second.end(),
				secondSet.at(subMap.first).begin(), secondSet.at(subMap.first).end(),
				std::inserter(maskCodeMap, maskCodeMap.begin()));

			std::set_difference(secondSet.at(subMap.first).begin(), secondSet.at(subMap.first).end(),
				subMap.second.begin(), subMap.second.end(),
				std::inserter(maskCodeMap, maskCodeMap.begin()));
		}
		else
		{
			maskCodeMap.insert(subMap.second.begin(), subMap.second.end());
		}
	}

	for (const auto& subMap : secondSet)
	{
		if (firstSet.count(subMap.first) <= 0)
		{
			maskCodeMap.insert(subMap.second.begin(), subMap.second.end());
		}
	}

	return std::move(maskCodeMap);
}

QString CommonInformationManager::GetTipsContent(const bool isAiEnable)
{
	bool isAi = false;
	// 只有在联机模式下才有AI
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		isAi = isAiEnable;
	}

	if (isAi)
	{
		QStringList tips;
		tips << tr("<li style='line-height: 30px;'>O:待测</li>")
			<< tr("<li style='line-height: 30px;'>P:检测中</li>")
			<< tr("<li style='line-height: 30px;'>F:已完成</li>")
			<< tr("<li style='line-height: 30px;'>R:复查</li>")
			<< tr("<li style='line-height: 30px;'>H:已传输</li>")
			<< tr("<li style='line-height: 30px;'>S:视觉识别</li>");
		auto tip = tips.join("\n");
		return tip;
	}
	else
	{
		QStringList tips;
		tips << tr("<li style='line-height: 30px;'>O:待测</li>")
			<< tr("<li style='line-height: 30px;'>P:检测中</li>")
			<< tr("<li style='line-height: 30px;'>F:已完成</li>")
			<< tr("<li style='line-height: 30px;'>R:复查</li>")
			<< tr("<li style='line-height: 30px;'>H:已传输</li>");
		auto tip = tips.join("\n");
		return tip;
	}
}

///
/// @brief 获取项目名称(包括计算项目)
///
/// @param[in]  iAssayCode  项目通道号
///
/// @return 项目名称
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月3日，新建函数
///
QString CommonInformationManager::GetAssayNameByCodeEx(int iAssayCode)
{
    // 判断是否计算项目
    if (iAssayCode >= tf::AssayCodeRange::CALC_RANGE_MIN && \
        iAssayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
    {
        std::shared_ptr<::tf::CalcAssayInfo> calcItem = GetCalcAssayInfo(iAssayCode);
        std::string strCalc = (calcItem == nullptr) ? "" : calcItem->name;
        return QString::fromStdString(strCalc);
    }
    else
    {
        return QString::fromStdString(GetAssayNameByCode(iAssayCode));
    }
}

QString CommonInformationManager::GetResultStatusLHIbyCode(int assayCode, int statusCode)
{
	QString resultString;
	const auto iter = m_mapChSpecialAssayCodeMaps.find(assayCode);
	if (iter == m_mapChSpecialAssayCodeMaps.end())
	{
		return resultString;
	}

	const auto qualitativeJudge = iter->second->qualitativeJudge;
	if (!qualitativeJudge.bUsing)
	{
		return resultString;
	}

	if (statusCode < 0 || statusCode >= qualitativeJudge.vecItems.size())
	{
		return resultString;
	}

	const auto qualitative = qualitativeJudge.vecItems[statusCode];
	return QString::fromStdString(qualitative.tag);

}

///
/// @brief 获取项目打印名称
///
/// @param[in]  assayCode  项目通道号
///
/// @return 项目打印名称
///
/// @par History:
/// @li 1226/zhangjing，2023年7月18日，新建函数
///
QString CommonInformationManager::GetPrintAssayName(int iAssayCode)
{
    // 判断是否计算项目
    if (iAssayCode >= tf::AssayCodeRange::CALC_RANGE_MIN && \
        iAssayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
    {
        std::shared_ptr<::tf::CalcAssayInfo> calcItem = GetCalcAssayInfo(iAssayCode);
        std::string strCalc = (calcItem == nullptr) ? "" : calcItem->printName;
        return QString::fromStdString(strCalc);
    }
    else
    {
        std::shared_ptr<tf::GeneralAssayInfo> spAssay = GetAssayInfo(iAssayCode);
        std::string strName = (spAssay == nullptr) ? "" : spAssay->printName;
        return QString::fromStdString(strName);
    }
}

bool CommonInformationManager::ModifyAssayShieldStatusCodes(int assayCode
	, const std::string& version, const std::vector<std::string>& shieldStatusCodes)
{
    if (assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE && assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
    {
        // 查询ise通用项目信息
        auto spGai = GetIseAssayInfo(assayCode, tf::DeviceType::DEVICE_TYPE_ISE1005, version);
        if (spGai == nullptr)
        {
            ULOG(LOG_ERROR, "GetIseSpecialAssayInfo() failed!");
            return false;
        }

        // 保存配置信息
        ::ise::tf::GeneralAssayInfo gai;
        gai.__set_id(spGai->id);
        gai.__set_lstShieldStatusCodes(shieldStatusCodes);

        if (ise::LogicControlProxy::ModifyAssayConfigInfo(gai))
        {
            spGai->__set_lstShieldStatusCodes(shieldStatusCodes);
            return true;
        }
    }
    else
    {
        // 查询ch特殊项目信息
        auto spGai = GetChemistryAssayInfo(assayCode, tf::DeviceType::DEVICE_TYPE_C1000, version);
        if (nullptr == spGai)
        {
            ULOG(LOG_ERROR, "GetIseSpecialAssayInfo() failed!");
            return false;
        }

        // 保存配置信息
        ::ch::tf::GeneralAssayInfo gai;
        gai.__set_id(spGai->id);
        gai.__set_lstShieldStatusCodes(shieldStatusCodes);

        if (ch::LogicControlProxy::ModifyAssayConfigInfo(gai))
        {
            spGai->__set_lstShieldStatusCodes(shieldStatusCodes);
            return true;
        }
    }

    return false;
}

std::shared_ptr<ch::tf::GeneralAssayInfo> CommonInformationManager::GetChemistryAssayInfo(int iAssayCode
	, ::tf::DeviceType::type type, const std::string& version)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);
    // 查找项目编号对应的项目信息
    auto range = m_mapChAssayCodeMaps.equal_range(iAssayCode);
    for (auto itr = range.first; itr != range.second; itr++)
    {
        // 类型是否一致
        if (type != itr->second->deviceType)
        {
            continue;
        }
        
        // 指定参数版本则获取指定的版本，未指定版本则获取启用的版本
        if ((!version.empty() && itr->second->version != version)
            || (version.empty() && !itr->second->enable))
        {
            continue;
        }

        return itr->second;
    }

    // 否则返回空
    return nullptr;
}

std::vector<std::shared_ptr<ch::tf::GeneralAssayInfo>> CommonInformationManager::GetAllChemistryAssayInfo(int iAssayCode, ::tf::DeviceType::type type)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);
	std::vector<std::shared_ptr<ch::tf::GeneralAssayInfo>> ChGAIs;

	// 查找项目编号对应的项目信息
	auto range = m_mapChAssayCodeMaps.equal_range(iAssayCode);
	for (auto itr = range.first; itr != range.second; itr++)
	{
		// 类型是否一致
		if (type != itr->second->deviceType)
		{
			continue;
		}

		ChGAIs.push_back(itr->second);
	}

	// 否则返回空
	return ChGAIs;

}

std::set<std::string> CommonInformationManager::GetAssayVersionList(int iAssayCode, ::tf::DeviceType::type type)
{
    // 查找项目编号对应的项目信息
    std::set<std::string> ret;
    auto range = m_mapChAssayCodeMaps.equal_range(iAssayCode);
    for (auto itr = range.first; itr != range.second; itr++)
    {
        // 类型是否一致
        if (type != itr->second->deviceType)
        {
            continue;
        }

        ret.insert(itr->second->version);
    }

    return std::move(ret);
}

///
/// @brief 获取免疫项目当前使用的项目的详细信息
///
/// @param[in]  iAssayCode  项目编号
/// @param[in]  type        机器类型
///
/// @return 项目详细信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月8日，新建函数
///
std::shared_ptr<im::tf::GeneralAssayInfo> CommonInformationManager::GetImmuneAssayInfo(int iAssayCode, ::tf::DeviceType::type devType, const std::string& ver)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);

    // 查找项目编号对应的项目信息
    auto range = m_mapImAssayCodeMaps.equal_range(iAssayCode);
    std::shared_ptr<im::tf::GeneralAssayInfo> pt = nullptr;

    bool isUsingFlag = false;
    for (auto itr = range.first; itr != range.second; itr++)
    {
        // 类型是否一致
        if (itr->second == nullptr || devType != itr->second->deviceType)
        {
            continue;
        }

        if (ver.empty())
        {
            // 如果存在在用版本，直接使用在用版本
            if (!isUsingFlag && itr->second->usingFlag)
            {
                pt = itr->second;
                isUsingFlag = true;
                continue;
            }
            // 是否最高版本
            if (!isUsingFlag && (pt == nullptr || (atof(pt->version.c_str()) < atof(itr->second->version.c_str()))))
            {
                pt = itr->second;
            }
        }
        else
        {
            // 指定版本
            if (itr->second->version == ver)
            {
                pt = itr->second;
                break;
            }
        }
    }

    // 否则返回空
    return pt;
}

///
/// @brief 获取免疫项目当前使用的项目的详细信息队列
///
/// @param[in]  iAssayCode  项目编号
/// @param[in]  type        机器类型
///
/// @return 项目详细信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月8日，新建函数
///
void CommonInformationManager::GetImmuneAssayInfos(std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& targetAssys, int iAssayCode, ::tf::DeviceType::type type)
{
    // ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode); // too frequency.
    targetAssys.clear();

    // 查找项目编号对应的项目信息
    auto range = m_mapImAssayCodeMaps.equal_range(iAssayCode);
    for (auto itr = range.first; itr != range.second; ++itr)
    {
        // 类型是否一致
        if (type != itr->second->deviceType)
        {
            continue;
        }
        targetAssys.push_back(itr->second);
    }
}

///
/// @brief 获取ISE项目当前使用的项目的详细信息
///
/// @param[in]  iAssayCode  项目编号
/// @param[in]  type		机器类型
/// @param[in]  version		参数版本（-1则不限制）
///
/// @return 项目详细信息
///
/// @par History:
/// @li 7951/LuoXin，2022年6月28日，新建函数
///
std::shared_ptr<ise::tf::GeneralAssayInfo> CommonInformationManager::GetIseAssayInfo(int iAssayCode
	, ::tf::DeviceType::type type, const std::string& version /*=""*/)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);
    // 查找项目编号对应的项目信息
    auto range = m_mapIseAssayCodeMaps.equal_range(iAssayCode);
    for (auto itr = range.first; itr != range.second; itr++)
    {
        // 类型是否一致
        if (type != itr->second->deviceType)
        {
            continue;
        }

        // 是否启用
        if (!itr->second->enable)
        {
            continue;
        }

        if (!version.empty() && version != itr->second->version)
        {
            continue;
        }

        return itr->second;
    }

    // 否则返回空
    return nullptr;
}

std::vector<std::shared_ptr<ise::tf::GeneralAssayInfo>> CommonInformationManager::GetAllIseAssayInfo(int iAssayCode, ::tf::DeviceType::type type)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);
	std::vector<std::shared_ptr<ise::tf::GeneralAssayInfo>> IseGAIs;

	// 查找项目编号对应的项目信息
	auto range = m_mapIseAssayCodeMaps.equal_range(iAssayCode);
	for (auto itr = range.first; itr != range.second; itr++)
	{
		// 类型是否一致
		if (type != itr->second->deviceType)
		{
			continue;
		}

		IseGAIs.push_back(itr->second);
	}

	// 否则返回空
	return IseGAIs;
}

///
/// @brief 根据生化项目关键信息获取生化项目特殊信息详细信息
///
/// @param[in]  iAssayCode  项目编号
/// @param[in]  type		机器类型
/// @param[in]  sampSrcType 样本源类型（-1则不限制）
///
/// @return 项目详细信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月17日，新建函数
/// @li 6950/ChenFei，2022年5月18日，加入样本源类型参数
///
std::vector<std::shared_ptr<ch::tf::SpecialAssayInfo>> CommonInformationManager::GetChemistrySpecialAssayInfo(int iAssayCode
    , ::tf::DeviceType::type type, int sampSrcType/* = -1*/, const std::string& version/* = ""*/)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);
    std::vector<std::shared_ptr<ch::tf::SpecialAssayInfo>> ChSAIs;
    std::shared_ptr<ch::tf::GeneralAssayInfo> spGAI;

	// 检查项目编号
	if ((iAssayCode <= 0) || (iAssayCode > ::ch::tf::g_ch_constants.ASSAY_CODE_I))
	{
		ULOG(LOG_INFO, "iAssayCode = %d", iAssayCode);
		return {};
	}

    // 获得当前在用的项目通用配置参数信息
    if (iAssayCode > ::ch::tf::g_ch_constants.ASSAY_CODE_SIND && iAssayCode <= ::ch::tf::g_ch_constants.ASSAY_CODE_I)
    {
        spGAI = GetChemistryAssayInfo(::ch::tf::g_ch_constants.ASSAY_CODE_SIND, type, version);
    }
    else
    {
        spGAI = GetChemistryAssayInfo(iAssayCode, type, version);
    }

    if (nullptr == spGAI)
    {
        ULOG(LOG_WARN, "Failed to execute GetChemistrySpecialAssayInfo(), assayCode=%d", iAssayCode);
        return ChSAIs;
    }

    auto findSAIFunc = [&] (int code)
    {    
        // 查找项目编号对应的项目信息
        auto range = m_mapChSpecialAssayCodeMaps.equal_range(code);
        for (auto itr = range.first; itr != range.second; itr++)
        {
            // 设备类型、参数版本是否一致
            std::shared_ptr<ch::tf::SpecialAssayInfo> spSAI = itr->second;
            if ((nullptr == spSAI) || (type != spSAI->deviceType) || (spGAI->version != spSAI->version ))
            {
                continue;
            }

            // 判断样本源类型
            if (sampSrcType != -1)
            {
                if (sampSrcType == itr->second->sampleSourceType)
                {
                    ChSAIs.push_back(itr->second);
                    break;
                }
            }
            else
            {
                ChSAIs.push_back(itr->second);
            }
        }};

    if (iAssayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND)
    {
        findSAIFunc(::ch::tf::g_ch_constants.ASSAY_CODE_L);
        findSAIFunc(::ch::tf::g_ch_constants.ASSAY_CODE_H);
        findSAIFunc(::ch::tf::g_ch_constants.ASSAY_CODE_I);
    }
    else
    {
        findSAIFunc(iAssayCode);
    }

    return ChSAIs;
}

std::shared_ptr<ch::tf::SpecialAssayInfo> CommonInformationManager::GetChemistrySpecialTwinsAssayInfo(int iAssayCode, ::tf::DeviceType::type type)
{
    // 查找项目编号对应的项目信息
    auto range = m_mapChSpecialAssayCodeMaps.equal_range(iAssayCode);
    for (auto itr = range.first; itr != range.second; itr++)
    {
        if (itr->second->iTwinsTestAssayCode > 0)
        {
            return itr->second;
        }
    }

    return nullptr;
}

std::shared_ptr<ch::tf::SpecialAssayInfo> CommonInformationManager::GetHighestPrioritySpecialAssay(int iAssayCode
    , ::tf::DeviceType::type deviceType, const std::string& version)
{
    ULOG(LOG_INFO, "%s(%d, %d, %s)", __FUNCTION__, iAssayCode, deviceType, version);

    // 获得指定项目、机型对应的特殊项目配置参数
    std::vector<std::shared_ptr<ch::tf::SpecialAssayInfo>> vecSAIs = GetChemistrySpecialAssayInfo(iAssayCode, deviceType);
    if (vecSAIs.empty())
    {
        ULOG(LOG_ERROR, "vecSAIs is empty");
        return nullptr;
    }

    for (auto& item : vecSAIs)
    {
        if (item->priority)
        {
            return item;
        }
    }

    return nullptr;
}

///
/// @brief
///     根据ISE项目关键信息获取ISE项目特殊信息
///
/// @param[in]  iAssayCode  项目编号
/// @param[in]  type		机器类型
///
/// @return     ISE项目特殊信息
///
/// @par History:
/// @li 6950/ChenFei，2022年05月18日，新建函数
///
std::vector<std::shared_ptr<ise::tf::SpecialAssayInfo>> CommonInformationManager::GetIseSpecialAssayInfo(int iAssayCode, ::tf::DeviceType::type type)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);
    std::vector<std::shared_ptr<ise::tf::SpecialAssayInfo>> iseSAIs;

    // 查询当前在用的电解质通用项目配置参数信息
    std::shared_ptr<ise::tf::GeneralAssayInfo> spGAI = GetIseAssayInfo(iAssayCode, type);
    if (nullptr == spGAI)
    {
        ULOG(LOG_ERROR, "spGAI is nullptr");
        return iseSAIs;
    }

    // 查找项目编号对应的项目信息
    auto range = m_mapIseSpecialAssayCodeMaps.equal_range(iAssayCode);
    for (auto itr = range.first; itr != range.second; itr++)
    {
        // 类型是否一致
        std::shared_ptr<ise::tf::SpecialAssayInfo> spSAI = itr->second;
        if ((nullptr == spSAI)
            || (type != spSAI->deviceType)
            || (spSAI->version != spGAI->version))
        {
            continue;
        }

        iseSAIs.push_back(itr->second);
    }

    return iseSAIs;
}

std::shared_ptr<ise::tf::SpecialAssayInfo> CommonInformationManager::GetIseSpecialAssayInfo(int iAssayCode, ::tf::SampleSourceType::type SST, ::tf::DeviceType::type type)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);

    // 查询当前在用的电解质通用项目配置参数信息
    std::shared_ptr<ise::tf::GeneralAssayInfo> spGAI = GetIseAssayInfo(iAssayCode, type);
    if (nullptr == spGAI)
    {
        ULOG(LOG_ERROR, "spGAI is nullptr");
        return nullptr;
    }

    // 查找项目编号对应的项目信息
    auto range = m_mapIseSpecialAssayCodeMaps.equal_range(iAssayCode);
    for (auto itr = range.first; itr != range.second; itr++)
    {
        // 类型是否一致
        std::shared_ptr<ise::tf::SpecialAssayInfo> spSAI = itr->second;
        if ((nullptr != spSAI)
            && (type == spSAI->deviceType)
            && (SST == spSAI->sampleSourceType)
            && (spSAI->version == spGAI->version))
        {
            return spSAI;
        }
    }

    return nullptr;
}

///
/// @brief	获取设备信息   
///
/// @return 设备信息
///
/// @par History:
/// @li 7951/LuoXin，2023年1月11日，新建函数
///
std::map<std::string, std::shared_ptr<const tf::DeviceInfo>> CommonInformationManager::GetDeviceMaps()
{
    std::map<std::string, std::shared_ptr<const tf::DeviceInfo>> tmpMaps;
	for (auto device : m_mapDevices)
	{
		tmpMaps[device.first] = std::make_shared<const tf::DeviceInfo>(*(device.second));
	}

	return tmpMaps;
}

bool CommonInformationManager::ModifyDeviceInfo(const tf::DeviceInfo& dev)
{
    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    auto iter = m_mapDevices.find(dev.deviceSN);
    if (iter != m_mapDevices.end())
    {
        tf::DeviceInfo di;
        di.__set_id(iter->second->id);

        if (dev.__isset.masked)
        {
            di.__set_masked(dev.masked);
        }

        if (dev.__isset.maskedAssayCode)
        {
            di.__set_maskedAssayCode(dev.maskedAssayCode);
        }

        if (dev.__isset.unAllocatedAssayCodes)
        {
            di.__set_unAllocatedAssayCodes(dev.unAllocatedAssayCodes);
        }

        if (dev.__isset.otherInfo)
        {
            di.__set_otherInfo(dev.otherInfo);
        }

        // 调用thrift接口修改数据库，修改成功后更新缓存
        if (DcsControlProxy::GetInstance()->ModifyDeviceInfo(di))
        {
            if (di.__isset.masked)
            {
                iter->second->masked = di.masked;
                // 发送设备屏蔽状态更新
                POST_MESSAGE(MSG_ID_DEVICE_MASK_UPDATE, QString::fromStdString(iter->second->groupName), QString::fromStdString(dev.deviceSN), di.masked);
            }

            if (di.__isset.maskedAssayCode)
            {
                iter->second->maskedAssayCode = di.maskedAssayCode;
            }

            if (di.__isset.unAllocatedAssayCodes)
            {
                iter->second->unAllocatedAssayCodes = di.unAllocatedAssayCodes;

				// 发送项目分配信息更新
				POST_MESSAGE(MSG_ID_MANAGER_UNALLOCATE_ASSAYS_UPDATE);
            }

            if (di.__isset.otherInfo && iter->second->otherInfo != di.otherInfo)
            {
				iter->second->otherInfo = di.otherInfo;
				// 发送设备的其他信息更新
				POST_MESSAGE(MSG_ID_DEVICE_OTHER_INFO_UPDATE, QString::fromStdString(iter->first));
            }

            return true;
        }
    }

    return false;
}

///
/// @brief  获取免疫设备单机版是否处于停机状态
///
/// @param[in]    
///
/// @return bool:true免疫设备处于停机状态 false免疫设备非停机状态
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年8月29日，新建函数
///
bool CommonInformationManager::GetImDeviceIsHalt()
{
    // 检查仪器状态，是否存在非停机状态的仪器
    auto& devs = GetDeviceMaps();
    for (const auto &SingleDev : devs)
    {
        if (SingleDev.second->deviceType != ::tf::DeviceType::DEVICE_TYPE_I6000)
        {
            continue;
        }
        if (SingleDev.second->status == tf::DeviceWorkState::DEVICE_STATUS_HALT)
        {
            return true;
        }
    }
    return false;
}

bool CommonInformationManager::IsExistDeviceRuning(int assayClassfy)
{
    for (DeviceMaps::iterator it = m_mapDevices.begin(); it != m_mapDevices.end(); ++it)
    {
        // 目前轨道会常在running状态，所以不参考其状态
        if (it->second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        // 如果指定了项目类型
        if (assayClassfy != -1 && it->second->deviceClassify != assayClassfy)
        {
            continue;
        }

        if (it->second->status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING
            || it->second->status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP 
            || it->second->status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT
            || it->second->status == ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK)
        {
            return true;
        }
    }

    return false;
}

bool CommonInformationManager::IsAllDeviceStandby()
{
    bool allStandby = true;
    for (DeviceMaps::iterator it = m_mapDevices.begin(); it != m_mapDevices.end(); ++it)
    {
        // 目前轨道会常在running状态，所以不参考其状态
        if (it->second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }
        if (it->second->status != ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY)
        {
            allStandby = false;
            break;
        }
    }

    return allStandby;
}

std::string CommonInformationManager::GetDeviceName(const std::string& deviceSn, const int moduleIndex)
{
    std::shared_ptr<const tf::DeviceInfo> dev = sm_instance->GetDeviceInfo(deviceSn);
    if (dev == nullptr)
    {
        return "";
    }

	// ISE的模块参数(只有两个模块的时候才显示A,B)
    std::string uninName = dev->__isset.groupName ? (dev->groupName + dev->name) : dev->name;
	if (dev->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE 
		&& dev->__isset.moduleCount
		&& dev->moduleCount > 1)
	{
		if (moduleIndex == 1)
		{
			uninName += "A";
		}
		else
		{
			uninName += "B";
		}
	}

    return uninName;
}

std::string CommonInformationManager::GetDeviceName(const std::string& deviceSn)
{
	std::shared_ptr<const tf::DeviceInfo> dev = sm_instance->GetDeviceInfo(deviceSn);
	if (dev == nullptr)
	{
		return "";
	}

	std::string uninName = dev->__isset.groupName ? (dev->groupName + dev->name) : dev->name;
	return uninName;
}

///
/// @brief 根据设备名称获取设备信息
///
/// @param[in]  deviceName  设备名
///
/// @return 设备详细信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月23日，新建函数
///
std::shared_ptr<const tf::DeviceInfo> CommonInformationManager::GetDeviceInfoByDeviceName(const std::string& deviceName)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, deviceName.c_str());

    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    // 根据设备名查找对应的信息
    for (const auto& device : m_mapDevices)
    {
        if (device.second->groupName + device.second->name  == deviceName)
        {
			return device.second;
        }
    }

    // 未找到返回空值
    return nullptr;
}

EnumLightColor CommonInformationManager::GetLightColor(const int status, const bool rgtAlarmReaded)
{
	// 判断设备状态
	switch (status)
	{
		// 未连接
	case ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT:
		return ELC_GRAY;
		// 待机、运行、维护、预热、加样停、进架停
	case ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY:
	case ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING:
	case ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN:
	case ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP:
    case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP:
    case ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK:
		return rgtAlarmReaded ? ELC_GREEN : ELC_YELLOW;
		// 异常加样停
	case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT:
		return ELC_YELLOW;
		// 停机
	case ::tf::DeviceWorkState::DEVICE_STATUS_HALT:
		return ELC_RED;
		// 其他状态
	default:
		return ELC_UNKNOWN;
	}
}

EnumLightColor CommonInformationManager::GetChSingleLightColor(const int statusCh, const int statusIse, const bool rgtAlarmReaded)
{
	// 合成后的状态灯颜色
	EnumLightColor composeColor = GetLightColor(statusCh, rgtAlarmReaded);
	EnumLightColor iseColor = GetLightColor(statusIse, rgtAlarmReaded);

	// 有一个是未知颜色，都返回未知颜色
	if (composeColor == ELC_UNKNOWN || iseColor == ELC_UNKNOWN)
	{
		return ELC_UNKNOWN;
	}

	/* 合成规则:
	1、CH灰色-取ISE的颜色
	2、CH红或者CH黄-取CH颜色
	3、CH绿、ISE为红或黄-黄色；CH绿、ISE为其他-保持CH颜色不变
	*/
	if (composeColor == ELC_GRAY)
	{
		composeColor = iseColor;
	}
	else if (composeColor == ELC_GREEN)
	{
		if (iseColor == ELC_RED || iseColor == ELC_YELLOW)
		{
			composeColor = ELC_YELLOW;
		}
	}

	return composeColor;
}

EnumLightColor CommonInformationManager::GetC2000LightColor(const int statusA, const int statusB, const bool rgtAlarmReaded)
{
	// 合成后的状态灯颜色
	EnumLightColor composeColor = GetLightColor(statusA, rgtAlarmReaded);
	EnumLightColor devBColor = GetLightColor(statusB, rgtAlarmReaded);

	// 有一个是未知颜色，都返回未知颜色
	if (composeColor == ELC_UNKNOWN || devBColor == ELC_UNKNOWN)
	{
		return ELC_UNKNOWN;
	}

	/* 合成规则:
	1、CHA灰色-取CHB的颜色
	2、CHA绿色，CHB是红色或黄色-黄色，否则保持CHA绿色
	3、CHA红色，CHB是绿色或黄色-黄色，否则保持CHA红色
	4、其他情况都为黄色
	*/
	if (composeColor == ELC_GRAY)
	{
		composeColor = devBColor;
	}
	else if (composeColor == ELC_GREEN)
	{
		if (devBColor == ELC_RED || devBColor == ELC_YELLOW)
		{
			composeColor = ELC_YELLOW;
		}
	}
	else if (composeColor == ELC_RED)
	{
		if (devBColor == ELC_GREEN || devBColor == ELC_YELLOW)
		{
			composeColor = ELC_YELLOW;
		}
	}
	else
	{
		composeColor = ELC_YELLOW;
	}

	return composeColor;
}

///
/// @brief  通过设备关键信息获取设备的状态
///     
///
/// @param[in]  keyInfo  设备的关键信息
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年12月11日，新建函数
///
tf::DeviceWorkState::type CommonInformationManager::GetDevStateByKeyInfo(const tf::DevicekeyInfo& keyInfo)
{
	// 获取设备状态，ISE以SN来整体判断设备状态，暂时控制时可以按照模块控制，状态按照整体的状态返回
	for (const auto& device : m_mapDevices)
	{
		if (device.second->deviceSN == keyInfo.sn)
		{
			return static_cast<tf::DeviceWorkState::type>(device.second->status);
		}
	}

	// 默认返回未连接
	return tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT;
}

QStringList CommonInformationManager::GetDeviceSnByName(const std::string& name)
{
    QStringList devSns;

    for (auto& dev : m_mapDevices)
    {
        if (dev.second->groupName == name || dev.second->groupName + dev.second->name == name)
        {
            devSns.push_back(QString::fromStdString(dev.second->deviceSN));
        }
    }

    return devSns;
}

///
/// @brief
///     设置项目信息
///
/// @param[in]  spAI  项目信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月2日，新建函数
///
bool CommonInformationManager::SetAssayInfo(std::shared_ptr<tf::GeneralAssayInfo> spAI)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 参数检查
    if (nullptr == spAI)
    {
        return false;
    }

    // 没有项目编号或项目名，返回失败
    if (!spAI->__isset.assayCode || !spAI->__isset.assayName)
    {
        return false;
    }

    // 更新项目编号映射
    m_mapAssayCodeMaps[spAI->assayCode] = spAI;

	// 每页项目数 = 行数 * 列数
	int pageNumber = m_singleAssayRowAmount * 7;
	// （项目编号，项目位置)；项目位置 = 页数*每页项目数 + 页内位置
	m_defaultPostionMap[spAI->assayCode] = spAI->pageIdx * pageNumber + spAI->positionIdx;

    // 更新项目名映射
    m_mapAssayNameMaps[spAI->assayName] = spAI;

    return true;
}

///
/// @brief
///     获取单实例
///
/// @return 单实例
///
/// @par History:
/// @li 4170/TangChuXian，2020年10月29日，新建函数
///
std::shared_ptr<CommonInformationManager> CommonInformationManager::GetInstance()
{
    return sm_instance;
}

///
/// @brief 获取对应机型的生化通用参数
///
/// @param[in]  type		机型
/// @param[out]  indexMaps  参数列表
///
/// @return true获取成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月17日，新建函数
///
bool CommonInformationManager::GetChAssayMaps(tf::DeviceType::type type, ChAssayIndexUniqueCodeMaps& indexMaps)
{
    for (const auto& chAssayInfo : m_mapChAssayCodeMaps)
    {
        // 判断类型(无效类型获取所有)
        if (type != tf::DeviceType::DEVICE_TYPE_INVALID && chAssayInfo.second->deviceType != type)
        {
            continue;
        }

        // 必须可用
        if (!chAssayInfo.second->enable)
        {
            continue;
        }

        // 从通过项目表中查找对应的项目名称
        std::shared_ptr<tf::GeneralAssayInfo> spAssay = GetAssayInfo(chAssayInfo.first);
        if (spAssay == nullptr)
        {
            continue;
        }

        // 插入表中
        indexMaps[spAssay->assayName] = chAssayInfo.second;
    }

    return true;
}

///
/// @brief 获取生化对应机型的生化通用参数(按照项目编号排序)
///
/// @param[in]  type		机型
/// @param[out]  indexMaps  参数列表
///
/// @return true获取成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月22日，新建函数
///
bool CommonInformationManager::GetChAssayMaps(tf::DeviceType::type type, ChAssayIndexUniqueOrderCodeMaps& indexOrderMaps)
{
	for (const auto& chAssayInfo : m_mapChAssayCodeMaps)
	{
		// 判断类型(无效类型获取所有)
		if (type != tf::DeviceType::DEVICE_TYPE_INVALID && chAssayInfo.second->deviceType != type)
		{
			continue;
		}

		// 必须可用
		if (!chAssayInfo.second->enable)
		{
			continue;
		}

		// 插入表中
		indexOrderMaps[chAssayInfo.first] = chAssayInfo.second;
	}

	return true;
}

std::shared_ptr<ise::tf::IseModuleInfo> CommonInformationManager::GetIseModuleMaps(ise::tf::SuppliesInfo& supplyInfo)
{
    ULOG(LOG_INFO, "%s(devicesn: %s, moduleIndex: %d)", __FUNCTION__, supplyInfo.deviceSN, supplyInfo.moduleIndex);
    auto sourceType = QSystemMonitor::GetInstance().GetCurrentType();
    for (const auto& module : m_iseModuleMaps)
    {
        if (module.second->deviceSn != supplyInfo.deviceSN)
        {
            continue;
        }

        if (module.second->moduleIndex != supplyInfo.moduleIndex)
        {
            continue;
        }

        if (module.second->sampleType != sourceType)
        {
            continue;
        }

        return module.second;
    }

    return Q_NULLPTR;
}

///
/// @brief 获取ISE对应机型的免疫通用参数
///
/// @param[in]   type       机型
/// @param[out]  indexMaps  参数列表
///
/// @return true获取成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月3日，新建函数
///
bool CommonInformationManager::GetIseAssayMaps(tf::DeviceType::type type, IseAssayIndexUniqueCodeMaps& indexMaps)
{
    auto sourceType = QSystemMonitor::GetInstance().GetCurrentType();
    for (const auto& iseAssayInfo : m_mapIseAssayCodeMaps)
    {
        // 判断类型(无效类型获取所有)
        if (type != tf::DeviceType::DEVICE_TYPE_INVALID && iseAssayInfo.second->deviceType != type)
        {
            continue;
        }

        // 必须可用
        if (!iseAssayInfo.second->enable)
        {
            continue;
        }

        // 从通过项目表中查找对应的项目名称
        std::shared_ptr<tf::GeneralAssayInfo> spAssay = GetAssayInfo(iseAssayInfo.first);
        if (spAssay == nullptr)
        {
            continue;
        }

        // 插入表中
        indexMaps[spAssay->assayName] = iseAssayInfo.second;
    }

    return true;
}

///
/// @brief 获取ISE对应机型的免疫通用参数(按编号排序)
///
/// @param[in]   type       机型
/// @param[out]  indexMaps  参数列表
///
/// @return true获取成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月22日，新建函数
///
bool CommonInformationManager::GetIseAssayMaps(tf::DeviceType::type type, IseAssayIndexUniqueOrderCodeMaps& indexMaps)
{
	auto sourceType = QSystemMonitor::GetInstance().GetCurrentType();
	for (const auto& iseAssayInfo : m_mapIseAssayCodeMaps)
	{
		// 判断类型(无效类型获取所有)
		if (type != tf::DeviceType::DEVICE_TYPE_INVALID && iseAssayInfo.second->deviceType != type)
		{
			continue;
		}

		// 必须可用
		if (!iseAssayInfo.second->enable)
		{
			continue;
		}

		// 插入表中
		indexMaps[iseAssayInfo.first] = iseAssayInfo.second;
	}

	return true;
}

///
/// @brief
///     更新通用项目信息
///
/// @param[in]  spAssayInfo  项目信息
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月6日，新建函数
///
void CommonInformationManager::UpdateGnrAssayInfo(std::shared_ptr<::tf::GeneralAssayInfo> spAssayInfo)
{
    // 参数检查
    if (spAssayInfo == Q_NULLPTR)
    {
        return;
    }

    // 更新缓存
    m_mapAssayCodeMaps[spAssayInfo->assayCode] = spAssayInfo;
    m_mapAssayNameMaps[spAssayInfo->assayName] = spAssayInfo;

    // 清理缓存中的无效数据
    auto it = m_mapAssayNameMaps.begin();
    while (it != m_mapAssayNameMaps.end())
    {
        // 如果数据无效，则删除
        if (it->first != it->second->assayName)
        {
            it = m_mapAssayNameMaps.erase(it);
            continue;
        }

        // 自增
        it++;
    }
}

void CommonInformationManager::GetSamePositiongAssayInfo(int pageIdx, int positionIdx, std::vector<std::shared_ptr<tf::GeneralAssayInfo>>& outAssays)
{
    outAssays.clear();
    AssayIndexCodeMaps::iterator it = m_mapAssayCodeMaps.begin();
    for (; it != m_mapAssayCodeMaps.end(); ++it)
    {
        if (it->second == nullptr)
        {
            continue;
        }
        if (it->second->pageIdx == pageIdx && it->second->positionIdx == positionIdx)
        {
            outAssays.push_back(it->second);
        }
    }
}

bool CommonInformationManager::UpdateAssayPosition(int assayCode, int pageIdx, int positionIdx)
{
    ULOG(LOG_INFO, "%s(assayCode:%d, pageIdx:%d, pos:%d.", __FUNCTION__, assayCode, pageIdx, positionIdx);

    AssayIndexCodeMaps::iterator it = m_mapAssayCodeMaps.find(assayCode);
    if (it == m_mapAssayCodeMaps.end())
    {
        ULOG(LOG_ERROR, "Failed to find generalassayinfo by assayCode:%d.", assayCode);
        return false;
    }

    if (it->second->pageIdx == pageIdx && it->second->positionIdx == positionIdx)
    {
        ULOG(LOG_WARN, "Same position skiped.");
        return true;
    }

    // 查询是否之前有位置相同的项目，若有则修改(将之前位置的项目位置设置为 - 1)
    std::vector<std::shared_ptr<tf::GeneralAssayInfo>> samePositionAssays;
    GetSamePositiongAssayInfo(pageIdx, positionIdx, samePositionAssays);
    if (samePositionAssays.size() > 0)
    {
        for (int i = 0; i < samePositionAssays.size(); ++i)
        {
            if (samePositionAssays[i] == nullptr)
            {
                continue;
            }

			// 不同的产品类型允许位置重复
			if (it->second->assayClassify != samePositionAssays[i]->assayClassify)
			{
				continue;
			}

            if (samePositionAssays[i]->pageIdx != -1)
            {
                tf::GeneralAssayInfo tempNull = *samePositionAssays[i];
                tempNull.__set_pageIdx(-1);
                tempNull.__set_positionIdx(-1);
                if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(tempNull))
                {
                    continue;
                }
                samePositionAssays[i]->__set_pageIdx(-1);
                samePositionAssays[i]->__set_positionIdx(-1);
            }
        }
    }

    // 更新数据库记录
    tf::GeneralAssayInfo tempAssay = *(it->second);
    tempAssay.__set_pageIdx(pageIdx);
    tempAssay.__set_positionIdx(positionIdx);
    if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(tempAssay))
    {
        ULOG(LOG_ERROR, "Failed to update generalassayinfo DB.");
        return false;
    }

    // 更新缓存记录
    it->second->__set_pageIdx(pageIdx);
    it->second->__set_positionIdx(positionIdx);

	// 每页项目数 = 行数 * 列数
	int pageNumber = m_singleAssayRowAmount * 7;
	// （项目编号，项目位置)；项目位置 = 页数*每页项目数 + 页内位置
	m_defaultPostionMap[assayCode] = pageIdx * pageNumber + positionIdx;

    return true;
}

///
/// @brief
///     获取所有的设备分类
///
/// @par History:
/// @li 7951/LuoXin，2022年6月23日，新建函数
///
std::vector<::tf::AssayClassify::type> CommonInformationManager::GetAllDeviceClassify()
{
    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    std::vector<::tf::AssayClassify::type> vecs;
    for (const auto& device : m_mapDevices)
    {
        vecs.push_back(device.second->deviceClassify);
    }
    return vecs;
}

///
/// @brief 是否包含目标设备分类
///
/// @param[in]  classifyType  目标设备类型
///
/// @return true:包含
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年1月8日，新建函数
///
bool CommonInformationManager::IsContainsAimClassify(::tf::AssayClassify::type classifyType)
{
	for (const auto& device : m_mapDevices)
	{
		if (device.second->deviceClassify == classifyType)
		{
			return true;
		}
	}

	return false;
}

///
/// @brief 根据名称获取组合项目的数据库主键
///
/// @param[in]  profileName  项目名称
///
/// @return 组合项目数据库主键
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月29日，新建函数
///
boost::optional<std::int64_t> CommonInformationManager::GetProfileInfoFromName(const std::string& profileName)
{
    for (const auto& profile : m_mapProfileAssay)
    {
        if (profile.second->profileName == profileName)
        {
            return boost::make_optional(profile.first);
        }
    }

    return boost::none;
}

void CommonInformationManager::GetProfileSubNames(const std::string& profileName, QStringList& subNames)
{
    for (const auto& profile : m_mapProfileAssay)
    {
        if (profile.second->profileName == profileName)
        {
            for (int assayCode : profile.second->subItems)
            {
                AssayIndexCodeMaps::iterator it = m_mapAssayCodeMaps.find(assayCode);
                if (it != m_mapAssayCodeMaps.end())
                {
                    subNames.append(QString::fromStdString(it->second->assayName));
                }
            }
            break;
        }
    }
}

void CommonInformationManager::GetProfileAssaysList(std::vector<std::pair<QString, QString>>& pfNamesCh, \
    std::vector<std::pair<QString, QString>>& pfNamesIm)
{
    ULOG(LOG_INFO, __FUNCTION__);

    for (const auto& profile : m_mapProfileAssay)
    {
        // 把子项目名组成字符串
        QString strAssays;
        int lstLen = profile.second->subItems.size();
        for (int i = 0; i < lstLen; ++i)
        {
            auto it = m_mapAssayCodeMaps.find(profile.second->subItems[i]);
            if (it == m_mapAssayCodeMaps.end())
            {
                ULOG(LOG_INFO, "Not exist assayname, assaycode is %d.", profile.second->subItems[i]);
                continue;
            }
            strAssays += QString::fromStdString(it->second->assayName);
            if (i + 1 < lstLen)
            {
                strAssays += tr("、");
            }
        }

        // 区分出项目类型
        QString proName = QString::fromStdString(profile.second->profileName);
        if (profile.second->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE)
        {
            pfNamesIm.push_back(std::pair<QString, QString>(proName, strAssays));
        }
        else
        {
            pfNamesCh.push_back(std::pair<QString, QString>(proName, strAssays));
        }
    }
}

bool CommonInformationManager::DeleteProfileInfo(const std::string& profileName)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, profileName.c_str());

    // 对数据库表中的组合项进行删除
    tf::ProfileInfoQueryCond profi;
    profi.__set_profileName(profileName);
    if (!DcsControlProxy::GetInstance()->DeleteProfileInfo(profi))
    {
        ULOG(LOG_ERROR, "Failed to delete profile.");
        return false;
    }

    // 通过组合项目名称对缓存中的项进行删除
    for (AssayProfileMap::iterator it = m_mapProfileAssay.begin(); it != m_mapProfileAssay.end(); ++it)
    {
        if (profileName == it->second->profileName)
        {
            m_mapProfileAssay.erase(it);
            break;
        }
    }

    return true;
}

bool CommonInformationManager::AddProfileInfo(const tf::ProfileInfo& profileInfo)
{
    ULOG(LOG_INFO, __FUNCTION__);

    tf::ResultLong ret;
    if (!DcsControlProxy::GetInstance()->AddProfileInfo(ret, profileInfo) || \
        ret.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Failed to add profileinfo.");
        return false;
    }

    std::shared_ptr<tf::ProfileInfo> nProf(new tf::ProfileInfo(profileInfo));
    nProf->__set_id(ret.value);
    m_mapProfileAssay.insert(std::pair<int64_t, std::shared_ptr<tf::ProfileInfo>>(ret.value, nProf));

    return true;
}

bool CommonInformationManager::ModifyProfileInfo(const tf::ProfileInfo& profileInfo)
{
    if (!DcsControlProxy::GetInstance()->ModifyProfileInfo(profileInfo))
    {
        ULOG(LOG_ERROR, "Faield to modify profileinfo.");
        return false;
    }

    auto it = m_mapProfileAssay.find(profileInfo.id);
    if (it != m_mapProfileAssay.end())
    {
        it->second = std::make_shared<tf::ProfileInfo>(profileInfo);
    }

    return true;
}

bool CommonInformationManager::ModifyProfilePosition(int assayCode, int pageIdx, int positionIdx)
{
    ULOG(LOG_INFO, "%s(assayCode:%d, pageIdx:%d, pos:%d).", __FUNCTION__, assayCode, pageIdx, positionIdx);

    std::shared_ptr<tf::ProfileInfo> tempAssay = GetCommonProfile(assayCode);
    if (tempAssay == nullptr)
    {
        ULOG(LOG_WARN, "Not found assayCode.");
        return false;
    }
    if (tempAssay->pageIdx == pageIdx && tempAssay->positionIdx == positionIdx)
    {
        ULOG(LOG_WARN, "Same position skip.");
        return true;
    }

    // 查询相同位置的组合项目是否存在
    std::vector<std::shared_ptr<tf::ProfileInfo>> samePositionAssays;
    GetSamePositionAssayInfoProf(pageIdx, positionIdx, samePositionAssays);
    for (int i = 0; i < samePositionAssays.size(); ++i)
    {
        tf::ProfileInfo tempProf = *(samePositionAssays[i]);
        tempProf.__set_pageIdx(-1);
        tempProf.__set_positionIdx(-1);
        if (!ModifyProfileInfo(tempProf))
        {
            continue;
        }
        samePositionAssays[i]->__set_pageIdx(-1);
        samePositionAssays[i]->__set_positionIdx(-1);
    }

    tempAssay->__set_pageIdx(pageIdx);
    tempAssay->__set_positionIdx(positionIdx);

    return ModifyProfileInfo(*tempAssay);
}

void CommonInformationManager::GetSamePositionAssayInfoProf(int pageIdx, int positionIdx, std::vector<std::shared_ptr<tf::ProfileInfo>>& sameProf)
{
    sameProf.clear();
    AssayProfileMap::iterator it = m_mapProfileAssay.begin();
    for (; it != m_mapProfileAssay.end(); ++it)
    {
        if (it->second == nullptr)
        {
            continue;
        }
        if (it->second->pageIdx == pageIdx && it->second->positionIdx == positionIdx)
        {
            sameProf.push_back(it->second);
        }
    }
}

void CommonInformationManager::GetDepartedAssayNames(QStringList& immyData, QStringList& chemiData, bool onlyShowed)
{
    auto assayMap = GetGeneralAssayMap();
    for (const auto& assay : assayMap)
    {
        // 如果只需要项目界面现实的项目，则进行positionIdx的判断
        if (onlyShowed && assay.second->positionIdx < 0)
        {
            continue;
        }

        // 过滤SIND、ISE,来自bug10934
        if (assay.second->assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND
            || assay.second->assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_ISE)
        {
            continue;
        }

        bool isIm = (assay.second->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE);
        isIm ? (immyData << QString::fromStdString(assay.first)) : (chemiData << QString::fromStdString(assay.first));
    }
}

///
/// @brief 根据类型码获取属性信息
///
/// @param[in]  typeCode  类型码
///
/// @return 耗材属性信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月5日，新建函数
///
boost::optional<std::shared_ptr<ch::tf::SupplyAttribute>> CommonInformationManager::GetSupplyAttributeByTypeCode(int typeCode, bool IsSampleProbe)
{
    for (const auto& supplyAttribute : m_supplyAttributeMap)
    {
        if (int(supplyAttribute.second->type) == typeCode && supplyAttribute.second->sampleProbe == IsSampleProbe)
        {
            return boost::make_optional(supplyAttribute.second);
        }
    }

    return boost::none;
}

///
/// @brief 根据类型码获取属性信息
///
/// @param[in]  typeCode  类型码
///
/// @return 耗材属性信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月5日，新建函数
///
boost::optional<std::shared_ptr<im::tf::SupplyAttribute>> CommonInformationManager::GetImSupplyAttributeByTypeCode(int typeCode)
{
    for (const auto& supplyAttribute : m_ImSupplyAttributeMap)
    {
        if (to_underlying(supplyAttribute.second->type) == typeCode)
        {
            return boost::make_optional(supplyAttribute.second);
        }
    }

    return boost::none;
}

///
/// @brief 根据类型码获取属性信息
///
/// @param[in]  typeCode  类型码
///
/// @return 耗材属性信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月13日，新建函数
///
boost::optional<std::shared_ptr<ise::tf::SupplyAttribute>> CommonInformationManager::GetISESupplyAttributeByTypeCode(int typeCode)
{
    for (const auto& supplyAttribute : m_iseSupplyAttributeMap)
    {
        if (to_underlying(supplyAttribute.second->type) == typeCode)
        {
            return boost::make_optional(supplyAttribute.second);
        }
    }

    return boost::none;
}

///
/// @brief 将当前单位与主单位进行换算
///
/// @param[in]  assayCode  项目编号
///
/// @return 换算后的结果
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月5日，新建函数
///
double CommonInformationManager::ToMainUnitValue(int assayCode, double value)
{
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = GetAssayInfo(assayCode);

    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", assayCode);
        return 0.0;
    }

    for (const auto& unit : spAssayInfo->units)
    {
        if (unit.isCurrent && unit.factor != 0.0)
            return value / unit.factor;
    }

    return 0.0;
}

///
/// @brief 将主单位和当前单位进行换算
///
/// @param[in]  assayCode  项目编号
///
/// @return 换算后的结果
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月5日，新建函数
///
double CommonInformationManager::ToCurrentUnitValue(int assayCode, double value)
{
    // 是否是计算项目
    bool isCalc = IsCalcAssay(assayCode);

    if (!isCalc)
    {
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = GetAssayInfo(assayCode);

        if (spAssayInfo == nullptr)
        {
            ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", assayCode);
            return value;
        }

        for (const auto& unit : spAssayInfo->units)
        {
            if (unit.isCurrent && unit.factor != 0.0)
                return value * unit.factor;
        }
    }

    return value;
}

///
/// @brief 获取对应机型的免疫通用参数
///
/// @param[in]   type       机型
/// @param[out]  indexMaps  参数列表
///
/// @return true获取成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月8日，新建函数
///
bool CommonInformationManager::GetImAssayMaps(tf::DeviceType::type type, ImAssayIndexUniqueCodeMaps& indexMaps)
{
    for (const auto& imAssayInfo : m_mapImAssayCodeMaps)
    {
        // 判断类型(无效类型获取所有)
        if (type != tf::DeviceType::DEVICE_TYPE_INVALID && imAssayInfo.second->deviceType != type)
        {
            continue;
        }

        // 从通过项目表中查找对应的项目名称
        std::shared_ptr<tf::GeneralAssayInfo> spAssay = GetAssayInfo(imAssayInfo.first);
        if (spAssay == nullptr)
        {
            continue;
        }

        // 插入表中
        indexMaps[spAssay->assayCode] = spAssay;
    }

    return true;
}

///
/// @brief 获取免疫对应机型的通用项目参数（按编号排序）
///
/// @param[in]   type       机型
/// @param[out]  indexMaps  参数列表
///
/// @return true获取成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月12日，新建函数
///
bool CommonInformationManager::GetImAssayMaps(tf::DeviceType::type type, ImAssayNameMaps& indexMaps)
{
    std::set<int> setRetAssay;
    for (const auto& imAssayInfo : m_mapImAssayCodeMaps)
    {
        // 判断类型(无效类型获取所有)
        if (type != tf::DeviceType::DEVICE_TYPE_INVALID && imAssayInfo.second->deviceType != type)
        {
            continue;
        }

        // 项目已经添加过了，则跳过
        if (setRetAssay.find(imAssayInfo.first) != setRetAssay.end())
        {
            continue;
        }

        // 必须可用
        //         if (!imAssayInfo.second->enable)
        //         {
        //             continue;
        //         }

        // 从通过项目表中查找对应的项目名称
        std::shared_ptr<tf::GeneralAssayInfo> spAssay = GetAssayInfo(imAssayInfo.first);
        if (spAssay == nullptr)
        {
            continue;
        }

        // 插入表中
        indexMaps.push_back(std::make_pair(spAssay->assayName, spAssay));
        setRetAssay.insert(spAssay->assayCode);
    }

    return true;
}

void CommonInformationManager::TranseAssayNameToAssayCode(const QStringList& assayNames, std::vector<int>& assayCode)
{
    for (const QString& aN : assayNames)
    {
        AssayIndexNameMaps::iterator it = m_mapAssayNameMaps.find(aN.toStdString());
        if (it != m_mapAssayNameMaps.end())
        {
            assayCode.push_back(it->second->assayCode);
        }
    }
}

///
/// @brief	获取当前单位转换倍率
///
/// @param[in]  assayCode   项目编号
/// @param[in]  factor		单位转换倍率
///
/// @return		成功返回true
///
/// @par History:
/// @li 7951/LuoXin，2022年9月5日，新建函数
///
bool CommonInformationManager::GetUnitFactor(int assayCode, double& factor)
{
    auto spAssayInfo = GetAssayInfo(assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo By assayCode[%d] Failed! ", assayCode);
        return false;
    }

    for (const tf::AssayUnit& unit : spAssayInfo->units)
    {
        if (unit.isCurrent)
        {
            factor = unit.factor;
            return true;
        }
    }

    ULOG(LOG_ERROR, "Not Find Current Unit Factor !");
    return false;
}

void CommonInformationManager::GetAssayCodeListCh(tf::DeviceType::type deviceType, std::vector<int32_t>& assayCodeList)
{
    bool validType = (deviceType != tf::DeviceType::DEVICE_TYPE_INVALID);
    for (const auto& assayItem : m_mapChAssayCodeMaps)
    {
        // 判断类型(无效类型获取所有),必须可用, 过滤掉血清指数
        if ((validType && assayItem.second->deviceType != deviceType) ||
            !assayItem.second->enable ||
            assayItem.first >= ch::tf::g_ch_constants.MAX_REAGENT_CODE)
        {
            continue;
        }
        assayCodeList.push_back(assayItem.first);
    }
}

void CommonInformationManager::GetAssayCodeListIse(tf::DeviceType::type deviceType, std::vector<int32_t>& assayCodeList)
{
    bool validType = (deviceType != tf::DeviceType::DEVICE_TYPE_INVALID);
    for (const auto& assayItem : m_mapIseAssayCodeMaps)
    {
        // 判断类型(无效类型获取所有),必须可用, 过滤ISE组合名
        if ((validType && assayItem.second->deviceType != deviceType) ||
            !assayItem.second->enable ||
            assayItem.first == ::ise::tf::g_ise_constants.ASSAY_CODE_ISE)
        {
            continue;
        }
        assayCodeList.push_back(assayItem.first);
    }
}

void CommonInformationManager::GetAssayCodeListIm(tf::DeviceType::type deviceType, std::vector<int32_t>& assayCodeList)
{
    bool validType = (deviceType != tf::DeviceType::DEVICE_TYPE_INVALID);
    for (const auto& assayItem : m_mapImAssayCodeMaps)
    {
        // 判断类型(无效类型获取所有),必须可用, 过滤ISE组合名
        if (validType && assayItem.second->deviceType != deviceType)
        {
            continue;
        }
        assayCodeList.push_back(assayItem.first);
    }
}

void CommonInformationManager::GetAssayItems(::tf::AssayClassify::type typeInfo, int32_t pageIdx, std::vector<tf::GeneralAssayPageItem>& outPage)
{
    for (AssayIndexCodeMaps::iterator it = m_mapAssayCodeMaps.begin(); it != m_mapAssayCodeMaps.end(); ++it)
    {
        if (it->second->assayClassify == typeInfo && it->second->pageIdx == pageIdx)
        {
            tf::GeneralAssayPageItem pageItem;
            pageItem.__set_pageIdx(it->second->pageIdx);
            pageItem.__set_positionIdx(it->second->positionIdx);
            pageItem.__set_assayCode(it->second->assayCode);
            pageItem.__set_assayClassify((tf::AssayClassify::type)it->second->assayClassify);
            pageItem.__set_reportAssay(it->second->reportAssay);
            // 试剂异常（待完善）
            pageItem.__set_masked(true);
            pageItem.__set_reagentAbnormal(true);
            pageItem.__set_caliAbnormal(true);

            outPage.push_back(pageItem);
		}
	}
}

void CommonInformationManager::GetAssayProfiles(::tf::AssayClassify::type typeInfo, int32_t pageIdx, std::vector<::tf::ProfileAssayPageItem>& outPage)
{
	for (AssayProfileMap::iterator it = m_mapProfileAssay.begin(); it != m_mapProfileAssay.end(); ++it)
	{
		if (it->second->assayClassify == typeInfo && it->second->pageIdx == pageIdx)
		{
			tf::ProfileAssayPageItem pageItem;
			pageItem.__set_pageIdx(it->second->pageIdx);
			pageItem.__set_positionIdx(it->second->positionIdx);
			pageItem.__set_id(it->second->id);
			pageItem.__set_assayClassify(it->second->assayClassify);

			// 包含的子项目
			if (!it->second->subItems.empty())
			{
				pageItem.__set_subAssays(it->second->subItems);
			}
			outPage.push_back(pageItem);
		}
	}
}

///
/// @brief 获取当前单位
///
/// @param[in]  assayCode  项目编号
///
/// @return 单位
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月13日，新建函数
///
QString CommonInformationManager::GetCurrentUnit(int assayCode)
{
	// 非计算项目
	if (!IsCalcAssay(assayCode))
	{
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = GetAssayInfo(assayCode);
		if (nullptr == spAssayInfo)
		{
			return "";
		}

		for (const tf::AssayUnit& unit : spAssayInfo->units)
		{
			if (unit.isCurrent)
			{
				return QString::fromStdString(unit.name);
			}
		}
	}
	// 计算项目
	else
	{
		std::shared_ptr<tf::CalcAssayInfo> spCalcAssayInfo = GetCalcAssayInfo(assayCode);
		if (spCalcAssayInfo)
		{
			return QString::fromStdString(spCalcAssayInfo->resultUnit);
		}
	}

	return "";
}

///
/// @brief 获取项目主单位
///
/// @param[in]  assayCode 项目编号
///
/// @return 项目的主单位
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月6日，新建函数
///
QString CommonInformationManager::GetAssayMainUnit(const int& assayCode)
{
	// 获取项目主单位
	QString mainUnit = "";
	auto spAssay = GetAssayInfo(assayCode);
	for (const tf::AssayUnit& unit : spAssay->units)
	{
		if (unit.isMain)
		{
			mainUnit = QString::fromStdString(unit.name);
            break;
		}
	}
	return mainUnit;

}

double CommonInformationManager::GetUnitChangeRate(int assayCode)
{
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = GetAssayInfo(assayCode);
    if (nullptr == spAssayInfo)
    {
        return 1;
    }
    double mainRat = -1;
    double curRat = -1;
    for (const ::tf::AssayUnit& ut : spAssayInfo->units)
    {
        if (ut.isCurrent && ut.isMain)
        {
            return 1;
        }

        if (ut.isMain)
        {
            mainRat = ut.factor;
        }
        else if (ut.isCurrent)
        {
            curRat = ut.factor;
        }
    }
    if (mainRat > 0 && curRat > 0)
    {
        return (curRat / mainRat);
    }

    return 1;
}

tf::RackType::type CommonInformationManager::GetRackTypeFromRack(int rack)
{
	 ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, rack);

	 std::shared_ptr<RackNumRange> pRnr = DictionaryQueryManager::GetInstance()->GetRackRange();
	 if (pRnr == nullptr)
	 {
		 ULOG(LOG_ERROR, "Null racknumranger settings.");
		 return ::tf::RackType::type::RACK_TYPE_INVALID;
	 }

	 // 判断value是否在Lower和upper之间,若是返回true否则返回false
	 auto fRangeIn = [](int lower, int upper, int value)->bool
	 {
		 return (value >= lower && value <= upper);
	 };

	 // 急诊
	 if (fRangeIn(pRnr->statLowerLmt, pRnr->statUpperLmt, rack))
	 {
		 return ::tf::RackType::type::RACK_TYPE_STAT;
	 }

	 // 常规
	 if (fRangeIn(pRnr->normalLowerLmt, pRnr->normalUpperLmt, rack))
	 {
		 return ::tf::RackType::type::RACK_TYPE_ROUTINE;
	 }

	 // 校准
	 if (fRangeIn(pRnr->caliLowerLmt, pRnr->caliUpperLmt, rack))
	 {
		 return ::tf::RackType::type::RACK_TYPE_CALI;
	 }

	 // 质控
	 if (fRangeIn(pRnr->qcLowerLmt, pRnr->qcUpperLmt, rack))
	 {
		 return ::tf::RackType::type::RACK_TYPE_QC;
	 }

	 // 清洗
	 if (fRangeIn(pRnr->washLowerLmt, pRnr->washUpperLmt, rack))
	 {
		 return ::tf::RackType::type::RACK_TYPE_WASH;
	 }

	 // 复查
	 if (fRangeIn(pRnr->rerunLowerLmt, pRnr->rerunUpperLmt, rack))
	 {
		 return ::tf::RackType::type::RACK_TYPE_RERUN;
	 }

	 return ::tf::RackType::type::RACK_TYPE_INVALID;
}

bool CommonInformationManager::IsRackRangOut(::tf::RackType::type rackType, int rack)
{
    std::shared_ptr<RackNumRange> pRnr = DictionaryQueryManager::GetInstance()->GetRackRange();
    if (pRnr == nullptr)
    {
        ULOG(LOG_ERROR, "Null racknumranger settings.");
        return false;
    }

	// 判断value是否在Lower和upper之间,若是返回false否则返回true
	auto fRangeOut = [](int lower, int upper, int value)->bool
	{
        return (value < lower || value > upper);
	};

	// 急诊
	if (rackType == ::tf::RackType::type::RACK_TYPE_STAT)
	{
		return fRangeOut(pRnr->statLowerLmt, pRnr->statUpperLmt, rack);
    }
	// 常规
	if (rackType == ::tf::RackType::type::RACK_TYPE_ROUTINE)
	{
		return fRangeOut(pRnr->normalLowerLmt, pRnr->normalUpperLmt, rack);
	}
	// 校准
	if (rackType == ::tf::RackType::type::RACK_TYPE_CALI)
	{
		return fRangeOut(pRnr->caliLowerLmt, pRnr->caliUpperLmt, rack);
	}
	// 质控
	if (rackType == ::tf::RackType::type::RACK_TYPE_QC)
	{
		return fRangeOut(pRnr->qcLowerLmt, pRnr->qcUpperLmt, rack);
	}
	// 清洗
	if (rackType == ::tf::RackType::type::RACK_TYPE_WASH)
	{
		return fRangeOut(pRnr->washLowerLmt, pRnr->washUpperLmt, rack);
	}
	// 复查
	if (rackType == ::tf::RackType::type::RACK_TYPE_RERUN)
	{
		return fRangeOut(pRnr->rerunLowerLmt, pRnr->rerunUpperLmt, rack);
	}

    return false;
}

///
/// @brief 获取架号范围
///
/// @param[in]  rackType  样本架类型
///
/// @return 某种类型的架号范围
///
/// @par History:
/// @li 1556/Chenjianlin，2024年5月9日，新建函数
///
RackRangeOneType CommonInformationManager::GetRackRange(::tf::RackType::type rackType)
{
	std::shared_ptr<RackNumRange> pRnr = DictionaryQueryManager::GetInstance()->GetRackRange();
	if (pRnr == nullptr)
	{
		ULOG(LOG_ERROR, "Null racknumranger settings.");
		return RackRangeOneType();
	}
	// 急诊
	if (rackType == ::tf::RackType::type::RACK_TYPE_STAT)
	{
		return RackRangeOneType(pRnr->statLowerLmt, pRnr->statUpperLmt);
	}
	// 常规
	if (rackType == ::tf::RackType::type::RACK_TYPE_ROUTINE)
	{
		return RackRangeOneType(pRnr->normalLowerLmt, pRnr->normalUpperLmt);
	}
	// 校准
	if (rackType == ::tf::RackType::type::RACK_TYPE_CALI)
	{
		return RackRangeOneType(pRnr->caliLowerLmt, pRnr->caliUpperLmt);
	}
	// 质控
	if (rackType == ::tf::RackType::type::RACK_TYPE_QC)
	{
		return RackRangeOneType(pRnr->qcLowerLmt, pRnr->qcUpperLmt);
	}
	// 清洗
	if (rackType == ::tf::RackType::type::RACK_TYPE_WASH)
	{
		return RackRangeOneType(pRnr->washLowerLmt, pRnr->washUpperLmt);
	}
	// 复查
	if (rackType == ::tf::RackType::type::RACK_TYPE_RERUN)
	{
		return RackRangeOneType(pRnr->rerunLowerLmt, pRnr->rerunUpperLmt);
	}
	ULOG(LOG_ERROR, "Rack range error.");
	return RackRangeOneType();
}

bool CommonInformationManager::IsCaClHaveChorIM(int assayCode, bool isCh)
{
	auto spCalcAssayInfo = GetCalcAssayInfo(assayCode);
	if (spCalcAssayInfo == nullptr)
	{
		return false;
	}

	for (const auto code : spCalcAssayInfo->vecAssayCode)
	{
		auto spGeneralAssayInfo = GetAssayInfo(code);
		if (spGeneralAssayInfo == nullptr)
		{
			continue;
		}

		// 是否包含生化项目
		if (isCh)
		{
			if (spGeneralAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
				|| spGeneralAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE)
			{
				return true;
			}
		}
		// 是否包含免疫项目
		else
		{
			if (spGeneralAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
			{
				return true;
			}
		}
	}

	return false;
}

tf::AssayClassify::type CommonInformationManager::GetAssayClassify(int assayCode)
{
    // 生化项目
    if ((assayCode >= tf::AssayCodeRange::CH_RANGE_MIN)
        && (assayCode <= tf::AssayCodeRange::CH_RANGE_MAX))
    {
        // 生化ISE项目
        if ((assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA)
            || (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K)
            || (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL))
        {
            return tf::AssayClassify::ASSAY_CLASSIFY_ISE;
        }
        // 生化比色法项目
        else if (assayCode <= ch::tf::g_ch_constants.ASSAY_CODE_I)
        {
            return tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY;
        }
    }
    // 免疫项目
    else if ((assayCode >= tf::AssayCodeRange::IM_RANGE_MIN)
        && (assayCode <= tf::AssayCodeRange::IM_RANGE_MAX))
    {
        return tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE;
    }

    return tf::AssayClassify::ASSAY_CLASSIFY_OTHER;
}

///
/// @brief  获取免疫显示字符串
///
/// @param[in]  assayCode  通道号
/// @param[in]  result  结果
///
/// @return 显示字符
///
/// @par History:
/// @li 1226/zhangjing，2023年3月18日，新建函数
///
QString CommonInformationManager::GetImDisplyResultStr(int assayCode, const im::tf::AssayTestResult& result)
{
    auto isShowConc = [](const SampleShowSet& sampleSet, tf::SampleType::type sampleType)->bool {
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
    };

    QString flag("");
    QString strResult(""); //所有未计算都要求显示空白
    DictionaryQueryManager* dicIns = DictionaryQueryManager::GetInstance();
    SampleShowSet sampleSet = dicIns->GetSampleResultSet();

    if (result.concEdit > 0 && result.concEdit != result.conc)
    {
        flag = QString::fromStdString(sampleSet.editflag);
    }

    // 校准品/质控品直接显示信号值 
    if(!isShowConc(sampleSet, result.sampType) && result.RLUEdit < 0)
    {
        // 出错订单无信号值，显示空
        return strResult;
    }
    if (!isShowConc(sampleSet, result.sampType) && !flag.isEmpty())
    {
        strResult = flag + QString::number(result.RLUEdit);
        return strResult;
    }
    else if (!isShowConc(sampleSet, result.sampType))
    {
        strResult = QString::number(result.RLU);
        return strResult;
    }

    // 小数点后保留几位小数(默认2位)
    int iPrecision = 2;
    // 获取当前单位与主单位的转化倍率
    double factor = 1.0;
    // 项目类型
    tf::AssayClassify::type classi = tf::AssayClassify::ASSAY_CLASSIFY_OTHER;

    if (CommonInformationManager::IsCalcAssay(assayCode))
    {
        auto spCalcInfo = GetCalcAssayInfo(assayCode);
        if (spCalcInfo != Q_NULLPTR)
        {
            iPrecision = spCalcInfo->decimalDigit;
        }
    }
    else
    {
        auto spAssayInfo = GetAssayInfo(assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            iPrecision = spAssayInfo->decimalPlace;
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

    if (result.concEdit < 0 && result.conc < 0)
    {
        return strResult;
    }

    double dConc = (result.__isset.concEdit && result.concEdit > 0) ? (result.concEdit) : (result.conc);

    ResultPrompt resultProm = dicIns->GetResultTips();
    QString refFlag = "";
    if (result.sampType == tf::SampleType::SAMPLE_TYPE_PATIENT)
    {
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

        QString lowerdangeFlag;
        QString upperdangeFlag;
        if (resultProm.enabledCritical)
        {
            // BUG25793
            if (resultProm.outRangeMarkType == 1)
            {
                lowerdangeFlag = "↓!";
                upperdangeFlag = "↑!";
            }
            else if (resultProm.outRangeMarkType == 2)
            {
                lowerdangeFlag = "H!";
                upperdangeFlag = "L!";
            }
        }

        // 参考范围标志
        if (resultProm.enabledOutRange)
        {          
            if (result.__isset.refRangeJudge && !refFlags.empty())
            {
                if (result.refRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
                {
                    refFlag = refFlags[0];
                }
                else if (result.refRangeJudge == ::tf::RangeJudge::LESS_LOWER)
                {
                    refFlag = refFlags[1];
                }
            }
        }
        // 危急范围标志
        if (resultProm.enabledCritical && result.__isset.crisisRangeJudge)
        {           
            if (result.crisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
            {
                refFlag = upperdangeFlag;
            }
            else if (result.crisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
            {
                refFlag = lowerdangeFlag;
            }
        }
    }

    strResult = QString::number(dConc*factor, 'f', iPrecision) + refFlag;
    // 校准品不显示线性范围判定-bug19441
    if (result.sampType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
    {
        return strResult;
    }

    // 查询通用项目信息
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> imGAIs;
    GetImmuneAssayInfos(imGAIs, assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000));
    if (imGAIs.size() <= 0)
    {
        ULOG(LOG_ERROR, "Failed to get immune assayinfo by %d.", assayCode);
        return strResult;
    }
    // 线性范围判定	
    double diluteRadio = result.preDilutionFactor * result.dilutionFactor;
    // 定性项目不乘稀释倍数
    if (imGAIs[0]->caliType == 0 && diluteRadio != 1.0)
    {
        diluteRadio = 1.0;
        strResult = QString::number(dConc*diluteRadio*factor, 'f', iPrecision) + refFlag;
    }
    // 质控品校准品不显示线性范围-bug22836
    if (result.sampType != tf::SampleType::type::SAMPLE_TYPE_PATIENT)
    {
        return strResult;
    }
    // 有大于小于号仍然要加参考范围标志bug24912
    if (!imGAIs[0]->extenL && dConc < imGAIs[0]->techRangeMin*diluteRadio)
    {
        strResult = "<" + QString::number(imGAIs[0]->techRangeMin*factor*diluteRadio, 'f', iPrecision) + refFlag;
        return strResult;
    }
    else if (!imGAIs[0]->extenH && dConc > imGAIs[0]->techRangeMax*diluteRadio)
    {
        strResult = ">" + QString::number(imGAIs[0]->techRangeMax*factor*diluteRadio, 'f', iPrecision) + refFlag;
        return strResult;
    }

    return strResult;
}

bool CommonInformationManager::GetImisSingle()
{
    //若为流水线模式则返回false
    if (DictionaryQueryManager::GetInstance()->GetPipeLine())
    {
        return false;
    }
    else
    {
        //判断是否存在除免疫以外设备
        std::unique_lock<std::mutex> buffLock(m_devMapMtx);
        std::shared_ptr<tf::DeviceInfo> spDevInfo = nullptr;
        // 统计i6000仪器个数
        int ImI6000DeviceNum = 0;
        for (const auto& atDev : m_mapDevices)
        {
            // 不算轨道和无效设备
            if (::tf::DeviceType::DEVICE_TYPE_INVALID == atDev.second->deviceType
                || ::tf::DeviceType::DEVICE_TYPE_TRACK == atDev.second->deviceType)
            {
                continue;
            }

            // 不等于i6000则返回false
            if (::tf::DeviceType::DEVICE_TYPE_I6000 != atDev.second->deviceType)
            {
                return false;
            }

            //判断i6000设备个数，大于等于2为联机
            ImI6000DeviceNum ++;
        }
        if (ImI6000DeviceNum != 1)
        {
            return false;
        }
        return true;
    }
}

///
/// @brief 判断是否免疫电机设备，若是则返回此设备，其他情况返回空
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月2日，新建函数
///
std::shared_ptr<const tf::DeviceInfo> CommonInformationManager::GetImSingleDevice()
{
    std::unique_lock<std::mutex> buffLock(m_devMapMtx);
    std::shared_ptr<const tf::DeviceInfo> spDevInfo = nullptr;
    int iImDevCnt = 0;
    for (const auto& atDev : m_mapDevices)
    {
        // 不算轨道
        if (::tf::DeviceType::DEVICE_TYPE_INVALID == atDev.second->deviceType
            || ::tf::DeviceType::DEVICE_TYPE_TRACK == atDev.second->deviceType)
        {
            continue;
        }
        // 计数累加
        iImDevCnt++;
        // 如果是免疫设备
        if (atDev.second->deviceType == ::tf::DeviceType::DEVICE_TYPE_I6000)
        {
            spDevInfo = atDev.second;
        }
    }
    // 如果不止一个设备，返回空
    if (1 != iImDevCnt)
    {
        return nullptr;
    }

	return spDevInfo;
}

///
/// @brief	获取该单位转换倍率
///
/// @param[in]  assayCode   项目编号
/// @param[in]  unti        单位名称
/// @param[in]  factor		单位转换倍率
///
/// @return		成功返回true
///
/// @par History:
/// @li 1226/zhangjing，2023年3月28日，新建函数
///
bool CommonInformationManager::GetNewUnitFactor(int assayCode, const QString& unit, double& factor)
{
    auto spAssayInfo = GetAssayInfo(assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo By assayCode[%d] Failed! ", assayCode);
        return false;
    }

    std::string name = unit.toStdString();
    for (const tf::AssayUnit& unit : spAssayInfo->units)
    {
        if (unit.name == name)
        {
            factor = unit.factor;
            return true;
        }
    }

    ULOG(LOG_ERROR, "Not Find Current Unit Factor !");
    return false;
}

bool CommonInformationManager::IsExistGeneralAssayName(int assayCode, const std::string& assayName)
{
    // abbreviation name.
    // full name.
    for (const auto& item : m_mapAssayNameMaps)
    {
        if (item.second->assayCode != assayCode && (
            item.second->assayName == assayName || item.second->assayFullName == assayName))
        {
            return true;
        }
    }

    return false;
}

bool CommonInformationManager::DeleteAssayUnit(const QString& assayName, const QString& unitName, QString& errInfo)
{
    std::string strAssayName = assayName.toStdString();
    std::string strUnitName = unitName.toStdString();
    ULOG(LOG_INFO, "%s, assayName:%s, unitNmae:%s.", __FUNCTION__, strAssayName.c_str(), strUnitName.c_str());

    AssayIndexNameMaps::iterator it = m_mapAssayNameMaps.find(strAssayName);
    if (it == m_mapAssayNameMaps.end())
    {
        errInfo = tr("项目名称异常!");
        return false;
    }

    // 临时删除
    ::tf::GeneralAssayInfo tempAssayInfo = *(it->second);
    std::vector<tf::AssayUnit> &listUnit = tempAssayInfo.units;
    for (std::vector<tf::AssayUnit>::iterator uIter = listUnit.begin(); uIter != listUnit.end(); ++uIter)
    {
        if (uIter->name == strUnitName)
        {
            if (uIter->isCurrent)
            {
                errInfo = tr("此单位为当前正在使用单位，不能删除");
                return false;
            }
            listUnit.erase(uIter);
            break;
        }
    }

    // 执行数据库对应单位的删除
    if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(tempAssayInfo))
    {
        errInfo = tr("执行单位删除操作失败！");
        return false;
    }

    // 更新该项目缓存中的单位信息
    it->second->__set_units(tempAssayInfo.units);
    return true;
}

bool CommonInformationManager::CanDeleteAssayUnit(const QString& assayName, const QString& unitName, QString& errInfo)
{
    AssayIndexNameMaps::iterator it = m_mapAssayNameMaps.find(assayName.toStdString());
    if (it == m_mapAssayNameMaps.end())
    {
        errInfo = tr("项目名称异常!");
        return false;
    }

    // 临时删除
    std::vector<tf::AssayUnit> &listUnit = it->second->units;
    std::string strDelete = unitName.toStdString();
    for (std::vector<tf::AssayUnit>::iterator uIter = listUnit.begin(); uIter != listUnit.end(); ++uIter)
    {
        if (uIter->name == strDelete)
        {
            if (uIter->isCurrent)
            {
                errInfo = tr("此单位为当前正在使用单位，不能删除");
                return false;
            }
            break;
        }
    }

    return true;
}

SOFTWARE_TYPE CommonInformationManager::GetSoftWareType()
{
    std::unique_lock<std::mutex> buffLock(m_devMapMtx);

    bool hasCh = std::find_if(m_mapDevices.begin(), m_mapDevices.end(), [](auto& iter)
    {
        return iter.second->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY;
    }) != m_mapDevices.end();

    bool hasIm = std::find_if(m_mapDevices.begin(), m_mapDevices.end(), [](auto& iter)
    {
        return iter.second->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE;
    }) != m_mapDevices.end();

    if (hasCh && hasIm)
    {
        return CHEMISTRY_AND_IMMUNE;
    }

    if (hasCh)
    {
        return CHEMISTRY;
    }

    if (hasIm)
    {
        return IMMUNE;
    }

    return UNKNOW;
}

bool CommonInformationManager::HasDeviceEqualStatus(::tf::DeviceWorkState::type status)
{
    std::unique_lock<std::mutex> buffLock(m_devMapMtx);
    for (const auto& dev : m_mapDevices)
    {
        if (dev.second->status == status)
        {
            return true;
        }
    }

    return false;
}

void CommonInformationManager::DeleteUnionQcSettings(int64_t qcDocId)
{
    ULOG(LOG_INFO, "%s qcDocId:%lld.", __FUNCTION__, qcDocId);

    AssayIndexCodeMaps::iterator it = m_mapAssayCodeMaps.begin();
    for (; it != m_mapAssayCodeMaps.end(); ++it)
    {
        tf::QcJudgeParam& qcRule = it->second->qcRules;
        if (qcRule.XQCDocID == -1 && qcRule.YQCDocID == -1)
        {
            continue;
        }

        if (qcRule.XQCDocID == qcDocId || qcRule.YQCDocID == qcDocId)
        {
            qcRule.__set_XQCDocID(-1);
            qcRule.__set_YQCDocID(-1);
            if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(*it->second))
            {
                ULOG(LOG_ERROR, "Failed to modify generalassayinfo.");
                continue;
            }
        }
    }
}
