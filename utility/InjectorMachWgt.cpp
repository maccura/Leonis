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
/// @file     InjectorMachWgt.cpp
/// @brief    注射器电机界面
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
#include "InjectorMachWgt.h"
#include "ui_InjectorMachWgt.h" 
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "ChNodeDebugConfigManager.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "ChStepModuleWgt.h"
#include "tipdlg.h"

InjectorMachWgt::InjectorMachWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InjectorMachWgt)
	, m_parent(qobject_cast<ChStepModuleWgt*>(parent))
{
    ui->setupUi(this);

	InitData();
    InitUi();
	InitConnect();
}

InjectorMachWgt::~InjectorMachWgt()
{
}

void InjectorMachWgt::SetData(const QString& titleName, const QString& val, const std::string& devSN, 
	const int row, const int nodeId)
{
	m_row = row;
	m_nodeId = nodeId;
	m_devSN = devSN;

	ui->label_title->setText(titleName);
	ui->doubleSpinBox->setValue(val.toDouble());
	UpdateCurrPos();
}

void InjectorMachWgt::UpdateCurrPos()
{
	// 根据节点id获取电机当前位置
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto cmd = cmds[ECT_GET_ABSOL_POS];
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, m_nodeId, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"查询当前位置: m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
		m_devSN, m_nodeId, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"查询当前位置失败.");
		return;
	}

	// 查询位置返回的值为“true:xxx”
	std::string pattern = "true:";
	size_t pos = ret.canResp.find(pattern);
	if (pos == std::string::npos)
	{
		ULOG(LOG_ERROR, u8"查询当前位置返回值错误，必须包含'true:' ");
		return;
	}

	std::string numStr = ret.canResp.substr(pos + pattern.length());
	int currPos = -1;
	try
	{
		currPos = std::stoi(numStr);
	}
	catch (const std::exception& ex)
	{
		ULOG(LOG_ERROR, "std::stoi() exception: %s", ex.what());
		return;
	}

	ui->spinBox_currPos->setValue(currPos);
}

void InjectorMachWgt::InitUi()
{

}

void InjectorMachWgt::InitData()
{

}

void InjectorMachWgt::InitConnect()
{
	connect(ui->btn_save, &QPushButton::clicked, this, &InjectorMachWgt::OnSaveClicked);
	connect(ui->btn_frontMove, &QPushButton::clicked, this, &InjectorMachWgt::OnFrontMoveClicked);
	connect(ui->btn_backMove, &QPushButton::clicked, this, &InjectorMachWgt::OnBackMoveClicked);
}

void InjectorMachWgt::OnSaveClicked()
{
	if (m_parent->getSelectRows().empty())
	{
		TipDlg(tr("未选中对应校准参数!")).exec();
		return;
	}

	emit saveClicked(m_row, ui->doubleSpinBox->value());
}

void InjectorMachWgt::OnFrontMoveClicked()
{
	if (m_parent->getSelectRows().empty())
	{
		TipDlg(tr("未选中对应校准参数!")).exec();
		return;
	}

	// 根据节点id 正向移动电机
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto cmd = cmds[ECT_FRONT_MOVE];

	if (cmd.scsd.data.cmdData.params.size() != 2)
	{
		ULOG(LOG_ERROR, u8"正向移动指令参数不为2.");
		return;
	}

	// 设置正向移动步数
	cmd.scsd.data.cmdData.params[0].paramVal.__set_i32Val(ui->spinBox_pos->value());
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, m_nodeId, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"正向移动: m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
		m_devSN, m_nodeId, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"正向移动失败.");
		return;
	}

	// 移动后更新当前位置
	ui->spinBox_currPos->setValue(ui->spinBox_currPos->value() + ui->spinBox_pos->value());
}

void InjectorMachWgt::OnBackMoveClicked()
{
	if (m_parent->getSelectRows().empty())
	{
		TipDlg(tr("未选中对应校准参数!")).exec();
		return;
	}

	// 根据节点id 反向移动电机
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto cmd = cmds[ECT_BACK_MOVE];
	if (cmd.scsd.data.cmdData.params.size() != 2)
	{
		ULOG(LOG_ERROR, u8"反向移动指令参数不为2.");
		return;
	}

	// 设置反向移动步数
	cmd.scsd.data.cmdData.params[0].paramVal.__set_i32Val(ui->spinBox_pos->value());
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, m_nodeId, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"反向移动: m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
		m_devSN, m_nodeId, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"反向移动失败.");
		return;
	}

	// 移动后更新当前位置
	ui->spinBox_currPos->setValue(ui->spinBox_currPos->value() - ui->spinBox_pos->value());
}
