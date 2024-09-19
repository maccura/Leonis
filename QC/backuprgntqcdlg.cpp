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
/// @file     backuprgntqcdlg.cpp
/// @brief    备用瓶质控申请对话框
///
/// @author   4170/TangChuXian
/// @date     2021年6月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "backuprgntqcdlg.h"
#include "ui_backuprgntqcdlg.h"
#include "shared/uicommon.h"
#include "SortHeaderView.h"
#include "shared/uidef.h"
#include "qcapplymodel.h"
#include <QHBoxLayout>
#include <QRadioButton>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include "src/common/Mlog/mlog.h"

BackupRgntQcDlg::BackupRgntQcDlg(const QStringList strDevNameList, QWidget *parent)
    : BaseDlg(parent),
      m_strCurDevName(""),
      m_strDevNameList(strDevNameList),
      m_pAllDevRBtn(Q_NULLPTR),
      m_bInit(false),
      m_iSortCol(-1),
      m_enSortOrder(SortHeaderView::NoOrder),
      m_ciBackupRgntQcTblDefaultRowCnt(20),
      m_ciBackupRgntQcTblDefaultColCnt(14)
{
    // 初始化Ui对象
    ui = new Ui::BackupRgntQcDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

BackupRgntQcDlg::~BackupRgntQcDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     排序内容
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月21日，新建函数
///
void BackupRgntQcDlg::SortContent()
{
    //无效index或NoOrder就设置为默认未排序状态
    if (m_iSortCol < 0 || m_enSortOrder == SortHeaderView::NoOrder) {
        m_pStdModel->setSortRole(UI_ITEM_ROLE_SEQ_NO_SORT);
        ui->BackRgntQcTbl->sortByColumn(Bqh_Pos, Qt::AscendingOrder);
        m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    }
    else
    {
        int iSortRole = UI_ITEM_ROLE_SEQ_NO_SORT;
        if (m_iSortCol == Bqh_Pos)
        {
            iSortRole = UI_ITEM_ROLE_SEQ_NO_SORT;
        }
        else if (m_iSortCol == Bqh_Selected)
        {
            iSortRole = UI_ITEM_ROLE_ICON_SORT;
        }
        else
        {
            iSortRole = Qt::DisplayRole;
        }
        m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = m_enSortOrder == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->BackRgntQcTbl->sortByColumn(m_iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void BackupRgntQcDlg::InitBeforeShow()
{
    // 创建模型
    m_pStdModel = new QcApplyModel(this, QcApplyModel::QC_APPLY_TYPE_BACKUP);
    ui->BackRgntQcTbl->setModel(m_pStdModel);
    m_pStdModel->SetView(ui->BackRgntQcTbl);

    // Sort Header
    // 升序 降序 原序
    m_pSortHeader = new SortHeaderView(Qt::Horizontal);
    m_pSortHeader->setStretchLastSection(true);
    ui->BackRgntQcTbl->setHorizontalHeader(m_pSortHeader);

    // 点击表头触发排序
    connect(m_pSortHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order)
    {
        // 记录排序参数
        m_iSortCol = logicIndex;
        m_enSortOrder = order;

        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            m_pStdModel->setSortRole(UI_ITEM_ROLE_SEQ_NO_SORT);
            ui->BackRgntQcTbl->sortByColumn(Bqh_DbIndex, Qt::AscendingOrder);
            m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
        }
        else
        {
            int iSortRole = UI_ITEM_ROLE_SEQ_NO_SORT;
            if (logicIndex == Bqh_Pos)
            {
                iSortRole = UI_ITEM_ROLE_SEQ_NO_SORT;
            }
            else if (logicIndex == Bqh_Selected)
            {
                iSortRole = UI_ITEM_ROLE_ICON_SORT;
            }
            else
            {
                iSortRole = Qt::DisplayRole;
            }
            m_pStdModel->setSortRole(iSortRole);
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            ui->BackRgntQcTbl->sortByColumn(logicIndex, qOrderFlag);
        }
    });

    // 允许排序
    ui->BackRgntQcTbl->setSortingEnabled(true);

    // 根据设备列表创建单选框
    QHBoxLayout* pHlayout = new QHBoxLayout(ui->DevListFrame);
    pHlayout->setMargin(0);
    pHlayout->setSpacing(10);
    pHlayout->addStretch(1);

    // 依次添加具体设备
    for (const auto& strDevName : m_strDevNameList)
    {
        QRadioButton* pRBtn;
        if (strDevName.isEmpty())
        {
            pRBtn = new QRadioButton(tr("全部"));
            pRBtn->setChecked(true);
            m_pAllDevRBtn = pRBtn;
        }
        else
        {
            pRBtn = new QRadioButton(strDevName);
        }
        connect(pRBtn, SIGNAL(clicked()), this, SLOT(OnDevRBtnClicked()));
        pHlayout->addWidget(pRBtn);
    }

	// 只有一个模块时，不显示模块选项（bug12040）
	if (m_strDevNameList.size() <= 2)
	{
		ui->DevListFrame->hide();
	}

    // 初始化项目表
    m_pStdModel->setRowCount(m_ciBackupRgntQcTblDefaultRowCnt);
    m_pStdModel->setColumnCount(m_ciBackupRgntQcTblDefaultColCnt);
    ui->BackRgntQcTbl->hideColumn(Bqh_DbIndex);

    // 设置表头
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<BackupRgntQcHeader, QString>> qcApplyHeader = {
        { Bqh_Pos, tr("试剂位") },
        { Bqh_AssayName, tr("项目名称") },
        { Bqh_RgntLot, tr("试剂批号") },
        { Bqh_RgntNo, tr("试剂瓶号") },
        { Bqh_UseStatus, tr("使用状态") },
        { Bqh_QcNo, tr("质控品编号") },
        { Bqh_QcName, tr("质控品名称") },
        { Bqh_QcBriefName, tr("质控品简称") },
        { Bqh_QcType, tr("质控品类型") },
        { Bqh_QcLevel, tr("质控品水平") },
        { Bqh_QcLot, tr("质控品批号") },
        { Bqh_ExpDate, tr("失效日期") },
        { Bqh_Selected, tr("选择") }
    };
    QStringList strHeaderList;
    for (auto it = qcApplyHeader.begin(); it != qcApplyHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    m_pStdModel->setHorizontalHeaderLabels(strHeaderList);
    m_pStdModel->horizontalHeaderItem(Bqh_Selected)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void BackupRgntQcDlg::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void BackupRgntQcDlg::InitStrResource()
{
    ui->QcNameLab->setText(tr("质控品名称"));
    ui->CloseBtn->setText(tr("关闭"));
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void BackupRgntQcDlg::InitConnect()
{
    // 当前质控名改变
    connect(ui->QcNameCombo, SIGNAL(ResetText()), this, SLOT(OnQryCondChanged()));

    // 选择列被点击
    connect(ui->BackRgntQcTbl, SIGNAL(clicked(const QModelIndex&)), this, SIGNAL(SigItemClicked(const QModelIndex&)));

    // 查询按钮被点击
    connect(ui->QueryBtn, SIGNAL(clicked()), this, SLOT(OnQueryBtnClicked()));

	// 初始状态筛选按钮不可用，有文字的时候才可以用
	ui->QueryBtn->setEnabled(false);
	connect(ui->QcNameCombo, &QComboBox::editTextChanged, this, [this] {
		if (ui->QcNameCombo->lineEdit()->text().isEmpty())
			ui->QueryBtn->setEnabled(false);
		else
			ui->QueryBtn->setEnabled(true);
	});

    // 取消按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 选择按钮被点击
    connect(ui->SelBtn, SIGNAL(clicked()), this, SIGNAL(SigSelBtnClicked()));

    // 重置查询
    //connect(ui->CondWidget, SIGNAL(SigResetQry()), this, SLOT(OnResetBtnClicked()));

    // 选中项改变
    QItemSelectionModel* pSelModel = ui->BackRgntQcTbl->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SIGNAL(SigSelectionChanged()));
    }
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void BackupRgntQcDlg::InitChildCtrl()
{
    // 设置标题
    SetTitleName(tr("备用瓶质控"));

    // 默认排序初始化
//     m_pStdModel->setSortRole(UI_ITEM_ROLE_SEQ_NO_SORT);
//     ui->BackRgntQcTbl->sortByColumn(Bqh_Pos, Qt::AscendingOrder);
//     ui->BackRgntQcTbl->sortByColumn(Bqh_DbIndex, Qt::AscendingOrder);
//     m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder);

    // 更新完成器(qt源码中原来的完成器会自动销毁)
    QStringList strDocNameList;
    for (int iIdx = 0; iIdx < ui->QcNameCombo->count(); iIdx++)
    {
        if (ui->QcNameCombo->itemText(iIdx).isEmpty())
        {
            continue;
        }

        strDocNameList.push_back(ui->QcNameCombo->itemText(iIdx));
    }

    // 表头
    //ui->BackRgntQcTbl->horizontalHeaderItem(Dqh_Selected)->setTextColor(UI_TEXT_COLOR_HEADER_MARK);
    ui->BackRgntQcTbl->verticalHeader()->setVisible(false);
    //ui->BackRgntQcTbl->setItemDelegateForColumn(Dqh_Selected, new McPixmapItemDelegate(ui->BackRgntQcTbl));

    // 设置表格选中模式为行选中，不可多选
    ui->BackRgntQcTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->BackRgntQcTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->BackRgntQcTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 表格列宽自适应
    ResizeTblColToContent(ui->BackRgntQcTbl);

    // 设置表格列宽比
//     QVector<double> vScale;
//     vScale << 1.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
//     SetTblColWidthScale(ui->BackRgntQcTbl, vScale);

    // 设置绝对列宽
//     QVector<double> vColWidth;
//     vColWidth << 162.0 << 160.0 << 161.0 << 160.0 << 161.0 << 131.0;
//     SetTblColAbsWidthScale(ui->BackRgntQcTbl, vColWidth);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void BackupRgntQcDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
    else if (m_pSortHeader->sortIndicatorSection() >= 0)
    {
        // 恢复默认排序
        m_pSortHeader->ResetAllIndex();
        m_pStdModel->setSortRole(UI_ITEM_ROLE_SEQ_NO_SORT);
        ui->BackRgntQcTbl->sortByColumn(Bqh_DbIndex, Qt::AscendingOrder);
        m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    }
}

///
/// @brief
///     设备单选框被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void BackupRgntQcDlg::OnDevRBtnClicked()
{
    // 获取信号发送者
    QRadioButton* pRBtn = qobject_cast<QRadioButton*>(sender());
    if (pRBtn == Q_NULLPTR)
    {
        return;
    }

    // 如果是【全部】单选框被点击，则将设备名置空
    if (pRBtn == m_pAllDevRBtn)
    {
        m_strCurDevName.clear();
    }
    else
    {
        m_strCurDevName = pRBtn->text();
    }

    // 通知当前选中设备改变
    emit SigCurDevNameChanged(m_strCurDevName);
}

///
/// @brief
///     查询按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月28日，新建函数
///
void BackupRgntQcDlg::OnQueryBtnClicked()
{
    // 发送质控名改变信号
    emit SigCurQcNameChanged();
}

///
/// @brief
///     重置按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月28日，新建函数
///
void BackupRgntQcDlg::OnResetBtnClicked()
{
    // 将质控名重置为空
    int iIdx = ui->QcNameCombo->findText("");
    if (iIdx >= 0)
    {
        ui->QcNameCombo->setCurrentIndex(iIdx);
    }
}

///
/// @brief
///     查询条件改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月10日，新建函数
///
void BackupRgntQcDlg::OnQryCondChanged()
{
    if (ui->QcNameCombo->currentText().isEmpty())
    {
        emit SigCurQcNameChanged();
    }
}
