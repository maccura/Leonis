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
/// @file     ChSwitchModuleWgt.cpp
/// @brief    应用-节点调试-生化开关模块
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
#include "ChSwitchModuleWgt.h"
#include "ui_ChSwitchModuleWgt.h" 
#include <QCheckBox>
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"

ChSwitchModuleWgt::ChSwitchModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChSwitchModuleWgt)
{
    ui->setupUi(this);

	InitData();
    InitUi();
	InitConnect();
}

ChSwitchModuleWgt::~ChSwitchModuleWgt()
{
}

void ChSwitchModuleWgt::UpdateUi(const string& devSn, shared_ptr<QVector<PVSModule>> switchDatas)
{
	m_devSN = devSn;
	m_spDatas = switchDatas;

	// 将布局中所有的控件移除
	QLayoutItem *child = nullptr;
	while ((child = m_gridLayout->takeAt(0)) != nullptr)
	{
		delete child->widget();
		delete child;
	}

	int i = 0;
	for (auto &data : (*switchDatas))
	{
		QCheckBox* box = new QCheckBox(this);
		connect(box, &QCheckBox::clicked, this, &ChSwitchModuleWgt::OnClickCheckBox);
		box->setProperty("idx", i);
		box->setText(data.name);
		m_gridLayout->addWidget(box, i / 2, i % 2, Qt::AlignLeft | Qt::AlignTop);
		i++;

		// 获取IO电平指令索引
		int cmdIdx = data.findGetIoIndex();
		if (cmdIdx == -1)
		{
			ULOG(LOG_WARN, u8"获取IO电平(获取启动/停止)指令未实现: nodeName=%s", data.name.toStdString());
			continue;
		}

		// 指令参数
		auto cmd = data.sendDatas[cmdIdx].cmd;
		// 发送指令
		auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, data.id, cmd.scsd);

		ULOG(LOG_INFO, "SendCanCommand(m_devSN=%s, nodeName=%s, cmdName=%s, nodeId=%d, sendData=%s, ret=%s)",
			m_devSN, data.name.toStdString(), cmd.cmdName, data.id, ToString(cmd.scsd), ToString(ret));
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

		box->setChecked(ret.canResp == "Low");
	}

	// 添加一个弹簧
	auto verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_gridLayout->addItem(verticalSpacer, i / 2, i % 2, Qt::AlignLeft | Qt::AlignTop);
}

void ChSwitchModuleWgt::InitUi()
{
	ui->scrollArea->setStyleSheet(".QScrollArea{background: transparent;};");
	ui->widget->setStyleSheet(".QWidget{background: transparent;}");
}

void ChSwitchModuleWgt::InitData()
{
	m_gridLayout = new QGridLayout(ui->widget);
	m_gridLayout->setContentsMargins(21, 10, 0, -1);
	ui->widget->setLayout(m_gridLayout);
}

void ChSwitchModuleWgt::InitConnect()
{

}

void ChSwitchModuleWgt::OnClickCheckBox(bool ck)
{
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
