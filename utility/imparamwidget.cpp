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
/// @file     assayparamwidget.cpp
/// @brief    应用界面->参数免疫
///
/// @author   4170/TangChuXian
/// @date     2020年5月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "imparamwidget.h"
#include "ui_imparamwidget.h"

#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CReadOnlyDelegate.h"

#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"

ImParamWidget::ImParamWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::ImParamWidget),
    m_bInit(false)
{
    ui->setupUi(this);

    // 初始化Tab页面; 设置表头
    ui->AssayTable->verticalHeader()->setVisible(false);
    ui->AssayTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->AssayTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->AssayTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_imSortModel = new QUtilitySortFilterProxyModel(this);
    m_imDataModel = new AssayListModel(ui->AssayTable);
    m_imDataModel->SetSortModel(m_imSortModel);
    m_imSortModel->setSourceModel(m_imDataModel);
    ui->AssayTable->setModel(m_imSortModel);
    m_imSortModel->SetTableViewSort(ui->AssayTable, { (int)AssayListModel::ASSAY_LIST_COL::NAME });
    // 还原默认顺序
    m_imSortModel->sort(-1, Qt::DescendingOrder);
    ui->AssayTable->sortByColumn(-1, Qt::DescendingOrder);

    ui->AssayTable->setColumnWidth(0, 100);
    ui->AssayTable->setColumnWidth(1, 259);

    // 设置标签鼠标事件穿透
    ui->show_label->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 监听项目编码管理器信息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, UpdateAssayTableData);

    // 保存按钮被点击
    connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    // 查询数据并填充到表格中
    UpdateAssayTableData();
}

ImParamWidget::~ImParamWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

void ImParamWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 测试项目表选中项改变
    connect(ui->AssayTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &ImParamWidget::OnSelectedAssayChanged);

    // 默认选中第一行
    if (m_imSortModel != nullptr && m_imSortModel->rowCount() > 0)
    {
        ui->AssayTable->selectRow(0);
    }
}

void ImParamWidget::UpdateAssayTableData()
{
    std::shared_ptr<CommonInformationManager> comIns = CommonInformationManager::GetInstance();
    ImAssayIndexCodeMaps imAssayCode = comIns->GetImAssayIndexCodeMaps();

    std::set<int> uniqCode;
    std::vector<AssayListModel::StAssayListRowItem> vecData;
    for (ImAssayIndexCodeMaps::iterator it = imAssayCode.begin(); it != imAssayCode.end(); ++it)
    {
        std::shared_ptr<::tf::GeneralAssayInfo> gAssay = comIns->GetAssayInfo(it->first);
        if (gAssay == nullptr)
        {
            continue;
        }

        // 排重
        if (uniqCode.find(it->first) != uniqCode.end())
        {
            continue;
        }
        uniqCode.insert(it->first);

        // 填充显示数据
        AssayListModel::StAssayListRowItem item;
        item.assayCode = it->first;
        item.model = it->second->deviceType;
        item.version = QString::fromStdString(it->second->version);
        item.sampleType = it->second->sampleSource;
        item.assayClassify = gAssay->assayClassify;
        item.name = QString::fromStdString(gAssay->assayName.empty() ? gAssay->assayFullName : gAssay->assayName);
        vecData.push_back(item);
    }

    m_imDataModel->SetData(vecData);
}

void ImParamWidget::showEvent(QShowEvent *event)
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

void ImParamWidget::OnSelectedAssayChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (!current.isValid())
    {
        ULOG(LOG_WARN, "Invalid current index.");
        return;
    }
    
    int iAssayCode = m_imDataModel->GetRowAssayCode(m_imSortModel->mapToSource(current).row());
    if (iAssayCode == INVALID_ASSAY_CODE)
    {
        ULOG(LOG_WARN, "Invalid assaycode.");
        return;
    }

    ui->AssayParamTabWgt->LoadAssayInfo(iAssayCode);
}

void ImParamWidget::OnSaveBtnClicked()
{
    ui->AssayParamTabWgt->SaveAssayInfo();
}
