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

#include "TrkDQIDebugWgt.h"
#include "TrkStepModuleWgt.h"
#include "TrkMagnetModuleWgt.h"
#include "TrkScanModuleWgt.h"
#include "ui_TrkDQIDebugWgt.h"
#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"

TrkDQIDebugWgt::TrkDQIDebugWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TrkDQIDebugWgt)
    , m_isInit(false)
{
    ui->setupUi(this);

    InitUi();
    InitData();
    InitConnect();
}

TrkDQIDebugWgt::~TrkDQIDebugWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

void TrkDQIDebugWgt::showEvent(QShowEvent *event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果不是第一次显示，则刷新界面
    if (m_isInit)
    {
        // 刷新表格
        UpdateUi();
    }
    else
    {
        m_isInit = true;
    }
}

void TrkDQIDebugWgt::OnSelectListItem()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->ModuleLstTbl->currentIndex();
    if (!selIdx.isValid() || ui->ModuleLstTbl->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 记录索引
    DQI::StackPageIdx enPageIdx = DQI::STACK_PAGE_IDX_STEP;
    QString pageName = ui->ModuleLstTbl->item(selIdx.row(), 0)->text();
    if(pageName == "电机")
    {
        enPageIdx = DQI::STACK_PAGE_IDX_STEP;
    }
    else if(pageName == "电磁铁")
    {
        enPageIdx = DQI::STACK_PAGE_IDX_ELECTROMAGNET;
    }
    else if(pageName == "扫码器")
    {
        enPageIdx = DQI::STACK_PAGE_IDX_SCANNER;
    }
    ui->stackedWidget_center->setCurrentIndex(enPageIdx);

}

void TrkDQIDebugWgt::InitUi()
{
    // 只有一列
    ui->ModuleLstTbl->setColumnCount(1);

    // 表头
    QStringList strHeaderList;
    strHeaderList << tr("节点调试");
    ui->ModuleLstTbl->setHorizontalHeaderLabels(strHeaderList);
    ui->ModuleLstTbl->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->ModuleLstTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ModuleLstTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ModuleLstTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ModuleLstTbl->horizontalHeader()->setStretchLastSection(true);

    ResizeTblColToContent(ui->ModuleLstTbl);
}

void TrkDQIDebugWgt::InitData()
{
    m_stepModuleWgt = new TrkStepModuleWgt(this);
    m_magnetModuleWgt = new TrkMagnetModuleWgt(this);
    m_scanModuleWgt = new TrkScanModuleWgt(this);
    ui->stackedWidget_center->insertWidget(DQI::STACK_PAGE_IDX_STEP, m_stepModuleWgt);
    ui->stackedWidget_center->insertWidget(DQI::STACK_PAGE_IDX_ELECTROMAGNET, m_magnetModuleWgt);
    ui->stackedWidget_center->insertWidget(DQI::STACK_PAGE_IDX_SCANNER, m_scanModuleWgt);
}

void TrkDQIDebugWgt::InitConnect()
{
    // 选中项改变
    QItemSelectionModel* pSelModel = ui->ModuleLstTbl->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, &QItemSelectionModel::selectionChanged, this, &TrkDQIDebugWgt::OnSelectListItem);
    }
    //注册消息

}

void TrkDQIDebugWgt::UpdateUi()
{

}

void TrkDQIDebugWgt::UpdateModuleList(ModuleInfo& info)
{
	//列表新建项
	ui->ModuleLstTbl->insertRow(0);
	ui->ModuleLstTbl->insertRow(1);
	ui->ModuleLstTbl->insertRow(2);

    // 将模块加载到列表中
    auto motorItem = new QTableWidgetItem("电机");
    ui->ModuleLstTbl->setItem(0, 0, motorItem);

    auto magnetItem = new QTableWidgetItem("电磁铁");
    ui->ModuleLstTbl->setItem(1, 0, magnetItem);

    auto scanItem = new QTableWidgetItem("扫码器");
    ui->ModuleLstTbl->setItem(2, 0, scanItem);

    m_stepModuleWgt->UpdateUi(info.nodeProperty, info.motors);
    m_magnetModuleWgt->UpdateUi(info.nodeProperty, info.electromagnets);
    m_scanModuleWgt->UpdateUi(info.nodeProperty, info.scanners);

}

