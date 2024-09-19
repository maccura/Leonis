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
/// @file     MaintainDefaultSetDlg.cpp
/// @brief    维护保养-组合设置-默认设置
///
/// @author   8580/GongZhiQiang
/// @date     2023年6月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "MaintainDefaultSetDlg.h"
#include "ui_MaintainDefaultSetDlg.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "maintaindatamng.h"

MaintainDefaultSetDlg::MaintainDefaultSetDlg(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::MaintainDefaultSetDlg();
	ui->setupUi(this);

	// 限制输入
	SetCtrlsRegExp();

	// 关闭按钮
	connect(ui->close_btn, &QPushButton::clicked, this, [=]() {this->close(); });
	// 保存按钮
	connect(ui->save_btn, &QPushButton::clicked, this, &MaintainDefaultSetDlg::OnSaveBtnClicked);
}

MaintainDefaultSetDlg::~MaintainDefaultSetDlg()
{
}

///
/// @brief	根据仪器类型初始化控件
///
/// @param[in]  itemType  维护项类型
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月21日，新建函数
///
void MaintainDefaultSetDlg::InitCtrls(const ::tf::MaintainItemType::type& itemType)
{
	// QLineEdit默认为0,并限制输入
	for (auto edit : findChildren<QLineEdit*>())
	{
		edit->setText("0");
	}

	// QCheckBox默认为false
	for (auto checkBox : findChildren<QCheckBox*>())
	{
		checkBox->setChecked(false);
	}

	// 设置标题
	SetTitleName(ConvertTfEnumToQString(itemType));

	// 切换界面
	switch (itemType)
	{
	case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_ChPipeFill);
	}break;
	case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_ProbeClean);
	}break;
	case ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_EFFLUENT:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_IseDrainage);
	}break;
	case ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_IsePipeFill);
	}break;
	default:
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_Normal);
		break;
	}
}

///
/// @brief	是否包含该含参维护单项
///     
/// @param[in]  itemType  维护项类型
///
/// @return true:包含
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
///
bool MaintainDefaultSetDlg::IsContainParamItem(int itemType)
{
	if (itemType == ::tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION ||
		itemType == ::tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN ||
		itemType == ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_EFFLUENT ||
		itemType == ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION
		)
	{
		return true;
	}
	else
	{
		return false;
	}
}

///
/// @brief	加载维护项数据到控件
///     
/// @param[in]  itemData  修改的维护项信息
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
///
void MaintainDefaultSetDlg::LoadDataToCtrls(const MaintainGroupCfgModel::ItemData & itemData)
{
	// 暂存维护项信息
	modifyItemTemp = itemData;

	// 初始化界面
	InitCtrls(itemData.item.itemType);

	// 设置次数
	ui->execution_times_edit->setText(QString::number(itemData.repetition));

	// 判空
	if (itemData.item.param.empty())
	{
		return;
	}

	// 解析维护项参数
	try
	{
		Document jsonDoc;
		if (!RapidjsonUtil::ParseRapidjson(itemData.item.param, jsonDoc))
		{
			ULOG(LOG_ERROR, "Failed to parse json string: %s", itemData.item.param);
			return;
		}

		// 根据维护类型更新界面参数
		switch (itemData.item.itemType)
		{
			// 比色管路填充
		case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION:
		{
			// 设置参数
			ui->acid_clean_solu_ckbox->setChecked(RapidjsonUtil::GetBoolForKey(jsonDoc, "bEnableAcidCleanSolution"));
			ui->acid_stan_solu_times_edit->setText(QString::number(RapidjsonUtil::GetIntForKey(jsonDoc, "iAcidCleanSolutionTimes")));
			ui->alkalinity_clean_solu_ckbox->setChecked(RapidjsonUtil::GetBoolForKey(jsonDoc, "bEnableAlkalinityCleanSolution"));
			ui->alkalinity_clean_solu_times_edit->setText(QString::number(RapidjsonUtil::GetIntForKey(jsonDoc, "iAlkalinityCleanSolutionTimes")));
		}break;

		// 针清洗
		case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN:
		{
			// 设置参数
			ui->sample_probe_ckbox->setChecked(RapidjsonUtil::GetBoolForKey(jsonDoc, "bEnableSampleProbeClean"));
			ui->sample_probe_clean_times_edit->setText(QString::number(RapidjsonUtil::GetIntForKey(jsonDoc, "iSampleProbeCleanTimes")));
			ui->reagent_probe_ckbox->setChecked(RapidjsonUtil::GetBoolForKey(jsonDoc, "bEnableReagentProbeClean"));
			ui->reagent_probe_clean_times_edit->setText(QString::number(RapidjsonUtil::GetIntForKey(jsonDoc, "iReagentProbeCleanTimes")));

		}break;

		// ISE排液
		case ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_EFFLUENT:
		{
			// 设置参数
			ui->Ise_drainage_times_edit->setText(QString::number(RapidjsonUtil::GetIntForKey(jsonDoc, "iDrainageTimes")));
		}break;

		// ISE管路填充
		case ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION:
		{
			// 设置参数
			ui->Ise_stan_solu_ckbox->setChecked(RapidjsonUtil::GetBoolForKey(jsonDoc, "bEnableStandardSolution"));
			ui->Ise_stan_solu_times_edit->setText(QString::number(RapidjsonUtil::GetIntForKey(jsonDoc, "iStandardSolutionTimes")));
			ui->Ise_buffer_ckbox->setChecked(RapidjsonUtil::GetBoolForKey(jsonDoc, "bEnableBuffer"));
			ui->Ise_buffer_times_edit->setText(QString::number(RapidjsonUtil::GetIntForKey(jsonDoc, "iBufferTimes")));
		}break;

		// 默认为空
		default:
		{}break;
		}
	}
	catch (exception& ex)
	{
		ULOG(LOG_ERROR, "exception: %s", ex.what());
		return;
	}
}

///
/// @brief	加载数据到控件
///     
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
///
MaintainGroupCfgModel::ItemData MaintainDefaultSetDlg::GetModifiedMaintainItem()
{
	return modifyItemTemp;
}

///
/// @brief	保存按钮被点击
///     
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
///
void MaintainDefaultSetDlg::OnSaveBtnClicked()
{
	// 保存重复次数 bug 26157 from 闫铃林
	modifyItemTemp.repetition = ui->execution_times_edit->text().toInt();
	if (modifyItemTemp.item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_AUTO_SHUTDOWN)
	{
		if (modifyItemTemp.repetition != 1)
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("设置失败！维护组合中仪器关机维护项有且仅有一项，请将执行次数设置为1！"), TipDlgType::SINGLE_BUTTON));
			pTipDlg->exec();
			return;
		}
	}

	QString noticeInfo = tr("次数参数输入范围[1-99],时间参数输入范围[1-60]分钟!");

	// 执行次数检查
	if (!MaintainDataMng::GetInstance()->CheckMaintainItemInputParams({ modifyItemTemp.repetition }, {}))
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), noticeInfo, TipDlgType::SINGLE_BUTTON));
		pTipDlg->exec();
		return;
	}

	// 保存参数
	Document doc;
	doc.SetObject();
	Document::AllocatorType& allocator = doc.GetAllocator();

	switch (ui->paramStackedWidget->currentIndex())
	{
		// 比色管路填充
	case ParamPageNum::page_ChPipeFill:
	{
		bool bEACS = ui->acid_clean_solu_ckbox->isChecked();
		int iACST = ui->acid_stan_solu_times_edit->text().toInt();
		bool bEALCS = ui->alkalinity_clean_solu_ckbox->isChecked();
		int iALCST = ui->alkalinity_clean_solu_times_edit->text().toInt();

		// 参数检查
		if (!MaintainDataMng::GetInstance()->CheckMaintainItemInputParams({ iACST , iALCST }, {}))
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), noticeInfo, TipDlgType::SINGLE_BUTTON));
			pTipDlg->exec();
			return ;
		}

		doc.AddMember("bEnableAcidCleanSolution", bEACS, allocator);
		doc.AddMember("iAcidCleanSolutionTimes", iACST, allocator);
		doc.AddMember("bEnableAlkalinityCleanSolution", bEALCS, allocator);
		doc.AddMember("iAlkalinityCleanSolutionTimes", iALCST, allocator);

		modifyItemTemp.item.__set_param(RapidjsonUtil::Rapidjson2String(doc));

	}break;

	// 针清洗
	case ParamPageNum::page_ProbeClean:
	{
		bool bESPC = ui->sample_probe_ckbox->isChecked();
		int iSPCT = ui->sample_probe_clean_times_edit->text().toInt();
		int iSPST = ui->sp_sock_time_edit->text().toInt();
		bool bERPC = ui->reagent_probe_ckbox->isChecked();
		int iRPCT = ui->reagent_probe_clean_times_edit->text().toInt();
		int iRPST = ui->rp_sock_time_edit->text().toInt();

		// 针清洗特殊检查
		if (!bESPC && !bERPC)
		{
			QString info = tr("请至少选择一种针进行清洗!");
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), info, TipDlgType::SINGLE_BUTTON));
			pTipDlg->exec();
			return ;
		}

		// 参数检查
		if (!MaintainDataMng::GetInstance()->CheckMaintainItemInputParams({ iSPCT , iRPCT }, { iSPST ,iRPST }))
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), noticeInfo, TipDlgType::SINGLE_BUTTON));
			pTipDlg->exec();
			return;
		}

		doc.AddMember("bEnableSampleProbeClean", bESPC, allocator);
		doc.AddMember("iSampleProbeCleanTimes", iSPCT, allocator);
		doc.AddMember("bEnableReagentProbeClean", bERPC, allocator);
		doc.AddMember("iReagentProbeCleanTimes", iRPCT, allocator);

		modifyItemTemp.item.__set_param(RapidjsonUtil::Rapidjson2String(doc));

	}break;

	// ISE排液
	case ParamPageNum::page_IseDrainage:
	{
		int iDT = ui->Ise_drainage_times_edit->text().toInt();

		// 参数检查
		if (!MaintainDataMng::GetInstance()->CheckMaintainItemInputParams({ iDT }, {}))
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), noticeInfo, TipDlgType::SINGLE_BUTTON));
			pTipDlg->exec();
			return;
		}

		doc.AddMember("iDrainageTimes", iDT, allocator);

		modifyItemTemp.item.__set_param(RapidjsonUtil::Rapidjson2String(doc));

	}break;

	// ISE管路填充
	case ParamPageNum::page_IsePipeFill:
	{
		bool bES = ui->Ise_stan_solu_ckbox->isChecked();
		int iSST = ui->Ise_stan_solu_times_edit->text().toInt();
		bool bEB = ui->Ise_buffer_ckbox->isChecked();
		int iBT = ui->Ise_buffer_times_edit->text().toInt();

		// 针清洗特殊检查
		if (!bES && !bEB)
		{
			QString info = tr("请至少选择一种填充液进行管路填充!");
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), info, TipDlgType::SINGLE_BUTTON));
			pTipDlg->exec();
			return;
		}

		// 参数检查
		if (!MaintainDataMng::GetInstance()->CheckMaintainItemInputParams({ iSST, iBT }, {}))
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), noticeInfo, TipDlgType::SINGLE_BUTTON));
			pTipDlg->exec();
			return;
		}

		doc.AddMember("bEnableStandardSolution", bES, allocator);
		doc.AddMember("iStandardSolutionTimes", iSST, allocator);
		doc.AddMember("bEnableBuffer", bEB, allocator);
		doc.AddMember("iBufferTimes", iBT, allocator);

		modifyItemTemp.item.__set_param(RapidjsonUtil::Rapidjson2String(doc));

	}break;

	default:
	{}break;
	}
	emit ModifiedMaintainItem();
	this->close();
}

///
/// @brief	为输入框设置正则表达式
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月21日，新建函数
///
void MaintainDefaultSetDlg::SetCtrlsRegExp()
{
	for (auto edit : findChildren<QLineEdit*>())
	{
		edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
	}
}
