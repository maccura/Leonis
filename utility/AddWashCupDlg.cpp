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
/// @file     AddWashCupDlg.cpp
/// @brief    添加/编辑反应杯清洗配置
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
#include "AddWashCupDlg.h"
#include "ui_AddWashCupDlg.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"

AddWashCupDlg::AddWashCupDlg(QWidget *parent)
	: BaseDlg(parent)
	,m_id(-1)
	, ui(new Ui::AddWashCupDlg)
{
	ui->setupUi(this);

	InitCtrls();
}

AddWashCupDlg::~AddWashCupDlg()
{

}

void AddWashCupDlg::LoadDataToCtrls(wc::CtrlsData data)
{
	ClearCtrls();

	// id
	m_id = data.id;
	// 序号
	ui->num_edit->setText(data.num);
	// 项目名称
	ui->assay_cbox->setCurrentText(data.assayName);
	// 清洗剂
	ui->detergent_type_cbox->setCurrentText(data.detergent);
}

void AddWashCupDlg::ClearCtrls()
{
	m_id = -1;
	ui->num_edit->clear();
	ui->detergent_type_cbox->setCurrentIndex(-1);

	InitAssayCodeCombox();
}

void AddWashCupDlg::LoadNumToCtrls(const QString& num)
{
    ui->num_edit->setText(num);
};

void AddWashCupDlg::InitCtrls()
{
	// 设置标题
	BaseDlg::SetTitleName(tr("新增"));

	InitAssayCodeCombox();

	// 初始化清洗液下拉框	
    AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_ACIDITY);
    AddTfEnumItemToComBoBox(ui->detergent_type_cbox, ::tf::DetergentType::DET_TYPE_ALKALINITY);

	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
}

void AddWashCupDlg::InitAssayCodeCombox()
{
	ui->assay_cbox->clear();

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
        // 排除ISE以及非生化项目
        if ((item.second->assayCode > ch::tf::g_ch_constants.ASSAY_CODE_SIND
            && item.second->assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
            || item.second->assayCode < ::tf::AssayCodeRange::CH_RANGE_MIN
            || item.second->assayCode > ::tf::AssayCodeRange::CH_RANGE_MAX)
        {
            continue;
        }

        ui->assay_cbox->addItem(QString::fromStdString(item.second->assayName), item.second->assayCode);        
    }

    ui->assay_cbox->setCurrentIndex(-1);
}

bool AddWashCupDlg::CheckUserData()
{
    for (const auto& spDev : CommonInformationManager::GetInstance()
            ->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000 }))
    {
        if (devIsRun(*spDev))
        {
            TipDlg(tr("保存失败"), tr("生化仪器运行中，不能修改反应杯特殊清洗配置")).exec();
            return false;
        }
    }

	if (ui->assay_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择项目")).exec();
		return false;
	}

	if (ui->detergent_type_cbox->currentIndex() == -1)
	{
		TipDlg(tr("保存失败"), tr("未选择R1清洗剂")).exec();
		return false;
	}

	return true;
}

bool AddWashCupDlg::CheckDataNoRepeat(ch::tf::SpecialWashCup swc)
{
	// 组装查询条件
	ch::tf::SpecialWashCupQueryCond qc;
	qc.__set_assayCode(swc.assayCode);

	// 查询
	ch::tf::SpecialWashCupQueryResp qr;
	ch::LogicControlProxy::QuerySpecialWashCup(qr, qc);
	if (qr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySpecialWashReagentProbe Failed !");
		TipDlg(tr("保存失败"), tr("保存失败")).exec();
		return false;
	}

	// 检查是否与已有的记录重复
	if (BaseDlg::GetTitleName() == tr("新增"))
	{
		if (qr.lstSpecialWashCup.size() > 0)
		{
			ULOG(LOG_INFO, "AddWashReagentProbe Is Exist !");
			TipDlg(tr("保存失败"), tr("当前配置已存在")).exec();
			return false;
		}
	}
	else
	{
		for (auto item : qr.lstSpecialWashCup)
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

void AddWashCupDlg::OnSaveBtnClicked()
{
	if (!CheckUserData())
	{
		ULOG(LOG_ERROR, "User Input Data Error !");
		return;
	}

	::ch::tf::SpecialWashCup swc;

	// id
	if (m_id >= 0)
	{
		swc.__set_id(m_id);
	}

    QString optStr(tr("反应杯特殊清洗"));

	// 项目编号
    swc.__set_assayCode(ui->assay_cbox->currentData().toInt());
    optStr += (tr("，项目：") + ui->assay_cbox->currentText());

	// R1清洗剂
    ::ch::tf::WashItem wi;
    wi.__set_detergentType((::tf::DetergentType::type)ui->detergent_type_cbox->currentData().toInt());
    optStr += tr("清洗方式：") + ConvertTfEnumToQString(wi.detergentType);
    swc.__set_washItem(std::move(wi));

	if (!CheckDataNoRepeat(swc))
	{
		return;
	}

	if (GetTitleName() == tr("新增"))
	{
		swc.__set_enable(true);
		swc.__set_deviceType(tf::DeviceType::DEVICE_TYPE_C1000);

		::tf::ResultLong ret;
		ch::LogicControlProxy::AddSpecialWashCup(ret, swc);
		if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
		{
			ULOG(LOG_ERROR, "AddSpecialWashCup Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}

        if (!AddOptLog(::tf::OperationType::ADD, tr("新增") + optStr))
        {
            ULOG(LOG_ERROR, "Add add special wash cup operate log failed !");
        }
	}
	else
	{
		if (!ch::LogicControlProxy::ModifySpecialWashCup(swc))
		{
			ULOG(LOG_ERROR, "ModifySpecialWashCup Failed !");
			TipDlg(tr("保存失败"), tr("保存失败")).exec();
			return;
		}

        if (!AddOptLog(::tf::OperationType::MOD, tr("修改") + optStr))
        {
            ULOG(LOG_ERROR, "Add add special wash cup operate log failed !");
        }
	}

	emit WashCupChanged();
	close();
}
