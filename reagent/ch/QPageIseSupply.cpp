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

#include "QPageIseSupply.h"
#include "ui_QPageIseSupply.h"
#include "QRecordISESupply.h"

#include "shared/CommonInformationManager.h"
#include "shared/QProgressBarShell.h"
#include "shared/ReagentCommon.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/DictionaryQueryManager.h"

#include "QIsePerfusionDlg.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/common/defs.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ise/IseConfigDefine.h"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ise/IseConfigSerialize.h"
#include "utility/maintaindatamng.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include <QMouseEvent>

QPageIseSupply::QPageIseSupply(QWidget *parent)
    : QWidget(parent)
    , m_regIseDialog(Q_NULLPTR)
	, m_isePerfusionDlg(new QIsePerfusionDlg(this))
{
    ui = new Ui::QPageIseSupply();
    ui->setupUi(this);
    // 试剂加载
    connect(ui->supplyLoad_btn, &QPushButton::clicked, this, [&]() 
    {
        OnApplyIseSupply();
    });

	// 液路灌注
	connect(ui->perfusion_btn, SIGNAL(clicked()), this, SLOT(OnISEPerfusion()));

	// ISE耗材更新消息
	REGISTER_HANDLER(MSG_ID_ISE_SUPPLY_INFO_UPDATE, this, OnUpdateSupplyForISE);

	// 监听耗材管理设置更新(ISE)
	REGISTER_HANDLER(MSG_ID_DEVICE_OTHER_INFO_UPDATE, this, OnDeviceOtherInfoUpdate);

    auto buttons = findChildren<QPushButton*>();
    for (auto button : buttons)
    {
        button->setEnabled(true);
    }

	ui->perfusion_btn->setEnabled(true);
    ui->supplyLoad_btn->setEnabled(true);

	m_mapIseCards[EM_IS_1] = ui->ise1_is;
	m_mapIseCards[EM_BUF_1] = ui->ise1_buf;
	m_mapIseCards[EM_IS_2] = ui->ise2_is;
	m_mapIseCards[EM_BUF_2] = ui->ise2_buf;

	// 注册事件过滤器
	for (auto &card : m_mapIseCards)
	{
		card->installEventFilter(this);
		card->getIcon()->installEventFilter(this);
	}

	// 样本针酸性清洗液
	connect(ui->desk_cln1, &QIseCard::signalRemainDetect, this, [=]()
	{
		// 获取余量探测维护组
		auto maintainGroup = MaintainDataMng::GetInstance()->GetSingleMaintainGrpByType(tf::MaintainItemType::MAINTAIN_ITEM_ISE_SURPLUS_DETECTION);

		// 设置设备
		::tf::DevicekeyInfo devKeyInfo;
		devKeyInfo.__set_sn(ui->desk_cln1->getDevSN());
		devKeyInfo.__set_modelIndex(0);

		// 扫描酸性清洗液
		auto encodeParam = MaintainDataMng::GetInstance()->GetIseSurplusDetectionParam({ ise::ise1005::tf::CabinetPos::type::POS_SAMPLER_DETERGENT_ACIDITY });

		// 执行维护
		tf::MaintainExeParams exeParams;
		exeParams.__set_groupId(maintainGroup.id);
		exeParams.__set_lstDev({ devKeyInfo });
		exeParams.__set_itemParams({ { tf::MaintainItemType::MAINTAIN_ITEM_ISE_SURPLUS_DETECTION , encodeParam } });
		if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
		{
			ULOG(LOG_ERROR, "Failed to execute ISE_SURPLUS_DETECTION.");
		}

	});
	// 样本针碱性清洗液
	connect(ui->desk_cln2, &QIseCard::signalRemainDetect, this, [=]()
	{
		// 获取余量探测维护组
		auto maintainGroup = MaintainDataMng::GetInstance()->GetSingleMaintainGrpByType(tf::MaintainItemType::MAINTAIN_ITEM_ISE_SURPLUS_DETECTION);

		// 设置设备
		::tf::DevicekeyInfo devKeyInfo;
		devKeyInfo.__set_sn(ui->desk_cln2->getDevSN());
		devKeyInfo.__set_modelIndex(0);

		// 扫描碱性清洗液
		auto encodeParam = MaintainDataMng::GetInstance()->GetIseSurplusDetectionParam({ ise::ise1005::tf::CabinetPos::type::POS_SAMPLER_DETERGENT_ALKALINITY });

		// 执行维护
		tf::MaintainExeParams exeParams;
		exeParams.__set_groupId(maintainGroup.id);
		exeParams.__set_lstDev({ devKeyInfo });
		exeParams.__set_itemParams({ { tf::MaintainItemType::MAINTAIN_ITEM_ISE_SURPLUS_DETECTION , encodeParam } });
		if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
		{
			ULOG(LOG_ERROR, "Failed to execute ISE_SURPLUS_DETECTION.");
		}
	});
}

QPageIseSupply::~QPageIseSupply()
{
}

void QPageIseSupply::OnUpdateSupplyForISE(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates)
{
	ULOG(LOG_INFO, "%s(supplyUpdates=[ %s ])", __FUNCTION__, ToString(supplyUpdates));

	if (!isVisible())
	{
		ULOG(LOG_INFO, "not need update");
		return;
	}

	// 若ISE的变化的耗材是当前的耗材，那么更新
	std::set< std::pair<int, int>> setPos;
	for (const auto& supplyUpdate : supplyUpdates)
	{
		if (supplyUpdate.suppliesInfo.deviceSN == m_devSn)
		{
			setPos.insert({ supplyUpdate.suppliesInfo.moduleIndex, supplyUpdate.suppliesInfo.pos });
		}
	}

	if (!setPos.empty())
	{
		UpdateIseSupplies(m_devSn, setPos);
	}
}

void QPageIseSupply::OnISEPerfusion()
{
	m_isePerfusionDlg->LoadDataToDlg({m_device});
	m_isePerfusionDlg->show();
}

void QPageIseSupply::OnDeviceOtherInfoUpdate(const QString &devSn)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, devSn.toStdString());

	if (!isVisible() || devSn.toStdString() != m_devSn)
	{
		ULOG(LOG_INFO, "not need update");
		return;
	}

	UpdateIseSupplies(m_devSn);
}

///
/// @brief 注册ISE耗材
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年3月7日，新建函数
///
void QPageIseSupply::OnApplyIseSupply()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_regIseDialog == Q_NULLPTR)
    {
        m_regIseDialog = new QRecordISESupply();
    }

	// 获取选中耗材
	if (m_selectSupply == -1)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未选中ISE的%1或者%2!").arg(ISE_IS_NAME).arg(ISE_BUF_NAME)));
		pTipDlg->exec();
		return;
	}

	int moduleIndex = 1;
	int sypplyType = ise::tf::SuppliesType::SUPPLIES_TYPE_IS;
	if (m_selectSupply == EM_BUF_1)
	{
		sypplyType = ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER;
	}
	else if (m_selectSupply == EM_IS_2)
	{
		moduleIndex = 2;
	}
	else if (m_selectSupply == EM_BUF_2)
	{
		moduleIndex = 2;
		sypplyType = ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER;
	}

	m_regIseDialog->StartDialog(m_device, moduleIndex, sypplyType);
}

void QPageIseSupply::updatePerfusionBtnEnable(const std::string &devSn, const int &status)
{
	if (m_device && devSn == m_device->deviceSN)
	{
		ui->perfusion_btn->setEnabled(status == tf::DeviceWorkState::DEVICE_STATUS_STANDBY);
	}
}

void QPageIseSupply::clearSelect()
{
	for (auto &card : m_mapIseCards)
	{
		card->SetChecked(false);
	}

	m_selectSupply = -1;
}

void QPageIseSupply::OnSelect(QObject * sender)
{
	for (auto it = m_mapIseCards.begin(); it != m_mapIseCards.end(); ++it)
	{
		if (it.value() == sender 
			|| it.value()->getIcon() == sender)
		{
			m_selectSupply = it.key();
			it.value()->SetChecked(true);
		}
		else
		{
			it.value()->SetChecked(false);
		}
	}
}

void QPageIseSupply::showEvent(QShowEvent *event)
{
	UpdateIseSupplies(m_devSn);
}

bool QPageIseSupply::eventFilter(QObject *obj, QEvent *event)
{
	// 非鼠标事件
	if ((event->type() != QEvent::MouseButtonPress))
	{
		return QWidget::eventFilter(obj, event);
	}

	// 判断是鼠标左键
	auto mouseEvent = static_cast<QMouseEvent *>(event);
	if (mouseEvent->button() != Qt::LeftButton)
	{
		return QWidget::eventFilter(obj, event);
	}

	OnSelect(obj);

	return QWidget::eventFilter(obj, event);
}

void QPageIseSupply::UpdateIseSupplies(const std::string& deviceSn, const std::set< std::pair<int, int>> &setPos/* = {}*/)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    if (deviceSn.empty())
    {
        ULOG(LOG_INFO, "the device is empty.");
        return;
    }

    auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSn);
    if (nullptr == spDevice)
    {
        ULOG(LOG_ERROR, "spDevice is nullptr");
        return;
    }

	// 缓存当前设备
	m_device = spDevice;
	m_devSn = deviceSn;

	// 清除选中
	clearSelect();

    // 查找配置信息
    IseDeviceOtherInfo washSet;
    if (!DecodeJson(washSet, spDevice->otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson IseDeviceOtherInfo Failed");
        return;
    }

	// 单机默认隐藏样本针酸碱清洗液
	if (!DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		ui->desk->hide();
		ui->desk_cln1->hide();
		ui->desk_cln2->hide();
	}

	// 获取ise设备耗材信息
	auto iseSupplies = CommonInformationManager::GetInstance()->GetIseSupplies(deviceSn, setPos);
	if (setPos.empty())
	{
		// 先隐藏
		ui->ise1->hide();
		ui->ise2->hide();
		ui->desk->hide();
		ui->desk_cln1->hide();
		ui->desk_cln2->hide();
		ui->groupBox_2->hide();
		ui->groupBox_4->hide();

		for (const auto& si : iseSupplies)
		{
			UpdateIseSupply(si.second, washSet);
		}
	}
	else
	{
		for (const auto& pos : setPos)
		{
			if (!iseSupplies.count(pos))
			{
				hideSupply(pos);
			}
			else
			{
				UpdateIseSupply(iseSupplies[pos], washSet);
			}
		}
	}

    // 两个都隐藏，才隐藏
	if (ui->ise1_is->isVisible() || ui->ise1_buf->isVisible())
	{
		ui->ise1->show();
		ui->groupBox_4->show();
	}
	else
	{
		ui->ise1->hide();
		ui->groupBox_4->hide();
	}

	if (ui->ise2_is->isVisible() || ui->ise2_buf->isVisible())
	{
		ui->ise2->show();
		ui->groupBox_2->show();
	}
	else
	{
		ui->ise2->hide();
		ui->groupBox_2->hide();
	}

    if (ui->desk_cln1->isVisible() || ui->desk_cln2->isVisible())
    {
        ui->desk->setText(tr("台面"));
        ui->desk->show();
    }
	else
	{
		ui->desk->hide();
	}

    // 是否只剩两个控件，那么上层控件调整为水平显示
    {
        int visable = 0;
        auto cards = findChildren<QIseCard*>();
        for (const auto& card : cards)
        {
            card->isVisible() ? visable++ : void(0);
        }

        // 当显示控件小于2的时候，设置水平显示
        if (visable <= 2)
        {
            auto boxs = findChildren<QGroupBox*>();
            for (const auto& box : boxs)
            {
               auto layout = qobject_cast<QBoxLayout*>(box->layout());
               if (layout != Q_NULLPTR)
               {
                   layout->setDirection(QBoxLayout::Direction::LeftToRight);
               }
            }
        }
        else
        {
            auto boxs = findChildren<QGroupBox*>();
            for (const auto& box : boxs)
            {
                auto layout = qobject_cast<QBoxLayout*>(box->layout());
                if (layout != Q_NULLPTR)
                {
                    layout->setDirection(QBoxLayout::Direction::TopToBottom);
                }
            }
        }
    }

	// 如果设备状态为待机允许点击管路填充
	ui->perfusion_btn->setEnabled(spDevice->status == tf::DeviceWorkState::DEVICE_STATUS_STANDBY);
}

void QPageIseSupply::UpdateIseSupply(const ise::tf::SuppliesInfo &si, const IseDeviceOtherInfo& otherInfo)
{
	auto iseTitle = GetIseName(si);

	// ISE设备1
	if (si.moduleIndex == 1)
	{
		// 缓释液体
		if (si.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER)
		{
			ui->ise1_buf->SetIseSupply(si);
		}
		else
		{
			ui->ise1_is->SetIseSupply(si);
		}

		ui->ise1->setText(iseTitle);
		ui->ise1->show();
		ui->groupBox_4->show();
	}
	// ISE设备2
	else if (si.moduleIndex == 2)
	{
		// 缓释液体
		if (si.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER)
		{
			ui->ise2_buf->SetIseSupply(si);
		}
		else
		{
			ui->ise2_is->SetIseSupply(si);
		}

		ui->ise2->setText(iseTitle);
		ui->ise2->show();
		ui->groupBox_2->show();
	}
	//台面
	else
	{
		// 联机状态才考虑是否显示样本针清洗液等
		if (!DictionaryQueryManager::GetInstance()->GetPipeLine())
		{
			return;
		}

		// 样本针酸性清洗液
		if (otherInfo.acidity)
		{
			if (si.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY)
			{
				ui->desk_cln1->SetSmpClnSupply(si);
				ui->desk_cln1->show();
			}
		}
		else
		{
			ui->desk_cln1->hide();
		}

		// 样本针碱性清洗液
		if (otherInfo.alkalinity)
		{
			if (si.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY)
			{
				ui->desk_cln2->SetSmpClnSupply(si);
				ui->desk_cln2->show();
			}
		}
		else
		{
			ui->desk_cln2->hide();
		}
	}
}

void QPageIseSupply::hideSupply(const std::pair<int, int>& posInfo)
{
	// 模块索引（0表示整机，≥1表示具体的模块）
	// 位置（1：内标液IS，2：缓冲液Buf，3：样本针酸性清洗剂，4：样本针碱性清洗剂）
	const QMap<std::pair<int, int>, QIseCard*> maps = 
	{
		{ { 1,ise::ise1005::tf::CabinetPos::type::POS_IS }, ui->ise1_is },
		{ { 1,ise::ise1005::tf::CabinetPos::type::POS_BUF }, ui->ise1_buf },
		{ { 2,ise::ise1005::tf::CabinetPos::type::POS_IS }, ui->ise2_is },
		{ { 2,ise::ise1005::tf::CabinetPos::type::POS_BUF }, ui->ise2_buf },
		{ { 0,ise::ise1005::tf::CabinetPos::type::POS_SAMPLER_DETERGENT_ACIDITY }, ui->desk_cln1 },
		{ { 0,ise::ise1005::tf::CabinetPos::type::POS_SAMPLER_DETERGENT_ALKALINITY }, ui->desk_cln2 }
	};

	if (maps.contains(posInfo))
	{
		maps[posInfo]->hide();
	}
}
