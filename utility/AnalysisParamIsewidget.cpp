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
/// @file     AnalysisParamIsewidget.cpp
/// @brief    应用界面->项目设置界面->分析参数界面
///
/// @author   7951/LuoXin
/// @date     2022年12月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年12月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "AnalysisParamIsewidget.h"
#include "ui_AnalysisParamIsewidget.h"
#include "src/common/Mlog/mlog.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

AnalysisParamIsewidget::AnalysisParamIsewidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::AnalysisParamIsewidget)
	, m_bSampleTypeFlag(false)
{
	ui->setupUi(this);

	// 样本源类型下拉框
	connect(ui->SampleTypeCombo, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnSampleSourceChanged(const QString&)));
}

AnalysisParamIsewidget::~AnalysisParamIsewidget()
{
}

bool AnalysisParamIsewidget::LoadAnalysisParam(const AssayListModel::StAssayListRowItem & item)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	Reset();

	// 判断待加载项有效
	if (item.assayCode == INVALID_ASSAY_CODE)
	{
		return false;
	}

	// 查询通用项目信息
	auto &CIM = CommonInformationManager::GetInstance();
	auto GAI = CIM->GetAssayInfo(item.assayCode);
	if (GAI == nullptr)
	{
		ULOG(LOG_ERROR, "GetAssayInfo() Failed!");
		return false;
	}

	// 查询ISE通用项目信息
	auto IseGAI = CIM->GetIseAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model);
	if (IseGAI == nullptr)
	{
		ULOG(LOG_ERROR, "GetIseAssayInfo() Failed!");
		return false;
	}

	// 查询ISE通用项目信息
	auto IseSAIs = CIM->GetIseSpecialAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model);
	if (IseSAIs.empty())
	{
		ULOG(LOG_ERROR, "GetIseSpecialAssayInfo() Failed!");
		return false;
	}

	// 项目名称
	ui->fullNameEdit->setText(QString::fromStdString(GAI->assayFullName));
	//英文简称
	ui->AssayNameEdit->setText(QString::fromStdString(GAI->assayName));
	// 中文名称
	ui->PrintNameEdit->setText(QString::fromStdString(GAI->printName));
	//通道号
	ui->AssayCodeEdit->setText(QString::number(GAI->assayCode));
	//样本类型
	ui->SampleTypeCombo->clear();
	QString text;
	for (auto IseSAI :IseSAIs)
	{
        AddTfEnumItemToComBoBox(ui->SampleTypeCombo, (::tf::SampleSourceType::type)IseSAI->sampleSourceType);

		if (IseSAI->sampleSourceType == item.sampleType)
		{
			text = ConvertTfEnumToQString((::tf::SampleSourceType::type)IseSAI->sampleSourceType);
		}
	}
	ui->SampleTypeCombo->setCurrentText(text);

	// 结果单位下拉框
	ui->ResUnitCombo->clear();
	QString currentText;
	for (const auto& unit : GAI->units)
	{
		ui->ResUnitCombo->addItem(QString::fromStdString(unit.name));
		if (unit.isCurrent)
		{
			currentText = QString::fromStdString(unit.name);
		}
	}
	ui->ResUnitCombo->setCurrentText(currentText);

	// 参数版本
	ui->AssayParamVerEdit->setText(QString::fromStdString(IseGAI->version));
	// 小数位
	ui->DecimalCombo->setCurrentText(QString::number(GAI->decimalPlace));

	m_bSampleTypeFlag = true;

	return true;
}

bool AnalysisParamIsewidget::GetAnalysisParam(tf::GeneralAssayInfo& GAI, ise::tf::GeneralAssayInfo& chGAI, int assayCode)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	// 判断待加载项有效
	if (assayCode == INVALID_ASSAY_CODE)
	{
		return false;
	}

	OnErr(ui->ResUnitCombo->currentIndex() == -1, tr("值错误： 结果单位"));
	//OnErr(ui->DecimalCombo->currentIndex() == -1, tr("值错误： 小数位数"));

	// 英文简称
    // 检查项目名称是否重复
    auto& CIM = CommonInformationManager::GetInstance();
    OnErr(CIM->CheckAssayNameExisted(assayCode, ui->fullNameEdit->text().toStdString()),
        tr("项目名称错误:项目名称为空或已存在"));
    GAI.__set_assayFullName(ui->fullNameEdit->text().toUtf8().data());

    // 检查打印名称是否重复
    OnErr(CIM->CheckAssayNameExisted(assayCode, ui->PrintNameEdit->text().toStdString()),
        tr("打印名称错误:打印名称为空或已存在"));
    GAI.__set_printName(ui->PrintNameEdit->text().toUtf8().data());

    // 检查英文简称是否重复
    OnErr(CIM->CheckAssayNameExisted(assayCode, ui->AssayNameEdit->text().toStdString()),
        tr("简称错误:简称为空或已存在"));
    GAI.__set_assayName(ui->AssayNameEdit->text().toUtf8().data());

	// 单位
	std::vector<::tf::AssayUnit> vecUnit;
	auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(ui->AssayCodeEdit->text().toInt());
	auto unit = spAssayInfo->units;
	for (auto item : unit)
	{
		if (item.isCurrent)
		{
			item.isCurrent = false;
		}

		if (item.name == ui->ResUnitCombo->currentText().toStdString())
		{
			item.isCurrent = true;
		}

		vecUnit.push_back(item);
	}

    if (vecUnit != unit)
    {
        POST_MESSAGE(MSG_ID_ASSAY_UNIT_UPDATE);
    }

	GAI.__set_units(vecUnit);
	
	// 小数位
	GAI.__set_decimalPlace(ui->DecimalCombo->currentText().toInt());

	return true;
}

void AnalysisParamIsewidget::Reset()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	m_bSampleTypeFlag = false;

	for (auto edit : QWidget::findChildren<QLineEdit*>())
	{
		edit->clear();
	}
}

bool AnalysisParamIsewidget::HasUnSaveChangedData()
{
    int code = ui->AssayCodeEdit->text().toInt();
    auto& spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(code);

    if (nullptr == spAssayInfo)
    {
        ULOG(LOG_INFO, "Get Ise assay info by code[%d] failed", code);
        return false;
    }

    if (spAssayInfo->assayName != ui->AssayNameEdit->text().toStdString())
    {
        return true;
    }

    if (spAssayInfo->printName != ui->PrintNameEdit->text().toStdString())
    {
        return true;
    }

    return false;
}

void AnalysisParamIsewidget::OnSampleSourceChanged(const QString &text)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	if (m_bSampleTypeFlag)
	{
		POST_MESSAGE(MSG_ID_ASSAY_SAMPLE_UPDATE, ui->SampleTypeCombo->currentData().toInt());
	}
}
