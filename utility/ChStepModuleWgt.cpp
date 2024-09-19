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
/// @file     ChStepModuleWgt.cpp
/// @brief    应用-节点调试-生化步进模块 
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
#include "ChStepModuleWgt.h"
#include "ui_ChStepModuleWgt.h" 
#include "DirectionWgt.h"
#include "InjectorMachWgt.h"
#include "ChExeCmdObj.h"
#include "ChNodeDebugWgt.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "shared/tipdlg.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "ChNodeDebugConfigManager.h"
#include <QtEndian>

ChStepModuleWgt::ChStepModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChStepModuleWgt)
	, m_parent(qobject_cast<ChNodeDebugWgt*>(parent))
{
    ui->setupUi(this);

	InitData();
    InitUi();
	InitConnect();
}

ChStepModuleWgt::~ChStepModuleWgt()
{
}

void ChStepModuleWgt::UpdateUi(const string& devSn, const QString& moduleName, shared_ptr<StepModule> stepData)
{
	// 缓存数据
	m_devSN = devSn;
	m_spData = stepData;
	m_moduleName = moduleName;

	// 清空表格数据
	ui->tableWidget->clearContents();
	ui->tableWidget->setRowCount(0);
	clearWidgets();
	ui->btn_execute->hide();

	// 遍历校准参数名称
	for (auto caliIt = (*stepData).caliParams.begin(); caliIt != (*stepData).caliParams.end(); ++caliIt)
	{
		// 校准索引
		int caliIdx = caliIt.key();

		// 校准参数
		auto caliValue = caliIt.value();

		// 待合并的行数
		int spanRows = caliValue.directions.size();
		// 合并单元格起始和终止行
		int spanStartRow = ui->tableWidget->rowCount();
		int spanEndRow = spanStartRow + spanRows - 1;

		// 方向索引
		int dirIdx = 0;
		// 遍历方向
		for (const auto& direction : caliValue.directions)
		{
			int rowCount = ui->tableWidget->rowCount();
			ui->tableWidget->insertRow(rowCount);

			// 名称
			auto item1 = new QTableWidgetItem(caliValue.name);
			item1->setData(Qt::UserRole + 1, caliIdx);
			item1->setData(Qt::UserRole + 2, dirIdx);
			item1->setData(Qt::UserRole + 3, QVariant::fromValue(pair<int, int>(spanStartRow, spanEndRow)));
			item1->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(rowCount, 0, item1);

			// 方向
			auto item2 = new QTableWidgetItem(direction.name);
			item2->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(rowCount, 1, item2);

			QString targetPos = "/";
			if (!direction.flashAddrs.empty())
			{
				// 查询每个节点的目标位置
				auto pair = GetTargetPos(direction, caliValue.name.toStdString());

				if (direction.caliParamType == ::tf::EmCanParamType::INT32)
				{
					targetPos = QString::number(pair.second.first);
				}
				else
				{
					targetPos = QString::number(pair.second.second);
				}
			}

			// 目标位置
			auto item3 = new QTableWidgetItem(targetPos);
			item3->setData(Qt::UserRole + 1, direction.id);
			item3->setTextAlignment(Qt::AlignCenter);
			ui->tableWidget->setItem(rowCount, 2, item3);

			dirIdx++;
		}

		// 合并单元格
		if (spanRows > 1)
		{
			ui->tableWidget->setSpan(spanStartRow, 0, spanRows, 1);
		}
	}
}

set<int> ChStepModuleWgt::getSelectRows() const
{
	set<int> selectRows;
	auto selectItems = ui->tableWidget->selectedItems();
	for (const auto& item : selectItems)
	{
		selectRows.insert(item->row());
	}

	return selectRows;
}

void ChStepModuleWgt::InitUi()
{
	// 初始化tableWidget
	ui->tableWidget->setColumnCount(3);
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("名称") << tr("方向") << tr("目标位置"));
	ui->tableWidget->verticalHeader()->setHidden(true);                                 // 隐藏列号
	ui->tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);               // 设置每次选择多行
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);               // 设选择行
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);				// 禁止编辑
	ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);// 根据内容自适应
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);                   // 最后一列占满
	ui->tableWidget->setColumnWidth(0, 450);
	ui->tableWidget->setColumnWidth(1, 180);

	ui->btn_execute->hide();
	ui->scrollArea->setStyleSheet(".QScrollArea{background: transparent;}");
	ui->widget->setStyleSheet(".QWidget{background: transparent;}");
}

void ChStepModuleWgt::InitData()
{
	m_layout = new QVBoxLayout(ui->widget);
	m_layout->setMargin(0);
	ui->widget->setLayout(m_layout);
}

void ChStepModuleWgt::InitConnect()
{
	connect(ui->btn_execute, &QPushButton::clicked, this, &ChStepModuleWgt::OnExecuteClicked);
	connect(ui->btnReset, &QPushButton::clicked, this, &ChStepModuleWgt::OnResetClicked);
	connect(ui->tableWidget, &QTableWidget::itemClicked, this, &ChStepModuleWgt::OnSelectTableItem);
}

std::pair<bool, pair<int, double>> ChStepModuleWgt::GetTargetPos(const Direction& data, const std::string& nodeName)
{
	if (data.flashAddrs.empty())
	{
		ULOG(LOG_WARN, u8"地址为空，无需执行.");
		return{ true, { -1, -1.0 } };
	}

	// 改变地址值=5个的以第3个为准，其余以第1个为准
	int flash = data.flashAddrs[0];
	if (data.flashAddrs.size() == 5)
	{
		flash = data.flashAddrs[2];
	}

	// 获取校准参数
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto cmd = cmds[ECT_READ_FLASH];
	if (cmd.scsd.data.cmdData.params.size() != 2)
	{
		ULOG(LOG_ERROR, u8"读取flash指令参数不为2.");
		return{ true,{ -1, -1.0 } };
	}

	// 设置flash地址
	cmd.scsd.data.cmdData.params[0].paramVal.__set_i32Val(flash);
	// 设置参数类型
	::tf::EmCanParamType::type pType = static_cast<::tf::EmCanParamType::type>(data.caliParamType);
	cmd.scsd.data.cmdData.params[1].__set_paramType(pType);

	tf::UnCanParamValue paramVal;
	if (pType == tf::EmCanParamType::INT32)
	{
		paramVal.__set_i32Val(0);
	}
	else if (pType == tf::EmCanParamType::DOUBLE64 || pType == tf::EmCanParamType::FLOAT32)
	{
		paramVal.__set_d64Val(0.0);
	}
	cmd.scsd.data.cmdData.params[1].__set_paramVal(paramVal);

	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, data.id, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeName=%s, cmdName=%s, nodeId=%d, direction=%s, sendData=%s, ret.canResp=%s)",
		m_devSN, nodeName, cmd.cmdName, data.id, data.name.toStdString(), ToString(cmd.scsd),
		ChExeCmdObject::toHexString(ret.canResp));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		return{ true,{ -1, -1.0 } };
	}

	if (pType == tf::EmCanParamType::INT32 && ret.canResp.size() != 4)
	{
		ULOG(LOG_ERROR, u8"指令[%s]返回值要求为Int，下位机实际返回数据位数=%d", cmd.cmdName, ret.canResp.size());
		return{ true,{ -1, -1.0 } };
	}
	else if (pType == tf::EmCanParamType::DOUBLE64 && ret.canResp.size() != 8)
	{
		ULOG(LOG_ERROR, u8"指令[%s]返回值要求为Double，下位机实际返回数据位数=%d", cmd.cmdName, ret.canResp.size());
		return{ true,{ -1, -1.0 } };
	}
	else if (pType == tf::EmCanParamType::FLOAT32 && ret.canResp.size() != 8)
	{
		ULOG(LOG_ERROR, u8"指令[%s]返回值要求为Float，下位机实际返回数据位数=%d", cmd.cmdName, ret.canResp.size());
		return{ true,{ -1, -1.0 } };
	}

	pair<int, double> targetPos = { -1, -1.0 };
	try
	{
		if (pType == tf::EmCanParamType::INT32)
		{
			targetPos.first = qFromBigEndian<int32_t>(ret.canResp.data());
		}
		else if (pType == tf::EmCanParamType::DOUBLE64 || pType == tf::EmCanParamType::FLOAT32)
		{
			int64_t tmp = qFromBigEndian<int64_t>(ret.canResp.data());
			targetPos.second = *((double*)(&tmp));
		}
	}
	catch (const std::exception& ex)
	{
		ULOG(LOG_ERROR, "qFromBigEndian exception: %s", ex.what());
		return{ true,{ -1, -1.0 } };
	}

	return{ true, targetPos };
}

void ChStepModuleWgt::clearWidgets()
{
	if (m_layout == nullptr)
	{
		ULOG(LOG_ERROR, "m_layout is nullptr");
		return;
	}

	QLayoutItem *item = nullptr;
	while ((item = m_layout->takeAt(0)) != nullptr)
	{
		delete item->widget();
		delete item;
	}

	m_dirWgtList.clear();
	m_injectWgt = nullptr;
}

void ChStepModuleWgt::RepalceParams(const pair<int, int>& rows, QVector<CanSendData>& cmds)
{
	// 获取方向上的数据
	map<int, int> mapNodeIdPos;
	for (int row = rows.first; row <= rows.second; ++row)
	{
		auto pos = ui->tableWidget->item(row, 2)->text().toStdString();
		if (pos == "/")
		{
			continue;
		}
		int nodeId = ui->tableWidget->item(row, 2)->data(Qt::UserRole + 1).toInt();
		mapNodeIdPos[nodeId] = stoi(pos);
	}

	// 遍历执行按钮指令 替换其中为CALI_PARAM的值
	for (auto &csd : cmds)
	{
		if (mapNodeIdPos.count(csd.nodeId))
		{
			// 如果指令参数为CALI_PARAM 表示需要替换成相应的校准参数
			for (auto &param : csd.cmd.scsd.data.cmdData.params)
			{
				if (param.paramType == tf::EmCanParamType::INT32
					&& param.paramVal.i32Val == CALI_PARAM)
				{
					param.paramVal.__set_i32Val(mapNodeIdPos[csd.nodeId]);
				}
			}
		}
	}
}

void ChStepModuleWgt::OnSaveDirClicked(const int row, const int currPos)
{
	if (ui->tableWidget->rowCount() < row + 1)
	{
		ULOG(LOG_ERROR, u8"表格现有行数小于%d.", row + 1);
		return;
	}

	ULOG(LOG_INFO, u8"保存校准参数: %s,方向: %s.", ui->tableWidget->item(row, 0)->text().toStdString(),
		ui->tableWidget->item(row, 1)->text().toStdString());

	// 校准索引和方向索引
	int caliIdx = ui->tableWidget->item(row, 0)->data(Qt::UserRole + 1).toInt();
	int dirIdx = ui->tableWidget->item(row, 0)->data(Qt::UserRole + 2).toInt();

	// 获取方向数据
	Direction d = (*m_spData).caliParams[caliIdx].directions[dirIdx];

	// 无校准参数无需进行写入
	if (d.flashAddrs.empty())
	{
		ULOG(LOG_INFO, u8"无校准参数, 无需进行写入.");
		return;
	}

	// 指令列表
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto writeFlashCmd = cmds[ECT_WRITE_FLASH];

	if (writeFlashCmd.scsd.data.cmdData.params.size() != 2)
	{
		ULOG(LOG_ERROR, u8"写入flash指令参数不为2.");
		return;
	}

	// 校准地址数量
	int flashAddrSize = d.flashAddrs.size();
	std::vector<int> vecPos;

	// 校准地址=5个 试管种类识别位、提供样本吸样位
	if (flashAddrSize == 5)
	{
		vecPos = { currPos + 1051 * 2, currPos + 1051, currPos, currPos - 1051, currPos - 1051 * 2 };
	}
	else
	{
		// 其他情况写入值一样
		for (int i = 0; i < flashAddrSize; i++)
		{
			vecPos.push_back(currPos);
		}
	}

	// 遍历校准参数进行写入
	int i = 0;
	for (const auto &flash : d.flashAddrs)
	{
		writeFlashCmd.scsd.data.cmdData.params[0].paramVal.__set_i32Val(flash);
		writeFlashCmd.scsd.data.cmdData.params[1].paramVal.__set_i32Val(vecPos[i]);

		auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, d.id, writeFlashCmd.scsd);

		// 打印发送命令日志
		ULOG(LOG_INFO, u8"写入flash指令: m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
			m_devSN, d.id, ToString(writeFlashCmd.scsd), ToString(ret));

		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_WARN, u8"写入flash失败.");
		}

		i++;
	}

	// 保存配置文件
	auto cmd = cmds[ECT_WRITE_CONFIG];
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, 0, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
		m_devSN, 0, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		return;
	}

	// 保存成功刷新界面
	ui->tableWidget->item(row, 2)->setText(QString::number(currPos));
}

void ChStepModuleWgt::OnSaveInjectorClicked(const int row, const double val)
{
	if (ui->tableWidget->rowCount() < row + 1)
	{
		ULOG(LOG_ERROR, u8"表格现有行数小于%d.", row + 1);
		return;
	}

	ULOG(LOG_INFO, u8"保存校准参数: %s", ui->tableWidget->item(row, 0)->text().toStdString());

	// 校准索引和方向索引
	int caliIdx = ui->tableWidget->item(row, 0)->data(Qt::UserRole + 1).toInt();
	int dirIdx = ui->tableWidget->item(row, 0)->data(Qt::UserRole + 2).toInt();

	// 获取方向数据
	Direction d = (*m_spData).caliParams[caliIdx].directions[dirIdx];

	// 无校准参数无需进行写入
	if (d.flashAddrs.empty())
	{
		ULOG(LOG_INFO, u8"无校准参数, 无需进行写入.");
		return;
	}

	// 指令列表
	auto cmds = ChNodeDebugConfigManager::GetInstance()->GetCmdMaps();
	auto writeFlashCmd = cmds[ECT_WRITE_FLASH];

	if (writeFlashCmd.scsd.data.cmdData.params.size() != 2)
	{
		ULOG(LOG_ERROR, u8"写入flash指令参数不为2.");
		return;
	}

	// 遍历校准参数进行写入
	for (const auto &flash : d.flashAddrs)
	{
		writeFlashCmd.scsd.data.cmdData.params[0].paramVal.__set_i32Val(flash);
		writeFlashCmd.scsd.data.cmdData.params[1].__set_paramType(::tf::EmCanParamType::DOUBLE64);
		tf::UnCanParamValue paramVal;
		paramVal.__set_d64Val(val);
		writeFlashCmd.scsd.data.cmdData.params[1].__set_paramVal(paramVal);
	
		auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, d.id, writeFlashCmd.scsd);

		// 打印发送命令日志
		ULOG(LOG_INFO, u8"写入flash指令: m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
			m_devSN, d.id, ToString(writeFlashCmd.scsd), ToString(ret));

		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_WARN, u8"写入flash失败.");
		}
	}

	// 保存配置文件
	auto cmd = cmds[ECT_WRITE_CONFIG];
	auto ret = ch::c1005::LogicControlProxy::SendCanCommand(m_devSN, 0, cmd.scsd);

	// 打印发送命令日志
	ULOG(LOG_INFO, u8"SendCanCommand(m_devSN=%s, nodeId=%d, sendData=%s, ret=%s",
		m_devSN, 0, ToString(cmd.scsd), ToString(ret));

	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, u8"指令[%s]执行失败.", cmd.cmdName);
		return;
	}

	// 保存成功刷新界面
	ui->tableWidget->item(row, 2)->setText(QString::number(val));
}

void ChStepModuleWgt::OnExecuteClicked()
{
	auto selectRows = getSelectRows();
	if (selectRows.empty())
	{
		TipDlg(tr("未选中对应校准参数!")).exec();
		return;
	}

	// 获取当前行
	int currRow = *selectRows.begin();

	// 获取当前选中的校准参数
	int idx = ui->tableWidget->item(currRow, 0)->data(Qt::UserRole + 1).toInt();
	CaliParam cp = (*m_spData).caliParams[idx];

	// 点击提示
	if (!cp.tips.isEmpty())
	{
		// 选择了取消直接返回
		if (TipDlg(cp.tips, TipDlgType::TWO_BUTTON, this).exec() == QDialog::Rejected)
		{
			return;
		}
	}

	// 获取行数
	pair<int, int> pairRows = ui->tableWidget->item(currRow, 0)->data(Qt::UserRole + 3).value<pair<int, int>>();

	// 获取当前选中的校准参数名称
	auto clickExeCmds = cp.clickExeCmds;

	// 替换其中为CALI_PARAM的值
	RepalceParams(pairRows, clickExeCmds);

	ULOG(LOG_INFO, u8"选中'%s',点击执行按钮", ui->tableWidget->item(currRow, 0)->text().toStdString());

	m_parent->SetDataAndRunCmd(ChExeCmdObject::ET_Exe, clickExeCmds, tr("执行中，请稍等..."));

	// 执行完后，查询一遍当前位置
	for (const auto& dw : m_dirWgtList)
	{
		dw->UpdateCurrPos();
	}
}

void ChStepModuleWgt::OnResetClicked()
{
	if (m_spData == nullptr)
	{
		TipDlg(tr("未选中模块!")).exec();
		return;
	}

	ULOG(LOG_INFO, u8"选中'%s',点击复位按钮", m_moduleName.toStdString());

	m_parent->SetDataAndRunCmd(ChExeCmdObject::ET_Reset, m_spData->clickResetCmds, tr("复位中，请稍等..."));

	// 执行完后，查询一遍当前位置
	for (const auto& dw : m_dirWgtList)
	{
		dw->UpdateCurrPos();
	}
	if (m_injectWgt != nullptr)
	{
		m_injectWgt->UpdateCurrPos();
	}
}

void ChStepModuleWgt::OnSelectTableItem(QTableWidgetItem *item)
{
	if (item == nullptr)
	{
		return;
	}

	int currRow = item->row();

	// 获取当前选中行的索引
	int idx = ui->tableWidget->item(currRow, 0)->data(Qt::UserRole + 1).toInt();

	// 未切换校准名称直接返回
	if (m_oldCaliIndex == idx)
	{
		ULOG(LOG_INFO, u8"选中新老校准名称一致，无需继续执行.");
		return;
	}

	// 清除之前的选中行
	ui->tableWidget->clearSelection();

	// 获取当前合并的行 并选中
	pair<int, int> pairRows = ui->tableWidget->item(currRow, 0)->data(Qt::UserRole + 3).value<pair<int, int>>();
	QTableWidgetSelectionRange range(pairRows.first, 0, pairRows.second, 2);
	ui->tableWidget->setRangeSelected(range, true);

	// 获取当前选中的校准参数
	CaliParam cp = (*m_spData).caliParams[idx];

	// 更新老的校准索引
	m_oldCaliIndex = idx;

	// 清除之前的控件
	clearWidgets();

	// 如果执行指令为空，隐藏执行按钮
	if (cp.clickExeCmds.empty())
	{
		ui->btn_execute->hide();

		if (cp.directions.empty())
		{
			ULOG(LOG_ERROR, u8"方向参数为空.");
			return;
		}

		// 添加注射器电机wgt
		InjectorMachWgt * imw = new InjectorMachWgt(this);
		m_injectWgt = imw;

		// 关联保存槽函数
		connect(imw, &InjectorMachWgt::saveClicked, this, &ChStepModuleWgt::OnSaveInjectorClicked);

		// 设置数据
		int row = pairRows.first;
		auto pos = ui->tableWidget->item(row, 2)->text();
		imw->SetData(cp.name, pos, m_devSN, row, cp.directions[0].id);
		m_layout->addWidget(imw);
	}
	else
	{
		// 遍历方向刷新右边数据
		int row = pairRows.first;
		for (auto &d : cp.directions)
		{
			// 如果方向名称=“/”不显示方向信息,只能复位和执行
			if (d.name == "/")
			{
				continue;
			}

			auto pos = ui->tableWidget->item(row, 2)->text();

			// 添加方向wgt
			DirectionWgt * dw = new DirectionWgt(this);

			m_dirWgtList.push_back(dw);

			// 关联保存槽函数
			connect(dw, &DirectionWgt::saveClicked, this, &ChStepModuleWgt::OnSaveDirClicked);

			// 设置数据
			dw->SetData(d.name, pos, m_devSN, row, d.id);
			m_layout->addWidget(dw);
			row++;
		}

		ui->btn_execute->show();
	}

	// 添加一个弹簧
	auto verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layout->addSpacerItem(verticalSpacer);
}