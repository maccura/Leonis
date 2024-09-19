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
/// @file     defaultqcdlg.cpp
/// @brief    默认质控品对话框
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

#include "defaultqcdlg.h"
#include "ui_defaultqcdlg.h"
#include "shared/uicommon.h"
#include "SortHeaderView.h"
#include "shared/uidef.h"
#include "qcapplymodel.h"
#include <QHBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include "src/common/Mlog/mlog.h"

DefaultQcDlg::DefaultQcDlg(const QStringList strDevNameList, QWidget *parent)
    : BaseDlg(parent),
      m_strCurDevName(""),
      m_strDevNameList(strDevNameList),
      m_pAllDevRBtn(Q_NULLPTR),
      m_bInit(false),
      m_ciDefaultQcTblDefaultRowCnt(20),
      m_ciDefaultQcTblDefaultColCnt(11)
{
    // 初始化Ui对象
    ui = new Ui::DefaultQcDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

DefaultQcDlg::~DefaultQcDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void DefaultQcDlg::InitBeforeShow()
{
    // 创建模型
    m_pStdModel = new QcApplyModel(this, QcApplyModel::QC_APPLY_TYPE_DEFAULT);
    ui->DefaultQcTbl->setModel(m_pStdModel);
    m_pStdModel->SetView(ui->DefaultQcTbl);

    // Sort Header
    // 升序 降序 原序
    m_pSortHeader = new SortHeaderView(Qt::Horizontal);
    m_pSortHeader->setStretchLastSection(true);
    ui->DefaultQcTbl->setHorizontalHeader(m_pSortHeader);

    // 默认排序初始化
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder);

    // 点击表头触发排序
    connect(m_pSortHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order)
    {
        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            m_pStdModel->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
            ui->DefaultQcTbl->sortByColumn(Dqh_DbIndex, Qt::AscendingOrder);
            m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
        }
        else
        {
            int iSortRole = (logicIndex == Dqh_Selected) ? UI_ITEM_ROLE_ICON_SORT : Qt::DisplayRole;
            m_pStdModel->setSortRole(iSortRole);
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            ui->DefaultQcTbl->sortByColumn(logicIndex, qOrderFlag);
        }
    });

    // 允许排序
    ui->DefaultQcTbl->setSortingEnabled(true);

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

    // 初始化项目表
    m_pStdModel->setRowCount(m_ciDefaultQcTblDefaultRowCnt);
    m_pStdModel->setColumnCount(m_ciDefaultQcTblDefaultColCnt);
    ui->DefaultQcTbl->hideColumn(Dqh_DbIndex);

    // 设置表头
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<DefaultQcHeader, QString>> qcApplyHeader = {
        { Dqh_Module, tr("模块") },
        { Dqh_AssayName, tr("项目名称") },
        { Dqh_QcNo, tr("质控品编号") },
        { Dqh_QcName, tr("质控品名称") },
        { Dqh_QcBriefName, tr("质控品简称") },
        { Dqh_QcType, tr("质控品类型") },
        { Dqh_QcLevel, tr("质控品水平") },
        { Dqh_QcLot, tr("质控品批号") },
        { Dqh_ExpDate, tr("失效日期") },
        { Dqh_Selected, tr("选择") }
    };
    QStringList strHeaderList;
    for (auto it = qcApplyHeader.begin(); it != qcApplyHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    m_pStdModel->setHorizontalHeaderLabels(strHeaderList);
    m_pStdModel->horizontalHeaderItem(Dqh_Selected)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void DefaultQcDlg::InitAfterShow()
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
void DefaultQcDlg::InitStrResource()
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
void DefaultQcDlg::InitConnect()
{
    // 当前质控名改变
    connect(ui->QcNameCombo, SIGNAL(ResetText()), this, SLOT(OnQryCondChanged()));

    // 选择列被点击
    connect(ui->DefaultQcTbl, SIGNAL(clicked(const QModelIndex&)), this, SIGNAL(SigItemClicked(const QModelIndex&)));

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
    QItemSelectionModel* pSelModel = ui->DefaultQcTbl->selectionModel();
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
void DefaultQcDlg::InitChildCtrl()
{
    // 设置标题
    SetTitleName(tr("默认质控"));

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
    //ui->DefaultQcTbl->horizontalHeaderItem(Dqh_Selected)->setTextColor(UI_TEXT_COLOR_HEADER_MARK);
    ui->DefaultQcTbl->verticalHeader()->setVisible(false);
    //ui->DefaultQcTbl->setItemDelegateForColumn(Dqh_Selected, new McPixmapItemDelegate(ui->DefaultQcTbl));

    // 设置表格选中模式为行选中，不可多选
    ui->DefaultQcTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->DefaultQcTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->DefaultQcTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 表格列宽自适应
    ResizeTblColToContent(ui->DefaultQcTbl);

    // 设置表格列宽比
//     QVector<double> vScale;
//     vScale << 1.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
//     SetTblColWidthScale(ui->DefaultQcTbl, vScale);

    // 设置绝对列宽
//     QVector<double> vColWidth;
//     vColWidth << 162.0 << 160.0 << 161.0 << 160.0 << 161.0 << 131.0;
//     SetTblColAbsWidthScale(ui->DefaultQcTbl, vColWidth);
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
void DefaultQcDlg::showEvent(QShowEvent *event)
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
        m_pStdModel->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
        ui->DefaultQcTbl->sortByColumn(Dqh_DbIndex, Qt::AscendingOrder);
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
void DefaultQcDlg::OnDevRBtnClicked()
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
void DefaultQcDlg::OnQueryBtnClicked()
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
void DefaultQcDlg::OnResetBtnClicked()
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
void DefaultQcDlg::OnQryCondChanged()
{
    if (ui->QcNameCombo->currentText().isEmpty())
    {
        emit SigCurQcNameChanged();
    }
}
