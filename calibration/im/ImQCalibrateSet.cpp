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
/// @file     ImQCalibrateSet.h
/// @brief    校准设置界面
///
/// @author   1226/ZhangJing
/// @date     2022年5月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2022年5月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ImQCalibrateSet.h"
#include "ui_ImQCalibrateSet.h"

#include <QStandardItemModel>
#include <QFileDialog>
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/QPostionEdit.h"
#include "shared/ReagentCommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/FileExporter.h"
#include "thrift/im/ImLogicControlProxy.h"

#include "SortHeaderView.h"
#include "ImCaliBrateRecordDialog.h"
#include "src/common/defs.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "manager/OperationLogManager.h"
#include "manager/UserInfoManager.h"
#include "Serialize.h"
#include "printcom.hpp"


ImQCalibrateSet::ImQCalibrateSet(QWidget *parent)
    : QWidget(parent),
    m_SelectModel(nullptr),
    m_CaliBrateGroup(nullptr),
	m_CaliPostionEditDialog(nullptr),
	m_bQuery(false),
	m_queryName(""),
    m_caliRecordDlg(nullptr)
{
    ui = new Ui::ImQCalibrateSet();
    ui->setupUi(this);
    // 设置失效日期代理（按设置日期格式显示）
    ui->CaliGroup_View->setItemDelegateForColumn(3, new CReadOnlyDelegate(this));

    ResetCaliBrateGroup();
    // 删除校准组
    connect(ui->del_Button, SIGNAL(clicked()), this, SLOT(OnDeleteCaliBrateGroup()));
    // 校准设置表中单元项被点击
    connect(ui->CaliGroup_View, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnCalSetTblItemClicked(const QModelIndex&)));
	// 下拉菜单切换
	connect(ui->CaliNameCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCaliComboxChange(int)));
    // 增加下拉菜单点×消息的响应筛选函数bug24985
    connect(ui->CaliNameCombox, &QFilterComboBox::ResetText, this, &ImQCalibrateSet::OnQueryBtnClicked);
	// 筛选按钮点击
	connect(ui->pushButton_search, SIGNAL(clicked()), this, SLOT(OnQueryBtnClicked()));
    
	// 点扫描按钮
	connect(ui->flat_m_regist, &QPushButton::clicked, this, [this] {
        if (m_caliRecordDlg == nullptr)
        {
            m_caliRecordDlg = new ImCaliBrateRecordDialog(this);
            connect(m_caliRecordDlg, SIGNAL(FinishCaliInfo(const im::tf::CaliDocGroup&)), this, SLOT(OnScanCalibrateGroup(const im::tf::CaliDocGroup&)));
        }
        // BUG21325
        m_caliRecordDlg->setInputMode(InputType::Input_Scan);
        m_caliRecordDlg->show();
	});

	connect(ui->CaliGroup_View->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
		// 设置 修改和删除按钮 是否禁用
		ui->del_Button->setEnabled(true);
		auto sels = selected.indexes();
		if (sels.size() < 1)
		{
			ui->del_Button->setEnabled(false);
			ui->CaliGroup_View->clearSelection();
		}
	});

    // 打印按钮被点击
    connect(ui->PrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));
    // 导出按钮被点击
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));
    // 监听校准组合变化信息
	REGISTER_HANDLER(MSG_ID_IM_CALI_GRP_UPDATE, this, UpdateSingleCaliGrpInfo);
	// 注册设备状态改变处理槽函数
	REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStateChange);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	UpdateCaliCombox();
    RefreshPage();
}

ImQCalibrateSet::~ImQCalibrateSet()
{
}

///
/// @brief
///     初始化校准品组
///
/// @par History:
/// @li 1226/zhangjing，2022年5月10日，新建函数
///
void ImQCalibrateSet::ResetCaliBrateGroup()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_CaliBrateGroup == nullptr)
    {
        m_CaliBrateGroup = new QStandardItemModel(this);
		m_ImCaliBrateGroupProxyModel = new QImCalibrateSetFilterDataModule(ui->CaliGroup_View);
		m_ImCaliBrateGroupProxyModel->setSourceModel(m_CaliBrateGroup);
		// 升序 降序 原序
		SortHeaderView *pImCaliBrateGroupHeader = new SortHeaderView(Qt::Horizontal, ui->CaliGroup_View);
		pImCaliBrateGroupHeader->setStretchLastSection(true);
		ui->CaliGroup_View->setHorizontalHeader(pImCaliBrateGroupHeader);
		connect(pImCaliBrateGroupHeader, &SortHeaderView::SortOrderChanged, this, [this, pImCaliBrateGroupHeader](int logicIndex, SortHeaderView::SortOrder order) {
			QTableView *view = ui->CaliGroup_View;
			//无效index或NoOrder就设置为默认未排序状态
			if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
                // 第一列按位置升序排列
                m_CaliBrateGroup->setSortRole(Qt::UserRole + 1);
                ui->CaliGroup_View->sortByColumn(0, Qt::AscendingOrder);
                //去掉排序三角样式
                ui->CaliGroup_View->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
                //-1则还原model默认顺序
                m_ImCaliBrateGroupProxyModel->sort(-1, Qt::DescendingOrder);
			}
			else
			{
                int iSortRole = (logicIndex == 0) ? (Qt::UserRole + 1) : Qt::DisplayRole;
                m_CaliBrateGroup->setSortRole(iSortRole);
                Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                view->sortByColumn(logicIndex, qOrderFlag);
			}
		});
		//去掉排序三角样式
		ui->CaliGroup_View->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
		//-1则还原model默认顺序
		m_ImCaliBrateGroupProxyModel->sort(-1, Qt::DescendingOrder);
        //ui->CaliGroup_View->setModel(m_CaliBrateGroup);
        ui->CaliGroup_View->setModel(m_ImCaliBrateGroupProxyModel);

        // 多选模式
        m_SelectModel = new QItemSelectionModel(m_ImCaliBrateGroupProxyModel);
        ui->CaliGroup_View->setSelectionModel(m_SelectModel);
        ui->CaliGroup_View->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ui->CaliGroup_View->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->CaliGroup_View->setEditTriggers(QAbstractItemView::NoEditTriggers);

		ui->CaliGroup_View->setSortingEnabled(true);
        // 选择每一行的槽函数
        connect(ui->CaliGroup_View, SIGNAL(clicked(QModelIndex)), this, SLOT(OnSelectGroup(QModelIndex)));

        // 设置校准品代理
        ui->CaliGroup_View->setItemDelegateForColumn(4, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(5, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(6, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(7, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(8, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(9, new QPositionDelegate(this));

        m_CaliBrateGroup->clear();
        QStringList horheadList;
        horheadList << tr("序号") << tr("校准品名称") << tr("校准品批号") << tr("失效日期") << tr("校准品1") << tr("校准品2")
            << tr("校准品3") << tr("校准品4") << tr("校准品5") << tr("校准品6");
        m_CaliBrateGroup->setHorizontalHeaderLabels(horheadList);
        for (int s = 4; s < horheadList.size(); ++s)
            m_CaliBrateGroup->setHeaderData(s, Qt::Horizontal, UI_TEXT_COLOR_HEADER_MARK, Qt::ForegroundRole);
    }

    m_CaliBrateGroup->setRowCount(0);
    ResizeTblColToContent(ui->CaliGroup_View);
}


///
/// @brief  刷新界面
///     
///
/// @par History:
/// @li 1226/zhangjing，2022年5月10日，新建函数
///
void ImQCalibrateSet::RefreshPage()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);	
    // 清除旧数据
    m_CaliGroupSave.clear();
    ResetCaliBrateGroup();
    if (m_CaliBrateGroup == nullptr)
    {
        return;
    }

    // 查询校准组合信息
    ::im::tf::CaliDocGroupQueryResp ResultDocGroup;
    ::im::tf::CaliDocGroupQueryCond queryDocGroupCondition;
    if (!im::LogicControlProxy::QueryCaliDocGroup(ResultDocGroup, queryDocGroupCondition)
        || ResultDocGroup.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || ResultDocGroup.lstCaliDocGroups.size() <= 0)
    {
        return;
    }

	// 获取操作行
	int iOpRow = ResultDocGroup.lstCaliDocGroups.size();
	if (iOpRow >= m_CaliBrateGroup->rowCount())
	{
		m_CaliBrateGroup->setRowCount(iOpRow);
	}

    int row = 0;
    // 依次显示校准品组的信息
    for (const auto& docGroup : ResultDocGroup.lstCaliDocGroups)
    {
        // 如果水平信息为空，则下一个
        if (docGroup.caliDocs.size() <= 0)
        {
            continue;
        }
        // bug26007 无视大小写
        if (!m_queryName.isEmpty() && !QString::fromStdString(docGroup.name).contains(m_queryName, Qt::CaseInsensitive))
        {
            continue;
        }
        AddCaliGrp(row, docGroup);
        m_CaliGroupSave[row]= docGroup;       
        row++;
    }
    // 纠正行数
    if (row != iOpRow)
    {
        m_CaliBrateGroup->setRowCount(row);
    }

	// 设置 修改和删除按钮 是否禁用
	QModelIndex index = ui->CaliGroup_View->currentIndex();
	ui->del_Button->setEnabled(true);
	if (m_CaliGroupSave.size() == 0 || index.row() < 0 || index.row() >= m_CaliGroupSave.size())
	{
		ui->del_Button->setEnabled(false);
	}
}


///
/// @brief
///     点击校准组名称
///
/// @param[in]  index  索引
///
/// @par History:
/// @li 1226/zhangjing，2022年5月10日，新建函数
///
void ImQCalibrateSet::OnSelectGroup(QModelIndex index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_CaliBrateGroup == nullptr)
    {
        return;
    }
    QStandardItem* seqItem = m_CaliBrateGroup->item(index.row(), 0);
    if (seqItem == nullptr)
    {
        return;
    }
    if (m_CaliGroupSave.find(index.row()) == m_CaliGroupSave.end())
    {
        return;
    }
    
    // 待处理
}

///
/// @brief
///     校准扫描
///
/// @par History:
/// @li 1226/zhangjing，2022年5月10日，新建函数
///
void ImQCalibrateSet::OnScanCalibrateGroup(const im::tf::CaliDocGroup& grp)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	UpdateCaliCombox();
    int row = m_CaliBrateGroup->rowCount();
    m_CaliBrateGroup->setRowCount(row+1);
    AddCaliGrp(row, grp);
    m_CaliGroupSave[row] = grp;
}

///
/// @brief
///     更新校准数据
///
/// @par History:
/// @li 1226/zhangjing，2022年5月10日，新建函数
///
void ImQCalibrateSet::OnUpdateCaliBrateData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    RefreshPage();
}

///
/// @brief
///     删除校准品组
///
/// @par History:
/// @li 1226/zhangjing，2022年5月10日，新建函数
///
void ImQCalibrateSet::OnDeleteCaliBrateGroup()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (ui->del_Button->property("devIsRun").toBool())
	{
		TipDlg(tr("提示"), tr("项目测试中不可删除校准品。")).exec();
		return;
	}

    // 删除之前先弹出一个对话框，然后进行选择
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除"), tr("确定删除选中数据？"), TipDlgType::TWO_BUTTON));
    if (QDialog::Rejected == pTipDlg->exec())
    {
        return;
    }

    if (!m_SelectModel)
    {
        return;
    }

    QModelIndexList curIndexList = m_SelectModel->selectedRows();
    for (const QModelIndex& index: curIndexList)
    {
        QModelIndex SourceIndex = m_ImCaliBrateGroupProxyModel->mapToSource(index);
        int iRow = SourceIndex.row();
        // 获取对应校准组合信息
        if (m_CaliGroupSave.find(iRow) == m_CaliGroupSave.end())
        {
            continue;
        }
        im::tf::CaliDocGroup pDeletGrp = m_CaliGroupSave[iRow];
        // 删除对应的校准品组
        im::tf::CaliDocGroupQueryCond qrc;
        qrc.__set_id(pDeletGrp.id);
        if (!im::LogicControlProxy::DeleteCaliDocGroup(qrc))
        {
            ULOG(LOG_ERROR, "failed delete calidoc group.");
            return;
        }
		// 删除校准品记录日志
        COperationLogManager::GetInstance()->AddCaliOperationLog(::tf::OperationType::type::DEL, pDeletGrp);
        // 更新校准概况界面-bug26004
        POST_MESSAGE(MSG_ID_IM_CALI_SET_UPDATE, tf::UpdateType::UPDATE_TYPE_DELETE, pDeletGrp);
    }
    ui->CaliGroup_View->update();
	UpdateCaliCombox();
    RefreshPage();

    // 设置 修改和删除按钮 是否禁用
    QModelIndex index = ui->CaliGroup_View->currentIndex();
    ui->del_Button->setEnabled(true);
    if (m_CaliGroupSave.size() == 0 || index.row() < 0 || index.row() >= m_CaliGroupSave.size())
    {
        ui->del_Button->setEnabled(false);
    }
}

///
/// @brief
///     校准设置表中单元项被点击
///
/// @param[in]  modIdx  被点击的单元项索引
///
/// @par History:
/// @li 1226/zhangjing，2022年5月13日，新建函数
///
void ImQCalibrateSet::OnCalSetTblItemClicked(const QModelIndex& modIdx)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    int iFirstPos = 4;

    // 检查参数
	if (!modIdx.isValid() || modIdx.column() < iFirstPos)
	{
		return;
	}
    QModelIndex SourceIndex = m_ImCaliBrateGroupProxyModel->mapToSource(modIdx);
    m_iClickSqe = SourceIndex.row();
    auto& Iter = m_CaliGroupSave.find(m_iClickSqe);
    if (Iter == m_CaliGroupSave.end())
    {
        return;
    }
    auto& pGrp = Iter->second;

    // 空白点击无效，直接返回
    if (modIdx.column() >= iFirstPos + pGrp.levelCnt)
    {
        return;
    }

	if (im::LogicControlProxy::IsCaliGrpCaling(pGrp, true))
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("该校准品正在校准，不能进行位置修改！")));
		pTipDlg->exec();
		return;
	}

	// 设置校准doc的架号位置号
	QList<im::tf::CaliDoc> lsCaliDos;
	for (const auto& doc : pGrp.caliDocs)
	{
		lsCaliDos.append(doc);
	}

	if (m_CaliPostionEditDialog == nullptr)
	{
		m_CaliPostionEditDialog = new QPostionEdit(this);
		connect(m_CaliPostionEditDialog, SIGNAL(ModifyIMCaliPostion()), this, SLOT(OnRefreshPostion()));
	}
	if (m_CaliPostionEditDialog->SetIMCaliDoc(lsCaliDos, modIdx.column() - iFirstPos + 1))
	{
        m_CaliPostionEditDialog->show();
	}	
}

///
/// @brief 更新下拉菜单 
///
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2022年12月12日，新建函数
///
void ImQCalibrateSet::UpdateCaliCombox()
{
	// 查询校准品名称
	im::tf::CaliDocGroupQueryResp _return;
	::im::tf::CaliDocGroupQueryCond queryDocGroupCondition;
	if (!im::LogicControlProxy::QueryCaliDocGroup(_return, queryDocGroupCondition)
		|| _return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| _return.lstCaliDocGroups.size() <= 0)
	{
		return;
	}

	QString strText = ui->CaliNameCombox->currentText();

	// 设置校准品下拉列表值
	ui->CaliNameCombox->clear();
	bool bExist = false;
	for (const auto& caliGrp : _return.lstCaliDocGroups)
	{
		QString strName = QString::fromStdString(caliGrp.name);
		// 未查询到进行添加
		if (-1 == ui->CaliNameCombox->findText(strName))
		{
			ui->CaliNameCombox->addItem(strName);
		}	
	}

	ui->CaliNameCombox->setCurrentText(strText);	
}

///
/// @bref
///		校准品下拉框事件
///
/// @param[in] sel 选择的sel
///
/// @par History:
/// @li 1226/zhangjing，2022年12月12日，新建函数
///
void ImQCalibrateSet::OnCaliComboxChange(int sel)
{
	if (sel < 0)
	{
		ULOG(LOG_WARN, "Invalid QcCombox sel.");
		return;
	}
}

///
/// @brief  查询按钮按下
///
/// @par History:
/// @li 1226/zhangjing，2022年12月12日，新建函数
///
void ImQCalibrateSet::OnQueryBtnClicked()
{
    auto&& projectName = ui->CaliNameCombox->currentText();

    // 如果在空的时候进行查找，直接返回
    if (projectName != m_queryName)
    {
        m_queryName = projectName;
        RefreshPage();
    }	
}


///
/// @brief
///     刷新校准修改位置
////
/// @par History:
/// @li 1226/zhangjing，2022年12月13日，新建函数
///
void ImQCalibrateSet::OnRefreshPostion()
{
	if (m_CaliPostionEditDialog == nullptr)
	{
		return;
	}
	auto docs = m_CaliPostionEditDialog->GetIMCaliDoc();
    if (docs && m_iClickSqe >= 0 && m_iClickSqe < m_CaliBrateGroup->rowCount())
    {
        im::tf::CaliDocGroup pGrp = m_CaliGroupSave[m_iClickSqe];
		std::vector<im::tf::CaliDoc> vtDoc;
		for (const auto& tmp : docs.value())
		{
			vtDoc.push_back(tmp);
		}
		pGrp.__set_caliDocs(vtDoc);
		if (!im::LogicControlProxy::ModifyCaliDocGroup(pGrp))
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("校准品位置保存失败！")));
			pTipDlg->exec();
			return ;
		}

        // 校准doc位置显示 bug17981
        m_CaliGroupSave[m_iClickSqe] = pGrp;
        int column = 4;
        for (const auto& caliDoc : pGrp.caliDocs)
        {
            if (caliDoc.__isset.rack && !caliDoc.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(caliDoc.rack) + "-" + QString::number(caliDoc.pos);
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), rack_pos, Qt::DisplayRole);
				if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_MICRO)
					m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), QPixmap(QString(":/Leonis/resource/image/icon-micro-cup-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
				else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_NORMAL)
					m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), QPixmap(QString(":/Leonis/resource/image/icon-normal-cup-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
				else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_ORIGIN)
					m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), QPixmap(QString(":/Leonis/resource/image/icon-ori-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
				else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_STORE)
					m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), QPixmap(QString(":/Leonis/resource/image/icon-store-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
                else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_ORIGIN)
                    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), QPixmap(QString(":/Leonis/resource/image/icon-ori-hv.png")), Qt::UserRole + 1);
                else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_STORE)
                    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), QPixmap(QString(":/Leonis/resource/image/icon-store-hv.png")), Qt::UserRole + 1);

            }
            else
            {
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), QVariant(), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(m_iClickSqe, column), tr("请设置位置"), Qt::DisplayRole);                
            }
            column++;
        }

        POST_MESSAGE(MSG_ID_IM_CALI_SET_UPDATE, tf::UpdateType::UPDATE_TYPE_MODIFY, pGrp);
	}

}

///
/// @brief  校准组发生更新   
///
/// @param[in]  cdg   校准组信息
/// @param[in]  type  更新类型
///
/// @par History:
/// @li 1226/zhangjing，2023年2月23日，新建函数
///
void ImQCalibrateSet::UpdateSingleCaliGrpInfo(const im::tf::CaliDocGroup& cdg, const im::tf::ChangeType::type type)
{
	if (type == im::tf::ChangeType::Delete || type == im::tf::ChangeType::Add)
	{
		UpdateCaliCombox();
	}
	// 如果更新的校准组信息不在显示范围内则不更新列表。(id=-代表不是固定某校准品更新)
	if (m_bQuery && cdg.name != m_queryName.toStdString() && cdg.id != -1)
	{
		return;
	}
    // 新增不需要更新表所有内容，只需要新增一条
    if (type == im::tf::ChangeType::Add && cdg.id != -1)
    {
        int row = m_CaliBrateGroup->rowCount();
        m_CaliBrateGroup->setRowCount(row+1);
        AddCaliGrp(row, cdg);
        m_CaliGroupSave[row] = cdg;

		// 添加操作日志
		COperationLogManager::GetInstance()->AddCaliOperationLog(::tf::OperationType::type::ADD, cdg);

        return;
    }
	RefreshPage();
}

///////////////////////////////////////////////////////////////////////////
/// @file     ImQCalibrateSet.h
/// @brief 	 排序代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
QImCalibrateSetFilterDataModule::QImCalibrateSetFilterDataModule(QObject * parent)
	: QSortFilterProxyModel(parent)
{
}

QImCalibrateSetFilterDataModule::~QImCalibrateSetFilterDataModule()
{
}

bool QImCalibrateSetFilterDataModule::IsNumber(const QVariant & data, double & value) const
{
    bool ok = false;
    value = data.toDouble(&ok);
    return ok;
}

bool QImCalibrateSetFilterDataModule::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
	// 获取源model
	QStandardItemModel* pSouceModel = dynamic_cast<QStandardItemModel*>(sourceModel());
	if (pSouceModel == nullptr)
	{
		return false;
	}

    // 如果是升序或者降序
    QVariant leftData = pSouceModel->data(source_left);
    QVariant rightData = pSouceModel->data(source_right);

    double leftValue = 0;
    double rightValue = 0;
    if (IsNumber(leftData, leftValue) && IsNumber(rightData, rightValue))
    {
        return leftValue < rightValue;
    }
    return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
}


///
/// @brief  设备状态变化
///
/// @param[in]  deviceInfo  设备信息
///
/// @par History:
/// @li 1226/zhangjing，2023年4月13日，新建函数
///
void ImQCalibrateSet::OnDevStateChange(tf::DeviceInfo deviceInfo)
{
	ULOG(LOG_INFO, "%s : devSn:%s", __FUNCTION__, deviceInfo.deviceSN.c_str());

	if (deviceInfo.deviceType == ::tf::DeviceType::DEVICE_TYPE_I6000)
	{
		// 项目测试中禁用保存按钮
		ui->del_Button->setProperty("devIsRun", (deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING || 
			deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP));
	}
}

///
/// @brief  重新函数清空排序-bug23758
///
/// @param[in]  event  
///
/// @par History:
/// @li 1226/zhangjing，2023年12月13日，新建函数
///
void ImQCalibrateSet::hideEvent(QHideEvent * event)
{
    // 第一列按位置升序排列
    m_CaliBrateGroup->setSortRole(Qt::UserRole + 1);
    ui->CaliGroup_View->sortByColumn(0, Qt::AscendingOrder);
    //去掉排序三角样式
    ui->CaliGroup_View->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    //-1则还原model默认顺序
    m_ImCaliBrateGroupProxyModel->sort(-1, Qt::DescendingOrder);

    QWidget::hideEvent(event);
}
void ImQCalibrateSet::showEvent(QShowEvent * event)
{
    // 基类处理
    QWidget::showEvent(event);
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("校准") + QString(" > ") + tr("校准设置"));
}
///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年12月14日，新建函数
///
void ImQCalibrateSet::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    CaliRecordInfo Info;
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    GetPrintExportInfo(Info);
    if (Info.vecRecord.empty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可打印的数据!")));
        pTipDlg->exec();
        return;
    }

    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机!")));
    pTipDlg->exec();
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年12月14日，新建函数
///
void ImQCalibrateSet::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    CaliRecordInfo Info;
    GetPrintExportInfo(Info);
    if (Info.vecRecord.empty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可导出的数据!")));
        pTipDlg->exec();
        return;
    }

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        //ALOGI << "Export canceled!";
        return;
    }

    bool bRect = false;
    QFileInfo FileInfo(strFilepath);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliRecordInfo.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportCaliRecordInfo(Info, strFilepath);
    }

    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}


void ImQCalibrateSet::GetPrintExportInfo(CaliRecordInfo& info)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QModelIndex index = ui->CaliGroup_View->indexAt(QPoint(0,0));
    int iRow = m_CaliBrateGroup->rowCount();
    for (int i = 0; i < iRow; i++)
    {
        
        QModelIndex ViewIndex = m_ImCaliBrateGroupProxyModel->index(i, 0);
        QModelIndex SourceIndex = m_ImCaliBrateGroupProxyModel->mapToSource(ViewIndex);
        auto& Iter =  m_CaliGroupSave.find(SourceIndex.row());
        if (Iter == m_CaliGroupSave.end())
        {
            continue;
        }

        auto& docGroup = Iter->second;

        // 如果水平信息为空，则下一个
        if (docGroup.caliDocs.size() <= 0)
        {
            continue;
        }
        if (!m_queryName.isEmpty() && !QString::fromStdString(docGroup.name).contains(m_queryName))
        {
            continue;
        }
        CaliRecord Record;
        Record.strIndex = ViewIndex.data(Qt::DisplayRole).value<QString>().toStdString();
        Record.strCaliName = docGroup.name;
        Record.strCaliLot = docGroup.lot;
        boost::posix_time::ptime posixTime = TimeStringToPosixTime(docGroup.expiryTime);
        Record.strExpirationDate = ConverPosixTimeToDateString(posixTime);
        if (docGroup.caliDocs.size() > 0)
        {
            const auto& caliDoc0 = docGroup.caliDocs[0];
            if (caliDoc0.__isset.rack && !caliDoc0.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(caliDoc0.rack) + "-" + QString::number(caliDoc0.pos);
                if(caliDoc0.tubeType != -1)
                    rack_pos += QString(" ") + ConvertTfEnumToQString(caliDoc0.tubeType);

                Record.strCalibrator1 = rack_pos.toStdString();
            }
            else
            {   // 异常时显示 * 
                Record.strCalibrator1 = "*";
            }
        }

        if (docGroup.caliDocs.size() > 1)
        {
            const auto& caliDoc1 = docGroup.caliDocs[1];
            if (caliDoc1.__isset.rack && !caliDoc1.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(caliDoc1.rack) + "-" + QString::number(caliDoc1.pos);
                if (caliDoc1.tubeType != -1)
                    rack_pos += QString(" ") + ConvertTfEnumToQString(caliDoc1.tubeType);

                Record.strCalibrator2 = rack_pos.toStdString();
            }
            else
            {
                Record.strCalibrator2 = "*";
            }
        }

        if (docGroup.caliDocs.size() > 2)
        {
            const auto& caliDoc2 = docGroup.caliDocs[2];
            if (caliDoc2.__isset.rack && !caliDoc2.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(caliDoc2.rack) + "-" + QString::number(caliDoc2.pos);
                if (caliDoc2.tubeType != -1)
                    rack_pos += QString(" ") + ConvertTfEnumToQString(caliDoc2.tubeType);

                Record.strCalibrator3 = rack_pos.toStdString();
            }
            else
            {
                Record.strCalibrator3 = "*";
            }
        }

        if (docGroup.caliDocs.size() > 3)
        {
            const auto& caliDoc3 = docGroup.caliDocs[3];
            if (caliDoc3.__isset.rack && !caliDoc3.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(caliDoc3.rack) + "-" + QString::number(caliDoc3.pos);
                if (caliDoc3.tubeType != -1)
                    rack_pos += QString(" ") + ConvertTfEnumToQString(caliDoc3.tubeType);

                Record.strCalibrator4 = rack_pos.toStdString();
            }
            else
            {
                Record.strCalibrator4 = "*";
            }
        }

        if (docGroup.caliDocs.size() > 4)
        {
            const auto& caliDoc4 = docGroup.caliDocs[4];
            if (caliDoc4.__isset.rack && !caliDoc4.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(caliDoc4.rack) + "-" + QString::number(caliDoc4.pos);
                if (caliDoc4.tubeType != -1)
                    rack_pos += QString(" ") + ConvertTfEnumToQString(caliDoc4.tubeType);

                Record.strCalibrator5 = rack_pos.toStdString();
            }
            else
            {
                Record.strCalibrator5 = "*";
            }
        }

        if (docGroup.caliDocs.size() > 5)
        {
            const auto& caliDoc5 = docGroup.caliDocs[5];
            if (caliDoc5.__isset.rack && !caliDoc5.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(caliDoc5.rack) + "-" + QString::number(caliDoc5.pos);
                if (caliDoc5.tubeType != -1)
                    rack_pos += QString(" ") + ConvertTfEnumToQString(caliDoc5.tubeType);

                Record.strCalibrator6 = rack_pos.toStdString();
            }
            else
            {
                Record.strCalibrator6 = "*";
            }
        }

        info.vecRecord.push_back(Record);
    }
}


void ImQCalibrateSet::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    if (userPms->IsPermissionShow(PSM_IM_PRINTEXPORT_CALIDOC))
    {
        ui->export_btn->show();
    }
    else
    {
        ui->export_btn->hide();
    }
}

///
/// @brief  添加了新的校准品
///
/// @param[in]  iRow  添加的行
/// @param[in]  grp  添加的校准组
///
/// @par History:
/// @li 1226/zhangjing，2024年1月8日，新建函数
///
void ImQCalibrateSet::AddCaliGrp(const int row, const im::tf::CaliDocGroup& docGroup)
{
    int column = 0;
    QString seq = QString::number(row + 1);
    m_CaliBrateGroup->setItem(row, column, CenterAligmentItem(seq));
    // 设置排序
    m_CaliBrateGroup->item(row, column++)->setData(row + 1, Qt::UserRole + 1);
    // 校准组名称
    m_CaliBrateGroup->setItem(row, column++, CenterAligmentItem(QString::fromStdString(docGroup.name)));
    m_CaliBrateGroup->setItem(row, column++, CenterAligmentItem(QString::fromStdString(docGroup.lot)));
    boost::posix_time::ptime posixTime = TimeStringToPosixTime(docGroup.expiryTime);
    QString strExpiryDate = QString::fromStdString(ConverPosixTimeToDateString(posixTime));
    QStandardItem* itemExprityDate = CenterAligmentItem(strExpiryDate);
    QDate expDate = QDate::fromString(strExpiryDate, UI_DATE_FORMAT);
    if (expDate.isValid() && (expDate <= QDate::currentDate()))
    {
        //SetItemColor(itemExprityDate, UI_REAGENT_WARNCOLOR);
        itemExprityDate->setData(QColor(UI_REAGENT_WARNFONT), Qt::ForegroundRole);
    }
    m_CaliBrateGroup->setItem(row, column++, itemExprityDate);
    // 校准doc位置显示
    for (const auto& caliDoc : docGroup.caliDocs)
    {
        if (caliDoc.__isset.rack && !caliDoc.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
        {
            QString rack_pos = QString::fromStdString(caliDoc.rack) + "-" + QString::number(caliDoc.pos);
            m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), rack_pos, Qt::DisplayRole);
            if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_MICRO)
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QPixmap(QString(":/Leonis/resource/image/icon-micro-cup-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_NORMAL)
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QPixmap(QString(":/Leonis/resource/image/icon-normal-cup-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_ORIGIN)
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QPixmap(QString(":/Leonis/resource/image/icon-ori-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_STORE)
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QPixmap(QString(":/Leonis/resource/image/icon-store-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_ORIGIN)
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QPixmap(QString(":/Leonis/resource/image/icon-ori-hv.png")), Qt::UserRole + 1);
            else if (caliDoc.tubeType == tf::TubeType::TUBE_TYPE_STORE)
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QPixmap(QString(":/Leonis/resource/image/icon-store-hv.png")), Qt::UserRole + 1);

        }
        else
        {
            m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QVariant(), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), tr("请设置位置"), Qt::DisplayRole);
        }
        column++;
    }
}
