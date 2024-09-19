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
/// @file     AddWashSampleProbeDlg.cpp
/// @brief    添加/编辑样本针清洗配置
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
#include "AddWashSampleProbeDlg.h"
#include "ui_AddWashSampleProbeDlg.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"

AddWashSampleProbeDlg::AddWashSampleProbeDlg(QWidget *parent)
	: BaseDlg(parent)
	,m_id(-1)
	, ui(new Ui::AddWashSampleProbeDlg)
{
	ui->setupUi(this);

	InitCtrls();
}

AddWashSampleProbeDlg::~AddWashSampleProbeDlg()
{

}

void AddWashSampleProbeDlg::LoadDataToCtrls(wsp::CtrlsData data)
{
	ClearCtrls();

	//序号
	ui->num_edit->setText(data.num);
	// 项目名称
	ui->assay_name_cbox->setCurrentText(data.assayName);
	// 清洗液
	ui->detergent_type_cbox->setCurrentText(data.detergentType);
	// 样本源
	ui->sample_source_type_cbox->setCurrentText(data.sampleSourceType);
	// 数据库主键
	m_id = data.id;
}

void AddWashSampleProbeDlg::ClearCtrls() 
{
	m_id = -1;
	ui->num_edit->clear();
	ui->sample_source_type_cbox->setCurrentIndex(-1);
	ui->assay_name_cbox->clear();
	ui->detergent_type_cbox->setCurrentIndex(-1);

	// 初始化项目名称下拉框
    std::string ISEName;
	for (auto item : CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap())
	{
		// 排除SIND和ISE项目以及非生化项目
		if (item.second->assayCode < ::tf::AssayCodeRange::CH_RANGE_MIN
			|| item.second->assayCode >= ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN)
		{
			continue;
		}

        if (item.second->assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL
            && item.second->assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_NA)
        {
            ISEName += ISEName.empty() ? item.second->assayName : "、" + item.second->assayName;
        }
        else
        {
	    	ui->assay_name_cbox->addItem(QString::fromStdString(item.second->assayName), item.second->assayCode);
        }
	}

    if (!ISEName.empty())
    {
        ui->assay_name_cbox->addItem(QString::fromStdString(ISEName), ::ise::tf::g_ise_constants.ASSAY_CODE_ISE);
    }

	ui->assay_name_cbox->setCurrentIndex(-1);
}

void AddWashSampleProbeDlg::LoadNumToCtrls(const QString& num)
{
    ui->num_edit->setText(num);
};

void AddWashSampleProbeDlg::InitCtrls()
{
	// 设置标题
	BaseDlg::SetTitleName(tr("新增"));

	// 初始化清洗液下拉框	
    AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_ACIDITY);
    AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_ALKALINITY);
    AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_WATER);
    AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_ULTRASONIC);

	// 初始化样本源下拉框
	for (auto each : ::tf::_SampleSourceType_VALUES_TO_NAMES)
	{
		auto type = (::tf::SampleSourceType::type)each.first;
        if (type == ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER)
        {
            continue;
        }

		AddTfEnumItemToComBoBox(ui->sample_source_type_cbox, type);
	}

    // 将样本源类型中的“其他”放在下拉框的最后
    AddTfEnumItemToComBoBox(ui->sample_source_type_cbox, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);

	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
}

bool AddWashSampleProbeDlg::CheckUserData()
{
    for (const auto& spDev : CommonInformationManager::GetInstance()
        ->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000 }))
    {
        if (devIsRun(*spDev))
        {
            TipDlg(tr("保存失败"), tr("生化仪器运行中，不能修改样本针特殊清洗配置")).exec();
            return false;
        }
    }

	if (ui->assay_name_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择试项目")).exec();
		return false;
	}

	if (ui->detergent_type_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择清洗液")).exec();
		return false;
	}

	if (ui->sample_source_type_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择样本源")).exec();
		return false;
	}

	return true;
}

bool AddWashSampleProbeDlg::CheckDataNoRepeat(const ch::tf::SpecialWashSampleProbe& swsp)
{
	// 组装查询条件
	ch::tf::SpecialWashSampleProbeQueryCond qc;
	qc.__set_assayCode(swsp.assayCode);
	qc.__set_sampleSourceType(swsp.sampleSourceType);

	// 查询
	ch::tf::SpecialWashSampleProbeQueryResp qr;
	ch::LogicControlProxy::QuerySpecialWashSampleProbe(qr, qc);
	if (qr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySpecialWashReagentProbe Failed !");
		TipDlg(tr("保存失败"), tr("保存失败")).exec();
		return false;
	}

	// 检查是否与已有的记录重复
	if (BaseDlg::GetTitleName() == tr("新增"))
	{
		if (qr.lstSpecialWashSampleProbe.size() > 0)
		{
			ULOG(LOG_INFO, "AddWashReagentProbe Is Exist !");
			TipDlg(tr("保存失败"), tr("当前配置已存在")).exec();
			return false;
		}
	}
	else
	{
		for (auto item : qr.lstSpecialWashSampleProbe)
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

void AddWashSampleProbeDlg::OnSaveBtnClicked()
{
	if (!CheckUserData())
	{
		ULOG(LOG_ERROR, "User Input Data Err !");
		return;
	}

	::ch::tf::SpecialWashSampleProbe swsp;

	// id
	if (m_id >= 0)
	{
		swsp.__set_id(m_id);
	}

    QString optStr(tr("样本针特殊清洗"));

    // 项目名称
    swsp.__set_assayCode(ui->assay_name_cbox->currentData().toInt());
    optStr += tr("，项目名称：") + ui->assay_name_cbox->currentText();

    // 样本源
    swsp.__set_sampleSourceType((::tf::SampleSourceType::type)ui->sample_source_type_cbox->currentData().toInt());
    optStr += tr("，样本类型：") + ConvertTfEnumToQString(swsp.sampleSourceType);

	// 清洗液
	swsp.__set_detergentType((::tf::DetergentType::type)ui->detergent_type_cbox->currentData().toInt());
    optStr += tr("，清洗方式：") + ConvertTfEnumToQString(swsp.detergentType);

	if (!CheckDataNoRepeat(swsp))
	{
		return;
	}

	if (GetTitleName() == tr("新增"))
	{
		swsp.__set_enable(true);
		swsp.__set_deviceType(tf::DeviceType::DEVICE_TYPE_C1000);

		::tf::ResultLong ret;
		ch::LogicControlProxy::AddSpecialWashSampleProbe(ret, swsp);
		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "AddSpecialWashSampleProbe Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}

        if (!AddOptLog(::tf::OperationType::ADD, tr("新增") + optStr))
        {
            ULOG(LOG_ERROR, "Add add special wash sample probe operate log failed !");
        }
	}
	else
	{
		if (!ch::LogicControlProxy::ModifySpecialWashSampleProbe(swsp))
		{
			ULOG(LOG_ERROR, "ModifySpecialWashReagentProbe Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}

        if (!AddOptLog(::tf::OperationType::MOD, tr("修改") + optStr))
        {
            ULOG(LOG_ERROR, "Add modify special wash sample probe operate log failed !");
        }
	}

	emit WashSampleProbeChanged();
	close();
}
