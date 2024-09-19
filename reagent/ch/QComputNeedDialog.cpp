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
/// @file     QComputNeedDialog.cpp
/// @brief    需求计算对话框
///
/// @author   5774/WuHongTao
/// @date     2022年3月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QComputNeedDialog.h"
#include "ui_QComputNeedDialog.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QDate>
#include "ReagentOverview.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/ChLogicControlProxy.h"

QComputNeedDialog::QComputNeedDialog(QWidget *parent)
    : BaseDlg(parent),
    m_ComputView(nullptr),
    m_ComputNeedMode(nullptr)
{
    ui = new Ui::QComputNeedDialog();
    ui->setupUi(this);
    ResetNeedtable();
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->Save_Button, SIGNAL(clicked()), this, SLOT(OnSave()));
    // 获取数据总览信息
    assert(static_cast<ReagentOverview*>(parent) != nullptr);
    static_cast<ReagentOverview*>(parent)->GetOverviewData(m_OverViewDatas);
    InitTable();
    Init();
    SetTitleName(tr("ComputNeed"));
}

QComputNeedDialog::~QComputNeedDialog()
{
}

///
/// @brief
///     初始化需求表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
void QComputNeedDialog::ResetNeedtable()
{
    if (m_ComputNeedMode == nullptr)
    {
        m_ComputNeedMode = new QStandardItemModel(this);
    }

    m_ComputNeedMode->clear();
    QStringList needDialogHeader;
    needDialogHeader << tr("Assay") << tr("dynamicCalc") << tr("Need") << tr("Residual")
        << tr("Demand") << tr("ResidualMap");
    m_ComputNeedMode->setHorizontalHeaderLabels(needDialogHeader);
    m_ComputNeedMode->setRowCount(30);
}

///
/// @brief
///     初始化页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
void QComputNeedDialog::InitTable()
{
    QStringList nameOfweek;
    nameOfweek << tr("Monday") << tr("Tuesday") << tr("Wednesday") << tr("Thursday")
        << tr("Friday") << tr("Saturday") << tr("Sunday");

    m_ComputViewList.clear();
    for (const QString& day : nameOfweek)
    {
        QTableView* ComputView = new QTableView(this);
		ComputView->setObjectName(day);
        ComputView->setModel(m_ComputNeedMode);
        ComputView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ComputView->setSelectionMode(QAbstractItemView::SingleSelection);
		ComputView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        connect(ComputView, &QTableView::doubleClicked, this, [=](QModelIndex index) {m_IndexList.push_back(index);});
        SetTabviewColumnEdit(ComputView, 2);
        m_ComputViewList.push_back(ComputView);
    }
}

///
/// @brief
///     初始化
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
void QComputNeedDialog::Init()
{
    QStringList nameOfweek;
    nameOfweek << tr("Monday") << tr("Tuesday") << tr("Wednesday") << tr("Thursday")
        << tr("Friday") << tr("Saturday") << tr("Sunday");

    // 依次添加页面(周一到周五)
    int index = 0;
    for (const QString& name : nameOfweek)
    {
        m_ComputViewList[index]->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
        ui->tabWidget->insertTab(index, m_ComputViewList[index], name);
        index++;
    }

    // 跳转到不同日期的试剂计算量
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(OnShowCurrentDayReagent(int)));
    // 初始化为星期1
    emit ui->tabWidget->currentChanged(0);
}

///
/// @brief
///     通过项目编号获取，对应试剂的总览信息
///
/// @param[in]  assaycode  项目编号
///
/// @return 试剂信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
bool QComputNeedDialog::GetReagentDataOverviewByCode(ch::tf::ReagentOverview& data, int assaycode)
{
    bool ret = false;
    // 总览信息
    for (const auto& dataView : m_OverViewDatas)
    {
        if (dataView.assayCode == assaycode)
        {
            data = dataView;
            ret = true;
            break;
        }
    }

    return ret;
}

///
/// @brief
///     设置表格哪一列能修改
///
/// @param[in]  tab  表格
/// @param[in]  index  列
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
void QComputNeedDialog::SetTabviewColumnEdit(QTableView* tab, int index)
{
    for (int i = 0 ; i < tab->model()->columnCount(); i++)
    {
        if (i == index)
        {
            continue;
        }
        tab->setItemDelegateForColumn(i, new CReadOnlyDelegate(this));
    }
}

///
/// @brief
///     显示当前日期的试剂需求量
///
/// @param[in]  index  日期序号
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
void QComputNeedDialog::OnShowCurrentDayReagent(int index)
{
    // 重新设置表
    ResetNeedtable();
    m_IndexList.clear();
    ui->Warning_Widget->clear();
    ch::tf::ReagentNeedQueryCond reagentNeedCond;
    // 设置星期几
    reagentNeedCond.__set_dayOfWeek(index);
    ch::tf::ReagentNeedQueryResp reagentNeedResp;
    ch::LogicControlProxy::QueryReagentNeed(reagentNeedResp, reagentNeedCond);

    if (reagentNeedResp.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS || reagentNeedResp.lstReagentNeed.empty())
    {
        return;
    }

    // 不能满足要求的项目数据
    int UnreachableNumber = 0;
    m_row = 0;
    int column = 0;
    // 依次处理需求数据
    for (const auto& need : reagentNeedResp.lstReagentNeed)
    {
        column = 0;
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(need.assayCode);
        if (Q_NULLPTR == spAssayInfo)
        {
            continue;
        }

        // 项目名称
        m_ComputNeedMode->setItem(m_row, column++, new QStandardItem(QString::fromStdString(spAssayInfo->assayName)));
        // 动态计算
        m_ComputNeedMode->setItem(m_row, column++, new QStandardItem(QString::number(need.dynamicCalc)));
        // 用户需求量
        QStandardItem* item = new QStandardItem(QString::number(need.need));
        // 保存数据Id
        item->setData(static_cast<qlonglong>(need.id));
        m_ComputNeedMode->setItem(m_row, column++, item);

        ch::tf::ReagentOverview dataOverView;
        if (!GetReagentDataOverviewByCode(dataOverView, need.assayCode))
        {
            continue;
        }
        // 当前余量
        m_ComputNeedMode->setItem(m_row, column++, new QStandardItem(QString::number(dataOverView.residual)));

        // 大于0才显示
        column++;
        if ((need.need - dataOverView.residual) > 0)
        {
            // 需求缺口
            UnreachableNumber++;
            m_ComputNeedMode->setItem(m_row, column, new QStandardItem(QString::number(need.need - dataOverView.residual)));
        }



        // 模块余量分布
        QString moduleShowMessage;
        for (const auto& moduleData : dataOverView.deviceResiduals)
        {
            moduleShowMessage += QString::fromStdString(" " + moduleData.first + ":") + QString::number(moduleData.second);
        }

        // 模块余量分布
        m_ComputNeedMode->setItem(m_row, column++, new QStandardItem(moduleShowMessage));
        m_row++;
    }

    // 显示多少个项目没有达标
    if (UnreachableNumber > 0)
    {
        ui->Warning_Widget->setText(QString("%1 project unreachable").arg(UnreachableNumber));
    }

    // 设置当前日期
    ui->Time_Widget->setText(QDate::currentDate().toString());
}

///
/// @brief
///     保存修改数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月17日，新建函数
///
void QComputNeedDialog::OnSave()
{
    if (m_IndexList.empty())
    {
        return;
    }

    // 数据修改列表
    std::vector<ch::tf::ReagentNeed> modifyNeedList;
    // 获取修改的行数
    for (const auto& index : m_IndexList)
    {
        // 行数大于数据的时候
        if (index.row() >= m_row)
        {
            continue;
        }

        QStandardItem* item = m_ComputNeedMode->item(index.row(), index.column());
        if (item == nullptr || item->text().isEmpty())
        {
            continue;
        }

        ch::tf::ReagentNeed modifyNeed;
        // 设置数据库主键
        modifyNeed.__set_id(item->data().toInt());
        // 设置修改数据
        modifyNeed.__set_need(item->text().toInt());
        modifyNeedList.push_back(modifyNeed);
    }

    if (!ch::LogicControlProxy::ModifyReagentNeed(modifyNeedList))
    {
        return;
    }

    // 关闭对话框
    close();
}
