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
/// @file     ReagentChemistryWidgetPlate.cpp
/// @brief    生化试剂盘源文件
///
/// @author   5774/WuHongTao
/// @date     2021年8月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年8月31日，新建文件
/// @li 7997/XuXiaoLong，2023年9月20日，进行维护
///
///////////////////////////////////////////////////////////////////////////

#include "ReagentChemistryWidgetPlate.h"
#include "ui_ReagentChemistryWidgetPlate.h"
#include "QRegisterSupplyInfo.h"
#include "QReagentScanDialog.h"
#include "datetimefmttool.h"
#include "ChReagentUnLoadDlg.h"

#include <QLabel>
#include <QDateTime>
#include <QRadioButton>
#include <QStandardItemModel>
#include <QMouseEvent>

#include "shared/ReagentCommon.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "shared/QProgressBarShell.h"
#include "manager/DictionaryQueryManager.h"

#include "thrift/ch/c1005/C1005LogicControlProxy.h"

#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ch/ChConfigDefine.h"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_constants.h"

#define MINOFSUPPLY(suppliesInfo) std::min_element(suppliesInfo.cavityInfos.begin(), suppliesInfo.cavityInfos.end(),\
                                                   [](ch::tf::CavityInfo p1, ch::tf::CavityInfo p2) {return p1.residual < p2.residual; })->residual;
#define COMBINSTRING(first,spliter,second) {((first).isEmpty())?(second):((first)+(spliter)+(second))} //合成字符串

#define UL01_TO_ML		10000.0			//	0.1ul转换为ml的倍率	
#define NORMAL_COLOR	"#333"			//  正常颜色
#define NOTICE_COLOR	"#FA9016"		//  提示颜色
#define WARNING_COLOR	"#F33"			//  警告颜色
#define MAXLOADER		5				//  在线试剂加载位置最大值

// 剩余测试数、状态Label对应文字及其颜色
struct TextAndColor
{
	QString remainCountText;	// 剩余测试数文字
	QString remainCountColor;	// 剩余测试数颜色
	vector<pair<QString, QString>> statusTextColors;// 状态文字、颜色
};

ReagentChemistryWidgetPlate::ReagentChemistryWidgetPlate(QWidget *parent /*= Q_NULLPTR*/)
	: QWidget(parent),
	ui(new Ui::ReagentChemistryWidgetPlate),
	m_registerDialog(nullptr),
    m_needUpdate(false),
	m_currentPostion(-1),
	m_focusState(false),
	m_scanDialog(nullptr),
    m_pProgressBar(Q_NULLPTR),
    m_currentDevModule(nullptr),
	m_currSelectInfo(nullptr),
	m_unloadDlg(nullptr)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->setupUi(this);
    // 显示前初始化
    InitBeforeShow();
}

ReagentChemistryWidgetPlate::~ReagentChemistryWidgetPlate()
{

}

void ReagentChemistryWidgetPlate::UpdateGroupDevice(std::vector<std::shared_ptr<const tf::DeviceInfo>>& deviceGroups)
{
	// 首先清除之前的元素
	ui->subDeviceGroup->hide();
	m_subDeviceVec.clear();
	auto layout = ui->subDeviceGroup->layout();
	while (layout->count() > 0)
	{
		auto item = layout->takeAt(0);
		auto widgetRemove = item->widget();
		delete widgetRemove;
		delete item;
	}

	if (deviceGroups.empty())
	{
		return;
	}

	for (auto& device : deviceGroups)
	{
		auto deviceName = device->groupName + device->name;
		QRadioButton *pButton = new QRadioButton(QString::fromStdString(deviceName));
		ui->subDeviceGroup->layout()->addWidget(pButton);
		// 保存设备信息 不能用map存，因为QRadioButton *作为key不保序，每次取第一个有问题
		m_subDeviceVec.push_back({ pButton , device });
		connect(pButton, &QRadioButton::clicked, this, [&]()
		{
			QRadioButton* pButton = qobject_cast<QRadioButton*>(sender());
			if (pButton == nullptr)
			{
				return;
			}

			auto iter = find_if(m_subDeviceVec.begin(), m_subDeviceVec.end(), [=](const auto &data)
			{
				return data.first == pButton;
			});

			if (iter == m_subDeviceVec.end())
			{
				return;
			}

			// 获取当前按钮对应的设备信息
			auto selectDevice = iter->second;
			// 更新试剂盘
			UpdateReagentPlate(selectDevice);
			// 更新耗材
			UpdateCabinetSupplies(selectDevice);
			// 更新加载信息
			UpdateReagentLoaderStatus(selectDevice);
			// 更新废液桶
			OnShowBucket();
		});
	}

	if (!m_subDeviceVec.empty())
	{
		// 默认选择第一个
		emit m_subDeviceVec.begin()->first->clicked();
		m_subDeviceVec.begin()->first->setChecked(true);
	}
	else// 不是2000速时避免重复更新
	{
		// 更新试剂盘
		UpdateReagentPlate(deviceGroups[0]);
		// 更新耗材
		UpdateCabinetSupplies(deviceGroups[0]);
		// 更新加载信息
		UpdateReagentLoaderStatus(deviceGroups[0]);
		// 更新废液桶
		OnShowBucket();
	}

	// 有两个设备才显示
	if (m_subDeviceVec.size() > 1)
	{
		ui->subDeviceGroup->show();
	}
}

void ReagentChemistryWidgetPlate::showEvent(QShowEvent *event)
{
    if (m_needUpdate)
    {
		UpdateReagentPlate(m_currentDevModule);
		UpdateCabinetSupplies(m_currentDevModule);
		UpdateReagentLoaderStatus(m_currentDevModule);
        OnShowBucket();
        m_needUpdate = false;
    }
}

void ReagentChemistryWidgetPlate::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 试剂盘对象实例化
    m_pReagentPlate = new QReagentView(ui->ReagentWidget);
    // 试剂盘显示
    m_pReagentPlate->show();
    // 槽位点击响应
    connect(m_pReagentPlate, SIGNAL(indexChanged(int)), this, SLOT(OnSelectedReagentSlotChanged(int)));
    // 信息录入
    connect(ui->reagentRegister, SIGNAL(clicked()), this, SLOT(onRegisterClicked()));
    // 试剂屏蔽
    connect(ui->reagentshield, SIGNAL(clicked()), this, SLOT(OnReagentShield()));
    // 优先使用
    connect(ui->reagentPriority, SIGNAL(clicked()), this, SLOT(OnPriority()));
    // 试剂卸载
    connect(ui->reagentUnload, SIGNAL(clicked()), this, SLOT(OnReagentUnload()));
    // 试剂扫描
    connect(ui->reagentScan, SIGNAL(clicked()), this, SLOT(OnScanReagent()));
    // 监听项目编码管理器信息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnRefreshReagentPlate);
    // 设备otherinfo更新
    REGISTER_HANDLER(MSG_ID_DEVICE_OTHER_INFO_UPDATE, this, OnDeviceOtherInfoUpdate);
    // 试剂&耗材更新消息
    REGISTER_HANDLER(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, this, OnUpdateSupply);
	// 设备状态的更新
	REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDeviceStatusUpdate);
    // 试剂加载器的更新
    REGISTER_HANDLER(MSG_ID_MANAGER_REAGENT_LOADER_UPDATE, this, OnLoaderStatusUpdate);
    // 废液桶报警值更新
    REGISTER_HANDLER(MSG_ID_BUCKET_SET_UPDATE, this, OnShowBucket);
    // 废液桶状态、余量更新
    REGISTER_HANDLER(MSG_ID_CH_WASTE_CNT_UPDATE, this, OnUpdateBucketStatus);
	// 试剂报警值更新
	REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_REAGENT, this, OnRefreshReagentPlate);
	// 耗材报警值更新
	REGISTER_HANDLER(MSG_ID_MANAGER_UPDATE_SUPPLY, this, OnRefreshCabinetSupply);
	// 未分配项目列表更新
	REGISTER_HANDLER(MSG_ID_MANAGER_UNALLOCATE_ASSAYS_UPDATE, this, OnRefreshReagentPlate);

    // 初始化功能按钮状态
    DisableFunctionBtn();
	ui->reagentScan->setDisabled(true);

    m_ReagentLoader.push_back(ui->loader_status_1);
    m_ReagentLoader.push_back(ui->loader_status_2);
    m_ReagentLoader.push_back(ui->loader_status_3);
    m_ReagentLoader.push_back(ui->loader_status_4);
    m_ReagentLoader.push_back(ui->loader_status_5);
    // 加载器初始化
    for (auto loader : m_ReagentLoader)
    {
		SetCtrlPropertyAndUpdateQss(loader, "states", "empty");
    }

    m_ReagentTips.push_back(ui->label_tips_1);
    m_ReagentTips.push_back(ui->label_tips_2);
    m_ReagentTips.push_back(ui->label_tips_3);
    m_ReagentTips.push_back(ui->label_tips_4);
    m_ReagentTips.push_back(ui->label_tips_5);
    for (auto tips : m_ReagentTips)
    {
        tips->setText("");
    }

	// 初始化反应杯清洗液
	m_cupWashLiquidMap[ch::c1005::tf::CabinetPos::POS_CUP_DETERGENT_ACIDITY1] = WashLiquidWgt(ui->supplyStatus1, ui->status_show1, ui->label_Aci1);
	m_cupWashLiquidMap[ch::c1005::tf::CabinetPos::POS_CUP_DETERGENT_ALKALINITY1] = WashLiquidWgt(ui->supplyStatus2, ui->status_show2, ui->label_Alka1);
	m_cupWashLiquidMap[ch::c1005::tf::CabinetPos::POS_CUP_DETERGENT_ACIDITY2] = WashLiquidWgt(ui->supplyStatus3, ui->status_show3, ui->label_Aci2);
	m_cupWashLiquidMap[ch::c1005::tf::CabinetPos::POS_CUP_DETERGENT_ALKALINITY2] = WashLiquidWgt(ui->supplyStatus4, ui->status_show4, ui->label_Alka2);

	// 切换选中耗材时候的动作
    for (const auto& washLiquid : m_cupWashLiquidMap)
    {
        connect(washLiquid.second.progressBar, SIGNAL(clicked()), this, SLOT(OnSelectSupply()));
		washLiquid.second.progressBar->setCheckable(true);
    }

	// 隐藏样本针 2023.12.13 xuxiaolong
	ui->frame_acidic->hide();
	ui->frame_alka->hide();

	ui->label_Aci1->setText(QString(CH_CUP_DETERGENT_ACIDITY_NAME) + "-1");
	ui->label_Aci2->setText(QString(CH_CUP_DETERGENT_ACIDITY_NAME) + "-2");
	ui->label_Alka1->setText(QString(CH_CUP_DETERGENT_ALKALINITY_NAME) + "-1");
	ui->label_Alka2->setText(QString(CH_CUP_DETERGENT_ALKALINITY_NAME) + "-2");
}

void ReagentChemistryWidgetPlate::UpdateReagentPlate(std::shared_ptr<const tf::DeviceInfo>& spModule, const std::set<int> &setPos/* = {}*/)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 首先判断是否获取模块信息
	if (spModule == nullptr)
	{
		return;
	}
	m_currentDevModule = spModule;

	auto chReagents = CIM_INSTANCE->GetChDiskReagentSupplies(m_currentDevModule->deviceSN, setPos);
	// 全盘更新
	if (setPos.empty())
	{
		// 卸载所有试剂
		for (int i = 1; i <= m_pReagentPlate->TotalCount(); i++)
		{
			m_pReagentPlate->UnloadReagent(i);
		}

		// 根据耗材信息绘制试剂盘
		for (const auto& cr : chReagents)
		{
			QReagentItem::ReagentInfo stuReagentInfo;
			SetReagentStatus(cr.second.supplyInfo, cr.second.reagentInfos, stuReagentInfo);
			m_pReagentPlate->LoadReagent(cr.first, stuReagentInfo);
		}
	}
	// 更新指定位置
	else
	{
		for (const auto& pos : setPos)
		{
			// 找不到表示已卸载
			if (!chReagents.count(pos))
			{
				m_pReagentPlate->UnloadReagent(pos);
			}
			else
			{
				QReagentItem::ReagentInfo stuReagentInfo;
				SetReagentStatus(chReagents[pos].supplyInfo, chReagents[pos].reagentInfos, stuReagentInfo);
				m_pReagentPlate->LoadReagent(pos, stuReagentInfo);
			}
		}
	}

	// 试剂盘名称
	m_pReagentPlate->SetPlateContent(QString::fromStdString(m_currentDevModule->groupName + m_currentDevModule->name));

	// 更新当前选中
	UpdateSelSlotInfo(m_currentPostion);
}

void ReagentChemistryWidgetPlate::UpdateCabinetSupplies(std::shared_ptr<const tf::DeviceInfo>& spModule, const std::set<int> &setPos/* = {}*/)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 首先判断是否获取模块信息
	if (spModule == nullptr)
	{
		return;
	}

	m_currentDevModule = spModule;

	// 解析otherinfo
	ChDeviceOtherInfo cdoi;
	if (!DecodeJson(cdoi, m_currentDevModule->otherInfo))
	{
		ULOG(LOG_WARN, "DecodeJson ChDeviceOtherInfo Failed");
	}

	// 设置当前选中耗材
	if (m_focusState && m_pProgressBar)
	{
		m_pProgressBar->setChecked(true);
	}

	// 判断反应杯酸碱清洗液是否存在
	auto funcJudgeExist = [cdoi](const int pos)->bool {

		// 非反应杯酸碱清洗液
		if (pos < ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY1 ||
			pos > ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY2)
		{
			return false;
		}

		// 未启用反应杯酸性清洗液
		if ((pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY1
			|| pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY2)
			&& !cdoi.acidity)
		{
			return false;
		}
		// 未启用反应杯碱性清洗液
		else if ((pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY1
			|| pos == ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY2)
			&& !cdoi.alkalinity)
		{
			return false;
		}

		return true;
	};

	// 更新位置
	std::set<int> updatePos = setPos;

	// 更新所有位置
	if (setPos.empty())
	{
		updatePos.insert({ ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY1, ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY1,
			ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY2, ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY2 });
	}

	// 获取生化仓外耗材
	auto chSupplies = CIM_INSTANCE->GetChCabinetSupplies(spModule->deviceSN, setPos);
	for (const auto& pos : updatePos)
	{
		WashLiquidWgt w = m_cupWashLiquidMap[pos];

		// 未启用
		if (!funcJudgeExist(pos))
		{
			w.progressBar->hide();
			w.progressBar->setChecked(false);
			continue;
		}

		w.progressBar->show();
		// 找不到则隐藏
		if (!chSupplies.count(pos))
		{
			w.progressBar->setChecked(false);
			w.progressBar->SetSupplyStatus(QProgressBarShell::SupplyStatus());
			w.tips->show();
			w.name->hide();
		}
		else
		{
			w.name->show();
			w.tips->hide();
			UpdateCabinetSupply(chSupplies[pos]);
		}
	}
}

void ReagentChemistryWidgetPlate::UpdateCabinetSupply(const ch::tf::SuppliesInfo& si)
{
	auto progressBar = m_cupWashLiquidMap[si.pos].progressBar;
	if (progressBar == nullptr)
	{
		return;
	}

	// 绘制状态
	QProgressBarShell::SupplyStatus statusData;

	// 根据位置确定不同的耗材类型
	switch (ch::c1005::tf::CabinetPos::type(si.pos))
	{
		// 酸1、酸2
	case ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY1:
	case ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ACIDITY2:
		statusData.supplyType = QProgressBarShell::ACIDITY;
		break;
		// 碱1、碱2
	case ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY1:
	case ch::c1005::tf::CabinetPos::type::POS_CUP_DETERGENT_ALKALINITY2:
		statusData.supplyType = QProgressBarShell::ALKALINITY;
		break;
	default:
		break;
	}

	// 扫描失败
	if (si.suppliesCode <= 0)
	{
		statusData.status = QProgressBarShell::SCANFAILED;
	}
	// 失效日期、开瓶有效期过期
	else if (ChSuppliesExpire(si.expiryTime) || GetOpenRestdays(si.openBottleExpiryTime) <= 0)
	{
		statusData.status = QProgressBarShell::EXPIRED;
	}
	// 当前
	else if (si.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_CURRENT)
	{
		statusData.status = QProgressBarShell::CURRENT;
	}
	// 备用
	else
	{
		statusData.status = QProgressBarShell::BACKUP;
	}

	// 填写耗材详细信息
	statusData.supplyInfo = si;

	// 计算百分比
	if (!si.cavityInfos.empty())
	{
		// 余量
		auto residual = GetSupplyResidual(si);
		statusData.blance = residual;

		// 总量
		auto total = GetSupplyMax(si);

		// 百分比
		auto progress = (total <= 0) ? 0 : 100 * (double(residual) / total);
		statusData.progress = progress;
	}

	progressBar->SetSupplyStatus(statusData);
}

void ReagentChemistryWidgetPlate::UpdateReagentLoaderStatus(std::shared_ptr<const tf::DeviceInfo>& spModule)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (!spModule)
	{
		return;
	}

	// 设置扫描按钮状态
	OnDeviceStatusUpdate(*spModule);

	// 从数据中心获取加载器信息
	auto reagentLoader = CIM_INSTANCE->getReagentLoaderByDevSn(QString::fromStdString(spModule->deviceSN));

	// 清空加载位状态
	auto funcClear = [=]() {
		for (auto &loader : m_ReagentLoader)
		{
			SetCtrlPropertyAndUpdateQss(loader, "states", "empty");
		}

		for (auto &loaderTip : m_ReagentTips)
		{
			loaderTip->clear();
		}
	};

	// 生化设备增加未屏蔽自动装卸载试剂判断
	ChDeviceOtherInfo chdoi;
	if (!DecodeJson(chdoi, spModule->otherInfo))
	{
		ULOG(LOG_ERROR, "DecodeJson ChDeviceOtherInfo Failed");
		return;
	}
	// 如果加载器屏蔽了，显示屏蔽
	if (!chdoi.loadReagentOnline.enable)
	{
		ui->loadable_status->setText(tr("屏蔽"));
		SetCtrlPropertyAndUpdateQss(ui->loadable_status, "colors", "blue_gray");
		funcClear();
		return;
	}
	else
	{
		QString txetColor = "red";
		// 托盘的状态
		switch (reagentLoader.loaderStatus)
		{
		case ch::tf::LoaderStatus::LOADER_STATUS_FAULT:
		case ch::tf::LoaderStatus::LOADER_STATUS_UNKNOWN:
			ui->loadable_status->setText(tr("故障"));
			break;
		case ch::tf::LoaderStatus::LOADER_STATUS_NORMAL:
			ui->loadable_status->setText(tr("加载器已推出，可进行试剂更换！"));
			txetColor = "light_blue";
			break;
		case ch::tf::LoaderStatus::LOADER_STATUS_RUNNING:
			ui->loadable_status->setText(tr("正在运行，请勿开盖！"));
			break;
		default:
			break;
		}

		// 文字颜色
		SetCtrlPropertyAndUpdateQss(ui->loadable_status, "colors", txetColor);
	}

	// 如果试剂加载位为空并且托盘状态为未知表示查询失败，试剂加载位设为空位
	if (reagentLoader.loaderStatus == ch::tf::LoaderStatus::LOADER_STATUS_UNKNOWN &&
		reagentLoader.loaderItems.size() == 0)
	{
		funcClear();
		return;
	}

	// 依次对加载位进行处理
	int loadNumber = 0;
	for (auto loadInfo : reagentLoader.loaderItems)
	{
		// 判断位置合法性
		if (loadInfo.pos > MAXLOADER || loadInfo.pos < 1)
		{
			continue;
		}

		// 是试剂还是耗材
		bool isReagent = (!loadInfo.reagentGroups.empty() || (loadInfo.suppliesInfo.suppliesCode > 0
			&& loadInfo.suppliesInfo.suppliesCode <= ::ch::tf::g_ch_constants.MAX_REAGENT_CODE));

		// 加载器状态
		QString states = GetLoaderStatus(loadInfo.itemStatus, isReagent);
		auto loaderItem = m_ReagentLoader[loadInfo.pos - 1];
		SetCtrlPropertyAndUpdateQss(loaderItem, "states", states);

		// 名称和余量Label
		auto loadText = m_ReagentTips[loadInfo.pos - 1];

		// 是否需要清除详细信息(目前只有扫描成功、正在加载、卸载成功显示名称余量等信息)
		bool clearDetailFlag = (loadInfo.itemStatus != ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_WAIT_GRAB)
			&& (loadInfo.itemStatus != ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_GRABING)
			&& (loadInfo.itemStatus != ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_UNLOAD);

		// 不需要显示名称和余量
		if (clearDetailFlag)
		{
			loadText->clear();
		}
		else
		{
			// 加载器底部显示名称和余量
			auto pair = GetLoaderNameRemain(loadInfo);
			loadText->setAroundText(pair.first + "\n" + pair.second);
		}
	}
}

void ReagentChemistryWidgetPlate::UpdateSelSlotInfo(int iIndex)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (iIndex == -1)
	{
		return;
	}

	// 对应的模块
	if (m_currentDevModule == nullptr)
	{
		ULOG(LOG_WARN, "have not select the module.");
		return;
	}

	OnFocusOut();
	ClearReagentDiskDatas();

	// 保存当前位置
	m_currentPostion = iIndex;
	// 设置当前选中
	m_pReagentPlate->SetIndex(iIndex);

	// 查询当前位置试剂信息
	auto chReagents = CIM_INSTANCE->GetChDiskReagentSupplies(m_currentDevModule->deviceSN, { iIndex });

	// 如果查询结果为空，说明是未放置，则返回
	if (chReagents.empty())
	{
		m_currSelectInfo = nullptr;
		// 未放置允许信息录入
		if (m_currentDevModule != nullptr)
		{
			ui->reagentRegister->setEnabled(true);
		}

		return;
	}

	// 获取当前选中的耗材
	m_currSelectInfo = std::make_shared<ChReagentInfo>(chReagents[iIndex]);

	// 扫描失败允许信息录入 但是不显示详细信息
	if (m_currSelectInfo->supplyInfo.suppliesCode <= 0)
	{
		if (m_currentDevModule != nullptr)
		{
			ui->reagentRegister->setEnabled(true);
		}
		ui->reagentName->setText(QObject::tr("扫描失败"));
		ui->reagentName->setStyleSheet("color:#F33;");
		return;
	}

	// 如果不是试剂则按照其他方式显示(酸碱清洗和纯水等等)
	if (m_currSelectInfo->supplyInfo.type != ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
	{
		// 耗材
		m_IsReagent = false;
		ShowSuppliesDetail(m_currSelectInfo->supplyInfo);
		// 耗材的时候，按照测试要求
		ui->reagentshield->setText(tr("试剂屏蔽"));
		return;
	}

	// 试剂类型
	m_IsReagent = true;
	// 位置信息
	ui->reagentPostion->setText(GetItemFromPostionAndDevice(m_currSelectInfo->supplyInfo.deviceSN, m_currSelectInfo->supplyInfo.pos)->text());
	// 名称
	ui->reagentName->setText(m_currSelectInfo->supplyInfo.name.c_str());
	// 批号
	ui->reagentLot->clear();
	auto lotSnMode = m_currSelectInfo->supplyInfo.lotSnMode;
	if (lotSnMode == tf::EnumLotSnMode::ELSM_LOT || lotSnMode == tf::EnumLotSnMode::ELSM_LOT_SN)
	{
		if (m_currSelectInfo->supplyInfo.__isset.lot && !m_currSelectInfo->supplyInfo.lot.empty())
		{
			// 批号
			ui->reagentLot->setText(m_currSelectInfo->supplyInfo.lot.c_str());
		}
	}

	// 当前选中试剂的同组试剂
	auto sameReagentGroups = CIM_INSTANCE->GetChGroupReagents(m_currentDevModule->deviceSN, m_currSelectInfo->supplyInfo.suppliesCode);

	// 显示备用编号，设置同组
	for (auto reagentGroup : sameReagentGroups)
	{
		// 设置当前的备用编号
		if (ShowPostionSpareNumber(reagentGroup.posInfo.pos, true))
		{
			m_backUpShowPos.insert(reagentGroup.posInfo.pos);
		}
	}

	// 当前选中的试剂组
	auto currReagentGroup = m_currSelectInfo->reagentInfos;

	// 显示试剂详细信息 试剂组为空只显示名称
	if (!currReagentGroup.empty())
	{
		ShowReagentGroupDetail(currReagentGroup, m_currSelectInfo->supplyInfo);
	}
	else
	{
		ui->reagentLot->clear();
		ui->reagentshield->setText(tr("试剂屏蔽"));
		ui->reagentshield->setEnabled(false);
	}

	// 非正在校准
	bool bNotCaling = true;
	for (const auto& rgt : currReagentGroup)
	{
		if (rgt.caliStatus == tf::CaliStatus::CALI_STATUS_DOING)
		{
			bNotCaling = false;
			break;
		}
	}

	// 非正在校准、开放试剂、扫描失败 允许信息录入
	if (bNotCaling && m_currentDevModule != nullptr
		&& (m_currSelectInfo->supplyInfo.suppliesCode >= ::ch::tf::g_ch_constants.MIN_OPEN_REAGENT_CODE 
			|| m_currSelectInfo->supplyInfo.suppliesCode <= 0))
	{
		ui->reagentRegister->setEnabled(true);
	}
}

QString ReagentChemistryWidgetPlate::GetReagentName(const int assayCode, const int supplyCode, const std::string& supplyName)
{
	QString name = QString::fromStdString(supplyName);

	// 项目不存在时返回耗材名称
	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CIM_INSTANCE->GetAssayInfo(assayCode);
	if (nullptr == spAssayInfo)
	{
		ULOG(LOG_WARN, "Invalid assay code,");
		return name;
	}

	// 项目名称
	name = QString::fromStdString(spAssayInfo->assayName);
	// 开放试剂 + "*"
	if (supplyCode >= ::ch::tf::g_ch_constants.MIN_OPEN_REAGENT_CODE)
	{
		name += "*";
	}

	// 返回项目名称
	return name;
}

///
/// @brief  获取试剂状态
///
/// @param[in]  supplyInfo  耗材信息
/// @param[in]  reagentInfo 试剂信息
/// @param[in]  unAllocated true项目未分配
///
/// @return vector（状态文字，状态颜色）
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年5月16日，新建函数
///
vector<pair<QString, QString>> GetReagentShow2Status(const ch::tf::SuppliesInfo& supplyInfo, 
	const ch::tf::ReagentGroup& reagentInfo, const bool unAllocated)
{
	vector<pair<QString, QString>> vecPair;

	// 项目未分配特殊处理，因为后台未将项目未分配的置为不可用，bug14082需要将未分配的状态置为不可用
	if (!unAllocated)
	{
		// 注册但未放置/已卸载
		if (supplyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_REGISTER ||
			supplyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_UNLOAD)
		{
			vecPair.push_back({ QObject::tr("未放置"),WARNING_COLOR });
		}
		// 试剂屏蔽
		else if (reagentInfo.reagentMask)
		{
			vecPair.push_back({ QObject::tr("试剂屏蔽"),NORMAL_COLOR });
		}
		// 校准屏蔽
		else if (reagentInfo.caliMask)
		{
			vecPair.push_back({ QObject::tr("校准屏蔽"),NORMAL_COLOR });
		}
		// 当前试剂
		else if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_CURRENT)
		{
			vecPair.push_back({ QObject::tr("在用"),NORMAL_COLOR });
		}
		// 备用试剂
		else if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_BACKUP)
		{
			vecPair.push_back({ QString(QObject::tr("备用%1-%2")).arg(reagentInfo.backupTotal).arg(reagentInfo.backupNum), NORMAL_COLOR });
		}
		// 不可用（空）
		else if (reagentInfo.usageStatus == tf::UsageStatus::USAGE_STATUS_CAN_NOT_USE)
		{
			//vecPair.push_back({ QObject::tr("不可用"),WARNING_COLOR });
		}
	}

	// 校准状态
	QString caliTxt = ThriftEnumTrans::GetCalibrateStatus(reagentInfo.caliStatus);
	QString caliColor = NORMAL_COLOR;
	if (reagentInfo.caliStatus == tf::CaliStatus::CALI_STATUS_FAIL
		|| (reagentInfo.caliStatus == tf::CaliStatus::CALI_STATUS_NOT && reagentInfo.caliCurveId < 0))
	{
		caliColor = WARNING_COLOR;
	}

	// 未校准但是存在校准曲线时、或者是SIND 校准状态显示为空
	if ((reagentInfo.caliStatus == tf::CaliStatus::CALI_STATUS_NOT && reagentInfo.caliCurveId > 0)
		|| reagentInfo.suppliesCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		caliTxt.clear();
	}

	// 校准状态不为空
	if (!caliTxt.isEmpty())
	{
		vecPair.push_back({ caliTxt, caliColor });
	}

	// 试剂失控
	if (reagentInfo.qcStatus == tf::QcStat::QC_STAT_UNCONTROLLABLE)
	{
		vecPair.push_back({ QObject::tr("试剂失控"), WARNING_COLOR });
	}

	// 校准曲线过期
	if (reagentInfo.suppliesCode != ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		RowCellInfo rci;
		MakeCaliCurveExpirateDate(reagentInfo, rci);
		QStringList strCalCurveExpLst = rci.m_text.split(":");
		if ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0)
			&& DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
		{
			// 最新需求，校准过期显示黄色
			vecPair.push_back({ QObject::tr("校准过期"), NOTICE_COLOR });
		}
	}

	// 开瓶过期
	if (GetOpenRestdays(reagentInfo.openBottleExpiryTime) <= 0)
	{
		vecPair.push_back({ QObject::tr("开瓶过期"), WARNING_COLOR });
	}

	return vecPair;
}

void ReagentChemistryWidgetPlate::ShowReagentGroupDetail(const std::vector<ch::tf::ReagentGroup>& reagentGroups, ch::tf::SuppliesInfo& supply)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	QString spliter = "/";
	QString reagentName;

	// 失效日期、剩余测试数、状态Label对应文字及其颜色
	QList<TextAndColor> TACDatas;
	for (auto reagentGroup : reagentGroups)
	{
		// 试剂名称
		if (reagentGroup.suppliesCode < 0)
		{
			reagentName = COMBINSTRING(reagentName, spliter, "?");
		}
		else
		{
			auto assayName = GetReagentName(reagentGroup.assayCode, supply.suppliesCode, supply.name);
			reagentName = COMBINSTRING(reagentName, spliter, assayName);
		}

		// 不满足适用机型其他信息不显示
		if (!reagentGroup.modelUsed)
		{
			continue;
		}

		// 备用试剂，处于待机或者停机（ChanDao问题单1577， 修改为任何状态均可设置优先级）
		if (reagentGroup.usageStatus == tf::UsageStatus::USAGE_STATUS_BACKUP)
		{
			ui->reagentPriority->setEnabled(true);
		}

		TextAndColor tac;

		// 过期时间
		QString date = QString::fromStdString(ConverStdStringToDateString(reagentGroup.expiryTime));
		auto dateTimeString = ToCfgFmtDateTime(date);
		QString dateColor = NORMAL_COLOR;
		if (ChSuppliesExpire(reagentGroup.expiryTime))
		{
			dateColor = WARNING_COLOR;
		}
		ui->reagentTimelimite->setText(QString("<span style='color:%1;'>%2</span>").arg(dateColor).arg(dateTimeString));

		// 试剂剩余次数
		tac.remainCountText = "0";
		if (reagentGroup.remainCount > 0)
		{
			tac.remainCountText = QString::number(reagentGroup.remainCount);
		}

		// 获取试剂报警量
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CIM_INSTANCE->GetAssayInfo(reagentGroup.assayCode);
		tac.remainCountColor = NORMAL_COLOR;
		if (spAssayInfo == nullptr)
		{
			continue;
		}
		if (reagentGroup.remainCount <= 0)
		{
			tac.remainCountColor = WARNING_COLOR;
		}
		else if (spAssayInfo->bottleAlarmThreshold > 0 && reagentGroup.remainCount <= spAssayInfo->bottleAlarmThreshold)
		{
			tac.remainCountColor = NOTICE_COLOR;
		}

		// 是否探测失败
		if (reagentGroup.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
		{
			tac.remainCountText = "0";
			tac.remainCountColor = WARNING_COLOR;
		}

		// 是否未分配项目
		auto bUnAllocate = CIM_INSTANCE->IsUnAllocatedAssay(reagentGroup.deviceSN, reagentGroup.assayCode);
		if (bUnAllocate)
		{
			tac.remainCountText = "?";
			tac.remainCountColor = WARNING_COLOR;
		}

		// 使用状态等
		tac.statusTextColors = GetReagentShow2Status(supply, reagentGroup, bUnAllocate);
		TACDatas.push_back(std::move(tac));

		// 试剂屏蔽--允许(耗材状态必须处于Load状态)
		if (ShieldReagentStat(*m_currentDevModule, supply.placeStatus))
		{
			ui->reagentshield->setText(reagentGroup.reagentMask ? tr("解除屏蔽") : tr("试剂屏蔽"));
			ui->reagentshield->setEnabled(true);
		}
	}

	// 试剂名称
	ui->reagentName->setText(reagentName);

	// 可用测试数、状态
	QString reagentLastTimes, reagentStatus;
	for (int i = 0; i < TACDatas.size(); i++)
	{
		if (i > 0)
		{
			reagentLastTimes += "/";
			reagentStatus += "/";
		}

		reagentLastTimes += QString("<font color='%1'>%2</font>")
			.arg(TACDatas[i].remainCountColor).arg(TACDatas[i].remainCountText);

		for (const auto& pair : TACDatas[i].statusTextColors)
		{
			reagentStatus += QString("<font color='%1'>%2</font>")
				.arg(pair.second).arg(pair.first) + "  ";
		}
	}

	ui->reagentLastTimes->setText(reagentLastTimes);
	ui->reagentStatus->setText(reagentStatus);
}

///
/// @brief  
///
/// @param[in]  supplyInfo  获取耗材显示状态
///
/// @return vector（状态文字，状态颜色）
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年5月16日，新建函数
///
vector<pair<QString, QString>> GetSuppliesShow2Status(const ch::tf::SuppliesInfo& supplyInfo)
{
	vector<pair<QString, QString>> vecPair;

	QString statusColor = NORMAL_COLOR;
	auto item = GetSupplyShowStatus(supplyInfo.placeStatus, supplyInfo.usageStatus, supplyInfo.id,
		supplyInfo.backupTotal, supplyInfo.backupNum);

	QString statusText = item->text();
	if (item->data(Qt::TextColorRole).isValid())
	{
		statusColor = WARNING_COLOR;
	}

	if (!item->text().isEmpty())
	{
		vecPair.push_back({ item->text(), statusColor });
	}

	// 开瓶过期
	if (GetOpenRestdays(supplyInfo.openBottleExpiryTime) <= 0)
	{
		vecPair.push_back({ QObject::tr("开瓶过期"), WARNING_COLOR });
	}

	return vecPair;
}

void ReagentChemistryWidgetPlate::ShowSuppliesDetail(const ch::tf::SuppliesInfo& suppliesInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 耗材名称--等于-1的时候特殊处理
	if (suppliesInfo.suppliesCode <= 0)
	{
		ui->reagentName->setText(QObject::tr("扫描失败"));
		ui->reagentName->setStyleSheet(QString("color:%1;").arg(WARNING_COLOR));
	}
	else
	{
		ui->reagentName->setText(ThriftEnumTrans::GetSupplyName(suppliesInfo.type, true));
	}

	// 位置信息
	ui->reagentPostion->setText(GetItemFromPostionAndDevice(suppliesInfo.deviceSN, suppliesInfo.pos)->text());
	// 批号
	ui->reagentLot->setText(suppliesInfo.lot.c_str());
	// 过期时间
	QString date = QString::fromStdString(ConverStdStringToDateString(suppliesInfo.expiryTime));
	auto dateTimeString = ToCfgFmtDateTime(date);
	QString dateColor = NORMAL_COLOR;
	if (ChSuppliesExpire(suppliesInfo.expiryTime))
	{
		dateColor = WARNING_COLOR;
	}
	ui->reagentTimelimite->setText(QString("<span style='color:%1;'>%2</span>").arg(dateColor).arg(dateTimeString));

	// 备用耗材，处于待机或者停机
	if (suppliesInfo.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_BACKUP)
	{
		// 耗材优先按钮使能
		ui->reagentPriority->setEnabled(true);
	}

	// 耗材状态
	QString reagentStatus;
	auto vecPairStatus = GetSuppliesShow2Status(suppliesInfo);
	for (const auto& pair : vecPairStatus)
	{
		reagentStatus += QString("<font color='%1'>%2</font>")
			.arg(pair.second).arg(pair.first) + "  ";
	}
	ui->reagentStatus->setText(reagentStatus);

	// 耗材余量
	auto residual = GetSupplyResidual(suppliesInfo);
	if (suppliesInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL || residual < 0)
	{
		// 液位探测失败显示“0.0ml”
		ui->reagentLastTimes->setText("0.0ml");
		ui->reagentLastTimes->setStyleSheet(QString("color:%1;").arg(WARNING_COLOR));
	}
	else
	{
		QString residualText = QString::number(QString::number(residual / UL01_TO_ML, 'f', 1).toDouble()) + "ml";
		ui->reagentLastTimes->setText(residualText);

		// 获取耗材报警量
		boost::optional<std::shared_ptr<ch::tf::SupplyAttribute>> supplyAttribute = CIM_INSTANCE->GetSupplyAttributeByTypeCode(suppliesInfo.type);
		if (supplyAttribute)
		{
			// 警告
			if (residual <= 0)
			{
				ui->reagentLastTimes->setStyleSheet(QString("color:%1;").arg(WARNING_COLOR));
			}
			// 提醒
			else if (supplyAttribute.value()->bottleAlarmThreshold > 0 && residual <= supplyAttribute.value()->bottleAlarmThreshold)
			{
				ui->reagentLastTimes->setStyleSheet(QString("color:%1;").arg(NOTICE_COLOR));
			}
		}
	}

	// 查询同组的耗材信息
	auto chSupplies = CIM_INSTANCE->GetChGroupSupplies(m_currentDevModule->deviceSN, suppliesInfo.suppliesCode);

	// 依次设置耗材
	for (auto supply : chSupplies)
	{
		// 设置当前的备用编号
		if (ShowPostionSpareNumber(supply.pos, true))
		{
			m_backUpShowPos.insert(supply.pos);
		}
	}
}

bool ReagentChemistryWidgetPlate::IsRightReagentOfSupply(const ch::tf::SuppliesInfo& supply, const ch::tf::ReagentGroup& reagent)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (reagent.reagentKeyInfos.empty())
	{
		return false;
	}

	// 依次判断是否是对应的试剂瓶信息
	for (auto keyinfo : reagent.reagentKeyInfos)
	{
		// 批号相同-瓶号相同
		if (keyinfo.lot != supply.lot || keyinfo.sn != supply.sn)
		{
			return false;
		}
	}

	return true;
}

void ReagentChemistryWidgetPlate::ClearReagentStatus()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 试剂名称
	ui->reagentName->setStyleSheet("color:#333;");
	ui->reagentName->setText("");
	// 位置信息
	ui->reagentPostion->setText("");
	// 状态信息
	ui->reagentStatus->setText("");
	// 剩余测试数目
	ui->reagentLastTimes->setStyleSheet("color:#333;");
	ui->reagentLastTimes->setText("");
	// 批号
	ui->reagentLot->setText("");
	// 过期时间
	ui->reagentTimelimite->setText("");
}

void ReagentChemistryWidgetPlate::SetReagentStatus(const ch::tf::SuppliesInfo& supplyInfo, const std::vector<ch::tf::ReagentGroup>& rgtGrops, QReagentItem::ReagentInfo& stuReagentInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 扫描失败 优先级最高
	if (supplyInfo.suppliesCode <= 0)
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::SCANFAIL;
		return;
	}

	// 初始化
	stuReagentInfo.states = 0;
	stuReagentInfo.seqNumber = 0;

	// 仓内耗材
	if (supplyInfo.type != ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT)
	{
		stuReagentInfo.isReagent = false;

		// 耗材剩余测试数
		int cavityNum = -1;
		if (!supplyInfo.cavityInfos.empty())
		{
			cavityNum = GetSupplyResidual(supplyInfo);
		}

		// 耗材可用测试数
		int blanceTotal = GetAvailableSupplyTimes(supplyInfo.suppliesCode, supplyInfo.deviceSN);

		// 开瓶有效期是否过期
		bool openExpired = (GetOpenRestdays(supplyInfo.openBottleExpiryTime) <= 0);

		// 有效期是否过期
		bool expired = ChSuppliesExpire(supplyInfo.expiryTime);

		// 耗材量等于0，可用测试数为0，状态不能用或者未知，开瓶有效期，和有效期过期,液位探测失败  显示红色
		if (cavityNum <= 0 || blanceTotal <= 0
			|| supplyInfo.usageStatus == tf::UsageStatus::type::USAGE_STATUS_CAN_NOT_USE
			|| supplyInfo.type == ch::tf::SuppliesType::SUPPLIES_TYPE_UNKNOWN
			|| supplyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_REGISTER
			|| openExpired || expired)
		{
			stuReagentInfo.states |= QReagentItem::ReagentState::WARNING;
		}

		// 空瓶状态，液位探测失败时也显示空瓶
		if (cavityNum <= 0 || supplyInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
		{
			stuReagentInfo.states |= QReagentItem::ReagentState::NONE;
		}

		// 小于提醒值
		boost::optional<std::shared_ptr<ch::tf::SupplyAttribute>> supplyAttribute = CIM_INSTANCE->GetSupplyAttributeByTypeCode(supplyInfo.type);
		if (supplyAttribute)
		{
			// 项目报警
			if (supplyAttribute.value()->assayAlarmThreshold > 0 
				&& blanceTotal <= supplyAttribute.value()->assayAlarmThreshold
				&& blanceTotal > 0)
			{
				stuReagentInfo.states |= QReagentItem::ReagentState::NOTICE;
			}
			// 瓶报警
			if (supplyAttribute.value()->bottleAlarmThreshold > 0 
				&& cavityNum <= supplyAttribute.value()->bottleAlarmThreshold
				&& cavityNum > 0)
			{
				stuReagentInfo.states |= QReagentItem::ReagentState::NOTICE;
			}
		}

		// 备用试剂
		if (supplyInfo.usageStatus == tf::UsageStatus::type::USAGE_STATUS_BACKUP)
		{
			stuReagentInfo.states |= QReagentItem::ReagentState::SPARE;
			stuReagentInfo.seqNumber = supplyInfo.backupNum;
		}

		// 在用试剂
		if (supplyInfo.usageStatus == tf::UsageStatus::type::USAGE_STATUS_CURRENT)
		{
			stuReagentInfo.states |= QReagentItem::ReagentState::AVAILABLE;
			stuReagentInfo.seqNumber = 0;
		}

		return;
	}

	// 查找对应的试剂
	if (rgtGrops.empty())
	{
		// 若没有对应的试剂则显示警告状态
		stuReagentInfo.states |= QReagentItem::ReagentState::WARNING;
		return;
	}

	// 判断试剂item状态 (为了实现双向同测展示)
	auto flags = JudgeItemStatus(rgtGrops);

	// 判断是否包含校准失败、报警、提醒、屏蔽、项目未启用、液位探测失败
	bool hasCalifailed = flags[0], hasWarn = flags[1], hasNotice = flags[2],
		hasShield = flags[3], hasAssayMask = flags[4], hasDetectFailed = flags[5], hasCaliCurveExp = flags[6];

	// 包含屏蔽状态
	if (hasShield)
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::DEACTIVATE;
	}

	// 其余信息可以根据第一个试剂组判断
	auto firstReagent = rgtGrops[0];

	// 获取耗材使用状态
	tf::UsageStatus::type enTfStatus = firstReagent.usageStatus;

	// 开瓶有效期是否过期
	bool openExpired = (GetOpenRestdays(firstReagent.openBottleExpiryTime) <= 0);

	// 有效期是否过期
	bool expired = ChSuppliesExpire(firstReagent.expiryTime);

	// 告警状态
	if (enTfStatus == tf::UsageStatus::type::USAGE_STATUS_CAN_NOT_USE
		|| supplyInfo.type == ch::tf::SuppliesType::SUPPLIES_TYPE_UNKNOWN
		|| supplyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_REGISTER
		|| openExpired || expired || hasWarn || hasCalifailed
		|| hasAssayMask)
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::WARNING;
	}

	// 空瓶状态，液位探测失败时也显示空瓶
	if ((firstReagent.remainCount <= 0 || hasDetectFailed)
		&& (supplyInfo.placeStatus != tf::PlaceStatus::PLACE_STATUS_REGISTER))
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::NONE;
	}

	// 提醒
	if (hasNotice || hasCaliCurveExp)
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::NOTICE;
	}

	// 备用试剂
	if (enTfStatus == tf::UsageStatus::type::USAGE_STATUS_BACKUP)
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::SPARE;
		stuReagentInfo.seqNumber = firstReagent.backupNum;
	}

	// 在用试剂
	if (enTfStatus == tf::UsageStatus::type::USAGE_STATUS_CURRENT)
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::AVAILABLE;
		stuReagentInfo.seqNumber = 0;
	}

	// 是否开放试剂
	if (firstReagent.openReagent)
	{
		stuReagentInfo.states |= QReagentItem::ReagentState::OPEN;
	}
}

void ReagentChemistryWidgetPlate::DisableFunctionBtn()
{
	// 信息录入
	ui->reagentRegister->setDisabled(true);
	// 试剂屏蔽
	ui->reagentshield->setDisabled(true);
	// 试剂优先
	ui->reagentPriority->setDisabled(true);
}

int ReagentChemistryWidgetPlate::GetSupplyMax(const ch::tf::SuppliesInfo& supplyInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	return std::accumulate(supplyInfo.cavityInfos.begin(), supplyInfo.cavityInfos.end(), 0, [](int a, auto& CavityInfo) { return (a + CavityInfo.maximum); });
}

bool ReagentChemistryWidgetPlate::ShowPostionSpareNumber(int pos, bool IsShow)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__,pos);

    // 设置显示备用编号
    if (IsShow)
    {
        auto reagentInfo = m_pReagentPlate->GetReagent(pos);
        if (reagentInfo)
        {
            // 设置同组标志便于备用编号的显示(添加)
            reagentInfo.value().states |= QReagentItem::ReagentState::SAMEGROUP;
            m_pReagentPlate->LoadReagent(pos, reagentInfo.value());
        }
        else
        {
            return false;
        }
    }
    // 取消备用编号的显示
    else
    {
        auto reagentInfo = m_pReagentPlate->GetReagent(pos);
        if (reagentInfo)
        {
            // 设置同组标志便于备用编号的显示(添加)
            reagentInfo.value().states.setFlag(QReagentItem::ReagentState::SAMEGROUP, false);
            m_pReagentPlate->LoadReagent(pos, reagentInfo.value());
        }
        else
        {
            return false;
        }
    }

    return true;
}

void ReagentChemistryWidgetPlate::PriorityReagent()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	
	// 对应的模块
	if (m_currentDevModule == nullptr || m_currentPostion == -1)
	{
		ULOG(LOG_WARN, "%s(): Have not select the reagent item.", __FUNCTION__);
		return;
	}

	// 弹框提示是否确认优先使用
	if (!IsReagentPriortyTipDlg(true))
	{
		ULOG(LOG_INFO, "Cancel priority!");
		return;
	}

	ch::tf::SuppliesInfoQueryCond qryCond;
	// 试剂位置
	qryCond.__set_pos(m_currentPostion);
	// 试剂盘号
	qryCond.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1);
	// 必须是在用的耗材
	qryCond.__set_placeStatus({ ::tf::PlaceStatus::PLACE_STATUS_REGISTER, ::tf::PlaceStatus::PLACE_STATUS_LOAD });
	// 设备编号
	qryCond.__set_deviceSN({ m_currentDevModule->deviceSN });

	// 如果选中的是耗材
	if (!m_IsReagent && m_currSelectInfo)
	{
		qryCond.__set_suppliesCode(m_currSelectInfo->supplyInfo.suppliesCode);
	}

	// 优先试剂
	if (m_IsReagent)
	{
		if (!ch::c1005::LogicControlProxy::PriorUsingReagents({ qryCond }))
		{
			ULOG(LOG_ERROR, "PriorUsingReagents() failed");
			return;
		}
	}
	// 优先耗材
	else
	{
		if (!ch::c1005::LogicControlProxy::PriorUsingSupplies({ qryCond }))
		{
			ULOG(LOG_ERROR, "PriorUsingSupplies() failed");
			return;
		}
	}
}

void ReagentChemistryWidgetPlate::PrioritySupply()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 判断输入参数是否符合要求
	if (m_pProgressBar == nullptr || m_currentDevModule == nullptr)
	{
		return;
	}

	auto supplyStatus = m_pProgressBar->GetStatus();
	if (supplyStatus.supplyInfo.usageStatus != ::tf::UsageStatus::type::USAGE_STATUS_BACKUP)
	{
		return;
	}

	// 弹框提示是否确认优先使用
	if (!IsReagentPriortyTipDlg(false))
	{
		ULOG(LOG_INFO, "Cancel priority!");
		return;
	}

	ch::tf::SuppliesInfoQueryCond qryCond;
	// 耗材位置
	qryCond.__set_pos(supplyStatus.supplyInfo.pos);
	// 设置耗材编码
	qryCond.__set_suppliesCode(supplyStatus.supplyInfo.suppliesCode);
	// 设置仓外耗材
	qryCond.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_CABINET);
	// 设备编号
	qryCond.__set_deviceSN({ supplyStatus.supplyInfo.deviceSN });
	// 必须是在用的耗材
	qryCond.__set_placeStatus({ ::tf::PlaceStatus::PLACE_STATUS_REGISTER, ::tf::PlaceStatus::PLACE_STATUS_LOAD });
	if (!ch::c1005::LogicControlProxy::PriorUsingSupplies({ qryCond }))
	{
		ULOG(LOG_ERROR, "PriorUsingSupplies() failed");
	}
	else
	{
		DisableFunctionBtn();
	}
}

void ReagentChemistryWidgetPlate::ClearReagentDiskDatas()
{
	if (m_pReagentPlate)
	{
		m_pReagentPlate->CancelSelect();
	}

	DisableFunctionBtn();
	ClearReagentStatus();
	m_currentPostion = -1;
	m_currSelectInfo = nullptr;

	// 取消之前显示的备用编号
	for (auto pos : m_backUpShowPos)
	{
		ShowPostionSpareNumber(pos, false);
	}
	m_backUpShowPos.clear();
}

std::vector<bool> ReagentChemistryWidgetPlate::JudgeItemStatus(const std::vector<ch::tf::ReagentGroup>& reGroupList)
{
	// 判断是否包含校准失败、报警、提醒、屏蔽、项目未分配、液位探测失败、校准曲线过期
	bool califailed = false, warn = false, notice = false, shield = false, unAllocateAssay = false, detectFail = false, caliCurveExp = false;

	for (auto &group : reGroupList)
	{
		if (group.assayCode != ch::tf::g_ch_constants.ASSAY_CODE_SIND
			&& (group.caliStatus == tf::CaliStatus::type::CALI_STATUS_FAIL ||
				(group.caliStatus == tf::CaliStatus::type::CALI_STATUS_NOT && group.caliCurveId <= 0)))
		{
			califailed = true;
		}

		// 试剂剩余测试数
		int usetimes = group.remainCount;
		// 试剂可用测试数
		int totalTestTimes = GetAvailableReagentTimes(group.assayCode, group.deviceSN);

		if ((usetimes <= 0) || (totalTestTimes <= 0))
		{
			warn = true;
		}

		// 获取试剂报警量
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CIM_INSTANCE->GetAssayInfo(group.assayCode);
		if (spAssayInfo)
		{
			// 项目报警
			if (spAssayInfo->__isset.assayAlarmThreshold
				&& spAssayInfo->assayAlarmThreshold > 0
				&& totalTestTimes <= spAssayInfo->assayAlarmThreshold
				&& totalTestTimes > 0)
			{
				notice = true;
			}

			// 瓶报警
			if (spAssayInfo->__isset.bottleAlarmThreshold
				&& spAssayInfo->bottleAlarmThreshold > 0
				&& usetimes <= spAssayInfo->bottleAlarmThreshold
				&& usetimes > 0)
			{
				notice = true;
			}
		}

		// 校准屏蔽和试剂屏蔽
		if (group.caliMask || group.reagentMask)
		{
			shield = true;
		}

		// 项目未分配
		if (CIM_INSTANCE->IsUnAllocatedAssay(group.deviceSN,group.assayCode))
		{
			unAllocateAssay = true;
		}

		// 液位探测失败
		if (group.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
		{
			detectFail = true;
		}

		// 校准曲线有效期
		if (group.suppliesCode != ch::tf::g_ch_constants.ASSAY_CODE_SIND)
		{
			RowCellInfo rci;
			MakeCaliCurveExpirateDate(group, rci);
			QStringList strCalCurveExpLst = rci.m_text.split(":");
			if ((strCalCurveExpLst.size() >= 2) && (strCalCurveExpLst[1].toInt() == 0)
				&& DictionaryQueryManager::GetInstance()->IsCaliLineExpire())
			{
				caliCurveExp = true;
			}
		}
	}

	return { califailed, warn, notice, shield, unAllocateAssay, detectFail, caliCurveExp };
}

QString ReagentChemistryWidgetPlate::GetLoaderStatus(const int status, const bool isReagent)
{
	switch (status)
	{
	case ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_IDLE:// 空位
	case ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_SCANING:// 正在扫描
	{
		return "empty";
	}
	case ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_WAIT_GRAB:// 扫描成功
	case ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_GRABING:// 正在加载
	{
		return isReagent ? "success" : "supply-success";
	}
	case ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_SCAN_FAIL:// 扫描失败
	{
		return "fail";
	}
	case ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_CANCEL_GRAB:// 暂停加载
	{
		return isReagent ? "suspend" : "supply-suspend";
	}
	case ch::tf::LoaderItemStatus::type::LOADER_ITEM_STATUS_UNLOAD:// 卸载成功
	{
		return isReagent ? "uninstall" : "supply-uninstall";
	}
	default:
		return "empty";
	}
}

std::pair<QString, QString> ReagentChemistryWidgetPlate::GetLoaderNameRemain(const ch::tf::ReagentLoaderItem &loaderItem)
{
	QString reagentName, remainNum;

	// 耗材名称
	QString supplyName = QString::fromStdString(loaderItem.suppliesInfo.name);

	bool isReagent = (loaderItem.suppliesInfo.type == ::ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT);
	// 试剂
	if (!loaderItem.reagentGroups.empty())
	{
		// 试剂组1
		auto firstReagent = loaderItem.reagentGroups.at(0);
		QString name1 = QString::fromStdString(CIM_INSTANCE->GetAssayNameByCode(firstReagent.assayCode));
		// 项目名称未找到赋值为耗材名称
		name1.isEmpty() ? (reagentName = supplyName) : (reagentName = name1);
		// 余量
		remainNum = firstReagent.remainCount > 0 ? QString::number(firstReagent.remainCount) : "0";
		if (name1.isEmpty())
		{
			remainNum.clear();
		}

		// 双向同测
		if (loaderItem.reagentGroups.size() > 1)
		{
			// 试剂组2
			auto secondReagent = loaderItem.reagentGroups.at(1);

			// 项目名称
			QString name2 = QString::fromStdString(CIM_INSTANCE->GetAssayNameByCode(secondReagent.assayCode));
			bool notFindAssay = name2.isEmpty();
			notFindAssay ? (name2 = supplyName) : void(0);
			if (!reagentName.isEmpty() || !name2.isEmpty())
			{
				reagentName += "/" + name2;
			}

			// 余量
			QString remain2 = secondReagent.remainCount > 0 ? QString::number(secondReagent.remainCount) : "0";
			if (notFindAssay)
			{
				remain2.clear();
			}
			if (!remainNum.isEmpty() || !remain2.isEmpty())
			{
				remainNum += "/" + remain2;
			}
		}
	}
	else// 耗材
	{
		// 试剂（项目未启用）只显示耗材名称
		if (isReagent)
		{
			reagentName = supplyName;
		}
		else
		{
			// 耗材余量
			auto residual = GetSupplyResidual(loaderItem.suppliesInfo);
			if (residual >= 0)
			{
				remainNum = QString::number(QString::number(residual / UL01_TO_ML, 'f', 1).toDouble()) + "ml";
			}
			else
			{
				// 余量小于0时显示0ml
				remainNum = "0.0ml";
			}

			// 耗材编号小于0时候特殊处理
			if (loaderItem.suppliesInfo.suppliesCode <= 0)
			{
				reagentName = tr("扫描失败");
				remainNum.clear();
			}
			else
			{
				reagentName = ThriftEnumTrans::GetSupplyName(loaderItem.suppliesInfo.type, true);
			}
		}
	}

	return { reagentName, remainNum };
}

void ReagentChemistryWidgetPlate::OnSelectedReagentSlotChanged(int iIndex)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 更新试剂盘选中槽位信息
	UpdateSelSlotInfo(iIndex);
}

void ReagentChemistryWidgetPlate::OnRefreshReagentPlate()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_currentDevModule == nullptr)
	{
		return;
	}

	UpdateReagentPlate(m_currentDevModule);
}

void ReagentChemistryWidgetPlate::OnRefreshCabinetSupply()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_currentDevModule == nullptr)
	{
		return;
	}

	UpdateCabinetSupplies(m_currentDevModule);
}

void ReagentChemistryWidgetPlate::OnDeviceOtherInfoUpdate(const QString &devSn)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devSn.toStdString());

	if (m_currentDevModule == nullptr || m_currentDevModule->deviceSN != devSn.toStdString())
	{
		return;
	}

	// 更新装载器状态
	UpdateReagentLoaderStatus(m_currentDevModule);

	// 更新反应杯酸碱清洗液显示
	UpdateCabinetSupplies(m_currentDevModule);
}

void ReagentChemistryWidgetPlate::onRegisterClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_registerDialog == nullptr)
    {
        m_registerDialog = new QRegisterSupplyInfo(this);
    }

    if (m_currentDevModule == nullptr)
    {
        return;
    }

	// 获取设备名称
	std::string devName = m_currentDevModule->groupName + m_currentDevModule->name;
	if(m_currSelectInfo)
	{
		m_registerDialog->SetSupplyPostion(m_currSelectInfo->supplyInfo, m_currentDevModule->deviceSN, devName + "-" + to_string(m_currSelectInfo->supplyInfo.pos));
	}
	else
	{
		ch::tf::SuppliesInfo si;
		si.__set_pos(m_currentPostion);
		m_registerDialog->SetSupplyPostion(si, m_currentDevModule->deviceSN, devName + "-" + to_string(m_currentPostion));
	}

    m_registerDialog->show();
}

void ReagentChemistryWidgetPlate::OnReagentShield()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 对应的模块
	if (m_currentDevModule == nullptr)
	{
		ULOG(LOG_WARN, "have not select the module");
		return;
	}

	auto &currMask = m_currSelectInfo->reagentInfos[0].reagentMask;

	// 弹框提示是否确认试剂屏蔽
	if (m_currSelectInfo && !m_currSelectInfo->reagentInfos.empty() &&
		!IsReagentShieldTipDlg(ui->reagentName->text(), !currMask))
	{
		return;
	}

	ch::tf::SuppliesInfoQueryCond qryCond;
	// 试剂位置
	qryCond.__set_pos(m_currentPostion);
	// 试剂盘号
	qryCond.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1);
	// 必须是在用的耗材
	qryCond.__set_placeStatus({ ::tf::PlaceStatus::PLACE_STATUS_REGISTER, ::tf::PlaceStatus::PLACE_STATUS_LOAD });
	// 设备编号
	qryCond.__set_deviceSN({ m_currentDevModule->deviceSN });

	// 当前试剂被屏蔽
	if (currMask)
	{
		if (!ch::c1005::LogicControlProxy::UnMaskReagentGroups({ qryCond }))
		{
			ULOG(LOG_ERROR, "UnMaskReagentGroups() failed");
			return;
		}

		currMask = false;
	}
	// 若当前试剂没有被屏蔽
	else
	{
		if (!ch::c1005::LogicControlProxy::MaskReagentGroups({ qryCond }))
		{
			ULOG(LOG_ERROR, "MaskReagentGroups() failed");
			return;
		}

		currMask = true;
	}
}

void ReagentChemistryWidgetPlate::OnPriority()
{
	m_focusState ? PrioritySupply() : PriorityReagent();
}

void ReagentChemistryWidgetPlate::OnReagentUnload()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_currentDevModule == nullptr)
	{
		ULOG(LOG_ERROR, "m_currentDevModule is nullptr");
		return;
	}

	if (m_unloadDlg == nullptr)
	{
		m_unloadDlg = new ChReagentUnLoadDlg(this);
	}

	m_unloadDlg->SetData(m_currentDevModule, m_currentPostion);
	m_unloadDlg->show();
}

void ReagentChemistryWidgetPlate::OnScanReagent()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_currentDevModule == nullptr)
	{
		ULOG(LOG_ERROR, "m_currentDevModule is nullptr");
		return;
	}

	if (m_scanDialog == nullptr)
	{
		m_scanDialog = new QReagentScanDialog(this);
	}
	m_scanDialog->show();
}

void ReagentChemistryWidgetPlate::OnUpdateSupply(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates)
{
	ULOG(LOG_INFO, u8"试剂盘收到刷新消息 %s(%s)", __FUNCTION__, ToString(supplyUpdates));

	if (!isVisible())
	{
		m_needUpdate = true;
		return;
	}

	// 更新的仓外耗材位置、试剂盘位置
	std::set<int> suppliesPos, platePos;
	for (auto supplyUpdate : supplyUpdates)
	{
		// 对应的模块为空
		if (m_currentDevModule == nullptr)
		{
			return;
		}

		// 不是当前设备没有必要更新
		if (supplyUpdate.deviceSN != m_currentDevModule->deviceSN)
		{
			continue;
		}

		// 仓外耗材
		if (supplyUpdate.posInfo.area == ch::tf::SuppliesArea::type::SUPPLIES_AREA_CABINET)
		{
			suppliesPos.insert(supplyUpdate.posInfo.pos);
			continue;
		}
		
		platePos.insert(supplyUpdate.posInfo.pos);
	}

	if (!suppliesPos.empty())
	{
		UpdateCabinetSupplies(m_currentDevModule, suppliesPos);
	}

	if (!platePos.empty())
	{
		UpdateReagentPlate(m_currentDevModule, platePos);
	}
}

void ReagentChemistryWidgetPlate::OnDeviceStatusUpdate(class tf::DeviceInfo deviceInfo)
{
	if (m_currentDevModule == nullptr || m_currentDevModule->deviceSN != deviceInfo.deviceSN)
	{
		return;
	}

	// 待机和停机时允许试剂扫描
	ui->reagentScan->setEnabled(deviceInfo.__isset.status && 
		(deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY
			|| deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_HALT));
}

void ReagentChemistryWidgetPlate::OnLoaderStatusUpdate(QString deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_currentDevModule != Q_NULLPTR && m_currentDevModule->deviceSN == deviceSN.toStdString())
	{
		UpdateReagentLoaderStatus(m_currentDevModule);
	}
}

void ReagentChemistryWidgetPlate::OnFocusOut()
{
	if (m_pProgressBar == nullptr)
	{
		return;
	}
	
	// 先禁止点击优先使用
    ui->reagentPriority->setEnabled(false);
    m_focusState = false;
    m_pProgressBar->setChecked(false);
    m_pProgressBar = nullptr;
}

void ReagentChemistryWidgetPlate::OnSelectSupply()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ui->reagentshield->setText(tr("试剂屏蔽"));

	// 取消试剂盘的选中
	ClearReagentDiskDatas();

    // 获取控件句柄
    QProgressBarShell* pProgressBar = qobject_cast<QProgressBarShell*>(sender());
    if (pProgressBar == nullptr)
    {
        return;
    }
	m_pProgressBar = pProgressBar;
	m_focusState = true;

	// 设置选中
	for (const auto &washLiquid : m_cupWashLiquidMap)
	{
		auto p = washLiquid.second.progressBar;
		if (pProgressBar == p)
		{
			p->setChecked(true);
		}
		else
		{
			p->setChecked(false);
		}
	}

    auto supplyStatus = pProgressBar->GetStatus();
    ui->reagentPriority->setEnabled(supplyStatus.supplyInfo.usageStatus == ::tf::UsageStatus::type::USAGE_STATUS_BACKUP);
}

void ReagentChemistryWidgetPlate::OnShowBucket()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (!isVisible())
    {
        m_needUpdate = true;
        return;
    }

    if (!m_currentDevModule)
    {
        return;
    }

    // 更新设备信息
    auto deviceInfo = CIM_INSTANCE->GetDeviceInfo(m_currentDevModule->deviceSN);
    if (!deviceInfo)
    {
        return;
    }

    m_currentDevModule = deviceInfo;
    if (m_currentDevModule->enableWasterContainer)
    {
        ui->supplyStatus7->show();
        ui->frame_discar->show();

        QProgressBarShell::SupplyStatus statusData;
        statusData.supplyType = QProgressBarShell::WASTELIQUID;
		auto cnt = CIM_INSTANCE->GetChWasteCnt(m_currentDevModule->deviceSN);

        statusData.blance = cnt;
        auto maxVolumn = ch::c1005::tf::g_c1005_constants.MAX_WASTE;
        auto progress = (maxVolumn == 0) ? 0 : 100 * (double(cnt) / maxVolumn);
        statusData.progress = progress;

        ui->status_show7->hide();
        ui->supplyStatus7->SetSupplyStatus(statusData);

    }
    else
    {
        ui->supplyStatus7->hide();
        ui->frame_discar->hide();
    }
}

void ReagentChemistryWidgetPlate::OnUpdateBucketStatus(const QString& devSN)
{
	if (m_currentDevModule && m_currentDevModule->deviceSN == devSN.toStdString())
	{
		OnShowBucket();
	}
}
