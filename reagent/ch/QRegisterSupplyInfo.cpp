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
/// @file     QRegisterSupplyInfo.cpp
/// @brief    试剂耗材信息录入
///
/// @author   5774/WuHongTao
/// @date     2022年3月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QRegisterSupplyInfo.h"
#include "ui_QRegisterSupplyInfo.h"
#include <QDateTime>
#include "shared/tipdlg.h"
#include "shared/ReagentCommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/leonis/thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_ui_control_constants.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"

QRegisterSupplyInfo::QRegisterSupplyInfo(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::QRegisterSupplyInfo();
    ui->setupUi(this);
    SetTitleName(tr("信息录入"));

    ui->reagent_pos->setEnabled(false);
    ui->reagent_lot->setValidator(new QRegExpValidator(QRegExp(UI_LOT), this));
    ui->reagent_sn->setValidator(new QRegExpValidator(QRegExp(UI_SN), this));

    connect(ui->ok_btn, SIGNAL(clicked()), this, SLOT(OnOkButtonClicked()));
    connect(ui->cancel_btn, SIGNAL(clicked()), this, SLOT(close()));
}

///
/// @brief
///     
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月10日，新建函数
///
QRegisterSupplyInfo::~QRegisterSupplyInfo()
{
}

void QRegisterSupplyInfo::SetSupplyPostion(const ch::tf::SuppliesInfo& supply, const std::string& devSn, const std::string &devName)
{
    ULOG(LOG_INFO, "%s(supply=%s, devSn=%s, devName=%s)", __FUNCTION__, ToString(supply), devSn, devName);

    m_currentDevSn = devSn;
	m_pos = supply.pos;

    ui->reagent_pos->setText(QString::fromStdString(devName));
    ui->reagent_lot->clear();
    ui->reagent_sn->clear();
    ui->reagent_date->setDate(QDate::currentDate().addDays(30));

    ui->reagent_lot->setValidator(new QRegExpValidator(QRegExp(UI_LOT), this));
    ui->reagent_sn->setValidator(new QRegExpValidator(QRegExp(UI_SN), this));

    if (supply.__isset.suppliesCode && supply.suppliesCode > 0)
    {
		QString strLot, strSn;
		strLot = ((supply.lotSnMode & tf::EnumLotSnMode::ELSM_LOT) != 0) ? QString::fromStdString(supply.lot) : "";
		strSn = ((supply.lotSnMode & tf::EnumLotSnMode::ELSM_SN) != 0) ? QString::fromStdString(supply.sn) : "";
        // 批号
        ui->reagent_lot->setText(strLot);
        // 瓶号
        ui->reagent_sn->setText(strSn);
        // 失效日期
        ui->reagent_date->setDate(QDateTime::fromString(QString::fromStdString(supply.expiryTime), "yyyy-MM-dd hh:mm:ss").date());
        // 刷新项目序列
        RefreshAssayList(supply.suppliesCode);
    }
    else
    {
        // 刷新项目序列
        RefreshAssayList(-1);
    }
}

void QRegisterSupplyInfo::RefreshAssayList(int suppliesCode)
{
	ULOG(LOG_INFO, "%s(suppliesCode=%d)", __FUNCTION__, suppliesCode);

	ui->ReagentName_comboBox->clear();

	// 获取启用的生化项目映射表
	ChAssayIndexUniqueOrderCodeMaps chAssayMap;
	CommonInformationManager::GetInstance()->GetChAssayMaps(::tf::DeviceType::DEVICE_TYPE_C1000, chAssayMap);

	// 获取通用项目映射表
	auto assayMap = CommonInformationManager::GetInstance()->GetAssayInfo();

	// 添加空行清除试剂组
	int i = 0;
	ui->ReagentName_comboBox->addItem("", -1);
	ui->ReagentName_comboBox->setItemData(i, false, Qt::UserRole + 1);

    // 设置下拉表--只显示开放试剂
	for (const auto& assay : chAssayMap)
	{
		// 试剂编号
		int reagenCode = assay.second->reagentCode;
		// 项目编号
		int assayCode = assay.second->assayCode;
		// 下拉框只显示开放试剂
		if (assayMap.count(assayCode) && assayMap[assayCode]->openAssay)
		{
			i++;
			// 设置项目选项
			ui->ReagentName_comboBox->addItem(QString::fromStdString(assayMap[assayCode]->assayName), reagenCode);
			ui->ReagentName_comboBox->setItemData(i, assay.second->enableLotNumManage, Qt::UserRole + 1);
		}
	}

    ui->ReagentName_comboBox->setCurrentIndex(ui->ReagentName_comboBox->findData(suppliesCode));
}

///
/// @brief
///     
///     保存数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月18日，新建函数
///
void QRegisterSupplyInfo::OnOkButtonClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	bool enableLotNumManage = ui->ReagentName_comboBox->itemData(ui->ReagentName_comboBox->currentIndex(), Qt::UserRole + 1).toBool();
	if (enableLotNumManage && ui->reagent_lot->text().isEmpty())
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("该项目启用了批号管理，批号不能为空.")));
		pTipDlg->exec();
		return;
	}

	int supplyCode = ui->ReagentName_comboBox->currentData().toInt();

    ch::tf::SuppliesInfo supply;
    supply.__set_suppliesCode(supplyCode);
	supply.__set_type(ch::tf::SuppliesType::SUPPLIES_TYPE_REAGENT);
	supply.__set_placeStatus(tf::PlaceStatus::PLACE_STATUS_REGISTER);
	supply.__set_manualRegister(true);
    supply.__set_version(1);
    supply.__set_name(ui->ReagentName_comboBox->currentText().toStdString());
    supply.__set_deviceSN(m_currentDevSn);
    supply.__set_area(::ch::tf::SuppliesArea::SUPPLIES_AREA_REAGENT_DISK1);
	if (m_pos != 0)
	{
		supply.__set_pos(m_pos);
	}

    // 批号 瓶号
	QString lot = ui->reagent_lot->text();
	QString sn = ui->reagent_sn->text();
	supply.__set_lot(lot.toStdString());
	supply.__set_sn(sn.toStdString());
	
	// 判断批号瓶号模式
	if(lot.isEmpty() && sn.isEmpty())
	{
		supply.__set_lotSnMode(tf::EnumLotSnMode::ELSM_NO_LOT_SN);
	}
	else if (!lot.isEmpty() && !sn.isEmpty())
	{
		supply.__set_lotSnMode(tf::EnumLotSnMode::ELSM_LOT_SN);
	}
	else if (!lot.isEmpty())
	{
		supply.__set_lotSnMode(tf::EnumLotSnMode::ELSM_LOT);
	}
	else if (!sn.isEmpty())
	{
		supply.__set_lotSnMode(tf::EnumLotSnMode::ELSM_SN);
	}

	// 根据项目参数设置试剂阶段
	auto spAssayInfo = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(supplyCode, ::tf::DeviceType::DEVICE_TYPE_C1000);
	if (spAssayInfo)
	{
		// R1-R3阶段 吸取量为0的排除
		if ((spAssayInfo->reagentAspirateVols.size() >= 2) && (spAssayInfo->reagentAspirateVols[1].reagent > 0))
		{
			supply.__set_subType(ch::tf::ReagentStage::type::REAGENT_STAGE_R1R3);
		}
		else
		{
			supply.__set_subType(ch::tf::ReagentStage::type::REAGENT_STAGE_R1);
		}
	}

    // 腔容量信息, 试剂盒都有两个腔
    std::vector<ch::tf::CavityInfo> cavityInfos;
    for (int i = 1; i <= 2; i++)
    {
        ch::tf::CavityInfo cavi;
        cavi.__set_cavityNum(i);
        cavi.__set_maximum((1 == i)? ::ch::tf::g_ch_constants.REAGENT_CAVITY1_SIZE: ::ch::tf::g_ch_constants.REAGENT_CAVITY2_SIZE);
        cavi.__set_residual(-1);
        cavityInfos.push_back(std::move(cavi));
    }
    supply.__set_cavityInfos(cavityInfos);

	// 过期时间
	QString m_expiryTime = ui->reagent_date->date().toString("yyyy-MM-dd") + QString(" 00:00:00");
	supply.__set_expiryTime(m_expiryTime.toStdString());

    // 登记耗材
	::ch::tf::ManualRegisterResult::type result = ch::c1005::LogicControlProxy::ManualRegisterSupplies(supply);
	// 登记失败提示
    if (result != ::ch::tf::ManualRegisterResult::MRR_SUCCESS)
    {
		QString txt = tr("登记失败");
		switch (result)
		{
		case ch::tf::ManualRegisterResult::MRR_INVALID:
			txt += tr(",耗材信息不完整.");
			break;
		case ch::tf::ManualRegisterResult::MRR_REGISTERED_ELSEWHERE:
			txt += tr(",耗材已登记.");
			break;
		case ch::tf::ManualRegisterResult::MRR_LOADED_ELSEWHERE:
			txt += tr(",耗材已放置.");
			break;
		case ch::tf::ManualRegisterResult::MRR_SCAN_LOAD:
			txt += tr(",射频卡扫描耗材不允许改变.");
			break;
		case ch::tf::ManualRegisterResult::MRR_CAVITY_NOT_MATCH:
			txt += tr(",放置的耗材与录入的耗材腔信息不匹配.");
			break;
		default:
			break;
		}

		ULOG(LOG_ERROR, "ManualRegisterSupplies function excute failed");
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(txt));
		pTipDlg->exec();
		return;
    }

	close();
}
