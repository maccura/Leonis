/***************************************************************************
*   This file is part of the BiochemistryDeviceUser project               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   xuxiaolong@maccura.com                                                *
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
/// @file     ChNodeDebugConfigManager.h
/// @brief    生化节点调试配置管理
///
/// @author   7997/XuXiaoLong
/// @date     2023年02月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年02月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "ChNodeDebugConfigManager.h"
#include "src/common/Mlog/mlog.h"
#include "manager/DictionaryQueryManager.h"

///
/// @brief 初始化单例对象
///
std::shared_ptr<ChNodeDebugConfigManager>    ChNodeDebugConfigManager::s_instance;
std::recursive_mutex                    ChNodeDebugConfigManager::s_mtx;

std::shared_ptr<ChNodeDebugConfigManager> ChNodeDebugConfigManager::GetInstance()
{
    // 检查是否存在单例对象
    if (nullptr == s_instance)
    {
        std::unique_lock<std::recursive_mutex> autoLock(s_mtx);

        // 检查单例对象是否存在
        if (nullptr == s_instance)
        {
            s_instance.reset(new ChNodeDebugConfigManager);
        }
    }

    return s_instance;
}

bool ChNodeDebugConfigManager::LoadConfig(const string& jsonFile)
{
    ULOG(LOG_INFO, "%s(jsonFile=%s)", __FUNCTION__, jsonFile);

    // 检查文件是否存在
	if (!boost::filesystem::exists(jsonFile))
	{
		ULOG(LOG_ERROR, "jsonFile:%s is not exist.", jsonFile);
		return{};
	}

	try
	{
		// 解析json字符串
		Document doc;
		if (!RapidjsonUtil::ParseJsonFile(jsonFile, doc))
		{
			ULOG(LOG_ERROR, "ParseJsonFile: %s failed.", jsonFile);
			return{};
		}

		// 指令列表
		const Value& vCmdList = RapidjsonUtil::GetArrayForKey(doc, "cmdList");
		ParseCmdList(vCmdList);

		// 步进模块列表
		const Value& vStepModuleList = RapidjsonUtil::GetArrayForKey(doc, "stepModuleList");
		ParseStepModuleList(vStepModuleList);

		// 开关模块列表
		const Value& vSwitchModuleList = RapidjsonUtil::GetArrayForKey(doc, "switchModuleList");
		ParseSwitchModuleList(vSwitchModuleList);

		// 超声混匀模块列表
		const Value& vUltraMixModuleList = RapidjsonUtil::GetArrayForKey(doc, "ultraMixModuleList");
		ParseUltraMixModuleList(vUltraMixModuleList);
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

    return true;
}

QMap<int, Cmd> ChNodeDebugConfigManager::GetCmdMaps() const
{
	return m_mapCmds;
}

QMap<int, ChNodeDebug> ChNodeDebugConfigManager::GetChNodeDebugMaps() const
{
	return m_mapChNodeDebugInfos;
}

ChNodeDebugConfigManager::ChNodeDebugConfigManager()
{

}

void ChNodeDebugConfigManager::ParseCmdList(const Value& vCmd)
{
	for (auto& cmd : vCmd.GetArray())
	{
		// 指令索引、指令id、指令参数
		int cmdIndex = -1;
		string cmdId, params, cmdName;

		cmdIndex = RapidjsonUtil::GetIntForKey(cmd, "cmdIndex");
		cmdId = RapidjsonUtil::GetStringForKey(cmd, "cmdId");
		params = RapidjsonUtil::GetStringForKey(cmd, "params");
		cmdName = RapidjsonUtil::GetStringForKey(cmd, "cmdName");

		// 未实现的指令不放入
		if (cmdId == "/")
		{
			ULOG(LOG_WARN, u8"未实现的指令：%s", cmdName);
			continue;
		}

		auto retPair = PackageCmd(cmdId, params);
		if (!retPair.first)
		{
			ULOG(LOG_WARN, "GetStCanSendData() failed.");
			continue;
		}

		retPair.second.cmdName = cmdName;
		m_mapCmds[cmdIndex] = retPair.second;
	}
}

void ChNodeDebugConfigManager::ParseStepModuleList(const Value& vStepModule)
{
	for (auto& module : vStepModule.GetArray())
	{
		// 模块id、单机还是联机
		int moduleId = RapidjsonUtil::GetIntForKey(module, "moduleId");
		string singleOrOnline = RapidjsonUtil::GetStringForKey(module, "singleOrOnline");

		// 单机不放入
		bool pipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();
		if ((pipeLine && singleOrOnline == u8"单机")
			|| (!pipeLine && singleOrOnline == u8"联机"))
		{
			continue;
		}

		// 内存中的模块信息
		ChNodeDebug cnd;

		// 步进模块信息
		shared_ptr<StepModule> spStepModuleInfo = nullptr;

		// 模块id重复，只需放入校准名称信息
		if (m_mapChNodeDebugInfos.contains(moduleId))
		{
			spStepModuleInfo = m_mapChNodeDebugInfos[moduleId].spStepModuleInfo;
		}
		else
		{
			// 模块类型
			cnd.moduleType = static_cast<EnumModuleType>(RapidjsonUtil::GetIntForKey(module, "moduleType"));
			// 模块名称
			cnd.moduleName = QString::fromStdString(RapidjsonUtil::GetStringForKey(module, "moduleName"));

			spStepModuleInfo = make_shared<StepModule>();

			// 点击模块执行指令
			spStepModuleInfo->enterModuleCmds = ConvertCmds(RapidjsonUtil::GetStringForKey(module, "clickModuleCmds"));

			// 切出模块执行指令
			spStepModuleInfo->outModuleCmds = ConvertCmds(RapidjsonUtil::GetStringForKey(module, "outModuleCmds"));

			// 点击复位执行指令
			spStepModuleInfo->clickResetCmds = ConvertCmds(RapidjsonUtil::GetStringForKey(module, "clickResetCmds"));

			cnd.spStepModuleInfo = spStepModuleInfo;
		}

		// 方向参数
		Direction d;
		// 方向名称
		d.name = QString::fromStdString(RapidjsonUtil::GetStringForKey(module, "direction"));
		// 节点id
		d.id = RapidjsonUtil::GetIntForKey(module, "nodeId");
		// 校准参数类型
		d.caliParamType = RapidjsonUtil::GetIntForKey(module, "caliParamType");
		// flash地址
		string flashAddrs = RapidjsonUtil::GetStringForKey(module, "flashAddrs");
		if (flashAddrs != "/" && !flashAddrs.empty())
		{
			vector<string> vecStrFlash;
			boost::split(vecStrFlash, flashAddrs, boost::is_any_of(","), boost::token_compress_on);
			vector<int> vecIntFlash(vecStrFlash.size());
			std::transform(vecStrFlash.begin(), vecStrFlash.end(), vecIntFlash.begin(), [](const string& str) {
				return stoi(str, nullptr, 16);
			});

			d.flashAddrs = vecIntFlash;
		}

		// 校准名称id
		int caliId = RapidjsonUtil::GetIntForKey(module, "caliId");

		// 校准id重复，只需放入方向
		if (spStepModuleInfo->caliParams.contains(caliId))
		{
			CaliParam &cp = spStepModuleInfo->caliParams[caliId];
			cp.directions.push_back(d);
		}
		else
		{
			// 校准参数
			CaliParam cp;
			// 校准名称
			cp.name = QString::fromStdString(RapidjsonUtil::GetStringForKey(module, "caliName"));
			// 点击校准参数名称提示
			auto tips = RapidjsonUtil::GetStringForKey(module, "tips");
			if (tips != "/")
			{
				cp.tips = QString::fromStdString(tips);
			}

			// 点击执行执行指令
			cp.clickExeCmds = ConvertCmds(RapidjsonUtil::GetStringForKey(module, "clickExeCmds"));
			// 方向
			cp.directions.push_back(d);

			spStepModuleInfo->caliParams[caliId] = cp;
		}

		if (!m_mapChNodeDebugInfos.contains(moduleId))
		{
			m_mapChNodeDebugInfos[moduleId] = cnd;
		}
	}
}

void ChNodeDebugConfigManager::ParseSwitchModuleList(const Value& vSwitchModule)
{
	for (auto& module : vSwitchModule.GetArray())
	{
		// 内存中的模块信息
		ChNodeDebug cnd;

		// 开关模块信息
		PVSModule cpvs;
		// 节点id
		cpvs.id = RapidjsonUtil::GetIntForKey(module, "nodeId");
		// 节点名称
		cpvs.name = QString::fromStdString(RapidjsonUtil::GetStringForKey(module, "nodeName"));

		// 节点指令数据
		vector<string> vecString{ RapidjsonUtil::GetStringForKey(module, "setIoH"),
			RapidjsonUtil::GetStringForKey(module, "setIoL"),
			RapidjsonUtil::GetStringForKey(module, "getIO") };
		for (const auto& cmd : vecString)
		{
			if (cmd == "/")
			{
				continue;
			}

			// 解析指令
			vector<string> vecCmd;
			boost::split(vecCmd, cmd, boost::is_any_of(","), boost::token_compress_on);
			if (vecCmd.size() <= 0)
			{
				continue;
			}

			// 组装数据
			CanSendData csd;
			csd.nodeId = cpvs.id;

			// 指令索引
			int cmdIdx = stoi(vecCmd[0]);

			// 2个值表示有参数
			if (vecCmd.size() == 2)
			{
				int len = vecCmd[1].length();
				if (len < 2)
				{
					ULOG(LOG_WARN, "vecCmd[1].length() < 2, cmd=%s", cmd);
					continue;
				}

				// 去除首位"{"和"}"
				auto param = vecCmd[1].substr(1, len - 2);

				// 解析参数值
				vector<string> vecParam;
				boost::split(vecParam, param, boost::is_any_of(";"), boost::token_compress_on);

				// 获取指令中的参数
				auto currCmd = m_mapCmds[cmdIdx];

				// 首先判断参数类型是否匹配
				int size = currCmd.scsd.data.cmdData.params.size();
				if (vecParam.size() != size)
				{
					ULOG(LOG_WARN, u8"指令参数和配置文件中的不一致, 指令名称=%s", currCmd.cmdName);
					continue;
				}

				// 替换指令中的参数
				ReplaceCmdParam(vecParam, currCmd);
				csd.cmd = currCmd;
			}
			else
			{
				// 只有1个值表示无需参数，直接取指令列表中的值
				csd.cmd = m_mapCmds[cmdIdx];
			}
			
			// 放入指令数据
			cpvs.sendDatas[cmdIdx] = csd;
		}

		// 模块id
		int moduleId = RapidjsonUtil::GetIntForKey(module, "moduleId");

		shared_ptr<QVector<PVSModule>> spCPVSModuleInfos = nullptr;
		// 包含该模块时，部分信息是一样的，无需再解析
		if (m_mapChNodeDebugInfos.contains(moduleId))
		{
			spCPVSModuleInfos = m_mapChNodeDebugInfos[moduleId].spVecCPVSModuleInfos;
		}
		else
		{
			// 模块类型
			cnd.moduleType = static_cast<EnumModuleType>(RapidjsonUtil::GetIntForKey(module, "moduleType"));
			// 模块名称
			cnd.moduleName = QString::fromStdString(RapidjsonUtil::GetStringForKey(module, "moduleName"));

			spCPVSModuleInfos = make_shared<QVector<PVSModule>>();
			cnd.spVecCPVSModuleInfos = spCPVSModuleInfos;
		}

		spCPVSModuleInfos->push_back(cpvs);

		if (!m_mapChNodeDebugInfos.contains(moduleId))
		{
			m_mapChNodeDebugInfos[moduleId] = cnd;
		}
	}
}

void ChNodeDebugConfigManager::ParseUltraMixModuleList(const Value & vUltraMixModule)
{
	for (const auto& um : vUltraMixModule.GetArray())
	{
		// 内存中的模块信息
		ChNodeDebug cnd;

		// 模块id
		int moduleId = RapidjsonUtil::GetIntForKey(um, "moduleId");

		shared_ptr<UltraMixModule> spUmModuleInfos = nullptr;
		// 包含该模块时，部分信息是一样的，无需再解析
		if (m_mapChNodeDebugInfos.contains(moduleId))
		{
			spUmModuleInfos = m_mapChNodeDebugInfos[moduleId].spUlMixModuleInfos;
		}
		else
		{
			spUmModuleInfos = make_shared<UltraMixModule>();
			cnd.spUlMixModuleInfos = spUmModuleInfos;

			// 模块类型
			cnd.moduleType = static_cast<EnumModuleType>(RapidjsonUtil::GetIntForKey(um, "moduleType"));
			// 模块名称
			cnd.moduleName = QString::fromStdString(RapidjsonUtil::GetStringForKey(um, "moduleName"));
			// 执行标定指令
			auto bdCmd = RapidjsonUtil::GetStringForKey(um, "BDcmds");
			
			// 解析指令
			vector<string> vecCmd;
			boost::split(vecCmd, bdCmd, boost::is_any_of(","), boost::token_compress_on);
			if (vecCmd.size() <= 0)
			{
				continue;
			}

			// 指令索引
			int cmdIdx = stoi(vecCmd[0]);

			// 2个值表示有参数
			if (vecCmd.size() == 2)
			{
				int len = vecCmd[1].length();
				if (len < 2)
				{
					ULOG(LOG_WARN, "vecCmd[1].length() < 2, cmd=%s", bdCmd);
					continue;
				}

				// 去除首位"{"和"}"
				auto param = vecCmd[1].substr(1, len - 2);

				// 解析参数值
				vector<string> vecParam;
				boost::split(vecParam, param, boost::is_any_of(";"), boost::token_compress_on);

				// 获取指令中的参数
				auto currCmd = m_mapCmds[cmdIdx];

				// 首先判断参数类型是否匹配
				int size = currCmd.scsd.data.cmdData.params.size();
				if (vecParam.size() != size)
				{
					ULOG(LOG_WARN, u8"指令参数和配置文件中的不一致, 指令名称=%s", currCmd.cmdName);
					continue;
				}

				// 替换指令中的参数
				ReplaceCmdParam(vecParam, currCmd);
				spUmModuleInfos->eleCaliCmd = currCmd;
			}
			else
			{
				// 只有1个值表示无需参数，直接取指令列表中的值
				spUmModuleInfos->eleCaliCmd = m_mapCmds[cmdIdx];
			}
		}

		// 超声混匀模块信息
		UltraMix ultraMixInfo;

		// 索引、参数名称、参数地址
		int idx = RapidjsonUtil::GetIntForKey(um, "eleRetIdx");
		QString name = QString::fromStdString(RapidjsonUtil::GetStringForKey(um, "paramName"));
		string strAddr = RapidjsonUtil::GetStringForKey(um, "paramAddr");
		int addr = 0;
		// 16进制
		if (strAddr.length() > 2 && (strAddr.substr(0, 2) == "0x" || strAddr.substr(0, 2) == "0X"))
		{
			addr = stoi(strAddr, nullptr, 16);
		}
		spUmModuleInfos->vecUltraMix.push_back(UltraMix(name, addr, idx));

		if (!m_mapChNodeDebugInfos.contains(moduleId))
		{
			m_mapChNodeDebugInfos[moduleId] = cnd;
		}
	}
}

std::pair<bool, Cmd> ChNodeDebugConfigManager::PackageCmd(const string &cmdId, const string& params)
{
	// 组装数据
	Cmd retData;
	// 指令类型
	retData.scsd.__set_type(tf::EmCanSendType::COMMAND);

	tf::UnCanData canData;
	tf::StCmdData cmdData;
	std::vector<tf::StCmdParam> stCmdParams;

	// 转换指令id（从string转为int，16进制）
	try
	{
		cmdData.__set_cmdId(stoi(cmdId, nullptr, 16));
	}
	catch (const std::exception& ex)
	{
		ULOG(LOG_ERROR, "std::exception error: %s,cmdId=%s, params=%s", ex.what(), cmdId, params);
		return{ false , retData };
	}

	// 无需参数
	if (params == "/")
	{
		canData.__set_cmdData(cmdData);
		retData.scsd.__set_data(canData);
		return{ true , retData };
	}

	// 解析参数
	vector<string> vecParams;
	boost::split(vecParams, params, boost::is_any_of("+"), boost::token_compress_on);

	// 遍历参数值
	for (auto &param : vecParams)
	{
		int len = param.length();
		if (len < 2)
		{
			ULOG(LOG_WARN, "param.length() < 2, cmdId=%d, params=%s", cmdId, params);
			continue;
		}

		// 去掉首位"("和")"
		param = param.substr(1, len - 2);

		// 解析参数类型和值
		vector<string> vecTypeVal;
		boost::split(vecTypeVal, param, boost::is_any_of(","), boost::token_compress_on);

		// vecTypeVal大小必为2
		if (vecTypeVal.size() != 2)
		{
			ULOG(LOG_WARN, u8"指令的格式不是(类型,参数)");
			continue;
		}

		// 命令参数
		tf::StCmdParam stCmdParam;
		ConvertParam(stoi(vecTypeVal[0]), vecTypeVal[1], stCmdParam);
		stCmdParams.push_back(std::move(stCmdParam));
	}

	if (!stCmdParams.empty())
	{
		cmdData.__set_params(stCmdParams);
	}

	canData.__set_cmdData(cmdData);
	retData.scsd.__set_data(canData);

	return{ true , retData };
}

QVector<CanSendData> ChNodeDebugConfigManager::ConvertCmds(const string &cmds)
{
	QVector<CanSendData> vecCanSendDatas;

	// 如果"点击执行按钮指令"列为空或者等于"/"，返回空
	if (cmds.empty() || cmds == "/")
	{
		return vecCanSendDatas;
	}

	// 分割执行指令
	vector<string> vecCmds;
	boost::split(vecCmds, cmds, boost::is_any_of("+"), boost::token_compress_on);

	// 遍历指令
	for (auto &cmd : vecCmds)
	{
		// 单个指令格式(0,1,{1;2})
		int len = cmd.length();
		if (len < 2)
		{
			ULOG(LOG_WARN, "cmd.length() < 2, cmds=%s", cmds);
			continue;
		}

		// 去掉首尾"("和")"
		cmd = cmd.substr(1, len - 2);

		// 分割单个指令
		vector<string> vecSingleCmd;
		boost::split(vecSingleCmd, cmd, boost::is_any_of(","), boost::token_compress_on);

		CanSendData csd;

		// 节点id
		int nodeId = stoi(vecSingleCmd[0]);
		csd.nodeId = nodeId;

		// 指令索引
		int cmdIdx = stoi(vecSingleCmd[1]);
		if (!m_mapCmds.contains(cmdIdx))
		{
			ULOG(LOG_WARN, "!m_mapCmds.contains(cmdIdx=%d)", cmdIdx);
			continue;
		}

		vector<string> vecParams;
		// 只有2个的表示该指令无需参数（0,1）,包含3个时才需取指令参数值
		if (vecSingleCmd.size() == 3)
		{
			// 分割参数
			int sLen = vecSingleCmd[2].length();
			if (sLen > 2)
			{
				// 去掉首尾"{"和"}"
				string param = vecSingleCmd[2].substr(1, sLen - 2);
				boost::split(vecParams, param, boost::is_any_of(";"), boost::token_compress_on);
			}
		}

		// 获取指令中的参数
		auto currCmd = m_mapCmds[cmdIdx];

		// 首先判断参数类型是否匹配
		int size = currCmd.scsd.data.cmdData.params.size();
		if (vecParams.size() != size)
		{
			ULOG(LOG_WARN, u8"指令参数和配置文件中的不一致, 指令名称=%s", currCmd.cmdName);
			continue;
		}

		// 替换指令中的参数
		ReplaceCmdParam(vecParams, currCmd);
		csd.cmd = currCmd;

		vecCanSendDatas.push_back(std::move(csd));
	}

	return vecCanSendDatas;
}

void ChNodeDebugConfigManager::ConvertParam(const int type, const string& strValue, tf::StCmdParam& outData)
{
	// 参数值
	tf::UnCanParamValue paramVal;

	try
	{
		// 参数类型
		switch (type)
		{
		case tf::EmCanParamType::INT32:
			outData.__set_paramType(tf::EmCanParamType::INT32);
			// 16进制
			if (strValue.length() > 2 && (strValue.substr(0, 2) == "0x" || strValue.substr(0, 2) == "0X"))
			{
				paramVal.__set_i32Val(stoi(strValue, nullptr, 16));
			}
			// 10进制
			else
			{
				paramVal.__set_i32Val(stoi(strValue));
			}
			break;
		case tf::EmCanParamType::FLOAT32:
			outData.__set_paramType(tf::EmCanParamType::FLOAT32);
			paramVal.__set_f32Val(stof(strValue));
			break;
		case tf::EmCanParamType::DOUBLE64:
			outData.__set_paramType(tf::EmCanParamType::DOUBLE64);
			paramVal.__set_d64Val(stod(strValue));
			break;
		default:
			break;
		}
	}
	catch (const std::exception& ex)
	{
		ULOG(LOG_ERROR, "std::exception error: %s.", ex.what());
	}

	outData.__set_paramVal(paramVal);
}

void ChNodeDebugConfigManager::ReplaceCmdParam(const std::vector<std::string> &vecParams, Cmd &cmd)
{
	int size = vecParams.size();

	// 替换指令中的参数
	for (int i = 0; i < size; i++)
	{
		auto &scp = cmd.scsd.data.cmdData.params[i];
		string value = vecParams[i];

		// 特殊处理 如果参数="caliParam",表示执行指令需将该参数替换为获取到的校准参数
		if (value == "caliParam")
		{
			// 用CALI_PARAM替换
			value = to_string(CALI_PARAM);
		}

		// 转换参数
		ConvertParam(scp.paramType, value, scp);
	}
}
