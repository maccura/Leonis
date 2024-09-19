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
/// @file     AssayAllocationDlg.cpp
/// @brief    应用--项目--项目分配
///
/// @author   7951/LuoXin
/// @date     2023年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QStandardItemModel>
#include "SortHeaderView.h"
#include "AssayAllocationDlg.h"
#include "ui_AssayAllocationDlg.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/public/SerializeUtil.hpp"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "shared/CommonInformationManager.h"

// 表格各信息所在列
enum TableViewColunm
{
    COLUNM_ASSAY_NAME,				// 项目名字
    COLUNM_ASSAY_CODE				// 项目编号
};

AssayAllocationDlg::AssayAllocationDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::AssayAllocationDlg)
    , m_pAssayModel(new QStandardItemModel)
{
    ui->setupUi(this);

    // 设置标题
    BaseDlg::SetTitleName(tr("项目分配"));

    InitCtrls();
}

AssayAllocationDlg::~AssayAllocationDlg()
{

}

void AssayAllocationDlg::LoadDataToCtrls()
{
    // 清空表格
    m_pAssayModel->removeRows(0, m_pAssayModel->rowCount());

    // 项目信息
    const auto& assayMap = CommonInformationManager::GetInstance()->GetAssayInfo();

    int iRow = 0;
    for (auto& iter : assayMap)
    {
        auto& assay = iter.second;
        if (assay->assayCode < ::tf::AssayCodeRange::CH_RANGE_MIN
            || assay->assayCode >ch::tf::g_ch_constants.ASSAY_CODE_SIND)
        {
            continue;
        }

        // 名称
        AddTextToTableView(m_pAssayModel, iRow, COLUNM_ASSAY_NAME, QString::fromStdString(assay->assayName));
        // 编号
        AddTextToTableView(m_pAssayModel, iRow, COLUNM_ASSAY_CODE, QString::number(assay->assayCode));

        // 勾选
        for (int i=2; i<m_pAssayModel->columnCount(); i++)
        {
            // 设备序列号
            std::string sn = m_pAssayModel->headerData(i, Qt::Horizontal, Qt::UserRole).toString().toStdString();
            const auto& dev = CommonInformationManager::GetInstance()->GetDeviceInfo(sn);
            if (dev == nullptr)
            {
                continue;
            }       

            std::set<int> codes;
            DecodeJson(codes, dev->unAllocatedAssayCodes);
            bool select = codes.find(assay->assayCode) == codes.end();

            // 设置勾选
            QStandardItem*si = new QStandardItem(); 
            si->setData(select, Qt::UserRole + 1);
            m_pAssayModel->setItem(iRow, i, si);
        }

        iRow++;
    }
}

void AssayAllocationDlg::showEvent(QShowEvent *event)
{
    // 基类先处理
    QWidget::showEvent(event);

    LoadDataToCtrls();

    // 还原默认顺序
    ui->tableView->sortByColumn(-1, Qt::DescendingOrder);
    m_sortModel->sort(-1, Qt::DescendingOrder);
}

void AssayAllocationDlg::InitCtrls()
{
    // Sort Header
    // 升序 降序 原序
    m_sortModel = new QUtilitySortFilterProxyModel(this);
    m_sortModel->setSourceModel(m_pAssayModel);
    ui->tableView->setModel(m_sortModel);
    m_sortModel->SetTableViewSort(ui->tableView, { COLUNM_ASSAY_NAME });

    // 表格表头
    QStringList titleList({ tr("名称"), tr("编号") });
    const auto& devVecs = CommonInformationManager::GetInstance()->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000 });

    for (auto& dev : devVecs)
    {
        if (dev->groupName.empty())
        {
            continue;
        }

        titleList.push_back(QString::fromStdString(dev->groupName + dev->name));
    }
    m_pAssayModel->setHorizontalHeaderLabels(titleList);
	
    // 列宽
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableView->setColumnWidth(0, 300);
	ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);

    // 表头设置设备的sn
    for (int i=0; i<devVecs.size(); i++)
    {
        if (devVecs[i]->groupName.empty())
        {
            continue;
        }

        m_pAssayModel->setHeaderData(i+2, Qt::Horizontal, QString::fromStdString(devVecs[i]->deviceSN), Qt::UserRole);
    }

    // 隐藏表格的code的列
    ui->tableView->hideColumn(COLUNM_ASSAY_CODE);

    // 勾选代理
    for (int i=2; i<m_pAssayModel->columnCount(); i++)
    {
        ui->tableView->setItemDelegateForColumn(i, new CheckBoxDelegate(this));
    }

    // 勾选框
    connect(ui->tableView, &QTableView::doubleClicked, this, [&](const QModelIndex& index)
    {
        if (index.column() > COLUNM_ASSAY_CODE)
        {
            m_pAssayModel->item(index.row(), index.column())->setData(
                !m_pAssayModel->item(index.row(), index.column())->data().toBool(), Qt::UserRole + 1);
        }
    });

    connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
}

void AssayAllocationDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const auto& CIM = CommonInformationManager::GetInstance();

    // 遍历所有列
    for (int j = 2; j < m_pAssayModel->columnCount(); j++)
    {
        // 遍历所有行
        std::set<int> unAllocatedAssayCodes;
        for (int i = 0; i < m_pAssayModel->rowCount(); i++)
        {
            if (!m_pAssayModel->item(i, j)->data(Qt::UserRole + 1).toBool())
            {
                int code = m_pAssayModel->data(m_pAssayModel->index(i, COLUNM_ASSAY_CODE)).toInt();
                unAllocatedAssayCodes.insert(code);
            }
        }

        std::string strXml;
        if (!Encode2Json(strXml, unAllocatedAssayCodes))
        {
            continue;
        }

        // 获取设备信息
        std::string sn = m_pAssayModel->headerData(j, Qt::Horizontal, Qt::UserRole).toString().toStdString();
        const auto& spAi = CIM->GetDeviceInfo(sn);

        // 未修改则跳过
        if (spAi == nullptr || strXml == spAi->unAllocatedAssayCodes)
        {
            continue;
        }

        // 修改项目分配
        tf::DeviceInfo di;
        di.__set_deviceSN(sn);
        di.__set_unAllocatedAssayCodes(std::move(strXml));
        if (!CIM->ModifyDeviceInfo(di))
        {
            TipDlg(tr("保存失败"), tr("保存项目分配的配置信息失败！")).exec();
        }
    }

    close();
}
