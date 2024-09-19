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
/// @file     analysisparamwidget.cpp
/// @brief    应用界面->项目设置界面->分析参数界面
///
/// @author   4170/TangChuXian
/// @date     2020年8月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "analysisparamwidget.h"
#include <float.h>
#include "ui_analysisparamwidget.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_types.h"
#include "thrift/ch/ChLogicControlProxy.h"

#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/QFilterComboBox.h"

#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

#define REF_TABLE_DEFAULT_ROW_CNT (5)	 // 参考区间表默认行数
#define REF_TABLE_DEFAULT_COLUMN_CNT (5) // 参考区间表默认列数


// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

AnalysisParamWidget::AnalysisParamWidget(QWidget *parent)
	: QWidget(parent),
	  m_bInit(false),
      m_bVersionFlag(false),
      m_bSampleTypeFlag(false)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	ui = new Ui::AnalysisParamWidget();
	ui->setupUi(this);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

    // 注册新增项目处理函数
    REGISTER_HANDLER(MSG_ID_UTILITY_ADD_ASSAY, this, OnAddAssay);
}

AnalysisParamWidget::~AnalysisParamWidget()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	delete ui;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void AnalysisParamWidget::InitAfterShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 初始化子控件
	InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void AnalysisParamWidget::InitChildCtrl()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初始化分析方法下拉框
    ui->AnalysisMethodCombo->setView(new QTipListView(ui->AnalysisMethodCombo));

	for (auto each : ch::tf::_AnalysisMethod_VALUES_TO_NAMES)
	{
		auto type = (::ch::tf::AnalysisMethod::type)each.first;
		AddTfEnumItemToComBoBox(ui->AnalysisMethodCombo, type);
	}

    connect(ui->AnalysisMethodCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [&](int index) 
    { 
        if (ui->AnalysisMethodCombo->currentData().toInt() == ::ch::tf::AnalysisMethod::ANALYSIS_METHOD_RATE_A_SAMPLE_BLANK)
        {
            ui->AnalysisMethodCombo->setToolTip(ui->AnalysisMethodCombo->currentText());
        }
        else
        {
            ui->AnalysisMethodCombo->setToolTip("");
        }
    });

	// 初始化反应时间下拉框
    ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_3MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_3MIN);
	ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_4MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_4MIN);
    ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_5MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_5MIN);
	ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_6MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_6MIN);
	ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_7MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_7MIN);
	ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_8MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_8MIN);
	ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_9MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_9MIN);
    ui->ReactionTimeCombo->addItem(ConvertTfEnumToQString(::ch::tf::ReactionTime::REACTION_TIME_10MIN)
		, ::ch::tf::ReactionTime::REACTION_TIME_10MIN);

	// 初始化测光点是否包含前一个点
	ui->take_point->addItem("1", false);
	ui->take_point->addItem("2", true);

	// 初始化主、副波长下拉框
	for (auto each : ch::tf::_WaveLength_VALUES_TO_NAMES)
	{
		auto type = (::ch::tf::WaveLength::type)each.first;

        // 排除散射
        if (type == ::ch::tf::WaveLength::WAVE_LENGTH_650)
        {
            continue;
        }

		AddTfEnumItemToComBoBox(ui->PrimaryOSVCombo, type);
		AddTfEnumItemToComBoBox(ui->SecondaryOSVCombo, type);
	}

	// 初始化保留小数位数下拉框
	for (auto i : {0, 1, 2, 3, 4})
	{
		ui->DecimalCombo->addItem(QString::number(i), i);
	}

	// 初始化常量、增量、减量样本的稀释搅拌力度
	auto combos = {ui->MixstdLevel, ui->MixincLevel, ui->MixdecLevel, ui->dilution_mixin_level };
	for (auto combo : combos)
	{
		for (int i = 0; i <= 16; i++)
		{
			combo->addItem(QString::number(i), i);
		}
	}

	// 初始化R1、R2试剂的富余量和搅拌力度
	std::list<std::pair<QComboBox*, QComboBox*>> combPairs = {
		{ui->DummyR1Combo, ui->MixSpeedR1Combo},
		{ui->DummyR3Combo, ui->MixSpeedR3Combo}};
	for (auto comboPair : combPairs)
	{
		for (auto i : {0, 1, 2})
		{
			comboPair.first->addItem(QString::number(i), i);
		}

		for (int i = 0; i <= 16; i++)
		{
			comboPair.second->addItem(QString::number(i), i);
		}
	}

	// 初始化前代、吸光度差界限值参数下拉框
	std::list<std::pair<QComboBox *, QComboBox *>> combPairs2 = {
		{            nullptr, ui->ProzoneSideCombo},
		{ui->M1PrePointCombo, ui->M1InspectSideCombo},
		{ui->M2PrePointCombo, ui->M2InspectSideCombo}};

	for (auto combPair : combPairs2)
	{
		for (auto i : {1, 2})
		{
			if (combPair.first != nullptr)
			{
				combPair.first->addItem(QString::number(i), i);
			}

		}

		for (auto each : ch::tf::_AssayParamSide_VALUES_TO_NAMES)
		{
			auto type = (::ch::tf::AssayParamSide::type)each.first;
			AddTfEnumItemToComBoBox(combPair.second, type);
		}
	}

	// 初始化反应界限吸光度下拉框
	for (auto each : ch::tf::_AssayParamSign_VALUES_TO_NAMES)
	{
		auto type = (::ch::tf::AssayParamSign::type)each.first;
		AddTfEnumItemToComBoBox(ui->ReactionSignCombo, type);
	}

	// 初始化吸光度检查类型下拉框
	auto combs2 = {ui->M1InspectWaveTypeCombo, ui->M2InspectWaveTypeCombo};
	for (auto comb : combs2)
	{
		for (auto each : ch::tf::_AssayParamWaveSel_VALUES_TO_NAMES)
		{
			auto type = (::ch::tf::AssayParamWaveSel::type)each.first;
			AddTfEnumItemToComBoBox(comb, type);
		}
	}

	// 初始化反应杯清洗下拉框
	for (auto each : ch::tf::_CupDetergentType_VALUES_TO_NAMES)
	{
		auto type = (::ch::tf::CupDetergentType::type)each.first;
		AddTfEnumItemToComBoBox(ui->ReactionCupWashCombo, type);
	}

    // 初始化控件列表
    m_sampCtrlsp =
    {
        { ui->SuckVolStdOrgEdit, ui->SuckVolStdMixEdit, ui->SuckVolStdDilluteEdit, ui->MixstdLevel },
        { ui->SuckVolDecOrgEdit, ui->SuckVolDecMixEdit, ui->SuckVolDecDilluteEdit, ui->MixdecLevel },
        { ui->SuckVolIncOrgEdit, ui->SuckVolIncMixEdit, ui->SuckVolIncDilluteEdit, ui->MixincLevel },
    };

    m_reagentCtrls =
    {
        { ui->RegentVolR1Edit, ui->RegentR1DilluteEdit, ui->DummyR1Combo, ui->MixSpeedR1Combo},
        { ui->RegentVolR3Edit, ui->RegentR3DilluteEdit, ui->DummyR3Combo, ui->MixSpeedR3Combo},
    };

    m_checkParamsCtrls =
    {
        { { ui->M1InspectWaveTypeCombo, ui->M1PrePointCombo, ui->M1InspectSideCombo },
        {   ui->M1InspectLowerLMTLineEdit,ui->M1InspectUpperLMTLineEdit,ui->M1InspectmLine_Edit,
            ui->M1InspectnLine_Edit,ui->M1InspectpLine_Edit,ui->M1InspectqLine_Edit,
            ui->M1InspectAbsPreLineEdit,ui->M1InspectAbsLatterLineEdit } },

        { { ui->M2InspectWaveTypeCombo, ui->M2PrePointCombo, ui->M2InspectSideCombo },
        {   ui->M2InspectLowerLMTLineEdit, ui->M2InspectUpperLMTLineEdit,ui->M2InspectmLine_Edit,
            ui->M2InspectnLine_Edit,ui->M2InspectpLine_Edit,ui->M2InspectqLine_Edit,
            ui->M2InspectAbsPreLineEdit,ui->M2InspectAbsLatterLineEdit } },
    };

	// 输入框限制
	SetCtrlsRegExp();

	// 设置稀释液选择的RadioButton互斥
	QButtonGroup* group = new QButtonGroup(this);
	group->addButton(ui->water_radio_Button);
	group->addButton(ui->dilution_radio_Button);
	group->setExclusive(true);

    // 稀释液
    connect(ui->dilution_radio_Button, &QRadioButton::toggled, [&](bool checked) 
    {
        int code = ui->AssayCodeEdit->text().toInt();
        bool isOpenAssay = (code < ::tf::AssayCodeRange::CH_RANGE_MIN || code >= ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN);
        
        if (isOpenAssay || UserInfoManager::GetInstance()->IsPermisson(PSM_DILUTION))
        {
            ui->water_radio_Button->setEnabled(true);
            ui->dilution_radio_Button->setEnabled(true);
            ui->dilution_mixin_level->setEnabled(true);
            ui->dilution_line_edit->setEnabled(ui->dilution_radio_Button->isChecked());
        }
        else
        {
            ui->water_radio_Button->setEnabled(false);
            ui->dilution_radio_Button->setEnabled(false);
            ui->dilution_line_edit->setEnabled(false);
            ui->dilution_mixin_level->setEnabled(false);
        }

        if (ui->water_radio_Button->isChecked())
        {
            ui->dilution_line_edit->clear();
        }
    });

	// 样本源类型下拉框
	connect(ui->SampleTypeCombo, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
        this, &AnalysisParamWidget::OnSampleSourceChanged);

    connect(ui->AssayParamVerCombo, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
        [=](const QString &text) 
    { 
        if (m_bVersionFlag)
        {
            POST_MESSAGE(MSG_ID_UTILITY_VERSION_UPDATE, text.toInt()); 
        }    
    });

    // 通知校准参数页面更新默认样本量
    connect(ui->SuckVolStdOrgEdit, &QLineEdit::textChanged, this, [&](const QString& vol)
    {
        emit SampleVolChanged(vol.toDouble());
    });
}

void AnalysisParamWidget::SetCtrlsRegExp()
{
    // 样本量
    ui->SuckVolStdOrgEdit->setValidator(new QDoubleValidator(1.0, 25.0, 1, ui->SuckVolStdOrgEdit));
    ui->SuckVolDecOrgEdit->setValidator(new QDoubleValidator(1.0, 25.0, 1, ui->SuckVolDecOrgEdit));
    ui->SuckVolIncOrgEdit->setValidator(new QDoubleValidator(1.0, 25.0, 1, ui->SuckVolIncOrgEdit));

    // 稀释样本量
    ui->SuckVolStdMixEdit->setValidator(new QDoubleValidator(1.0, 25.0, 1, ui->SuckVolStdMixEdit));
    ui->SuckVolDecMixEdit->setValidator(new QDoubleValidator(1.0, 25.0, 1, ui->SuckVolDecMixEdit));
    ui->SuckVolIncMixEdit->setValidator(new QDoubleValidator(1.0, 25.0, 1, ui->SuckVolIncMixEdit));

    // 稀释液量
    ui->SuckVolStdDilluteEdit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_REACTION_MAX_VOL / 10, 1, ui->SuckVolStdDilluteEdit));
    ui->SuckVolDecDilluteEdit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_REACTION_MAX_VOL / 10, 1, ui->SuckVolDecDilluteEdit));
    ui->SuckVolIncDilluteEdit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_REACTION_MAX_VOL / 10, 1, ui->SuckVolIncDilluteEdit));

    // 试剂量支持0.5的步进，bug3232
    ui->RegentVolR1Edit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_PROBE_MAX_VOL / 10, 1, ui->RegentVolR1Edit));
    ui->RegentVolR3Edit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_PROBE_MAX_VOL / 10, 1, ui->RegentVolR3Edit));

    // 稀释液量
    ui->RegentR1DilluteEdit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_REACTION_MAX_VOL / 10, 1, ui->RegentR1DilluteEdit));
    ui->RegentR3DilluteEdit->setValidator(new QDoubleValidator(
        REAGENT_PROBE_MIN_VOL / 10, REAGENT_REACTION_MAX_VOL / 10, 1, ui->RegentR3DilluteEdit));

    // 开瓶有效期[1, 9999],bug2858
    ui->openBottleValidityEdit->setValidator(new QIntValidator(1, 9999, ui->openBottleValidityEdit));

	for (auto edit : findChildren<QLineEdit*>())
	{

		// 限制输入整数
		if (edit->objectName().contains("Abs") || edit->objectName().startsWith("LinearDiagLMTLineEdit"))
		{
			edit->setValidator(new QIntValidator(0, UI_INT_MAX_UNM,edit));
		}

		// 限制输入正整数
		if (edit->objectName().startsWith("LinearDiag") || edit->objectName().startsWith("setValue") 
            || edit->objectName().endsWith("Code") || edit->objectName().startsWith("ReactionLmtLineEdit"))
		{
			edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
		}

        // 测光点限制输入1-34
        if (edit->objectName().endsWith("_Edit"))
        {
            edit->setValidator(new QIntValidator(1, 34, edit));
        }

        if (edit->objectName().endsWith("LMTLineEdit"))
        {
            edit->setValidator(new QIntValidator(edit));
        }
	}
}

bool AnalysisParamWidget::CheckUserInputData(int code)
{
    const auto& CIM = CommonInformationManager::GetInstance();

    // 去除空格
    QString tempStr = ui->fullNameEdit->text().trimmed();
    // 检查项目名称是否重复
    OnErr(tempStr.isEmpty() || CIM->CheckAssayNameExisted(code, tempStr.toStdString()),
        tr("项目名称错误:项目名称为空或与其他的项目的名称重复"));

    // 去除空格
    tempStr = ui->PrintNameEdit->text().trimmed();
    // 检查打印名称是否重复
    OnErr(tempStr.isEmpty() || CIM->CheckAssayNameExisted(code, tempStr.toStdString()),
        tr("打印名称错误:打印名称为空或与其他项目的名称重复"));

    // 去除空格
    tempStr = ui->AssayNameEdit->text().trimmed();
    // 检查英文简称是否重复
    OnErr(tempStr.isEmpty() || CIM->CheckAssayNameExisted(code, tempStr.toStdString()),
        tr("简称错误:简称为空或与其他的项目的名称重复"));

    // 分析方法
    OnErr(ui->AnalysisMethodCombo->currentIndex() == -1, tr("分析方法错误:分析方法不能为空"));

    // 五分钟项目禁止设置R2的量
    int emReactionTime = ui->ReactionTimeCombo->currentData().toInt();
    OnErr((emReactionTime == ::ch::tf::ReactionTime::REACTION_TIME_5MIN
		|| emReactionTime == ::ch::tf::ReactionTime::REACTION_TIME_4MIN
        || emReactionTime == ::ch::tf::ReactionTime::REACTION_TIME_3MIN)
        && !ui->RegentVolR3Edit->text().isEmpty(), tr("反应时间小于等于五分钟不能设置R2的试剂量"));

    // 测光点
    int m = ui->m_Edit->text().toInt();
    int n = ui->n_Edit->text().toInt();
    int p = ui->p_Edit->text().toInt();
    int q = ui->q_Edit->text().toInt();

    int halfPoint = C1005_METER_POINT_MAX / 2;
    OnErr(ui->ReactionTimeCombo->currentData().toInt() == ::ch::tf::ReactionTime::REACTION_TIME_5MIN &&
        (m > halfPoint || n > halfPoint || p > halfPoint || q > halfPoint),
        tr("测光点设置错误：反应时间设置为五分钟时，1≤测光点≤") + QString::number(halfPoint));

    OnErr(ui->take_point->currentData().toBool() && m < 2,
        tr("测光点设置错误：测光点浓度计算公式选择2时，第一个光点必须大于1"));

    switch (ui->AnalysisMethodCombo->currentData().toInt())
    {
        // 1点法 1≤m≤34
        case ::ch::tf::AnalysisMethod::ANALYSIS_METHOD_1POINT :
            OnErr(m < C1005_METER_POINT_MIN || m > C1005_METER_POINT_MAX || n!=0||p!=0||q!=0,
                tr("测光点设置错误，1点法：[m][0][0][0],1≤m≤34"));
            break;
            // 2点速率法\2点终点法 1≤m＜n≤34
        case ::ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_END:
        case ::ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_RATE:
            OnErr(m < C1005_METER_POINT_MIN || m > C1005_METER_POINT_MAX || n <= m || p != 0 || q != 0,
                tr("测光点设置错误，二点速率法\\二点终点法：[m][n][0][0],1≤m＜n≤34"));
            break;
            // 速率A法 1≤m＜n≤34，m+2<n
        case ::ch::tf::AnalysisMethod::ANALYSIS_METHOD_RATE_A:
            OnErr(m < C1005_METER_POINT_MIN || m > C1005_METER_POINT_MAX || n < m || m+2>=n || p != 0 || q != 0,
                tr("测光点设置错误，速率A法：[m][n][0][0],1≤m＜n≤34，m+2<n"));
            break;
            // 速率A带样品空白修正法1≤m＜n＜p＜q≤34，m+2＜n,p+2＜q
        case ::ch::tf::AnalysisMethod::ANALYSIS_METHOD_RATE_A_SAMPLE_BLANK:
            OnErr(!(C1005_METER_POINT_MIN <= m && m<n && n<p && p<q && q <= C1005_METER_POINT_MAX) || (m+2 >= n) || (p+2 >= q),
                tr("测光点设置错误，速率A带样品空白修正法：1≤m＜n＜p＜q≤34，m+2＜n,p+2＜q"));
            break;
        default:
            break;
    } 

    OnErr(ui->take_point->currentIndex() == -1, tr("未指定：测光点->是否将前一点纳入吸光度计算"));

    // 主波长
    OnErr(ui->PrimaryOSVCombo->currentText().isEmpty(), tr("主波长不能为空"));

    // 波长
    OnErr(ui->PrimaryOSVCombo->currentText() == ui->SecondaryOSVCombo->currentText(), tr("主波长和次波长不可相同"));

    // 小数位
    OnErr(ui->DecimalCombo->currentIndex() == -1, tr("值错误：未设置小数位"));

    // 样本量常量
    double tempData = ui->SuckVolStdOrgEdit->text().toDouble() * 10;
    OnErr(tempData < SAMPLE_PROBE_MIN_VOL || tempData > SAMPLE_PROBE_MAX_VOL, tr("值错误：%1≤样本量常量≤%2")
            .arg(QString::number(SAMPLE_PROBE_MIN_VOL / 10)).arg(QString::number(SAMPLE_PROBE_MAX_VOL / 10)));
    // 样本量减量
    tempData = ui->SuckVolDecOrgEdit->text().toDouble() * 10;
    OnErr((tempData < SAMPLE_PROBE_MIN_VOL || tempData > SAMPLE_PROBE_MAX_VOL) && !ui->SuckVolDecOrgEdit->text().isEmpty(),
        tr("值错误：%1≤样本量减量≤%2").arg(QString::number(SAMPLE_PROBE_MIN_VOL / 10)).arg(QString::number(SAMPLE_PROBE_MAX_VOL / 10)));
    // 样本量增量
    tempData = ui->SuckVolIncOrgEdit->text().toDouble() * 10;
    OnErr((tempData < SAMPLE_PROBE_MIN_VOL || tempData > SAMPLE_PROBE_MAX_VOL) && !ui->SuckVolIncOrgEdit->text().isEmpty(),
        tr("值错误：%1≤样本量增量≤%2").arg(QString::number(SAMPLE_PROBE_MIN_VOL / 10)).arg(QString::number(SAMPLE_PROBE_MAX_VOL / 10)));
    
    // 稀释样本量常量
    tempData = ui->SuckVolStdMixEdit->text().toDouble() * 10;
    OnErr((tempData < SAMPLE_PROBE_MIN_VOL || tempData > SAMPLE_PROBE_MAX_VOL) && !ui->SuckVolStdMixEdit->text().isEmpty(),
        tr("值错误：%1≤稀释样本量常量≤%2").arg(QString::number(SAMPLE_PROBE_MIN_VOL / 10)).arg(QString::number(SAMPLE_PROBE_MAX_VOL / 10)));
    // 稀释样本量减量
    tempData = ui->SuckVolDecMixEdit->text().toDouble() * 10;
    OnErr((tempData < SAMPLE_PROBE_MIN_VOL || tempData > SAMPLE_PROBE_MAX_VOL) && !ui->SuckVolDecMixEdit->text().isEmpty(),
        tr("值错误：%1≤稀释样本量减量≤%2").arg(QString::number(SAMPLE_PROBE_MIN_VOL / 10)).arg(QString::number(SAMPLE_PROBE_MAX_VOL / 10)));
    // 稀释样本量增量
    tempData = ui->SuckVolIncMixEdit->text().toDouble() * 10;
    OnErr((tempData < SAMPLE_PROBE_MIN_VOL || tempData > SAMPLE_PROBE_MAX_VOL) && !ui->SuckVolIncMixEdit->text().isEmpty(),
        tr("值错误：%1≤稀释样本量增量≤%2").arg(QString::number(SAMPLE_PROBE_MIN_VOL / 10)).arg(QString::number(SAMPLE_PROBE_MAX_VOL / 10)));
   
    // 稀释液量常量
    tempData = ui->SuckVolStdDilluteEdit->text().toDouble() * 10;
    OnErr((tempData < REAGENT_PROBE_MIN_VOL || tempData > REAGENT_PROBE_MAX_VOL) && !ui->SuckVolStdDilluteEdit->text().isEmpty(),
        tr("值错误：%1≤稀释液量常量≤%2，步进值0.5").arg(QString::number(REAGENT_PROBE_MIN_VOL / 10)).arg(QString::number(REAGENT_PROBE_MAX_VOL / 10)));
    // 稀释液量减量
    tempData = ui->SuckVolDecDilluteEdit->text().toDouble() * 10;
    OnErr((tempData < REAGENT_PROBE_MIN_VOL || tempData > REAGENT_PROBE_MAX_VOL) && !ui->SuckVolDecDilluteEdit->text().isEmpty(),
        tr("值错误：%1≤稀释液量减量≤%2，步进值0.5").arg(QString::number(REAGENT_PROBE_MIN_VOL / 10)).arg(QString::number(REAGENT_PROBE_MAX_VOL / 10)));
    // 稀释液量增量
    tempData = ui->SuckVolIncDilluteEdit->text().toDouble() * 10;
    OnErr((tempData < REAGENT_PROBE_MIN_VOL || tempData > REAGENT_PROBE_MAX_VOL) && !ui->SuckVolIncDilluteEdit->text().isEmpty(),
        tr("值错误：%1≤稀释液量增量≤%2，步进值0.5").arg(QString::number(REAGENT_PROBE_MIN_VOL / 10)).arg(QString::number(REAGENT_PROBE_MAX_VOL / 10)));

    // 稀释液量R1
    OnErr(!CheckSampleVolIsValid(ui->RegentR1DilluteEdit), tr("值错误：10≤稀释液量R1≤100，步进值0.5"));
    // 稀释液量R2
    OnErr(!CheckSampleVolIsValid(ui->RegentR3DilluteEdit), tr("值错误：10≤稀释液量R2≤100，步进值0.5"));
    
    // 试剂编号
    OnErr(ui->RegentVolR1Code->text().toInt() <= 0, tr("值错误：R1的试剂编号必须大于0"));
       
    // 稀释液编号
    int iData = ui->dilution_line_edit->text().toInt();
    OnErr(ui->dilution_radio_Button->isChecked() && (910 > iData || iData > 919),
        tr("值错误：稀释液编号范围为：910≤编号≤919"));

    OnErr(ui->dilution_radio_Button->isChecked() && ui->dilution_mixin_level->currentIndex() == -1,
        tr("值错误：未设置稀释液的混匀等级"));

    // 前带界限值参数
    if (!(ui->ProzonemLine_Edit->text().isEmpty() && ui->ProzonenLine_Edit->text().isEmpty()
        && ui->ProzonepLine_Edit->text().isEmpty() && ui->ProzoneqLine_Edit->text().isEmpty()))
    {
        OnErr(!CheckMeteringPointIsValid(ui->ProzonemLine_Edit->text().toInt(), ui->ProzonenLine_Edit->text().toInt(),
            ui->ProzonepLine_Edit->text().toInt(), ui->ProzoneqLine_Edit->text().toInt()),
            tr("前带界限测光点值错误，四个测光点应该依次递增,且1≤测光点≤34"));
    }

    // 吸光度检查
    for (auto checkParamsCtrlGroup : m_checkParamsCtrls)
    {
        auto edits = get<1>(checkParamsCtrlGroup);

        OnErr(get<0>(checkParamsCtrlGroup).at(1)->currentData().toInt() == 2 && edits.at(2)->text().toInt() < 2,
            tr("测光点设置错误，测观点浓度计算公式选择2时，第一个光点必须大于1"));

        if (!(edits.at(2)->text().isEmpty() && edits.at(3)->text().isEmpty()
            && edits.at(4)->text().isEmpty() && edits.at(5)->text().isEmpty()))
        {
            OnErr(!CheckMeteringPointIsValid(edits.at(2)->text().toInt(), edits.at(3)->text().toInt(),
                edits.at(4)->text().toInt(), edits.at(5)->text().toInt()),
                tr("吸光度差检查测光点值错误，四个测光点应该依次递增,且1≤测光点≤34"));
        }
    }

    // 反应杯清洗、双向同测
    bool bOk = false;
    ui->ReactionCupWashCombo->currentData().toInt(&bOk);
    OnErr(!bOk, tr("值错误： 反应杯清洗类型"));

    return true;
}

///
/// @brief 窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void AnalysisParamWidget::showEvent(QShowEvent *event)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 让基类处理事件
	QWidget::showEvent(event);

	// 第一次显示时初始化
	if (!m_bInit)
	{
		m_bInit = true;
		InitAfterShow();
	}
}

///
/// @brief
///     复位控件
///
/// @par History:
/// @li 6950/ChenFei，2022年05月18日，新建函数
///
void AnalysisParamWidget::Reset(bool updateSampleType)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (updateSampleType)
    {
        ui->SampleTypeCombo->clear();
        ui->fristSampleTypeCombo->clear();
    }

	ui->water_radio_Button->setChecked(true);

	for (auto edit : QWidget::findChildren<QLineEdit*>())
	{
		edit->clear();
	}

	for (auto box : QWidget::findChildren<QComboBox*>())
	{
		box->setCurrentIndex(0);
	}

    ui->AnalysisMethodCombo->setCurrentIndex(-1);
}

void AnalysisParamWidget::LoadCombTwinsTest(std::shared_ptr<::ch::tf::SpecialAssayInfo> chSAI)
{
	// 复位控件
	ui->combTwinsTest->clear();
	ui->combTwinsTest->addItem(tr(""), -1);
	ui->combTwinsTest->setCurrentIndex(0);
	ui->combTwinsTest->setEditable(false);
	ui->combTwinsTest->setEnabled(true);

	// 参数检查
	if (chSAI->assayCode == INVALID_ASSAY_CODE)
	{
		return;
	}

	// 添加同类型的项目到组合框
	std::set<int> assaycodes;
    auto& CIM = CommonInformationManager::GetInstance();
    auto& assays = CIM->GetChemistrySpecialAssayMap();
	for (auto& assay : assays)
	{
		// 排除非同样本源类型&&型号设备项目&&参数版本&&开放项目
        if (assay.second->sampleSourceType != chSAI->sampleSourceType
            || assay.second->deviceType != chSAI->deviceType)
		{
			continue;
		}

		// 排除S.IND项目
		if (assay.second->assayCode >= ch::tf::g_ch_constants.ASSAY_CODE_SIND
            && assay.second->assayCode <= ch::tf::g_ch_constants.ASSAY_CODE_I)
		{
			continue;
		}

		// 排除自己、已存在的项目
		if (assay.second->assayCode == chSAI->assayCode
            || assaycodes.count(assay.second->assayCode))
		{
			continue;
		}

		ui->combTwinsTest->addItem(QString::fromStdString(CIM->GetAssayNameByCode(assay.second->assayCode)), assay.second->assayCode);
		assaycodes.insert(assay.second->assayCode);
	}

	switch (chSAI->emTwinsTestType)
	{
		// 如果已成为前段项目 -> 显示后段项目，可选择
		case ::ch::tf::TwinsTestType::TWINS_TEST_TYPE_FIRST: {
			ui->combTwinsTest->setCurrentIndex(ui->combTwinsTest->findData(chSAI->iTwinsTestAssayCode));
			ui->combTwinsTest->setEnabled(true);
			break;
		}

		// 如果已成为后段项目 -> 显示前段项目，不可选择
		case ::ch::tf::TwinsTestType::TWINS_TEST_TYPE_SECOND: {
			ui->combTwinsTest->setCurrentIndex(ui->combTwinsTest->findData(chSAI->iTwinsTestAssayCode));
			ui->combTwinsTest->setEnabled(false);
			break;
		}

		// 如果不是双向同测项目
		case ::ch::tf::TwinsTestType::TWINS_TEST_TYPE_NONE: {
			ui->combTwinsTest->setCurrentIndex(0);
			ui->combTwinsTest->setEnabled(true);
			break;
		}

		default:
			break;
	}
}

void AnalysisParamWidget::UpdateCtrlsEnabled(bool isOpenAssay)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const auto& userPms = UserInfoManager::GetInstance();

    // 小数位数
    ui->DecimalCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_ASSAY_RESULT_PRICISION));
    // 简称
    ui->AssayNameEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_ASSAY_SET_SHORTNAME));
    // 样本类型
    ui->SampleTypeCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_ASSAY_SAMPLETYPE));
    // 主波长
    ui->PrimaryOSVCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SECONDARY_OSV));
    // 打印名称
    ui->PrintNameEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_ASSAY_SET_PRINTNAME));
    // 结果单位
    ui->ResUnitCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PMS_ASSAY_RESULT_UNIT));
	// 次波长
	ui->SecondaryOSVCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_PRIMARY_OSV));
	// 分析方法
	ui->AnalysisMethodCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_ANALYSIS_METHOD));
    // 反应时间
	ui->ReactionTimeCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REACTION_TIME));
    // 参数版本
    ui->AssayParamVerCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_CH_VERSION_CHAMGED));

    // 样本优先类型
    ui->fristSampleTypeCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_FIRST_SAMPLE_TYPE));

	// 测光点
	ui->m_Edit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_TAKE_POINT));
	ui->n_Edit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_TAKE_POINT));
	ui->p_Edit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_TAKE_POINT));
	ui->q_Edit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_TAKE_POINT));
	ui->take_point->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_TAKE_POINT));

    // 样本量
    ui->SuckVolStdOrgEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SAMPLE_VOL_STD));
    ui->SuckVolIncOrgEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SAMPLE_VOL_INC));
    ui->SuckVolDecOrgEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SAMPLE_VOL_DEC));

    // 稀释样本量
    ui->SuckVolStdMixEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SUCK_SAMPLE_VOL));
    ui->SuckVolDecMixEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SUCK_SAMPLE_VOL));
    ui->SuckVolIncMixEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SUCK_SAMPLE_VOL));

    // 稀释液量
    ui->SuckVolStdDilluteEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SUCK_VOL));
    ui->SuckVolDecDilluteEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SUCK_VOL));
    ui->SuckVolIncDilluteEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_SUCK_VOL));

    // 混匀等级
    ui->MixstdLevel->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_MIX_LEVEL));
    ui->MixdecLevel->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_MIX_LEVEL));
    ui->MixincLevel->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_MIX_LEVEL));

	// 稀释液
    if (isOpenAssay || userPms->IsPermisson(PSM_DILUTION))
    {
        ui->water_radio_Button->setEnabled(true);
        ui->dilution_radio_Button->setEnabled(true);
        ui->dilution_mixin_level->setEnabled(true);
        ui->dilution_line_edit->setEnabled(ui->dilution_radio_Button->isChecked());
    } 
    else
    {
        ui->water_radio_Button->setEnabled(false);
        ui->dilution_radio_Button->setEnabled(false);
        ui->dilution_line_edit->setEnabled(false);
        ui->dilution_mixin_level->setEnabled(false);
    }

    // 试剂量
    ui->RegentVolR1Edit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_VOL));
    ui->RegentVolR3Edit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_VOL));

    // 稀释液量
    ui->RegentR1DilluteEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_DILUTION));
    ui->RegentR3DilluteEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_DILUTION));

    // 富余量
    ui->DummyR1Combo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_DUMMY));
    ui->DummyR3Combo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_DUMMY));

    // 混匀等级
    ui->MixSpeedR1Combo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_LEVEL));
    ui->MixSpeedR3Combo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_LEVEL));

    // 试剂编号
    ui->RegentVolR1Code->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REAGENT_NUM));

    // 开瓶有效期，封闭项目不显示
    ui->openBottleValidityEdit->setVisible(isOpenAssay);
    ui->label_4->setVisible(isOpenAssay);

    // 线性检查界限值
    ui->LinearDiagLMTLineEdit4_8->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_LINEAR_CHECK));
    ui->LinearDiagLMTLineEdit9->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_LINEAR_CHECK));
    ui->setValue1LineEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_LINEAR_CHECK));
    ui->setValue2LineEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_LINEAR_CHECK));

    // 反应界限吸光度
    ui->ReactionLmtLineEdit->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REACTION_LMT));
    ui->ReactionSignCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REACTION_LMT));

    // 前带界限，封闭项目不显示
    ui->lmt_widget->setVisible(isOpenAssay);

    // 吸光度检查
    ui->groupBox_4->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_INSPECT_CHECK));

    // 智能计算
    ui->checkBox->setVisible(isOpenAssay || userPms->IsPermisson(PMS_ASSAY_SET_INTELLECT_CALC));

    // 反应杯清洗
    ui->ReactionCupWashCombo->setEnabled(isOpenAssay || userPms->IsPermisson(PSM_REACTION_CUP));
}

bool AnalysisParamWidget::CheckMeteringPointIsValid(int m, int n, int p, int q)
{
    // m<n<p<q，0表示缺省
    if (m <= 0)
    {
        return false;
    }

    if ((n <= m && n != 0)|| (n == 0 && p != 0))
    {
        return false;
    }

    if ((p <= n && p != 0) || (p == 0 && q != 0))
    {
        return false;
    }

    if (q <= p && q != 0)
    {
        return false;
    }

    return true;
}

bool AnalysisParamWidget::CheckSampleVolIsValid(QLineEdit* edit)
{
    int tempData = edit->text().toDouble() * 10;

    return (tempData >= REAGENT_PROBE_MIN_VOL
        && tempData <= REAGENT_REACTION_MAX_VOL
        && (tempData % 5) == 0)
        || edit->text().isEmpty();
}

ch::tf::AnalyCheckParam AnalysisParamWidget::GetAnalyChkParam()
{
    ch::tf::AnalyCheckParam analyCheckParam;
    // 线性检查参数
    ch::tf::LinearityLimit linearityLimit;
    linearityLimit.__set_waveSel(ch::tf::AssayParamWaveSel::ASSAY_PARAM_WAVE_SEL_PRIMARY_SUB_DEPUTY);
    linearityLimit.__set_lmt4_8(ui->LinearDiagLMTLineEdit4_8->text().toInt());
    linearityLimit.__set_lmt9(ui->LinearDiagLMTLineEdit9->text().toInt());
    QString strData = ui->setValue1LineEdit->text();
    linearityLimit.__set_setValue1(strData.isEmpty() ? INT_MAX : strData.toInt());
    strData = ui->setValue2LineEdit->text();
    linearityLimit.__set_setValue2(strData.isEmpty() ? INT_MAX : strData.toInt());
    analyCheckParam.__set_linearityLimit(linearityLimit);

    // 前带界限值参数
    ch::tf::ProzoneLimit prozoneLimit;
    prozoneLimit.__set_waveSel(ch::tf::AssayParamWaveSel::ASSAY_PARAM_WAVE_SEL_PRIMARY);
    strData = ui->ProzoneLowerLMTLineEdit->text();
    prozoneLimit.__set_lowerLMT(strData.isEmpty() ? INT_MIN : strData.toInt());
    strData = ui->ProzoneUpperLMTLineEdit->text();
    prozoneLimit.__set_upperLMT(strData.isEmpty() ? INT_MAX : strData.toInt());
    prozoneLimit.__set_m(ui->ProzonemLine_Edit->text().toInt());
    prozoneLimit.__set_n(ui->ProzonenLine_Edit->text().toInt());
    prozoneLimit.__set_p(ui->ProzonepLine_Edit->text().toInt());
    prozoneLimit.__set_q(ui->ProzoneqLine_Edit->text().toInt());

    int curIdx = ui->ProzoneSideCombo->currentIndex();
    prozoneLimit.__set_side((ch::tf::AssayParamSide::type)ui->ProzoneSideCombo->currentData().toInt());

    strData = ui->ProzoneAbsPreLineEdit->text();
    prozoneLimit.__set_absPre(strData.isEmpty() ? INT_MAX : strData.toInt());
    strData = ui->ProzoneAbsLaterLineEdit->text();
    prozoneLimit.__set_absLatter(strData.isEmpty() ? INT_MAX : strData.toInt());
    analyCheckParam.__set_prozoneLimit(prozoneLimit);

    // 反应界限值参数
    ch::tf::AbsLimit absLimit;
    absLimit.__set_waveSel(ch::tf::AssayParamWaveSel::ASSAY_PARAM_WAVE_SEL_PRIMARY);
    strData = ui->ReactionLmtLineEdit->text();
    absLimit.__set_reactionLmt(strData.isEmpty() ? INT_MAX : strData.toInt());
    absLimit.__set_reactionSign((ch::tf::AssayParamSign::type)ui->ReactionSignCombo->currentData().toInt());
    analyCheckParam.__set_absLimit(absLimit);

    //----------------------------
    // 吸光度检查
    //----------------------------
    vector<ch::tf::AbsDiffCheck> absChks;
    for (auto checkParamsCtrlGroup : m_checkParamsCtrls)
    {
        ch::tf::AbsDiffCheck absChk;
        // 组合框类控件
        auto combos = get<0>(checkParamsCtrlGroup);
        absChk.__set_waveSel((ch::tf::AssayParamWaveSel::type)combos.at(0)->currentData().toInt());
        absChk.__set_takePrePoint(combos.at(1)->currentIndex() == 1);

        curIdx = combos.at(2)->currentIndex();
        absChk.__set_side((ch::tf::AssayParamSide::type)combos.at(2)->currentData().toInt());

        // 编辑器类控件
        auto edits = get<1>(checkParamsCtrlGroup);
        strData = edits.at(0)->text();
        absChk.__set_lowerLMT(strData.isEmpty() ? INT_MIN : strData.toInt());
        strData = edits.at(1)->text();
        absChk.__set_upperLMT(strData.isEmpty() ? INT_MAX : strData.toInt());
        absChk.__set_m(edits.at(2)->text().toInt());
        absChk.__set_n(edits.at(3)->text().toInt());
        absChk.__set_p(edits.at(4)->text().toInt());
        absChk.__set_q(edits.at(5)->text().toInt());

        strData = edits.at(6)->text();
        absChk.__set_absPre(strData.isEmpty() ? INT_MAX : strData.toInt());
        strData = edits.at(7)->text();
        absChk.__set_absLatter(strData.isEmpty() ? INT_MAX : strData.toInt());

        absChks.push_back(std::move(absChk));
    }
    analyCheckParam.__set_absDiffCheck1(absChks.at(0));
    analyCheckParam.__set_absDiffCheck2(absChks.at(1));

    return analyCheckParam;
}

std::vector<ch::tf::SampleAspirateVol> AnalysisParamWidget::GetSampleAspirateVols()
{
    std::vector<ch::tf::SampleAspirateVol> sampleAspirateVols;
    for (auto sampCtrlGroup : m_sampCtrlsp)
    {
        ch::tf::SampleAspirateVol sampAspVol;
        sampAspVol.__set_originalSample(get<0>(sampCtrlGroup)->text().toFloat() * 10);
        sampAspVol.__set_mixingLevel(get<3>(sampCtrlGroup)->currentData().toInt());
        
        int vol = get<1>(sampCtrlGroup)->text().toFloat() * 10;
        if (vol > 0)
        {
            sampAspVol.__set_sample4Test(vol);
        }

        int sampleVol = get<2>(sampCtrlGroup)->text().toFloat() * 10;
        if (sampleVol > 0)
        {
            sampAspVol.__set_diluent(get<2>(sampCtrlGroup)->text().toFloat() * 10);
        }

        sampleAspirateVols.push_back(std::move(sampAspVol));
    }

    return sampleAspirateVols;
}

void AnalysisParamWidget::OnSampleSourceChanged(const QString &text)
{
    Q_UNUSED(text);

	if (m_bSampleTypeFlag)
	{
		POST_MESSAGE(MSG_ID_ASSAY_SAMPLE_UPDATE, ui->SampleTypeCombo->currentData().toInt());
	}
}

void AnalysisParamWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    const auto& spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(ui->AssayCodeEdit->text().toInt());
    UpdateCtrlsEnabled(spAssay == nullptr || spAssay->openAssay);
}

void AnalysisParamWidget::OnAddAssay(QString name, QString unit, int code, QString version, QSet<int> sampleSources)
{
    m_bSampleTypeFlag = false;
    m_bVersionFlag = false;

    // 新增项目
    const auto& CIM = CommonInformationManager::GetInstance();
    if (CIM->GetAssayInfo(code) == nullptr)
    {
        Reset();

        ui->fullNameEdit->setText(name);

        ui->AssayCodeEdit->setText(QString::number(code));

        ui->ResUnitCombo->clear();
        ui->ResUnitCombo->addItem(unit);

        ui->AssayParamVerCombo->clear();
        ui->AssayParamVerCombo->addItem("1", 1);

        // 新增封闭项目隐藏开瓶有效期
        ui->openBottleValidityEdit->setVisible(
            code >= ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN
            && code <= ::tf::AssayCodeRange::CH_OPEN_RANGE_MAX);
    }

    // 设置样本类型下拉框
    std::set<int> sampleTypes;
    for (const int& ss : sampleSources)
    {
        sampleTypes.insert(ss);
    }
    UpdateSampleTypeToCommbox(ui->SampleTypeCombo, sampleTypes);

    // 设置优先样本类型下拉框
    std::set<int> fristSampleTypes;
    for (auto& spSai : CIM->GetChemistrySpecialAssayInfo(code, tf::DeviceType::DEVICE_TYPE_C1000, -1, version.toStdString()))
    {
        if (sampleSources.contains(spSai->sampleSourceType))
        {
            fristSampleTypes.insert(spSai->sampleSourceType);
        }
    }
    UpdateSampleTypeToCommbox(ui->fristSampleTypeCombo, fristSampleTypes);

    ui->ResUnitCombo->setCurrentIndex(0);
    ui->AssayParamVerCombo->setCurrentIndex(0);

    m_bSampleTypeFlag = true;
    m_bVersionFlag = true;

    ui->SampleTypeCombo->setCurrentIndex(0);
    OnSampleSourceChanged(ui->SampleTypeCombo->currentText());
}

void AnalysisParamWidget::UpdateSampleTypeToCommbox(QComboBox* box, const std::set<int>& sampleTypes)
{
    box->clear();

    for (int ss : sampleTypes)
    {
        if (ss == tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER)
        {
            continue;
        }

        AddTfEnumItemToComBoBox(box, (tf::SampleSourceType::type)ss);
    }

    if (sampleTypes.find(tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER) != sampleTypes.end())
    {
        AddTfEnumItemToComBoBox(box, tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);
    }
}

void AnalysisParamWidget::LoadAnalysisParam(const AssayListModel::StAssayListRowItem &item, bool updateSampleType)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	m_bSampleTypeFlag = false;
    m_bVersionFlag = false;

	// 复位界面
	Reset(updateSampleType);

    if (item.assayCode == INVALID_ASSAY_CODE)
    {
        return;
    }

    ui->AssayNameEdit->setText(item.name);
    ui->fullNameEdit->setText(item.name);
    ui->PrintNameEdit->setText(item.name);
    ui->AssayCodeEdit->setText(QString::number(item.assayCode));
    ui->RegentVolR1Code->setText(QString::number(item.assayCode));
    ui->AssayParamVerCombo->setCurrentText(item.version);

    // 查询通用项目信息
    const auto &CIM = CommonInformationManager::GetInstance();
    auto GAI = CIM->GetAssayInfo(item.assayCode);
    UpdateCtrlsEnabled(GAI == nullptr || GAI->openAssay);
    ui->combTwinsTest->setEnabled(false);

	// 判断待加载项有效
	if (item.assayCode == INVALID_ASSAY_CODE)
	{
		return;
	}

	if (GAI == nullptr)
	{
        m_bSampleTypeFlag = true;
        m_bVersionFlag = true;
		
		ULOG(LOG_INFO, "GetAssayInfo() Failed!");		
		return;
	}

    // 参数版本
    bool existVer = false;
    auto curVer = item.version;
    ui->AssayParamVerCombo->clear();
    for (auto& ver : CIM->GetAssayVersionList(item.assayCode, (::tf::DeviceType::type)item.model))
    {
		auto qsVer = QString::fromStdString(ver);
        ui->AssayParamVerCombo->addItem(qsVer, qsVer);
        if (qsVer == curVer)
        {
            existVer = true;
        }
    }

    if (!existVer)
    {
        ui->AssayParamVerCombo->addItem(item.version, curVer);
    }
    ui->AssayParamVerCombo->setCurrentText(item.version);

	// 查询生化通用项目信息
	auto chGAI = CIM->GetChemistryAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model, item.version.toStdString());
	if (chGAI == nullptr)
	{
		ULOG(LOG_ERROR, "GetChemistryAssayInfo() Failed!");
        m_bSampleTypeFlag = true;
        m_bVersionFlag = true;
		return;
	}

    // 查询生化特殊项目信息
    auto chSAIs = CIM->GetChemistrySpecialAssayInfo(item.assayCode, (::tf::DeviceType::type)item.model, -1, item.version.toStdString());
    if (chSAIs.empty())
    {
        ULOG(LOG_ERROR, "GetChemistrySpecialAssayInfo() Failed!");
        m_bSampleTypeFlag = true;
        m_bVersionFlag = true;
        return;
    }

    // 第一优先级样本源
    int fristSampleType = -1;

    // 当前项目
    shared_ptr<ch::tf::SpecialAssayInfo> chSAI = nullptr;

    std::set<int> ssSet;
    for (auto assay : chSAIs)
    {
        if (assay->sampleSourceType == item.sampleType)
        {
            chSAI = assay;
        }

        if (assay->priority)
        {
            fristSampleType = assay->sampleSourceType;

            // 未设置样本类型，显示优先样本类型
            if (item.sampleType == -1)
            {
                chSAI = assay;
            }
        }

        ssSet.insert(assay->sampleSourceType);
    }

    if (updateSampleType)
    {
        UpdateSampleTypeToCommbox(ui->SampleTypeCombo, ssSet);
        UpdateSampleTypeToCommbox(ui->fristSampleTypeCombo, ssSet);
    }

	// 将信息刷入到控件
	//----------------------------
	// 基本参数
	//----------------------------
	ui->AssayNameEdit->setText(item.name);
	ui->fullNameEdit->setText(QString::fromStdString(GAI->assayFullName));
	ui->PrintNameEdit->setText(GAI->printName.c_str());

    // 样本类型
    ui->SampleTypeCombo->setCurrentText(ConvertTfEnumToQString((tf::SampleSourceType::type)item.sampleType));
    ui->fristSampleTypeCombo->setCurrentText(ConvertTfEnumToQString((tf::SampleSourceType::type)fristSampleType));

	ui->AnalysisMethodCombo->setCurrentIndex(ui->AnalysisMethodCombo->findText(ConvertTfEnumToQString(chGAI->analysisMethod)));
	ui->ReactionTimeCombo->setCurrentText(ConvertTfEnumToQString(chGAI->reactionTime));
	ui->m_Edit->setText(chGAI->analyPoints.m > 0 ? QString::number(chGAI->analyPoints.m) : "");
	ui->n_Edit->setText(chGAI->analyPoints.n > 0 ? QString::number(chGAI->analyPoints.n) : "");
	ui->p_Edit->setText(chGAI->analyPoints.p > 0 ? QString::number(chGAI->analyPoints.p) : "");
	ui->q_Edit->setText(chGAI->analyPoints.q > 0 ? QString::number(chGAI->analyPoints.q) : "");
	ui->take_point->setCurrentIndex(chGAI->analyPoints.takePrePoint ? 1 : 0);
	if (chGAI->waveLengths.size() > 0)
	{
		ui->PrimaryOSVCombo->setCurrentIndex(ui->PrimaryOSVCombo->findData(chGAI->waveLengths.at(0)));
	}

	if (chGAI->waveLengths.size() > 1)
	{
		ui->SecondaryOSVCombo->setCurrentIndex(ui->SecondaryOSVCombo->findData(chGAI->waveLengths.at(1)));
	}

	// 结果单位下拉框
	ui->ResUnitCombo->clear();
	auto unitTypeMaps = DictionaryQueryManager::GetInstance()->GetMapUnitType();
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

	// 小数位
	ui->DecimalCombo->setCurrentIndex(GAI->decimalPlace);

	//----------------------------
	// 样本量
	//----------------------------
	if (chSAI != nullptr && chSAI->__isset.sampleAspirateVols)
	{
		auto &sampleAspirateVols = chSAI->sampleAspirateVols;
		for (int i = 0; i < sampleAspirateVols.size(); i++)
		{
			auto &sampAspInfo = sampleAspirateVols.at(i);
			if (sampAspInfo.originalSample > 0)
			{
				get<0>(m_sampCtrlsp[i])->setText(QString::number(sampAspInfo.originalSample / 10.0));
			}
			
            if (sampAspInfo.sample4Test > 0)
            {
                get<1>(m_sampCtrlsp[i])->setText(QString::number(sampAspInfo.sample4Test / 10.0));
            }

            if (sampAspInfo.diluent > 0)
            {
                get<2>(m_sampCtrlsp[i])->setText(QString::number(sampAspInfo.diluent / 10.0));
            }

            if (sampAspInfo.mixingLevel > 0)
            {
                get<3>(m_sampCtrlsp[i])->setCurrentIndex(get<3>(m_sampCtrlsp[i])->findData(sampAspInfo.mixingLevel));
            }
		}
	}

	// 稀释液
	if (chGAI->diluentCode == 0)
	{
		ui->water_radio_Button->setChecked(true);
	}
    else
    {
        ui->dilution_radio_Button->setChecked(true);
        ui->dilution_line_edit->setText(QString::number(chGAI->diluentCode));
    }

    ui->dilution_mixin_level->setCurrentText(QString::number(chGAI->diluentMixLevel));
	
	//----------------------------
	// 试剂分注量
	//----------------------------
	if(GAI->__isset.stableDays)
	{
		ui->openBottleValidityEdit->setText(GAI->stableDays == 0 ? "" : QString::number(GAI->stableDays));
	}
	if (chGAI->__isset.reagentAspirateVols)
	{
		auto &reagentAspirateVols = chGAI->reagentAspirateVols;
		for (int i = 0; i < reagentAspirateVols.size(); i++)
		{
			auto &reagentAspInfo = reagentAspirateVols.at(i);
			if (reagentAspInfo.reagent > 0)
			{
				get<0>(m_reagentCtrls[i])->setText(QString::number(reagentAspInfo.reagent / 10.0));
			}

            if (reagentAspInfo.diluent > 0)
            {
                get<1>(m_reagentCtrls[i])->setText(QString::number(reagentAspInfo.diluent / 10.0));
            }

            if (reagentAspInfo.reagentDummy > 0)
            {
                auto idx = get<2>(m_reagentCtrls[i])->findData(reagentAspInfo.reagentDummy);
                get<2>(m_reagentCtrls[i])->setCurrentIndex(idx);
            }

            if (reagentAspInfo.mixingLevel >= 0)
            {
                get<3>(m_reagentCtrls[i])->setCurrentIndex(reagentAspInfo.mixingLevel);
            }
		}

        if (chGAI->__isset.reagentCode)
        {
            ui->RegentVolR1Code->setText(QString::number(chGAI->reagentCode));
        }
	}

	//----------------------------
	// 线性检查、前带界限、反应界限
	//----------------------------
	if (chSAI != nullptr && chSAI->__isset.analyChkParam)
	{
		auto &analyChkParam = chSAI->analyChkParam;
		auto &linearityLimit = analyChkParam.linearityLimit;
		ui->LinearDiagLMTLineEdit4_8->setText(linearityLimit.lmt4_8 == 0 ? "" : QString::number(linearityLimit.lmt4_8));
		ui->LinearDiagLMTLineEdit9->setText(linearityLimit.lmt9 == 0 ? "" : QString::number(linearityLimit.lmt9));
		ui->setValue1LineEdit->setText(linearityLimit.setValue1 == INT_MAX ? "" : QString::number(linearityLimit.setValue1));
		ui->setValue2LineEdit->setText(linearityLimit.setValue2 == INT_MAX ? "" : QString::number(linearityLimit.setValue2));

		auto &prozoneLimit = analyChkParam.prozoneLimit;
		ui->ProzoneLowerLMTLineEdit->setText(prozoneLimit.lowerLMT == INT_MIN ? "" : QString::number(prozoneLimit.lowerLMT));
		ui->ProzoneUpperLMTLineEdit->setText(prozoneLimit.upperLMT == INT_MAX ? "" : QString::number(prozoneLimit.upperLMT));
		ui->ProzonemLine_Edit->setText(prozoneLimit.m > 0 ? QString::number(prozoneLimit.m) : "");
		ui->ProzonenLine_Edit->setText(prozoneLimit.n > 0 ? QString::number(prozoneLimit.n) : "");
		ui->ProzonepLine_Edit->setText(prozoneLimit.p > 0 ? QString::number(prozoneLimit.p) : "");
		ui->ProzoneqLine_Edit->setText(prozoneLimit.q > 0 ? QString::number(prozoneLimit.q) : "");

		ui->ProzoneSideCombo->setCurrentIndex(ui->ProzoneSideCombo->findData(prozoneLimit.side));
		ui->ProzoneAbsPreLineEdit->setText(prozoneLimit.absPre == INT_MAX ? "" : QString::number(prozoneLimit.absPre));
		ui->ProzoneAbsLaterLineEdit->setText(prozoneLimit.absLatter == INT_MAX ? "" : QString::number(prozoneLimit.absLatter));

		auto &absLimit = analyChkParam.absLimit;
		ui->ReactionLmtLineEdit->setText(absLimit.reactionLmt == INT_MAX ? "" : QString::number(absLimit.reactionLmt));
		ui->ReactionSignCombo->setCurrentIndex(ui->ReactionSignCombo->findData(absLimit.reactionSign));
	}

	//----------------------------
	// 吸光度检查
	//----------------------------
	if (chSAI != nullptr && chSAI->__isset.analyChkParam)
	{
		auto &analyChkParam = chSAI->analyChkParam;
		auto checkParams = {analyChkParam.absDiffCheck1, analyChkParam.absDiffCheck2};

		int i = 0;
		for (auto &checkParam : checkParams)
		{
			auto &ctrls = m_checkParamsCtrls.at(i);
			auto combos = get<0>(ctrls);

			combos.at(0)->setCurrentIndex(combos.at(0)->findData(checkParam.waveSel));
			combos.at(1)->setCurrentIndex(checkParam.takePrePoint ? 1 : 0);
			combos.at(2)->setCurrentIndex(combos.at(2)->findData(checkParam.side));

			auto edits = get<1>(ctrls);
			edits.at(0)->setText(checkParam.lowerLMT == INT_MIN ? "" : QString::number(checkParam.lowerLMT));
			edits.at(1)->setText(checkParam.upperLMT == INT_MAX ? "" : QString::number(checkParam.upperLMT));
			edits.at(2)->setText(checkParam.m > 0 ? QString::number(checkParam.m) : "");
			edits.at(3)->setText(checkParam.n > 0 ? QString::number(checkParam.n) : "");
			edits.at(4)->setText(checkParam.p > 0 ? QString::number(checkParam.p) : "");
			edits.at(5)->setText(checkParam.q > 0 ? QString::number(checkParam.q) : "");
			edits.at(6)->setText(checkParam.absPre == INT_MAX ? "" : QString::number(checkParam.absPre));
			edits.at(7)->setText(checkParam.absLatter == INT_MAX ? "" : QString::number(checkParam.absLatter));

			i++;
		}
	}

    // 智能计算
    ui->checkBox->setChecked(chGAI->enableIntellectCalc);

	//----------------------------
	// 反应杯清洗、双向同测
	//----------------------------
	if (chGAI->__isset.cupDetergentType)
	{
		ui->ReactionCupWashCombo->setCurrentIndex(ui->ReactionCupWashCombo->findData(chGAI->cupDetergentType));
	}

	// 双向同测
    if (chSAI != nullptr)
    {
    	LoadCombTwinsTest(chSAI);
    }

    // 存在双向同测，但不是当前版本的样本类型,禁用双向同测下拉框
    if (ui->combTwinsTest->isEnabled())
    {
        const auto& taSpSai = CIM->GetChemistrySpecialTwinsAssayInfo(
            item.assayCode, (::tf::DeviceType::type)item.model);

        if (taSpSai != nullptr 
            && (taSpSai->version != chSAI->version
            || taSpSai->sampleSourceType != chSAI->sampleSourceType))
        {
            ui->combTwinsTest->setEnabled(false);
        }
    }

	m_bSampleTypeFlag = true;
    m_bVersionFlag = true;
}

bool AnalysisParamWidget::GetAnalysisParam(tf::GeneralAssayInfo& GAI, ch::tf::GeneralAssayInfo& chGAI,
    ch::tf::SpecialAssayInfo& chSAI, int assayCode)
{
    // 判断待加载项有效
    if (assayCode == INVALID_ASSAY_CODE)
    {
        return true;
    }

    // 检查参数的有效性
    if (!CheckUserInputData(assayCode))
    {
        return false;
    }

    // 项目编号
    GAI.__set_assayCode(assayCode);
    chGAI.__set_assayCode(assayCode);
    chSAI.__set_assayCode(assayCode);

    // 设备类型
    chGAI.__set_deviceType(tf::DeviceType::DEVICE_TYPE_C1000);
    chSAI.__set_deviceType(tf::DeviceType::DEVICE_TYPE_C1000);

    // 参数版本
    std::string ver = ui->AssayParamVerCombo->currentText().toStdString();
    chGAI.__set_version(ver);
    chSAI.__set_version(ver);

    // 项目名称
    QString tempStr = ui->fullNameEdit->text().trimmed();
    GAI.__set_assayFullName(tempStr.toStdString());

    // 打印名称
    tempStr = ui->PrintNameEdit->text().trimmed();
    GAI.__set_printName(tempStr.toStdString());

    // 简称
    tempStr = ui->AssayNameEdit->text().trimmed();
    GAI.__set_assayName(tempStr.toStdString());

    // 样本类型
    chSAI.__set_sampleSourceType(ui->SampleTypeCombo->currentData().toInt());

    chGAI.__set_analysisMethod((::ch::tf::AnalysisMethod::type)ui->AnalysisMethodCombo->currentData().toInt());
    chGAI.__set_reactionTime((::ch::tf::ReactionTime::type)ui->ReactionTimeCombo->currentData().toInt());

    ch::tf::AnalyPointInfo analyPointInfo;
    analyPointInfo.__set_m(ui->m_Edit->text().toInt());
    analyPointInfo.__set_n(ui->n_Edit->text().toInt());
    analyPointInfo.__set_p(ui->p_Edit->text().toInt());
    analyPointInfo.__set_q(ui->q_Edit->text().toInt());

    analyPointInfo.__set_takePrePoint(ui->take_point->currentData().toBool());
    chGAI.__set_analyPoints(analyPointInfo);

    std::vector<int32_t> wavlens;
    if (ui->PrimaryOSVCombo->currentIndex() != -1)
    {
        wavlens.push_back(ui->PrimaryOSVCombo->currentData().toInt());
    }
    if (ui->SecondaryOSVCombo->currentIndex() != -1)
    {
        wavlens.push_back(ui->SecondaryOSVCombo->currentData().toInt());
    }

    if (!wavlens.empty())
    {
        chGAI.__set_waveLengths(wavlens);
    }

    // 单位
    vector<::tf::AssayUnit> vecUnit;
    const auto& CIM = CommonInformationManager::GetInstance();
    auto spAssayInfo = CIM->GetAssayInfo(ui->AssayCodeEdit->text().toInt());

    if (spAssayInfo != nullptr)
    {
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
    }
    else
    {
        ::tf::AssayUnit unit;
        unit.factor = 1;
        unit.isCurrent = true;
        unit.isMain = true;
        unit.isUserDefine = true;
        unit.name = ui->ResUnitCombo->currentText().toStdString();
        vecUnit.push_back(std::move(unit));
    }

    GAI.__set_units(vecUnit);

    GAI.__set_decimalPlace(ui->DecimalCombo->currentText().toInt());

    // 样本量
    chSAI.__set_sampleAspirateVols(std::move(GetSampleAspirateVols()));   

    // 稀释液
    if (ui->water_radio_Button->isChecked())
    {
        chGAI.__set_diluentCode(0);
    }
    else
    {
        chGAI.__set_diluentCode(ui->dilution_line_edit->text().toInt());
    }

    chGAI.__set_diluentMixLevel(ui->dilution_mixin_level->currentData().toInt());
    
    //----------------------------
    // 试剂分注量
    //----------------------------

    // 开放项目才检查时间开瓶有效期
    if (GAI.assayCode >= ::tf::AssayCodeRange::CH_OPEN_RANGE_MIN
        && GAI.assayCode <= ::tf::AssayCodeRange::CH_OPEN_RANGE_MAX)
    {
        int openDay = ui->openBottleValidityEdit->text().toInt();
        OnErr(openDay <= 0, tr("值错误：试剂的开瓶有效期不能为空且必须大于0"));
        GAI.__set_stableDays(openDay);
    }

    std::vector<ch::tf::ReagentAspirateVol> reagentAspirateVols;
    int i = 0;
    for (auto reagentCtrlGroup : m_reagentCtrls)
    {
        ch::tf::ReagentAspirateVol reagentAspirateVol;
        reagentAspirateVol.__set_reagent(-1);

        auto stage(ch::tf::ReagentStage::REAGENT_STAGE_R1);
        if (i == 1)
        {
            stage = ch::tf::ReagentStage::REAGENT_STAGE_R3;
        }

        reagentAspirateVol.__set_stage(stage);
        reagentAspirateVol.__set_reagent(get<0>(reagentCtrlGroup)->text().toInt() * 10);
        if (reagentAspirateVol.reagent <= 0)
        {
            break;
        }

        reagentAspirateVol.__set_diluent(get<1>(reagentCtrlGroup)->text().toInt() * 10);
        reagentAspirateVol.__set_reagentDummy(get<2>(reagentCtrlGroup)->currentData().toInt());
        reagentAspirateVol.__set_mixingLevel(get<3>(reagentCtrlGroup)->currentData().toInt());

        // 试剂量限制，bug：14233
        if (reagentAspirateVol.diluent != 0)
        {
            OnErr(reagentAspirateVol.reagent == 0, tr("值错误：设置稀释液量之前请先设置试剂量"));

            // 稀释液量不为0时，试剂量[10-43]
            OnErr(reagentAspirateVol.reagent < REAGENT_PROBE_MIN_VOL
                || reagentAspirateVol.reagent > 430,
                tr("值错误：%1≤试剂量≤43，步进值0.5")
                .arg(QString::number(REAGENT_PROBE_MIN_VOL / 10)));

            // 试剂量+稀释液量[75-100]
            double vol = reagentAspirateVol.reagent + reagentAspirateVol.diluent;
            OnErr(vol > REAGENT_REACTION_MAX_VOL || vol < REAGENT_REACTION_MIN_VOL,
                tr("值错误：%1≤试剂量+稀释液量≤%2")
                .arg(QString::number(REAGENT_REACTION_MIN_VOL / 10))
                .arg(QString::number(REAGENT_REACTION_MAX_VOL / 10)));
        }
        else
        {
            // 稀释液量为0时，试剂量[10-150]
            OnErr((reagentAspirateVol.reagent < REAGENT_PROBE_MIN_VOL 
                || reagentAspirateVol.reagent > REAGENT_PROBE_MAX_VOL) 
                , tr("值错误：%1≤试剂量≤%2，步进值0.5")
                .arg(QString::number(REAGENT_PROBE_MIN_VOL / 10))
                .arg(QString::number(REAGENT_PROBE_MAX_VOL / 10)));

        }
        
        reagentAspirateVols.push_back(std::move(reagentAspirateVol));

        i++;
    }
    chGAI.__set_reagentAspirateVols(reagentAspirateVols);

    // 试剂编号
    chGAI.__set_reagentCode(ui->RegentVolR1Code->text().toInt());

    if (!UiCommon::Instance()->CheckReagentVolumeIsValid(chGAI.reagentAspirateVols, chSAI.sampleAspirateVols, false))
    {
        return false;
    }

    chSAI.__set_analyChkParam(std::move(GetAnalyChkParam()));

    // 智能计算
    chGAI.__set_enableIntellectCalc(ui->checkBox->isChecked());

    //----------------------------
    // 反应杯清洗、双向同测
    //----------------------------
    chGAI.__set_cupDetergentType((ch::tf::CupDetergentType::type)ui->ReactionCupWashCombo->currentData().toInt());

    // 双向同测
    if (ui->combTwinsTest->currentIndex() <= 0)
    {
        chSAI.__set_emTwinsTestType(::ch::tf::TwinsTestType::TWINS_TEST_TYPE_NONE);
        chSAI.__set_iTwinsTestAssayCode(INVALID_ASSAY_CODE);
    }
    else if (ui->combTwinsTest->isEnabled())
    {
        // 前项目        
        chSAI.__set_emTwinsTestType(::ch::tf::TwinsTestType::TWINS_TEST_TYPE_FIRST);
        chSAI.__set_iTwinsTestAssayCode(ui->combTwinsTest->currentData().toInt());       
    }
    else
    {
        chSAI.__set_iTwinsTestAssayCode(ui->combTwinsTest->currentData().toInt());
    }

    // 获取另一个项目的信息
    int otherCode = -1;
    std::shared_ptr<ch::tf::SpecialAssayInfo> spFirstSai = nullptr;
    std::shared_ptr<ch::tf::SpecialAssayInfo> spSecondSai = nullptr;

    // 当前保存的样本源是双向同测
    if (chSAI.iTwinsTestAssayCode > 0)
    {
        const auto& saiVecs = CIM->GetChemistrySpecialAssayInfo(chSAI.iTwinsTestAssayCode,
            tf::DeviceType::DEVICE_TYPE_C1000, chSAI.sampleSourceType);

        // 获取双项同测的另一个项目信息
        OnErr(saiVecs.size() != 1, tr("双项同测值错误：后项目[%1]不存在启用的版本").arg(ui->combTwinsTest->currentText()));        
        spSecondSai = saiVecs[0];
        spFirstSai = std::make_shared<ch::tf::SpecialAssayInfo>(chSAI);
    } 
    else
    {
        // 当前项目的另一个样本源被设置为双向同测
        for (const auto& sai : CIM->GetChemistrySpecialAssayInfo(
                assayCode, tf::DeviceType::DEVICE_TYPE_C1000, -1, ver))
        {
            // 一个项目只有一种样本源可以被设置为双向同测
            if (sai->iTwinsTestAssayCode > 0)
            {
                spSecondSai = CIM->GetChemistrySpecialTwinsAssayInfo(
                    sai->iTwinsTestAssayCode, tf::DeviceType::DEVICE_TYPE_C1000);

                spFirstSai = sai;
            }
        }
    }

    if (spFirstSai != nullptr && spSecondSai != nullptr)
    {    
        // 另一个项目的通用参数
        auto spSecondGai = CIM->GetAssayInfo(spSecondSai->assayCode);

        // 另一个项目的生化通用参数
        auto spSecondChgai = CIM->GetChemistryAssayInfo(spSecondSai->assayCode, tf::DeviceType::DEVICE_TYPE_C1000);

        if (spSecondGai == nullptr || spSecondChgai == nullptr)
        {
            OnErr(true, tr("保存失败"));
        }

        QString erText = tr("双项同测参数错误：项目[%1]与项目[%2]的")
            .arg(QString::fromStdString(GAI.assayName)).arg(QString::fromStdString(spSecondGai->assayName));

        // bug11588 双向同测项目样本量（样本量、稀释样本量、稀释液量、混匀等级、稀释液）
        // 试剂量（试剂量、稀释液量、富余量、混匀等级、试剂编号、开瓶有效期）、反应杯清洗设置需一致  
        OnErr(spSecondChgai->reagentAspirateVols[0] != chGAI.reagentAspirateVols[0], 
                erText + tr("试剂量R1不一致"));
        
        // 试剂量若设置R2才检查R2的试剂量是否一致
        if (spSecondChgai->reagentAspirateVols.size() == 2
            && chGAI.reagentAspirateVols.size() == 2
            && spSecondChgai->reagentAspirateVols[1] != chGAI.reagentAspirateVols[1])
        {
            OnErr(true, erText + tr("试剂量R2不一致"));
        }

        // 试剂编号
        OnErr(spSecondChgai->reagentCode != chGAI.reagentCode, erText + tr("试剂编号不一致"));

        // 稀释液
        OnErr(spSecondChgai->diluentCode != chGAI.diluentCode, erText + tr("稀释液不一致"));

        // 反应杯清洗液类型
        OnErr(spSecondChgai->cupDetergentType != chGAI.cupDetergentType, erText + tr("反应杯清洗液不一致"));
        OnErr(spSecondChgai->diluentMixLevel != chGAI.diluentMixLevel, erText + tr("稀释液混匀等级不一致"));

        // 样本量
        OnErr(spFirstSai->sampleAspirateVols != spSecondSai->sampleAspirateVols, erText + tr("样本量不一致"));
    }

    if (ui->fristSampleTypeCombo->count() > 0)
    {
        // 样本优先级
        const auto& specialAssayInfos = CIM->GetChemistrySpecialAssayInfo(
            ui->AssayCodeEdit->text().toInt(), tf::DeviceType::DEVICE_TYPE_C1000);

        for (auto& specialAssayInfo : specialAssayInfos)
        {
            if ((specialAssayInfo->sampleSourceType == ui->fristSampleTypeCombo->currentData().toInt())
                && !specialAssayInfo->priority)
            {
                // 修改样本优先级
                specialAssayInfo->__set_priority(true);
                auto ret = ch::LogicControlProxy::ModifySpecialAssayInfo(*specialAssayInfo);
                if (::tf::ThriftResult::THRIFT_RESULT_SUCCESS != ret)
                {
                    QString errStr(tr("修改样本优先级失败！"));
                    ULOG(LOG_ERROR, errStr.toLocal8Bit().data());
                    return false;
                }
            }
        }
    }

    // 项目名称
    ui->fullNameEdit->setText(QString::fromStdString(GAI.assayFullName));
    // 打印名称
    ui->PrintNameEdit->setText(QString::fromStdString(GAI.printName));
    // 简称
    ui->AssayNameEdit->setText(QString::fromStdString(GAI.assayName));

    return true;
}

bool AnalysisParamWidget::HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item)
{
    int code = ui->AssayCodeEdit->text().toInt();
    if (code <= 0) 
    {
        return true;
    }

    const auto& CIM = CommonInformationManager::GetInstance();
    const auto& spGai = CIM->GetAssayInfo(code);
    if (spGai == nullptr)
    {
        return true;
    }

    // 简称
    if (ui->AssayNameEdit->text().toStdString() != spGai->assayName)
    {
        ULOG(LOG_INFO, "Please save assayName");
        return true;
    }
   
    // 小数位数
    if (ui->DecimalCombo->currentText().toInt() != spGai->decimalPlace)
    {
        ULOG(LOG_INFO, "Please save decimalPlace");
        return true;
    }
    
    // 打印名称
    if (ui->PrintNameEdit->text().toStdString() != spGai->printName)
    {
        ULOG(LOG_INFO, "Please save printName");
        return true;
    }
   
    // 结果单位
    for (auto& ut : spGai->units)
    {
        if (ut.isCurrent && ut.name != ui->ResUnitCombo->currentText().toStdString())
        {
            ULOG(LOG_INFO, "Please save units");
            return true;
        }
    }
    
    // 开瓶有效期
    if (ui->openBottleValidityEdit->text().toInt() != spGai->stableDays)
    {
        ULOG(LOG_INFO, "Please save stableDays");
        return true;
    }

    // 版本号
	std::string curVer = ui->AssayParamVerCombo->currentText().toStdString();
    const auto& verSet = CIM->GetAssayVersionList(item.assayCode, (::tf::DeviceType::type)item.model);
    if (verSet.find(curVer) == verSet.end())
    {
        ULOG(LOG_INFO, "Please save version");
        return true;
    }

    const auto& spChGai = CIM->GetChemistryAssayInfo(code, (::tf::DeviceType::type)item.model, curVer);
    if (spChGai == nullptr)
    {
        ULOG(LOG_INFO, "GetChemistryAssayInfo failed");
        return true;
    }

    // 主波长
    if (spChGai->waveLengths[0] != ui->PrimaryOSVCombo->currentData().toInt())
    {
        ULOG(LOG_INFO, "Please save waveLengths first");
        return true;
    }
    
    // 次波长
    if (spChGai->waveLengths.size() == 2
        && spChGai->waveLengths[1] != ui->SecondaryOSVCombo->currentData().toInt())
    {
        ULOG(LOG_INFO, "Please save waveLengths second");
        return true;
    }

    // 分析方法
    if (ui->AnalysisMethodCombo->currentData().toInt() != spChGai->analysisMethod)
    {
        ULOG(LOG_INFO, "Please save analysisMethod");
        return true;
    }

    // 反应时间
    if (ui->ReactionTimeCombo->currentData().toInt() != spChGai->reactionTime)
    {
        ULOG(LOG_INFO, "Please save reactionTime");
        return true;
    }

    // 测光点
    if (spChGai->analyPoints.m != ui->m_Edit->text().toInt()
        || spChGai->analyPoints.n != ui->n_Edit->text().toInt()
        || spChGai->analyPoints.p != ui->p_Edit->text().toInt()
        || spChGai->analyPoints.q != ui->q_Edit->text().toInt()
        || spChGai->analyPoints.takePrePoint != ui->take_point->currentData().toBool())
    {
        ULOG(LOG_INFO, "Please save analyPoints");
        return true;
    }

    // 反应杯清洗
    if (spChGai->cupDetergentType != ui->ReactionCupWashCombo->currentData())
    {
        ULOG(LOG_INFO, "Please save cupDetergentType");
        return true;
    }

    // 智能计算
    if (ui->checkBox->isChecked() != spChGai->enableIntellectCalc)
    {
        ULOG(LOG_INFO, "Please save enableIntellectCalc");
        return true;
    }

    // 试剂量
    for (int i = 0; i < spChGai->reagentAspirateVols.size(); i++)
    {
        auto &reagentAspInfo = spChGai->reagentAspirateVols.at(i);
        if (get<0>(m_reagentCtrls[i])->text().toInt() * 10 != reagentAspInfo.reagent)
        {
            ULOG(LOG_INFO, "Please save reagentAspInfo.reagent");
            return true;
        }

        if (get<1>(m_reagentCtrls[i])->text().toInt() * 10 != reagentAspInfo.diluent)
        {
            ULOG(LOG_INFO, "Please save reagentAspInfo.diluent");
            return true;
        }

        if (get<2>(m_reagentCtrls[i])->currentData().toInt() != reagentAspInfo.reagentDummy)
        {
            ULOG(LOG_INFO, "Please save reagentAspInfo.reagentDummy");
            return true;
        }

        if (get<3>(m_reagentCtrls[i])->currentData().toInt() != reagentAspInfo.mixingLevel)
        {
            ULOG(LOG_INFO, "Please save reagentAspInfo.mixingLevel");
            return true;
        }       
    }

    if (ui->RegentVolR1Code->text().toInt() != spChGai->reagentCode)
    {
        ULOG(LOG_INFO, "Please save reagentCode");
        return true;
    }

    int curSample = ui->SampleTypeCombo->currentData().toInt();
    const auto& spSais = CIM->GetChemistrySpecialAssayInfo(
        code, (::tf::DeviceType::type)item.model, curSample, curVer);
    if (spSais.size() != 1)
    {
        ULOG(LOG_INFO, "GetChemistrySpecialAssayInfo failed");
        return false;
    }

    const auto& sai = *spSais[0];

    // 分析检查参数
    if (sai.analyChkParam != GetAnalyChkParam())
    {
        ULOG(LOG_INFO, "Please save analyChkParam");
        return true;
    }

    // 样本量
    if (sai.sampleAspirateVols != GetSampleAspirateVols())
    {
        ULOG(LOG_INFO, "Please save sampleAspirateVols");
        return true;
    }

    // 稀释液   
    if (ui->water_radio_Button->isChecked())
    {
        if (spChGai->diluentCode != 0)
        {
            ULOG(LOG_INFO, "Please save diluentCode");
            return true;
        }
    }
    else if (ui->dilution_line_edit->text().toInt() != spChGai->diluentCode)
    {
        ULOG(LOG_INFO, "Please save diluentCode");
        return true;
    }

    if (ui->dilution_mixin_level->currentData().toInt() != spChGai->diluentMixLevel)
    {
        ULOG(LOG_INFO, "Please save diluentMixLevel");
        return true;
    }

    // 双项同测
    if (ui->combTwinsTest->currentIndex() <= 0
        && (sai.emTwinsTestType != ::ch::tf::TwinsTestType::TWINS_TEST_TYPE_NONE
            || sai.iTwinsTestAssayCode != INVALID_ASSAY_CODE))
    {
        ULOG(LOG_INFO, "Please save combTwinsTest");
        return true;
    }   
    else if (ui->combTwinsTest->currentIndex() > 0
        && sai.iTwinsTestAssayCode != ui->combTwinsTest->currentData().toInt())
    {
        ULOG(LOG_INFO, "Please save iTwinsTestAssayCode");
        return true;
    }

    return false;
}

QVector<int> AnalysisParamWidget::GetCurAssaySampleSource()
{
    QVector<int> result;
    for (int i=0; i< ui->SampleTypeCombo->count(); i++)
    {
        result.push_back(ui->SampleTypeCombo->itemData(i).toInt());
    }

    return result;
}

void AnalysisParamWidget::UpdateFirstSampleTypeCommbox(int assayCode, int deviceType, const string& version)
{
    ui->fristSampleTypeCombo->clear();

    QString fristSampleType;
    std::set<int> sampleTypeSet;

    ::ch::tf::SpecialAssayInfoQueryResp saiQr;
    ::ch::tf::SpecialAssayInfoQueryCond saiQc;
    saiQc.__set_assayCode(assayCode);
    saiQc.__set_deviceType((::tf::DeviceType::type)deviceType);
    saiQc.__set_version(version);
    ch::LogicControlProxy::QuerySpecialAssayInfo(saiQr, saiQc);

    for (auto& sai : saiQr.lstSpecialAssayInfos)
    {
        if (sai.priority)
        {
            fristSampleType = ConvertTfEnumToQString((::tf::SampleSourceType::type)sai.sampleSourceType);
        }

        sampleTypeSet.insert(sai.sampleSourceType);
    }

    UpdateSampleTypeToCommbox(ui->fristSampleTypeCombo, sampleTypeSet);

    ui->fristSampleTypeCombo->setCurrentText(fristSampleType);
}

void AnalysisParamWidget::UpdateVersionCommbox(int oldVersion, int newVersion)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_bVersionFlag = false;

    for (int i = 0; i < ui->AssayParamVerCombo->count(); i++)
    {
        if (ui->AssayParamVerCombo->itemData(i).toInt() == oldVersion)
        {
            ui->AssayParamVerCombo->setItemData(i, newVersion);
            ui->AssayParamVerCombo->setItemText(i, QString::number(newVersion));
            break;
        }
    }

    ui->AssayParamVerCombo->setCurrentText(QString::number(newVersion));
    m_bVersionFlag = true;
}
