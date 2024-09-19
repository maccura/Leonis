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
/// @file     ChPumpModuleWgt.cpp
/// @brief    应用-节点调试-生化泵模块   
///
/// @author   7997/XuXiaoLong
/// @date     2024年3月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年3月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ChPumpModuleWgt.h"
#include "ui_ChPumpModuleWgt.h" 
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include <boost/algorithm/string.hpp>
#include "ChNodeDebugConfigManager.h"

const QString ckFindName = "checkBox_";						// 查找checkBox前缀
const QString btnExeFindName = "btn_exe_";					// 查找执行按钮前缀
const QString btnSaveFindName = "btn_save_";				// 查找保存按钮前缀
const QString spinSetElecFindName = "spinBox_setElec_";		// 查找设置电压前缀
const QString spinTarElecFindName = "spinBox_tarElec_";		// 查找目标电压前缀

ChPumpModuleWgt::ChPumpModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChPumpModuleWgt)
{
    ui->setupUi(this);

	InitData();
    InitUi();
	InitConnect();
}

ChPumpModuleWgt::~ChPumpModuleWgt()
{
}

void ChPumpModuleWgt::UpdateUi(const string& devSn, shared_ptr<QVector<PVSModule>> pumpDatas)
{
	m_bUpdate = true;
	m_devSN = devSn;
	m_spDatas = pumpDatas;

	// 指令列表
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();

	int i = -1;
	for (auto& pump : (*pumpDatas))
	{
		i++;

		// 主循环泵
		if (i == 0)
		{
			// 获取当前PWM值
			if (!cmds.contains(ECT_GET_PWM))
			{
				ULOG(LOG_WARN, u8"获取当前PWM值指令未实现, nodeName=%s, nodeId=%d", pump.name.toStdString(), pump.id);
				continue;
			}

			auto pair = GetCurrPwm(cmds[ECT_GET_PWM], pump.name.toStdString(), pump.id);
			auto spinBox = this->findChild<QSpinBox*>(spinSetElecFindName + QString::number(i));
			spinBox->setValue(pair.second);
		}

		// 孵育泵、制冷泵
		if (i == 1 || i == 2)
		{
			// 获取当前电压值
			if (!cmds.contains(ECT_GET_ELEC))
			{
				ULOG(LOG_WARN, u8"获取泵电压值指令未实现, nodeName=%s, nodeId=%d", pump.name.toStdString(), pump.id);
				continue;

			}

			auto pair = GetCurrElec(cmds[ECT_GET_ELEC], pump.name.toStdString(), pump.id);
			auto spinBox = this->findChild<QSpinBox*>(spinSetElecFindName + QString::number(i));
			spinBox->setValue(pair.second);

			// 孵育泵、制冷泵无需设置开关
			continue;
		}

		// 开关checkbox
		auto ck = this->findChild<QCheckBox*>(ckFindName + QString::number(i));
		if (ck == nullptr)
		{
			continue;
		}
		ck->setText(pump.name);

		// 获取IO电平指令索引
		int cmdIdx = pump.findGetIoIndex();
		if (cmdIdx == -1)
		{
			ULOG(LOG_WARN, u8"获取IO电平(获取启动/停止)指令未实现: nodeName=%s", pump.name.toStdString());
			continue;
		}

		// 指令参数
		auto cmd = pump.sendDatas[cmdIdx].cmd;
		// 发送指令
		auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, pump.id, cmd.scsd);

		ULOG(LOG_INFO, "SendCanCommand(m_devSN=%s, nodeName=%s, cmdName=%s, nodeId=%d, sendData=%s, ret=%s)",
			m_devSN, pump.name.toStdString(), cmd.cmdName, pump.id, ToString(cmd.scsd), ToString(ret));

		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_WARN, u8"指令[%s]执行失败.", cmd.cmdName);
			continue;
		}

		if (ret.canResp != "Low" && ret.canResp != "Hight")
		{
			ULOG(LOG_WARN, u8"指令[%s]返回值错误，必须为Low或者Hight.", cmd.cmdName);
			continue;
		}

		ck->setChecked(ret.canResp == "Low");
	}

	m_bUpdate = false;
}

void ChPumpModuleWgt::InitUi()
{
	// 设置属性
	for (int i = 0; i < 6; i++)
	{
		// 设置QCheckBox属性
		auto ckBox = this->findChild<QCheckBox*>(ckFindName + QString::number(i));
		if (ckBox)
		{
			ckBox->setProperty("idx", i);
		}

		// 为主循环泵、孵育泵、制冷泵等按钮添加相应的信息
		if (i < 3)
		{
			auto btnExe = this->findChild<QPushButton*>(btnExeFindName + QString::number(i));
			if (btnExe)
			{
				btnExe->setProperty("idx", i);
			}
			auto btnSave = this->findChild<QPushButton*>(btnSaveFindName + QString::number(i));
			if (btnSave)
			{
				btnSave->setProperty("idx", i);
			}

			// 设置电压不能更改
			auto spinBox = this->findChild<QSpinBox*>(spinSetElecFindName + QString::number(i));
			if (spinBox)
			{
				spinBox->setEnabled(false);
			}

			// 设置孵育泵和制冷泵开关默认打开且disable
			if (i != 0)
			{
				ckBox->setChecked(true);
				ckBox->setEnabled(false);
			}
		}
	}
}

void ChPumpModuleWgt::InitData()
{

}

void ChPumpModuleWgt::InitConnect()
{
	for (auto& ck : this->findChildren<QCheckBox*>())
	{
		connect(ck, &QCheckBox::clicked, this, &ChPumpModuleWgt::OnClickCheckBox);
	}

	for (auto& btn : this->findChildren<QPushButton*>())
	{
		connect(btn, &QPushButton::clicked, this, &ChPumpModuleWgt::OnClickPushButton);
	}
}

std::pair<bool, int> ChPumpModuleWgt::GetCurrPwm(const Cmd& cmd, const std::string& nodeName, const int nodeId)
{
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, nodeId, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, "SendCanCommand(m_devSN=%s, nodeName=%s, cmdName=%s, nodeId=%d, sendData=%s, ret=%s)",
		m_devSN, nodeName, cmd.cmdName, nodeId, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		return{ false, -1 };
	}

	// 以“,”分割
	vector<string> vecStrParam;
	boost::split(vecStrParam, ret.canResp, boost::is_any_of(","), boost::token_compress_on);
	if (vecStrParam.size() != 2)
	{
		ULOG(LOG_ERROR, u8"指令[%s]返回值错误，未包含','且个数不为2", cmd.cmdName);
		return{ false, -1 };
	}

	// 查询PWM返回的值为“period:xxx,dutyRatio:xxx”
	string patternPer = "period:", patternDuty = "dutyRatio:";
	size_t posPer = vecStrParam[0].find(patternPer);
	size_t posDuty = vecStrParam[1].find(patternDuty);
	if (posPer == std::string::npos || posDuty == std::string::npos)
	{
		ULOG(LOG_ERROR, u8"指令[%s]返回值错误，必须包含'period:'和'dutyRatio:'", cmd.cmdName);
		return{ false, -1 };
	}

	// 周期
	std::string period = ret.canResp.substr(posPer + patternPer.length());
	// 占空比
	std::string dutyRatio = ret.canResp.substr(posDuty + patternDuty.length());

	int curPwm = -1;
	try
	{
		curPwm = std::stoi(dutyRatio) / 6000;
	}
	catch (const std::exception& ex)
	{
		ULOG(LOG_ERROR, "std::stoi() exception: %s", ex.what());
		return{ false, -1 };
	}

	return{ true, curPwm };
}

std::pair<bool, int> ChPumpModuleWgt::GetCurrElec(const Cmd& cmd, const std::string& nodeName, const int nodeId)
{
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, nodeId, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, "SendCanCommand(m_devSN=%s, nodeName=%s, cmdName=%s, nodeId=%d, sendData=%s, ret=%s)",
		m_devSN, nodeName, cmd.cmdName, nodeId, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		return{ false, -1 };
	}

	string pattern = u8"泵电压:";
	int pos = ret.canResp.find(pattern);
	if (pos == std::string::npos)
	{
		ULOG(LOG_ERROR, u8"指令[%s]返回值错误，未包含'泵电压:'", cmd.cmdName);
		return{ false, -1 };
	}

	// 获取电压值
	std::string strElec = ret.canResp.substr(pos + pattern.length());

	int curElec = -1;
	try
	{
		curElec = std::stoi(strElec);
	}
	catch (const std::exception& ex)
	{
		ULOG(LOG_ERROR, "std::stoi() exception: %s", ex.what());
		return{ false, -1 };
	}

	return{ true, curElec };
}

void ChPumpModuleWgt::OnClickCheckBox(bool ck)
{
	if (m_bUpdate)
	{
		return;
	}

	auto ckBox = qobject_cast<QCheckBox*>(QObject::sender());
	if (ckBox == nullptr)
	{
		return;
	}

	int idx = ckBox->property("idx").toInt();
	if (idx < 0)
	{
		return;
	}

	// 获取节点数据
	PVSModule &data = (*m_spDatas)[idx];
	// 指令索引
	int cmdIdx = ck ? data.findSetIoLIndex() : data.findSetIoHIndex();
	std::string str = ck ? u8"设置IO低电平(启动)指令" : u8"设置IO高电平(停止)指令";
	if (cmdIdx == -1)
	{
		ULOG(LOG_ERROR, u8"%s未实现: nodeName=%s", str, data.name.toStdString());
		return;
	}

	// 指令参数
	auto cmd = data.sendDatas[cmdIdx].cmd;

	// 发送指令
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, data.id, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"%s: m_devSN=%s, nodeName=%s, nodeId=%d, sendData=%s, ret=%s",
		str, m_devSN, data.name.toStdString(), data.id, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"%s失败.", str);
		return;
	}
}

void ChPumpModuleWgt::OnClickPushButton()
{
	auto btn = qobject_cast<QPushButton*>(QObject::sender());
	if (btn == nullptr)
	{
		return;
	}

	int idx = btn->property("idx").toInt();
	QString text = btn->objectName();
	if (idx < 0)
	{
		return;
	}

	// 指令列表
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();

	// 节点数据
	PVSModule data = (*m_spDatas)[idx];
	int nodeId = data.id;
	string nodeName = data.name.toStdString();

	// 执行
	if (text.contains(btnExeFindName))
	{
		auto tarSpinBox = this->findChild<QSpinBox*>(spinTarElecFindName + QString::number(idx));
		auto setSpinBox = this->findChild<QSpinBox*>(spinSetElecFindName + QString::number(idx));
		int elecVal = tarSpinBox->value();

		// 主循环泵
		if (idx == 0)
		{
			// 获取指令
			if (!cmds.contains(ECT_SET_PWM))
			{
				ULOG(LOG_WARN, u8"设置PWM指令未实现, nodeName=%s.", nodeName);
				return;
			}

			// 组装数据
			auto cmd = cmds[ECT_SET_PWM];
			if (cmd.scsd.data.cmdData.params.size() != 2)
			{
				ULOG(LOG_ERROR, u8"指令[%s]参数不为2.", cmd.cmdName);
				return;
			}

			// 周期固为3000，占空比=6000 * 电压
			cmd.scsd.data.cmdData.params[1].paramVal.__set_i32Val(6000 * elecVal);

			// 设置PWM值指令
			auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, nodeId, cmd.scsd);
			ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeName=%s, nodeId=%d, cmdName=%s, sendData=%s, ret=%s)",
				m_devSN, nodeName, nodeId, cmd.cmdName, ToString(cmd.scsd), ToString(ret));

			if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
				return;
			}

			// 成功后将数据赋值到对应的设置电压处
			setSpinBox->setValue(6000 * elecVal);
		}
		// 孵育泵、制冷泵
		else
		{
			// 获取指令
			if (!cmds.contains(ECT_SET_ELEC))
			{
				ULOG(LOG_WARN, u8"设置泵电压指令未实现, nodeName=%s.", nodeName);
				return;
			}

			// 组装数据
			auto cmd = cmds[ECT_SET_ELEC];
			if (cmd.scsd.data.cmdData.params.size() != 2)
			{
				ULOG(LOG_ERROR, u8"指令[%s]参数不为2.", cmd.cmdName);
				return;
			}

			// 修改电压值
			cmd.scsd.data.cmdData.params[1].paramVal.__set_i32Val(elecVal);

			// 设置泵电压指令
			auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, nodeId, cmd.scsd);
			ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeName=%s, nodeId=%d, cmdName=%s, sendData=%s, ret=%s)",
				m_devSN, nodeName, nodeId, cmd.cmdName, ToString(cmd.scsd), ToString(ret));

			if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
				return;
			}

			// 成功后将数据赋值到对应的设置电压处
			setSpinBox->setValue(elecVal);
		}
	}
	// 保存
	else if (text.contains(btnSaveFindName))
	{
		// 获取指令
		if (!cmds.contains(ECT_WRITE_CONFIG))
		{
			ULOG(LOG_WARN, u8"保存配置文件指令未实现, nodeName=%s.", nodeName);
			return;
		}
		auto cmd = cmds[ECT_WRITE_CONFIG];

		// 保存数据指令
		auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, nodeId, cmd.scsd);
		ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeName=%s, nodeId=%d, cmdName=%s, sendData=%s, ret=%s)",
			m_devSN, nodeName, nodeId, cmd.cmdName, ToString(cmd.scsd), ToString(ret));

		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		}
	}
}
