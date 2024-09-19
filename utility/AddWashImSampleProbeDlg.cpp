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
/// @file     AddWashImSampleProbeDlg.cpp
/// @brief 	 添加/编辑样本针清洗配置
///
/// @author   7656/zhang.changjiang
/// @date      2023年3月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年3月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "AddWashImSampleProbeDlg.h"
#include "ui_AddWashImSampleProbeDlg.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"

AddWashImSampleProbeDlg::AddWashImSampleProbeDlg(QWidget *parent)
	: BaseDlg(parent)
	,m_id(-1)
	, ui(new Ui::AddWashImSampleProbeDlg)
{
	ui->setupUi(this);

	InitCtrls();
}

AddWashImSampleProbeDlg::~AddWashImSampleProbeDlg()
{

}

void AddWashImSampleProbeDlg::LoadDataToCtrls(wsp::ImCtrlsData data)
{
	ClearCtrls();

	//序号
	ui->num_edit->setText(data.num);
	// 项目名称
	ui->assay_name_cbox->setCurrentText(data.assayName);
	// 样本源
	ui->sample_source_type_cbox->setCurrentText(data.sampleSourceType);
	// 数据库主键
	m_id = data.id;
}

void AddWashImSampleProbeDlg::ClearCtrls()
{
	m_id = -1;
	ui->num_edit->clear();
	ui->sample_source_type_cbox->setCurrentIndex(-1);
	ui->assay_name_cbox->clear();

	// 获取项目信息
	m_mapAssayInfo = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();
	if (m_mapAssayInfo.empty())
	{
		ULOG(LOG_ERROR, "GetChAssayMaps Failed !");
		return;
	}

	// 初始化项目名称下拉框
	for (auto item : m_mapAssayInfo)
	{
		if (item.second->assayCode < ::tf::AssayCodeRange::IM_RANGE_MIN || item.second->assayCode > ::tf::AssayCodeRange::IM_RANGE_MAX)
		{
			continue;
		}

		ui->assay_name_cbox->addItem(QString::fromStdString(item.second->assayName));
	}

	ui->assay_name_cbox->setCurrentIndex(-1);
}

void AddWashImSampleProbeDlg::LoadNumToCtrls(const QString& num)
{
    ui->num_edit->setText(num);
};

void AddWashImSampleProbeDlg::InitCtrls()
{
    connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    //获取样本类型
    SOFTWARE_TYPE devType = CommonInformationManager::GetInstance()->GetSoftWareType();

	// 设置标题
	BaseDlg::SetTitleName(tr("新增"));

	// 获取项目信息
	m_mapAssayInfo = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();
	if (m_mapAssayInfo.empty())
	{
		ULOG(LOG_ERROR, "GetChAssayMaps Failed !");
		return;
	}

	// 初始化项目名称下拉框
	for (auto item : m_mapAssayInfo)
	{
		ui->assay_name_cbox->addItem(QString::fromStdString(item.second->assayName));
	}

	// 初始化样本源下拉框
	for (auto each : ::tf::_SampleSourceType_VALUES_TO_NAMES)
	{
		auto type = (::tf::SampleSourceType::type)each.first;
        if (devType == SOFTWARE_TYPE::IMMUNE)
        {
            if (type == (::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY))
            {
                continue;
            }
        }
		AddTfEnumItemToComBoBox(ui->sample_source_type_cbox, type);
	}
}

bool AddWashImSampleProbeDlg::CheckUserData()
{
	if (ui->assay_name_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择试项目")).exec();
		return false;
	}

	if (ui->sample_source_type_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择样本源")).exec();
		return false;
	}

	return true;
}

bool AddWashImSampleProbeDlg::CheckDataNoRepeat(const im::tf::SpecialWashSampleProbe& swsp)
{
	// 组装查询条件
	im::tf::SpecialWashSampleProbeQueryCond qc;
	qc.__set_assayCode(swsp.assayCode);
	qc.__set_sampleSourceType(swsp.sampleSourceType);

	// 查询
	im::tf::SpecialWashSampleProbeQueryResp qr;
	im::LogicControlProxy::QuerySpecialWashSampleProbe(qr, qc);
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

void AddWashImSampleProbeDlg::OnSaveBtnClicked()
{
	if (!CheckUserData())
	{
		ULOG(LOG_ERROR, "User Input Data Err !");
		return;
	}

	::im::tf::SpecialWashSampleProbe swsp;

	// id
	if (m_id >= 0)
	{
		swsp.__set_id(m_id);
	}

	// 项目名称
	for (auto item : m_mapAssayInfo)
	{
		if (item.second->assayName == ui->assay_name_cbox->currentText().toStdString())
		{
			swsp.__set_assayCode(item.second->assayCode);
			break;
		}
	}

	// 样本源
	swsp.__set_sampleSourceType((::tf::SampleSourceType::type)ui->sample_source_type_cbox->currentIndex());


	if (!CheckDataNoRepeat(swsp))
	{
		return;
	}

	if (BaseDlg::GetTitleName() == tr("新增"))
	{
		swsp.__set_enable(true);
		swsp.__set_deviceType(tf::DeviceType::DEVICE_TYPE_I6000);

		::tf::ResultLong ret;
		im::LogicControlProxy::AddSpecialWashSampleProbe(ret, swsp);
		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "AddSpecialWashSampleProbe Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}
		emit WashImSampleProbeChanged(false, true);
	}
	else
	{
		if (!im::LogicControlProxy::ModifySpecialWashSampleProbe(swsp))
		{
			ULOG(LOG_ERROR, "ModifySpecialWashReagentProbe Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}
		emit WashImSampleProbeChanged(false, false);
	}
	
	close();
}
