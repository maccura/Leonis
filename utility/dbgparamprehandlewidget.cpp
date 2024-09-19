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
/// @file     dbgparamprehandlewidget.cpp
/// @brief    应用界面->工程师调试->参数（免疫）预处理
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
#include "dbgparamprehandlewidget.h"
#include "ui_dbgparamprehandlewidget.h"
#include <QStandardItemModel>

#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CommonInformationManager.h"
#include "shared/CReadOnlyDelegate.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "UtilityCommonFunctions.h"
#include "src/common/Mlog/mlog.h"

#define DEFAULT_EMPTY_ROW_CNT 8 // 空行数


// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

DbgParamPrehandleWidget::DbgParamPrehandleWidget(QWidget *parent)
    : QWidget(parent),
    m_bInit(false)
{
    ui = new Ui::DbgParamPrehandleWidget();
    ui->setupUi(this);
    m_tbModel = new QStandardItemModel();
    m_tbModel2 = new QStandardItemModel();

    ui->AssayNaneEdit->setDisabled(true);
    ui->AssayCodeEdit_3->setDisabled(true);
    ui->DltIdEdit->setDisabled(true);
    ui->WhetherTubeEdit->setDisabled(true);
    ui->FirAddRgntVolEdit->setDisabled(true);
    ui->SndAddRgntVolEdit->setDisabled(true);

    // 初始化下拉框
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_FAST), ::im::tf::reactPhaseType::REACT_PHASE_FAST);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_ONE), ::im::tf::reactPhaseType::REACT_PHASE_ONE);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_TWO), ::im::tf::reactPhaseType::REACT_PHASE_TWO);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_FTWO), ::im::tf::reactPhaseType::REACT_PHASE_FTWO);
    ui->ReactionMethodCombo->addItem(ThriftEnumTrans::GetTestWayIm(::im::tf::reactPhaseType::REACT_PHASE_PRE), ::im::tf::reactPhaseType::REACT_PHASE_PRE);
    ui->ReactionMethodCombo->setEnabled(false);

    ui->RgntParamTbl->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
    ui->RgntParamTbl_2->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
}

DbgParamPrehandleWidget::~DbgParamPrehandleWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

void DbgParamPrehandleWidget::LoadAnalysisParam(int iAssayCode)
{
    CleanUi();

    std::shared_ptr<CommonInformationManager> insCom = CommonInformationManager::GetInstance();
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
    insCom->GetImmuneAssayInfos(vecImAssayInfo, iAssayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);

    for (const auto& it : vecImAssayInfo)
    {
        // 预处理试剂
        if (it->reactInfo.reactPhase == im::tf::reactPhaseType::type::REACT_PHASE_PRE)
        {
            // 项目名
            std::shared_ptr<::tf::GeneralAssayInfo> pGeneral = insCom->GetAssayInfo(iAssayCode);
            if (pGeneral != nullptr)
            {
                ui->AssayNaneEdit->setText(QString::fromStdString(pGeneral->assayName));
            }
            // 通道号
            ui->AssayCodeEdit_3->setText(QString::number(iAssayCode));

            // 吸试剂量列表
            utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl, m_tbModel, 1, it->reactInfo.reactInfo);
            utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl_2, m_tbModel2, 2, it->reactInfo.reactInfo);

            for (const auto& stp : it->reactInfo.reactInfo)
            {
                // 第一步试剂加样数
                if (stp.reactStep == 1)
                {
                    int iVol = utilcomm::GetSuckVol(stp.listSuckReagents);
                    ui->FirAddRgntVolEdit->setText(iVol == -1 ? "" : QString::number(iVol));
                }
                // 第二步
                if (stp.reactStep == 2)
                {
                    int iVol = utilcomm::GetSuckVol(stp.listSuckReagents);
                    ui->SndAddRgntVolEdit->setText(iVol == -1 ? "" : QString::number(iVol));
                }
            }
            // 默认稀释倍数
            ui->DefaultDltRatioEdit->setText(QString::number(it->reactInfo.dilutionRatio));
            // 反应方式
            ui->ReactionMethodCombo->setCurrentText(ThriftEnumTrans::GetTestWayIm(it->reactInfo.reactPhase));
            // 最大稀释倍数
            ui->MaxDltRatioEdit->setText(QString::number(it->reactInfo.maxDilutionRatio));
            // 加样量
            ui->SampVolEdit->setText(QString::number(it->reactInfo.suckVol));
            // 稀释液ID
            ui->DltIdEdit->setText(QString::number(it->reactInfo.dilutionNumber));
            // 是否转管
            ui->WhetherTubeEdit->setText((iAssayCode > 1950 && iAssayCode <= 2000) ? tr("是") : tr("否"));

            break;
        }
    }
}

bool DbgParamPrehandleWidget::IsExistPreAssay(int iAssayCode)
{
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
    CommonInformationManager::GetInstance()->GetImmuneAssayInfos(vecImAssayInfo, iAssayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);

    for (const auto& it : vecImAssayInfo)
    {
        // 预处理试剂
        if (it->reactInfo.reactPhase == im::tf::reactPhaseType::type::REACT_PHASE_PRE)
        {
            return true;
        }
    }

    return false;
}

bool DbgParamPrehandleWidget::SavePrehandleInfo()
{
    int iAssayCode = ui->AssayCodeEdit_3->text().toInt();

    std::shared_ptr<CommonInformationManager> insCom = CommonInformationManager::GetInstance();
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
    insCom->GetImmuneAssayInfos(vecImAssayInfo, iAssayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);

    // 使用预处理项目作为临时容器
    std::shared_ptr<im::tf::GeneralAssayInfo> tempAss = nullptr;
    for (auto& ti : vecImAssayInfo)
    {
        if (ti->reactInfo.reactPhase == im::tf::reactPhaseType::type::REACT_PHASE_PRE)
        {
            tempAss = std::make_shared<im::tf::GeneralAssayInfo>(*ti);
        }
    }
    if (tempAss == nullptr)
    {
        return false;
    }

    // 获取UI设置的值
    {
        // 第一步加试剂数量，TODO
        int firstStepVol = ui->FirAddRgntVolEdit->text().toInt();
        // 默认稀释倍数
        tempAss->reactInfo.__set_dilutionRatio(ui->DefaultDltRatioEdit->text().toInt());
        // 反应方式
        int iReactWay = ThriftEnumTrans::TransTestWayIm(ui->ReactionMethodCombo->currentText());
        if (iReactWay != -1)
        {
            tempAss->reactInfo.__set_reactPhase((::im::tf::reactPhaseType::type)iReactWay);
        }
        // 试剂腔吸量
        utilcomm::GetReagentSuckFromTb(ui->RgntParamTbl, m_tbModel, 1, tempAss->reactInfo.reactInfo);
        utilcomm::GetReagentSuckFromTb(ui->RgntParamTbl_2, m_tbModel2, 2, tempAss->reactInfo.reactInfo);

        // 第二步加试剂数量，TODO
        int secondStepVol = ui->SndAddRgntVolEdit->text().toInt();
        // 最大稀释倍数
        tempAss->reactInfo.__set_maxDilutionRatio(ui->MaxDltRatioEdit->text().toInt());
        // 加样量
        tempAss->reactInfo.__set_suckVol(ui->SampVolEdit->text().toInt());
    }

    // 对每个版本参数应用最新设置
    for (const auto& imAssay : vecImAssayInfo)
    {
        if (imAssay->reactInfo.reactPhase != im::tf::reactPhaseType::type::REACT_PHASE_PRE)
        {
            continue;
        }
        std::shared_ptr<im::tf::GeneralAssayInfo> tempCur = std::make_shared<im::tf::GeneralAssayInfo>(*imAssay);
        tempCur->reactInfo.__set_dilutionRatio(tempAss->reactInfo.dilutionRatio);
        tempCur->reactInfo.__set_reactPhase(tempAss->reactInfo.reactPhase);
        tempCur->reactInfo.__set_reactInfo(tempAss->reactInfo.reactInfo);
        tempCur->reactInfo.__set_maxDilutionRatio(tempAss->reactInfo.maxDilutionRatio);
        tempCur->reactInfo.__set_suckVol(tempAss->reactInfo.suckVol);
        if (!im::LogicControlProxy::ModifyGeneralAssayInfo(*tempCur))
        {
            ULOG(LOG_WARN, "Failed to save imAssayinfo settings.");
            continue;
        }
        *imAssay = *tempCur;
    }
    return true;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void DbgParamPrehandleWidget::InitAfterShow()
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
    m_tbModel->setHorizontalHeaderLabels(strHeaderList);
    m_tbModel2->setHorizontalHeaderLabels(strHeaderList);
    ui->RgntParamTbl->setModel(m_tbModel);
    ui->RgntParamTbl_2->setModel(m_tbModel2);

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
void DbgParamPrehandleWidget::showEvent(QShowEvent *event)
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

bool DbgParamPrehandleWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->inherits("QComboBox") && event->type() == QEvent::Wheel)
    {
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void DbgParamPrehandleWidget::CleanUi()
{
    ui->AssayNaneEdit->clear();
    ui->FirAddRgntVolEdit->clear();
    ui->DefaultDltRatioEdit->clear();
    ui->AssayCodeEdit_3->clear();

    ui->ReactionMethodCombo->setCurrentIndex(-1);
    ui->SndAddRgntVolEdit->clear();
    ui->MaxDltRatioEdit->clear();
    ui->SampVolEdit->clear();

    utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl, m_tbModel, 0, {});
    utilcomm::AssignReagentSuckTb(this, ui->RgntParamTbl_2, m_tbModel2, 0, {});

    ui->DltIdEdit->clear();
    ui->WhetherTubeEdit->clear();
}
