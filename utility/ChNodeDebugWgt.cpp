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
/// @file     ChNodeDebugWgt.cpp
/// @brief    生化节点调试界面
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
#include "ChNodeDebugWgt.h"
#include "ui_ChNodeDebugWgt.h" 
#include "ChStepModuleWgt.h"
#include "ChPumpModuleWgt.h"
#include "ChValveModuleWgt.h"
#include "ChSwitchModuleWgt.h"
#include "ChUltraMixModuleWgt.h"
#include "ChNodeDebugConfigManager.h"
#include "shared/mcprogressdlg.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"

ChNodeDebugWgt::ChNodeDebugWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChNodeDebugWgt)
{
    ui->setupUi(this);

	InitUi();
	InitData();
	InitConnect();
}

ChNodeDebugWgt::~ChNodeDebugWgt()
{
	m_workThread->quit();
	m_workThread->wait();
	delete m_workThread;
	delete m_chExeCmdObj;
}

void ChNodeDebugWgt::UpdateUi(const string& devSN)
{
	m_devSN = devSN;
}

void ChNodeDebugWgt::SetDataAndRunCmd(ChExeCmdObject::ExeType type, const QVector<CanSendData>& cmds, const QString& tips, const Cmd& eleCmd/* = Cmd()*/)
{
	// 设置数据
	m_chExeCmdObj->SetData(type, m_devSN, cmds, eleCmd);

	// 显示对话框
	m_progressDlg->SetProgressTitle(tips);
	m_progressDlg->show();

	// 启动线程 执行指令
	m_workThread->start();

	// 等待执行完成
	m_loop.exec();
}

void ChNodeDebugWgt::Export(const string& devSN)
{

}

void ChNodeDebugWgt::Import(const string& devSN)
{

}

void ChNodeDebugWgt::InitUi()
{
	// 选中一行，禁止编辑
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// 添加标题
	ui->tableWidget->setColumnCount(1);
	ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("节点调试"));
	ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

void ChNodeDebugWgt::InitData()
{
	// 加载配置文件
	LoadConfigFile();

	// 进度条对话框
	m_progressDlg = std::make_shared<McProgressDlg>(this);
	m_progressDlg->SetModule(true);
	m_progressDlg->close();

	// 创建后台线程
	m_workThread = new QThread;
	m_chExeCmdObj = new ChExeCmdObject;
	m_chExeCmdObj->moveToThread(m_workThread);

	m_stepModuleWgt = new ChStepModuleWgt(this);
	m_valveModuleWgt = new ChValveModuleWgt(this);
	m_pumpModuleWgt = new ChPumpModuleWgt(this);
	m_switchModuleWgt = new ChSwitchModuleWgt(this);
	m_ultraMixMoudleWgt = new ChUltraMixModuleWgt(this);

	// 插入一页空白页
	ui->stackedWidget_center->insertWidget(EMT_BLANK, new QWidget(this));

	ui->stackedWidget_center->insertWidget(EMT_STEP, m_stepModuleWgt);
	ui->stackedWidget_center->insertWidget(EMT_VALVE, m_valveModuleWgt);
	ui->stackedWidget_center->insertWidget(EMT_PUMP, m_pumpModuleWgt);
	ui->stackedWidget_center->insertWidget(EMT_SWITCH, m_switchModuleWgt);
	ui->stackedWidget_center->insertWidget(EMT_ULTRA_MIX, m_ultraMixMoudleWgt);

	// 默认显示空白页
	ui->stackedWidget_center->setCurrentIndex(EMT_BLANK);

	// 添加tableWidget内容
	auto nodeInfo = ChNodeDebugConfigManager::GetInstance()->GetChNodeDebugMaps();
	for (auto it = nodeInfo.begin(); it != nodeInfo.end(); ++it)
	{
		int rowCount = ui->tableWidget->rowCount();
		ui->tableWidget->insertRow(rowCount);

		auto *item = new QTableWidgetItem(it.value().moduleName);
		item->setData(Qt::UserRole + 1, it.key());
		item->setTextAlignment(Qt::AlignCenter);
		ui->tableWidget->setItem(rowCount, 0, item);
	}

	// 查询所有生化设备
	m_spMapChNodeDebugInfos = make_shared<QMap<string, QMap<int, ChNodeDebug>>>();
	auto chDevices = CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::type::DEVICE_TYPE_C1000 });
	for (const auto& dev : chDevices)
	{
		m_spMapChNodeDebugInfos->insert(dev->deviceSN, nodeInfo);
	}
	
	qRegisterMetaType<::tf::StCanResult>("::tf::StCanResult");
}

void ChNodeDebugWgt::InitConnect()
{
	connect(ui->tableWidget, &QTableWidget::itemClicked, this, &ChNodeDebugWgt::OnSelectTableItem);

	// 连接线程信号和槽
	connect(m_workThread, &QThread::started, m_chExeCmdObj, &ChExeCmdObject::ExeCmd);
	connect(m_chExeCmdObj, &ChExeCmdObject::finished, this, &ChNodeDebugWgt::OnThreadFinished);
}

void ChNodeDebugWgt::LoadConfigFile()
{
	// 判断语言
	string strLan = DictionaryQueryManager::GetCurrentLanuageType();
	string jsonFile = GetCurrentDir() + "/ui_cfg/engineerDebugCfg/" + strLan + "/c1005NodeDebug.json";
	if (!ChNodeDebugConfigManager::GetInstance()->LoadConfig(jsonFile))
	{
		ULOG(LOG_ERROR, "LoadConfig failed.");
	}
}

void ChNodeDebugWgt::OutModuleExeCmds(const int moduleId)
{
	auto module = (*m_spMapChNodeDebugInfos)[m_devSN][moduleId];

	ULOG(LOG_INFO, u8"切出模块[%s]执行指令", module.moduleName.toStdString());

	SetDataAndRunCmd(ChExeCmdObject::ET_OutModule, module.spStepModuleInfo->outModuleCmds,
		tr("切出模块[%1]执行相应复位，请稍等...").arg(module.moduleName));
}

void ChNodeDebugWgt::EnterModuleExeCmds(const int moduleId)
{
	auto module = (*m_spMapChNodeDebugInfos)[m_devSN][moduleId];

	ULOG(LOG_INFO, u8"进入模块[%s]执行指令", module.moduleName.toStdString());

	SetDataAndRunCmd(ChExeCmdObject::ET_EnterModule, module.spStepModuleInfo->enterModuleCmds,
		tr("点击模块[%1]执行相应复位，请稍等...").arg(module.moduleName));
}

void ChNodeDebugWgt::OnThreadFinished(const int type, ::tf::StCanResult result)
{
	m_workThread->quit();
	m_workThread->wait();

	m_loop.quit();

	// 隐藏对话框
	m_progressDlg->close();

	if (type == ChExeCmdObject::ET_EleCali)
	{
		m_ultraMixMoudleWgt->UpdateEleCaliVals(result.canResp);
	}
}

void ChNodeDebugWgt::OnSelectTableItem(QTableWidgetItem *item)
{
	if (item == nullptr)
	{
		return;
	}

	// 模块id
	int moduleId = item->data(Qt::UserRole + 1).toInt();
	if (m_devSN.empty() || moduleId <= 0)
	{
		return;
	}

	// 获取当前选中模块
	auto &currModule = (*m_spMapChNodeDebugInfos)[m_devSN][moduleId];

	// 新的模块
	auto currModuleIdType = pair<int, int>(moduleId, (int)currModule.moduleType);

	// 新老模块一样无需处理
	if (currModuleIdType == m_oldModuleIdType)
	{
		ULOG(LOG_INFO, u8"选中新老模块一致，无需继续执行.");
		return;
	}

	// 老的是步进节点，执行切出模块指令
	if (m_oldModuleIdType.second == EMT_STEP && m_oldModuleIdType.first != -1)
	{
		OutModuleExeCmds(m_oldModuleIdType.first);
	}

	// 新的是步进节点，执行点击模块执行
	if (currModuleIdType.second == EMT_STEP && currModuleIdType.first != -1)
	{
		EnterModuleExeCmds(currModuleIdType.first);
	}

	m_oldModuleIdType = currModuleIdType;

	// 模块类型
	switch (currModule.moduleType)
	{
	case EMT_STEP:
		m_stepModuleWgt->UpdateUi(m_devSN, currModule.moduleName, currModule.spStepModuleInfo);
		break;
	case EMT_PUMP:
		m_pumpModuleWgt->UpdateUi(m_devSN, currModule.spVecCPVSModuleInfos);
		break;
	case EMT_VALVE:
		m_valveModuleWgt->UpdateUi(m_devSN, currModule.spVecCPVSModuleInfos);
		break;
	case EMT_SWITCH:
		m_switchModuleWgt->UpdateUi(m_devSN, currModule.spVecCPVSModuleInfos);
		break;
	case EMT_ULTRA_MIX:
		m_ultraMixMoudleWgt->UpdateUi(m_devSN, currModule.spUlMixModuleInfos);
		break;
	default:
		break;
	}

	// 根据模块类型切换页面
	ui->stackedWidget_center->setCurrentIndex(currModule.moduleType);
}
