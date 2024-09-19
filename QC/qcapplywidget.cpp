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
/// @file     qcapplywidget.cpp
/// @brief    质控申请界面
///
/// @author   4170/TangChuXian
/// @date     2022年12月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "qcapplywidget.h"
#include <QStandardItem>
#include <QSortFilterProxyModel>
#include "qcapplymodel.h"
#include "defaultqcdlg.h"
#include "backuprgntqcdlg.h"
#include "ui_qcapplywidget.h"
#include "ui_defaultqcdlg.h"
#include "ui_backuprgntqcdlg.h"

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"

#include "shared/CReadOnlyDelegate.h"
#include "shared/mcpixmapitemdelegate.h"
#include "uidcsadapter/uidcsadapter.h"
#include "src/common/Mlog/mlog.h"

Q_DECLARE_METATYPE(QC_APPLY_INFO)

// 模型更新数据效率优化器
// class ViewModelDataUpdateGuard
// {
// public:
//     ViewModelDataUpdateGuard(QTableView* pTbl) : m_pTbl(pTbl), m_pModel(pTbl->model()), m_pSortModel(Q_NULLPTR)
//     {
//         // 参数检查
//         if (m_pTbl == Q_NULLPTR || m_pModel == Q_NULLPTR)
//         {
//             return;
//         }
// 
//         // 记录隐藏列
//         int iColCnt = m_pModel->columnCount();
//         for (int i = 0; i < iColCnt; i++)
//         {
//             if (m_pTbl->isColumnHidden(i))
//             {
//                 setHideColum.insert(i);
//             }
//         }
// 
//         // 分离view和model
//         m_pTbl->setModel(Q_NULLPTR);
// 
//         // 如果不是是排序模型
//         if (!m_pModel->inherits("QSortFilterProxyModel"))
//         {
//             return;
//         }
// 
//         // 排序模型特殊处理
//         m_pSortModel = qobject_cast<QSortFilterProxyModel*>(m_pModel);
//         if (m_pSortModel == Q_NULLPTR)
//         {
//             return;
//         }
// 
//         // 记录真实模型
//         m_pModel = m_pSortModel->sourceModel();
//         m_pSortModel->setSourceModel(Q_NULLPTR);
//     }
// 
//     // 结束监听
//     void EndGuard()
//     {
//         // 参数检查
//         if (m_pSortModel == Q_NULLPTR)
//         {
//             if (m_pTbl == Q_NULLPTR || m_pModel == Q_NULLPTR || (m_pTbl->model() == m_pModel))
//             {
//                 return;
//             }
// 
//             // 重新设置模型
//             m_pTbl->setModel(m_pModel);
//         }
//         else
//         {
//             if (m_pTbl == Q_NULLPTR || m_pModel == Q_NULLPTR || (m_pTbl->model() == m_pSortModel))
//             {
//                 return;
//             }
// 
//             // 重新设置模型
//             m_pSortModel->setSourceModel(m_pModel);
//             m_pTbl->setModel(m_pModel);
//         }
// 
//         // 隐藏对应列
//         for (int iHideCol : setHideColum)
//         {
//             m_pTbl->hideColumn(iHideCol);
//         }
//     }
// 
//     // 析构
//     ~ViewModelDataUpdateGuard()
//     {
//         EndGuard();
//     }
// 
// private:
//     QTableView*             m_pTbl;         // 表格
//     QAbstractItemModel*     m_pModel;       // 模型
//     QSortFilterProxyModel*  m_pSortModel;   // 排序模型
//     std::set<int>           setHideColum;   // 隐藏列集合
// }
// ;

QcApplyWidget::QcApplyWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false),
      m_bNeedUpdateQcApplyInfo(true),
      m_strQryQcName(""),
      m_ciQcApplyTblDefaultRowCnt(44),
      m_ciQcApplyTblDefaultColCnt(11),
      m_cstrSelBtnStatusName("status"),
      m_cstrSelBtnStatusValSel(true),
      m_cstrSelBtnStatusValCancelSel(false),
      m_pDefaultQcDlg(Q_NULLPTR),
      m_pBackupRgntQcDlg(Q_NULLPTR)
{
    // 初始化UI对象
    ui = new Ui::QcApplyWidget;
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

QcApplyWidget::~QcApplyWidget()
{
}

///
/// @brief 界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年11月5日，新建函数
///
void QcApplyWidget::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 选择按钮默认状态为选择
    ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    ui->calibrate_sel_btn->setEnabled(false);

    // 创建模型
    m_pStdModel = new QcApplyModel(this, QcApplyModel::QC_APPLY_TYPE_ON_USE);
    m_pStdModel->SetView(ui->QcApplyTable);
    ui->QcApplyTable->setModel(m_pStdModel);

    // Sort Header
    // 升序 降序 原序
    m_pSortHeader = new SortHeaderView(Qt::Horizontal);
    m_pSortHeader->setStretchLastSection(true);
    ui->QcApplyTable->setHorizontalHeader(m_pSortHeader);

    // 默认排序初始化
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder);

    // 点击表头触发排序
    connect(m_pSortHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order) 
    {
        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            m_pStdModel->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
            ui->QcApplyTable->sortByColumn(Qah_DbIndex, Qt::AscendingOrder);
            m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
        }
        else
        {
            int iSortRole = (logicIndex == Qah_Selected) ? UI_ITEM_ROLE_ICON_SORT : Qt::DisplayRole;
            m_pStdModel->setSortRole(iSortRole);
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            ui->QcApplyTable->sortByColumn(logicIndex, qOrderFlag);
        }
    });

    // 允许排序
    ui->QcApplyTable->setSortingEnabled(true);

    // 最后一列列宽伸缩，垂直表头不可见
    ui->QcApplyTable->verticalHeader()->setVisible(false);

    // 设置试剂信息表默认行数和列数
    //m_pStdModel->setRowCount(m_ciQcApplyTblDefaultRowCnt);
    m_pStdModel->setColumnCount(m_ciQcApplyTblDefaultColCnt);

    // 选择列设置代理
    ui->QcApplyTable->setItemDelegateForColumn(Qah_Selected, new McPixmapItemDelegate(this));

    // 隐藏数据库主键列
    ui->QcApplyTable->hideColumn(Qah_DbIndex);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，新建函数
///
void QcApplyWidget::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<QcApplyHeader, QString>> qcApplyHeader = {
        { Qah_Module, tr("模块") },
        { Qah_AssayName, tr("项目名称") },
        { Qah_QcNo, tr("质控品编号") },
        { Qah_QcName, tr("质控品名称") },
        { Qah_QcBriefName, tr("质控品简称") },
        { Qah_QcType, tr("质控品类型") },
        { Qah_QcLevel, tr("质控品水平") },
        { Qah_QcLot, tr("质控品批号") },
        { Qah_QcReason, tr("质控原因") },
        { Qah_Selected, tr("选择") }
    };
    QStringList strHeaderList;
    for (auto it = qcApplyHeader.begin(); it != qcApplyHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    m_pStdModel->setHorizontalHeaderLabels(strHeaderList);
    m_pStdModel->horizontalHeaderItem(Qah_Selected)->setData(UI_TEXT_COLOR_HEADER_MARK, Qt::TextColorRole);

    // 设置表格选中模式为行选中，不可多选，不可编辑
    ui->QcApplyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->QcApplyTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->QcApplyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 自适应列宽
    ResizeTblColToContent(ui->QcApplyTable);

    // 更新设备名
    UpdateDevNameList();

    // 更新质控品下拉框
    UpdateQcDocCombo();

    // 更新质控申请表
    ReloadQcReqTable();

    // 初始化对话框
    // 获取质控品名称列表
//     QStringList strDevNameList;
//     m_pDefaultQcDlg = new DefaultQcDlg(m_strDevNameList, this);
//     LoadDlg(m_pDefaultQcDlg);
//     InitConnect(m_pDefaultQcDlg);
//     if (!gUiAdapterPtr()->GetGroupDevNameListButIse(strDevNameList))
//     {
//         ULOG(LOG_ERROR, "%s(), GetBkQcDevList() failed", __FUNCTION__);
//         return;
//     }
// 
//     // 初始化对话框
//     strDevNameList.push_front("");
// 
//     // 初始化对话框
//     m_pBackupRgntQcDlg = new BackupRgntQcDlg(strDevNameList, this);
//     LoadDlg(m_pBackupRgntQcDlg);
//     InitConnect(m_pBackupRgntQcDlg);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，新建函数
///
void QcApplyWidget::InitConnect()
{
    // 选择按钮被点击
    connect(ui->calibrate_sel_btn, SIGNAL(clicked()), this, SLOT(OnSelBtnClicked()));

    // 质控申请表单元格被点击
    connect(ui->QcApplyTable, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnQcReqTableItemClicked(const QModelIndex&)));

    // 选中项改变
    QItemSelectionModel* pSelModel = ui->QcApplyTable->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(UpdateQcApplySelBtn()));
    }

    // 质控品下拉框选项改变
    connect(ui->QcDocCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnQcDocComboIndexChanged(int)));
    connect(ui->QcDocCombo, SIGNAL(ResetText()), this, SLOT(OnQcDocComboReset()));

    // 查询按钮被点击
    connect(ui->QueryBtn, SIGNAL(clicked()), this, SLOT(OnQueryBtnClicked()));

	// 初始状态筛选按钮不可用，有文字的时候才可以用
	ui->QueryBtn->setEnabled(false);
	connect(ui->QcDocCombo, &QComboBox::editTextChanged, this, [this] {
		if (ui->QcDocCombo->lineEdit()->text().isEmpty())
			ui->QueryBtn->setEnabled(false);
		else
			ui->QueryBtn->setEnabled(true);
	});

    // 默认质控按钮点击
    connect(ui->DefaultQcBtn, SIGNAL(clicked()), this, SLOT(OnDefaultQcBtnClicked()));

    // 备用瓶质控
    connect(ui->BackupRgntQcBtn, SIGNAL(clicked()), this, SLOT(OnBackupRgntQcBtnClicked()));

    // 监听试剂更新信息
//     REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, UpdateSingleQcReqInfo);
// 
    // 监听免疫质控申请更新信息
    REGISTER_HANDLER(MSG_ID_IM_QCAPP_INFO_UPDATE, this, OnImQcApplyInfoUpdate);

    // 监听生化质控申请更新信息
    REGISTER_HANDLER(MSG_ID_QCAPP_INFO_UPDATE, this, OnChQcApplyInfoUpdate);

    // 监听免疫试剂信息更新（临时）
    REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, OnImReagentInfoUpdate);

    // 监听生化试剂信息更新（临时）
    REGISTER_HANDLER(MSG_ID_CH_REAGENT_SUPPLY_INFO_UPDATE, this, OnChReagentInfoUpdate);

    // 监听生化试剂信息更新（临时）
    REGISTER_HANDLER(MSG_ID_REAGENT_LOADER_UPDATE, this, OnChReagentInfoUpdate);

    // 监听质控品信息更新
    REGISTER_HANDLER(MSG_ID_QC_DOC_INFO_UPDATE, this, OnQcDocInfoInfoUpdate);

    // 角落按钮索引更新
    REGISTER_HANDLER(MSG_ID_QC_TAB_CORNER_INDEX_UPDATE, this, OnTabCornerIndexChanged);

    // 角落筛选条件被重置
    REGISTER_HANDLER(MSG_ID_QC_TAB_CORNER_QRY_COND_RESET, this, OnTabCornerQryCondReset);

    // 项目信息更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, ReloadQcReqTable);

    // 监听免疫试剂更新（剩余测试数为0时更新）
    //REGISTER_HANDLER(MSG_ID_IM_REAGENT_INFO_UPDATE, this, UpdateImReagentChanged);
}

///
/// @brief
///     初始化信号槽连接
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::InitConnect(DefaultQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 单元格被点击
    connect(pDlg, SIGNAL(SigItemClicked(const QModelIndex&)), this, SLOT(OnDefaultQcDlgTblItemClicked(const QModelIndex&)));

    // 当前质控名改变
    connect(pDlg, SIGNAL(SigCurQcNameChanged()), this, SLOT(OnDefaultQcDlgQcNameChanged()));

    // 当前选中设备改变
    connect(pDlg, SIGNAL(SigCurDevNameChanged(const QString&)), this, SLOT(OnDefaultQcDlgDevNameChanged(const QString&)));
    connect(pDlg, SIGNAL(SigCurDevNameChanged(const QString&)), this, SLOT(OnDefaultQcDlgSelectionChanged()));

    // 对话框数据更新
    connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));
    connect(pDlg, SIGNAL(SigDataUpdate()), this, SLOT(OnDlgDataUpdate()));

    // 选择按钮被点击
    connect(pDlg, SIGNAL(SigSelBtnClicked()), this, SLOT(OnDefaultQcDlgSelBtnClicked()));

    // 选中项改变
    connect(pDlg, SIGNAL(SigSelectionChanged()), this, SLOT(OnDefaultQcDlgSelectionChanged()));
}

///
/// @brief
///     初始化信号槽连接
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::InitConnect(BackupRgntQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 单元格被点击
    connect(pDlg, SIGNAL(SigItemClicked(const QModelIndex&)), this, SLOT(OnBackupRgntQcDlgTblItemClicked(const QModelIndex&)));

    // 当前质控名改变
    connect(pDlg, SIGNAL(SigCurQcNameChanged()), this, SLOT(OnBackupRgntQcDlgQcNameChanged()));

    // 当前选中设备改变
    connect(pDlg, SIGNAL(SigCurDevNameChanged(const QString&)), this, SLOT(OnBackupRgntQcDlgDevNameChanged(const QString&)));
    connect(pDlg, SIGNAL(SigCurDevNameChanged(const QString&)), this, SLOT(OnBackupRgntQcDlgSelectionChanged()));

    // 对话框数据更新
    connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));
    connect(pDlg, SIGNAL(SigDataUpdate()), this, SLOT(OnDlgDataUpdate()));

    // 选择按钮被点击
    connect(pDlg, SIGNAL(SigSelBtnClicked()), this, SLOT(OnBackupRgntQcDlgSelBtnClicked()));

    // 选中项改变
    connect(pDlg, SIGNAL(SigSelectionChanged()), this, SLOT(OnBackupRgntQcDlgSelectionChanged()));
}

///
/// @brief
///     加载对话框
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::LoadDlg(DefaultQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 选择按钮默认状态为选择
    pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    pDlg->ui->SelBtn->setEnabled(false);

    // 如果设备列表只有1个，则隐藏设备显示
    if (m_strDevNameList.size() <= 1)
    {
        pDlg->ui->DevListFrame->hide();
        pDlg->m_strCurDevName = m_strCurDevName;
    }

    // 初始化下拉框
    // 清空质控品下拉框列表
    pDlg->ui->QcNameCombo->clear();

    // 获取质控品名称列表
    QStringList strQcDocNameList;
    if (!gUiAdapterPtr()->GetQcDocList(strQcDocNameList))
    {
        ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
        return;
    }

    // 将质控品文档添加到下拉框中
    pDlg->ui->QcNameCombo->addItems(UiCommon::QStringListFiltSameItem(strQcDocNameList));
	pDlg->ui->QcNameCombo->setCurrentIndex(-1);

    // 失效日期列设置代理
    pDlg->ui->DefaultQcTbl->setItemDelegateForColumn(DefaultQcDlg::Dqh_ExpDate, new CReadOnlyDelegate(pDlg));

    // 选择列设置代理
    pDlg->ui->DefaultQcTbl->setItemDelegateForColumn(DefaultQcDlg::Dqh_Selected, new McPixmapItemDelegate(pDlg));

    // 更新对话框
    UpdateDlg(pDlg);
}

///
/// @brief
///     加载对话框
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::LoadDlg(BackupRgntQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 选择按钮默认状态为选择
    pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    pDlg->ui->SelBtn->setEnabled(false);

    // 如果设备列表只有1个，则隐藏设备显示
    if (m_strDevNameList.size() <= 1)
    {
        pDlg->ui->DevListFrame->hide();
        pDlg->m_strCurDevName = m_strCurDevName;
    }

    // 初始化下拉框
    // 清空质控品下拉框列表
    pDlg->ui->QcNameCombo->clear();

    // 获取质控品名称列表
    QStringList strQcDocNameList;
    if (!gUiAdapterPtr()->GetQcDocListButIse(strQcDocNameList))
    {
        ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
        return;
    }

    // 将质控品文档添加到下拉框中
    pDlg->ui->QcNameCombo->addItems(UiCommon::QStringListFiltSameItem(strQcDocNameList));
	pDlg->ui->QcNameCombo->setCurrentIndex(-1);

    // 失效日期列设置代理
    pDlg->ui->BackRgntQcTbl->setItemDelegateForColumn(BackupRgntQcDlg::Bqh_ExpDate, new CReadOnlyDelegate(pDlg));

    // 选择列设置代理
    pDlg->ui->BackRgntQcTbl->setItemDelegateForColumn(BackupRgntQcDlg::Bqh_Selected, new McPixmapItemDelegate(pDlg));

    // 更新对话框
    UpdateDlg(pDlg);
}

///
/// @brief
///     Tab角落按钮索引改变
///
/// @param[in]  iBtnIdx  按钮索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月15日，新建函数
///
void QcApplyWidget::OnTabCornerIndexChanged(int iBtnIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果界面未显示，则不响应
    if (!isVisible())
    {
        return;
    }

    if (iBtnIdx < 0 || iBtnIdx >= m_strDevNameList.size())
    {
        ULOG(LOG_ERROR, "%s(), invalid param", __FUNCTION__);
        return;
    }

    m_strCurDevName = m_strDevNameList.at(iBtnIdx);

    // 更新质控品下拉框
    UpdateQcDocCombo();

    // 更新质控申请表
    ReloadQcReqTable();

    // 更新选择按钮状态
    UpdateQcApplySelBtn();

    // 滚动到顶部
    ui->QcApplyTable->scrollToTop();
}

///
/// @brief
///     角落查询条件被重置
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建函数
///
void QcApplyWidget::OnTabCornerQryCondReset()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 重置下拉框内容为空
    int iIdx = ui->QcDocCombo->findText("");
    if (iIdx >= 0)
    {
        ui->QcDocCombo->setCurrentIndex(iIdx);
        ReloadQcReqTable();
    }
}

///
/// @brief
///     默认质控对话框表格单元格被点击
///
/// @param[in]  index  表格单元项索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnDefaultQcDlgTblItemClicked(const QModelIndex& index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    DefaultQcDlg* pDlg = m_pDefaultQcDlg;
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 检查参数
    if (!index.data().isValid() || (index.column() != DefaultQcDlg::Dqh_Selected))
    {
        // 该参数不可点击修改，无需报警
        ULOG(LOG_INFO, "%s(), not sel or cancel sel", __FUNCTION__);
        return;
    }

    // 获取该行信息
    QC_APPLY_INFO stuQcApplyItem;
    if (!pDlg->m_pStdModel->GetApplyInfoByRow(index.row(), stuQcApplyItem))
    {
        ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
        return;
    }

    // 更新质控申请选择状态(// 选择状态反置)(暂时取消监听对话框信息更新)
    disconnect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));
    if (!gUiAdapterPtr(stuQcApplyItem.strDevName, stuQcApplyItem.strDevGrpName, stuQcApplyItem.iModuleNo)->UpdateDefaultQcItemSelected(stuQcApplyItem, !stuQcApplyItem.bSelected))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcApplyItemSelected() failed", __FUNCTION__);
        connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));
        return;
    }
    connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));

    // 缓存质控申请数据
    pDlg->m_pStdModel->SetApplyInfoByRow(index.row(), stuQcApplyItem);
    pDlg->m_pStdModel->UpdateLaout();

    // 更新质控申请表质控原因
    UpdateQcApplyReason(stuQcApplyItem);

    // 选择按钮状态取反
    bool bExeSel = stuQcApplyItem.bSelected;
    if (bExeSel)
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("取消选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
    else
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    }

    // 如果按选择列排序,则恢复排序
    int iSortCol = pDlg->m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = pDlg->m_pSortHeader->GetCurSortOrder();
    if (iSortCol == DefaultQcDlg::Dqh_Selected && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = UI_ITEM_ROLE_ICON_SORT;
        pDlg->m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        pDlg->ui->DefaultQcTbl->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     默认质控对话框质控名选项改变
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnDefaultQcDlgQcNameChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    DefaultQcDlg* pDlg = qobject_cast<DefaultQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 更新对话框
    UpdateDlg(pDlg);

    // 滚动到顶部
    pDlg->ui->DefaultQcTbl->scrollToTop();

    // 设置筛选文本
    //QString strQcDocName = pDlg->ui->QcNameCombo->currentText();
    //pDlg->ui->CondWidget->SetText(strQcDocName.isEmpty() ? strQcDocName : m_cstrScreenTip + strQcDocName);
}

///
/// @brief
///     默认质控对话框设备名选项改变
///
/// @param[in]  strDevName  当前设备名
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnDefaultQcDlgDevNameChanged(const QString& strDevName)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    DefaultQcDlg* pDlg = qobject_cast<DefaultQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取之前的选中项
    QString strSelDocName = pDlg->ui->QcNameCombo->currentText();
    pDlg->ui->QcNameCombo->blockSignals(true);

    // 初始化下拉框
    // 清空质控品下拉框列表
    pDlg->ui->QcNameCombo->clear();

    // 获取质控品名称列表
    QStringList strQcDocNameList;
    if (!gUiAdapterPtr()->WhetherContainOtherSubDev(strDevName))
    {
        if (!gUiAdapterPtr(strDevName)->GetQcDocList(strQcDocNameList))
        {
            ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
            pDlg->ui->QcNameCombo->blockSignals(false);
            return;
        }
    }
    else
    {
        // 包含其他子项目
        QStringList strSubDevNameList;
        if (!gUiAdapterPtr()->GetSubDevNameList(strDevName, strSubDevNameList) || strSubDevNameList.isEmpty())
        {
            ULOG(LOG_ERROR, "%s(), GetSubDevNameList() failed", __FUNCTION__);
            pDlg->ui->QcNameCombo->blockSignals(false);
            return;
        }

        // 以第一个子设备去获取质控文档表
        if (!gUiAdapterPtr(strSubDevNameList.first(), strDevName)->GetQcDocList(strQcDocNameList))
        {
            ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
            return;
        }
    }

    // 将质控品文档添加到下拉框中
    pDlg->ui->QcNameCombo->addItems(UiCommon::QStringListFiltSameItem(strQcDocNameList));
	pDlg->ui->QcNameCombo->setCurrentIndex(-1);

    // 如果之前的选中项任然存在，则设置当前质控文档
    if (pDlg->ui->QcNameCombo->findText(strSelDocName) >= 0)
    {
        pDlg->ui->QcNameCombo->setCurrentText(strSelDocName);
    }

    // 更新对话框
    UpdateDlg(pDlg);
    pDlg->ui->QcNameCombo->blockSignals(false);

    // 滚动到顶部
    pDlg->ui->DefaultQcTbl->scrollToTop();
}

///
/// @brief
///     备用瓶质控对话框表格单元格被点击
///
/// @param[in]  index  表格单元项索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnBackupRgntQcDlgTblItemClicked(const QModelIndex& index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    BackupRgntQcDlg* pDlg = m_pBackupRgntQcDlg;
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 检查参数
    if (!index.data().isValid() || (index.column() != BackupRgntQcDlg::Bqh_Selected))
    {
        // 该参数不可点击修改，无需报警
        ULOG(LOG_INFO, "%s(), not sel or cancel sel", __FUNCTION__);
        return;
    }

    // 获取该行信息
    QC_APPLY_INFO stuQcApplyItem;
    if (!pDlg->m_pStdModel->GetApplyInfoByRow(index.row(), stuQcApplyItem))
    {
        ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
        return;
    }

    // 更新质控申请选择状态(// 选择状态反置)(并暂时取消对话框数据更新消息监听)
    disconnect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));
    if (!gUiAdapterPtr(stuQcApplyItem.strDevName, stuQcApplyItem.strDevGrpName)->UpdateBackupRgntItemSelected(stuQcApplyItem, !stuQcApplyItem.bSelected))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcApplyItemSelected() failed", __FUNCTION__);
        connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));
        return;
    }
    connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));

    // 缓存质控申请数据
    pDlg->m_pStdModel->SetApplyInfoByRow(index.row(), stuQcApplyItem);
    pDlg->m_pStdModel->UpdateLaout();

    // 选择按钮状态取反
    bool bExeSel = stuQcApplyItem.bSelected;
    if (bExeSel)
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("取消选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
    else
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    }

    // 如果按选择列排序,则恢复排序
    int iSortCol = pDlg->m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = pDlg->m_pSortHeader->GetCurSortOrder();
    if (iSortCol == BackupRgntQcDlg::Bqh_Selected && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = UI_ITEM_ROLE_ICON_SORT;
        pDlg->m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        pDlg->ui->BackRgntQcTbl->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     备用瓶质控对话框质控名选项改变
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnBackupRgntQcDlgQcNameChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    BackupRgntQcDlg* pDlg = qobject_cast<BackupRgntQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 更新对话框
    UpdateDlg(pDlg);

    // 滚动到顶部
    pDlg->ui->BackRgntQcTbl->scrollToTop();

}

///
/// @brief
///     备用瓶质控对话框设备名选项改变
///
/// @param[in]  strDevName  当前设备名
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnBackupRgntQcDlgDevNameChanged(const QString& strDevName)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    BackupRgntQcDlg* pDlg = qobject_cast<BackupRgntQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取之前的选中项
    QString strSelDocName = pDlg->ui->QcNameCombo->currentText();
    pDlg->ui->QcNameCombo->blockSignals(true);

    // 初始化下拉框
    // 清空质控品下拉框列表
    pDlg->ui->QcNameCombo->clear();

    // 获取质控品名称列表
    QStringList strQcDocNameList;
    if (!gUiAdapterPtr()->GetQcDocListButIse(strQcDocNameList))
    {
        ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
        pDlg->ui->QcNameCombo->blockSignals(false);
        return;
    }

    // 将质控品文档添加到下拉框中
    pDlg->ui->QcNameCombo->addItems(UiCommon::QStringListFiltSameItem(strQcDocNameList));
	pDlg->ui->QcNameCombo->setCurrentIndex(-1);

    // 如果之前的选中项任然存在，则设置当前质控文档
    if (pDlg->ui->QcNameCombo->findText(strSelDocName) >= 0)
    {
        pDlg->ui->QcNameCombo->setCurrentText(strSelDocName);
    }

    // 更新对话框
    UpdateDlg(pDlg);
    pDlg->ui->QcNameCombo->blockSignals(false);

    // 滚动到顶部
    pDlg->ui->BackRgntQcTbl->scrollToTop();
}

///
/// @brief
///     对话框数据更新
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月4日，新建函数
///
void QcApplyWidget::OnDlgDataUpdate()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 尝试获取默认对话框
    {
        DefaultQcDlg* pDlg = qobject_cast<DefaultQcDlg*>(sender());
        if (pDlg != Q_NULLPTR && pDlg->isVisible())
        {
            // 刷新对话框
            UpdateDlgQcDocCombo(pDlg);
            UpdateDlg(pDlg);
            return;
        }
    }

    // 尝试获取备用瓶质控对话框
    {
        BackupRgntQcDlg* pDlg = qobject_cast<BackupRgntQcDlg*>(sender());
        if (pDlg != Q_NULLPTR && pDlg->isVisible())
        {
            // 刷新对话框
            UpdateDlgQcDocCombo(pDlg);
            UpdateDlg(pDlg);
            return;
        }
    }
}

///
/// @brief
///     试剂信息更新(临时)
///
/// @param[in]  reag  设备编号
/// @param[in]  type  位置信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年01月04日，新建函数
///
void QcApplyWidget::OnImReagentInfoUpdate(const im::tf::ReagentInfoTable& reag, const im::tf::ChangeType::type type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(reag);
    Q_UNUSED(type);
    ReloadQcReqTable();
    emit SigDlgDataUpdate();
}

///
/// @brief
///     试剂信息更新(临时)
///
/// @param[in]  supplyUpdates  耗材更新信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年01月04日，新建函数
///
void QcApplyWidget::OnChReagentInfoUpdate(const std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>>& supplyUpdates)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 遍历所有耗材更新消息，如果都是余量更新，则忽略
    for (const auto& stuSplUpdate : supplyUpdates)
    {
        // 如果是修改则忽略
        /*if (stuSplUpdate.updateType == tf::UpdateType::UPDATE_TYPE_MODIFY)
        {
            continue;
        }*/

        // 试剂有加卸载，则更新
        Q_UNUSED(supplyUpdates);
        ReloadQcReqTable();
        emit SigDlgDataUpdate();
        return;
    }
}

///
/// @brief
///     质控申请信息更新(临时)
///
/// @param[in]  qa  质控申请信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年01月04日，新建函数
///
void QcApplyWidget::OnImQcApplyInfoUpdate(im::tf::QcApply qa)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(qa);
    ReloadQcReqTable();
    emit SigDlgDataUpdate();
}

///
/// @brief
///     质控申请信息更新(临时)
///
/// @param[in]  sus  质控申请信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年01月04日，新建函数
///
void QcApplyWidget::OnChQcApplyInfoUpdate(std::vector<ch::tf::QcApply, std::allocator<ch::tf::QcApply>> sus)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 质控申请表
//     for (const auto& stuApply : sus)
//     {
//         // 翻译质控申请更新
//         TranslateQcApplyUpdate(stuApply);
//     }
    ReloadQcReqTable();
    emit SigDlgDataUpdate();
}

///
/// @brief
///     质控品信息更新
///
/// @param[in]  vQcDoc  质控品信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年1月4日，新建函数
///
void QcApplyWidget::OnQcDocInfoInfoUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> vQcDoc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(vQcDoc);
    UpdateQcDocCombo();
    ReloadQcReqTable();
    emit SigDlgDataUpdate();
}

///
/// @brief
///     免疫试剂信息更新
///
/// @param[in]  stuRgntInfo  试剂信息
/// @param[in]  changeType   更新方式
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月31日，新建函数
///
void QcApplyWidget::UpdateImReagentChanged(const im::tf::ReagentInfoTable& stuRgntInfo, im::tf::ChangeType::type changeType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if ((stuRgntInfo.residualTestNum <= 0) || 
        (changeType == im::tf::ChangeType::Delete) ||
        (changeType == im::tf::ChangeType::Add))
    {
        ReloadQcReqTable();
        emit SigDlgDataUpdate();
    }
}

///
/// @brief
///     选择按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void QcApplyWidget::OnSelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取选中项
    QItemSelectionModel* pSelModel = ui->QcApplyTable->selectionModel();
    if (pSelModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pSelModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 判断是执行选择还是取消选择
    bool bExeSel = ui->calibrate_sel_btn->property(m_cstrSelBtnStatusName).toBool();
    QModelIndexList selModIdxList = pSelModel->selectedRows();
    for (const auto& indexRow : selModIdxList)
    {
        // 如果未显示则忽略
        if (ui->QcApplyTable->isRowHidden(indexRow.row()))
        {
            continue;
        }

        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!m_pStdModel->GetApplyInfoByRow(indexRow.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            continue;
        }

        // 更新质控申请选择状态(// 选择状态反置)
        if (!gUiAdapterPtr(stuQcApplyItem.strDevName, stuQcApplyItem.strDevGrpName, stuQcApplyItem.iModuleNo)->UpdateQcApplyItemSelected(stuQcApplyItem, bExeSel))
        {
            ULOG(LOG_ERROR, "%s(), UpdateQcApplyItemSelected() failed", __FUNCTION__);
            continue;
        }

        // 更新表格信息
        m_pStdModel->SetApplyInfoByRow(indexRow.row(), stuQcApplyItem);
    }

    // 更新表格
    m_pStdModel->UpdateLaout();

    // 选择按钮状态取反
    if (bExeSel)
    {
        // 将选择按钮文本重置，默认禁能
        ui->calibrate_sel_btn->setText(tr("取消选择"));
        ui->calibrate_sel_btn->setEnabled(true);
        ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
    else
    {
        // 将选择按钮文本重置，默认禁能
        ui->calibrate_sel_btn->setText(tr("选择"));
        ui->calibrate_sel_btn->setEnabled(true);
        ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    }

    // 如果按选择列排序,则恢复排序
    int iSortCol = m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = m_pSortHeader->GetCurSortOrder();
    if (iSortCol == Qah_Selected && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = UI_ITEM_ROLE_ICON_SORT;
        m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->QcApplyTable->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     更新质控申请选择按钮
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void QcApplyWidget::UpdateQcApplySelBtn()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_pStdModel->SetSelQcApplyId("");

    // 更新质控申请选择按钮且没有选中项改变
    UpdateQcApplySelBtnWithoutSelChange();
}

///
/// @brief
///     更新质控申请选择按钮且没有选中项改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月24日，新建函数
///
void QcApplyWidget::UpdateQcApplySelBtnWithoutSelChange()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 将选择按钮文本重置，默认禁能
    ui->calibrate_sel_btn->setText(tr("选择"));
    ui->calibrate_sel_btn->setEnabled(false);
    ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);

    // 获取选中项
    QItemSelectionModel* pSelModel = ui->QcApplyTable->selectionModel();
    if (pSelModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pSelModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 判断是执行选择还是取消选择
    bool bExeSel = ui->calibrate_sel_btn->property(m_cstrSelBtnStatusName).toBool();
    QModelIndexList selModIdxList = pSelModel->selectedRows();
    for (const auto& indexRow : selModIdxList)
    {
        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!m_pStdModel->GetApplyInfoByRow(indexRow.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            continue;;
        }

        // 选择了非空行，按钮使能
        ui->calibrate_sel_btn->setEnabled(true);

        // 如果是未选择
        if (!stuQcApplyItem.bSelected)
        {
            ui->calibrate_sel_btn->setText(tr("选择"));
            ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
            break;
        }

        // 都是选择时，默认为取消选择
        ui->calibrate_sel_btn->setText(tr("取消选择"));
        ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
}

///
/// @brief
///     默认质控对话框选择按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月6日，新建函数
///
void QcApplyWidget::OnDefaultQcDlgSelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    DefaultQcDlg* pDlg = qobject_cast<DefaultQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中项
    QItemSelectionModel* pSelModel = pDlg->ui->DefaultQcTbl->selectionModel();
    if (pSelModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pSelModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 暂时断开数据更新监听（接下来要批量更新选择）
    disconnect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));

    // 判断是执行选择还是取消选择
    bool bExeSel = pDlg->ui->SelBtn->property(m_cstrSelBtnStatusName).toBool();
    QModelIndexList selModIdxList = pSelModel->selectedRows();
    for (const auto& indexRow : selModIdxList)
    {
        // 如果未显示则忽略
        if (pDlg->ui->DefaultQcTbl->isRowHidden(indexRow.row()))
        {
            continue;
        }

        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!pDlg->m_pStdModel->GetApplyInfoByRow(indexRow.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            continue;;
        }

        // 更新质控申请选择状态(// 选择状态反置)(暂时取消监听对话框信息更新)
        if (!gUiAdapterPtr(stuQcApplyItem.strDevName, stuQcApplyItem.strDevGrpName, stuQcApplyItem.iModuleNo)->UpdateDefaultQcItemSelected(stuQcApplyItem, bExeSel))
        {
            ULOG(LOG_ERROR, "%s(), UpdateQcApplyItemSelected() failed", __FUNCTION__);
            continue;
        }

        // 缓存质控申请数据
        pDlg->m_pStdModel->SetApplyInfoByRow(indexRow.row(), stuQcApplyItem);

        // 更新质控申请表质控原因
        UpdateQcApplyReason(stuQcApplyItem);
    }
    pDlg->m_pStdModel->UpdateLaout();

    // 重新监听数据更新
    connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));

    // 选择按钮状态取反
    if (bExeSel)
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("取消选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
    else
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    }

    // 如果按选择列排序,则恢复排序
    int iSortCol = pDlg->m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = pDlg->m_pSortHeader->GetCurSortOrder();
    if (iSortCol == DefaultQcDlg::Dqh_Selected && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = UI_ITEM_ROLE_ICON_SORT;
        pDlg->m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        pDlg->ui->DefaultQcTbl->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     默认质控对话框选中项改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月6日，新建函数
///
void QcApplyWidget::OnDefaultQcDlgSelectionChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    DefaultQcDlg* pDlg = qobject_cast<DefaultQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 选择项清空
    pDlg->m_pStdModel->SetSelQcApplyId("");

    // 将选择按钮文本重置，默认禁能
    pDlg->ui->SelBtn->setText(tr("选择"));
    pDlg->ui->SelBtn->setEnabled(false);
    pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);

    // 获取选中项
    QItemSelectionModel* pSelModel = pDlg->ui->DefaultQcTbl->selectionModel();
    if (pSelModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pSelModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 判断是执行选择还是取消选择
    bool bExeSel = pDlg->ui->SelBtn->property(m_cstrSelBtnStatusName).toBool();
    QModelIndexList selModIdxList = pSelModel->selectedRows();
    for (const auto& indexRow : selModIdxList)
    {
        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!pDlg->m_pStdModel->GetApplyInfoByRow(indexRow.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            continue;;
        }

        // 选择了非空行，按钮使能
        pDlg->ui->SelBtn->setEnabled(true);

        // 如果是未选择
        if (!stuQcApplyItem.bSelected)
        {
            pDlg->ui->SelBtn->setText(tr("选择"));
            pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
            break;
        }

        // 都是选择时，默认为取消选择
        pDlg->ui->SelBtn->setText(tr("取消选择"));
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
}

///
/// @brief
///     备用瓶质控对话框选择按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月6日，新建函数
///
void QcApplyWidget::OnBackupRgntQcDlgSelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    BackupRgntQcDlg* pDlg = qobject_cast<BackupRgntQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 获取选中项
    QItemSelectionModel* pSelModel = pDlg->ui->BackRgntQcTbl->selectionModel();
    if (pSelModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pSelModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 暂时断开数据更新监听（接下来要批量更新选择）
    disconnect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));

    // 判断是执行选择还是取消选择
    bool bExeSel = pDlg->ui->SelBtn->property(m_cstrSelBtnStatusName).toBool();
    QModelIndexList selModIdxList = pSelModel->selectedRows();
    for (const auto& indexRow : selModIdxList)
    {
        // 如果未显示则忽略
        if (pDlg->ui->BackRgntQcTbl->isRowHidden(indexRow.row()))
        {
            continue;
        }

        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!pDlg->m_pStdModel->GetApplyInfoByRow(indexRow.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            continue;;
        }

        // 更新质控申请选择状态(// 选择状态反置)(并暂时取消对话框数据更新消息监听)
        if (!gUiAdapterPtr(stuQcApplyItem.strDevName, stuQcApplyItem.strDevGrpName)->UpdateBackupRgntItemSelected(stuQcApplyItem, bExeSel))
        {
            ULOG(LOG_ERROR, "%s(), UpdateQcApplyItemSelected() failed", __FUNCTION__);
            continue;
        }

        // 缓存质控申请数据
        pDlg->m_pStdModel->SetApplyInfoByRow(indexRow.row(), stuQcApplyItem);
    }
    pDlg->m_pStdModel->UpdateLaout();

    // 重新监听数据更新
    connect(this, SIGNAL(SigDlgDataUpdate()), pDlg, SIGNAL(SigDataUpdate()));

    // 选择按钮状态取反
    if (bExeSel)
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("取消选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
    else
    {
        // 将选择按钮文本重置，默认禁能
        pDlg->ui->SelBtn->setText(tr("选择"));
        pDlg->ui->SelBtn->setEnabled(true);
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    }

    // 如果按选择列排序,则恢复排序
    int iSortCol = pDlg->m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = pDlg->m_pSortHeader->GetCurSortOrder();
    if (iSortCol == BackupRgntQcDlg::Bqh_Selected && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = UI_ITEM_ROLE_ICON_SORT;
        pDlg->m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        pDlg->ui->BackRgntQcTbl->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     备用瓶质控对话框选中项改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月6日，新建函数
///
void QcApplyWidget::OnBackupRgntQcDlgSelectionChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信号发送者
    BackupRgntQcDlg* pDlg = qobject_cast<BackupRgntQcDlg*>(sender());
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 选择项清空
    pDlg->m_pStdModel->SetSelQcApplyId("");

    // 将选择按钮文本重置，默认禁能
    pDlg->ui->SelBtn->setText(tr("选择"));
    pDlg->ui->SelBtn->setEnabled(false);
    pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);

    // 获取选中项
    QItemSelectionModel* pSelModel = pDlg->ui->BackRgntQcTbl->selectionModel();
    if (pSelModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pSelModel == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 判断是执行选择还是取消选择
    bool bExeSel = pDlg->ui->SelBtn->property(m_cstrSelBtnStatusName).toBool();
    QModelIndexList selModIdxList = pSelModel->selectedRows();
    for (const auto& indexRow : selModIdxList)
    {
        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!pDlg->m_pStdModel->GetApplyInfoByRow(indexRow.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            continue;;
        }

        // 选择了非空行，按钮使能
        pDlg->ui->SelBtn->setEnabled(true);

        // 如果是未选择
        if (!stuQcApplyItem.bSelected)
        {
            pDlg->ui->SelBtn->setText(tr("选择"));
            pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
            break;
        }

        // 都是选择时，默认为取消选择
        pDlg->ui->SelBtn->setText(tr("取消选择"));
        pDlg->ui->SelBtn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
}

///
/// @brief
///     更新对话框
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::UpdateDlg(DefaultQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取当前选中项ID
    QModelIndex curIndex = pDlg->ui->DefaultQcTbl->currentIndex();
    if (curIndex.isValid())
    {
        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!pDlg->m_pStdModel->GetApplyInfoByRow(curIndex.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            return;;
        }

        pDlg->m_pStdModel->SetSelQcApplyId(stuQcApplyItem.strID);
    }

    // 禁能排序
    //pDlg->ui->DefaultQcTbl->setSortingEnabled(false);

    // 清空表格内容
    //pDlg->m_pStdModel->setRowCount(0);
    //pDlg->m_pStdModel->setRowCount(pDlg->m_ciDefaultQcTblDefaultRowCnt);

    // 获取当前质控品名称
    QString strQcDocName = pDlg->ui->QcNameCombo->currentText();

    // 清空数据
    pDlg->m_pStdModel->m_lstDataBuffer.clear();

    // 获取质控申请表信息
    //QList<QC_APPLY_INFO> stuQcApplyInfo;
    if (!gUiAdapterPtr()->WhetherContainOtherSubDev(pDlg->m_strCurDevName))
    {
        // 没有包含其他子项目
        if (!gUiAdapterPtr(pDlg->m_strCurDevName)->QueryDefaultQcTblInfo("", pDlg->m_pStdModel->m_lstDataBuffer))
        {
            ULOG(LOG_ERROR, "%s(), QueryDefaultQcTblInfo() failed", __FUNCTION__);
            return;
        }
    }
    else
    {
        // 包含其他子项目
        QStringList strSubDevNameList;
        if (!gUiAdapterPtr()->GetSubDevNameList(pDlg->m_strCurDevName, strSubDevNameList))
        {
            ULOG(LOG_ERROR, "%s(), QueryQcApplyTblInfo() failed", __FUNCTION__);
            return;
        }

        // 遍历所有子设备
        for (const QString& strSubDevName : strSubDevNameList)
        {
            QList<QC_APPLY_INFO> stuSubQcApplyInfo;
            if (!gUiAdapterPtr(strSubDevName, pDlg->m_strCurDevName)->QueryDefaultQcTblInfo("", stuSubQcApplyInfo))
            {
                ULOG(LOG_ERROR, "%s(), QueryDefaultQcTblInfo() failed", __FUNCTION__);
                continue;
            }

            pDlg->m_pStdModel->m_lstDataBuffer.append(stuSubQcApplyInfo);
        }
    }

    // 质控品过滤
    pDlg->m_pStdModel->SetQcNameFilter(strQcDocName);

    // 将数据加载到质控申请表中
//     int iRow = 0;
//     for (const auto& stuQcApplyItem : stuQcApplyInfo)
//     {
//         // 在查询结果中进行质控品名称过滤
//         if (!strQcDocName.isEmpty() && !stuQcApplyItem.strQcName.contains(strQcDocName))
//         {
//             continue;
//         }
// 
//         // 如果行号大于等于行数，则行数自动增加（始终预留一行空白行）
//         if (iRow >= pDlg->m_pStdModel->rowCount())
//         {
//             pDlg->m_pStdModel->setRowCount(iRow + 1);
//         }
// 
//         // 构造沟渠日期
//         QStringList strDateTimeList = stuQcApplyItem.strQcExpDate.split(" ");
//         QString strDate("");
//         if (strDateTimeList.size() >= 1)
//         {
//             strDate = strDateTimeList.front();
//         }
// 
//         QList<QStandardItem*> itemList;
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strModuleName));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strAssayName));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strQcNo));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strQcName));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strQcBriefName));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strQcSourceType));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strQcLevel));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strQcLot));
//         itemList.push_back(GetDateItem(strDate));
//         itemList.push_back(new QStandardItem(""));
//         itemList.push_back(new QStandardItem(stuQcApplyItem.strID));
//         pDlg->m_pStdModel->insertRow(iRow, itemList);
// 
//         // 添加数据
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_Module, new QStandardItem(stuQcApplyItem.strModuleName));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_AssayName, new QStandardItem(stuQcApplyItem.strAssayName));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_QcNo, new QStandardItem(stuQcApplyItem.strQcNo));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_QcName, new QStandardItem(stuQcApplyItem.strQcName));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_QcBriefName, new QStandardItem(stuQcApplyItem.strQcBriefName));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_QcType, new QStandardItem(stuQcApplyItem.strQcSourceType));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_QcLevel, new QStandardItem(stuQcApplyItem.strQcLevel));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_QcLot, new QStandardItem(stuQcApplyItem.strQcLot));
// // 		pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_ExpDate, GetDateItem(strDate));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_Selected, new QStandardItem(""));
// //         pDlg->m_pStdModel->setItem(iRow, DefaultQcDlg::Dqh_DbIndex, new QStandardItem(stuQcApplyItem.strID));
// 
//         // 如果选中ID匹配，则选中该行
//         if (!strSelID.isEmpty() && (stuQcApplyItem.strID == strSelID))
//         {
//             pDlg->ui->DefaultQcTbl->selectRow(iRow);
//         }
// 
//         // 设置选中图标
//         pDlg->m_pStdModel->item(iRow, DefaultQcDlg::Dqh_Selected)->setIcon(stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());
//         pDlg->m_pStdModel->item(iRow, DefaultQcDlg::Dqh_Selected)->setData(stuQcApplyItem.bSelected ? (":/Leonis/resource/image/icon-select.png") : "", UI_ITEM_ROLE_ICON_SORT);
// 
//         // 缓存质控申请数据
//         pDlg->m_pStdModel->item(iRow, DefaultQcDlg::Dqh_DbIndex)->setData(QVariant::fromValue<QC_APPLY_INFO>(stuQcApplyItem), Qt::UserRole);
// 
//         // 行号自增
//         ++iRow;
//     }
// 
//     // 设置默认排序值
//     SetForDefaultSort(pDlg->m_pStdModel);
// 
//     // 单元格内容居中显示
//     SetTblTextAlign(pDlg->m_pStdModel, Qt::AlignCenter);
// 
//     // 使能更新
//     guardTbl.EndGuard();

    // 设置行数
    pDlg->m_pStdModel->setRowCount(pDlg->m_pStdModel->m_lstDataBuffer.size());

    // 更新表格
    pDlg->m_pStdModel->UpdateLaout();

    // 列宽自适应
    //ResizeTblColToContent(pDlg->ui->DefaultQcTbl);

    // 使能排序
    //pDlg->ui->DefaultQcTbl->setSortingEnabled(true);

    // 滚动到顶部
    //pDlg->ui->DefaultQcTbl->scrollToTop();

    // 如果有排序,则恢复排序
    int iSortCol = pDlg->m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = pDlg->m_pSortHeader->GetCurSortOrder();
    if (iSortCol >= 0 && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = (iSortCol == Qah_Selected) ? UI_ITEM_ROLE_ICON_SORT : Qt::DisplayRole;
        pDlg->m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        pDlg->ui->DefaultQcTbl->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     更新对话框
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::UpdateDlg(BackupRgntQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 表格更新数据效率优化
    //ViewModelDataUpdateGuard guardTbl(pDlg->ui->BackRgntQcTbl);

    // 获取当前选中项ID
    QModelIndex curIndex = pDlg->ui->BackRgntQcTbl->currentIndex();
    if (curIndex.isValid())
    {
        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!pDlg->m_pStdModel->GetApplyInfoByRow(curIndex.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            return;;
        }
        pDlg->m_pStdModel->SetSelQcApplyId(stuQcApplyItem.strID);
    }

    // 重置选中索引
    disconnect(pDlg, SIGNAL(SigSelectionChanged()), this, SLOT(OnBackupRgntQcDlgSelectionChanged()));
    pDlg->ui->BackRgntQcTbl->setCurrentIndex(QModelIndex());
    connect(pDlg, SIGNAL(SigSelectionChanged()), this, SLOT(OnBackupRgntQcDlgSelectionChanged()));

    // 禁能排序
    //pDlg->ui->BackRgntQcTbl->setSortingEnabled(false);

    // 清空表格内容
    //pDlg->m_pStdModel->setRowCount(0);
    //pDlg->m_pStdModel->setRowCount(pDlg->m_ciBackupRgntQcTblDefaultRowCnt);

    // 获取当前质控品名称
    QString strQcDocName = pDlg->ui->QcNameCombo->currentText();

    // 清空数据
    pDlg->m_pStdModel->m_lstDataBuffer.clear();

    // 获取质控申请表信息
    //QList<QC_APPLY_INFO> stuQcApplyInfo;
    if (!gUiAdapterPtr()->WhetherContainOtherSubDev(pDlg->m_strCurDevName))
    {
        // 没有包含其他子项目
        if (!gUiAdapterPtr(pDlg->m_strCurDevName)->QueryBackupRgntQcTblInfo("", pDlg->m_pStdModel->m_lstDataBuffer))
        {
            ULOG(LOG_ERROR, "%s(), QueryBackupRgntQcTblInfo() failed", __FUNCTION__);
            return;
        }
    }
    else
    {
        // 包含其他子项目
        QStringList strSubDevNameList;
        if (!gUiAdapterPtr()->GetSubDevNameList(pDlg->m_strCurDevName, strSubDevNameList))
        {
            ULOG(LOG_ERROR, "%s(), GetSubDevNameList() failed", __FUNCTION__);
            return;
        }

        // 遍历所有子设备
        for (const QString& strSubDevName : strSubDevNameList)
        {
            QList<QC_APPLY_INFO> stuSubQcApplyInfo;
            if (!gUiAdapterPtr(strSubDevName, pDlg->m_strCurDevName)->QueryBackupRgntQcTblInfo("", stuSubQcApplyInfo))
            {
                ULOG(LOG_ERROR, "%s(), QueryBackupRgntQcTblInfo() failed", __FUNCTION__);
                continue;
            }

            pDlg->m_pStdModel->m_lstDataBuffer.append(stuSubQcApplyInfo);
        }
    }

    // 备用瓶质控默认按位置排序
    pDlg->m_pStdModel->sortBackupQcApplyInfo(BackupRgntQcDlg::Bqh_Pos, Qt::AscendingOrder);

    // 质控品过滤
    pDlg->m_pStdModel->SetQcNameFilter(strQcDocName);

    // 将数据加载到质控申请表中
//     int iRow = 0;
//     for (const auto& stuQcApplyItem : stuQcApplyInfo)
//     {
//         // 在查询结果中进行质控品名称过滤
//         if (!strQcDocName.isEmpty() && !stuQcApplyItem.strQcName.contains(strQcDocName))
//         {
//             continue;
//         }
// 
//         // 如果行号大于等于行数，则行数自动增加（始终预留一行空白行）
//         if (iRow >= pDlg->m_pStdModel->rowCount())
//         {
//             pDlg->m_pStdModel->setRowCount(iRow + 1);
//         }
// 
//         // 构造沟渠日期
//         QStringList strDateTimeList = stuQcApplyItem.strQcExpDate.split(" ");
//         QString strDate("");
//         if (strDateTimeList.size() >= 1)
//         {
//             strDate = strDateTimeList.front();
//         }
// 
//         // 添加数据
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_Pos, new QStandardItem(stuQcApplyItem.strPos));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_AssayName, new QStandardItem(stuQcApplyItem.strAssayName));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_UseStatus, new QStandardItem(stuQcApplyItem.strRgntUseStatus));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_RgntNo, new QStandardItem(stuQcApplyItem.strRgntNo));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_RgntLot, new QStandardItem(stuQcApplyItem.strRgntLot));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_QcNo, new QStandardItem(stuQcApplyItem.strQcNo));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_QcName, new QStandardItem(stuQcApplyItem.strQcName));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_QcBriefName, new QStandardItem(stuQcApplyItem.strQcBriefName));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_QcType, new QStandardItem(stuQcApplyItem.strQcSourceType));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_QcLevel, new QStandardItem(stuQcApplyItem.strQcLevel));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_QcLot, new QStandardItem(stuQcApplyItem.strQcLot));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_ExpDate, GetDateItem(strDate));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_Selected, new QStandardItem(""));
//         pDlg->m_pStdModel->setItem(iRow, BackupRgntQcDlg::Bqh_DbIndex, new QStandardItem(stuQcApplyItem.strID));
// 
//         // 如果选中ID匹配，则选中该行
//         if (!strSelID.isEmpty() && (stuQcApplyItem.strID == strSelID))
//         {
//             pDlg->ui->BackRgntQcTbl->selectRow(iRow);
//         }
// 
//         // 位置列添加排序值
//         QString strSortVal = QString("%1%2").arg(stuQcApplyItem.strDevName).arg(stuQcApplyItem.iBackupRgntPos, 2, 10, QChar('0'));
//         pDlg->m_pStdModel->item(iRow, BackupRgntQcDlg::Bqh_Pos)->setData(strSortVal, UI_ITEM_ROLE_SEQ_NO_SORT);
// 
//         // 设置选中图标
//         pDlg->m_pStdModel->item(iRow, BackupRgntQcDlg::Bqh_Selected)->setIcon(stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());
//         pDlg->m_pStdModel->item(iRow, BackupRgntQcDlg::Bqh_Selected)->setData(stuQcApplyItem.bSelected ? (":/Leonis/resource/image/icon-select.png") : "", UI_ITEM_ROLE_ICON_SORT);
// 
//         // 缓存质控申请数据
//         pDlg->m_pStdModel->item(iRow, BackupRgntQcDlg::Bqh_DbIndex)->setData(QVariant::fromValue<QC_APPLY_INFO>(stuQcApplyItem), Qt::UserRole);
// 
//         // 行号自增
//         ++iRow;
//     }
// 
//     // 设置为原序排序
//     SetForDefaultSort(pDlg->m_pStdModel);
// 
//     // 单元格内容居中显示
//     SetTblTextAlign(pDlg->m_pStdModel, Qt::AlignCenter);
// 
//     // 使能更新
//     guardTbl.EndGuard();
// 

   // 设置行数
   pDlg->m_pStdModel->setRowCount(pDlg->m_pStdModel->m_lstDataBuffer.size());

   // 更新表格
   pDlg->m_pStdModel->UpdateLaout();

   // 列宽自适应
   //ResizeTblColToContent(pDlg->ui->BackRgntQcTbl);
// 
//     // 排序内容
//     pDlg->SortContent();
// 
//     // 使能排序
//     pDlg->ui->BackRgntQcTbl->setSortingEnabled(true);

    // 滚动到顶部
    //pDlg->ui->BackRgntQcTbl->scrollToTop();

    // 如果有排序,则恢复排序
    int iSortCol = pDlg->m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = pDlg->m_pSortHeader->GetCurSortOrder();
    if (iSortCol >= 0 && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = (iSortCol == Qah_Selected) ? UI_ITEM_ROLE_ICON_SORT : Qt::DisplayRole;
        pDlg->m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        pDlg->ui->BackRgntQcTbl->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     更新对话框质控品下拉框
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月18日，新建函数
///
void QcApplyWidget::UpdateDlgQcDocCombo(DefaultQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 信号阻塞
    pDlg->ui->QcNameCombo->blockSignals(true);

    // 记录质控品选项
    QString strSelQcDocName = pDlg->ui->QcNameCombo->currentText();

    // 初始化下拉框
    // 清空质控品下拉框列表
    //pDlg->ui->QcNameCombo->clear();

    // 获取质控品名称列表
    QStringList strQcDocNameList;
    if (!gUiAdapterPtr()->GetQcDocList(strQcDocNameList))
    {
        ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
        pDlg->ui->QcNameCombo->blockSignals(false);
        return;
    }

    // 将质控品文档添加到下拉框中
    QStringList strQcDocList = UiCommon::QStringListFiltSameItem(strQcDocNameList);
    for (const QString& strQcDocInfo : strQcDocList)
    {
        int iIdx = pDlg->ui->QcNameCombo->findText(strQcDocInfo);
        if (iIdx >= 0)
        {
            continue;
        }

        // 没有，则添加
        pDlg->ui->QcNameCombo->addItem(strQcDocInfo);
    }

    // 移除没有的Item项
    for (int iIdx = 0; iIdx < pDlg->ui->QcNameCombo->count(); iIdx++)
    {
        QString strItemText = pDlg->ui->QcNameCombo->itemText(iIdx);
        if (strQcDocList.contains(strItemText))
        {
            continue;
        }

        pDlg->ui->QcNameCombo->removeItem(iIdx);
    }
    
	//pDlg->ui->QcNameCombo->setCurrentIndex(-1);

    // 尝试恢复原来的选项
    if (pDlg->ui->QcNameCombo->findText(strSelQcDocName) >= 0)
    {
        pDlg->ui->QcNameCombo->setCurrentText(strSelQcDocName);
    }

    // 信号阻塞
    pDlg->ui->QcNameCombo->blockSignals(false);
}

///
/// @brief
///     更新对话框质控品下拉框
///
/// @param[in]  pDlg  目标窗口
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月18日，新建函数
///
void QcApplyWidget::UpdateDlgQcDocCombo(BackupRgntQcDlg* pDlg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (pDlg == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "%s(), pDlg == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 信号阻塞
    pDlg->ui->QcNameCombo->blockSignals(true);

    // 记录质控品选项
    QString strSelQcDocName = pDlg->ui->QcNameCombo->currentText();

    // 初始化下拉框
    // 清空质控品下拉框列表
    //pDlg->ui->QcNameCombo->clear();

    // 获取质控品名称列表
    QStringList strQcDocNameList;
    if (!gUiAdapterPtr()->GetQcDocListButIse(strQcDocNameList))
    {
        ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
        pDlg->ui->QcNameCombo->blockSignals(false);
        return;
    }

    // 将质控品文档添加到下拉框中
    QStringList strQcDocList = UiCommon::QStringListFiltSameItem(strQcDocNameList);
    for (const QString& strQcDocInfo : strQcDocList)
    {
        int iIdx = pDlg->ui->QcNameCombo->findText(strQcDocInfo);
        if (iIdx >= 0)
        {
            continue;
        }

        // 没有，则添加
        pDlg->ui->QcNameCombo->addItem(strQcDocInfo);
    }

    // 移除没有的Item项
    for (int iIdx = 0; iIdx < pDlg->ui->QcNameCombo->count(); iIdx++)
    {
        QString strItemText = pDlg->ui->QcNameCombo->itemText(iIdx);
        if (strQcDocList.contains(strItemText))
        {
            continue;
        }

        pDlg->ui->QcNameCombo->removeItem(iIdx);
    }

    // 将质控品文档添加到下拉框中
//     pDlg->ui->QcNameCombo->addItems(UiCommon::QStringListFiltSameItem(strQcDocNameList));
// 	pDlg->ui->QcNameCombo->setCurrentIndex(-1);

    // 尝试恢复原来的选项
    if (pDlg->ui->QcNameCombo->findText(strSelQcDocName) >= 0)
    {
        pDlg->ui->QcNameCombo->setCurrentText(strSelQcDocName);
    }

    // 信号阻塞
    pDlg->ui->QcNameCombo->blockSignals(false);
}

///
/// @brief
///     设置为默认排序
///
/// @param[in]  pTableModel  表格控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月30日，新建函数
///
// void QcApplyWidget::SetForDefaultSort(QStandardItemModel* pTableModel)
// {
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     // 参数检查
//     if (pTableModel == Q_NULLPTR)
//     {
//         ULOG(LOG_WARN, "%s(), pTableModel == Q_NULLPTR", __FUNCTION__);
//         return;
//     }
// 
//     // 将数据加载到质控申请表中
//     for (int iRow = 0; iRow < pTableModel->rowCount(); iRow++)
//     {
//         // 设置默认排序值
//         for (int iCol = 0; iCol < pTableModel->columnCount(); iCol++)
//         {
//             // 获取Item
//             QStandardItem* pItem = pTableModel->item(iRow, iCol);
//             if (pItem == Q_NULLPTR)
//             {
//                 continue;
//             }
// 
//             // 设置默认排序值为行号
//             pItem->setData(iRow, UI_ITEM_ROLE_DEFAULT_SORT);
//         }
//     }
// }

///
/// @brief 获取日期Item,如果过期，则用红色填充单元格，并且字体也为红色
///
/// @param[in]  StrDate  yyyy-MM-dd固定格式的日期字符串
///
/// @return 设置好的item
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月14日，新建函数
///
QStandardItem* QcApplyWidget::GetDateItem(const QString & StrDate)
{
	QStandardItem * item = new QStandardItem(StrDate);
	item->setTextAlignment(Qt::AlignCenter);

	// 失效日期
	QDate expDate = QDate::fromString(StrDate, "yyyy-MM-dd");

	// 是否过期
	if (expDate.isValid() && expDate <= QDate::currentDate())
	{
		item->setData(UI_REAGENT_WARNCOLOR, Qt::UserRole + 5);
		item->setData(QColor(UI_REAGENT_WARNFONT), Qt::ForegroundRole);
	}

	return item;
}

///
/// @brief
///     解析质控申请更新
///
/// @param[in]  chQcApply  质控申请
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月27日，新建函数
///
void QcApplyWidget::TranslateQcApplyUpdate(const ch::tf::QcApply& chQcApply)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 参数检查
    if (!chQcApply.__isset.assayCode ||
        !chQcApply.__isset.deviceSN || 
        !chQcApply.__isset.qcDocId || 
        !chQcApply.__isset.qcType || 
        !chQcApply.__isset.select || 
        ((chQcApply.qcType == ch::tf::QcType::QC_TYPE_BACKUP) && (!chQcApply.__isset.lot || !chQcApply.__isset.sn)))
    {
        ULOG(LOG_WARN, "%s(), invalid param.", __FUNCTION__);
        return;
    }

    // 获取模型
    QcApplyModel* pModel = Q_NULLPTR;
    int iDataCol = -1;
    if (chQcApply.qcType == ch::tf::QcType::QC_TYPE_ONUSE)
    {
        pModel = m_pStdModel;
        iDataCol = Qah_DbIndex;
    }
    else if (chQcApply.qcType == ch::tf::QcType::QC_TYPE_DEFAULT && m_pDefaultQcDlg != Q_NULLPTR)
    {
        pModel = m_pDefaultQcDlg->m_pStdModel;
        iDataCol = DefaultQcDlg::Dqh_DbIndex;
    }
    else if (chQcApply.qcType == ch::tf::QcType::QC_TYPE_BACKUP  && m_pBackupRgntQcDlg != Q_NULLPTR)
    {
        pModel = m_pBackupRgntQcDlg->m_pStdModel;
        iDataCol = BackupRgntQcDlg::Bqh_DbIndex;
    }

    // 没有对应质控类型则返回
    if ((pModel == Q_NULLPTR) || (iDataCol < 0))
    {
        ULOG(LOG_INFO, "%s(), unknown qc type", __FUNCTION__);
        return;
    }

    // 遍历模型，找到对应数据
    for (int iRow = 0; iRow < pModel->rowCount(); iRow++)
    {
        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!pModel->GetApplyInfoByRow(iRow, stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            return;;
        }

        // 比对信息看是否匹配
        QPair<QString, QString> strPairDevName = gUiAdapterPtr()->GetDevNameBySn(QString::fromStdString(chQcApply.deviceSN));
        if ((stuQcApplyItem.strDevName != strPairDevName.first) || (stuQcApplyItem.strDevGrpName != strPairDevName.second))
        {
            continue;
        }

        // 模块号匹配
        if (chQcApply.__isset.moduleIndex)
        {
            if (chQcApply.moduleIndex != stuQcApplyItem.iModuleNo)
            {
                continue;
            }
        }

        // 质控品匹配
        if (chQcApply.qcDocId != stuQcApplyItem.strQcDocID.toLongLong())
        {
            continue;
        }

        // 项目匹配
        // 获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(chQcApply.assayCode);
        if (Q_NULLPTR == spAssayInfo)
        {
            continue;
        }

        // 项目不匹配则跳过
        if (spAssayInfo->assayName != stuQcApplyItem.strAssayName.toStdString())
        {
            continue;
        }

        // 备用瓶需要批号和瓶号匹配
        if (chQcApply.qcType == ch::tf::QcType::QC_TYPE_BACKUP)
        {
            QString strLot = QString::fromStdString(chQcApply.lot);
            QString strSn = QString::fromStdString(chQcApply.sn);
            if (stuQcApplyItem.strRgntLot != strLot || stuQcApplyItem.strRgntNo != strSn)
            {
                continue;
            }
        }

        // 已经找到对应项
        // 选项未改变则忽略
        if (chQcApply.select == stuQcApplyItem.bSelected)
        {
            break;
        }

        // 构造改行选择列被点击
        if (chQcApply.qcType == ch::tf::QcType::QC_TYPE_ONUSE)
        {
            OnQcReqTableItemClicked(pModel->index(iRow, Qah_Selected));

            // 修改质控原因
            stuQcApplyItem.bSelected = chQcApply.select;
            if (stuQcApplyItem.strQcReason != ConvertTfEnumToQString(tf::QcReason::QC_REASON_MANNUAL))
            {
                stuQcApplyItem.strQcReason = chQcApply.__isset.qcReason ? ConvertTfEnumToQString(chQcApply.qcReason) : "";
            }

            pModel->SetApplyInfoByRow(iRow, stuQcApplyItem);
            pModel->UpdateLaout();
        }
        else if (chQcApply.qcType == ch::tf::QcType::QC_TYPE_DEFAULT)
        {
            OnDefaultQcDlgTblItemClicked(pModel->index(iRow, DefaultQcDlg::Dqh_Selected));
        }
        else if (chQcApply.qcType == ch::tf::QcType::QC_TYPE_BACKUP)
        {
            OnBackupRgntQcDlgTblItemClicked(pModel->index(iRow, BackupRgntQcDlg::Bqh_Selected));
        }
        break;
    }
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，新建函数
///
void QcApplyWidget::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 更新菜单位置
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("质控") + QString(" > ") + tr("质控申请"));

    // 如果是第一次显示则初始化, 显示后初始化
    if (!m_bInit)
    {
        // 显示之后初始化
        m_bInit = true;
        InitAfterShow();
    }
    else
    {
        // 通知设备列表更新
        POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevName));

        // 有更新则刷新质控申请表
        if (m_bNeedUpdateQcApplyInfo)
        {
            ReloadQcReqTable();
            m_bNeedUpdateQcApplyInfo = false;
        }

        if (m_pSortHeader->sortIndicatorSection() >= 0)
        {
            // 恢复默认排序
            m_pSortHeader->ResetAllIndex();
            m_pStdModel->setSortRole(UI_ITEM_ROLE_DEFAULT_SORT);
            ui->QcApplyTable->sortByColumn(Qah_DbIndex, Qt::AscendingOrder);
            m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
        }
    }
}

///
/// @brief
///     更新质控申请表
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，新建函数
///
void QcApplyWidget::ReloadQcReqTable()
{
    // 界面不显示则不刷新
    if (!this->isVisible())
    {
        m_bNeedUpdateQcApplyInfo = true;
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取当前选中项ID
    QModelIndex curIndex = ui->QcApplyTable->currentIndex();
    QString strSelID("");
    if (curIndex.isValid())
    {
        // 获取该行信息
        QC_APPLY_INFO stuQcApplyItem;
        if (!m_pStdModel->GetApplyInfoByRow(curIndex.row(), stuQcApplyItem))
        {
            ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
            return;;
        }

        strSelID = stuQcApplyItem.strID;
        m_pStdModel->SetSelQcApplyId(strSelID);
    }

    // 重置选中项
    QItemSelectionModel* pSelModel = ui->QcApplyTable->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        disconnect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(UpdateQcApplySelBtn()));
    }
    ui->QcApplyTable->setCurrentIndex(QModelIndex());
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(UpdateQcApplySelBtn()));
    }

    // 禁能排序
    //ui->QcApplyTable->setSortingEnabled(false);

    // 清空表格内容
    //m_pStdModel->setRowCount(0);
    //m_pStdModel->setRowCount(m_ciQcApplyTblDefaultRowCnt);

    // 获取当前质控品名称
    m_strQryQcName = ui->QcDocCombo->currentText();

    // 清空数据
    m_pStdModel->m_lstDataBuffer.clear();

    // 获取质控申请表信息
    //QList<QC_APPLY_INFO> stuQcApplyInfo;
    if (!gUiAdapterPtr()->WhetherContainOtherSubDev(m_strCurDevName))
    {
        // 没有包含其他子项目
        if (!gUiAdapterPtr(m_strCurDevName)->QueryQcApplyTblInfo("", m_pStdModel->m_lstDataBuffer))
        {
            ULOG(LOG_ERROR, "%s(), QueryQcApplyTblInfo() failed", __FUNCTION__);
            return;
        }
    }
    else
    {
        // 包含其他子项目
        QStringList strSubDevNameList;
        if (!gUiAdapterPtr()->GetSubDevNameList(m_strCurDevName, strSubDevNameList))
        {
            ULOG(LOG_ERROR, "%s(), QueryQcApplyTblInfo() failed", __FUNCTION__);
            return;
        }

        // 遍历所有子设备
        for (const QString& strSubDevName : strSubDevNameList)
        {
            QList<QC_APPLY_INFO> stuSubQcApplyInfo;
            if (!gUiAdapterPtr(strSubDevName, m_strCurDevName)->QueryQcApplyTblInfo("", stuSubQcApplyInfo))
            {
                ULOG(LOG_ERROR, "%s(), QueryQcApplyTblInfo() failed", __FUNCTION__);
                continue;
            }

            m_pStdModel->m_lstDataBuffer.append(stuSubQcApplyInfo);
        }
    }

    // 质控品过滤
    m_pStdModel->SetQcNameFilter(m_strQryQcName);

    // 将数据加载到质控申请表中
//     int iRow = 0;
//     for (const auto& stuQcApplyItem : stuQcApplyInfo)
//     {
//         // 在查询结果中进行质控品名称过滤
//         if (!m_strQryQcName.isEmpty() && !stuQcApplyItem.strQcName.contains(m_strQryQcName))
//         {
//             continue;
//         }
// 
//         // 如果行号大于等于行数，则行数自动增加（始终预留一行空白行）
//         if (iRow >= m_pStdModel->rowCount())
//         {
//             m_pStdModel->setRowCount(iRow + 1);
//         }
// 
//         // 添加数据
//         m_pStdModel->setItem(iRow, Qah_Module, new QStandardItem(stuQcApplyItem.strModuleName));
//         m_pStdModel->setItem(iRow, Qah_AssayName, new QStandardItem(stuQcApplyItem.strAssayName));
//         m_pStdModel->setItem(iRow, Qah_QcNo, new QStandardItem(stuQcApplyItem.strQcNo));
//         m_pStdModel->setItem(iRow, Qah_QcName, new QStandardItem(stuQcApplyItem.strQcName));
//         m_pStdModel->setItem(iRow, Qah_QcBriefName, new QStandardItem(stuQcApplyItem.strQcBriefName));
//         m_pStdModel->setItem(iRow, Qah_QcType, new QStandardItem(stuQcApplyItem.strQcSourceType));
//         m_pStdModel->setItem(iRow, Qah_QcLevel, new QStandardItem(stuQcApplyItem.strQcLevel));
//         m_pStdModel->setItem(iRow, Qah_QcLot, new QStandardItem(stuQcApplyItem.strQcLot));
//         m_pStdModel->setItem(iRow, Qah_QcReason, new QStandardItem(stuQcApplyItem.strQcReason));
//         m_pStdModel->setItem(iRow, Qah_Selected, new QStandardItem(""));
//         m_pStdModel->setItem(iRow, Qah_DbIndex, new QStandardItem(stuQcApplyItem.strID));
// 
//         // 如果选中ID匹配，则选中该行
//         if (!strSelID.isEmpty() && (stuQcApplyItem.strID == strSelID))
//         {
//             ui->QcApplyTable->selectRow(iRow);
//         }
// 
//         // 设置选中图标
//         m_pStdModel->item(iRow, Qah_Selected)->setIcon(stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());
//         m_pStdModel->item(iRow, Qah_Selected)->setData(stuQcApplyItem.bSelected ? ":/Leonis/resource/image/icon-select.png" : "", UI_ITEM_ROLE_ICON_SORT);
// 
//         // 缓存质控申请数据
//         m_pStdModel->item(iRow, Qah_DbIndex)->setData(QVariant::fromValue<QC_APPLY_INFO>(stuQcApplyItem), Qt::UserRole);
// 
//         // 行号自增
//         ++iRow;
//     }
// 
//     // 设置默认排序值
//     SetForDefaultSort(m_pStdModel);
// 
//     // 单元格内容居中显示
//     SetTblTextAlign(m_pStdModel, Qt::AlignCenter);
// 
//     // 使能更新
//     guardTbl.EndGuard();

    m_pStdModel->setRowCount(m_pStdModel->m_lstDataBuffer.size());

    // 更新表格
    m_pStdModel->UpdateLaout();

    // 列宽自适应
    //ResizeTblColToContent(ui->QcApplyTable);

    // 使能排序
    //ui->QcApplyTable->setSortingEnabled(true);

    // 滚动到顶部
    //ui->QcApplyTable->scrollToTop();

    // 如果有排序,则恢复排序
    int iSortCol = m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = m_pSortHeader->GetCurSortOrder();
    if (iSortCol >= 0 && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = (iSortCol == Qah_Selected) ? UI_ITEM_ROLE_ICON_SORT : Qt::DisplayRole;
        m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->QcApplyTable->sortByColumn(iSortCol, qOrderFlag);
    }

    // 更新质控申请选择按钮且没有选中项改变
    UpdateQcApplySelBtnWithoutSelChange();
}

///
/// @brief
///     更新单条质控申请信息
///
/// @param[in]  stuQcApplyInfo  质控申请信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月22日，新建函数
///
void QcApplyWidget::UpdateQcApplyReason(QC_APPLY_INFO stuQcApplyInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 遍历表格，找到对应项
    for (QC_APPLY_INFO& stuOnUseQcApply : m_pStdModel->m_lstDataBuffer)
    {
        // 设备名不匹配则跳过
        if (stuQcApplyInfo.strModuleName != stuOnUseQcApply.strModuleName)
        {
            continue;
        }

        // ID不匹配则跳过
        if (stuQcApplyInfo.strID != stuOnUseQcApply.strID)
        {
            continue;
        }

        // 更新质控原因
        stuOnUseQcApply.strQcReason = stuQcApplyInfo.strQcReason;
        stuOnUseQcApply.strDefApplyID = stuQcApplyInfo.bSelected ? stuQcApplyInfo.strDefApplyID : "";
        break;
    }
}

///
/// @brief
///     更新质控品下拉框
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月27日，新建函数
///
void QcApplyWidget::UpdateQcDocCombo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 屏蔽信号
    ui->QcDocCombo->blockSignals(true);

    // 获取之前的选中项
    QString strSelDocName = ui->QcDocCombo->currentText();

    // 清空质控品下拉框列表
    ui->QcDocCombo->clear();

    // 获取质控品名称列表
    QStringList strQcDocNameList;
    if (!gUiAdapterPtr()->WhetherContainOtherSubDev(m_strCurDevName))
    {
        if (!gUiAdapterPtr(m_strCurDevName)->GetQcDocList(strQcDocNameList))
        {
            ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
            ui->QcDocCombo->blockSignals(false);
            return;
        }
    }
    else
    {
        // 包含其他子项目
        QStringList strSubDevNameList;
        if (!gUiAdapterPtr()->GetSubDevNameList(m_strCurDevName, strSubDevNameList) || strSubDevNameList.isEmpty())
        {
            ULOG(LOG_ERROR, "%s(), GetSubDevNameList() failed", __FUNCTION__);
            ui->QcDocCombo->blockSignals(false);
            return;
        }

        // 以第一个子设备去获取质控文档表
        if (!gUiAdapterPtr(strSubDevNameList.first(), m_strCurDevName)->GetQcDocList(strQcDocNameList))
        {
            ULOG(LOG_ERROR, "%s(), GetQcDocList() failed", __FUNCTION__);
            ui->QcDocCombo->blockSignals(false);
            return;
        }
    }

    // 将质控品文档添加到下拉框中
    ui->QcDocCombo->addItems(UiCommon::QStringListFiltSameItem(strQcDocNameList));
	ui->QcDocCombo->setCurrentIndex(-1);

    // 如果之前的选中项任然存在，则设置当前质控文档
    if (ui->QcDocCombo->findText(strSelDocName) >= 0)
    {
        ui->QcDocCombo->setCurrentText(strSelDocName);
    }

    // 恢复信号
    ui->QcDocCombo->blockSignals(false);
}

///
/// @brief
///     更新设备列表
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月19日，新建函数
///
void QcApplyWidget::UpdateDevNameList()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取设备列表
    if (!gUiAdapterPtr()->GetGroupDevNameList(m_strDevNameList))
    {
        ULOG(LOG_ERROR, "%s(), GetDevNameList() failed", __FUNCTION__);
        return;
    }

    // 如果没有设备，特殊处理
    if (m_strDevNameList.empty())
    {
        ULOG(LOG_INFO, "%s(), No Device", __FUNCTION__);
        return;
    }

    // 如果只有一个设备，特殊处理
    if (m_strDevNameList.size() == 1)
    {
        // 更新当前设备
        m_strCurDevName = m_strDevNameList[0];

        // 通知设备列表更新
        POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevName));
        return;
    }

    // 在最前部插入名称为空的设备代表全部
    m_strDevNameList.insert(m_strDevNameList.begin(), "");

    // 默认当前设备名为空
    m_strCurDevName = "";

    // 通知设备列表更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevName));
}

///
/// @brief
///     更新单条质控申请信息
///
/// @param[in]  reagentInfo  试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月11日，新建函数
///
// void QcApplyWidget::UpdateSingleQcReqInfo(const im::tf::ReagentInfoTable& stuRgntInfo)
// {
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     ReloadQcReqTable();
// }

///
/// @brief
///     质控申请表格被点击
///
/// @param[in]  modIdx 被点击表格信息 
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，代码重构
///
void QcApplyWidget::OnQcReqTableItemClicked(const QModelIndex& modIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 检查参数
    if (!modIdx.isValid() || (modIdx.column() != Qah_Selected))
    {
        // 该参数不可点击修改，无需报警
        ULOG(LOG_INFO, "%s(), not sel or cancel sel", __FUNCTION__);
        return;
    }

    // 获取该行信息
    QC_APPLY_INFO stuQcApplyItem;
    if (!m_pStdModel->GetApplyInfoByRow(modIdx.row(), stuQcApplyItem))
    {
        ULOG(LOG_INFO, "%s(), GetApplyInfoByRow failed", __FUNCTION__);
        return;;
    }

    // 更新质控申请选择状态(// 选择状态反置)
    if (!gUiAdapterPtr(stuQcApplyItem.strDevName, stuQcApplyItem.strDevGrpName, stuQcApplyItem.iModuleNo)->UpdateQcApplyItemSelected(stuQcApplyItem, !stuQcApplyItem.bSelected))
    {
        ULOG(LOG_ERROR, "%s(), UpdateQcApplyItemSelected() failed", __FUNCTION__);
        return;
    }

    // 更新表格信息
    m_pStdModel->SetApplyInfoByRow(modIdx.row(), stuQcApplyItem);
    m_pStdModel->UpdateLaout();

    // 选择按钮状态取反
    bool bExeSel = stuQcApplyItem.bSelected;
    if (bExeSel)
    {
        // 将选择按钮文本重置，默认禁能
        ui->calibrate_sel_btn->setText(tr("取消选择"));
        ui->calibrate_sel_btn->setEnabled(true);
        ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValCancelSel);
    }
    else
    {
        // 将选择按钮文本重置，默认禁能
        ui->calibrate_sel_btn->setText(tr("选择"));
        ui->calibrate_sel_btn->setEnabled(true);
        ui->calibrate_sel_btn->setProperty(m_cstrSelBtnStatusName, m_cstrSelBtnStatusValSel);
    }

    // 如果按选择列排序,则恢复排序
    int iSortCol = m_pSortHeader->sortIndicatorSection();
    SortHeaderView::SortOrder order = m_pSortHeader->GetCurSortOrder();
    if (iSortCol == Qah_Selected && order != SortHeaderView::NoOrder)
    {
        // 恢复排序
        int iSortRole = UI_ITEM_ROLE_ICON_SORT;
        m_pStdModel->setSortRole(iSortRole);
        Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        ui->QcApplyTable->sortByColumn(iSortCol, qOrderFlag);
    }
}

///
/// @brief
///     质控品下拉框当前选项改变
///
/// @param[in]  iIndex  质控品下拉框当前选中索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月27日，新建函数
///
void QcApplyWidget::OnQcDocComboIndexChanged(int iIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    Q_UNUSED(iIndex);
    // 文本不为空则忽略
    if (!ui->QcDocCombo->currentText().isEmpty())
    {
        return;
    }

    // 更新质控申请表
    ReloadQcReqTable();

    // 滚动到顶部
    ui->QcApplyTable->scrollToTop();

    // 通知设备列表更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevName));
}

///
/// @brief
///     质控品重置
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
void QcApplyWidget::OnQcDocComboReset()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新质控申请表
    ReloadQcReqTable();

    // 滚动到顶部
    ui->QcApplyTable->scrollToTop();

    // 通知设备列表更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevName));
}

///
/// @brief
///     查询按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月28日，新建函数
///
void QcApplyWidget::OnQueryBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新质控申请表
    ReloadQcReqTable();

    // 通知设备列表更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, m_strDevNameList, m_strDevNameList.indexOf(m_strCurDevName));
}

///
/// @brief
///     重置按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月28日，新建函数
///
void QcApplyWidget::OnResetBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 重置下拉框内容为空
    int iIdx = ui->QcDocCombo->findText("");
    if (iIdx >= 0)
    {
        ui->QcDocCombo->setCurrentIndex(iIdx);
        ReloadQcReqTable();
    }
}

///
/// @brief
///     默认质控按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnDefaultQcBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_pDefaultQcDlg == Q_NULLPTR)
    {
        // 初始化对话框
        m_pDefaultQcDlg = new DefaultQcDlg(m_strDevNameList, this);
        LoadDlg(m_pDefaultQcDlg);
        InitConnect(m_pDefaultQcDlg);
    }
    else
    {
        // 刷新对话框
        UpdateDlgQcDocCombo(m_pDefaultQcDlg);
        UpdateDlg(m_pDefaultQcDlg);
    }

    // 弹出对话框
    m_pDefaultQcDlg->exec();
}

///
/// @brief
///     备用瓶质控按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月16日，新建函数
///
void QcApplyWidget::OnBackupRgntQcBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_pBackupRgntQcDlg == Q_NULLPTR)
    {
        // 获取质控品名称列表
        QStringList strDevNameList;
        if (!gUiAdapterPtr()->GetGroupDevNameListButIse(strDevNameList))
        {
            ULOG(LOG_ERROR, "%s(), GetBkQcDevList() failed", __FUNCTION__);
            return;
        }

        // 初始化对话框
        strDevNameList.push_front("");

        // 初始化对话框
        m_pBackupRgntQcDlg = new BackupRgntQcDlg(strDevNameList, this);
        LoadDlg(m_pBackupRgntQcDlg);
        InitConnect(m_pBackupRgntQcDlg);
    }
    else
    {
        // 刷新对话框
        UpdateDlgQcDocCombo(m_pBackupRgntQcDlg);
        UpdateDlg(m_pBackupRgntQcDlg);
    }

    // 弹出对话框
    m_pBackupRgntQcDlg->exec();
}

///
/// @brief
///     质控申请信息更新
///
/// @param[in]  qcAplInfo  质控申请信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月9日，新建函数
///
// void QcApplyWidget::UpdateQcApplyInfo(im::tf::QcApply qcAplInfo)
// {
//     ULOG(LOG_INFO, "%s()", __FUNCTION__);
//     Q_UNUSED(qcAplInfo);
//     ReloadQcReqTable();
// }
