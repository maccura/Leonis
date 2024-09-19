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
/// @file     AddWashReagentProbeDlg.cpp
/// @brief    添加/编辑试剂针清洗配置
///
/// @author   7951/LuoXin
/// @date     2022年9月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "AddWashReagentProbeDlg.h"
#include "ui_AddWashReagentProbeDlg.h"
#include <QRegExpValidator>
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"

AddWashReagentProbeDlg::AddWashReagentProbeDlg(QWidget *parent)
	: BaseDlg(parent)
	, m_id(-1)
	, ui(new Ui::AddWashReagentProbeDlg)
{
	ui->setupUi(this);

	InitCtrls();
}

AddWashReagentProbeDlg::~AddWashReagentProbeDlg()
{

}

void AddWashReagentProbeDlg::LoadDataToCtrls(wrp::CtrlsData data)
{
	ClearCtrls();

	//序号
	ui->num_edit->setText(data.num);
	// 试剂针
	ui->probe_type_cbox->setCurrentText(data.probe);
	// 污染物
	ui->from_assay_cbox->setCurrentText(data.fromAssay);
	// 受污染物
	ui->to_assay_cbox->setCurrentText(data.toAssay);
	// 清洗液
	ui->detergent_type_cbox->setCurrentText(data.detergentType);
	// 数据库主键
	m_id = data.id;
}

void AddWashReagentProbeDlg::ClearCtrls()
{
	m_id = -1;
	ui->num_edit->clear();
	ui->probe_type_cbox->setCurrentIndex(-1);
	ui->detergent_type_cbox->setCurrentIndex(-1);

	InitAssayCodeCombox();

	ui->to_assay_cbox->setCurrentIndex(-1);
	ui->from_assay_cbox->setCurrentIndex(-1);
}

void AddWashReagentProbeDlg::LoadNumToCtrls(const QString& num)
{
    ui->num_edit->setText(num);
};

void AddWashReagentProbeDlg::InitCtrls()
{
	// 设置标题
	BaseDlg::SetTitleName(tr("新增"));

	InitAssayCodeCombox();

	// 初始化清洗液下拉框	
	AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_ACIDITY);
	AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_ALKALINITY);
    AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_WATER);

	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
}

void AddWashReagentProbeDlg::InitAssayCodeCombox()
{
	ui->from_assay_cbox->clear();
	ui->to_assay_cbox->clear();

	// 获取项目信息
	m_mapAssayInfo = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();
	if (m_mapAssayInfo.empty())
	{
		ULOG(LOG_ERROR, "GetChAssayMaps Failed !");
		return;
	}

	// 初始化污染物下拉框
    std::string ISEName;
	for (auto item : m_mapAssayInfo)
	{
		// 排除ISE以及非生化项目
		if ((item.second->assayCode > ch::tf::g_ch_constants.ASSAY_CODE_SIND
			&& item.second->assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
			|| item.second->assayCode < ::tf::AssayCodeRange::CH_RANGE_MIN
			|| item.second->assayCode > ::tf::AssayCodeRange::CH_RANGE_MAX)
		{
			continue;
		}

        ui->from_assay_cbox->addItem(QString::fromStdString(item.second->assayName), item.second->assayCode);
        ui->to_assay_cbox->addItem(QString::fromStdString(item.second->assayName), item.second->assayCode);       
	}
}

bool AddWashReagentProbeDlg::CheckUserData()
{
    for (const auto& spDev : CommonInformationManager::GetInstance()
        ->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000 }))
    {
        if (devIsRun(*spDev))
        {
            TipDlg(tr("保存失败"), tr("生化仪器运行中，不能修改试剂针特殊清洗配置")).exec();
            return false;
        }
    }

	if (ui->probe_type_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择试剂针")).exec();
		return false;
	}

	if (ui->from_assay_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择污染物")).exec();
		return false;
	}

	if (ui->to_assay_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择受污染物")).exec();
		return false;
	}

	if (ui->to_assay_cbox->currentIndex() == ui->from_assay_cbox->currentIndex())
	{
		TipDlg(tr("保存失败"), tr("污染物不能和受污染物相同")).exec();
		return false;
	}

	if (ui->detergent_type_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择清洗液")).exec();
		return false;
	}

	return true;
}

bool AddWashReagentProbeDlg::CheckDataNoRepeat(const ch::tf::SpecialWashReagentProbe& swrp)
{
	// 组装查询条件
	ch::tf::SpecialWashReagentProbeQueryCond qc;
	qc.__set_probeNum(swrp.probeNum);
	qc.__set_fromAssay(swrp.fromAssay);
	qc.__set_toAssay(swrp.toAssay);

	// 查询
	ch::tf::SpecialWashReagentProbeQueryResp qr;
	ch::LogicControlProxy::QuerySpecialWashReagentProbe(qr, qc);
	if (qr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySpecialWashReagentProbe Failed !");
		TipDlg(tr("保存失败"), tr("保存失败")).exec();
		return false;
	}

	// 检查是否与已有的记录重复
	if (BaseDlg::GetTitleName() == tr("新增"))
	{
		if (qr.lstSpecialWashItems.size() > 0)
		{
			ULOG(LOG_INFO, "AddWashReagentProbe Is Exist !");
			TipDlg(tr("保存失败"), tr("当前配置已存在")).exec();
			return false;
		}
	}
	else
	{
		for (auto item : qr.lstSpecialWashItems)
		{
			if (m_id != item.id)
			{
				ULOG(LOG_INFO, "ModifyWashReagentProbe Is Exist !");
				TipDlg(tr("保存失败"), tr("当前配置已存在")).exec();
				return false;
			}
		}
	}

	return true;
}

void AddWashReagentProbeDlg::OnSaveBtnClicked()
{
	if (!CheckUserData())
	{
		ULOG(LOG_ERROR, "User Input Data Err !");
		return;
	}

	::ch::tf::SpecialWashReagentProbe swrp;

	// id
	if (m_id >= 0)
	{
		swrp.__set_id(m_id);
	}

    // 操作记录
    QString optStr(tr("试剂针特殊清洗"));

	// 试剂针
    int probeNum = ui->probe_type_cbox->currentIndex() + 1;
	swrp.__set_probeNum(probeNum);

    // 阶段
    swrp.__set_fromReagentStage((::ch::tf::ReagentStage::type)probeNum);
    swrp.__set_toReagentStage((::ch::tf::ReagentStage::type)probeNum);
    optStr += tr("，试剂针：") + ConvertTfEnumToQString(swrp.fromReagentStage);

	// 污染物
    swrp.__set_fromAssay(ui->from_assay_cbox->currentData().toInt());
    optStr += tr(",污染物：") + ui->from_assay_cbox->currentText();

	// 受污染物
    swrp.__set_toAssay(ui->to_assay_cbox->currentData().toInt());
    optStr += tr(",受污染物：") + ui->to_assay_cbox->currentText();

	// 清洗液
	swrp.__set_detergentType((::tf::DetergentType::type)ui->detergent_type_cbox->currentData().toInt());
    optStr += tr(",清洗方式：") + ConvertTfEnumToQString(swrp.detergentType);

	if (!CheckDataNoRepeat(swrp))
	{
		return;
	}

	if (GetTitleName() == tr("新增"))
	{
		swrp.__set_enable(true);
		swrp.__set_deviceType(tf::DeviceType::DEVICE_TYPE_C1000);

		::tf::ResultLong ret;
		ch::LogicControlProxy::AddSpecialWashReagentProbe(ret, swrp);
		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "AddSpecialWashReagentProbe Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}

        // 添加操作日志
        if (!AddOptLog(::tf::OperationType::ADD, tr("新增") + optStr))
        {
            ULOG(LOG_ERROR, "Add add special wash sample probe operate log failed !");
        }
	}
	else
	{
		if (!ch::LogicControlProxy::ModifySpecialWashReagentProbe(swrp))
		{
			ULOG(LOG_ERROR, "ModifySpecialWashReagentProbe Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}

        if (!AddOptLog(::tf::OperationType::MOD, tr("修改") + optStr))
        {
            ULOG(LOG_ERROR, "Add modify special wash reagent probe operate log failed !");
        }
	}

	emit WashReagentProbeChanged();
	close();
}
