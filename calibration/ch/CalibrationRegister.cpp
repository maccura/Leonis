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
/// @file     CalibrationRegister.h
/// @brief    校准液的注册页面
///
/// @author   5774/WuHongTao
/// @date     2021年11月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "CalibrationRegister.h"
#include "shared/CommonInformationManager.h"
#define defaultRow 20

///
/// @brief
///     构造函数
///
/// @param[in]  parent  
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
CalibrationRegister::CalibrationRegister(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(-1)
    , m_assayDialog(nullptr)
    , m_currentSize(-1)
    , m_currentSection(-1)
    , m_posEditDialog(nullptr)
{
    ui.setupUi(this);
    Init();
}

///
/// @brief
///     析构函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
CalibrationRegister::~CalibrationRegister()
{
}

///
/// @brief
///     初始化函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
void CalibrationRegister::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 表头--校准组的头部
    QStringList groupHeaderList;
    groupHeaderList << tr("No") << tr("Calibrate") << tr("Lot")<< tr("ExpireDate") ;
    ui.caliGroup->setRowCount(defaultRow);
    ui.caliGroup->setColumnCount(groupHeaderList.size());
    ui.caliGroup->setHorizontalHeaderLabels(groupHeaderList);
    ui.caliGroup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.caliGroup->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 校准品的相信信息
    QStringList detailHeaderList;
    detailHeaderList << tr("name") << tr("caliBrate_1") << tr("caliBrate_2") << tr("caliBrate_3")
        << tr("caliBrate_4") << tr("caliBrate_5") << tr("caliBrate_6") << tr("CupType");
    ui.caliDetail->setRowCount(defaultRow);
    ui.caliDetail->setColumnCount(detailHeaderList.size());
    ui.caliDetail->setHorizontalHeaderLabels(detailHeaderList);
    ui.caliDetail->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.caliDetail->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 项目控件选中消息回调
    connect(ui.caliGroup, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(OnItemClicked(QTableWidgetItem*)));
    connect(ui.caliGroup, SIGNAL(cellClicked(int, int)), this, SLOT(OnCellClicked(int, int)));
    // 表头选中信号回调
    connect(ui.caliDetail->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnHeaderClicked(int)));
    // 选中行数回调
    connect(this, SIGNAL(currentRowSelect(int)), this, SLOT(OnShowGroupDetail(int)));
    // 选中编辑按钮
    connect(ui.editBtn, SIGNAL(clicked()), this, SLOT(OnShowCaliGroupDetail()));
    // 选中删除按钮
    connect(ui.delBtn, SIGNAL(clicked()), this, SLOT(OnDeleteCaliDroup()));
    // 查询校准组信息
    QueryDocGroups();
}

///
/// @brief
///     获取项目名称，根据项目码
///
/// @param[in]  assayCode  项目码
///
/// @return 项目名称
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月5日，新建函数
///
QString CalibrationRegister::GetAssayNameFromCode(const int assayCode)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (Q_NULLPTR == spAssayInfo)
    {
        ULOG(LOG_WARN, "Invalid assay code,");
        return QString();
    }

    // 返回项目名称
    return QString::fromStdString(spAssayInfo->assayName);
}

///
/// @brief
///     查询校准组信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
void CalibrationRegister::QueryDocGroups()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ::ch::tf::CaliDocGroupQueryResp _return;
    ::ch::tf::CaliDocGroupQueryCond cdgq;

    // 查询校准组信息
    if (!ch::LogicControlProxy::QueryCaliDocGroup(_return, cdgq))
    {
        ULOG(LOG_ERROR, "The caliBrate query failed");
        return;
    }

    // 若没有数据也返回
    if (_return.lstCaliDocGroup.size() <= 0)
    {
        return;
    }

    // 显示校准组的信息
    ShowCaliBrateGroupData(_return.lstCaliDocGroup);
}

///
/// @brief
///     显示校准组信息列表
///
/// @param[in]  groups   校准组信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
void CalibrationRegister::ShowCaliBrateGroupData(std::vector<::ch::tf::CaliDocGroup>& groups)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 如果校准组信息，大于行数则增加行数
    m_currentSize = groups.size();
    if (ui.caliGroup->rowCount() < m_currentSize)
    {
        ui.caliGroup->setRowCount(m_currentSize);
    }

    // 依次显示校准组信息
    int row = 0;
    // 清空校准组数据
    m_groups.clear();
    for (auto caliGroup : groups)
    {
        m_groups.push_back(caliGroup);
        int column = 0;
        // 编号No信息
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(row));
        ui.caliGroup->setItem(row, column++, item);

        // Calibrate名称信息
        item = new QTableWidgetItem(QString::fromStdString(caliGroup.name));
        ui.caliGroup->setItem(row, column++, item);

        row++;
    }
}

///
/// @brief
///     显示校准品详细信息
///
/// @param[in]  doc    校准品信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
void CalibrationRegister::ShowCaliDocData(int level, ::ch::tf::CaliDoc& doc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 确认水平范围是否超限
    if (level > ui.caliDetail->columnCount())
    {
        ULOG(LOG_ERROR, "The level %d is out of the range", level);
        return;
    }

    // 如果成分信息数目，大于行数则增加行数
    if (ui.caliDetail->rowCount() < doc.compositions.size())
    {
        ui.caliDetail->setRowCount(doc.compositions.size());
    }

    int row = 0;
    for (auto com : doc.compositions)
    {
        QTableWidgetItem* item = nullptr;
        if (level == 1)
        {
            // 项目名称
            item = new QTableWidgetItem(GetAssayNameFromCode(com.assayCode));
            ui.caliDetail->setItem(row, 0, item);
        }

        // 浓度信息
        item = new QTableWidgetItem(QString::number(com.conc));
        ui.caliDetail->setItem(row, level, item);

        row++;
    }

    // 获取对应的表头控件
    QTableWidgetItem* verItem = ui.caliDetail->horizontalHeaderItem(level);
    if (verItem == nullptr)
    {
        return;
    }

    // 如果位置信息存在
    if (doc.__isset.pos && doc.__isset.rack)
    {
        //设置对应的位置
        verItem->setText(QString("%1:%2").arg(QString::fromStdString(doc.rack)).arg(doc.pos));
    }
    // 默认显得表头
    else
    {
        //设置对应的位置
        verItem->setText(QString("caliBrate%1").arg(level));
    }
}

///
/// @brief
///     显示校准组详细信息
///
/// @param[in]  index  选中的校准组行数
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
void CalibrationRegister::OnShowGroupDetail(int index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // index超出范围
    if (m_groups.size() <= index || index < 0)
    {
        ULOG(LOG_ERROR, "the index is outof range");
        return;
    }

    ui.caliDetail->clear();
    QStringList detailHeaderList;
    detailHeaderList << tr("name") << tr("caliBrate_1") << tr("caliBrate_2") << tr("caliBrate_3")
        << tr("caliBrate_4") << tr("caliBrate_5") << tr("caliBrate_6") << tr("CupType");
    ui.caliDetail->setHorizontalHeaderLabels(detailHeaderList);

    // 清空当前的校准组信息列表
    m_calicDocs.clear();
    // 首先清空详细信息表
    ui.caliDetail->clearContents();
    // 查找校准品数据
    ::ch::tf::CaliDocQueryResp _return;
    ::ch::tf::CaliDocQueryCond cdqc;
    for (auto CaliDocKeyInfo : m_groups[index].caliDocKeyInfos)
    {
        cdqc.__set_lot(CaliDocKeyInfo.lot);
        cdqc.__set_sn(CaliDocKeyInfo.sn);
        cdqc.__set_level(CaliDocKeyInfo.level);

        // 查询校准品的信息
        if (!ch::LogicControlProxy::QueryCaliDoc(_return, cdqc) || _return.lstCaliDocs.size() < 1)
        {
            ULOG(LOG_ERROR, "can't find the calibrate data");
            continue;
        }

        // 存储当前选中的校准文档列表组--以备显示详细信息的时候使用
        m_calicDocs.push_back(_return.lstCaliDocs[0]);
        ShowCaliDocData(CaliDocKeyInfo.level, _return.lstCaliDocs[0]);
    }
}

///
/// @brief
///     单元格选中信号处理函数
///
/// @param[in]  item  选中的单元格
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
void CalibrationRegister::OnItemClicked(QTableWidgetItem* item)
{
    emit currentRowSelect(item->row());
}

///
/// @brief
///     编辑按钮杯选中
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
void CalibrationRegister::OnShowCaliGroupDetail()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_assayDialog == nullptr)
    {
        m_assayDialog = new AssayDialog(this);
        m_assayDialog->setModal(true);
        //关闭窗口的时候刷新主界面的数据
        connect(m_assayDialog, SIGNAL(closeWindow()), this, SLOT(OnUpdateAllGroup()));
    }

    // 新增校准组
    if (m_currentIndex == -1)
    {
        m_assayDialog->SetTitle("new");
        m_assayDialog->Reset();
        m_assayDialog->show();
    }
    else
    {
        m_assayDialog->SetTitle("detail");
        m_assayDialog->Reset();
        m_assayDialog->show();
        m_assayDialog->DetailShow(m_calicDocs, m_groups[m_currentIndex]);
    }
}

///
/// @brief
///     删除选中的校准组
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月11日，新建函数
///
void CalibrationRegister::OnDeleteCaliDroup()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 检测范围是否在合理范围之内
    if (m_currentIndex < 0 || m_currentIndex >= m_groups.size())
    {
        return;
    }

    // 删除对应的校准组
    ::ch::tf::CaliDocGroupQueryCond cdgq;
    cdgq.__set_id(m_groups[m_currentIndex].id);
    if (!ch::LogicControlProxy::DeleteCaliDocGroup(cdgq))
    {
        ULOG(LOG_ERROR, "Delete CaliGroup  %s() failed", m_groups[m_currentIndex].name.c_str());
    }

    // 刷新数据
    OnUpdateAllGroup();
}

///
/// @brief
///     更新校准组数据
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
void CalibrationRegister::OnUpdateAllGroup()
{
    // 清空校准组数据，然后重新刷新数据
    ui.caliGroup->clearContents();
    ui.caliDetail->clearContents();
    QueryDocGroups();
}

///
/// @brief
///     单元格被选中的槽函数
///
/// @param[in]  row  选中单元格的行
/// @param[in]  column  选中单元格的列
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月9日，新建函数
///
void CalibrationRegister::OnCellClicked(int row, int column)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 超出显示范围
    if (row >= m_currentSize)
    {
        m_currentIndex = -1;
        // 清空详细信息表
        ui.caliDetail->clearContents();
    }
    // 显示范围之内
    else
    {
        m_currentIndex = row;
        emit currentRowSelect(row);
    }
}

///
/// @brief
///     位置编辑完成
///
/// @param[in]  rack  架号
/// @param[in]  pos  位置号
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月11日，新建函数
///
void CalibrationRegister::OnEditFinishedPostion(int rack, int pos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 判断值的合法性
    if (m_currentSection <= 0 || m_currentSection > m_calicDocs.size())
    {
        ULOG(LOG_ERROR, "Section %d is outof the range", m_currentSection);
        return;
    }

    // 获取表头的对象
    QTableWidgetItem* hearItem = ui.caliDetail->horizontalHeaderItem(m_currentSection);
    hearItem->setText(QString("%1:%2").arg(rack).arg(pos));

    // 设置位置,修改
    ::ch::tf::CaliDoc doc;
    doc.__set_pos(pos);
    doc.__set_rack(to_string(rack));
    doc.__set_id(m_calicDocs[m_currentSection-1].id);

    // 修改校准文档
    if (!ch::LogicControlProxy::ModifyCaliDoc(doc))
    {
        ULOG(LOG_ERROR, "caliDoc modify failed db is %d()", doc.id);
    }

}

///
/// @brief
///     表头选中的回调函数
///
/// @param[in]  selction  表头选中标签
/// @par History:
/// @li 5774/WuHongTao，2021年11月11日，新建函数
///
void CalibrationRegister::OnHeaderClicked(int selction)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 判断行值是否在范围之内
    if (m_currentIndex < 0 || m_currentIndex >= m_groups.size() || m_calicDocs.size() == 0)
    {
        ULOG(LOG_ERROR, "The index %d is out of range", m_currentIndex);
        return;
    }

    // 判断列项是否在范围之内
    if (selction <= 0 || selction > m_groups[m_currentIndex].caliDocKeyInfos.size())
    {
        ULOG(LOG_ERROR, "The section %d is out of range", selction);
        return;
    }

    //  列赋值
    m_currentSection = selction;

    //启动编辑框
    if (m_posEditDialog == nullptr)
    {
        m_posEditDialog = new DialogEdit(this);
        // 位置编辑完成
        connect(m_posEditDialog, SIGNAL(closeWindowEdit(int, int)), this, SLOT(OnEditFinishedPostion(int, int)));
    }

    m_posEditDialog->show();
}
