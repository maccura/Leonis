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
/// @file     ChValveModuleWgt.cpp
/// @brief    应用-节点调试-生化阀模块
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
#include "ChValveModuleWgt.h"
#include "ui_ChValveModuleWgt.h" 
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"

ChValveModuleWgt::ChValveModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChValveModuleWgt)
{
    ui->setupUi(this);

	InitData();
    InitUi();
	InitConnect();
}

ChValveModuleWgt::~ChValveModuleWgt()
{
}

void ChValveModuleWgt::UpdateUi(const string& devSn, shared_ptr<QVector<PVSModule>> valveDatas)
{
	m_bUpdate = true;
	m_devSN = devSn;
	m_spDatas = valveDatas;

	int i = 0;
	QString findName = "checkBox_";
	for (auto& valve : (*valveDatas))
	{
		i++;

		auto ck = this->findChild<QCheckBox*>(findName + QString::number(i));
		if (ck == nullptr)
		{
			continue;
		}
		ck->setText(valve.name);
		ck->setProperty("idx", i - 1);

		// 获取IO电平指令索引
		int cmdIdx = valve.findGetIoIndex();
		if (cmdIdx == -1)
		{
			ULOG(LOG_WARN, u8"获取IO电平(获取启动/停止)指令未实现: nodeName=%s", valve.name.toStdString());
			continue;
		}

		// 指令参数
		auto cmd = valve.sendDatas[cmdIdx].cmd;
		// 发送指令
		auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, valve.id, cmd.scsd);

		ULOG(LOG_INFO, "SendCanCommand(m_devSN=%s, nodeName=%s, cmdName=%s, nodeId=%d, sendData=%s, ret=%s)",
			m_devSN, valve.name.toStdString(), cmd.cmdName, valve.id, ToString(cmd.scsd), ToString(ret));
		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_WARN, u8"指令[%s]指令失败.", cmd.cmdName);
			continue;
		}

		if (ret.canResp != "Low" && ret.canResp != "Hight")
		{
			ULOG(LOG_WARN, u8"指令[%s]返回值错误，必须为Low或者Hight.", cmd.cmdName);
			continue;
		}

		// 是否勾选
		ck->setChecked(ret.canResp == "Low");
	}

	m_bUpdate = false;
}

void ChValveModuleWgt::InitUi()
{

}

void ChValveModuleWgt::InitData()
{

}

void ChValveModuleWgt::InitConnect()
{
	for (auto& ck : this->findChildren<QCheckBox*>())
	{
		connect(ck, &QCheckBox::clicked, this, &ChValveModuleWgt::OnClickCheckBox);
	}
}

void ChValveModuleWgt::OnClickCheckBox(bool ck)
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
