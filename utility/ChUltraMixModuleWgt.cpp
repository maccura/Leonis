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
/// @file     ChUltraMixModuleWgt.cpp
/// @brief    应用-节点调试-生化超声混匀模块
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
#include "ChUltraMixModuleWgt.h"
#include "ui_ChUltraMixModuleWgt.h" 
#include "ChNodeDebugWgt.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "ChNodeDebugConfigManager.h"
#include <QtEndian>

ChUltraMixModuleWgt::ChUltraMixModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChUltraMixModuleWgt)
	, m_parent(qobject_cast<ChNodeDebugWgt*>(parent))
{
    ui->setupUi(this);

	InitData();
    InitUi();
	InitConnect();
}

ChUltraMixModuleWgt::~ChUltraMixModuleWgt()
{
}

void ChUltraMixModuleWgt::UpdateUi(const string& devSn, shared_ptr<UltraMixModule> ultraMixDatas)
{
	m_devSN = devSn;
	m_spDatas = ultraMixDatas;

	// 清空表格数据
	ui->tableWidget->clearContents();
	ui->tableWidget->setRowCount(0);
	ui->textEdit->clear();
	ui->widget->hide();

	// 遍历超声混匀信息
	for (const auto &um : (*ultraMixDatas).vecUltraMix)
	{
		int rowCount = ui->tableWidget->rowCount();
		ui->tableWidget->insertRow(rowCount);

		// 名称
		auto item1 = new QTableWidgetItem(um.name);
		item1->setData(Qt::UserRole + 1, um.idx);
		item1->setData(Qt::UserRole + 2, um.paramAddr);
		item1->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget->setItem(rowCount, 0, item1);

		// 参数值
		auto pair = GetCaliParam(um.name, um.paramAddr);
		auto item2 = new QTableWidgetItem(QString::number(pair.second));
		item2->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget->setItem(rowCount, 1, item2);
	}
}

void ChUltraMixModuleWgt::UpdateEleCaliVals(const string & strData)
{
	ui->textEdit->clear();
	ui->textEdit->setText(QString::fromStdString(strData));

	// 先全置为0
	m_eleCaliVals.fill(0, m_eleCaliVals.size());

	// 解析返回值
}

void ChUltraMixModuleWgt::InitUi()
{
	// 初始化tableWidget
	ui->tableWidget->setColumnCount(2);
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("名称") << tr("参数值"));
	ui->tableWidget->verticalHeader()->setHidden(true);                                 // 隐藏列号
	ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);              // 设置每次选择单行
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);               // 设选择行
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);				// 禁止编辑
	ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);// 根据内容自适应
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);                   // 最后一列占满
	ui->tableWidget->setColumnWidth(0, 400);

	ui->pb_useRet->hide();
}

void ChUltraMixModuleWgt::InitData()
{
	// 电流标定默认返回24个值
	m_eleCaliVals.reserve(24);
}

void ChUltraMixModuleWgt::InitConnect()
{
	connect(ui->pb_save, &QPushButton::clicked, this, &ChUltraMixModuleWgt::OnSaveClicked);
	connect(ui->pb_eleCali, &QPushButton::clicked, this, &ChUltraMixModuleWgt::OnEleCaliClicked);
	connect(ui->pb_useRet, &QPushButton::clicked, this, &ChUltraMixModuleWgt::OnUseReturnClicked);
	connect(ui->tableWidget, &QTableWidget::itemClicked, this, &ChUltraMixModuleWgt::OnSelectTableItem);
}

pair<bool, int> ChUltraMixModuleWgt::GetCaliParam(const QString &name, const int addr)
{
	// 获取校准参数
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto cmd = cmds[ECT_READ_FLASH];
	if (cmd.scsd.data.cmdData.params.size() != 2)
	{
		ULOG(LOG_ERROR, u8"读取flash指令参数不为2.");
		return{ false, -1 };
	}

	// 设置flash地址
	cmd.scsd.data.cmdData.params[0].paramVal.__set_i32Val(addr);
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, 0, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeName=%s, cmdName=%s, nodeId=%d, sendData=%s, ret.canResp=%s)",
		m_devSN, name.toStdString(), cmd.cmdName, 0, ToString(cmd.scsd),
		ChExeCmdObject::toHexString(ret.canResp));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		return{ false, -1 };
	}

	if (ret.canResp.size() != 4)
	{
		ULOG(LOG_ERROR, u8"指令[%s]返回值要求为Int，下位机实际返回数据位数=%d", cmd.cmdName, ret.canResp.size());
		return{ false, -1 };
	}

	int32_t targetPos = -1;
	try
	{
		targetPos = qFromBigEndian<int32_t>(ret.canResp.data());
	}
	catch (const std::exception& ex)
	{
		ULOG(LOG_ERROR, "qFromBigEndian exception: %s", ex.what());
		return{ false, -1 };
	}

	return{ true, targetPos };

	return pair<bool, int>();
}

void ChUltraMixModuleWgt::OnSelectTableItem(QTableWidgetItem *item)
{
	if (item == nullptr)
	{
		return;
	}

	ui->pb_useRet->hide();
	ui->widget->show();
	int currRow = item->row();

	// 名称
	QString name = ui->tableWidget->item(currRow, 0)->text();
	ui->label_name->setText(name);

	// 参考值
	int currVal = ui->tableWidget->item(currRow, 1)->text().toInt();
	ui->spinBox_val->setValue(currVal);

	// 当前选中行是否是电流
	int idx = ui->tableWidget->item(currRow, 0)->data(Qt::UserRole + 1).toInt();
	if (idx >= 0)
	{
		//ui->pb_useRet->show();
	}
}

void ChUltraMixModuleWgt::OnSaveClicked()
{
	// 获取当前选中行
	int row = ui->tableWidget->currentRow();
	if (row < 0)
	{
		ULOG(LOG_ERROR, u8"未选中任何行.");
		return;
	}

	// 校准地址
	auto addr = ui->tableWidget->item(row, 0)->data(Qt::UserRole + 2).toInt();

	// 指令列表
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto writeFlashCmd = cmds[ECT_WRITE_FLASH];

	if (writeFlashCmd.scsd.data.cmdData.params.size() != 2)
	{
		ULOG(LOG_ERROR, u8"写入flash指令参数不为2.");
		return;
	}

	// 待写入参数
	auto val = ui->spinBox_val->value();

	// 更新校准参数地址
	writeFlashCmd.scsd.data.cmdData.params[0].paramVal.__set_i32Val(addr);
	writeFlashCmd.scsd.data.cmdData.params[1].paramVal.__set_i32Val(val);

	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, 0, writeFlashCmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"写入flash指令: m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
		m_devSN, 0, ToString(writeFlashCmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_WARN, u8"写入flash失败.");
	}

	// 保存配置文件
	auto cmd = cmds[ECT_WRITE_CONFIG];
	ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, 0, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
		m_devSN, 0, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		return;
	}

	// 保存成功刷新界面
	ui->tableWidget->item(row, 1)->setText(QString::number(val));
}

void ChUltraMixModuleWgt::OnEleCaliClicked()
{
	// 执行电流标定
	m_parent->SetDataAndRunCmd(ChExeCmdObject::ET_EleCali, {}, tr("电流标定执行中，请稍等..."), m_spDatas->eleCaliCmd);
}

void ChUltraMixModuleWgt::OnUseReturnClicked()
{
	// 获取当前选中行
	int currRow = ui->tableWidget->currentRow();
	if (currRow < 0)
	{
		ULOG(LOG_ERROR, u8"未选中任何行.");
		return;
	}

	// 当前选中行是否是电流
	int idx = ui->tableWidget->item(currRow, 0)->data(Qt::UserRole + 1).toInt();
	if (idx < 0)
	{
		return;
	}

	if (m_eleCaliVals.size() >= idx + 1)
	{
		ui->spinBox_val->setValue(m_eleCaliVals.at(idx));
	}
}
