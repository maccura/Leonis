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
/// @file     BackUnitDlg.cpp
/// @brief    备选单位弹窗
///
/// @author   7951/LuoXin
/// @date     2022年8月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年8月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "BackUnitDlg.h"
#include "AddBackupUnitDlg.h"
#include "ui_BackUnitDlg.h"

#include <QStandardItemModel>
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"

#include "src/common/Mlog/mlog.h"
#include "src/thrift/gen-cpp/defs_types.h"

// 表格中各列的信息
enum TABLE_VIEW_COLUMN
{
    BACK_UNIT_COLUMN,				// 备选单位所在列
    UNIT_FACTOR_COLUMN				// 单位倍率所在列
};

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return ;\
}

BackUnitDlg::BackUnitDlg(QWidget *parent)
    : BaseDlg(parent)
    , ui(new Ui::BackUnitDlg)
    , m_tabViewMode(new QStandardItemModel)
    , m_addBackupUnitDlg(new AddBackupUnitDlg(this))
{
    ui->setupUi(this);

    // 设置标题
    BaseDlg::SetTitleName(tr("备选单位"));

    InitCtrls();
}

BackUnitDlg::~BackUnitDlg()
{

}

void BackUnitDlg::InitCtrls()
{
    ui->tableView->setModel(m_tabViewMode);

    // 设置样本源优先级列表表头
    QStringList sampleHeaderListString;
    sampleHeaderListString << tr("备选单位") << tr("单位倍率");
    m_tabViewMode->setHorizontalHeaderLabels(sampleHeaderListString);

    // 修改
    connect(ui->modify_btn, &QPushButton::clicked, this, &BackUnitDlg::OnModifyBtn);
    // 删除
    connect(ui->del_btn, &QPushButton::clicked, this, &BackUnitDlg::OnDelBtnClicked);

    // 新增
    connect(ui->add_btn, &QPushButton::clicked, this, [&] {
        m_addBackupUnitDlg->ShowAddWidget(m_currentAssayName);
    });

    // 改变行
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&] {
        bool hasRow = ui->tableView->selectionModel()->hasSelection();
        ui->modify_btn->setEnabled(hasRow);
        ui->del_btn->setEnabled(hasRow);
    });

    // 新增行完毕
    connect(m_addBackupUnitDlg, &AddBackupUnitDlg::AddComplete, this, [&](QString unit, QString factor) {
        int iRow = m_tabViewMode->rowCount();
        AddTextToTableView(m_tabViewMode, iRow, BACK_UNIT_COLUMN, unit);
        AddTextToTableView(m_tabViewMode, iRow, UNIT_FACTOR_COLUMN, factor);
        ui->tableView->selectRow(iRow);
    });

    // 编辑行完毕
    connect(m_addBackupUnitDlg, &AddBackupUnitDlg::ModifyComplete, this, [&](QString unit, QString factor) {
        int iRow = ui->tableView->currentIndex().row();
        m_tabViewMode->item(iRow, BACK_UNIT_COLUMN)->setText(unit);
        m_tabViewMode->item(iRow, UNIT_FACTOR_COLUMN)->setText(factor);
    });

    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void BackUnitDlg::LoadDataToCtrls(int assayCode)
{
    ui->modify_btn->setEnabled(false);
    ui->del_btn->setEnabled(false);

    // 查询通用项目信息
    auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo() Failed! assayCode = %d", assayCode);
        return;
    }
    m_currentAssayName = QString::fromStdString(spAssayInfo->assayName);

    // 记录旧的选中行
    QModelIndex oldSelectedRow = ui->tableView->currentIndex();

    // 清空表格
    m_tabViewMode->removeRows(0, m_tabViewMode->rowCount());

    // 显示单位
    QString strMainUnit;
    int iRow = -1;
    for (const tf::AssayUnit& item : spAssayInfo->units)
    {
        if (item.isMain)
        {
            // 主单位
            strMainUnit = QString::fromStdString(item.name);
        }
        else if (item.isUserDefine)
        {
            iRow++;
            AddTextToTableView(m_tabViewMode, iRow, BACK_UNIT_COLUMN, QString::fromStdString(item.name));
            AddTextToTableView(m_tabViewMode, iRow, UNIT_FACTOR_COLUMN, QString::number(item.factor, 'f', 4));
        }
    }

    // 清空控件
    ui->assay_name_label->clear();
    ui->main_unit_label->clear();

    // 项目名称
    ui->assay_name_label->setText(tr("项目:  ") + m_currentAssayName);
    ui->main_unit_label->setText(tr("主单位:  ") + strMainUnit);

    // 恢复选中;默认选中第一行
    int iSelectRow = oldSelectedRow.isValid() ? oldSelectedRow.row() : 0;
    // 如果删除的是末尾行，则选中被删除行的上一行
    int tabRowCnt = m_tabViewMode->rowCount();
    if (iSelectRow >= tabRowCnt && tabRowCnt > 0)
    {
        iSelectRow = tabRowCnt - 1;
    }
    ui->tableView->selectRow(iSelectRow);
}

void BackUnitDlg::OnDelBtnClicked()
{
    // 检测放置在前面，用以减少弹框次数
    const auto& CIM = CommonInformationManager::GetInstance();
    QModelIndexList selectedItems = ui->tableView->selectionModel()->selectedRows();
    QStringList avlidUnit;
    for (auto& rowIndex : selectedItems)
    {
        // 空白行
        if (m_tabViewMode->data(rowIndex).isNull())
        {
            continue;
        }
        QString unitName = m_tabViewMode->data(m_tabViewMode->index(rowIndex.row(), BACK_UNIT_COLUMN)).toString();
        QString errInfo;
        if (!CIM->CanDeleteAssayUnit(m_currentAssayName, unitName, errInfo))
        {
            TipDlg(errInfo).exec();
            return;
        }
        avlidUnit.append(unitName);
    }

    // 弹出提示框
    if (TipDlg(tr("删除"), tr("是否删除选中单位"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    for (const QString& strUnit : avlidUnit)
    {
        QString errInfo;
        if (!CIM->DeleteAssayUnit(m_currentAssayName, strUnit, errInfo))
        {
            TipDlg(tr("删除失败"), errInfo).exec();
            break;
        }

        // 填充操作日志，暂时使用常量字符串
        QString temStr(tr("删除项目") + m_currentAssayName + tr("的备选单位") + strUnit);
        if (!AddOptLog(::tf::OperationType::DEL, temStr))
        {
            ULOG(LOG_ERROR, "Add delete backup unit operate log failed! ");
        }
    }

    const auto& spAssayInfo = CIM->GetAssayInfo(m_currentAssayName.toStdString());
    if (spAssayInfo != nullptr)
    {
        // 通知项目更新
        CIM->UpdateAssayCfgInfo({ spAssayInfo->assayCode });
        LoadDataToCtrls(spAssayInfo->assayCode);
    }
}

void BackUnitDlg::OnModifyBtn()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 备选单位
    QString unit = m_tabViewMode->data(m_tabViewMode->index(ui->tableView->currentIndex().row(), BACK_UNIT_COLUMN)).toString();
    // 倍率
    QString factor = m_tabViewMode->data(m_tabViewMode->index(ui->tableView->currentIndex().row(), UNIT_FACTOR_COLUMN)).toString();

    // 不能修改当前单位BUG:0024736
    if (IsCurrentUnit(unit))
    {
        TipDlg(tr("单位%1为当前单位，不能修改！").arg(unit)).exec();
        return;
    }

    m_addBackupUnitDlg->ShowModifyWidget(m_currentAssayName, unit, factor);
}

///
/// @bref
///		判断是否是当前单位
///
/// @param[in] strUnitName 被判定的单位名
///
/// @par History:
/// @li 8276/huchunli, 2024年1月8日，新建函数
///
bool BackUnitDlg::IsCurrentUnit(const QString& strUnitName) const
{
    std::shared_ptr<::tf::GeneralAssayInfo> pAssay = CommonInformationManager::GetInstance()->GetAssayInfo(m_currentAssayName.toStdString());
    if (pAssay == nullptr)
    {
        ULOG(LOG_WARN, "Faild to get assayinfo by name:%s.", strUnitName.toStdString().c_str());
        return false;
    }
    for (const auto& uItem : pAssay->units)
    {
        if (uItem.isCurrent && strUnitName.toStdString() == uItem.name)
        {
            return true;
        }
    }

    return false;
}
