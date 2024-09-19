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
/// @file     ChReagentUnLoadDlg.cpp
/// @brief    生化试剂卸载对话框
///
/// @author   7997/XuXiaoLong
/// @date     2024年8月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年8月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ChReagentUnLoadDlg.h"
#include "ui_ChReagentUnLoadDlg.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/ReagentCommon.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ch/ChConfigDefine.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include <QRegExpValidator>

ChReagentUnLoadDlg::ChReagentUnLoadDlg(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::ChReagentUnLoadDlg();
    ui->setupUi(this);

	InitUi();
	InitConnect();
}

ChReagentUnLoadDlg::~ChReagentUnLoadDlg()
{

}

void ChReagentUnLoadDlg::SetData(std::shared_ptr<const tf::DeviceInfo> devInfo, const int selectPos)
{
	m_spDevInfo = devInfo;
	ui->select_edit->clear();
	if (selectPos > 0)
	{
		ui->select_edit->setText(QString::number(selectPos));
	}
	else
	{
		ui->select_edit->setText(QString::number(1));
	}
}

void ChReagentUnLoadDlg::InitUi()
{
	SetTitleName(tr("试剂卸载"));

	// 设置输入限制，限制位置信息只能输入"数字、逗号、-"，验证工作留到点击确定的时候
	QRegExp rxSymbols(UI_REAGENT_POS);
	QRegExpValidator* validator = new QRegExpValidator(rxSymbols, this);

	ui->select_edit->setValidator(validator);
	ui->select_edit->setAttribute(Qt::WA_InputMethodEnabled, false);
}

void ChReagentUnLoadDlg::InitConnect()
{
	connect(ui->ok_btn, &QPushButton::clicked, this, &ChReagentUnLoadDlg::OnSureClicked);
	connect(ui->cancel_btn, &QPushButton::clicked, this, &ChReagentUnLoadDlg::close);
}

QString ChReagentUnLoadDlg::CheckUnloadStatus(const std::set<int> &setPos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 只处理生化设备
	if (m_spDevInfo->deviceType != ::tf::DeviceType::DEVICE_TYPE_C1000)
	{
		return tr("所选设备非生化设备.");
	}

	// 只有运行和待机能进行卸载
	if (m_spDevInfo->status != tf::DeviceWorkState::DEVICE_STATUS_STANDBY
		&& m_spDevInfo->status != tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
	{
		return tr("只有待机或者运行态可以卸载试剂.");
	}

	// 生化设备增加未屏蔽自动装卸载试剂判断
	ChDeviceOtherInfo chdoi;
	if (!DecodeJson(chdoi, m_spDevInfo->otherInfo))
	{
		return tr("解析设备otherinfo失败.");
	}
	if (!chdoi.loadReagentOnline.enable)
	{
		return tr("该设备已屏蔽自动装卸载试剂.");
	}

	// 查询在线试剂加载
	auto reagentLoader = CommonInformationManager::GetInstance()->\
		getReagentLoaderByDevSn(QString::fromStdString(m_spDevInfo->deviceSN));
	// 试剂加载器状态异常
	if (reagentLoader.loaderStatus != ::ch::tf::LoaderStatus::LOADER_STATUS_NORMAL
		&& reagentLoader.loaderStatus != ::ch::tf::LoaderStatus::LOADER_STATUS_RUNNING)
	{
		return tr("试剂加载器状态异常.");
	}

	// 试剂加载位有空位(有试剂位为空或扫描成功时方可进行试剂卸载)
	bool hasEmptyPos = false;
	for (auto &item : reagentLoader.loaderItems)
	{
		if (item.itemStatus == ::ch::tf::LoaderItemStatus::LOADER_ITEM_STATUS_IDLE
			|| item.itemStatus == ::ch::tf::LoaderItemStatus::LOADER_ITEM_STATUS_WAIT_GRAB)
		{
			hasEmptyPos = true;
			break;
		}
	}
	if (!hasEmptyPos)
	{
		return tr("试剂加载器无空位.");
	}

	// 获取输入位置对于的耗材信息
	std::map<int, ChReagentInfo> mapReagentSupplies = CommonInformationManager::GetInstance()->\
		GetChDiskReagentSupplies(m_spDevInfo->deviceSN, setPos);
	for (const auto &pos : setPos)
	{
		// 已卸载
		if (!mapReagentSupplies.count(pos))
		{
			return tr("输入位置包含空位，位置=%1.").arg(pos);
		}

		// 耗材信息
		auto spyInfo = mapReagentSupplies[pos].supplyInfo;

		// 扫描成功（扫描失败的允许卸载）
		if (spyInfo.suppliesCode > 0)
		{
			// 未放置
			if (spyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_REGISTER)
			{
				return tr("输入位置包含未放置状态，位置=%1.").arg(pos);
			}
			// 卸载中
			else if (spyInfo.placeStatus == tf::PlaceStatus::PLACE_STATUS_UNLOADING)
			{
				return tr("输入位置包含卸载中状态，位置=%1.").arg(pos);
			}
		}

		// 开放试剂
		if (spyInfo.suppliesCode <= ::ch::tf::g_ch_constants.MAX_REAGENT_CODE
			&& spyInfo.suppliesCode >= ::ch::tf::g_ch_constants.MIN_OPEN_REAGENT_CODE)
		{
			return tr("输入位置包含开放试剂，位置=%1.").arg(pos);
		}

		// 使用状态
		auto useStates = spyInfo.usageStatus;
		if (mapReagentSupplies[pos].reagentInfos.size() > 0)
		{
			useStates = mapReagentSupplies[pos].reagentInfos[0].usageStatus;
		}
		// 在用且非待机
		if (useStates == tf::UsageStatus::USAGE_STATUS_CURRENT && m_spDevInfo->status != tf::DeviceWorkState::DEVICE_STATUS_STANDBY)
		{
			return tr("输入位置包含在用瓶，位置=%1.").arg(pos);
		}
	}

	return "";
}

void ChReagentUnLoadDlg::OnSureClicked()
{
	// 获取手动输入的试剂位置
	std::vector<int> selectPositions;
	if (!ParserInputPositions(ui->select_edit->text(), selectPositions))
	{
		ULOG(LOG_ERROR, u8"解析输入的位置信息错误.");
		return;
	}

	// 范围只能是1-65
	auto iter = std::find_if(selectPositions.begin(), selectPositions.end(), [](int pos) {
		return pos > 65 || pos < 1;
	});
	if (iter != selectPositions.end())
	{
		TipDlg(tr("位置范围只能是1-65之间.")).exec();
		return;
	}

	// 检查是否可卸载
	std::set<int> setPositions(selectPositions.begin(), selectPositions.end());
	auto strErorr = CheckUnloadStatus(setPositions);
	if (!strErorr.isEmpty())
	{
		TipDlg(strErorr).exec();
		return;
	}

	// 卸载试剂
	if (!ch::c1005::LogicControlProxy::UnloadSupplies(m_spDevInfo->deviceSN, selectPositions))
	{
		ULOG(LOG_ERROR, "UnloadSupplies Failed!");
	}

	close();
}
