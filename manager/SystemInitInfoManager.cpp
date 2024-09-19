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
/// @file     SystemInitInfoManager.h
/// @brief    系统初始化信息管理器
///
/// @author   8580/GongZhiQiang
/// @date     2024年1月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年1月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SystemInitInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "src/public/DictionaryKeyName.h"
#include "thrift/DcsControlProxy.h"
#include "manager/SystemConfigManager.h"
#include "utility/SoftwareVersionDlg.h"
#include "src/dcs/manager/im/i6000/I6000AlarmCodeDefine.h"
#include "src/db/Dictionary.hxx"
#include "regcom.hpp"
#include <QCoreApplication>

// 系统日志文件相对路径
#define SYSTEM_LOG_FILES_PATH			("/logs")
// 系统报警码文件相对路径
#define SYSTEM_ALARM_CODE_FILES_PATH	("/dcs_cfg/alarm/alarm_desc")

///														 
/// @brief 初始化单例对象
///
std::shared_ptr<SystemInitInfoManager>      SystemInitInfoManager::s_instance;
std::recursive_mutex						SystemInitInfoManager::s_mtx;

SystemInitInfoManager::SystemInitInfoManager()
{
	
}

std::shared_ptr<SystemInitInfoManager> SystemInitInfoManager::GetInstance()
{
    // 检查是否存在单例对象
    if (nullptr == s_instance)
    {
        std::unique_lock<std::recursive_mutex> autoLock(s_mtx);

        // 检查单例对象是否存在
        if (nullptr == s_instance)
        {
            s_instance.reset(new SystemInitInfoManager);
        }
    }

    return s_instance;
}

bool SystemInitInfoManager::SystemDefaultInfoInit()
{
	// 保存系统UUID（debug版本使用）
#ifdef _DEBUG
	if (!SaveSystemUUID())
	{
		ULOG(LOG_ERROR, "Failed to execute SaveSystemUUID()");
		return false;
	}
#endif
	
	// 保存系统配置文件路径（仪器云需要）
	if (!SaveSystemPaths())
	{
		ULOG(LOG_ERROR, "Failed to execute SaveSystemPaths()");
		return false;
	}

	// 设置周维护间隔起始时间
	if (!SetMaintainIntervalSetTime())
	{
		ULOG(LOG_ERROR, "Failed to execute SetWeekMaintainIntervalTime()");
		return false;
	}

    //设置免疫软件仪器状态灯默认报警码
    if (!UpdateI6000AffectLightAlarms())
    {
        ULOG(LOG_ERROR, "Failed to execute UpdateI6000AffectLightAlarms()");
        return false;
    }

	// 保存版本号信息到数据库
	SoftwareVersionDlg::SaveVersion2Db();

	return true;
}

bool SystemInitInfoManager::SaveSystemUUID()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 如果注册了，就不设置UUID
	if (!SystemConfigManager::GetInstance()->GetNeedRegister())
	{
		ULOG(LOG_INFO, "The system has been registered!");
		return true;
	}

	// 获取仪器硬件编码
	std::string deviceCode;
	if (::reg::regcom::get_device_code(deviceCode) != EXIT_SUCCESS ||
		deviceCode.empty())
	{
		CFGLOG(ERROR, "get_device_code is failed!");
		return false;
	}

	// 存储UUID
	tf::DictionaryInfo di;
	// 设置数据
	di.__set_keyName(DKN_SYSTEM_UUID);
	di.__set_value(deviceCode);

	// 添加数据字典信息
	if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
	{
		ULOG(LOG_ERROR, "ModifyDictionaryInfo DKN_SYSTEM_UUID Failed");
		return false;
	}

	return true;
}


bool SystemInitInfoManager::SaveSystemPaths()
{
	// 应用程序路径
	std::string appPath = QCoreApplication::applicationDirPath().toStdString();

	// 日志文件路径
	std::string logPath = SYSTEM_LOG_FILES_PATH;

	// 报警码文件路径
	std::string alarmCodePath = SYSTEM_ALARM_CODE_FILES_PATH;

	std::string pathsJson;
	try
	{
		// 根节点
		Document doc;
		doc.SetObject();
		Document::AllocatorType& allocator = doc.GetAllocator();

		// 应用程序路径
		doc.AddMember("appPath", { appPath.c_str(), allocator }, allocator);
		// 日志文件路径
		doc.AddMember("logPath", { logPath.c_str(), allocator }, allocator);
		// 报警码文件路径
		doc.AddMember("alarmCodePath", { alarmCodePath.c_str(), allocator }, allocator);

		pathsJson = RapidjsonUtil::Rapidjson2String(doc);
	}
	catch (exception& ex)
	{
		ULOG(LOG_ERROR, "exception: %s", ex.what());
		return false;
	}
	catch (...)
	{
		ULOG(LOG_ERROR, "unknown exception");
		return false;
	}

	// 存储系统日志路径
	tf::DictionaryInfo di;
	// 设置数据
	di.__set_keyName(DKN_SYSTEM_PATHS);
	di.__set_value(pathsJson);

	// 添加数据字典信息
	if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
	{
		ULOG(LOG_ERROR, "ModifyDictionaryInfo System Log Path Failed");
		return false;
	}

	return true;
}

bool SystemInitInfoManager::SetMaintainIntervalSetTime()
{
    ULOG(LOG_INFO, __FUNCTION__);

	// 查询所有维护组
	tf::MaintainGroupQueryCond grpQryCond;
	tf::MaintainGroupQueryResp grpQryResp;
	if (!DcsControlProxy::GetInstance()->QueryMaintainGroup(grpQryResp, grpQryCond) 
		|| (grpQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS))
	{
		ULOG(LOG_ERROR, "Failed to query all maintaingroup.");
		return false;
	}

    // 对存在间隔维护设置的维护组 赋值初始配置时间
    std::vector<::tf::MaintainGroup> modifyedGroup;
    std::string curTime = GetCurrentLocalTimeString();
    for (auto& gItem : grpQryResp.lstMaintainGroups)
    {
        for (auto& item : gItem.autoMaintainCfgs)
        {
            if (item.intervalSetTime.empty())
            {
                item.__set_intervalSetTime(curTime);
                modifyedGroup.push_back(gItem);
                ULOG(LOG_INFO, "Set devSN:%s, %s, %s.", item.deviceSN.c_str(), gItem.groupName.c_str(), curTime.c_str());
            }
        }
    }

	// 存在修改则重新保存维护
	if (modifyedGroup.size() > 0)
	{
		if (!DcsControlProxy::GetInstance()->ModifyMaintainGroups(modifyedGroup))
		{
			ULOG(LOG_ERROR, "Faild to modify auto maintain config.");
			return false;
		}
	}
	
	return true;
}

///
/// @brief 更新免疫仪器灯报警码
///
///
/// @return true:更新成功
///
/// @par History:
/// @li 7915/LeiDingXiang，2024年1月31日，新建函数
///
bool SystemInitInfoManager::UpdateI6000AffectLightAlarms()
{
    ::tf::DictionaryInfoQueryResp ret;
    ::tf::DictionaryInfoQueryCond diqc;
    diqc.__set_keyName(DKN_I6000_AFFECT_LIGHT_ALARMS);
    DcsControlProxy::GetInstance()->QueryDictionaryInfo(ret, diqc);
    if (ret.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(ERROR, "Failed to execute QueryDictionaryInfo!");
        return false;
    }

    //存在则返回true
    if (!ret.lstDictionaryInfos.empty())
    {
        ULOG(LOG_INFO, "DKN_I6000_AFFECT_LIGHT_ALARMS's value was exist,dont modify.");
        return true;
    }

    std::set<AlarmCode> imSupplySet;
    imSupplySet.insert(GetAlarmCode(ERR_REAG_INSUFFICIENT));

    std::string json;

    if (!Dictionary::EncodeAlarmCodeSet(imSupplySet, json))
    {
        ULOG(LOG_ERROR, "Failed to execute EncodeAlarmCodeSet()");
        return false;
    }

    tf::DictionaryInfo di;
    di.__set_keyName(DKN_I6000_AFFECT_LIGHT_ALARMS);
    di.__set_value(json);

    // 添加数据字典信息
    ::tf::ResultLong iret;
    if (!DcsControlProxy::GetInstance()->AddDictionaryInfo(di, iret))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo System Log Path Failed");
        return false;
    }
    return true;
}

///
/// @brief 获取报警码
///
///
/// @return AlarmCode 报警码封装
///
/// @par History:
/// @li 7915/LeiDingXiang，2024年1月31日，新建函数
///
AlarmCode SystemInitInfoManager::GetAlarmCode(std::vector<int> Code)
{
    if (Code.size() != 3)
    {
        ULOG(LOG_ERROR, "GetAlarmCode Failed");
        return AlarmCode();
    }

    return AlarmCode(Code[0], Code[1], Code[2]);
}
