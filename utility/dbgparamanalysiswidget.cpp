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
/// @file     dbgparamanalysiswidget.cpp
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
#include "dbgparamanalysiswidget.h"
#include "ui_dbgparamanalysiswidget.h"
#include <QStandardItemModel>

#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CReadOnlyDelegate.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "UtilityCommonFunctions.h"

#include "src/common/Mlog/mlog.h"


#define  DEFAULT_EMPTY_ROW_CNT 3 // 默认空行数


// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

DbgParamAnalysisWidget::DbgParamAnalysisWidget(QWidget *parent)
	: QWidget(parent),
	  m_bInit(false)
{
	ui = new Ui::DbgParamAnalysisWidget();
	ui->setupUi(this);
    m_tabModel = new QStandardItemModel();
    m_tabModel2 = new QStandardItemModel();

    // 项目名/项目编号/稀释液编号不允许修改
    ui->AssayNaneEdit->setDisabled(true);
    ui->AssayCodeEdit_3->setDisabled(true);
    ui->DltIdEdit->setDisabled(true);
    ui->FirAddRgntVolEdit->setDisabled(true);
    ui->SndAddRgntVolEdit->setDisabled(true);

    // 初始化下拉框
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_FAST), ::im::tf::reactPhaseType::REACT_PHASE_FAST);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_ONE), ::im::tf::reactPhaseType::REACT_PHASE_ONE);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_TWO), ::im::tf::reactPhaseType::REACT_PHASE_TWO);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_FTWO), ::im::tf::reactPhaseType::REACT_PHASE_FTWO);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_PRE), ::im::tf::reactPhaseType::REACT_PHASE_PRE);

    // 监听反应方式改变
    ui->FastModeCB->setEnabled(false);
    connect(ui->ReactionMethodCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnReactionMethodChanged()));

    ui->RgntParamTbl->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
    ui->RgntParamTbl_2->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
}

DbgParamAnalysisWidget::~DbgParamAnalysisWidget()
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
void DbgParamAnalysisWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置表头
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<RgntParamHeader, QString>> qcApplyHeader = {
        { Rph_RgntSubPos, tr("试剂子位置") },
        { Rph_RgntVol, tr("试剂量(ul)") }
    };
    QStringList strHeaderList;
    for (auto it = qcApplyHeader.begin(); it != qcApplyHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    m_tabModel->setHorizontalHeaderLabels(strHeaderList);
    m_tabModel2->setHorizontalHeaderLabels(strHeaderList);
    ui->RgntParamTbl->setModel(m_tabModel);
    ui->RgntParamTbl_2->setModel(m_tabModel2);

    // 表头
    ui->RgntParamTbl->verticalHeader()->setVisible(false);
    ui->RgntParamTbl_2->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->RgntParamTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->RgntParamTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->RgntParamTbl->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    ui->RgntParamTbl_2->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->RgntParamTbl_2->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->RgntParamTbl_2->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    // 列宽自适应
    ResizeTblColToContent(ui->RgntParamTbl);
    ResizeTblColToContent(ui->RgntParamTbl_2);
}

///
/// @brief 窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void DbgParamAnalysisWidget::showEvent(QShowEvent *event)
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

bool DbgParamAnalysisWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->inherits("QComboBox") && event->type() == QEvent::Wheel)
    {
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

///
/// @brief
///     反应方式改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月11日，新建函数
///
void DbgParamAnalysisWidget::OnReactionMethodChanged()
{
    QVariant curData = ui->ReactionMethodCombo->currentData();
    if (!curData.isValid())
    {
        ui->FastModeCB->setChecked(false);
        return;
    }

    // 快速模式是则选中
    ui->FastModeCB->setChecked(int(::im::tf::reactPhaseType::REACT_PHASE_FAST) == curData.toInt());
}

void DbgParamAnalysisWidget::CleanImParameters()
{
    ui->DefaultDltRatioEdit->clear();
    ui->ReactionMethodCombo->setCurrentIndex(-1);
    ui->FirAddRgntVolEdit->clear();
    ui->SndAddRgntVolEdit->clear();
    ui->MaxDltRatioEdit->clear();
    ui->SampVolEdit->clear();
    ui->DltIdEdit->clear();

    utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl, m_tabModel, 0, {});
    utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl_2, m_tabModel2, 0, {});

    ui->FastModeCB->setChecked(false);
    ui->FirstWashCB->setChecked(false);
}

void DbgParamAnalysisWidget::LoadAnalysisParam(int iAssayCode)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, iAssayCode);

    // 清除分析参数UI上的Ctrl的值
    CleanImParameters();

    std::shared_ptr<CommonInformationManager> comIns = CommonInformationManager::GetInstance();
    std::shared_ptr<tf::GeneralAssayInfo> generalInfo = comIns->GetAssayInfo(iAssayCode);
    if (generalInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null general assayinfo.");
        return;
    }

    // 初始化界面数据
    // 项目名
    ui->AssayNaneEdit->setText(QString::fromStdString(generalInfo->assayName));
    // 项目编号
    ui->AssayCodeEdit_3->setText(QString::number(iAssayCode));

    std::shared_ptr<::im::tf::GeneralAssayInfo> imAssay = comIns->GetImmuneAssayInfo(iAssayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
    if (imAssay == nullptr)
    {
        ULOG(LOG_WARN, "Null im general assayinfo.");
        return;
    }
    if (imAssay->__isset.reactInfo)
    {
        int stepIdx1 = imAssay->reactInfo.reactPhase == im::tf::reactPhaseType::type::REACT_PHASE_PRE ? 3 : 1;
        int stepIdx2 = stepIdx1 + 1;

        // 默认稀释倍数
        ui->DefaultDltRatioEdit->setText(QString::number(imAssay->reactInfo.dilutionRatio));

        // 反应方式
        ui->ReactionMethodCombo->setCurrentText(ThriftEnumTrans::GetTestWayIm(imAssay->reactInfo.reactPhase));

        // 吸样列表
        utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl, m_tabModel, stepIdx1, imAssay->reactInfo.reactInfo);
        utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl_2, m_tabModel2, stepIdx2, imAssay->reactInfo.reactInfo);

        bool bFirstWash(false);
        // 加试剂量, 第一步加试剂总量，如果有suck两次，则两次的和
        for (const auto& stp : imAssay->reactInfo.reactInfo)
        {
            // 第一步
            if (stp.reactStep == stepIdx1)
            {
                int iVol = utilcomm::GetSuckVol(stp.listSuckReagents);
                ui->FirAddRgntVolEdit->setText(iVol == -1 ? "" : QString::number(iVol));
                bFirstWash = stp.reactAfterWash;
            }
            // 第二步
            if (stp.reactStep == stepIdx2)
            {
                int iVol = utilcomm::GetSuckVol(stp.listSuckReagents);
                ui->SndAddRgntVolEdit->setText(iVol == -1 ? "" : QString::number(iVol));
            }
        }
        // 最大稀释倍数
        ui->MaxDltRatioEdit->setText(QString::number(imAssay->reactInfo.maxDilutionRatio));
        // 加样量
        ui->SampVolEdit->setText(QString::number(imAssay->reactInfo.suckVol));
        // 是否是快速模式
        ui->FastModeCB->setChecked(imAssay->reactInfo.reactPhase == ::im::tf::reactPhaseType::REACT_PHASE_FAST);

        // 第一次清洗; TODO:confirm: 第一次吸样后清洗？
        ui->FirstWashCB->setChecked(bFirstWash);
        // 稀释液ID
        ui->DltIdEdit->setText(QString::number(imAssay->reactInfo.dilutionNumber));
    }
}

bool DbgParamAnalysisWidget::SaveAnalysisInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int assayCode = ui->AssayCodeEdit_3->text().toInt();

    std::shared_ptr<CommonInformationManager> comIns = CommonInformationManager::GetInstance();
    std::shared_ptr<::im::tf::GeneralAssayInfo> imAssayVm = comIns->GetImmuneAssayInfo(assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
    if (imAssayVm == nullptr)
    {
        ULOG(LOG_WARN, "Null im general assayinfo.");
        return false;
    }
    int stepIdx1 = imAssayVm->reactInfo.reactPhase == im::tf::reactPhaseType::type::REACT_PHASE_PRE ? 3 : 1;
    int stepIdx2 = stepIdx1 + 1;
    // 拷贝成临时对象
    std::shared_ptr<::im::tf::GeneralAssayInfo> tempImAssay = std::make_shared<::im::tf::GeneralAssayInfo>(*imAssayVm);

    // 获取界面值
    {
        // 第一步加试剂数量,TODO:
        int iFirstStepReagVol = ui->FirAddRgntVolEdit->text().toInt();
        // 默认稀释倍数
        tempImAssay->reactInfo.dilutionRatio = ui->DefaultDltRatioEdit->text().toInt();
        // 反应方式
        int iReactWay = ThriftEnumTrans::TransTestWayIm(ui->ReactionMethodCombo->currentText());
        if (iReactWay != -1)
        {
            tempImAssay->reactInfo.reactPhase = (::im::tf::reactPhaseType::type)iReactWay;
        }
        // 试剂腔加样量
        utilcomm::GetReagentSuckFromTb(ui->RgntParamTbl, m_tabModel, stepIdx1, tempImAssay->reactInfo.reactInfo);
        utilcomm::GetReagentSuckFromTb(ui->RgntParamTbl_2, m_tabModel2, stepIdx2, tempImAssay->reactInfo.reactInfo);

        // 第二步加试剂数量,TODO:
        int iSecondStepReagVol = ui->SndAddRgntVolEdit->text().toInt();
        // 最大稀释倍数
        tempImAssay->reactInfo.maxDilutionRatio = ui->MaxDltRatioEdit->text().toInt();
        // 加样量
        tempImAssay->reactInfo.suckVol = ui->SampVolEdit->text().toInt();
    }

    // 对每个版本参数应用最新设置
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssay;
    comIns->GetImmuneAssayInfos(vecImAssay, assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
    bool bFirstWash(ui->FirstWashCB->isChecked());
    for (const auto& imAssay : vecImAssay)
    {
        if (imAssay->reactInfo.reactPhase == im::tf::reactPhaseType::type::REACT_PHASE_PRE)
        {
            continue;
        }
        std::shared_ptr<im::tf::GeneralAssayInfo> tempCur = std::make_shared<im::tf::GeneralAssayInfo>(*imAssay);
        tempCur->reactInfo.__set_dilutionRatio(tempImAssay->reactInfo.dilutionRatio);
        tempCur->reactInfo.__set_reactPhase(tempImAssay->reactInfo.reactPhase);
        tempCur->reactInfo.__set_reactInfo(tempImAssay->reactInfo.reactInfo);
        tempCur->reactInfo.__set_maxDilutionRatio(tempImAssay->reactInfo.maxDilutionRatio);
        tempCur->reactInfo.__set_suckVol(tempImAssay->reactInfo.suckVol);

        for (auto& stp : tempCur->reactInfo.reactInfo)
        {
            // 第一步
            if (stp.reactStep != stepIdx1)
            {
                continue;
            }

            stp.__set_reactAfterWash(bFirstWash);
        }

        if (!im::LogicControlProxy::ModifyGeneralAssayInfo(*tempCur))
        {
            ULOG(LOG_WARN, "Failed to save imAssayinfo settings.");
            continue;
        }
        *imAssay = *tempCur;
    }

    return true;
}
