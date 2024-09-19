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
/// @file     QSampleDataBrowse.h
/// @brief    数据浏览页面
///
/// @author   5774/WuHongTaoassay_list

/// @date     2022年5月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSampleDataBrowse.h"
#include "ui_QSampleDataBrowse.h"
#include <QButtonGroup>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QComboBox>
#include <QScrollBar>
#include <QToolTip>
#include <QItemDelegate>
#include <QCheckBox>
#include <QProgressBar>
#include <QScroller>

#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/QComDelegate.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/FileExporter.h"
#include "shared/DataManagerQc.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

#include "QSampleAssayModel.h"
#include "QPushSampleToRemote.h"
#include "QPatientDialog.h"
#include "QDialogAi.h"
#include "QReCheckDialog.h"
#include "itemresultdetaildlg.h"
#include "QAssayResultDetail.h"
#include "QReCalcDialog.h"
#include "SortHeaderView.h"
#include "printsetdlg.h"
#include "WorkpageCommon.h"
#include "src/public/ch/ChCommon.h"
#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "Serialize.h"
#include "printcom.hpp"

Q_DECLARE_METATYPE(im::tf::AssayTestResult)
Q_DECLARE_METATYPE(ch::tf::AssayTestResult)
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return;\
}

QSampleDataBrowse::QSampleDataBrowse(QWidget *parent)
    : QWorkShellPage(parent),
    m_sampleDetailDialog(Q_NULLPTR),
    m_sampleInfo(boost::none),
    m_recheckDialog(Q_NULLPTR),
    m_paTientDialog(Q_NULLPTR),
    m_filterModule(Q_NULLPTR),
    m_transferDialog(Q_NULLPTR),
    m_reCalcDialog(Q_NULLPTR),
	m_dialogAi(Q_NULLPTR),
    m_selectCheckBtn(Q_NULLPTR),
    m_statusShow(Q_NULLPTR),
    m_selectAssayCheckBtn(Q_NULLPTR),
    m_assayButton(Q_NULLPTR),
    m_shiftKey(false),
	m_isAdjustWidth(false),
	m_needUpdateDetail(false),
    m_preventRow(-1),
    m_searchTime(0),
    m_lastDirection(true),
    m_sampleTestMode(::tf::TestMode::SEQNO_MODE)
{
    ui = new Ui::QSampleDataBrowse();
    ui->setupUi(this);
    m_pPrintSetDlg = nullptr;
    Init();
}

QSampleDataBrowse::~QSampleDataBrowse()
{
}

///
/// @brief 刷新右上坐标控件
///
/// @param[in]  index  页面序号
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QSampleDataBrowse::RefreshCornerWidgets(int index)
{
    // 句柄检查（不能为空）
    if (m_parent == nullptr || m_cornerItem == nullptr)
    {
        return;
    }

    m_pageChange = true;
    // 设置切换句柄
    m_parent->setCornerWidget(m_cornerItem);
    m_cornerItem->show();

    bool isSample = (ui->stackedWidget->currentIndex() == 0);
    QSampleAssayModel::VIEWMOUDLE curModule = isSample ? QSampleAssayModel::VIEWMOUDLE::DATABROWSE : QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE;
    QSampleAssayModel::Instance().SetSampleModuleType(curModule);

    if (m_filterModule == nullptr)
    {
        m_filterModule = new QSampleFilterDataModule();
    }
    m_filterModule->setSourceModel(&QSampleAssayModel::Instance());
    ui->sample_list->setModel(m_filterModule);
    SetSampleColumnWidth();

    ResetAssayModel();

    auto row = QSampleAssayModel::Instance().GetCurrentRow();
    QModelIndex indexMode = m_filterModule->mapFromSource(QSampleAssayModel::Instance().index(row, 1));
    if (!indexMode.isValid())
    {
		QSampleDetailModel::Instance().SetData(boost::optional<tf::SampleInfo>());
        return;
    }

    if (ui->stackedWidget->currentIndex() == 0)
    {
        ui->sample_list->setCurrentIndex(indexMode);
        ui->sample_list->scrollTo(indexMode, QAbstractItemView::PositionAtCenter);
        OnAssayChanged(indexMode, true);
    }
    else
    {
        ui->show_assay_list->setCurrentIndex(indexMode);
        ui->show_assay_list->scrollTo(indexMode, QAbstractItemView::PositionAtCenter);
    }

    if (m_filterModule != Q_NULLPTR)
    {
        auto sampleNumber = m_filterModule->rowCount();
        // 样本量
        ui->sample_num_statistics->setText(QString(tr("共%1条")).arg(sampleNumber));
    }

    // 设置各按钮状态
    UpdateButtonStatus();
    // 设置页面允许功能状态
    PageAllowFun();
}

void QSampleDataBrowse::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
	POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, tr("> 工作 > ") + tr("数据浏览"));
    // 发送筛选消息，解决数据浏览与历史数据都有快捷筛选导致数据浏览不显示快捷筛选的问题 add by chenjianlin 20230816
    if (!m_strFilter.isEmpty())
    {
        emit ShowTipMessage(m_strFilter);
    }

	// 按项目展示
	if (m_showByAssayRadioBtn->isChecked())
	{
		OnShowByAssay();
	}

	// 按样本显示
	if (m_showBySampleRadioBtn->isChecked())
	{
		OnShowBySample();
	}

	// 构造查询条件（更新通信设置）
    DictionaryQueryManager::GetCommParamSet(m_cps);

	m_isAdjustWidth = true;


    // 根据情况更新快捷按钮; 
    // 放到eventShow中，是为了解决未初始完成无法获取按钮字符大小，无法计算出字符宽度，导致超出按钮宽度无法设置内容省略号
    if (m_filterDialog != Q_NULLPTR)
    {
        int i = 0;
        ui->label->hide();
        for (auto& buttonDetial : m_filterDialog->GetButtonInfo())
        {
            if (i >= m_fastButton.size())
            {
                break;
            }
            if (!buttonDetial.second.empty())
            {
                //modify bug 0024964 by wuht
                auto searchName = QString::fromStdString(buttonDetial.second);
                m_fastButton[i]->setToolTip(searchName.size() > 5 ? searchName : "");
                UiCommon::SetButtonTextWithEllipsis(m_fastButton[i], searchName); // 超出长度则显示省略号
            }
            if (buttonDetial.first)
            {
                ui->label->show();
            }

            buttonDetial.first ? m_fastButton[i]->show() : m_fastButton[i]->hide();
            i++;
        }
    }
}

void QSampleDataBrowse::DealCurrentIndex(const QModelIndex& current, bool isSample)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设置当前行为选中行
    auto fSelectCurrentIndex = [&](const QModelIndex& currentIndex)
    {
        if (!currentIndex.isValid() || m_filterModule == Q_NULLPTR)
        {
            return;
        }

        // 从代理model获取源model的Index
        auto sourceIndex = m_filterModule->mapToSource(currentIndex);
        if (!sourceIndex.isValid())
        {
            return;
        }

        auto view = ui->sample_list;
        int flag = 0;
        // 是否是按样本展示
        if (!isSample)
        {
            // 按项目展示
            flag = 1;
            view = ui->show_assay_list;
        }

        QSampleAssayModel::Instance().SetSelectFlag(flag, sourceIndex.row(), true);
		if (currentIndex.isValid())
		{
			if (!CheckIndexRange(currentIndex.row()))
			{
				ULOG(LOG_INFO, "%s(The currentrow is out of range)", __FUNCTION__);
				return;
			}

			view->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}
    };

    // 选中一个范围内的Index
    auto fSelectRangeIndex = [&](int start, int end)
    {
        int startIndex = start;
        while (startIndex <= end)
        {
            if (!CheckIndexRange(startIndex))
            {
                return;
            }

            auto checkIndex = m_filterModule->index(startIndex++, 0);
            if (!checkIndex.isValid())
            {
                continue;
            }

            fSelectCurrentIndex(checkIndex);
        }
    };

    if (!current.isValid())
    {
        return;
    }

    if (m_shiftKey && CheckIndexRange(m_preventRow))
    {
        int currentRow = current.row();
        // 检查范围正确与否
        if (!CheckIndexRange(currentRow))
        {
            return;
        }

        if (m_preventRow < currentRow)
        {
            fSelectRangeIndex(m_preventRow, currentRow);
        }
        else
        {
            fSelectRangeIndex(currentRow, m_preventRow);
        }
    }
    // 只需要选中当前行
    else
    {
        // 更新上一次选择的行
        int currentRow = current.row();
        if (CheckIndexRange(currentRow))
        {
            m_preventRow = currentRow;
        }

        fSelectCurrentIndex(current);
    }
}

bool QSampleDataBrowse::CheckIndexRange(int row)
{
	return (row >= 0 && row < m_filterModule->rowCount());
}

void QSampleDataBrowse::ProcOnSelectAll(QCheckBox* pCheck, QTableView* tbView, int sFlag)
{
    if (pCheck == Q_NULLPTR || tbView == Q_NULLPTR)
    {
        return;
    }
    // 全选
    if (pCheck->isChecked())
    {
        if (m_filterModule != Q_NULLPTR)
        {
            m_filterModule->SetSelectedAll(sFlag);
            tbView->selectAll();
        }
    }
    // 全部取消
    else
    {
        tbView->clearSelection();
        QSampleAssayModel::Instance().ClearSelectedFlag(sFlag);
    }
    UpdateButtonStatus();
}

void QSampleDataBrowse::ProcOnSortBy(QTableView* tbView, SortHeaderView* pSortHeader, int logicIndex, int order)
{
    if (tbView == nullptr || pSortHeader == nullptr)
    {
        return;
    }

	m_filterModule->SetSortOrder(order);
    //无效index或NoOrder就设置为默认未排序状态
    if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
		//还原model默认顺序
		m_filterModule->sort(0, Qt::AscendingOrder);
        //去掉排序三角样式
        pSortHeader->setSortIndicator(-1, Qt::AscendingOrder);
    }
    else
    {
        switch (order)
        {
        case SortHeaderView::DescOrder:
		{
			pSortHeader->setSortIndicator(logicIndex, Qt::DescendingOrder);
			tbView->sortByColumn(logicIndex, Qt::DescendingOrder);
		}
           
            break;
        case SortHeaderView::AscOrder:
		{
			pSortHeader->setSortIndicator(logicIndex, Qt::AscendingOrder);
			tbView->sortByColumn(logicIndex, Qt::AscendingOrder);
		}
            break;
        }
    }
}

void QSampleDataBrowse::AssignSampleComb(QComboBox* pComb)
{
    if (pComb == nullptr)
    {
        return;
    }

    pComb->clear();
    if (m_sampleTestMode == ::tf::TestMode::BARCODE_MODE)
    {
        pComb->addItem(tr("样本条码"), 1);
        pComb->addItem(tr("样本号"), 0);
    }
    else
    {
        pComb->addItem(tr("样本号"), 0);
        pComb->addItem(tr("样本条码"), 1);
    }
}

///
/// @brief 初始化页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QSampleDataBrowse::Init()
{
    // 硬件暂不支持视觉识别，暂时隐藏对应按钮——mod_tcx
    ui->flat_Ai->setVisible(false);

    m_controlModify = false;
    m_pageChange = false;
    m_checkBoxed = false;
    m_assayButton = Q_NULLPTR;

    // 注册项目设置更新消息
    REGISTER_HANDLER(MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE, this, UpdateItemShowDatas);
    // 处理历史数据审核消息
    REGISTER_HANDLER(MSG_ID_SAMPLE_AUDIT_UPDATE_HIS, this, OnAuditChanged);
    // 工作页面的设置更新
    REGISTER_HANDLER(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE, this, OnUpdateButtonStatus);
    // 关闭进度条对话框（打印对话框）
    //REGISTER_HANDLER(MSG_ID_PROGRESS_DIALOG_CLOSE, this, OnPrintFinsihed);
    // 处理复查结果更新消息
    REGISTER_HANDLER(MSG_ID_HIS_RETEST_RESULT_UPDATE, this, OnRetestResultChanged);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	// 导出设置
	m_exportDataMap[EIE_RESULT] = DATATYPE::DOUBLETYPE;
	m_exportDataMap[EIE_ORIGINAL_RESULT] = DATATYPE::DOUBLETYPE;
	m_exportDataMap[EIE_CONC] = DATATYPE::DOUBLETYPE;
	m_exportDataMap[EIE_ABS] = DATATYPE::INTTYPE;
	m_exportDataMap[EIE_SUCK_VOL] = DATATYPE::INTTYPE;
	m_exportDataMap[EIE_PRIMARY_WAVE] = DATATYPE::INTTYPE;
	m_exportDataMap[EIE_DEPUTY_WAVE] = DATATYPE::INTTYPE;

    // 1：右上角部件的显示
    m_cornerItem = new QWidget(m_parent);
    QHBoxLayout* hlayout = new QHBoxLayout(m_cornerItem);
    m_cornerItem->setLayout(hlayout);

    m_showResultDetail = nullptr;
    m_assayShowModel = nullptr;
    m_selectSampleModel = nullptr;
    m_filterDialog = nullptr;
    m_filterDialog = new QFilterSample(this);
    connect(m_filterDialog, SIGNAL(finished()), this, SLOT(OnShowFilterSampleInfo()));
    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    // 设置互斥
    pButtonGroup->setExclusive(true);
	m_showBySampleRadioBtn = new QRadioButton(tr("按样本"));
	m_showBySampleRadioBtn->setObjectName(QStringLiteral("showBySample_btn"));
	m_showBySampleRadioBtn->setCheckable(true);
    connect(m_showBySampleRadioBtn, SIGNAL(clicked()), this, SLOT(OnShowBySample()));
	m_showBySampleRadioBtn->setChecked(true);
    pButtonGroup->addButton(m_showBySampleRadioBtn);
    hlayout->addWidget(m_showBySampleRadioBtn);

	m_showByAssayRadioBtn = new QRadioButton(tr("按项目"));
	m_showByAssayRadioBtn->setObjectName(QStringLiteral("showByAssay_btn"));
    connect(m_showByAssayRadioBtn, SIGNAL(clicked()), this, SLOT(OnShowByAssay()));
    pButtonGroup->addButton(m_showByAssayRadioBtn);
    hlayout->addWidget(m_showByAssayRadioBtn);
    m_cornerItem->hide();

    // 设置代理
    ui->sample_list->setItemDelegate(new CReadOnlyDelegate(this));

    SortHeaderView *pSampleHeader = new SortHeaderView(Qt::Horizontal, ui->sample_list);
	pSampleHeader->setTextElideMode(Qt::ElideLeft);

    ui->sample_list->setHorizontalHeader(pSampleHeader);
    // 全选对话框
    {
        auto header = ui->sample_list->horizontalHeader();
        m_selectCheckBtn = new QCheckBox(header);
        connect(m_selectCheckBtn, &QCheckBox::clicked, this, [&]() {
            auto spCheckBox = qobject_cast<QCheckBox*>(sender());
            ProcOnSelectAll(spCheckBox, ui->sample_list, 0);
        });

        auto model = header->model();
        if (model != Q_NULLPTR)
        {
            auto index = model->index(0, 0);
            if (index.isValid())
            {
                header->setIndexWidget(index, m_selectCheckBtn);
            }
        }

        // 设置按钮的位置
        m_selectCheckBtn->setGeometry(header->sectionViewportPosition(0) + header->sectionSize(0) + 5, 5, 50, header->height());
    }

    SortHeaderView *pShowAssayHeader = new SortHeaderView(Qt::Horizontal, ui->show_assay_list);
	pShowAssayHeader->setTextElideMode(Qt::ElideLeft);

    ui->show_assay_list->setHorizontalHeader(pShowAssayHeader);
    // 全选对话框
    {
        auto header = ui->show_assay_list->horizontalHeader();
        m_selectAssayCheckBtn = new QCheckBox(header);
        connect(m_selectAssayCheckBtn, &QCheckBox::clicked, this, [&]() {
            auto spCheckBox = qobject_cast<QCheckBox*>(sender());
            ProcOnSelectAll(spCheckBox, ui->show_assay_list, 1);
        });

        m_selectAssayCheckBtn->setGeometry(header->sectionViewportPosition(0) + header->sectionSize(0) + 5, 5, 50, header->height());
    }

    ui->sample_list->setSortingEnabled(true);
    ui->show_assay_list->setSortingEnabled(true);
    ui->sample_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->sample_list->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));
    ui->sample_list->setItemDelegateForColumn(1, new QStatusDelegate(this));
    ui->sample_list->setSelectionMode(QAbstractItemView::MultiSelection);
	// 设置触摸屏的拖动模式（平移推动模式/滚动条拖动模式）
	ui->sample_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	ui->sample_list->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	// 单手指滑动
	// QScroller::grabGesture(ui->sample_list, QScroller::LeftMouseButtonGesture);
    ui->show_assay_list->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));
    ui->show_assay_list->setItemDelegateForColumn(1, new QStatusDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn(9, new CReadOnlyDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn(11, new CReadOnlyDelegate(this));
    ui->show_assay_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->show_assay_list->setSelectionMode(QAbstractItemView::MultiSelection);
	// 设置触摸屏的拖动模式（平移推动模式/滚动条拖动模式）
	ui->show_assay_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	ui->show_assay_list->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(pSampleHeader, &SortHeaderView::SortOrderChanged, this, [this, pSampleHeader](int logicIndex, SortHeaderView::SortOrder order)
    {
        if (logicIndex == static_cast<int>(SampleColumn::COL::INVALID) || logicIndex == static_cast<int>(SampleColumn::COL::STATUS))
        {
            return;
        }
        ProcOnSortBy(ui->sample_list, pSampleHeader, logicIndex, order); 
    });
    connect(pShowAssayHeader, &SortHeaderView::SortOrderChanged, this, [this, pShowAssayHeader](int logicIndex, SortHeaderView::SortOrder order)
    {
        if (logicIndex == static_cast<int>(SampleColumn::COLASSAY::INVALID) || logicIndex == static_cast<int>(SampleColumn::COLASSAY::STATUS))
        {
            return;
        }
        ProcOnSortBy(ui->show_assay_list, pShowAssayHeader, logicIndex, order);
    });

    //去掉排序三角样式
    ui->sample_list->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    ui->show_assay_list->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    pSampleHeader->ResetAllIndex();
    pShowAssayHeader->ResetAllIndex();

    // 注册事件处理对象
    ui->sample_list->viewport()->installEventFilter(this);
    ui->show_assay_list->viewport()->installEventFilter(this);
	ui->assay_list->viewport()->installEventFilter(this);
    // 2:样本列表的显示
    SetSampleBrowseModel();
    // 设置各按钮状态
    UpdateButtonStatus();

    // 更新全选按钮的状态
    connect(&QSampleAssayModel::Instance(), &QSampleAssayModel::selectItemChanged, this, [&]()
    {
        CheckSelectAllButtonStatus();
    });

	// 数据浏览按样本展示的情况，当有项目被选中的时候需要更新复查按钮状态
	connect(&QSampleDetailModel::Instance(), &QSampleDetailModel::selectStatusChanged, this, [&]()
	{
		auto selectData = DataPrivate::Instance().GetAllSelectFlag(0);
		if (selectData.empty())
		{
			return;
		}

		bool onlyOne = (selectData.size() == 1) ? true : false;
		// 当项目中有选中的时候，开启复查
		bool hasTested = false;
		auto testItems = QSampleDetailModel::Instance().GetTestSelectedItems();
		if (testItems.size() > 0)
		{
			hasTested = true;
		}

		// 复查功能要求有被测试的项目，同时只能选中一个
		if (hasTested && onlyOne)
		{
			ui->flat_recheck->setEnabled(true);
		}
		else
		{
			ui->flat_recheck->setEnabled(false);
		}
	});

	// 刷新结果详情中的数据
	connect(&DataPrivate::Instance(), &DataPrivate::testItemChanged, this, [&](std::set<int64_t> updateDbs)
	{
		if (m_needUpdateDetail)
		{
			// 刷新结果详情
			if (m_sampleDetailDialog != Q_NULLPTR && m_sampleDetailDialog->isVisible())
			{
				ShowSampleDetail(m_sampleDetailDialog);
			}

			m_needUpdateDetail = false;
		}
	});

    // 数据改变
    connect(&DataPrivate::Instance(), &DataPrivate::sampleChanged, this, [&](DataPrivate::ACTIONTYPE mode, int start, int end)
    {
        ULOG(LOG_INFO, "%s(mode:%d start: %d end: %d)", __FUNCTION__, mode, start, end);
		if (!this->isVisible())
		{
			return;
		}

        // 当页面不在当前页面的时候，不需要更新
        QTableView* view = Q_NULLPTR;
        if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
        {
            view = ui->sample_list;
            OnChangedRow(0);
        }
        // 按照项目展示
        else
        {
            view = ui->show_assay_list;
            OnChangedRow(1);
        }

        auto curModIdx = view->currentIndex();
        if (!curModIdx.isValid())
        {
			if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
			{
				boost::optional<tf::SampleInfo> data = boost::none;
				QSampleDetailModel::Instance().SetData(data);
			}

            if (m_filterModule != Q_NULLPTR)
            {
                auto sampleNumber = m_filterModule->rowCount();
                // 样本量
                ui->sample_num_statistics->setText(QString(tr("共%1条")).arg(sampleNumber));
            }
            CheckSelectAllButtonStatus();
        }
        else
        {
            // 删除的时候特殊处理
            if (mode == DataPrivate::ACTIONTYPE::DELETESAMPLE)
            {
                //ui->sample_list->clearSelection();
                QSampleAssayModel::Instance().ClearSelectedFlag(0);
                OnAssayChanged(curModIdx, true);
                QSampleAssayModel::Instance().SetSelectFlag(0, curModIdx.row(), true);
                UpdateButtonStatus();
                CheckSelectAllButtonStatus();
            }
            else
            {
				auto sourceIndex = m_filterModule->mapToSource(m_filterModule->index(curModIdx.row(), curModIdx.column()));
				if (sourceIndex.isValid())
				{
					if (sourceIndex.row() <= end && sourceIndex.row() >= start)
					{
						OnAssayChanged(curModIdx, true);
					}
				}
            }

            ui->sample_list->selectionModel()->select(curModIdx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }

        auto sourceIndex = m_filterModule->mapToSource(m_filterModule->index(curModIdx.row(), curModIdx.column()));
        if (sourceIndex.isValid())
        {
			// 范围内才更新(不实时更新结果详情的数据)
			if (sourceIndex.row() <= end 
				&& sourceIndex.row() >= start
				&& m_needUpdateDetail)
			{
				
				OnDataChanged();
				m_needUpdateDetail = false;
			}
        }
    });

    // 按样本展示
    connect(ui->sample_list->verticalScrollBar(), &QScrollBar::valueChanged, this, [&](int value)
    {
        OnChangedRow(value);
    });

	// 按样本展示调整列宽
	connect(ui->sample_list->horizontalHeader(), &QHeaderView::sectionResized, this, [&](int index, int oldwidth, int newWidth)
	{
		if (!m_isAdjustWidth)
		{
			return;
		}

		SampleColumn::Instance().SetAttributeOfType(SampleColumn::SAMPLEMODE::SAMPLE
			, index, newWidth);
	});

	// 按样本展示的项目列表调整列宽
	connect(ui->assay_list->horizontalHeader(), &QHeaderView::sectionResized, this, [&](int index, int oldwidth, int newWidth)
	{
		if (!m_isAdjustWidth)
		{
			return;
		}

		SampleColumn::Instance().SetAttributeOfType(SampleColumn::SAMPLEMODE::SAMPLEASSAY
			, index, newWidth);
	});

	// 按项目展示调整列宽
	connect(ui->show_assay_list->horizontalHeader(), &QHeaderView::sectionResized, this, [&](int index, int oldwidth, int newWidth)
	{
		if (!m_isAdjustWidth)
		{
			return;
		}

		SampleColumn::Instance().SetAttributeOfType(SampleColumn::SAMPLEMODE::ASSAYLIST
			, index, newWidth);
	});

    // 按样本展示中复选框和？跟随
    connect(ui->sample_list->horizontalScrollBar(), &QScrollBar::valueChanged, this, [&](int value)
    {
        auto header = ui->sample_list->horizontalHeader();
        if (header == Q_NULLPTR || m_selectCheckBtn == Q_NULLPTR || m_statusShow == Q_NULLPTR)
        {
            return;
        }
        m_selectCheckBtn->setGeometry(header->sectionViewportPosition(0) + 5, 0, 50, header->height());
        m_statusShow->setGeometry(header->sectionViewportPosition(1) + header->sectionSize(1) - 10, 14, 50, header->height());
    });

    // 按项目展示
    connect(ui->show_assay_list->verticalScrollBar(), &QScrollBar::valueChanged, this, [&](int value)
    {
        OnChangedRow(value);
    });

    // 按项目展示中复选框和？跟随
    connect(ui->show_assay_list->horizontalScrollBar(), &QScrollBar::valueChanged, this, [&](int value)
    {
        auto header = ui->show_assay_list->horizontalHeader();
        if (header == Q_NULLPTR || m_selectAssayCheckBtn == Q_NULLPTR || m_assayButton == Q_NULLPTR)
        {
            return;
        }
        m_selectAssayCheckBtn->setGeometry(header->sectionViewportPosition(0) + 5, 0, 50, header->height());
        m_assayButton->setGeometry(header->sectionViewportPosition(1) + header->sectionSize(1) - 10, 14, 50, header->height());
    });

    // 按样本展示(项目展示区域)
	ui->assay_list->horizontalHeader()->setTextElideMode(Qt::ElideLeft);
    connect(ui->assay_list->verticalScrollBar(), &QScrollBar::valueChanged, this, [&](int value)
    {
        auto view = ui->assay_list;
        auto firtsRow = view->verticalScrollBar()->value();
        // 获取指向QAbstractItemModel对象的指针
        QAbstractItemModel* model = view->model();
        if (model == Q_NULLPTR)
        {
            return;
        }

        // 获取第一行在视口中的位置
        int firstRowViewportPos = view->rowViewportPosition(firtsRow);
        // 计算当前可见的行数和第一个可见行的索引
        int firstVisibleRow = qMax(0, view->rowAt(firstRowViewportPos));
        auto rowCount = model->rowCount() - 1;
        auto computRow = view->rowAt(firstRowViewportPos + view->viewport()->height());
        if (rowCount < 0 || computRow < 0)
        {
            return;
        }

        int lastVisibleRow = qMin(rowCount, computRow);
        QSampleDetailModel::Instance().SetRange(firstVisibleRow, lastVisibleRow);
    });

    // 数据更新
    connect(&QDataItemInfo::Instance(), &QDataItemInfo::ColumnChanged, this, [&]()
    {
        // 更新信息
        boost::optional<tf::SampleInfo> data = boost::none;
        QSampleDetailModel::Instance().SetData(data);
    });

	// 视觉识别结果
	connect(ui->flat_Ai, &QPushButton::clicked, this, [&]()
	{
		int64_t db = -1;
		if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
		{
			auto select = DataPrivate::Instance().GetAllSelectFlag(1);
			if (!select.empty())
			{
				db = *(select.begin());
			}
		}
		else
		{
			auto select = DataPrivate::Instance().GetAllSelectFlag(0);
			if (!select.empty())
			{
				db = *(select.begin());
			}
		}

		if (db == -1)
		{
			return;
		}

		auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(db);
		if (spSampleInfo == Q_NULLPTR)
		{
			return;
		}

		if (m_dialogAi == Q_NULLPTR)
		{
			m_dialogAi = new QDialogAi(this);
		}

		m_dialogAi->SetSample(spSampleInfo);
		m_dialogAi->show();

	});

    // 审核按钮按钮
    connect(ui->flat_examine, &QPushButton::clicked, this, [&]()
    {
        auto selectedSampels = DataPrivate::Instance().GetAllSelectFlag(0);
        if (selectedSampels.size() <= 0)
        {
            TipDlg(tr("审核"), tr("未选中记录信息.")).exec();
            ULOG(LOG_ERROR, "have not selected any samples");
            return;
        }

		// 获取当前审核状态bug0011249
        if (!WorkpageCommon::ConfirmVerify(ui->flat_examine))
        {
            ULOG(LOG_INFO, "Cancel audit!");
            return;
        }

		std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
		if (pLoginUserInfo == nullptr)
		{
			ULOG(LOG_ERROR, "Can't get UserInfo.");
			return;
		}

        bool bVerifyStat = GetIsExamin();
		int auditCount = 0;
		int total = 1;
		total = selectedSampels.size();
        std::vector<int64_t> vecSampleID;
        for (const auto& db : selectedSampels)
        {
			//auditProgressBar->setValue(auditCount++ / total);
            // 如果当前行为空，则跳过
            auto sample = DataPrivate::Instance().GetSampleInfoByDb(db);
            if (!sample)
            {
                continue;
            }

            if (sample->status != tf::SampleStatus::type::SAMPLE_STATUS_TESTED)
            {
                continue;
            }

            sample->__set_audit(bVerifyStat);

            // 更新样本信息
            if (!DcsControlProxy::GetInstance()->ModifySampleInfo(*sample))
            {
                ULOG(LOG_ERROR, "ModifySampleInfo failed!");
                continue;
            }

			// 更新审核者信息	
			if (!WorkpageCommon::UpdateAduitInfo(sample->patientInfoId, sample->id, pLoginUserInfo->username))
			{
				ULOG(LOG_ERROR, "audit name, patient id: %id, sample id: %id failed!", pLoginUserInfo->username, sample->patientInfoId, sample->id);
				continue;
			}

            vecSampleID.push_back(sample->id);
        }

		//auditProgressBar->close();
		// 是否启用审核以后自动上传(bug3443)
		if(m_cps.bUploadResults && m_cps.bUploadAfterAudit)
		{
			// 审核以后自动上传
			// modify bug 0010694 by wuht
			if (bVerifyStat)
			{
				DcsControlProxy::GetInstance()->PushSampleToLIS(vecSampleID, tf::LisTransferType::TRANSFER_AUTO, false);
			}
		}

        POST_MESSAGE(MSG_ID_SAMPLE_AUDIT_UPDATE_DATABROWSE, vecSampleID, bVerifyStat);

    });

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index)
    {
        ui->lineEdit->setMaxLength(25);
        ui->lineEdit->clear();
        m_searchTime = 0;
    });

    connect(ui->lineEdit, &QLineEdit::textChanged, this, [&]()
    {
        m_searchTime = 0;
    });

    // 查询按钮
    connect(ui->seach_btn, &QPushButton::clicked, this, [&]()
    {
        auto searchkey = ui->lineEdit->text();
        if (searchkey == "9527")
        {
            DataPrivate::Instance().SetPrintSampleStatus(true);
        }
        else if (searchkey == "7259")
        {
            DataPrivate::Instance().SetPrintSampleStatus(false);
        }

        OnSeachSample(true, true);
    });

    // 查询按钮(向下)
    connect(ui->btn_next, &QPushButton::clicked, this, [&]()
    {
        OnSeachSample(true);
    });

    // 查询按钮(向上)
    connect(ui->btn_pre, &QPushButton::clicked, this, [&]()
    {
        OnSeachSample(false);
    });

    // 3:初始化项目列表信息
    ui->assay_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->assay_list->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->assay_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	// 设置触摸屏的拖动模式（平移推动模式/滚动条拖动模式）
	ui->assay_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	ui->assay_list->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->assay_list->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));
    ui->assay_list->horizontalHeader()->setHighlightSections(false);
    ui->assay_list->verticalHeader()->setVisible(false);

	ui->assay_list->setColumnWidth(0, 45);
	ui->assay_list->setColumnWidth(1, 200);
	ui->assay_list->setColumnWidth(5, 110);
	ui->assay_list->horizontalHeader()->setStretchLastSection(true);

    // 病人按钮被按下
    connect(ui->patient_btn, SIGNAL(clicked()), this, SLOT(OnPatientInfoBtn()));
    // 手工传输
    connect(ui->flat_manual_transfer, SIGNAL(clicked()), this, SLOT(OnTransfer()));
    // 复查按钮
    connect(ui->flat_recheck, SIGNAL(clicked()), this, SLOT(OnRecheckSample()));

    // 选择模式，增加移动
    connect(ui->sample_list->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QSampleDataBrowse::OnSampleListCurrentRowChanged);

    connect(ui->sample_list, &QTableView::clicked, this, &QSampleDataBrowse::OnClickedSampleList);

    // 设置点击样本列表事件的处理函数
    connect(ui->show_assay_list, &QTableView::clicked, this, &QSampleDataBrowse::OnClickedAssayList);

    // 设备状态的更新
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDeviceUpdate);
    // 项目配置信息更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnDataChanged);
    // 同步delegate的顺序
    REGISTER_HANDLER(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE, this, OnUpdateColumn);
    // 条码/序号模式切换
    REGISTER_HANDLER(MSG_ID_DETECTION_UPDATE, this, OnChangeMode);

    ResetAssayModel();
    ui->stackedWidget->setCurrentIndex(0);
    // 显示结果详情
    connect(ui->result_detail_btn, SIGNAL(clicked()), this, SLOT(OnShowResultDetail()));
    // 显示筛选结果对话框
    connect(ui->selectCheck, SIGNAL(clicked()), this, SLOT(OnShowFilterDialog()));
    // 删除样本的功能
    connect(ui->del_Button, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));
    // 恢复到全部数据模式
    connect(this, SIGNAL(CloseFilterLable()), this, SLOT(OnReturnPage()));
    //打印按钮被点击
    connect(ui->flat_print, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));
    // 数据导出按钮被选中
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));
    // 重新计算按钮
    connect(ui->flat_recalculate, SIGNAL(clicked()), this, SLOT(OnReCalcBtnClicked()));

    m_fastButton.push_back(ui->pushButton_3);
    m_fastButton.push_back(ui->pushButton_8);
    m_fastButton.push_back(ui->pushButton_9);
    m_fastButton.push_back(ui->pushButton_13);
    m_fastButton.push_back(ui->pushButton_14);

    auto clearOtherFocus = [&](QPushButton* targetButton)
    {
        if (targetButton == Q_NULLPTR)
        {
            return;
        }

        for (auto& button : m_fastButton)
        {
            if (targetButton == button)
            {
                continue;
            }

            // 取消选中
            button->setChecked(false);
            button->clearFocus();
        }
    };

    int index = 1;
    for (auto& button : m_fastButton)
    {
        QVariant pos(index);
        button->setProperty("postion", pos);
        connect(button, &QPushButton::clicked, this, [=]()
        {
            int pos = button->property("postion").toInt();
            if (m_filterDialog != Q_NULLPTR)
            {
                if (button->isChecked())
                {
                    m_filterDialog->SetCurrentFastCond(pos);
                    clearOtherFocus(button);
                }
                else
                {
                    m_filterDialog->SetCurrentFastCond(boost::none);
                    OnReturnPage();
                    // 隐藏快捷筛选标签
                    emit SignalHideFilterLable();
                }
            }
        });
        index++;
    }

    // 是否显示筛选按钮，当使能的时候
    connect(m_filterDialog, &QFilterSample::enableFilter, this, [&](int index, bool status)
    {
        // 参数检测
        if (index >= m_fastButton.size() || index < 0)
        {
            return;
        }

        auto button = m_fastButton[index];
        if (status)
        {
            button->show();
        }
        else
        {
            button->hide();
        }

		bool isallHide = true;
		for (const auto & button : m_fastButton)
		{
			if (button->isVisible())
			{
				isallHide = false;
			}
		}

		if (isallHide)
		{
			ui->label->hide();
		}
		else
		{
			ui->label->show();
		}
    });

    // 显示快捷名称
    connect(m_filterDialog, &QFilterSample::changeFilterName, this, [&](int index, QString name)
    {
        // 参数检测
        if (index >= m_fastButton.size() || index < 0)
        {
            return;
        }

        auto button = m_fastButton[index];
		//modify bug 0024964 by wuht
        button->setToolTip(name.size() > 5 ? name : "");
        UiCommon::SetButtonTextWithEllipsis(button, name);
    });

    // 刷新范围的更新
    OnChangedRow(0);
	// 初始化按钮状态
	OnUpdateButtonStatus();
    auto headView = ui->sample_list->horizontalHeader();
    m_statusShow = new CustomButton("", headView);
    m_statusShow->setObjectName(QStringLiteral("tooltip_btn"));
	QString tips = CommonInformationManager::GetInstance()->GetTipsContent(m_workSet.aiRecognition);
    m_statusShow->setToolTip(tips);
	// 设置按钮的位置
	m_statusShow->setGeometry(headView->sectionViewportPosition(0) + headView->sectionSize(0) + 53, 15, 50, headView->height());

    DetectionSetting detectionSetting;
    if (!DictionaryQueryManager::GetDetectionConfig(detectionSetting))
    {
        ULOG(LOG_ERROR, "Failed to get inJectionModelConfig.");
        return;
    }
    m_sampleTestMode = tf::TestMode::type(detectionSetting.testMode);
    AssignSampleComb(ui->comboBox);

    //多语言处理
    QString /*tmpStr = ui->result_detail_btn->fontMetrics().elidedText(ui->result_detail_btn->text(), Qt::ElideRight, 100);
    ui->result_detail_btn->setToolTip(ui->result_detail_btn->text());
    ui->result_detail_btn->setText(tmpStr);

    tmpStr = ui->flat_manual_transfer->fontMetrics().elidedText(ui->flat_manual_transfer->text(), Qt::ElideRight, 100);
    ui->flat_manual_transfer->setToolTip(ui->flat_manual_transfer->text());
    ui->flat_manual_transfer->setText(tmpStr);*/

    tmpStr = ui->flat_recalculate->fontMetrics().elidedText(ui->flat_recalculate->text(), Qt::ElideRight, 100);
    ui->flat_recalculate->setToolTip(ui->flat_recalculate->text());
    ui->flat_recalculate->setText(tmpStr);

    //tmpStr = ui->flat_Ai->fontMetrics().elidedText(ui->flat_Ai->text(), Qt::ElideRight, 100);
    //ui->flat_Ai->setToolTip(ui->flat_Ai->text());
    //ui->flat_Ai->setText(tmpStr);
}

void QSampleDataBrowse::OnChangeMode(DetectionSetting detectionSetting)
{
    m_sampleTestMode = tf::TestMode::type(detectionSetting.testMode);
    AssignSampleComb(ui->comboBox);
}

///
/// @brief 重置项目详情列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QSampleDataBrowse::ResetAssayModel()
{
    if (m_assayShowModel == nullptr)
    {
        m_assayShowModel = new QStandardItemModel(ui->assay_list);
    }
    m_assayShowModel->clear();
    QStringList assayHeadList = { tr("复查"), tr("项目"), tr("检测结果"),
        tr("结果状态"), tr("复查结果"), tr("结果状态"), tr("单位") , tr("模块") };
    m_assayShowModel->setHorizontalHeaderLabels(assayHeadList);
    m_assayShowModel->setRowCount(0);
    ui->assay_list->setModel(&QSampleDetailModel::Instance());
    ui->assay_list->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));

    OnUpdateColumn();
}

///
/// @brief 重置数据浏览样本筛选数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QSampleDataBrowse::ResetSampleSelectModel()
{
    ui->sample_list->setModel(&QSampleAssayModel::Instance());
    QSampleAssayModel::Instance().SetSampleModuleType(QSampleAssayModel::VIEWMOUDLE::DATABROWSE);
    ui->sample_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->sample_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->sample_list->horizontalHeader()->setHighlightSections(false);
    ui->sample_list->verticalHeader()->setVisible(false);
    SetSampleColumnWidth();
}

///
/// @brief 检查全选checkBox的是状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月14日，新建函数
///
void QSampleDataBrowse::CheckSelectAllButtonStatus()
{
    if (m_filterModule == Q_NULLPTR)
    {
        return;
    }

    int allCount = m_filterModule->rowCount();
    // 按项目展示
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        int selectCount = DataPrivate::Instance().GetAllSelectFlag(1).size();
        bool bSetChecked = (selectCount >= allCount && allCount != 0);
        m_selectAssayCheckBtn->setChecked(bSetChecked);
    }
    // 按样本展示
    else
    {
        int selectCount = DataPrivate::Instance().GetAllSelectFlag(0).size();
        bool bSetChecked = (selectCount >= allCount && allCount != 0);
        m_selectCheckBtn->setChecked(bSetChecked);
    }
	UpdateButtonStatus();
}

///
/// @brief 设置样本浏览模型
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QSampleDataBrowse::SetSampleBrowseModel()
{
    ResetAssayModel();
    if (m_filterModule == nullptr)
    {
        QSampleAssayModel::Instance().SetSampleModuleType(QSampleAssayModel::VIEWMOUDLE::DATABROWSE);
        m_filterModule = new QSampleFilterDataModule(ui->sample_list);
        m_filterModule->setSourceModel(&QSampleAssayModel::Instance());
    }

    ui->sample_list->setModel(m_filterModule);

    // 设置表格选中模式为行选中，不可多选
    ui->sample_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->sample_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->sample_list->horizontalHeader()->setHighlightSections(false);
    ui->sample_list->verticalHeader()->setVisible(false);
    ui->sample_list->horizontalHeader()->setStretchLastSection(true);
    SetSampleColumnWidth();

    // 设置表格选中模式为行选中，不可多选
    ui->show_assay_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->show_assay_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->show_assay_list->horizontalHeader()->setHighlightSections(false);
    ui->show_assay_list->verticalHeader()->setVisible(false);
    ui->show_assay_list->horizontalHeader()->setStretchLastSection(true);

    // 设置页面允许功能状态
    PageAllowFun();
}

///
/// @brief 根据数据库主键获取项目结果信息
///
/// @param[in]  id  数据库主键
///
/// @return 项目结果信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月16日，新建函数
///
std::shared_ptr<ch::tf::AssayTestResult> QSampleDataBrowse::GetAssayResultByid(int64_t id)
{
    ch::tf::AssayTestResultQueryCond queryAssaycond;
    queryAssaycond.__set_id(id);
    ch::tf::AssayTestResultQueryResp assayTestResult;

    if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        return nullptr;
    }

    return std::make_shared<ch::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);
}

std::shared_ptr<tf::CalcAssayTestResult> QSampleDataBrowse::GetCalcAssayResultByid(int64_t id)
{
	::tf::CalcAssayTestResultQueryCond queryAssaycond;
	queryAssaycond.__set_id(id);
	::tf::CalcAssayTestResultQueryResp assayTestResult;

	if (!DcsControlProxy::GetInstance()->QueryCalcAssayTestResult(assayTestResult, queryAssaycond)
		|| assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| assayTestResult.lstCalcAssayTestResult.empty())
	{
		return nullptr;
	}

	return std::make_shared<tf::CalcAssayTestResult>(assayTestResult.lstCalcAssayTestResult[0]);
}

///
/// @brief
///     根据免疫测试项目结果ID获取测试项目结果信息
///
/// @param[in]  id  测试项目结果ID
///
/// @return 测试项目结果信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月19日，新建函数
///
std::shared_ptr<im::tf::AssayTestResult> QSampleDataBrowse::ImGetAssayResultByid(int64_t id)
{
    ULOG(LOG_INFO, "%s(%lld)", __FUNCTION__, id);
    // 构造查询条件
    im::tf::AssayTestResultQueryCond queryAssaycond;
    im::tf::AssayTestResultQueryResp assayTestResult;
    queryAssaycond.__set_id(id);

    if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        return nullptr;
    }

    return std::make_shared<im::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);
}

///
/// @brief 为某一行设置checkbox
///
/// @param[in]  table  表格的view
/// @param[in]  row    具体第几行
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QSampleDataBrowse::SetSelectStatusBoxForRow(bool enable, QTableView *table, int row, int& column, bool isSelect, int assayCode)
{
    if (m_assayShowModel == Q_NULLPTR)
    {
        return;
    }

    auto checkBoxItem = new QStandardItem("");
    m_assayShowModel->setItem(row, column++, checkBoxItem);

    QWidget *widget = new QWidget(table);
    QCheckBox *checkbox = new QCheckBox();
    checkbox->setProperty("assayCode", assayCode);
    QHBoxLayout *hLayout = new QHBoxLayout();

    checkbox->setEnabled(enable);
    checkbox->setChecked(isSelect);

    hLayout->addWidget(checkbox);
    hLayout->setAlignment(checkbox, Qt::AlignCenter);
    hLayout->setMargin(0);
    widget->setLayout(hLayout);
    table->setIndexWidget(table->model()->index(row, 0), widget);
    connect(checkbox, &QCheckBox::clicked, this,
        [&]()
    {
        // 如不能获取对应的控件，则返回
        auto sendSrc = qobject_cast<QCheckBox*>(sender());
        if (sendSrc == Q_NULLPTR)
        {
            return;
        }

        tf::SampleInfo sampleInfo;
        tf::TestItem testItem;
        // 设置编号
        testItem.__set_assayCode(sendSrc->property("assayCode").toInt());
        // 设置复查与否
        testItem.__set_rerun(sendSrc->isChecked());
        sampleInfo.__set_testItems(std::vector<tf::TestItem>(1, testItem));
        // 保存数据
        OnSaveModify(sampleInfo);
    }
    );
}

void QSampleDataBrowse::OnAssayChanged(const QModelIndex& index, bool isupdate)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (QSampleAssayModel::Instance().GetModule() != QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        return;
    }

    if (!index.isValid())
    {
        QSampleDetailModel::Instance().SetData(boost::optional<tf::SampleInfo>());
        return;
    }

    // 获取数据模型(外面是过滤)
    auto sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
    if (sampleProxyMoudel == nullptr)
    {
        return;
    }

    // 重置正在测试的项目
    DataPrivate::Instance().ClearRuntimeList();
    auto sourceData = sampleProxyMoudel->GetSampleByIndex(index);
    if (!sourceData)
    {
        UpdateButtonStatus();
        QSampleDetailModel::Instance().SetData(sourceData, m_sampleTestMode);
        return;
    }

    if (isupdate)
    {
        QSampleDetailModel::Instance().SetData(sourceData, m_sampleTestMode);
    }

    UpdateButtonStatus();
}

void QSampleDataBrowse::OnUpdateAssay()
{
    auto sourceData = QSampleDetailModel::Instance().GetCurrentSample();
    if (!sourceData)
    {
        return;
    }

	ULOG(LOG_INFO, "%s(update sample id: %d, seqNo: %s)", __FUNCTION__, sourceData->id, sourceData->seqNo);
    QSampleDetailModel::Instance().SetData(sourceData, m_sampleTestMode);
}

void QSampleDataBrowse::keyPressEvent(QKeyEvent* event)
{
    if (event == Q_NULLPTR)
    {
        return;
    }

    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        m_controlModify = true;
    }

    if (event->modifiers().testFlag(Qt::ShiftModifier))
    {
        m_shiftKey = true;
    }
}

void QSampleDataBrowse::keyReleaseEvent(QKeyEvent* event)
{
    if (event == Q_NULLPTR)
    {
        return;
    }

    if (event->key() == Qt::Key_Control)
    {
        m_controlModify = false;
    }

    if (event->key() == Qt::Key_Shift)
    {
        m_shiftKey = false;
    }
}

void QSampleDataBrowse::OnUpdateColumn()
{
    auto funcAssignAttri = [this](QTableView *qTb, const std::vector<int>& vecColumns, SampleColumn::SAMPLEMODE modeType)
	{
        /*int iResultAssayCol = ((modeType == QDataItemInfo::SAMPLEMODE::SAMPLEASSAY) ? (int)QDataItemInfo::SAMPLEASSAY::ASSAY : \
            (modeType == QDataItemInfo::SAMPLEMODE::ASSAYLIST ? (int)QDataItemInfo::COLASSAY::ASSAY : -1));*/
        
		int iResultAssayCol;
		if (modeType == SampleColumn::SAMPLEMODE::SAMPLEASSAY)
		{
			iResultAssayCol = (int)SampleColumn::SAMPLEASSAY::ASSAY;
		}
		else if(modeType == SampleColumn::SAMPLEMODE::ASSAYLIST)
		{
			iResultAssayCol = (int)SampleColumn::COLASSAY::ASSAY;
		}
		else
		{
			iResultAssayCol = -1;
		}
		
		for (int iCol : vecColumns)
        {
            auto typeColumn = SampleColumn::Instance().GetAttributeByType(modeType, iCol);
            if (typeColumn)
            {
                auto attribute = typeColumn.value();
                // 设置列
                if (iCol == iResultAssayCol)
                {
                    qTb->setItemDelegateForColumn(attribute.first, new QResultDelegate(this));
                }
                else
                {
                    qTb->setItemDelegateForColumn(attribute.first, new CReadOnlyDelegate(this));
                }

                // 设置宽度
                if (attribute.second > 0)
                {
                    qTb->setColumnWidth(attribute.first, attribute.second);
                }
            }
        }
    };

	ui->assay_list->setItemDelegate(new CReadOnlyDelegate(ui->assay_list));

	// 按样本展示的项目列表需改变的类型
    std::vector<int> sampleAssayTypes = {
        (int)SampleColumn::SAMPLEASSAY::ASSAY,
        (int)SampleColumn::SAMPLEASSAY::RESULT,
        (int)SampleColumn::SAMPLEASSAY::RESULTSTATUS,
        (int)SampleColumn::SAMPLEASSAY::RECHECKRESULT,
        (int)SampleColumn::SAMPLEASSAY::RECHECKSTATUS,
        (int)SampleColumn::SAMPLEASSAY::UNIT,
        (int)SampleColumn::SAMPLEASSAY::MODULE,
        (int)SampleColumn::SAMPLEASSAY::FIRSTABSORB,
        (int)SampleColumn::SAMPLEASSAY::REABSORB };
    funcAssignAttri(ui->assay_list, sampleAssayTypes, SampleColumn::SAMPLEMODE::SAMPLEASSAY);

	// 按项目展示
    std::vector<int> assayTypes = {
        (int)SampleColumn::COLASSAY::SEQNO,
        (int)SampleColumn::COLASSAY::BARCODE,
        (int)SampleColumn::COLASSAY::TYPE,
        (int)SampleColumn::COLASSAY::POS,
        (int)SampleColumn::COLASSAY::ENDTIME,
        (int)SampleColumn::COLASSAY::RECHECKENDTIME,
        (int)SampleColumn::COLASSAY::AUDIT,
        (int)SampleColumn::COLASSAY::PRINT,
        (int)SampleColumn::COLASSAY::ASSAY,
        (int)SampleColumn::COLASSAY::RESULT,
        (int)SampleColumn::COLASSAY::RESULTSTATUS,
        (int)SampleColumn::COLASSAY::RECHECKRESULT,
        (int)SampleColumn::COLASSAY::RECHECKSTATUS,
        (int)SampleColumn::COLASSAY::UNIT,
        (int)SampleColumn::COLASSAY::MODULE,
        (int)SampleColumn::COLASSAY::FIRSTABSORB,
        (int)SampleColumn::COLASSAY::REABSORB };
    funcAssignAttri(ui->show_assay_list, assayTypes, SampleColumn::SAMPLEMODE::ASSAYLIST);
	
	// 按样本展示
    std::vector<int> sampleTypes = {
        (int)SampleColumn::COL::SEQNO,
        (int)SampleColumn::COL::BARCODE,
        (int)SampleColumn::COL::TYPE,
        (int)SampleColumn::COL::POS,
        (int)SampleColumn::COL::ENDTIME,
        (int)SampleColumn::COL::RECHECKENDTIME,
        (int)SampleColumn::COL::AUDIT,
        (int)SampleColumn::COL::PRINT };
    funcAssignAttri(ui->sample_list, sampleTypes, SampleColumn::SAMPLEMODE::SAMPLE);
}

void QSampleDataBrowse::OnClickedSampleList(const QModelIndex& current)
{
    ULOG(LOG_INFO, "%s(clicked)", __FUNCTION__);

    if (!current.isValid())
    {
        return;
    }

    if (!CheckIndexRange(current.row()))
    {
        ULOG(LOG_INFO, "%s(The currentrow is out of range)", __FUNCTION__);
        return;
    }

    // 第0列-checkbox
    int column = current.column();
    if (0 != column)
    {
        if (!m_controlModify)
        {
            ui->sample_list->clearSelection();
            QSampleAssayModel::Instance().ClearSelectedFlag(0);
        }

        DealCurrentIndex(current);
    }
    else
    {
        // 更新上一次选择的行
        m_preventRow = current.row();
        auto sourceIndex = m_filterModule->mapToSource(m_filterModule->index(current.row(), current.column()));
        if (!sourceIndex.isValid())
        {
            return;
        }

        // 获取之前的状态
        auto flagData = DataPrivate::Instance().GetSelectFlag(0, sourceIndex.row());
        if (!flagData.isValid())
        {
            return;
        }
        auto flag = flagData.toBool();
        // 取反
        QSampleAssayModel::Instance().SetSelectFlag(0, sourceIndex.row(), !flag);
        if (!flag)
        {
            ui->sample_list->selectionModel()->select(current, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        else
        {
            ui->sample_list->selectionModel()->select(current, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);

            // 项目列表显示选中样本的
            int selectedRow = DataPrivate::Instance().GetPrevSelectedRow(0);
            if (selectedRow != -1)
            {
                QModelIndex preIdx = ui->sample_list->model()->index(selectedRow, 0);
                if (preIdx.isValid())
                {
                    OnAssayChanged(preIdx, true);
                }
            }
            return;
        }
    }

    OnAssayChanged(current, true);
}

void QSampleDataBrowse::OnClickedAssayList(const QModelIndex& current)
{
    if (!current.isValid())
    {
        return;
    }

    if (!CheckIndexRange(current.row()))
    {
        ULOG(LOG_INFO, "%s(The currentrow is out of range)", __FUNCTION__);
        return;
    }

    // 第0列-checkbox
    int column = current.column();
    if (0 != column)
    {
        if (!m_controlModify)
        {
            ui->show_assay_list->clearSelection();
            QSampleAssayModel::Instance().ClearSelectedFlag(1);
        }

        DealCurrentIndex(current, false);
    }
    else
    {
        // 更新上一次选择的行
        m_preventRow = current.row();
        auto sourceIndex = m_filterModule->mapToSource(m_filterModule->index(current.row(), current.column()));
        if (!sourceIndex.isValid())
        {
            return;
        }

        // 获取之前的状态(bug3194，bug3188)
        auto flagData = DataPrivate::Instance().GetSelectFlag(1, sourceIndex.row());
        if (!flagData.isValid())
        {
            return;
        }
        auto flag = flagData.toBool();
        // 取反
        QSampleAssayModel::Instance().SetSelectFlag(1, sourceIndex.row(), !flag);
        if (!flag)
        {
            ui->show_assay_list->selectionModel()->select(current, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        else
        {
            ui->show_assay_list->selectionModel()->select(current, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
        }
    }

    QSampleAssayModel::Instance().SetCurrentSampleRow(current.row());
    // modify bug0011088 by wuht
    UpdateButtonStatus();
}

///
/// @brief 显示结果详情
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QSampleDataBrowse::OnShowResultDetail()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 创建结果详情对话框
    if (m_sampleDetailDialog == Q_NULLPTR)
    {
        m_sampleDetailDialog = new ItemResultDetailDlg(this);
        connect(m_sampleDetailDialog, &ItemResultDetailDlg::SigPrevBtnClicked, this, [&]() {OnMoveBtn(false); });
        connect(m_sampleDetailDialog, &ItemResultDetailDlg::SigNextBtnClicked, this, [&]() {OnMoveBtn(true); });
		connect(m_sampleDetailDialog, &ItemResultDetailDlg::dataChanged, this, [&]()
		{
			m_needUpdateDetail = true;
		});
    }

    // 刷新成功
    if (!ShowSampleDetail(m_sampleDetailDialog))
    {
        return;
    }

    // 弹出对话框
    m_sampleDetailDialog->exec();
}

///
/// @brief 筛选对话框
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QSampleDataBrowse::OnShowFilterDialog()
{
    if (m_filterDialog == nullptr)
    {
        m_filterDialog = new QFilterSample(this);
        // 显示筛选后的内容
        connect(m_filterDialog, SIGNAL(finished()), this, SLOT(OnShowFilterSampleInfo()));
    }

    m_filterDialog->show();
}

///
/// @brief 筛选范围页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QSampleDataBrowse::OnReturnPage()
{
    // 当前窗口切换时不清除
    if (this->isHidden())
    {
        return;
    }

    if (m_filterModule != Q_NULLPTR)
    {
        m_filterModule->SetEnable(false);
    }

	m_filterModule->invalidate();
	CheckSelectAllButtonStatus();
    //RefreshCornerWidgets(ui->stackedWidget->currentIndex());
	// 取消选中状态
	for (auto& button : m_fastButton)
	{
		button->setChecked(false);
		button->clearFocus();
	}
    // 请除快捷筛选内容
    m_strFilter.clear();
    UpdateButtonStatus();
}

bool QSampleDataBrowse::UpdatePatientInfoStep()
{
    std::tuple<int, int, int64_t> keyData;
    if (!GetCurrentPatientKey(keyData))
    {
        ULOG(LOG_WARN, "Failed to get current patient key.");
        return false;
    }

    // 条目总数量
    auto totalCount = std::get<0>(keyData);
    // 当前行
    auto currentRow = std::get<1>(keyData);
    // 样本Id
    auto sampleId = std::get<2>(keyData);

    m_paTientDialog->UpdatePatientData(sampleId, !IsExistPatientSampleByDirection(-1).isValid(), !IsExistPatientSampleByDirection(1).isValid());

    return true;
}

bool QSampleDataBrowse::GetIsExamin()
{
    return (ui->flat_examine->text() == tr("审核"));
}
QModelIndex QSampleDataBrowse::IsExistPatientSampleByDirection(int step)
{
    QSampleFilterDataModule* sampleProxyMoudel = Q_NULLPTR;
    QTableView* list = Q_NULLPTR;
    // 是否按样本展示
    bool showSample = QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE;

    if (showSample)
    {
        list = ui->sample_list;
    }
    else
    {
        list = ui->show_assay_list;
    }

    if (step == 0)
        return list->currentIndex();

    // 获取样本的数据库主键
    sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(list->model());
    if (sampleProxyMoudel == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "%s(), pSampleProxyModel == Q_NULLPTR", __FUNCTION__);
        return QModelIndex();
    }

    QModelIndex tempModIdx = sampleProxyMoudel->index(list->currentIndex().row() + step, list->currentIndex().column());

    while (tempModIdx.isValid())
    {
        bool isPatientType = false;

        if (showSample)
        {
            // 获取选中项目数据
            auto sampleData = sampleProxyMoudel->GetSampleByIndex(tempModIdx);
            if (!sampleData)
            {
                ULOG(LOG_WARN, "%s(), sampleData", __FUNCTION__);
                return QModelIndex();
            }

            isPatientType = (sampleData->sampleType == tf::SampleType::SAMPLE_TYPE_PATIENT);
        }
        else
        {
            // 获取选中项目数据
            auto testItemData = sampleProxyMoudel->GetTestItemByIndex(tempModIdx);
            if (!testItemData)
            {
                ULOG(LOG_WARN, "%s(), testItemData", __FUNCTION__);
                return QModelIndex();
            }

            auto spSample = DataPrivate::Instance().GetSampleInfoByDb(testItemData->sampleInfoId);
            if (!spSample)
            {
                ULOG(LOG_WARN, "GetSampleInfoByDb failed, sanmpleId is %lld.", testItemData->sampleInfoId);
                return QModelIndex();
            }

            isPatientType = (spSample->sampleType == tf::SampleType::SAMPLE_TYPE_PATIENT);
        }

        if (isPatientType)
        {
            return tempModIdx;
        }

        tempModIdx = sampleProxyMoudel->index(tempModIdx.row() + step, tempModIdx.column());
    }

    return QModelIndex();
}

///
/// @brief 查看病人信息按钮被按下
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QSampleDataBrowse::OnPatientInfoBtn()
{
    if (m_paTientDialog == nullptr)
    {
        m_paTientDialog = new QPatientDialog(this);
        // 上一条下一条
        connect(m_paTientDialog, SIGNAL(MoveButtonPressed(int)), this, SLOT(MovePatientInfo(int)));
    }

    UpdatePatientInfoStep();

    m_paTientDialog->show();
}

///
/// @brief 浏览上一条或者下一条病人信息
///
/// @param[in]  orient  1:下一条，-1：上一条
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QSampleDataBrowse::MovePatientInfo(int orient)
{
    if (m_paTientDialog == nullptr)
    {
        return;
    }

    QTableView* list = Q_NULLPTR;
	int mode = 0;
    // 是否按样本展示
    bool showSample = QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE;
    if (showSample)
    {
		mode = 0;
        list = ui->sample_list;
    }
    else
    {
		mode = 1;
        list = ui->show_assay_list;
    }

    const auto& nextModelIndex = IsExistPatientSampleByDirection(orient);
    if (nextModelIndex.isValid())
    {
		QSampleAssayModel::Instance().ClearSelectedFlag(mode);
		QSampleAssayModel::Instance().SetSelectFlag(mode, nextModelIndex.row(), true, false);
		list->clearSelection();
        list->setCurrentIndex(nextModelIndex);
    }

    // 设置各按钮状态
    UpdateButtonStatus();

    UpdatePatientInfoStep();
}

///
/// @brief 刷新过滤样本
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QSampleDataBrowse::OnShowFilterSampleInfo()
{
    // 重置模型
    //ResetAssayModel();
    if (m_filterModule == nullptr)
    {
        m_filterModule = new QSampleFilterDataModule(/*ui->sample_list*/);
        m_filterModule->setSourceModel(&QSampleAssayModel::Instance());
        ui->sample_list->setModel(m_filterModule);
    }

	// 按项目展示
	if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
	{
		ui->show_assay_list->clearSelection();
		QSampleAssayModel::Instance().ClearSelectedFlag(1);
	}
	// 按样本展示
	else
	{
		ui->sample_list->clearSelection();
		QSampleAssayModel::Instance().ClearSelectedFlag(0);
	}

    // 开启筛选
    m_filterModule->SetEnable(true);
    FilterConDition filterCond = m_filterDialog->GetFilterCond();
	// modify bug0012210 by wuht
// 	if (!filterCond.enable)
// 	{
// 		return;
// 	}

    // 发送筛选消息
    m_strFilter = QString(tr("筛选:  %1个筛选条件")).arg(filterCond.GetCondNumber().first);
	m_strFilter += "|";
	m_strFilter += QString::fromStdString(filterCond.GetCondNumber().second);
    emit ShowTipMessage(m_strFilter);
    m_filterModule->Setcond(filterCond);

	auto clearOtherFocus = [&](QPushButton* targetButton)
	{
		if (targetButton == Q_NULLPTR)
		{
			return;
		}

		for (auto& button : m_fastButton)
		{
			if (targetButton == button)
			{
				button->setChecked(true);
				continue;
			}

			// 取消选中
			button->setChecked(false);
			button->clearFocus();
		}
	};

	for (auto& button : m_fastButton)
	{
		// 取消选中
		button->setChecked(false);
		button->clearFocus();
	}

	auto currentPage = m_filterDialog->GetCurrentPage();
	if (currentPage)
	{
		auto index = currentPage.value();
		if (index <= m_fastButton.size() && index > 0)
		{
			clearOtherFocus(m_fastButton[index -1]);
		}
	}

    RefreshCornerWidgets(ui->stackedWidget->currentIndex());
    UpdateButtonStatus();
}

///
/// @brief 删除选中的样本
///
///
/// @return true删除成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
bool QSampleDataBrowse::DealSeriesSampIe(int method)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 查找登记表中对应的选中模型和标准模型
    QItemSelectionModel* slectmodule = ui->sample_list->selectionModel();
    auto sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
    // 如果模型为空，则返回
    if (slectmodule == Q_NULLPTR || sampleProxyMoudel == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "Model is null");
        return true;
    }

    // 获取选中范围
    QModelIndexList selectIndexs = slectmodule->selectedRows();

    // 删除样本
    if (1 == method)
    {
        // 逐行获取要删除的样本信息的数据库主键
        std::vector<int64_t> delDbNos;
        for (const auto& index : selectIndexs)
        {
            // 如果当前行为空，则跳过
            auto sample = sampleProxyMoudel->GetSampleByIndex(index);
            // 样本值
            auto sampleValue = sample.value();
            // 存在样本处于测试中（P状态）
            if (sampleValue.status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
            {
                if (selectIndexs.count() > 1)
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("选中样本中正在测试的样本不可删除，是否继续删除其他样本？"), TipDlgType::TWO_BUTTON));
                    if (pTipDlg->exec() == QDialog::Rejected)
                    {
                        ULOG(LOG_INFO, "Cancel delete!");
                        return false;
                    }
                }
                else
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("样本正在检测中，不可删除！")));
                    pTipDlg->exec();
                    return false;
                }

                // 若继续
                continue;
            }

            delDbNos.push_back(sampleValue.id);
        }

        // 删除选中样本
        if (!DcsControlProxy::GetInstance()->RemoveSampleInfos(delDbNos))
        {
            ULOG(LOG_ERROR, "RemoveSampleInfos failed!");
            return false;
        }
    }
    // 审核或者取消审核样本
    else if (2 == method)
    {
        bool bStatus = GetIsExamin();

        std::vector<int64_t> vecSampleID;
        for (const auto& index : selectIndexs)
        {
            // 如果当前行为空，则跳过
            auto sample = sampleProxyMoudel->GetSampleByIndex(index);
            if (!sample.has_value())
            {
                continue;
            }

            auto sampleInfo = sample.value();
            sampleInfo.__set_audit(bStatus);

            // 更新样本信息
            if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
            {
                ULOG(LOG_ERROR, "ModifySampleInfo failed!");
                continue;
            }
            vecSampleID.push_back(sampleInfo.id);
        }
        POST_MESSAGE(MSG_ID_SAMPLE_AUDIT_UPDATE_DATABROWSE, vecSampleID, bStatus);
    }

    return true;
}

///
/// @brief 事件过滤器
///
/// @param[in]  target  目标对象
/// @param[in]  event   时间对象
///
/// @return 处理则返回true，否则返回false
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月12日，新建函数
///
bool QSampleDataBrowse::eventFilter(QObject* target, QEvent* event)
{
    QTableView* view = Q_NULLPTR;
	QTableView* viewAssay = Q_NULLPTR;
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        view = ui->sample_list;
		viewAssay = ui->assay_list;
    }
    // 按照项目展示
    else
    {
        view = ui->show_assay_list;
    }

    // qtabview和focusOut同时满足
    if (target == view->viewport())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent != nullptr)
            {
                auto index = view->indexAt(mouseEvent->pos());
				/*(modify bug0012086 by wuht)
                if (!index.isValid())
                {
                    // 按样本展示
                    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
                    {
                        // 设置各按钮状态
                        QSampleAssayModel::Instance().SetCurrentSampleRow(-1);
                        view->setCurrentIndex(QModelIndex());
                        boost::optional<tf::SampleInfo> sourceData = boost::none;
                        QSampleDetailModel::Instance().SetData(sourceData);
                        UpdateButtonStatus();
                        ResetAssayModel();
                    }
                    // 按项目展示
                    else
                    {
                        QSampleAssayModel::Instance().SetCurrentSampleRow(-1);
                        view->setCurrentIndex(QModelIndex());
                        UpdateButtonStatus();
                        ResetAssayModel();
                    }

                    m_preventRow = -1;
                }*/
            }
        }
		// 冒泡显示 modify bug1615 by wuht
		else if (event->type() == QEvent::ToolTip)
		{
			QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
			if (helpEvent != Q_NULLPTR)
			{
				auto index = view->indexAt(helpEvent->pos());
				if (index.isValid())
				{
					auto rawData = view->model()->data(index, Qt::DisplayRole);
					QString tipContent = rawData.toString();
					// modify bug0011923 by wuht
					QToolTip::showText(helpEvent->globalPos(), ToCfgFmtDateTime(tipContent));
				}
			}
		}
    }
	// 项目展示 modify bug1615 by wuht
	else if (viewAssay != Q_NULLPTR && target == viewAssay->viewport())
	{
		// 冒泡显示
		if (event->type() == QEvent::ToolTip)
		{
			QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
			if (helpEvent != Q_NULLPTR)
			{
				auto index = viewAssay->indexAt(helpEvent->pos());
				if (index.isValid())
				{
					auto rawData = viewAssay->model()->data(index, Qt::DisplayRole);
					QString tipContent = rawData.toString();
					// modify bug0011923 by wuht
					QToolTip::showText(helpEvent->globalPos(), ToCfgFmtDateTime(tipContent));
				}
			}
		}
    }

    // 当按Ctrl+Esc时，m_controlModify没有收到Release复位，所以在这里统一重置
    if (event->type() == QEvent::WindowDeactivate)
    {
        m_controlModify = false;
        m_shiftKey = false;
    }

    return QWorkShellPage::eventFilter(target, event);
}

///
/// @brief
///     根据结果关键信息获取项目结果(可考虑使用通用项目结果类型，使UI与设备类型隔离)
///
/// @param[in]  stuResultKey  结果关键信息
/// @param[out] pVarRlt       项目测试结果信息
///
/// @return 项目结果
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月19日，新建函数
///
QString QSampleDataBrowse::GetItemResult(const tf::TestResultKeyInfo& stuResultKey, QVariant* pVarRlt /*= Q_NULLPTR*/)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 构造项目结果保存临时变量
    QVariant varRltTemp;
    if (pVarRlt == Q_NULLPTR)
    {
        pVarRlt = &varRltTemp;
    }

    // 判断项目结果设备类型
    if (stuResultKey.deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
    {
        ULOG(LOG_INFO, "%s(), stuResultKey.deviceType == tf::DeviceType::DEVICE_TYPE_I6000", __FUNCTION__);
        // 免疫
        // 构造查询条件
        im::tf::AssayTestResultQueryCond queryAssaycond;
        im::tf::AssayTestResultQueryResp assayTestResult;
        queryAssaycond.__set_id(stuResultKey.assayTestResultId);

        if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
            || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || assayTestResult.lstAssayTestResult.empty()
            || !assayTestResult.lstAssayTestResult[0].__isset.conc)
        {
            ULOG(LOG_ERROR, "%s()", __FUNCTION__);
            return QString("");
        }

        // 保存结果信息
        pVarRlt->setValue<im::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);

        return QString::number(assayTestResult.lstAssayTestResult[0].conc);
    }

    // 判断项目结果设备类型
    if (stuResultKey.deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
    {
        ULOG(LOG_INFO, "%s(), stuResultKey.deviceType == tf::DeviceType::DEVICE_TYPE_C1000", __FUNCTION__);
        // 生化
        // 构造查询条件
        ch::tf::AssayTestResultQueryCond queryAssaycond;
        ch::tf::AssayTestResultQueryResp assayTestResult;
        queryAssaycond.__set_id(stuResultKey.assayTestResultId);

        if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
            || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || assayTestResult.lstAssayTestResult.empty()
            || !assayTestResult.lstAssayTestResult[0].__isset.conc)
        {
            ULOG(LOG_ERROR, "%s()", __FUNCTION__);
            return QString("");
        }

        // 保存结果信息
        pVarRlt->setValue<ch::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);

        return QString::number(assayTestResult.lstAssayTestResult[0].conc);
    }

    return QString("");
}

///
/// @brief 刷新结果详情页面
///
/// @param[in]  dialog  结果详情的对话框
///
/// @return true,刷新成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
bool QSampleDataBrowse::ShowSampleDetail(ItemResultDetailDlg* dialog)
{
    if (dialog == Q_NULLPTR)
    {
        return false;
    }

    // 按样本展示
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        // 获取当前行号
        QModelIndex SelModIndex = ui->sample_list->currentIndex();
        if (!SelModIndex.isValid())
        {
            ULOG(LOG_WARN, "%s(), !index.isValid()", __FUNCTION__);
            return false;
        }

        // 获取样本的数据库主键
        auto pSampleProxyModel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
        if (pSampleProxyModel == Q_NULLPTR)
        {
            ULOG(LOG_WARN, "%s(), pSampleProxyModel == Q_NULLPTR", __FUNCTION__);
            return false;
        }

        // 获取选中样本数据
        auto stuSelSampleInfo = pSampleProxyModel->GetSampleByIndex(SelModIndex);
        if (!stuSelSampleInfo)
        {
            ULOG(LOG_WARN, "%s(), !stuSelSampleInfo", __FUNCTION__);
            return false;
        }

        // 设置样本ID
        std::vector<int64_t> testItems;
        // 只有一行
        if (pSampleProxyModel->rowCount() <= 1)
        {
            dialog->SetSampleId(stuSelSampleInfo->id, testItems, true, true);
        }
        // 最后一行
        else if (pSampleProxyModel->rowCount() == (SelModIndex.row() + 1))
        {
            dialog->SetSampleId(stuSelSampleInfo->id, testItems, false, true);
        }
        // 第一行
        else if (SelModIndex.row() == 0)
        {
            dialog->SetSampleId(stuSelSampleInfo->id, testItems, true, false);
        }
        else
        {
            dialog->SetSampleId(stuSelSampleInfo->id, testItems, false, false);
        }
    }
    // 按项目展示
    else
    {
        // 获取当前行号
        QModelIndex SelModIndex = ui->show_assay_list->currentIndex();
        if (!SelModIndex.isValid())
        {
            ULOG(LOG_WARN, "%s(), !index.isValid()", __FUNCTION__);
            return false;
        }

        // 获取样本的数据库主键
        auto pSampleProxyModel = static_cast<QSampleFilterDataModule*>(ui->show_assay_list->model());
        if (pSampleProxyModel == Q_NULLPTR)
        {
            ULOG(LOG_WARN, "%s(), pSampleProxyModel == Q_NULLPTR", __FUNCTION__);
            return false;
        }

        // 获取选中项目数据
        auto testItemInfo = pSampleProxyModel->GetTestItemByIndex(SelModIndex);
        if (!testItemInfo)
        {
            ULOG(LOG_WARN, "%s(), !testItemInfo", __FUNCTION__);
            return false;
        }

		int assayCode = testItemInfo.value().assayCode;
		bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(assayCode);
		if (isCalc)
		{
			ULOG(LOG_WARN, "%s(the assayCode: %d is calc),", __FUNCTION__, assayCode);
			return false;
		}

        // 设置样本ID
        std::vector<int64_t> testItems;
        testItems.push_back(testItemInfo->id);
        // 只有一行
        if (pSampleProxyModel->rowCount() <= 1)
        {
            dialog->SetSampleId(testItemInfo->sampleInfoId, testItems, true, true);
        }
        // 最后一行
        else if (pSampleProxyModel->rowCount() == (SelModIndex.row() + 1))
        {
            dialog->SetSampleId(testItemInfo->sampleInfoId, testItems, false, true);
        }
        // 第一行
        else if (SelModIndex.row() == 0)
        {
            dialog->SetSampleId(testItemInfo->sampleInfoId, testItems, true, false);
        }
        else
        {
            dialog->SetSampleId(testItemInfo->sampleInfoId, testItems, false, false);
        }
    }

    return true;
}

///
/// @brief 获取当前选中的项目的索引
///
///
/// @return 返回索引
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月28日，新建函数
///
boost::optional<std::tuple<QModelIndex, QTableView*, QSampleFilterDataModule*>> QSampleDataBrowse::GetCurrentIndex()
{
    QSampleFilterDataModule* sampleProxyMoudel = Q_NULLPTR;
    QModelIndex curModIdx;
    QTableView* list = Q_NULLPTR;
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        // 获取登记表的当前索引和标准模型
        curModIdx = ui->sample_list->currentIndex();
        list = ui->sample_list;
        // 判断是否选中有效索引
        if (!curModIdx.isValid())
        {
            ULOG(LOG_ERROR, "Select invalid item");
            return boost::none;
        }

        sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
        if (Q_NULLPTR == sampleProxyMoudel)
        {
            return boost::none;
        }
    }
    else
    {
        // 获取当前行号
        curModIdx = ui->show_assay_list->currentIndex();
        list = ui->show_assay_list;
        if (!curModIdx.isValid())
        {
            ULOG(LOG_WARN, "%s(), !index.isValid()", __FUNCTION__);
            return boost::none;
        }

        // 获取样本的数据库主键
        sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->show_assay_list->model());
        if (sampleProxyMoudel == Q_NULLPTR)
        {
            ULOG(LOG_WARN, "%s(), pSampleProxyModel == Q_NULLPTR", __FUNCTION__);
            return boost::none;
        }
    }

    // 返回最终值
    return boost::make_optional(std::make_tuple(curModIdx, list, sampleProxyMoudel));
}

///
/// @brief 更新按钮状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月5日，新建函数
///
void QSampleDataBrowse::UpdateButtonStatus()
{
    ui->selectCheck->setEnabled(false);
    ui->patient_btn->setEnabled(false);
    ui->result_detail_btn->setEnabled(false);
    ui->flat_recheck->setEnabled(false);
    ui->flat_examine->setText(tr("审核"));
    ui->flat_examine->setEnabled(false);
    ui->flat_manual_transfer->setEnabled(false);
    ui->flat_print->setEnabled(false);
    ui->export_btn->setEnabled(false);
    ui->del_Button->setEnabled(false);
    ui->flat_recalculate->setEnabled(false);
	ui->flat_Ai->setEnabled(false);

	// 是否开启AI识别
	if (QDataItemInfo::Instance().IsAiRecognition())
	{
		ui->flat_Ai->show();
	}
 	else
 	{
 		ui->flat_Ai->hide();
 	}

	// 样本是否是测试完成状态
	auto IsSampleTestStatus = [](std::shared_ptr<tf::SampleInfo>& spSample)->bool
	{
		if (spSample == Q_NULLPTR)
		{
			return false;
		}

		return (spSample->__isset.status && spSample->status == tf::SampleStatus::type::SAMPLE_STATUS_TESTED);
	};

    if (m_filterModule != Q_NULLPTR)
    {
        auto sampleNumber = m_filterModule->rowCount();
        // 样本量
        ui->sample_num_statistics->setText(QString(tr("共%1条")).arg(sampleNumber));

        // 只有有数据的情况下才能进行筛选
        if (sampleNumber > 0)
        {
            ui->flat_recalculate->setEnabled(true);
        }
        ui->selectCheck->setEnabled(true);
    }

    // 按项目展示的处理
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        bool onlyOne = (DataPrivate::Instance().GetAllSelectFlag(1).size() == 1) ? true : false;

        // 获取当前行号
        QModelIndex CurrentIndex = ui->show_assay_list->currentIndex();
        if (!CurrentIndex.isValid())
        {
            ULOG(LOG_WARN, "%s(), CurrentIndex.isValid()", __FUNCTION__);
            return;
        }

        // 获取样本的数据库主键
        auto pSampleProxyModel = static_cast<QSampleFilterDataModule*>(ui->show_assay_list->model());
        if (pSampleProxyModel == Q_NULLPTR)
        {
            ULOG(LOG_WARN, "%s(), pSampleProxyModel == Q_NULLPTR", __FUNCTION__);
            return;
        }

        // 获取选中项目数据
        auto testItemData = pSampleProxyModel->GetTestItemByIndex(CurrentIndex);
        if (!testItemData)
        {
            ULOG(LOG_WARN, "%s(), testItemData", __FUNCTION__);
            return;
        }

        // 1:计算项目不能显示结果详情 2:只能选中一个项目
        if (!CommonInformationManager::IsCalcAssay(testItemData.value().assayCode)
            && onlyOne)
        {
            ui->result_detail_btn->setEnabled(true);
        }

        // 患者信息，只能有一个选中的时候才能使能
        if (onlyOne)
        {
            ui->patient_btn->setEnabled(true);
        }

        // 质控和校准的样本
        auto sampleData = DataPrivate::Instance().GetSampleInfoByDb(testItemData.value().sampleInfoId);
        // 若当前行没有数据
        if (!sampleData)
        {
            return;
        }

		// 1：已审核的样本才能打印
		// 2: 未开启审核功能
		if ((sampleData->__isset.audit && sampleData->audit)
			|| !m_workSet.audit)
		{
			ui->flat_print->setEnabled(true);
		}

		ui->export_btn->setEnabled(true);

        if (sampleData->__isset.sampleType &&
            (sampleData->sampleType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR
                || sampleData->sampleType == tf::SampleType::SAMPLE_TYPE_QC))
        {
            ui->patient_btn->setEnabled(false);
        }

		ui->flat_Ai->setEnabled(true);
        return;
    }

    auto selectData = DataPrivate::Instance().GetAllSelectFlag(0);
    if (selectData.empty())
    {
        return;
    }

	// modify bug0013239 by wuht 
	auto indexCurrent = ui->sample_list->currentIndex();
	if (!indexCurrent.isValid() && m_filterModule->GetFilterEnable())
	{
		ULOG(LOG_WARN, "%s(), indexCurrent.isValid()", __FUNCTION__);
		return;
	}

    bool onlyOne = (selectData.size() == 1) ? true : false;
    auto sampleData = DataPrivate::Instance().GetSampleInfoByDb(*selectData.begin());
    // 若当前行没有数据
    if (sampleData == Q_NULLPTR)
    {
        return;
    }

    if (sampleData->__isset.audit && sampleData->audit)
    {
        ui->flat_examine->setText(tr("取消审核"));
    }
    else
    {
        ui->flat_examine->setText(tr("审核"));
    }

    // 患者信息和结果详情，都只能选中一个的时候才能使能
    if (onlyOne)
    {
        // 病人信息
        ui->patient_btn->setEnabled(true);
        // 结果详情
        ui->result_detail_btn->setEnabled(true);
    }

    // 复查按钮要求必须有测试完成的项目（当没有测试完成的项目的时候，置灰）
    bool hasTested = false; /// 是否已经选择了复查
    bool allHasTested = true;   /// 是否全部测试完成，若是则可以审核
    auto vecItem = DataPrivate::Instance().GetSampleTestItems(sampleData->id);
    for (const auto& item : vecItem)
    {
        // 除开计算项目
        if (tf::AssayCodeRange::CALC_RANGE_MIN <= item->assayCode
            && item->assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
        {
            continue;
        }

		// 最新需求，复选框勾选，只作为选中使用，不作为选择复查
		/*
        if (item->status == tf::TestItemStatus::TEST_ITEM_STATUS_PENDING && item->rerun)
        {
            hasTested = true;
        }*/

        // 项目测试完毕
        if (item->status != tf::TestItemStatus::TEST_ITEM_STATUS_TESTED)
        {
            allHasTested = false;
        }
    }

	// 当项目中有选中的时候，开启复查
	auto testItems = QSampleDetailModel::Instance().GetTestSelectedItems();
	if (testItems.size() > 0)
	{
		hasTested = true;
	}

    // 复查功能要求有被测试的项目，同时只能选中一个
	// 要求必须是相同模式才能复查
	bool isSameMode = (sampleData->testMode == m_sampleTestMode)? true: false;
    if (hasTested && onlyOne && isSameMode)
    {
        ui->flat_recheck->setEnabled(true);
    }

    // 审核功能
    if (allHasTested && !vecItem.empty())
    {
        ui->flat_examine->setEnabled(true);
    }

    // 测试完成状态的样本才能被传输
    if (IsSampleTestStatus(sampleData))
    {
        ui->flat_manual_transfer->setEnabled(true);
    }

	// 1：已审核的样本才能打印
	// 2: 未开启审核功能
	if ((sampleData->__isset.audit && sampleData->audit)
		|| !m_workSet.audit)
	{
		ui->flat_print->setEnabled(true);
	}

    ui->export_btn->setEnabled(true);
    ui->del_Button->setEnabled(true);

    // 质控和校准的样本
    if (sampleData->__isset.sampleType &&
        (sampleData->sampleType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR
            || sampleData->sampleType == tf::SampleType::SAMPLE_TYPE_QC))
    {
        ui->patient_btn->setEnabled(false);
        ui->flat_recheck->setEnabled(false);

    }

    // 只有校准样本不允许传输，质控可以
    if (sampleData->__isset.sampleType &&
        (sampleData->sampleType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR))
    {
        ui->flat_manual_transfer->setEnabled(false);
    }

	ui->flat_Ai->setEnabled(true);
}

///
/// @brief 获取当前样本
///
///
/// @return 当前样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月28日，新建函数
///
boost::optional<tf::SampleInfo> QSampleDataBrowse::GetCurrentSample()
{
    // 获取当前样本
    auto sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
    if (nullptr == sampleProxyMoudel)
    {
        return boost::none;
    }

    auto sample = sampleProxyMoudel->GetSampleByIndex(ui->sample_list->currentIndex());
    if (!sample)
    {
        return boost::none;
    }

    return sample;
}

///
/// @brief 页面允许功能
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月29日，新建函数
///
void QSampleDataBrowse::PageAllowFun()
{
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        ui->flat_recheck->setEnabled(false);
        ui->flat_examine->setEnabled(false);
        ui->flat_manual_transfer->setEnabled(false);
        ui->del_Button->setEnabled(false);
        //ui->flat_del_all->setEnabled(false);
    }
    else
    {
        ui->flat_recheck->setEnabled(true);
        ui->flat_examine->setEnabled(true);
        ui->flat_manual_transfer->setEnabled(true);
        ui->del_Button->setEnabled(true);
        //ui->flat_del_all->setEnabled(true);

        UpdateButtonStatus();
    }
}

///
/// @brief 设置当前位置的信息
///
/// @param[in]  model  model
/// @param[in]  row    行
/// @param[in]  column 列
/// @param[in]  value  值
///
/// @return true 设置正确
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月13日，新建函数
///
bool QSampleDataBrowse::SetModelData(QStandardItemModel* model, int row, int column, QString value)
{
    if (model == Q_NULLPTR)
    {
        return false;
    }

    auto item = model->item(row, column);
    if (item == Q_NULLPTR)
    {
        auto newItem = new QStandardItem(value);
        m_assayShowModel->setItem(row, column, newItem);
    }
    else
    {
        m_assayShowModel->setData(m_assayShowModel->index(row, column), value);
    }

    return true;
}

void QSampleDataBrowse::UpdateItemShowDatas(int  assayCode)
{
    DataPrivate::Instance().UpdateItemShowDatas(assayCode);
}

///
/// @brief 删除对应的样本
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月20日，新建函数
///
void QSampleDataBrowse::OnDelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    auto selectedSampels = DataPrivate::Instance().GetAllSelectFlag(0);
    if (selectedSampels.size() <= 0)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未选中记录信息.")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "have not selected any samples");
        return;
    }

    // 弹框提示是否确认删除
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除"), tr("确定删除选中数据?"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel delete!");
            return;
        }
    }

    auto f = [&](std::set<int64_t>& sampledbs)->bool
    {
        bool deleteAlways = false;
        std::vector<int64_t> dbsVes;
		std::vector<std::shared_ptr<tf::SampleInfo>> sampleVec;
        for (const auto& db : sampledbs)
        {
            // 如果当前行为空，则跳过
            auto sample = DataPrivate::Instance().GetSampleInfoByDb(db);
            if (!sample)
            {
                continue;
            }

            // 存在样本处于测试中（P状态）
            if (sample->status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
            {
                if (sampledbs.size() > 1)
                {
                    // 不需要再次询问
                    if (deleteAlways)
                    {
                        continue;
                    }

                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("选中样本中正在测试的样本不可删除，是否继续删除其他样本？"), TipDlgType::TWO_BUTTON));
                    if (pTipDlg->exec() == QDialog::Rejected)
                    {
                        ULOG(LOG_INFO, "Cancel delete!");
                        return false;
                    }
                    deleteAlways = true;
                }
                else
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("样本正在检测中，不可删除！")));
                    pTipDlg->exec();
                    return false;
                }

                continue;
            }

			sampleVec.push_back(sample);
            dbsVes.push_back(db);
        }

        // 删除选中样本
        if (!DcsControlProxy::GetInstance()->RemoveSampleInfos(dbsVes))
        {
            ULOG(LOG_ERROR, "RemoveSampleInfos failed!");
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除失败.")));
            pTipDlg->exec();
            return false;
        }

		// 依次增加操作日志
		for (const auto& sample : sampleVec)
		{
			WorkpageCommon::DeleteSampleOptLog(*sample);
		}

        return true;
    };

    // 删除选中样本信息
    if (!f(selectedSampels))
    {
        return;
    }

    ResetAssayModel();
    QSampleAssayModel::Instance().SetCurrentSampleRow(-1);
	ui->sample_list->setCurrentIndex(QModelIndex());
	QSampleAssayModel::Instance().ClearSelectedFlag(0);
    // 设置各按钮状态
    UpdateButtonStatus();
}

///
/// @brief 获取单样本的打印数据
///
/// @param[out]  vecSampleDatas  组装好的打印数据
/// @param[in]   QueryCondition   查询条件，类型待定
/// @param[in]   selSamples		选择的样本信息
///
/// @return true 设置正确
///
/// @par History:
/// @li 6889/ChenWei，2023年3月24日，新建函数
///
bool QSampleDataBrowse::GetPrintData(SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds, const std::vector<tf::SampleInfo>& selSamples)
{
	// 构建打印格式的样本信息
	vecSampleDatas.clear();
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	for (const tf::SampleInfo& sample : selSamples)
	{
		// 样本信息祥光
		SampleInfo info;
        info.strPrintTime = strPrintTime.toStdString();        // 打印时间
		auto softType = CommonInformationManager::GetInstance()->GetSoftWareType();
		if (SOFTWARE_TYPE::IMMUNE == softType)
		{
			info.strInstrumentModel = " ";
		}
		else if (SOFTWARE_TYPE::CHEMISTRY == softType)
		{
			info.strInstrumentModel = "C1005";
		}
		else
		{
			info.strInstrumentModel = "";
		}

        //info.strInstrumentModel = std::string(tr("i 6000").toStdString());						// 仪器型号 

		info.strSourceType = ThriftEnumTrans::GetSourceTypeName(sample.sampleSourceType).toStdString();///< 样本类型
		info.strSequenceNO = QDataItemInfo::Instance().GetSeqNo(sample).toStdString();                            ///< 样本号
		info.strSampleID = sample.barcode;                           ///< 样本ID

        auto patientInfo = GetPatientInfo(sample.patientInfoId);

        // 与病人信息相关
        if (patientInfo != Q_NULLPTR)
        {
            // 姓名
            info.strPatientName = patientInfo->name;

            // 性别
			if (patientInfo->gender >= 0)
			{
				info.strGender = ConvertTfEnumToQString(patientInfo->gender).toStdString();
			}

            // 年龄
            if (patientInfo->age > 0)
            {
                info.strAge = std::to_string(patientInfo->age);
                if (patientInfo->ageUnit != -1)
                {
                    info.strAge += " " + ConvertTfEnumToQString(patientInfo->ageUnit).toStdString();
                }
            }

			info.strInspectionPersonnel = patientInfo->tester;           ///< 检测者
            info.strCaseNo = patientInfo->accountNo;                     ///< 病历号
            info.strDepartment = patientInfo->department;                ///< 科室
            info.strDoctor = patientInfo->attendingDoctor;               ///< 医生
            info.strBedNo = patientInfo->bedNo;                          ///< 病床号
            info.strInpatientWard = patientInfo->inpatientArea;			 ///< 病区	
            info.strClinicalDiagnosis = patientInfo->diagnosis;			 ///< 临床诊断		
            if (sample.audit)
            {
                info.strReviewers = patientInfo->auditor;					 ///< 审核人员	
				info.strDateAudit = patientInfo->auditorTime;                ///< 审核时间
            }
        }

		// 项目信息相关
        int iIndex = 0;
        auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(sample.id);
        if (!testItems)
        {
            return false;
        }

        std::vector<std::shared_ptr<tf::TestItem>> vecItem;
        QSampleDetailModel::Instance().SortItem(vecItem, testItems.value());
		for (auto& testitem : vecItem)
		{
			ItemInfo item;

            // 序号
            item.strIndex = QString::number(++iIndex).toStdString();

            // 项目名称
			item.strItemName = CommonInformationManager::GetInstance()->GetPrintAssayName(testitem->assayCode).toStdString();

            // 简称
            std::shared_ptr<tf::GeneralAssayInfo> pGeneralAssay = CommonInformationManager::GetInstance()->GetAssayInfo(testitem->assayCode);
            if(pGeneralAssay != nullptr)
                item.strShortName = pGeneralAssay->assayName;    

			auto spResult = DataPrivate::Instance().GetAssayResultByTestItem(*(testitem.get()));
			if (spResult != Q_NULLPTR )
			{
                // 有复查结果的打印复查结果
                bool bReCheck = false;
                AssayResult Result;
                if ((*spResult)[1].has_value())
                {
                    bReCheck = true;
                    Result = (*spResult)[1];
                    info.strCompTime = sample.endRetestTime;					 ///< 检测时间		
                }
                else if ((*spResult)[0].has_value())
                {
                    bReCheck = false;
                    Result = (*spResult)[0];
                    info.strCompTime = sample.endTestTime;					 ///< 检测时间		
                }

               
                item.strResult = QDataItemInfo::Instance().GetAssayResult(bReCheck, *(testitem.get()), std::make_shared<tf::SampleInfo>(sample), true).toStdString(); ///< 用于打印的结果
				item.strResultState = QDataItemInfo::Instance().GetAssayResultStatus(Result, item.strResult).toStdString();                ///< 用于打印的结果状态（“↓”或者“↑”） 
 
                ///< 结果单位 
                const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();

                // 显示非浓度的情况下，不需要显示单位
                if (WorkpageCommon::IsShowConc(sampleSet, sample.sampleType))
                {
                    item.strUnit = QDataItemInfo::Instance().GetAssayResultUnitName(testitem->assayCode);
                }

				item.strModule = QDataItemInfo::Instance().GetDeivceName(Result).toStdString();                  ///< 设备名称
                QString strRefRange = QDataItemInfo::Instance().GetRefStr(Result, sample.sampleSourceType, patientInfo);
                QStringList RefRangeList = strRefRange.split(QRegExp(";|；"));
                strRefRange = RefRangeList.join(";\n");
				item.strRefRange = strRefRange.toStdString();             ///< 参考区间
			}
            
			info.vecItems.push_back(item);
		}

		vecSampleDatas.push_back(info);
	}
    return true;
}

///
/// @brief 获取用户选择的样本信息
///
/// @param[in]  samples  返回的样本信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年9月22日，新建函数
///
void QSampleDataBrowse::GetSelectedSamples(std::vector<tf::SampleInfo>& samples)
{
	samples.clear();
	// 按样本展示
	if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
	{
		auto selecteds = DataPrivate::Instance().GetAllSelectFlag(0);
		for (const auto& db : selecteds)
		{
			auto spSample = DataPrivate::Instance().GetSampleInfoByDb(db);
			if (spSample != Q_NULLPTR)
			{
                if ((spSample->__isset.audit && spSample->audit)
                    || !m_workSet.audit)
                {
                    samples.push_back(*spSample);
                }
			}
		}
	}
	else // 按项目展示
	{
		auto selecteds = DataPrivate::Instance().GetAllSelectFlag(1);
		for (const auto& testItemdb : selecteds)
		{
			auto testItem = DataPrivate::Instance().GetTestItemInfoByDb(testItemdb);
			if (testItem != Q_NULLPTR)
			{
				auto spSample = DataPrivate::Instance().GetSampleInfoByDb(testItem->sampleInfoId);
				if (spSample != Q_NULLPTR)
				{
					samples.push_back(*spSample);
				}
			}
		}
	}
}

///
/// @brief 根据病人key获取病人详细信息
///
/// @param[in]  db  数据库记录ID
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年9月22日，新建函数
///
std::shared_ptr<tf::PatientInfo> QSampleDataBrowse::GetPatientInfo(const int64_t db)
{
	::tf::PatientInfoQueryCond patienQryCond;
	patienQryCond.__set_id(db);
	::tf::PatientInfoQueryResp patienQryResp;
	// 执行查询条件
	if (!DcsControlProxy::GetInstance()->QueryPatientInfo(patienQryResp, patienQryCond)
		|| patienQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "query patient Info failed!");
		return Q_NULLPTR;
	}
	// 空的时候给一个永远查不到的值
	if (patienQryResp.lstPatientInfos.empty())
	{
		return Q_NULLPTR;
	}
	return std::make_shared<tf::PatientInfo>(patienQryResp.lstPatientInfos[0]);
}

///
/// @brief 获取按样本打印的打印数据
///
/// @param[out]  vecSampleDatas  组装好的打印数据
/// @param[in]   QueryCondition   查询条件，类型待定
/// @param[in]   selSamples		选择的样本信息
///
/// @return true 设置正确
///
/// @par History:
/// @li 6889/ChenWei，2023年3月24日，新建函数
///
bool QSampleDataBrowse::GetPrintDataBySample(SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds, const std::vector<tf::SampleInfo>& selSamples)
{
	// 组装打印格式的样本信息
	vecSampleDatas.clear();
    SimpleSampleInfo info;
	auto softType = CommonInformationManager::GetInstance()->GetSoftWareType();
	if (SOFTWARE_TYPE::IMMUNE == softType)
	{
		info.strInstrumentModel = "";
	}
	else if(SOFTWARE_TYPE::CHEMISTRY == softType)
	{
		info.strInstrumentModel = "C1005";
	}
	else
	{
		info.strInstrumentModel = "";
	}

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    info.strPrintTime = strPrintTime.toStdString();
    std::vector<ItemResult> vecTempItem;
    for (const tf::SampleInfo& sample : selSamples)
    {
        ItemResult item;
        item.strSequenceNO = QDataItemInfo::Instance().GetSeqNo(sample).toStdString();								    // 样本号
        item.strSampleID = sample.barcode;								    // 样本条码
        std::string strCompTime;
        auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(sample.id);
        if (!testItems)
        {
            return false;
        }

        std::vector<std::shared_ptr<tf::TestItem>> vecItem;
        QSampleDetailModel::Instance().SortItem(vecItem, testItems.value());
        for (const auto& testitem : vecItem)
        {
            // 项目名称
            item.strItemName = CommonInformationManager::GetInstance()->GetPrintAssayName(testitem->assayCode).toStdString();								
            auto spResult = DataPrivate::Instance().GetAssayResultByTestItem(*(testitem.get()));
            if (spResult != Q_NULLPTR && (*spResult)[0].has_value())
            {
                // 有复查结果的打印复查结果
                bool bReCheck = false;
                AssayResult Result;
                if ((*spResult)[1].has_value())
                {
                    bReCheck = true;
                    Result = (*spResult)[1];
                }
                else if ((*spResult)[0].has_value())
                {
                    bReCheck = false;
                    Result = (*spResult)[0];
                }

                item.strResult = QDataItemInfo::Instance().GetAssayResult(bReCheck, *(testitem.get()), std::make_shared<tf::SampleInfo>(sample), true).toStdString();///< 用于打印的结果 
                item.strModule = QDataItemInfo::Instance().GetDeivceName(Result).toStdString();                  ///< 设备名称

                // 检测完成时间
                strCompTime = QDataItemInfo::Instance().GetAssayEndTime(Result).toStdString();
            }

            vecTempItem.push_back(std::move(item));
        }

        if (!vecTempItem.empty())
        {
            vecTempItem[0].strCompTime = strCompTime;
            info.vecItemResults.insert(info.vecItemResults.end(), vecTempItem.begin(), vecTempItem.end());
            vecTempItem.clear();
        }

        // 没结果时特殊处理
        if(sample.testItems.size() == 0)
            info.vecItemResults.push_back(std::move(item));
    }
        
    vecSampleDatas.push_back(info);

    return true;
}

///
/// @brief 获取按项目打印的打印数据
///
/// @param[out]  vecSampleDatas  组装好的打印数据
/// @param[in]   QueryCondition   查询条件，类型待定
///
/// @return true 设置正确
///
/// @par History:
/// @li 6889/ChenWei，2023年3月24日，新建函数
///
bool QSampleDataBrowse::GetPrintItemData(ItemSampleInfoVector& vecItemDatas, EM_RESULT_MODE ResultMode)
{
    QTableView* list = Q_NULLPTR;
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        list = ui->sample_list;
    }
    else
    {
        list = ui->show_assay_list;
    }

    auto sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(list->model());
    QItemSelectionModel* slectmodule = list->selectionModel();
    QModelIndexList selectIndexs = slectmodule->selectedRows();
    if (slectmodule == Q_NULLPTR || sampleProxyMoudel == Q_NULLPTR || selectIndexs.count() <= 0)
    {
        return false;
    }

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ItemSampleInfo info;
    info.strPrintTime = strPrintTime.toStdString();
    info.strFactory = std::string(tr("maccura").toStdString());		        						// 试剂厂家

	auto softType = CommonInformationManager::GetInstance()->GetSoftWareType();
	if (SOFTWARE_TYPE::IMMUNE == softType)
	{
		info.strInstrumentModel = " ";
	}
	else if (SOFTWARE_TYPE::CHEMISTRY == softType)
	{
		info.strInstrumentModel = "C1005";
	}
	else
	{
		info.strInstrumentModel = "";
	}

    //info.strInstrumentModel = std::string(tr("c1005").toStdString());	        					// 仪器型号
    info.strTemperature = std::string(tr("    - ℃").toStdString());								// 实验室温度
    info.strHumidness = std::string(tr("    - %RH").toStdString());	    							// 实验室湿度
    //auto selecteds = DataPrivate::Instance().GetAllSelectFlag(1);
    for (const auto& index : selectIndexs)
    {
        auto testItem = sampleProxyMoudel->GetTestItemByIndex(index);
        std::shared_ptr<tf::TestItem> pItem = std::make_shared<tf::TestItem>(testItem.value());
        auto spSample = DataPrivate::Instance().GetSampleInfoByDb(pItem->sampleInfoId);
        if (spSample == Q_NULLPTR || (m_workSet.audit && !(spSample->__isset.audit && spSample->audit)))
        {
            continue;
        }

        ItemSampleResult item;
        bool isCalc = CommonInformationManager::IsCalcAssay(pItem->assayCode);
        if (!isCalc)
        {
            // 将查询到的项目数据组装成打印数据
            auto spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(pItem->deviceSN);
            if (spDevInfo == Q_NULLPTR)
            {
                continue;
            }

            // 根据设备类型填写打印数据
            switch (spDevInfo->deviceClassify)
            {
            case ::tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE:
            {
                // 查询结果表，根据查询创建检测结果和复查结果界面
                ::im::tf::AssayTestResultQueryResp queryAssaycond;
                ::im::tf::AssayTestResultQueryCond assayTestResult;
                assayTestResult.__set_sampleInfoId(pItem->sampleInfoId);
                assayTestResult.__set_testItemId(pItem->id);
                ::im::i6000::LogicControlProxy::QueryAssayTestResult(queryAssaycond, assayTestResult);
                if (queryAssaycond.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS || queryAssaycond.lstAssayTestResult.empty())
                {
                    ULOG(LOG_ERROR, "%s()", __FUNCTION__);
                    continue;
                }

                ::im::tf::AssayTestResult result = queryAssaycond.lstAssayTestResult.back();
                item.strRLU = QString::number(result.RLUEdit).toStdString();         // 信号值
                item.strDetectionTime = result.endTime;                                         // 检测完成时间
            }
            break;
            case ::tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY:
            {
                ch::tf::AssayTestResultQueryCond queryAssaycond;
                ch::tf::AssayTestResultQueryResp assayTestResult;
                // 根据测试项目的ID来查询结果
                queryAssaycond.__set_testItemId(pItem->id);

                if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                    || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                    || assayTestResult.lstAssayTestResult.empty())
                {
                    ULOG(LOG_ERROR, "%s()", __FUNCTION__);
                    continue;
                }

                auto result = assayTestResult.lstAssayTestResult.back();
                item.strRLU = std::to_string(result.conc);												// 信号值
                item.strDetectionTime = result.endTime;                                                 // 检测完成时间
            }
            break;
            case ::tf::AssayClassify::type::ASSAY_CLASSIFY_ISE:
            {
                ise::tf::AssayTestResultQueryCond queryAssaycond;
                ise::tf::AssayTestResultQueryResp assayTestResult;
                // 根据测试项目的ID来查询结果
                queryAssaycond.__set_testItemId(pItem->id);

                if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                    || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                    || assayTestResult.lstAssayTestResult.empty())
                {
                    ULOG(LOG_ERROR, "%s()", __FUNCTION__);
                    continue;
                }

                auto result = assayTestResult.lstAssayTestResult.back();
                item.strRLU = std::to_string(result.conc);												// 信号值
                item.strDetectionTime = result.endTime;                                             // 检测完成时间
            }
            break;
            }
        }

        // 模块名称
        //item.strModule = spDevInfo->groupName + spDevInfo->name;

        // 样本号
        item.strSampleNumber = QDataItemInfo::Instance().GetSeqNo(*spSample).toStdString();// spSample->seqNo;

        // 样本条码
        item.strSampleID = spSample->barcode;

        // 项目名称
        item.strItemName = CommonInformationManager::GetInstance()->GetPrintAssayName(pItem->assayCode).toStdString();
        auto spResult = DataPrivate::Instance().GetAssayResultByTestItem(*pItem);
        if (spResult != Q_NULLPTR)
        {
            // 首次结果
            if ((ResultMode & EM_RESULT_MODE::PRINT_RESULT_MODE_FIRST) && (*spResult)[0].has_value())
            {
                auto firstResult = (*spResult)[0];

                // 结果单位 
                const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();

                // 显示非浓度的情况下，不需要显示单位
                if (WorkpageCommon::IsShowConc(sampleSet, spSample->sampleType))
                {
                    item.strUnit = QDataItemInfo::Instance().GetAssayResultUnitName(pItem->assayCode);
                }

                item.strResult = QDataItemInfo::Instance().GetAssayResult(false, *pItem, spSample, true).toStdString();          ///< 结果
                if (typeid(im::tf::AssayTestResult) == firstResult.value().type() && !isCalc)
                {
                    auto assayResult = boost::any_cast<im::tf::AssayTestResult>(firstResult.value());
                    if (assayResult.RLUEdit >= 0)
                        item.strRLU = QString::number(assayResult.RLUEdit).toStdString();             // 信号值
                }

                // 检测完成时间
                item.strDetectionTime = pItem->endTime;
                info.vecResult.push_back(item);
            }

            // 复查结果
            if ((ResultMode & EM_RESULT_MODE::PRINT_RESULT_MODE_RERUN) && (*spResult)[1].has_value())
            {
                ItemSampleResult ReTestItem = item;
                auto ReTestResult = (*spResult)[1];
                // 结果单位
                const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();

                // 显示非浓度的情况下，不需要显示单位
                if (!WorkpageCommon::IsShowConc(sampleSet, spSample->sampleType))
                {
                    ReTestItem.strUnit = QDataItemInfo::Instance().GetAssayResultUnitName(pItem->assayCode);
                }

                // 复查结果加 "R" 标记
                QString strResult = QDataItemInfo::Instance().GetAssayResult(true, *pItem, spSample, true);
                if (!strResult.isEmpty())
                {
                    ReTestItem.strResult = strResult.toStdString() + std::string(" R");
                    if (typeid(im::tf::AssayTestResult) == ReTestResult.value().type() && !isCalc)
                    {
                        auto assayResult = boost::any_cast<im::tf::AssayTestResult>(ReTestResult.value());
                        if (assayResult.RLUEdit >= 0)
                            ReTestItem.strRLU = QString::number(assayResult.RLUEdit).toStdString();            // 信号值
                    }

                    // 检测完成时间
                    ReTestItem.strDetectionTime = pItem->retestEndTime;
                    info.vecResult.push_back(ReTestItem);
                }
            }
        }
    }

    vecItemDatas.push_back(info);

    return true;
}

///
/// @brief  响应打印按钮
///
/// @return 
///
/// @par History:
/// @li 6889/ChenWei，2023年3月23日，新建函数
///
void QSampleDataBrowse::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 构造数据的查询条件
    QString QueryCondition = " "; // 待完善

	// 获取用户选择的样本信息
	std::vector<tf::SampleInfo> selSamples;
	GetSelectedSamples(selSamples);
	// 检查样本是否可以打印 	0021373: [工作] 数据浏览界面选择请求状态、正在测定状态的样本能进行打印 modify by chenjianlin 20230922
	for (const auto& atSample : selSamples)
	{
		if (atSample.status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING || atSample.status == tf::SampleStatus::type::SAMPLE_STATUS_PENDING )
		{
			TipDlg(tr("提示"), tr("您选择了请求或正在测定的样本，请重新选择或稍后再试。")).exec();
			return;
		}
	}

    // 根据按项目或按样本展示来设置对话框模式
    PrintSetDlg::PrintMode mode = PrintSetDlg::UNKNOMN;
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
        mode = PrintSetDlg::PRINTBYSAMPLE;
    else if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
        mode = PrintSetDlg::PRINTBYITEM;

	// 设置回调函数
    if (m_pPrintSetDlg == nullptr)
    {
        m_pPrintSetDlg = std::shared_ptr<PrintSetDlg>(new PrintSetDlg(false, this));
    }
    m_pPrintSetDlg->SetModel(mode);
    m_pPrintSetDlg->SetSampleGetFun([=](SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)->bool {
        return GetPrintData(vecSampleDatas, sampIds, selSamples);
    });

    m_pPrintSetDlg->SetSimpleSampleGetFun([=](SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)->bool {
        return GetPrintDataBySample(vecSampleDatas, sampIds, selSamples);
    });

    m_pPrintSetDlg->SetItemDataGetFun([&](ItemSampleInfoVector& vecSampleDatas)->bool {
        EM_RESULT_MODE ResultMode = (EM_RESULT_MODE)m_pPrintSetDlg->GetResultType(); // 结果类型
        return GetPrintItemData(vecSampleDatas, ResultMode);
    });

	// 显示对话框
	if (QDialog::Rejected == m_pPrintSetDlg->exec() || !m_pPrintSetDlg->IsPrintFinished())
	{
		return;
	}

	for (const auto& atSample : selSamples)
	{
		tf::SampleInfo modifySample;
		modifySample.__set_id(atSample.id);
		modifySample.__set_printed(true);
		// 修改样本为已打印状态
		DcsControlProxy::GetInstance()->ModifySampleInfo(modifySample);
		auto samples = { atSample.id };
		DcsControlProxy::GetInstance()->UpdateHistoryBaseDataPrint(samples, true);
		//QHistorySampleAssayModel::Instance().UpdatePrintFlag(samples, true);
	}
}

void QSampleDataBrowse::PrintSample()
{
    // 若没有免疫设备，则采用生化的打印策略
    auto device = CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_I6000 });
    if (device.empty())
    {
        std::set<int64_t>&& sampleIds = DataPrivate::Instance().GetAllSelectFlag(0);
        for (auto& db : sampleIds)
        {
            // 获取样本信息
            auto spSample = DataPrivate::Instance().GetSampleInfoByDb(db);
            if (!spSample)
            {
                continue;
            }
            // 获取testItemId信息
            auto vecItem = DataPrivate::Instance().GetSampleTestItems(spSample->id);

            for (auto& item : vecItem)
            {
                auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item->assayCode);
                if (spAssayInfo == nullptr)
                    continue;
                auto spSpecialInfo = CommonInformationManager::GetInstance()->GetChemistrySpecialAssayInfo(item->assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000, spSample->sampleSourceType);
                if (spSpecialInfo.empty())
                    continue;

                auto spPatient = GetPatientInfoById(spSample->patientInfoId);
                if (spPatient)
                {
                    // =========================页眉   以下=========================
                    // 姓名
                    spPatient->name;
                    // 性别
                    ThriftEnumTrans::GetGender(spPatient->gender, " ");
                    // 年龄
                    ThriftEnumTrans::GetAge(spPatient);
                    // 样本类型
                    ThriftEnumTrans::GetSourceTypeName(spSample->sampleSourceType, " ");
                    // 病历号
                    spPatient->medicalRecordNo;
                    // 样本号
                    spSample->seqNo;
                    // 样本条码
                    spSample->barcode;
                    // 送检科室
                    spPatient->department;
                    // 送检医生
                    spPatient->doctor;
                    // 临床诊断
                    spPatient->diagnosis;
                    // 主治医生
                    // spPatient->attendingDoctor;
                    // 备注
                    spPatient->comment;
                    // =========================页眉   以上=========================

                    // =========================内容   以下=========================
                    // 项目名称
                    spAssayInfo->assayName;
                    // 中文名称
                    spAssayInfo->printName;
                    // 结果
                    if (item->retestEditConc)
                        item->editConc;
                    else if (item->lastTestResultKey.assayTestResultId > 0)
                        item->retestConc;
                    else if (item->firstTestResultKey.assayTestResultId > 0)
                        item->conc;
                    // 单位
                    CommonInformationManager::GetInstance()->GetCurrentUnit(item->assayCode);
                    // 参考范围
                    bool found = false;
                    tf::AssayReferenceItem defaultReferenceItem;
                    for (auto& r : spSpecialInfo[0]->referenceRanges.Items)
                    {
                        // 样本类型
                        if (r.sampleSourceType == spSample->sampleSourceType &&
                            spPatient->gender == r.enGender &&
                            spPatient->ageUnit == r.enAgeUnit &&
                            spPatient->age >= r.iLowerAge &&
                            spPatient->age <= r.iUpperAge)
                        {
                            // 参考低值
                            r.fLowerRefRang;
                            // 参考高值
                            r.fUpperRefRang;
                            r.strRefRange;
                            found = true;
                            break;
                        }
                        else if (r.bAutoDefault && r.sampleSourceType == spSample->sampleSourceType)
                        {
                            defaultReferenceItem = r;
                        }
                    }
                    if (!found)
                        defaultReferenceItem.strRefRange;
                    // =========================内容   以上=========================

                    // =========================页脚   以下=========================
                    // 采样日期
                    spPatient->samplingTime;
                    // 送检日期
                    spPatient->inspectTime;
                    // 检验日期
                    spPatient->testTime;
                    // 打印时间
                    QDateTime::currentDateTime();
                    // 检验者
                    spPatient->tester;
                    // 审核者
                    spPatient->auditor;
                    // =========================页脚   以上=========================
                }
            }
        }
    }
}

void QSampleDataBrowse::PrintItem()
{
    // 若没有免疫设备，则采用生化的打印策略
    auto device = CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_I6000 });
    if (device.empty())
    {
        std::set<int64_t>&& itemIds = DataPrivate::Instance().GetAllSelectFlag(1);
        for (const auto& db : itemIds)
        {
            // 如果当前行为空，则跳过
            auto item = DataPrivate::Instance().GetTestItemInfoByDb(db);
            if (!item)
            {
                continue;
            }
            // 获取样本信息
            auto spSample = DataPrivate::Instance().GetSampleInfoByDb(item->sampleInfoId);
            if (!spSample)
            {
                continue;
            }

            auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item->assayCode);
            if (spAssayInfo == nullptr)
                continue;
            auto spSpecialInfo = CommonInformationManager::GetInstance()->GetChemistrySpecialAssayInfo(item->assayCode, ::tf::DeviceType::DEVICE_TYPE_C1000, spSample->sampleSourceType);
            if (spSpecialInfo.empty())
                continue;

            auto spPatient = GetPatientInfoById(spSample->patientInfoId);
            if (spPatient)
            {
                // =========================页眉   以下=========================
                // 姓名
                spPatient->name;
                // 性别
                ThriftEnumTrans::GetGender(spPatient->gender, " ");
                // 年龄
                ThriftEnumTrans::GetAge(spPatient);
                // 样本类型
                ThriftEnumTrans::GetSourceTypeName(spSample->sampleSourceType, " ");
                // 病历号
                spPatient->medicalRecordNo;
                // 样本号
                spSample->seqNo;
                // 样本条码
                spSample->barcode;
                // 送检科室
                spPatient->department;
                // 送检医生
                spPatient->doctor;
                // 临床诊断
                spPatient->diagnosis;
                // 主治医生
                // spPatient->attendingDoctor;
                // 备注
                spPatient->comment;
                // =========================页眉   以上=========================

                // =========================内容   以下=========================
                // 项目名称
                spAssayInfo->assayName;
                // 中文名称
                spAssayInfo->printName;
                // 结果
                if (item->retestEditConc)
                    item->editConc;
                else if (item->lastTestResultKey.assayTestResultId > 0)
                    item->retestConc;
                else if (item->firstTestResultKey.assayTestResultId > 0)
                    item->conc;
                // 单位
                CommonInformationManager::GetInstance()->GetCurrentUnit(item->assayCode);
                // 参考范围
                bool found = false;
                tf::AssayReferenceItem defaultReferenceItem;
                for (auto& r : spSpecialInfo[0]->referenceRanges.Items)
                {
                    // 样本类型
                    if (r.sampleSourceType == spSample->sampleSourceType &&
                        spPatient->gender == r.enGender &&
                        spPatient->ageUnit == r.enAgeUnit &&
                        spPatient->age >= r.iLowerAge &&
                        spPatient->age <= r.iUpperAge)
                    {
                        // 参考低值
                        r.fLowerRefRang;
                        // 参考高值
                        r.fUpperRefRang;
                        r.strRefRange;
                        found = true;
                        break;
                    }
                    else if (r.bAutoDefault && r.sampleSourceType == spSample->sampleSourceType)
                    {
                        defaultReferenceItem = r;
                    }
                }
                if (!found)
                    defaultReferenceItem.strRefRange;
                // =========================内容   以上=========================

                // =========================页脚   以下=========================
                // 采样日期
                spPatient->samplingTime;
                // 送检日期
                spPatient->inspectTime;
                // 检验日期
                spPatient->testTime;
                // 打印时间
                QDateTime::currentDateTime();
                // 检验者
                spPatient->tester;
                // 审核者
                spPatient->auditor;
                // =========================页脚   以上=========================
            }
        }
    }
}

///
/// @brief
///     数据导出按钮被选中
///
/// @par History:
/// @li 5774/WuHongTao，2022年4月6日，新建函数
///
void QSampleDataBrowse::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "ExportData %s()", __FUNCTION__);

    switch (QSampleAssayModel::Instance().GetModule())
    {
    case QSampleAssayModel::VIEWMOUDLE::DATABROWSE:
        ExportSample();
        break;
    case QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
        ExPortItem();
        break;
    default:
        break;
    }

    ULOG(LOG_INFO, "ExportData", __FUNCTION__);
}

void QSampleDataBrowse::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    OnUpdateButtonStatus();
}

std::shared_ptr<ch::tf::AssayTestResult> QSampleDataBrowse::GetChAssayTestResult(int64_t id)
{

    if (id < 0)
        return nullptr;

    ch::tf::AssayTestResultQueryCond queryAssaycond;
    ch::tf::AssayTestResultQueryResp assayTestResult;
    // 根据测试项目的ID来查询结果
    queryAssaycond.__set_id(id);

    if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
    }
    else
        return std::make_shared<ch::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);

    return nullptr;
}

std::shared_ptr<ise::tf::AssayTestResult> QSampleDataBrowse::GetIseAssayTestResult(int64_t id)
{
    if (id < 0)
        return nullptr;

    ise::tf::AssayTestResultQueryCond queryAssaycond;
    ise::tf::AssayTestResultQueryResp assayTestResult;
    // 根据测试项目的ID来查询结果
    queryAssaycond.__set_id(id);

    if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
    }
    else
        return std::make_shared<ise::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);

    return nullptr;
}

std::shared_ptr<tf::PatientInfo> QSampleDataBrowse::GetPatientInfoById(int64_t id)
{
    if (id <= 0)
        return nullptr;

    tf::PatientInfoQueryCond qryCond;
    tf::PatientInfoQueryResp qryResp;
    qryCond.__set_id(id);

    if (!DcsControlProxy::GetInstance()->QueryPatientInfo(qryResp, qryCond)
        || qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstPatientInfos.empty())
        return nullptr;
    else
        return std::make_shared<tf::PatientInfo>(qryResp.lstPatientInfos[0]);

    return nullptr;
}

///
///  @brief 按样本导出
///
///
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
bool QSampleDataBrowse::ExportSample()
{
	ULOG(LOG_INFO, __FUNCTION__);
	std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (pLoginUserInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't get UserInfo.");
		return false;
	}

	if (pLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)   // 普通用户不能导出
	{
		return false;
	}

    // 若没有免疫设备，则采用生化的导出策略
    //auto device = CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_I6000 });
	auto softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();
	// 生免联机
    if (softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
    {
        // 	bug0020858 add by ldx 20230913
		QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
		if (!fileName.isNull())
		{
			if (fileName.isEmpty())
			{
				std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("文件名为空！")));
				pTipDlg->exec();
				return false;
			}
		}
		else
		{
			return false;
		}

		auto selecteds = DataPrivate::Instance().GetAllSelectFlag(0);
		if (selecteds.empty())
		{
			return false;
		}

		// 在线程里面依次导出生化和免疫的信息
		auto exportAll = [this](QString fileName, int type, std::set<int64_t> selecteds)->void
		{
			// 耗时操作放在线程里完成（生化）
			QFileInfo FileInfo(fileName);
			QString strSuffix = FileInfo.suffix();
			QString path = FileInfo.absolutePath() + "/";
			QString fileNameCh = path + FileInfo.completeBaseName() + "_CH." + strSuffix;
			ExportSampleData(tf::UserType::type(type), selecteds, fileNameCh);

			// 耗时操作放在线程里完成(免疫)
			QString fileNameIM = path + FileInfo.completeBaseName() + "_IM." + strSuffix;
			ExportData_im(fileNameIM, tf::UserType::type(type), selecteds);
		};

		std::thread thExport(exportAll, fileName, pLoginUserInfo->type, selecteds);
		thExport.detach();
		return true;
    }
	// 免疫
	else if(softwareType == SOFTWARE_TYPE::IMMUNE)
	{
		// 导出免疫数据
		return ExportSample_im(pLoginUserInfo->type);
	}
	// 生化
	else
	{
		// 	bug0020858 add by ldx 20230913
		QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
		if (!fileName.isNull())
		{
			if (fileName.isEmpty())
			{
				std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("文件名为空！")));
				pTipDlg->exec();
				return false;
			}
		}
		else
		{
			return false;
		}

		auto selecteds = DataPrivate::Instance().GetAllSelectFlag(0);
		if (selecteds.empty())
		{
			return false;
		}

		// 耗时操作放在线程里完成
		std::thread thCh(std::bind(&QSampleDataBrowse::ExportSampleData, this, pLoginUserInfo->type, selecteds, fileName));
		thCh.detach();
		return true;
	}
}

bool QSampleDataBrowse::ExportSample_im(tf::UserType::type UType)
{
    // 	bug0020858 add by ldx 20230913
    // 弹出保存文件对话框
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (!fileName.isNull())
    {
        if (fileName.isEmpty())
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("文件名为空！")));
            pTipDlg->exec();
            return false;
        }
    }
    else
    {
        return false;
    }

    auto selecteds = DataPrivate::Instance().GetAllSelectFlag(0);
    if (selecteds.empty())
    {
        return false;
    }

    // 耗时操作放在线程里完成
    std::thread th(std::bind(&QSampleDataBrowse::ExportData_im, this, fileName, UType, selecteds));
    th.detach();

    return true;
}

bool QSampleDataBrowse::ExportData_im(QString fileName, tf::UserType::type UType, std::set<int64_t>& vecSampleFlag)
{
    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    ULOG(LOG_INFO, "ExportData open progress");
    ExpSampleInfoVector vecSamples;
    for (const auto& db : vecSampleFlag)
    {
        // 如果当前行为空，则跳过
        std::shared_ptr<tf::SampleInfo> pSample = DataPrivate::Instance().GetSampleInfoByDb(db);
        if (pSample == nullptr)
        {
            continue;
        }

        // 样本信息
        SampleExportInfo SampleInfo;
        SampleInfo.strSampleNo = QDataItemInfo::Instance().GetSeqNo(*pSample);
        SampleInfo.strBarcode = QString::fromStdString(pSample->barcode);
        SampleInfo.strSampleSourceType = ConvertTfEnumToQString((::tf::SampleSourceType::type)pSample->sampleSourceType);
        SampleInfo.strPos = QDataItemInfo::Instance().GetPos(*pSample);

        // 项目信息
        QMap<int, int> mapItemCounts;           // 重复次数
        QMap<int, int> mapReviewItemCounts;     // 复查重复次数
        auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(pSample->id);
        if (!testItems)
        {
            return false;
        }

        std::vector<std::shared_ptr<tf::TestItem>> vecItem;
        QSampleDetailModel::Instance().SortItem(vecItem, testItems.value());
        for (auto it = vecItem.begin(); it != vecItem.end(); ++it)
        {
			if (*it == nullptr)
			{
				continue;
			}

            bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay((*it)->assayCode);
			// 是否含有免疫项目，对于联机版免疫页面导出，当含有免疫项目的时候，才导出对应的计算项目
			if (isCalc)
			{
				bool isHaveIm = CommonInformationManager::GetInstance()->IsCaClHaveChorIM((*it)->assayCode, false);
				if (!isHaveIm)
				{
					continue;
				}
			}

			auto spGenneral = CommonInformationManager::GetInstance()->GetAssayInfo((*it)->assayCode);
			if (spGenneral == nullptr && !isCalc)
			{
				continue;
			}

			// 只导出免疫项目(0013739)
			if (!(isCalc || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
			{
				continue;
			}

            auto pResults = DataPrivate::Instance().GetAssayResultByTestItem((**it));

            // 首次检测结果
            auto firstResult = (*pResults)[0];
            im::tf::AssayTestResult FirstTestResult;
            if (firstResult.has_value() && typeid(im::tf::AssayTestResult) == firstResult.value().type())
            {
                FirstTestResult = boost::any_cast<im::tf::AssayTestResult>(firstResult.value());
            }

            auto ReTestResult = (*pResults)[1];
            im::tf::AssayTestResult LastTestResult;
            if (ReTestResult.has_value() && typeid(im::tf::AssayTestResult) == ReTestResult.value().type())
            {
                LastTestResult = boost::any_cast<im::tf::AssayTestResult>(ReTestResult.value());
            }

            // 统计项目重复次数
            auto item = mapItemCounts.find((**it).assayCode);
            if (item == mapItemCounts.end())
            {
                // 记录一次重复
                mapItemCounts[(**it).assayCode] = 1;
            }
            else
            {
                item.value() += 1;
            }

            // 项目重复次数
            SampleInfo.strFirstTestCounts = QString::number(mapItemCounts[(**it).assayCode]);
            if ((**it).__isset.retestConc)
            {
                // 统计项目重复次数
                auto LastTestItem = mapReviewItemCounts.find((**it).assayCode);
                if (LastTestItem == mapReviewItemCounts.end())
                {
                    // 记录一次重复
                    mapReviewItemCounts[(**it).assayCode] = 1;
                }
                else
                {
                    LastTestItem.value() += 1;
                }
            }

            // 复查项目重复次数
            auto LastTestItem = mapReviewItemCounts.find((**it).assayCode);
            if (LastTestItem != mapReviewItemCounts.end())
            {
                SampleInfo.strReTestCounts = QString::number(LastTestItem.value());
            }

            MakeExportItemResult(SampleInfo, pSample, *it, pResults);
            vecSamples.push_back(std::move(SampleInfo));
        }
    }

    // 导出文件
    bool bRet = false;
    FileExporter fileExporter;
    QFileInfo FileInfo(fileName);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        SampleExportModule info;
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        info.strExportTime = strPrintTime.toStdString();
        fileExporter.ExportSampleInfoToPdf(vecSamples, info);
        std::string strInfo = GetJsonString(info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportSampleInfo.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRet = true;
    }
    else
    {
        bRet = fileExporter.ExportSampleInfo(vecSamples, fileName, UType);
    }

    ULOG(LOG_INFO, "ExportData close progress");
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条

    // 弹框提示导出失败
    //std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")));
    //pTipDlg->exec();

    return bRet;
}

bool QSampleDataBrowse::ConvertData(const DATATYPE dstType, const QString& input, QVariant& dst)
{
	bool isOk = false;
	if (dstType == DATATYPE::INTTYPE)
	{
		dst = input.toInt(&isOk);
	}
	else if(dstType == DATATYPE::DOUBLETYPE)
	{
		dst = input.toDouble(&isOk);
	}

	return isOk;
}

bool QSampleDataBrowse::GenTitleList(const tf::UserType::type UType, QVariantList& updateTitle, const std::set<ExportInfoEn>& configData)
{
	updateTitle.clear();
	for (auto iter : configData)
	{
		// 需要排除吸光度，放在数据最后
		if (iter == ExportInfoEn::EIE_PRIMARY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_DEPUTY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_ALL_WAVE_ABS)
		{
			continue;
		}

		auto pSEInfo = SEMetaInfos::GetInstance()->GetMetaInfo(iter);
		if (pSEInfo != nullptr && pSEInfo->m_UserType <= UType)
		{
			updateTitle << (pSEInfo->m_strName + "\t");
		}
	}

	return true;
}

bool QSampleDataBrowse::ExportSampleData(const tf::UserType::type UType, const std::set<int64_t>& sampleIds, const QString & path)
{
	// 打开进度条
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    // 生化常规、急诊、质控导出表头(每个波长吸光度需要动态添加)
    QVariantList title_ch_normal ;
    // 生化常规、急诊、质控 动态波长的表头
    QVariantList wareLenTitle;
    // 生化校准导出表头(每个波长吸光度需要动态添加)
    QVariantList title_ch_cali ;
    // 生化校准 动态波长的表头
    QVariantList wareLenTitle_cali;
    // ise导出表头（常规、急诊、质控）
    QVariantList title_ise_normal ;
    // ise生化校准导出表头
    QVariantList title_ise_cali;
    // K:样本主键  
    //V:样本中包含哪些校准项目以及测试当前测试次数（K:项目编号，V:当前累计测试次数）
    std::map<int64_t, std::map<int,int>> caliSampleId;

    // 每一行的数据(每一个string为一行数据，‘\t’分隔)
    QVariantList content_ch;
    QVariantList content_ch_cali;
    QVariantList content_ise;
    QVariantList content_ise_cali;

	std::map<ExportType, std::set<ExportInfoEn>> savedata;
	if (!DictionaryQueryManager::GetExportConfig(savedata))
	{
		ULOG(LOG_ERROR, "Failed to get export config.");
		return false;
	}

	for (const auto& data : savedata)
	{
		if (data.first == ExportType::CH_NORMAL_AND_QC_TYPE)
		{
			GenTitleList(UType, title_ch_normal, data.second);
		}

		if (data.first == ExportType::CH_CALI_TYPE)
		{
			GenTitleList(UType, title_ch_cali, data.second);
		}

		if (data.first == ExportType::CH_ISE_SAM_AND_QC_TYPE)
		{
			GenTitleList(UType, title_ise_normal, data.second);
		}

		if (data.first == ExportType::CH_ISE_CALI_TYPE)
		{
			GenTitleList(UType, title_ise_cali, data.second);
		}
	}

	bool isXlsx = false;
	// 判断是否导出为excel表，否则作为文本文件写入数据
	QFileInfo FileInfo(path);
	QString strSuffix = FileInfo.suffix();
	// 导出为xlsx表格
	if (strSuffix == "xlsx")
	{
		isXlsx = true;
	}

    // 获取样本属于第几次校准
    auto timesFunc = [&](int64_t sampleId, int assayCode)->int {
        // 没有导出过的校准样本 或者
        // 如果这个样本的这个项目没有导出过，那么就是第一次
        if (caliSampleId.find(sampleId) == caliSampleId.end() ||
            caliSampleId[sampleId].find(assayCode) == caliSampleId[sampleId].end())
        {
            caliSampleId[sampleId][assayCode] = 1;
        }
        else
            caliSampleId[sampleId][assayCode] += 1;

        return caliSampleId[sampleId][assayCode];
    };

	ExpSampleInfoVector vecSamples;
    for (auto& db : sampleIds)
    {
        // 获取样本信息
        auto spSample = DataPrivate::Instance().GetSampleInfoByDb(db);
        if (!spSample)
        {
            continue;
        }

		SampleExportInfo sampleExportBase;
		auto isGet = GetSampleExport(spSample, sampleExportBase);
		if (!isGet)
		{
			continue;
		}

        // 获取testItemId信息
        auto vecItem = DataPrivate::Instance().GetSampleTestItems(spSample->id);
        for (auto& item : vecItem)
        {
            bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(item->assayCode);
            auto spGenneral = CommonInformationManager::GetInstance()->GetAssayInfo(item->assayCode);
            // 不是计算项目，通用项目中也找不到
            if (spGenneral == nullptr && !isCalc)
            {
                continue;
            }

            // 只导出生化项目(0013739)
            if (!(isCalc || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
                || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE))
            {
                continue;
            }

			SampleExportInfo sampleExport = sampleExportBase;
			auto isGet = GetTestItemExport(item, sampleExport);
			if (!isGet)
			{
				continue;
			}

            if (item->assayCode >= tf::AssayCodeRange::CH_RANGE_MIN && item->assayCode <= tf::AssayCodeRange::CH_RANGE_MAX)
            {
                // ise项目
                if (item->assayCode >= ise::tf::g_ise_constants.ASSAY_CODE_NA && item->assayCode <= ise::tf::g_ise_constants.ASSAY_CODE_CL)
                {
                    // 常规、急诊、质控的导出
                    if (spSample->sampleType != tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
                    {	
						auto setInfoIseIter = savedata.find(ExportType::CH_ISE_SAM_AND_QC_TYPE);
						if (setInfoIseIter == savedata.end())
						{
							continue;
						}

						if (item->firstTestResultKey.assayTestResultId > 0)
						{
							auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
							if (!isGet)
							{
								continue;
							}

							auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIseIter->second);
							content_ise.push_back(rawData);
							// for pdf
							vecSamples.push_back(sampleExport);
						}

						if (item->lastTestResultKey.assayTestResultId > 0)
						{
							auto isGet = GetResultExport(item->lastTestResultKey, item->assayCode, sampleExport);
							if (!isGet)
							{
								continue;
							}

							auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIseIter->second);
							content_ise.push_back(rawData);
							// for pdf
							vecSamples.push_back(sampleExport);
						}
                    }
                    // 导出校准项目
                    else
                    {
						// 校准项目，需要记录重复多少次
						sampleExport.strCaliCount = QString::number(timesFunc(item->sampleInfoId, item->assayCode));
						auto setInfoIseIter = savedata.find(ExportType::CH_ISE_CALI_TYPE);
						if (setInfoIseIter == savedata.end())
						{
							continue;
						}

						auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
						if (!isGet)
						{
							continue;
						}

						auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIseIter->second);
						content_ise_cali.push_back(rawData);
						// for pdf
						vecSamples.push_back(sampleExport);
                    }
                }
                // 普通生化项目
                else
                {
                    // 常规、急诊、质控的导出
                    if (spSample->sampleType != tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
                    {			
						auto setInfoIter = savedata.find(ExportType::CH_NORMAL_AND_QC_TYPE);
						if (setInfoIter == savedata.end())
						{
							continue;
						}

						QVariantList tmpTitleFirst;
						if (item->firstTestResultKey.assayTestResultId > 0)
						{
							auto spAssayTestResult = GetAssayResultByid(item->firstTestResultKey.assayTestResultId);
							if (spAssayTestResult == Q_NULLPTR)
							{
								continue;
							}

							auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
							if (!isGet)
							{
								continue;
							}

							isGet = GetResultAbs(spAssayTestResult, sampleExport, tmpTitleFirst);
							if (!isGet)
							{
								continue;
							}

							auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIter->second);
							content_ch.push_back(rawData);
							// for pdf
							vecSamples.push_back(sampleExport);
						}

						QVariantList tmpTitleSecond;
						if (item->lastTestResultKey.assayTestResultId > 0)
						{
							auto spAssayTestResult = GetAssayResultByid(item->lastTestResultKey.assayTestResultId);
							if (spAssayTestResult == Q_NULLPTR)
							{
								continue;
							}

							auto isGet = GetResultExport(item->lastTestResultKey, item->assayCode, sampleExport);
							if (!isGet)
							{
								continue;
							}

							isGet = GetResultAbs(spAssayTestResult, sampleExport, tmpTitleSecond);
							if (!isGet)
							{
								continue;
							}

							auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIter->second);
							content_ch.push_back(rawData);
							// for pdf
							vecSamples.push_back(sampleExport);
						}

						if (tmpTitleFirst.empty())
						{
							continue;
						}

						int dataCount = 0;
						for (const auto& data : content_ch)
						{
							if (!data.canConvert<QVariantList>())
							{
								continue;
							}

							auto dataList = data.toList();
							dataCount += dataList.size();
						}

						int titleCount = 0;
						for (const auto& data : wareLenTitle)
						{
							if (!data.canConvert<QVariantList>())
							{
								continue;
							}

							auto dataList = data.toList();
							titleCount += dataList.size();
						}

						if (dataCount < titleCount)
						{
							continue;
						}

						if (tmpTitleFirst.size() < wareLenTitle.size())
						{
							continue;
						}

						wareLenTitle.clear();
						auto tmpTitles = tmpTitleFirst;
						// 主次吸光度只要显示其中一个就要求显示空白
						auto configData = setInfoIter->second;
						if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
							|| configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0)
						{
							wareLenTitle << tmpTitles[0];
						}

						// 要求显示主吸光度
						if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
							&& tmpTitles.size() >= 2)
						{
							wareLenTitle << tmpTitles[1];
						}

						// 要求显示次吸光度
						if (configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0
							&& tmpTitles.size() >= 3)
						{
							wareLenTitle << tmpTitles[2];
						}

						// 要求显示所有吸光度
						if (configData.count(ExportInfoEn::EIE_ALL_WAVE_ABS) > 0
							&& tmpTitles.size() >= 4)
						{
							int index = 0;
							for (const auto& titles : tmpTitles)
							{
								if (index++ < 3)
								{
									continue;
								}
								wareLenTitle << titles;
							}
						}
                    }
                    // 导出校准项目
                    else
                    {
						// 校准项目，需要记录重复多少次
						sampleExport.strCaliCount = QString::number(timesFunc(item->sampleInfoId, item->assayCode));
						QVariantList tmpTitles;
						auto setInfoIter = savedata.find(ExportType::CH_CALI_TYPE);
						if (setInfoIter == savedata.end())
						{
							continue;
						}

						auto spAssayTestResult = GetAssayResultByid(item->firstTestResultKey.assayTestResultId);
						if (spAssayTestResult == Q_NULLPTR)
						{
							continue;
						}

						auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
						if (!isGet)
						{
							continue;
						}

						isGet = GetResultAbs(spAssayTestResult, sampleExport, tmpTitles);
						if (!isGet)
						{
							continue;
						}

						auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIter->second);
						content_ch_cali.push_back(rawData);
						// for pdf
						vecSamples.push_back(sampleExport);

						if (tmpTitles.empty())
						{
							continue;
						}

						int dataCount = 0;
						for (const auto& data : content_ch_cali)
						{
							if (!data.canConvert<QVariantList>())
							{
								continue;
							}

							auto dataList = data.toList();
							dataCount += dataList.size();
						}

						int titleCount = 0;
						for (const auto& data : wareLenTitle_cali)
						{
							if (!data.canConvert<QVariantList>())
							{
								continue;
							}

							auto dataList = data.toList();
							titleCount += dataList.size();
						}

						if (dataCount < titleCount)
						{
							continue;
						}

						if (tmpTitles.size() < wareLenTitle_cali.size())
						{
							continue;
						}

						wareLenTitle_cali.clear();
						// 主次吸光度只要显示其中一个就要求显示空白
						auto configData = setInfoIter->second;
						if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
							|| configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0)
						{
							wareLenTitle_cali << tmpTitles[0];
						}

						if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
							&& tmpTitles.size() >= 2)
						{
							wareLenTitle_cali << tmpTitles[1];
						}

						// 要求显示次吸光度
						if (configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0
							&& tmpTitles.size() >= 3)
						{
							wareLenTitle_cali << tmpTitles[2];
						}

						// 要求显示所有吸光度
						if (configData.count(ExportInfoEn::EIE_ALL_WAVE_ABS) > 0
							&& tmpTitles.size() >= 4)
						{
							int index = 0;
							for (const auto& titles : tmpTitles)
							{
								if (index++ < 3)
								{
									continue;
								}
								wareLenTitle_cali << titles;
							}
						}
                    }
                }
            }
            // 计算项目
            else if (item->assayCode >= tf::AssayCodeRange::CALC_RANGE_MIN && item->assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
            {
				// 计算项目暂时显示在生化的列表中
				auto iter = savedata.find(ExportType::CH_NORMAL_AND_QC_TYPE);
				if (iter == savedata.end())
				{
					continue;
				}

				// 是否含有生化项目，对于联机版生化页面导出，当含有生化项目的时候，才导出对应的计算项目
				bool isHaveCh = CommonInformationManager::GetInstance()->IsCaClHaveChorIM(item->assayCode);
				if (!isHaveCh)
				{
					continue;
				}

				if (item->firstTestResultKey.assayTestResultId > 0)
				{
					auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
					if (!isGet)
					{
						continue;
					}

					auto rawData = GetExportData(isXlsx, UType, sampleExport, iter->second);
					content_ch.push_back(rawData);
					// for pdf
					vecSamples.push_back(sampleExport);
				}

				if (item->lastTestResultKey.assayTestResultId > 0)
				{
					auto isGet = GetResultExport(item->lastTestResultKey, item->assayCode, sampleExport);
					if (!isGet)
					{
						continue;
					}

					auto rawData = GetExportData(isXlsx, UType, sampleExport, iter->second);
					content_ch.push_back(rawData);
					// for pdf
					vecSamples.push_back(sampleExport);
				}
            }
        }
    }

	for (const auto& title : wareLenTitle)
	{
		for (const auto& data : title.toList())
		{
			title_ch_normal << (data.toString() + "\t");
		}
	}

	for (const auto& title : wareLenTitle_cali)
	{
		for (const auto& data : title.toList())
		{
			title_ch_cali << (data.toString() + "\t");
		}
	}

	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 30);
    // 导出文件
    FileExporter fileExporter;
    auto& classify = fileExporter.GetDataClassify();
    classify.clear();
    // 要输出的信息（每一个QString为一行数据，列之间的数据用'\t'分割）
    QVariantList strExportTextList_ch;
    QVariantList strExportTextList_ch_cali;
    QVariantList strExportTextList_ise;
    QVariantList strExportTextList_ise_cali;
    if (!content_ch.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("生化_常规"));
        exTask.title = std::move(title_ch_normal);
        exTask.contents = std::move(content_ch);
        classify.append(exTask);
    }
    if (!content_ch_cali.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("生化_校准"));
        exTask.title = std::move(title_ch_cali);
        exTask.contents = std::move(content_ch_cali);
        classify.append(exTask);
    }
    if (!content_ise.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("电解质_常规"));
        exTask.title = std::move(title_ise_normal);
        exTask.contents = std::move(content_ise);
        classify.append(exTask);
    }
    if (!content_ise_cali.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("电解质_校准"));
        exTask.title = std::move(title_ise_cali);
        exTask.contents = std::move(content_ise_cali);
        classify.append(exTask);
    }

	fileExporter.ExportInfoToFileByClassify(path, classify, vecSamples);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE, false);
	return  true;
}

bool QSampleDataBrowse::GetSampleExport(const std::shared_ptr<tf::SampleInfo> spSample, SampleExportInfo& exportData)
{
	if (spSample == Q_NULLPTR)
	{
		return false;
	}

	// 样本申请模式（序号模式，样本架模式，条码模式）
	exportData.strSampleSendModle = ThriftEnumTrans::GetTestMode(spSample->testMode);
	// 检测模式
	exportData.strTestModle = ThriftEnumTrans::GetTestMode(spSample->testMode);
	// 样本类型
	exportData.strSampleSourceType = ThriftEnumTrans::GetSourceTypeName(spSample->sampleSourceType, " ");
	// 样本管类型
	exportData.strTubeType = ThriftEnumTrans::GetTubeType(spSample->tubeType, " ");
	// 样本条码号
	exportData.strBarcode = ToExQString(spSample->barcode);
	// 样本类别
	exportData.strSampleTypeStat = ThriftEnumTrans::GetSampleTypeStat(spSample->sampleType, spSample->stat);
	// 样本号
	exportData.strSampleNo = QDataItemInfo::GetSeqNo(*spSample);
	// 架号
	exportData.strRack = ToExQString(spSample->rack);
	// 位置号
	exportData.strPos = QString::number(spSample->pos);
	return true;
}

bool QSampleDataBrowse::GetTestItemExport(const std::shared_ptr<tf::TestItem> spTestIetm, SampleExportInfo& exportData)
{
	if (Q_NULLPTR == spTestIetm)
	{
		return false;
	}

	// 模块
	auto testModule = QString::fromStdString(spTestIetm->__isset.deviceSN ? CommonInformationManager::GetDeviceName((spTestIetm->deviceSN)) : " ");
	exportData.strTestModule = testModule;
	// 重复次数
	exportData.strFirstTestCounts = QString::number(spTestIetm->repeatIdx);

	// 是否计算项目
	if (CommonInformationManager::GetInstance()->IsCalcAssay(spTestIetm->assayCode))
	{
		auto spCalcAssayInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(spTestIetm->assayCode);
		if (spCalcAssayInfo == nullptr)
		{
			return false;
		}

		// 项目名称
		exportData.strItemName = ToExQString(spCalcAssayInfo->name);
		return true;
	}

	auto spGeneralAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(spTestIetm->assayCode);
	if (spGeneralAssayInfo == nullptr)
	{
		return false;
	}

	// 项目名称
	exportData.strItemName = ToExQString(spGeneralAssayInfo->assayName);
	return true;
}

bool QSampleDataBrowse::GetResultExport(const tf::TestResultKeyInfo& resultKey, const int assayCode, SampleExportInfo& exportData)
{
	// 是否计算项目
	if (CommonInformationManager::GetInstance()->IsCalcAssay(assayCode))
	{
		auto spCalcAssayInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
		if (spCalcAssayInfo == nullptr)
		{
			return false;
		}

		auto spAssayTestResult = GetCalcAssayResultByid(resultKey.assayTestResultId);
		if (spAssayTestResult == Q_NULLPTR)
		{
			return false;
		}

		// 原始结果
		if (spAssayTestResult->__isset.result)
		{
			exportData.strOrignialTestResult = ToExQString(spAssayTestResult->result);
		}

		// 数据报警（bug0013258 modify by wuht）
		exportData.strResultStatus = ToExQString(spAssayTestResult->resultStatusCodes);

		// 结果
		if (spAssayTestResult->__isset.editResult && !spAssayTestResult->editResult.empty())
		{
			// modify bug12502 by wuht
			exportData.strTestResult = ("*" + ToExQString(spAssayTestResult->editResult));
			// 浓度
			exportData.strConc = ("*" + ToExQString(spAssayTestResult->editResult));
		}
		else
		{
			exportData.strTestResult = ToExQString(spAssayTestResult->result);
			// 浓度
			exportData.strConc = ToExQString(spAssayTestResult->result);
		}

		// 结果生成时间
		if (spAssayTestResult->__isset.dateTime)
		{
			exportData.strEndTestTime = ToExQString((spAssayTestResult->dateTime));
		}

		return true;
	}

	auto spGeneralAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
	if (spGeneralAssayInfo == nullptr)
	{
		return false;
	}

	// ISE项目
	if (CommonInformationManager::GetInstance()->IsAssayISE(assayCode))
	{
		auto spAssayTestResult = GetIseAssayTestResult(resultKey.assayTestResultId);
		if (spAssayTestResult == Q_NULLPTR)
		{
			return false;
		}

		// 项目检测完成时间
		exportData.strEndTestTime = ToExQString(spAssayTestResult->endTime);
		// 原始结果
		auto orignData = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayTestResult->assayCode, spAssayTestResult->conc);
		exportData.strOrignialTestResult = QString::number(orignData, 'f', spGeneralAssayInfo->decimalPlace);

		// 结果
		double showResult;
		if (spAssayTestResult->__isset.concEdit)
		{
			// 转化修改结果为double
			std::stringstream editConc(spAssayTestResult->concEdit);
			double tempResult;
			if ((editConc >> tempResult))
			{
				showResult = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayTestResult->assayCode, tempResult);
			}
			else
			{
				showResult = orignData;
			}
		}
		else
		{
			showResult = orignData;
		}

		if (spAssayTestResult->__isset.concEdit)
		{
			exportData.strTestResult = "*" + QString::number(showResult, 'f', spGeneralAssayInfo->decimalPlace);
		}
		else
		{
			exportData.strTestResult = QString::number(showResult, 'f', spGeneralAssayInfo->decimalPlace);
		}
		// 结果就是浓度
		exportData.strConc = exportData.strTestResult;

		// 计算电动势
		exportData.strCalcuLateEmf = QString::number(spAssayTestResult->testEMF.sampleEMF, 'f',3);
		// 数据报警
		exportData.strResultStatus = ToExQString(spAssayTestResult->resultStatusCodes);
		// 视觉识别结果
		exportData.strAIResult = " ";/*QString::number((::ch::tf::AIResult::type)(spAssayTestResult->aiResult))*/;
		// 手工稀释倍数(机外稀释)20240715最新需求,只有当稀释倍数大于1的时候，才显示倍数
		if (spAssayTestResult->preDilutionFactor > 1)
		{
			exportData.strPreDilutionFactor = QString::number(spAssayTestResult->preDilutionFactor);
		}
		// 机内稀释,20240715最新需求,只有当稀释倍数大于1的时候，才显示倍数
		if (spAssayTestResult->dilutionFactor > 1)
		{
			exportData.strDilutionFactor = QString::number(spAssayTestResult->dilutionFactor);
		}
		// 单位
		auto strUnit = CommonInformationManager::GetInstance()->GetCurrentUnit(spAssayTestResult->assayCode);
		exportData.strUnit = strUnit;
		if (spAssayTestResult->__isset.resultSupplyInfo)
		{
			// 内部标准液批号
			exportData.strIsLOt = ToExQString(spAssayTestResult->resultSupplyInfo.IS_Lot);
			// 内部标准液瓶号
			exportData.strIsSn = ToExQString(spAssayTestResult->resultSupplyInfo.IS_Sn);
			// 缓冲液批号
			exportData.strDiluentLot = ToExQString(spAssayTestResult->resultSupplyInfo.diluent_Lot);
			// 缓冲液瓶号
			exportData.strDiluentSn = ToExQString(spAssayTestResult->resultSupplyInfo.diluent_Sn);
		}

		if (spAssayTestResult->__isset.resultCaliInfo)
		{
			// 校准品名称
			exportData.strCaliName = ToExQString(spAssayTestResult->resultCaliInfo.calibratorName);
			// 校准品批号
			exportData.strCaliLot = ToExQString(spAssayTestResult->resultCaliInfo.calibratorLot);
			// 校准时间
			exportData.strCaliTime = ToExQString(spAssayTestResult->resultCaliInfo.caliDate);
		}

		// 针酸性清洗液批号
		exportData.strSampleProbeAcidityLot = " ";
		// 针碱性清洗液批号
		exportData.strSampleProbeAlkalintyLot = " ";
	}
	else
	{
		auto spAssayTestResult = GetAssayResultByid(resultKey.assayTestResultId);
		if (spAssayTestResult == Q_NULLPTR)
		{
			return false;
		}

		std::shared_ptr<ch::tf::GeneralAssayInfo>  spChGeneralAssayInfo = Q_NULLPTR;
		if (assayCode == ch::tf::g_ch_constants.ASSAY_CODE_H || assayCode == ch::tf::g_ch_constants.ASSAY_CODE_L || assayCode == ch::tf::g_ch_constants.ASSAY_CODE_I)
		{
			spChGeneralAssayInfo = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(ch::tf::g_ch_constants.ASSAY_CODE_SIND, tf::DeviceType::DEVICE_TYPE_C1000);
		}
		else
		{
			spChGeneralAssayInfo = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(assayCode, tf::DeviceType::DEVICE_TYPE_C1000);
		}

		if (spChGeneralAssayInfo == Q_NULLPTR)
		{
			return false;
		}

		// 校准点数就是校准品数目
		exportData.strCaliPoints = QString::number(spChGeneralAssayInfo->caliQuantity);
		// 加R1时间
		exportData.strAddReagent1Time = " ";
		// 加样时间
		exportData.strAddSampleTime = " ";
		// 混匀时间1
		exportData.strVortexTime1 = " ";
		// 加R2时间
		exportData.strAddReagent2Time = " ";
		// 混匀时间2
		exportData.strVortexTime2 = " ";
		// 项目检测完成时间
		exportData.strEndTestTime = ToExQString(spAssayTestResult->endTime);
		// 原始结果
		auto result = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayTestResult->assayCode, spAssayTestResult->conc);
		exportData.strOrignialTestResult = QString::number(result, 'f', spGeneralAssayInfo->decimalPlace);		
		// 结果(有可能是被修改过的)
		double showResult;
		if (spAssayTestResult->__isset.concEdit)
		{
			// 转化修改结果为double
            std::stringstream editConc(spAssayTestResult->concEdit);
			double tempResult;
			if ((editConc >> tempResult))
			{
				showResult = CommonInformationManager::GetInstance()->ToCurrentUnitValue(spAssayTestResult->assayCode, tempResult);
			}
			else
			{
				showResult = result;
			}
		}
		else
		{
			showResult = result;
		}

		if (spAssayTestResult->__isset.concEdit)
		{
			exportData.strTestResult = "*" + QString::number(showResult, 'f', spGeneralAssayInfo->decimalPlace);
		}
		else
		{
			exportData.strTestResult = QString::number(showResult, 'f', spGeneralAssayInfo->decimalPlace);
		}
		// 结果就是浓度
		exportData.strConc = exportData.strTestResult;
		// 计算吸光度
		exportData.strCalculateAbs = QString::number(spAssayTestResult->abs);
		// 吸光度
		exportData.strAbs = QString::number(spAssayTestResult->abs);
		// 结果状态(数据报警)
		exportData.strResultStatus = ToExQString(spAssayTestResult->resultStatusCodes);
		// 单位
		exportData.strUnit = CommonInformationManager::GetInstance()->GetCurrentUnit(spAssayTestResult->assayCode);
		// 样本量(常量，增量，减量)
		//exportData.strSuckVol = QString::number(double(spAssayTestResult->sampleVol)/10, 'f', 1);
		if (spAssayTestResult->dilutionFactor == 1)
		{
			exportData.strSuckVol = CommonInformationManager::GetInstance()->GetDilutionFactor(spAssayTestResult);
		}

		// 手工稀释倍数(机外稀释)20240715最新需求,只有当稀释倍数大于1的时候，才显示倍数
		if (spAssayTestResult->preDilutionFactor > 1)
		{
			exportData.strPreDilutionFactor = QString::number(spAssayTestResult->preDilutionFactor);
		}
		// 机内稀释倍数20240715最新需求,只有当稀释倍数大于1的时候，才显示倍数
		if (spAssayTestResult->dilutionFactor > 1)
		{
			exportData.strDilutionFactor = CommonInformationManager::GetInstance()->GetDilutionFactor(spAssayTestResult);
		}

		// 视觉识别结果
		exportData.strAIResult = " "/*QString::number((::ch::tf::AIResult::type)(spAssayTestResult->aiResult))*/;
		// 试剂盘
		exportData.strReagentDisk = " ";
		// 反应杯号
		exportData.strAssayCupBatchNo = QString::number(spAssayTestResult->cupSN);
		// 试剂批号
		exportData.strReagentBatchNo = ((spAssayTestResult->reagentKeyInfos.size() > 0 ) ? ToExQString(spAssayTestResult->reagentKeyInfos[0].lot) : " ");
		// 试剂瓶号
		exportData.strReagentSerialNo = ((spAssayTestResult->reagentKeyInfos.size() > 0) ? ToExQString(spAssayTestResult->reagentKeyInfos[0].sn) : " ");
		// 试剂上机时间
		exportData.strRgtRegisterT = (spAssayTestResult->__isset.rgtRegisterTime ? QString::fromStdString(spAssayTestResult->rgtRegisterTime) : " ");

		if (spAssayTestResult->__isset.resultSupplyInfo)
		{
			// 试剂失效日期
			exportData.strReagentExpTime = ToExQString(spAssayTestResult->resultSupplyInfo.reagentexpiryDate);
			// 开瓶有效期
			exportData.strOpenBottleExpiryTime = (spAssayTestResult->resultSupplyInfo.reagentopenBottleExpiryTime >= 0) ? QString::number(spAssayTestResult->resultSupplyInfo.reagentopenBottleExpiryTime) : "0";
		}


		// 酸性清洗液批号
		exportData.strAcidityLot = " ";
		// 碱性清洗液批号
		exportData.strAlkalintyLot = " ";
		// 针酸性清洗液批号
		exportData.strSampleProbeAcidityLot = " ";
		// 针碱性清洗液批号
		exportData.strSampleProbeAlkalintyLot = " ";

		if (spAssayTestResult->__isset.resultCaliInfo)
		{
			// 校准品名称
			exportData.strCaliName = ToExQString(spAssayTestResult->resultCaliInfo.calibratorName);
			// 校准品批号
			exportData.strCaliLot = ToExQString(spAssayTestResult->resultCaliInfo.calibratorLot);
			// 校准时间
			exportData.strCaliTime = ToExQString(spAssayTestResult->resultCaliInfo.caliDate);
		}

		// 校准方法
		exportData.strCaliType = ThriftEnumTrans::GetTestItemMethod(spChGeneralAssayInfo->calibrationType);
		// 校准水平
		exportData.strCaliLevel = QString::number(spAssayTestResult->caliLevel);
		// 稀释样本量
		exportData.strDilutionSampleVol = QString::number(double(spAssayTestResult->caliDilutSampleVol) / 10, 'f', 1);
		// 稀释液量
		exportData.strDiluentVol = QString::number(double(spAssayTestResult->caliDilutVol) / 10, 'f', 1);
		// 分析方法
		exportData.strAnalysisMethod = ThriftEnumTrans::GetAnalysisMethodName(spChGeneralAssayInfo->analysisMethod);
		// 主波长
		exportData.strPrimaryWave = (!spAssayTestResult->primarySubWaves.empty() ? QString::number(spAssayTestResult->primarySubWaves[0]) : " ");
		// 次波长
		exportData.strDeputyWave = ((spAssayTestResult->primarySubWaves.size() > 1) ? QString::number(spAssayTestResult->primarySubWaves[1]) : " ");
		// 反应时间
		exportData.strReactionTime = QString::number(((spAssayTestResult->detectPoints.size() == 34) ? 10 : 5));
		// 测光点(总数，非m,n)
		exportData.strDetectPoint = QString::number(static_cast<qulonglong>(spAssayTestResult->detectPoints.size()));

		// 校准曲线失效日期
		//exportData.strCaliLot = (spAssayTestResult->resultCaliInfo.__isset.calibratorExpiryDate ? ToExQString(spAssayTestResult->resultCaliInfo.calibratorExpiryDate) : " ");
	}

	return true;
}

bool QSampleDataBrowse::GetResultAbs(const std::shared_ptr<ch::tf::AssayTestResult> spAssayTestResult, SampleExportInfo& exportData, QVariantList& dynamicTitle)
{
	// 最大测光点数目
	int maxPoints = 34;
	// 次波长序号(波长数组中)
	int deputyWave = 2;
	if (spAssayTestResult == Q_NULLPTR)
	{
		return false;
	}

	// 水空白波长
	std::vector<std::map<int32_t, int32_t>> waterBlanks;
	QVariantList dyTitles;
	QVariantList rowDatas;
	if (DecodeWaterBlankOds(spAssayTestResult->waterBlanks, waterBlanks) && !waterBlanks.empty())
	{
		QVariantList rowBlankDatas;
		QVariantList titleBlank;
		for (auto& wb : waterBlanks[0])
		{
			rowBlankDatas << wb.second;
			titleBlank << (QString::number(wb.first) + tr("波长水空白"));
		}
		dyTitles.push_back(titleBlank);
		rowDatas.push_back(rowBlankDatas);
	}

	// 测光点的吸光度
	{
		std::map<int, std::vector<int>> wlClassify;
		// 获取各个测光点的吸光度的值列表
		for (const auto& detectData : spAssayTestResult->detectPoints)
		{
			// 将每个波长的吸光度都分类出来
			auto ods = detectData.ods;
			for (auto& od : ods)
			{
				wlClassify[od.first].push_back(od.second);
			}
		}

		// 导出主副波长
		int loop = 0;
		do
		{
			QVariantList rowInnerDatas;
			QVariantList titleMain;
			QString waveName;
			if (loop == 0)
				waveName = tr("主波长吸光度");
			else
				waveName = tr("副波长吸光度");

			for (size_t o_i = 0; o_i < maxPoints; )
			{
				if (loop >= spAssayTestResult->primarySubWaves.size() || wlClassify.find(spAssayTestResult->primarySubWaves[loop]) == wlClassify.end())
				{
					rowInnerDatas << " ";
				}
				else
				{
					auto& odsVec = wlClassify[spAssayTestResult->primarySubWaves[loop]];
					// 按照最多34个点导出，不足的，导出为" ";
					if (o_i < odsVec.size())
					{
						rowInnerDatas << odsVec[o_i];
					}
					else
					{
						rowInnerDatas << " ";
					}
				}

				titleMain << (waveName + QString::number(++o_i));
			}
			dyTitles.push_back(titleMain);
			rowDatas.push_back(rowInnerDatas);

		} while (++loop < 2);

		// 重新组织波长和吸光度数据
		for (auto& wl : wlClassify)
		{
			QVariantList rowReInnerDatas;
			QVariantList titleAll;
			for (size_t o_i = 0; o_i < maxPoints; )
			{
				// 按照最多34个点导出，不足的，导出为" ";
				if (o_i < wl.second.size())
				{
					rowReInnerDatas << wl.second[o_i];
					titleAll << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
				}
				else
				{
					rowReInnerDatas << " ";
					titleAll << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
				}
			}
			dyTitles.push_back(titleAll);
			rowDatas.push_back(rowReInnerDatas);
		}
	}

	auto fjoin = [](const QVariantList& rawData, const QString flag)->QString
	{
		QStringList strWaveAbs;
		for (const auto& data : rawData)
		{
			strWaveAbs.push_back(data.toString());
		}

		return strWaveAbs.join(flag);
	};

	// 保存标题
	for (const auto& dataTitle : dyTitles)
	{
		auto waveTitle = fjoin(dataTitle.toList(), "\t");
		dynamicTitle.push_back(dataTitle);
	}

	exportData.strPrimaryWaveAbs.clear();
	exportData.strDeputyWaveAbs.clear();
	exportData.strAllWaveAbs.clear();
	// 保存数据
	int i = 0;
	for (const auto& data : rowDatas)
	{
		auto waveData = fjoin(data.toList(), "\t");
		if (i == 0 || i == 1)
		{
			exportData.strPrimaryWaveAbs += (waveData + "\t");
		}

		if(i == deputyWave)
		{
			exportData.strDeputyWaveAbs = (waveData);
		}

		if (i > deputyWave)
		{
			exportData.strAllWaveAbs += (waveData + "\t");
		}

		i++;
	}

	exportData.strPrimaryWaveAbs.chop(1);
	exportData.strAllWaveAbs.chop(1);
	return true;
}

QVariantList QSampleDataBrowse::GetExportData(bool isXlsx, const tf::UserType::type UType, const SampleExportInfo& srcData, const std::set<ExportInfoEn>& configData)
{
	QString flag = "\t";
	if (isXlsx)
	{
		flag.clear();
	}

	// 记录导出文本
	QVariantList strExportTextList;
	// 内容
	for (auto iter : configData)
	{
		// 需要排除吸光度，放在数据最后
		if (iter == ExportInfoEn::EIE_PRIMARY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_DEPUTY_WAVE_ABS
			|| iter == ExportInfoEn::EIE_ALL_WAVE_ABS)
		{
			continue;
		}

		auto pSEInfo = SEMetaInfos::GetInstance()->GetMetaInfo(iter);
		if (pSEInfo != nullptr && pSEInfo->m_UserType <= UType)
		{
			auto data = srcData.*(pSEInfo->m_Field);
			if (isXlsx)
			{
				auto iterExport = m_exportDataMap.find(iter);
				if (iterExport != m_exportDataMap.end())
				{
					QVariant convertData;
					if (ConvertData(iterExport->second, data, convertData))
					{
						strExportTextList << convertData;
						continue;
					}
				}
			}

			strExportTextList << (data + flag);
		}
	}

	if (configData.count(EIE_PRIMARY_WAVE_ABS) > 0)
	{
		auto absLists = srcData.strPrimaryWaveAbs.split("\t");
		for (const auto& abs : absLists)
		{
			QVariant convertData;
			if (isXlsx && ConvertData(DATATYPE::INTTYPE, abs, convertData))
			{
				strExportTextList << convertData;
				continue;
			}

			strExportTextList << (abs + flag);
		}
	}

	if (configData.count(EIE_DEPUTY_WAVE_ABS) > 0)
	{
		auto absLists = srcData.strDeputyWaveAbs.split("\t");
		for (const auto& abs : absLists)
		{
			QVariant convertData;
			if (isXlsx && ConvertData(DATATYPE::INTTYPE, abs, convertData))
			{
				strExportTextList << convertData;
				continue;
			}

			strExportTextList << (abs + flag);
		}
	}

	if (configData.count(EIE_ALL_WAVE_ABS) > 0)
	{
		auto absLists = srcData.strAllWaveAbs.split("\t");
		for (const auto& abs : absLists)
		{
			QVariant convertData;
			if (isXlsx && ConvertData(DATATYPE::INTTYPE, abs, convertData))
			{
				strExportTextList << convertData;
				continue;
			}

			strExportTextList << (abs + flag);
		}
	}

	return strExportTextList;
}

///
///  @brief 按项目导出
///
///
///
///  @return	true 成功 false 失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
bool QSampleDataBrowse::ExPortItem()
{
	ULOG(LOG_INFO, __FUNCTION__);
	std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (pLoginUserInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't get UserInfo.");
		return false;
	}

	if (pLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)   // 普通用户不能导出
	{
		return false;
	}

	auto softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();
	// 生免联机
	if (softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
	{
		// 	bug0020858 add by ldx 20230913
		QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
		if (!fileName.isNull())
		{
			if (fileName.isEmpty())
			{
				std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("文件名为空！")));
				pTipDlg->exec();
				return false;
			}
		}
		else
		{
			return false;
		}

		auto selecteds = DataPrivate::Instance().GetAllSelectFlag(1);
		if (selecteds.empty())
		{
			return false;
		}

		// 在线程里面依次导出生化和免疫的信息
		auto exportAll = [this](QString fileName, int type, std::set<int64_t> selecteds)->void
		{
			// 耗时操作放在线程里完成（生化）
			QFileInfo FileInfo(fileName);
			QString strSuffix = FileInfo.suffix();
			QString path = FileInfo.absolutePath() + "/";
			QString fileNameCh = path + FileInfo.completeBaseName() + "_CH." + strSuffix;
			ExportItem(tf::UserType::type(type), selecteds, fileNameCh);

			// 耗时操作放在线程里完成(免疫)
			QString fileNameIM = path + FileInfo.completeBaseName() + "_IM." + strSuffix;
			ExportItemData_im(fileNameIM, tf::UserType::type(type), selecteds);
		};

		std::thread thExport(exportAll, fileName, pLoginUserInfo->type, selecteds);
		thExport.detach();
	}
	else if (softwareType == SOFTWARE_TYPE::IMMUNE)
	{
		// 	bug0020858 add by ldx 20230913
		// 弹出保存文件对话框
		QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
		if (!fileName.isNull())
		{
			if (fileName.isEmpty())
			{
				std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("文件名为空！")));
				pTipDlg->exec();
				return false;
			}
		}
		else
		{
			return false;
		}

		auto selecteds = DataPrivate::Instance().GetAllSelectFlag(1);

		// 耗时操作放在线程里完成
		std::thread th(std::bind(&QSampleDataBrowse::ExportItemData_im, this, fileName, pLoginUserInfo->type, selecteds));
		th.detach();
	}
	// 生化
	else
	{
		// 	bug0020858 add by ldx 20230913
		QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
		if (!fileName.isNull())
		{
			if (fileName.isEmpty())
			{
				std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("文件名为空！")));
				pTipDlg->exec();
				return false;
			}
		}
		else
		{
			return false;
		}

		auto selecteds = DataPrivate::Instance().GetAllSelectFlag(1);
		if (selecteds.empty())
		{
			return false;
		}

		// 耗时操作放在线程里完成
		std::thread th(std::bind(&QSampleDataBrowse::ExportItem, this, pLoginUserInfo->type, selecteds, fileName));
		th.detach();
	}

    return true;
}

bool QSampleDataBrowse::ExportItemData_im(QString fileName, tf::UserType::type UType, std::set<int64_t>& vecSample)
{
    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    ExpSampleInfoVector vecSamples;

    // 导出文本列表
    for (const auto& db : vecSample)
    {
        // 如果当前行为空，则跳过
        auto item = DataPrivate::Instance().GetTestItemInfoByDb(db);
        if (!item)
        {
            continue;
        }

        bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(item->assayCode);
		// 是否含有免疫项目，对于联机版免疫页面导出，当含有免疫项目的时候，才导出对应的计算项目
		if (isCalc)
		{
			bool isHaveIm = CommonInformationManager::GetInstance()->IsCaClHaveChorIM(item->assayCode, false);
			if (!isHaveIm)
			{
				continue;
			}
		}

		auto spGenneral = CommonInformationManager::GetInstance()->GetAssayInfo(item->assayCode);
		if (spGenneral == nullptr && !isCalc)
		{
			continue;
		}

		// 只导出免疫项目(0013739)
		if (!(isCalc || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
		{
			continue;
		}

        auto pSample = DataPrivate::Instance().GetSampleInfoByDb(item->sampleInfoId);

        // 样本信息
        SampleExportInfo SampleInfo;
        SampleInfo.strSampleNo = QDataItemInfo::Instance().GetSeqNo(*pSample);
        SampleInfo.strBarcode = QString::fromStdString(pSample->barcode);
        SampleInfo.strSampleSourceType = ConvertTfEnumToQString((::tf::SampleSourceType::type)pSample->sampleSourceType);
        SampleInfo.strPos = QDataItemInfo::Instance().GetPos(*pSample);

        auto pResults = DataPrivate::Instance().GetAssayResultByTestItem(*item);

        // 首次检测结果
        auto firstResult = (*pResults)[0];
        im::tf::AssayTestResult FirstTestResult;
        if (firstResult.has_value() && typeid(im::tf::AssayTestResult) == firstResult.value().type())
        {
            FirstTestResult = boost::any_cast<im::tf::AssayTestResult>(firstResult.value());
        }

        auto ReTestResult = (*pResults)[1];
        im::tf::AssayTestResult LastTestResult;
        if (ReTestResult.has_value() && typeid(im::tf::AssayTestResult) == ReTestResult.value().type())
        {
            LastTestResult = boost::any_cast<im::tf::AssayTestResult>(ReTestResult.value());
        }

        MakeExportItemResult(SampleInfo, pSample, item, pResults);

        vecSamples.push_back(std::move(SampleInfo));

    }

    // 导出文件
    bool bRet = false;
    FileExporter fileExporter;
    QFileInfo FileInfo(fileName);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        SampleExportModule info;
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        info.strExportTime = strPrintTime.toStdString();
        fileExporter.ExportSampleInfoToPdf(vecSamples, info);
        std::string strInfo = GetJsonString(info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportSampleInfo.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRet = true;
    }
    else
    {
        bRet = fileExporter.ExportSampleInfo(vecSamples, fileName, UType);
    }
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条

    // 弹框提示导出失败
    /*std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")));
    pTipDlg->exec();*/
    return bRet;
}


void QSampleDataBrowse::MakeExportItemResult(SampleExportInfo& SampleInfo, std::shared_ptr<tf::SampleInfo> pSample, std::shared_ptr<tf::TestItem> pItem, std::shared_ptr<std::vector<boost::optional<boost::any>>> pResults)
{
    auto firstResult = (*pResults)[0];
    im::tf::AssayTestResult FirstTestResult;
    if (firstResult.has_value() && typeid(im::tf::AssayTestResult) == firstResult.value().type())
    {
        FirstTestResult = boost::any_cast<im::tf::AssayTestResult>(firstResult.value());
    }
    
    auto ReTestResult = (*pResults)[1];
    im::tf::AssayTestResult LastTestResult;
    if (ReTestResult.has_value() && typeid(im::tf::AssayTestResult) == ReTestResult.value().type())
    {
        LastTestResult = boost::any_cast<im::tf::AssayTestResult>(ReTestResult.value());
    }

    SampleInfo.strPos = QDataItemInfo::Instance().GetPos(*pSample);

    // 样本管类型, 无样本管时导出为空
    if (FirstTestResult.tubeType > 0)
        SampleInfo.strTubeType = ConvertTfEnumToQString((::tf::TubeType::type)FirstTestResult.tubeType);

    // 检测完成时间
    SampleInfo.strEndTestTime = QString::fromStdString(pItem->endTime);

    // 检测模式
    if (FirstTestResult.detectMode >= 0)
        SampleInfo.strTestModle = ConvertTfEnumToQString((::im::tf::DetectMode::type)FirstTestResult.detectMode);

    // 进样模式
    SampleInfo.strSampleSendModle = ThriftEnumTrans::GetTestMode(FirstTestResult.testMode);
    // 项目名称
    SampleInfo.strItemName = CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(pItem->assayCode);

    QString strResult;
    bool isCalc = CommonInformationManager::IsCalcAssay(pItem->assayCode);
    if (isCalc)
    {
        strResult = QDataItemInfo::Instance().SetAssayResultFlag(pItem->conc, false, false, *pItem, pSample->sampleType);
        SampleInfo.strTestResult = strResult;
    }
    else if (pItem->__isset.firstTestResultKey)
    {
        // 信号值, 计算项目没有信号值
        SampleInfo.strRUL = QString::fromStdString(pItem->RLU);

        // 原始信号值
        if (FirstTestResult.RLU >= 0)
            SampleInfo.strOrignialRUL = QString::number(FirstTestResult.RLU);

        // 检测结果
        strResult = QDataItemInfo::Instance().SetAssayResultFlag(FirstTestResult.concEdit, false, false, *pItem, pSample->sampleType);
        SampleInfo.strTestResult = strResult;

        // 原始结果
        SampleInfo.strOrignialTestResult = QDataItemInfo::Instance().SetAssayResultFlag(FirstTestResult.conc, false, true, *pItem, pSample->sampleType, true);
    }

    // 结果状态
    SampleInfo.strResultStatus = QDataItemInfo::Instance().GetAssayResultStatus((AssayResult)FirstTestResult, strResult.toStdString());

    // 视觉识别结果
    SampleInfo.strAIResult = ConvertTfEnumToQString((::im::tf::AIResult::type)FirstTestResult.aiResult);

    ///< 结果单位 
    const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();

    // 显示非浓度的情况下，不需要显示单位
    if (WorkpageCommon::IsShowConc(sampleSet, pSample->sampleType))
    {
        // 单位
        if (isCalc)
        {
            std::shared_ptr<tf::CalcAssayInfo> spCalcInfo = CommonInformationManager::GetInstance()->GetCalcAssayInfo(pItem->assayCode);
            SampleInfo.strUnit = QString::fromStdString(spCalcInfo->resultUnit);
        }
        else
        {
            SampleInfo.strUnit = QString::fromStdString(QDataItemInfo::Instance().GetAssayResultUnitName(pItem->assayCode));
        }
    }

    // 模块
    SampleInfo.strTestModule = QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(FirstTestResult.deviceSN));

    // 手工稀释倍数, 质控结果不导出稀释倍数，界面没显示
    tf::SampleType::type sampleType = pSample->sampleType;
    if (tf::SampleType::SAMPLE_TYPE_QC != sampleType && FirstTestResult.preDilutionFactor > 0)
        SampleInfo.strPreDilutionFactor = QString::number(FirstTestResult.preDilutionFactor);

    // 机内稀释倍数
    if (tf::SampleType::SAMPLE_TYPE_QC != sampleType && FirstTestResult.dilutionFactor > 0)
        SampleInfo.strDilutionFactor = QString::number(FirstTestResult.dilutionFactor);

    // 试剂批号
    SampleInfo.strReagentBatchNo = QString::fromStdString(FirstTestResult.reagentBatchNo);

    // 试剂瓶号
    SampleInfo.strReagentSerialNo = QString::fromStdString(FirstTestResult.reagentSerialNo);

    // 底物液批号
    SampleInfo.strSubstrateBatchNo = QString::fromStdString(FirstTestResult.supplyInfo.substrateABatchNo);

    // 底物液瓶号
    SampleInfo.strSubstrateBottleNo = QString::fromStdString(FirstTestResult.supplyInfo.substrateASerialNo);

    // 清洗缓冲液批号
    SampleInfo.strCleanFluidBatchNo = QString::fromStdString(FirstTestResult.supplyInfo.cleanFluidBatchNo);

    // 反应杯批号
    SampleInfo.strAssayCupBatchNo = QString::fromStdString(FirstTestResult.supplyInfo.assayCupBatchNo);

    // 稀释液批号
    SampleInfo.strDiluentBatchNo = QString::fromStdString(FirstTestResult.supplyInfo.diluentBatchNo);

    // 试剂上机时间
    SampleInfo.strRgtRegisterT = QString::fromStdString(FirstTestResult.supplyInfo.reagentRegisterTime);

    // 试剂失效日期
    SampleInfo.strReagentExpTime = QString::fromStdString(FirstTestResult.supplyInfo.reagentExpTime);

    // 校准品批号
    SampleInfo.strCaliLot = QString::fromStdString(FirstTestResult.caliLot);

    if (FirstTestResult.curCaliTime != "1970-01-01 00:00:00" && tf::SampleType::SAMPLE_TYPE_CALIBRATOR != sampleType)
    {
        // 校准曲线信息
        SampleInfo.strFirstCurCaliTime = QString::fromStdString(FirstTestResult.curCaliTime)
            + "   "
            + QString::fromStdString(FirstTestResult.reagentBatchNo);
    }

    // 复查结果
    if (pItem->__isset.lastTestResultKey)
    {
        QString LastResult;
        if (!isCalc)
        {
            // 小数点后保留几位小数(默认2位)
            int left = 2;
            auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(pItem->assayCode);
            if (spAssayInfo != Q_NULLPTR)
            {
                left = spAssayInfo->decimalPlace;
            }

            if (LastTestResult.RLUEdit >= 0)
            {
                // 复查信号值
                SampleInfo.strReTestRUL = QString::number(LastTestResult.RLUEdit);
            }
            else if (LastTestResult.RLU >= 0)
            {
                SampleInfo.strReTestRUL = QString::number(LastTestResult.RLU);
            }

            // 复查原始信号值
            if (LastTestResult.RLU >= 0)
                SampleInfo.strReTestOriglRUL = QString::number(LastTestResult.RLU);

            // 复查结果 
            LastResult = QDataItemInfo::Instance().SetAssayResultFlag(LastTestResult.concEdit, true, false, *pItem, pSample->sampleType);
            SampleInfo.strReTestResult = LastResult;

            // 复查原始结果
            SampleInfo.strReOriglTestResult = QDataItemInfo::Instance().SetAssayResultFlag(LastTestResult.conc, false, true, *pItem, pSample->sampleType, true);
        }
        else
        {
            // 复查结果 
            LastResult = QDataItemInfo::Instance().SetAssayResultFlag(pItem->retestConc, true, false, *pItem, pSample->sampleType);
            SampleInfo.strReTestResult = LastResult;
        }

        if (LastResult.isEmpty())
        {
            return;
        }

        // 复查数据报警
        SampleInfo.strReResultStatus = QDataItemInfo::Instance().GetAssayResultStatus((AssayResult)LastTestResult, LastResult.toStdString());

        // 复查样本管类型
        if (LastTestResult.tubeType > 0)
            SampleInfo.strReTestTubeType = ConvertTfEnumToQString((::tf::TubeType::type)LastTestResult.tubeType);

        // 复查视觉识别结果
        if (LastTestResult.aiResult >= 0)
            SampleInfo.strReAIResult = ConvertTfEnumToQString((::im::tf::AIResult::type)LastTestResult.aiResult);

        // 模块
        SampleInfo.strReTestModule = QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(LastTestResult.deviceSN));

        // 复查手工稀释倍数
        if (tf::SampleType::SAMPLE_TYPE_QC != sampleType && LastTestResult.preDilutionFactor > 0)
            SampleInfo.strRePreDilFactor = QString::number(LastTestResult.preDilutionFactor);

        // 复查机内稀释倍数
        if (tf::SampleType::SAMPLE_TYPE_QC != sampleType && LastTestResult.dilutionFactor > 0)
            SampleInfo.strReDilFactor = QString::number(LastTestResult.dilutionFactor);

        // 复查完成时间
        SampleInfo.strReEndTestTime = QString::fromStdString(pItem->retestEndTime);

        // 复查检测模式
        if (LastTestResult.detectMode >= 0)
            SampleInfo.strReTestModle = ConvertTfEnumToQString((::im::tf::DetectMode::type)LastTestResult.detectMode);

        // 复查进样模式
        SampleInfo.strReSampleSendModle = ThriftEnumTrans::GetTestMode(LastTestResult.testMode);

        // 试剂批号
        SampleInfo.strReReagentBatchNo = QString::fromStdString(LastTestResult.reagentBatchNo);

        // 试剂瓶号
        SampleInfo.strReReagentSerialNo = QString::fromStdString(LastTestResult.reagentSerialNo);

        // 底物液批号
        SampleInfo.strReSubBatchNo = QString::fromStdString(LastTestResult.supplyInfo.substrateABatchNo);

        // 底物液瓶号
        SampleInfo.strReSubBottleNo = QString::fromStdString(LastTestResult.supplyInfo.substrateASerialNo);

        // 清洗缓冲液批号
        SampleInfo.strReCleanFluidBatchNo = QString::fromStdString(LastTestResult.supplyInfo.cleanFluidBatchNo);

        // 反应杯批号
        SampleInfo.strReAssayCupBatchNo = QString::fromStdString(LastTestResult.supplyInfo.assayCupBatchNo);

        // 稀释液批号
        SampleInfo.strReDiluentBatchNo = QString::fromStdString(LastTestResult.supplyInfo.diluentBatchNo);

        // 试剂上机时间
        SampleInfo.strReRgtRegisterT = QString::fromStdString(LastTestResult.supplyInfo.reagentRegisterTime);

        // 试剂失效日期
        SampleInfo.strReReagentExpTime = QString::fromStdString(LastTestResult.supplyInfo.reagentExpTime);

        // 校准品批号
        SampleInfo.strReCaliLot = QString::fromStdString(LastTestResult.caliLot);

        if (LastTestResult.curCaliTime != "1970-01-01 00:00:00" && tf::SampleType::SAMPLE_TYPE_CALIBRATOR != sampleType)
        {
            // 校准曲线信息
            SampleInfo.strReTestCurCaliTime = QString::fromStdString(LastTestResult.curCaliTime)
                + "   "
                + QString::fromStdString(LastTestResult.reagentBatchNo);
        }
    }
}


bool QSampleDataBrowse::ExportItem(const tf::UserType::type UType, const std::set<int64_t>& itemIds, const QString & path)
{
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    // 生化常规、急诊、质控导出表头(每个波长吸光度需要动态添加)
    QVariantList title_ch_normal;
    // 生化常规、急诊、质控 动态波长的表头
    QVariantList wareLenTitle;
    // 生化校准导出表头(每个波长吸光度需要动态添加)
    QVariantList title_ch_cali;
    // 生化校准 动态波长的表头
    QVariantList wareLenTitle_cali;
    // ise导出表头（常规、急诊、质控）
    QVariantList title_ise_normal;
    // ise生化校准导出表头
    QVariantList title_ise_cali;
    // 校准品类型的样本ID集合
    // K:样本主键  
    //V:样本中包含哪些校准项目以及测试当前测试次数（K:项目编号，V:当前累计测试次数）
    std::map<int64_t, std::map<int, int>> caliSampleId;

    // 每一行的数据(每一个string为一行数据，‘\t’分隔)
    QVariantList content_ch;
    QVariantList content_ch_cali;
    QVariantList content_ise;
    QVariantList content_ise_cali;

    // 获取样本属于第几次校准
    auto timesFunc = [&](int64_t sampleId, int assayCode)->int {
        // 没有导出过的校准样本 或者
        // 如果这个样本的这个项目没有导出过，那么就是第一次
        if (caliSampleId.find(sampleId) == caliSampleId.end() ||
            caliSampleId[sampleId].find(assayCode) == caliSampleId[sampleId].end())
        {
            caliSampleId[sampleId][assayCode] = 1;
        }
        else
            caliSampleId[sampleId][assayCode] += 1;

        return caliSampleId[sampleId][assayCode];
    };

	std::map<ExportType, std::set<ExportInfoEn>> savedata;
	if (!DictionaryQueryManager::GetExportConfig(savedata))
	{
		ULOG(LOG_ERROR, "Failed to get export config.");
		return false;
	}

	for (const auto& data : savedata)
	{
		if (data.first == ExportType::CH_NORMAL_AND_QC_TYPE)
		{
			GenTitleList(UType, title_ch_normal, data.second);
		}

		if (data.first == ExportType::CH_CALI_TYPE)
		{
			GenTitleList(UType, title_ch_cali, data.second);
		}

		if (data.first == ExportType::CH_ISE_SAM_AND_QC_TYPE)
		{
			GenTitleList(UType, title_ise_normal, data.second);
		}

		if (data.first == ExportType::CH_ISE_CALI_TYPE)
		{
			GenTitleList(UType, title_ise_cali, data.second);
		}
	}

	bool isXlsx = false;
	// 判断是否导出为excel表，否则作为文本文件写入数据
	QFileInfo FileInfo(path);
	QString strSuffix = FileInfo.suffix();
	// 导出为xlsx表格
	if (strSuffix == "xlsx")
	{
		isXlsx = true;
	}

	ExpSampleInfoVector vecSamples;
    for (const auto& db : itemIds)
    {
		SampleExportInfo sampleExport;
        // 如果当前行为空，则跳过
        auto item = DataPrivate::Instance().GetTestItemInfoByDb(db);
        if (!item)
        {
            continue;
        }

        bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(item->assayCode);
		auto spGenneral = CommonInformationManager::GetInstance()->GetAssayInfo(item->assayCode);
        // 不是计算项目，通用项目中也找不到
        if (spGenneral == nullptr && !isCalc)
		{
			continue;
		}

		// 只导出生化项目(0013739)
        if (!(isCalc || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
            || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE))
		{
			continue;
		}

		// 获取项目相关信息
		auto isGet = GetTestItemExport(item, sampleExport);
		if (!isGet)
		{
			continue;
		}

        // 获取样本信息
        auto spSample = DataPrivate::Instance().GetSampleInfoByDb(item->sampleInfoId);
        if (!spSample)
        {
            continue;
        }

		// 获取样本相关信息
		isGet = GetSampleExport(spSample, sampleExport);
		if (!isGet)
		{
			continue;
		}

        if (item->assayCode >= tf::AssayCodeRange::CH_RANGE_MIN && item->assayCode <= tf::AssayCodeRange::CH_RANGE_MAX)
        {
            // ise项目
            if (item->assayCode >= ise::tf::g_ise_constants.ASSAY_CODE_NA && item->assayCode <= ise::tf::g_ise_constants.ASSAY_CODE_CL)
            {
                // 常规、急诊、质控的导出
                if (spSample->sampleType != tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
                {
					auto setInfoIseIter = savedata.find(ExportType::CH_ISE_SAM_AND_QC_TYPE);
					if (setInfoIseIter == savedata.end())
					{
						continue;
					}

					if (item->firstTestResultKey.assayTestResultId > 0)
					{
						auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
						if (!isGet)
						{
							continue;
						}

						auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIseIter->second);
						content_ise.push_back(rawData);
						// for pdf
						vecSamples.push_back(sampleExport);
					}

					if (item->lastTestResultKey.assayTestResultId > 0)
					{
						auto isGet = GetResultExport(item->lastTestResultKey, item->assayCode, sampleExport);
						if (!isGet)
						{
							continue;
						}

						auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIseIter->second);
						content_ise.push_back(rawData);
						// for pdf
						vecSamples.push_back(sampleExport);
					}
                }
                // 导出校准项目
                else
                {
					// 校准项目，需要记录重复多少次
					sampleExport.strCaliCount = QString::number(timesFunc(item->sampleInfoId, item->assayCode));
					auto setInfoIseIter = savedata.find(ExportType::CH_ISE_CALI_TYPE);
					if (setInfoIseIter == savedata.end())
					{
						continue;
					}

					auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
					if (!isGet)
					{
						continue;
					}

					auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIseIter->second);
					content_ise_cali.push_back(rawData);
					// for pdf
					vecSamples.push_back(sampleExport);
                }
            }
            // 普通生化项目
            else
            {
                // 常规、急诊、质控的导出
                if (spSample->sampleType != tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
                {
					auto setInfoIter = savedata.find(ExportType::CH_NORMAL_AND_QC_TYPE);
					if (setInfoIter == savedata.end())
					{
						continue;
					}

					QVariantList tmpTitleFirst;
					if (item->firstTestResultKey.assayTestResultId > 0)
					{
						auto spAssayTestResult = GetAssayResultByid(item->firstTestResultKey.assayTestResultId);
						if (spAssayTestResult == Q_NULLPTR)
						{
							continue;
						}

						auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
						if (!isGet)
						{
							continue;
						}

						isGet = GetResultAbs(spAssayTestResult, sampleExport, tmpTitleFirst);
						if (!isGet)
						{
							continue;
						}

						auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIter->second);
						content_ch.push_back(rawData);
						// for pdf
						vecSamples.push_back(sampleExport);
					}

					QVariantList tmpTitleSecond;
					if (item->lastTestResultKey.assayTestResultId > 0)
					{
						auto spAssayTestResult = GetAssayResultByid(item->lastTestResultKey.assayTestResultId);
						if (spAssayTestResult == Q_NULLPTR)
						{
							continue;
						}

						auto isGet = GetResultExport(item->lastTestResultKey, item->assayCode, sampleExport);
						if (!isGet)
						{
							continue;
						}

						isGet = GetResultAbs(spAssayTestResult, sampleExport, tmpTitleSecond);
						if (!isGet)
						{
							continue;
						}

						auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIter->second);
						content_ch.push_back(rawData);
						// for pdf
						vecSamples.push_back(sampleExport);
					}

					if (tmpTitleFirst.empty())
					{
						continue;
					}

					int dataCount = 0;
					for (const auto& data : content_ch)
					{
						if (!data.canConvert<QVariantList>())
						{
							continue;
						}

						auto dataList = data.toList();
						dataCount += dataList.size();
					}

					int titleCount = 0;
					for (const auto& data : wareLenTitle)
					{
						if (!data.canConvert<QVariantList>())
						{
							continue;
						}

						auto dataList = data.toList();
						titleCount += dataList.size();
					}

					if (dataCount < titleCount)
					{
						continue;
					}

					if (tmpTitleFirst.size() < wareLenTitle.size())
					{
						continue;
					}

					wareLenTitle.clear();
					auto tmpTitles = tmpTitleFirst;
					// 主次吸光度只要显示其中一个就要求显示空白
					auto configData = setInfoIter->second;
					if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
						|| configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0)
					{
						wareLenTitle << tmpTitles[0];
					}

					// 要求显示主吸光度
					if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
						&& tmpTitles.size() >= 2)
					{
						wareLenTitle << tmpTitles[1];
					}

					// 要求显示次吸光度
					if (configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0
						&& tmpTitles.size() >= 3)
					{
						wareLenTitle << tmpTitles[2];
					}

					// 要求显示所有吸光度
					if (configData.count(ExportInfoEn::EIE_ALL_WAVE_ABS) > 0
						&& tmpTitles.size() >= 4)
					{
						int index = 0;
						for (const auto& titles : tmpTitles)
						{
							if (index++ < 3)
							{
								continue;
							}

							wareLenTitle << titles;
						}
					}

                }
                // 导出校准项目
                else
                {
					// 校准项目，需要记录重复多少次
					sampleExport.strCaliCount = QString::number(timesFunc(item->sampleInfoId, item->assayCode));
					QVariantList tmpTitles;
					auto setInfoIter = savedata.find(ExportType::CH_CALI_TYPE);
					if (setInfoIter == savedata.end())
					{
						continue;
					}

					auto spAssayTestResult = GetAssayResultByid(item->firstTestResultKey.assayTestResultId);
					if (spAssayTestResult == Q_NULLPTR)
					{
						continue;
					}

					auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
					if (!isGet)
					{
						continue;
					}

					isGet = GetResultAbs(spAssayTestResult, sampleExport, tmpTitles);
					if (!isGet)
					{
						continue;
					}

					auto rawData = GetExportData(isXlsx, UType, sampleExport, setInfoIter->second);
					content_ch_cali.push_back(rawData);
					// for pdf
					vecSamples.push_back(sampleExport);

					if (tmpTitles.empty())
					{
						continue;
					}

					int dataCount = 0;
					for (const auto& data : content_ch_cali)
					{
						if (!data.canConvert<QVariantList>())
						{
							continue;
						}

						auto dataList = data.toList();
						dataCount += dataList.size();
					}

					int titleCount = 0;
					for (const auto& data : wareLenTitle_cali)
					{
						if (!data.canConvert<QVariantList>())
						{
							continue;
						}

						auto dataList = data.toList();
						titleCount += dataList.size();
					}

					if (dataCount < titleCount)
					{
						continue;
					}

					if (tmpTitles.size() < wareLenTitle_cali.size())
					{
						continue;
					}

					wareLenTitle_cali.clear();
					// 主次吸光度只要显示其中一个就要求显示空白
					auto configData = setInfoIter->second;
					if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
						|| configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0)
					{
						wareLenTitle_cali << tmpTitles[0];
					}

					if (configData.count(ExportInfoEn::EIE_PRIMARY_WAVE_ABS) > 0
						&& tmpTitles.size() >= 2)
					{
						wareLenTitle_cali << tmpTitles[1];
					}

					// 要求显示次吸光度
					if (configData.count(ExportInfoEn::EIE_DEPUTY_WAVE_ABS) > 0
						&& tmpTitles.size() >= 3)
					{
						wareLenTitle_cali << tmpTitles[2];
					}

					// 要求显示所有吸光度
					if (configData.count(ExportInfoEn::EIE_ALL_WAVE_ABS) > 0
						&& tmpTitles.size() >= 4)
					{
						int index = 0;
						for (const auto& titles : tmpTitles)
						{
							if (index++ < 3)
							{
								continue;
							}
							wareLenTitle_cali << titles;
						}
					}
                }
            }
        }
        // 计算项目
        else if (item->assayCode >= tf::AssayCodeRange::CALC_RANGE_MIN && item->assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
        {
			// 计算项目暂时显示在生化的列表中
			auto iter = savedata.find(ExportType::CH_NORMAL_AND_QC_TYPE);
			if (iter == savedata.end())
			{
				continue;
			}

			// 是否含有生化项目，对于联机版生化页面导出，当含有生化项目的时候，才导出对应的计算项目
			bool isHaveCh = CommonInformationManager::GetInstance()->IsCaClHaveChorIM(item->assayCode);
			if (!isHaveCh)
			{
				continue;
			}

			if (item->firstTestResultKey.assayTestResultId > 0)
			{
				auto isGet = GetResultExport(item->firstTestResultKey, item->assayCode, sampleExport);
				if (!isGet)
				{
					continue;
				}

				auto rawData = GetExportData(isXlsx, UType, sampleExport, iter->second);
				content_ch.push_back(rawData);
				// for pdf
				vecSamples.push_back(sampleExport);
			}

			if (item->lastTestResultKey.assayTestResultId > 0)
			{
				auto isGet = GetResultExport(item->lastTestResultKey, item->assayCode, sampleExport);
				if (!isGet)
				{
					continue;
				}

				auto rawData = GetExportData(isXlsx, UType, sampleExport, iter->second);
				content_ch.push_back(rawData);
				// for pdf
				vecSamples.push_back(sampleExport);
			}
        }
    }

	for (const auto& title : wareLenTitle)
	{
		for (const auto& data : title.toList())
		{
			title_ch_normal << (data.toString() + "\t");
		}
	}

	for (const auto& title : wareLenTitle_cali)
	{
		for (const auto& data : title.toList())
		{
			title_ch_cali << (data.toString() + "\t");
		}
	}

	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 30);

    // 导出文件
    FileExporter fileExporter;
    auto& classify = fileExporter.GetDataClassify();
    classify.clear();
    // 要输出的信息（每一个QString为一行数据，列之间的数据用'\t'分割）
    QVariantList strExportTextList_ch;
    QVariantList strExportTextList_ch_cali;
    QVariantList strExportTextList_ise;
    QVariantList strExportTextList_ise_cali;
    if (!content_ch.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("生化_常规"));
        exTask.title = std::move(title_ch_normal);
        exTask.contents = std::move(content_ch);
        classify.append(exTask);
    }
    if (!content_ch_cali.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("生化_校准"));
        exTask.title = std::move(title_ch_cali);
        exTask.contents = std::move(content_ch_cali);
        classify.append(exTask);
    }
    if (!content_ise.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("电解质_常规"));
        exTask.title = std::move(title_ise_normal);
        exTask.contents = std::move(content_ise);
        classify.append(exTask);
    }
    if (!content_ise_cali.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("电解质_校准"));
        exTask.title = std::move(title_ise_cali);
        exTask.contents = std::move(content_ise_cali);
        classify.append(exTask);
    }

    fileExporter.ExportInfoToFileByClassify(path, classify, vecSamples);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE, false);                 // 关闭进度条
	return true;
}

///
///  @brief 创建表头
///
///
///  @param[out]   strExportTextList  数据导出列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月22日，新建函数
///
void QSampleDataBrowse::CreateHeader(QStringList & strExportTextList)
{
    // 记录导出标题
    strExportTextList.push_back(std::move(QObject::tr("数据浏览")));

    // 记录导出表头
    QString strHeader("");
    strHeader += QObject::tr("序号") + "\t";
    strHeader += QObject::tr("样本号") + "\t";
    strHeader += QObject::tr("样本ID") + "\t";
    strHeader += QObject::tr("样本架号") + "\t";
    strHeader += QObject::tr("样本位置") + "\t";

    strHeader += QObject::tr("测试项目") + "\t";
    strHeader += QObject::tr("信号值") + "\t";
    strHeader += QObject::tr("结果") + "\t";
    strHeader += QObject::tr("单位") + "\t";
    strHeader += QObject::tr("结果状态") + "\t";
    strHeader += QObject::tr("稀释倍数") + "\t";
    strHeader += QObject::tr("检测完成时间") + "\t";
    strHeader += QObject::tr("底物液批号") + "\t";
    strHeader += QObject::tr("底物液瓶号") + "\t";
    strHeader += QObject::tr("清洗缓冲液批号") + "\t";
    strHeader += QObject::tr("反应杯批号") + "\t";
    strHeader += QObject::tr("试剂批号") + "\t";
    strHeader += QObject::tr("试剂瓶号") + "\t";
    strHeader += QObject::tr("试剂上机时间") + "\t";
    strHeader += QObject::tr("校准曲线信息") + "\t";

    strHeader += QObject::tr("复查信号值") + "\t";
    strHeader += QObject::tr("复查结果") + "\t";
    strHeader += QObject::tr("单位") + "\t";
    strHeader += QObject::tr("复查结果状态") + "\t";
    strHeader += QObject::tr("复查稀释倍数") + "\t";
    strHeader += QObject::tr("复查完成时间") + "\t";
    strHeader += QObject::tr("底物液批号") + "\t";
    strHeader += QObject::tr("底物液瓶号") + "\t";
    strHeader += QObject::tr("清洗缓冲液批号") + "\t";
    strHeader += QObject::tr("反应杯批号") + "\t";
    strHeader += QObject::tr("试剂批号") + "\t";
    strHeader += QObject::tr("试剂瓶号") + "\t";
    strHeader += QObject::tr("试剂上机时间") + "\t";
    strHeader += QObject::tr("校准曲线信息") + "\t";
    strHeader += QObject::tr("模块") + "\t";

    strExportTextList.push_back(std::move(strHeader));
}

void QSampleDataBrowse::SetSampleColumnWidth()
{
    ui->sample_list->horizontalHeader()->setMinimumSectionSize(45);
    ui->sample_list->setColumnWidth(0, 45);
    ui->sample_list->setColumnWidth(1, 63);
    ui->sample_list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->sample_list->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	OnUpdateColumn();
}

void QSampleDataBrowse::SetAssayColumnWidth()
{
    ui->show_assay_list->horizontalHeader()->setMinimumSectionSize(45);
    ui->show_assay_list->setColumnWidth(0, 45);
    ui->show_assay_list->setColumnWidth(1, 63);
    ui->show_assay_list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->show_assay_list->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
	OnUpdateColumn();
}

void QSampleDataBrowse::OnShowBySample()
{
    auto pSampleHeader = dynamic_cast<SortHeaderView*>(ui->sample_list->horizontalHeader());
    if (pSampleHeader != nullptr)
    {
        pSampleHeader->ResetAllIndex();
    }

    ui->stackedWidget->setCurrentIndex(0);
    QSampleAssayModel::Instance().SetSampleModuleType(QSampleAssayModel::VIEWMOUDLE::DATABROWSE);
    if (m_filterModule == nullptr)
    {
        m_filterModule = new QSampleFilterDataModule(/*ui->sample_list*/);
    }

    m_pageChange = true;
    m_filterModule->setSourceModel(&QSampleAssayModel::Instance());
    ui->sample_list->setModel(m_filterModule);
    SetSampleColumnWidth();

	ui->sample_list->setFocus();
    // 设置数据关注行
	auto row = QSampleAssayModel::Instance().GetCurrentRow();
	auto currentIndex = m_filterModule->mapFromSource(QSampleAssayModel::Instance().index(row, 2));
	emit ui->sample_list->clicked(currentIndex);
	ui->sample_list->setCurrentIndex(currentIndex);
	// modify bug0011806
	QSampleAssayModel::Instance().ClearSelectedFlag(0);
	QSampleAssayModel::Instance().SetSelectFlag(0, row, true, false);
    // 设置页面允许功能状态
    PageAllowFun();
    // 设置各按钮状态
    UpdateButtonStatus();

	// 恢复原序
	ProcOnSortBy(ui->sample_list, dynamic_cast<SortHeaderView*>(ui->sample_list->horizontalHeader()), 0, SortHeaderView::NoOrder);
}

void QSampleDataBrowse::OnShowByAssay()
{
    auto pShowAssayHeader = dynamic_cast<SortHeaderView*>(ui->show_assay_list->horizontalHeader());
    if (pShowAssayHeader != nullptr)
    {
        pShowAssayHeader->ResetAllIndex();
    }

    ui->stackedWidget->setCurrentIndex(1);
    QSampleAssayModel::Instance().SetSampleModuleType(QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE);
    if (m_filterModule == nullptr)
    {
        m_filterModule = new QSampleFilterDataModule(/*ui->show_assay_list*/);
    }

	ui->show_assay_list->setFocus();
    m_pageChange = true;
    m_filterModule->setSourceModel(&QSampleAssayModel::Instance());
    ui->show_assay_list->setModel(m_filterModule);
    SetAssayColumnWidth();

    if (m_assayButton == Q_NULLPTR)
    {
        auto header = ui->show_assay_list->horizontalHeader();
        m_assayButton = new CustomButton("", header);
        m_assayButton->setObjectName(QStringLiteral("tooltip_item_btn"));
		QString tips = CommonInformationManager::GetInstance()->GetTipsContent(m_workSet.aiRecognition);
        m_assayButton->setToolTip(tips);
        // 设置按钮的位置
        m_assayButton->setGeometry(header->sectionViewportPosition(1) + header->sectionSize(1) - 10, 15, 50, header->height());
        m_assayButton->show();

        // 选择模式，增加移动
        connect(ui->show_assay_list->selectionModel(), &QItemSelectionModel::currentRowChanged,
            [=](const QModelIndex& current, const QModelIndex& pre)
        {
            if (!current.isValid())
            {
                return;
            }

			if (m_filterModule != Q_NULLPTR)
			{
				bool isChanged = m_filterModule->IsRowCountChanged();
				if (isChanged)
				{
					ULOG(LOG_INFO, "%s(the line count is changed)", __FUNCTION__);
					m_filterModule->SetCountChangeFlag(false);
					return;
				}

				int rowCount = m_filterModule->rowCount();
				if (rowCount <= 1)
				{
					return;
				}
			}

			int row = current.row();
			int column = current.column();
            if (!CheckIndexRange(current.row()))
            {
                ULOG(LOG_INFO, "%s(The currentrow is out of range)", __FUNCTION__);
                return;
            }

            // 第0列-checkbox
            if (0 == column /*&& !m_pageChange*/)
            {
                return;
            }

            m_pageChange = false;
            if (current == pre)
            {
                return;
            }

            if (!m_controlModify)
            {
                ui->show_assay_list->clearSelection();
                QSampleAssayModel::Instance().ClearSelectedFlag(1);
            }

            DealCurrentIndex(current, false);

            QSampleAssayModel::Instance().SetCurrentSampleRow(current.row());
            UpdateButtonStatus();
        });
    }

    // 设置数据关注行
    auto row = QSampleAssayModel::Instance().GetCurrentRow();
	auto index = m_filterModule->mapFromSource(QSampleAssayModel::Instance().index(row, 2));
    ui->show_assay_list->setCurrentIndex(index);
	// modify bug0011806
	QSampleAssayModel::Instance().ClearSelectedFlag(1);
	QSampleAssayModel::Instance().SetSelectFlag(1, row, true, false);
    // 设置页面允许功能状态
    PageAllowFun();
    // 设置各按钮状态
    UpdateButtonStatus();

	// 恢复原序
	ProcOnSortBy(ui->show_assay_list, dynamic_cast<SortHeaderView*>(ui->show_assay_list->horizontalHeader()), 0, SortHeaderView::NoOrder);
}

///
/// @brief 设备状态更新
///
/// @param[in]  deviceInfo  设备状态信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月7日，新建函数
///
void QSampleDataBrowse::OnDeviceUpdate(class tf::DeviceInfo deviceInfo)
{
    return;
    // 运行状态下不能删除样本
    if (deviceInfo.__isset.status && deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
    {
        ui->del_Button->setEnabled(false);
        //ui->flat_del_all->setEnabled(false);
        ui->sample_list->setCurrentIndex(QModelIndex());
        ResetAssayModel();
    }
    else
    {
        ui->del_Button->setEnabled(true);
        //ui->flat_del_all->setEnabled(true);
    }
}

///
/// @brief 下一条
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
void QSampleDataBrowse::OnMoveBtn(bool direction)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QSampleFilterDataModule* sampleProxyMoudel = Q_NULLPTR;
    QModelIndex curModIdx;
    QTableView* list = Q_NULLPTR;
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        // 获取登记表的当前索引和标准模型
        curModIdx = ui->sample_list->currentIndex();
        list = ui->sample_list;
        // 判断是否选中有效索引
        if (!curModIdx.isValid())
        {
            ULOG(LOG_ERROR, "Select invalid item");
            return;
        }

        sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
        if (Q_NULLPTR == sampleProxyMoudel)
        {
            return;
        }

        // 获取选中项目数据
        auto sampleData = sampleProxyMoudel->GetSampleByIndex(curModIdx);
        if (!sampleData)
        {
            ULOG(LOG_WARN, "%s(), sampleData", __FUNCTION__);
            return;
        }
    }
    else
    {
        // 获取当前行号
        curModIdx = ui->show_assay_list->currentIndex();
        list = ui->show_assay_list;
        if (!curModIdx.isValid())
        {
            ULOG(LOG_WARN, "%s(), !index.isValid()", __FUNCTION__);
            return;
        }

        // 获取样本的数据库主键
        sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->show_assay_list->model());
        if (sampleProxyMoudel == Q_NULLPTR)
        {
            ULOG(LOG_WARN, "%s(), pSampleProxyModel == Q_NULLPTR", __FUNCTION__);
            return;
        }

        // 获取选中项目数据
        auto testItemData = sampleProxyMoudel->GetTestItemByIndex(curModIdx);
        if (!testItemData)
        {
            ULOG(LOG_WARN, "%s(), testItemData", __FUNCTION__);
            return;
        }
    }

    if (direction)
    {
        // 获取选中项后一行
        int iNextRow = curModIdx.row() + 1;
        if (iNextRow <= sampleProxyMoudel->rowCount())
        {
            auto index = list->model()->index(iNextRow, 1);
            list->setCurrentIndex(index);
            // 刷新当前内容
			do
			{
				if (ShowSampleDetail(m_sampleDetailDialog))
				{
					break;
				}

				OnMoveBtn(direction);
			} while (false);
        }
    }
    else
    {
        // 获取选中项后一行
        int iNextRow = curModIdx.row() - 1;
        if (iNextRow >= 0)
        {
            auto index = list->model()->index(iNextRow, 1);
            list->setCurrentIndex(index);
            // 刷新当前内容
			do
			{
				if (ShowSampleDetail(m_sampleDetailDialog))
				{
					break;
				}

				OnMoveBtn(direction);
			} 
			while (false);
        }
    }

    // 设置各按钮状态
    UpdateButtonStatus();
}

///
/// @brief 数据改变的时候刷新页面数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月19日，新建函数
///
void QSampleDataBrowse::OnDataChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 刷新结果详情
    if (m_sampleDetailDialog != Q_NULLPTR && m_sampleDetailDialog->isVisible())
    {
        ShowSampleDetail(m_sampleDetailDialog);
    }

    // 更新数据
    OnUpdateAssay();
    // 设置各按钮状态
    UpdateButtonStatus();
}

///
/// @brief 手工传输
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月2日，新建函数
///
void QSampleDataBrowse::OnTransfer()
{
    if (m_transferDialog == Q_NULLPTR)
    {
        m_transferDialog = new QPushSampleToRemote(this);
        // 传输样本到Lis
        connect(m_transferDialog, &QPushSampleToRemote::selectResultIndex, this, [&](bool reCheck, int reCheckCnt)
        {
            int32_t transType = reCheck ? tf::LisTransferType::TRANSFER_LAST : tf::LisTransferType::TRANSFER_FIRST;
            DcsControlProxy::GetInstance()->PushSampleToLIS(m_transferDialog->GetSelectSamples(), transType, false);
        });
    }

    // 查找登记表中对应的选中模型和标准模型
    QItemSelectionModel* slectmodule = ui->sample_list->selectionModel();
    auto sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
    // 如果模型为空，则返回
    if (slectmodule == Q_NULLPTR || sampleProxyMoudel == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "Model is null");
        return;
    }

    // 获取选中范围
    QModelIndexList selectIndexs = slectmodule->selectedRows();
    // 逐行获取要删除的样本信息的数据库主键
    std::vector<int64_t> selectdbs;
    for (const auto& index : selectIndexs)
    {
        // 如果当前行为空，则跳过
        auto sample = sampleProxyMoudel->GetSampleByIndex(index);
        if (sample == boost::none)
            continue;

        // 样本值
        auto sampleValue = sample.value();
        selectdbs.push_back(sampleValue.id);
    }

    m_transferDialog->SetPageAttribute(selectdbs, /*selectdbs.size() == 1 ? GetMaxTestCount(selectdbs[0]) : */-1);
    m_transferDialog->SetWorkSet(m_workSet);

    // 需求：如果只选了一个样本，又没有复查，就不弹窗，直接传输首查结果
    bool bReCheck = false;
    if (selectdbs.size() == 1)
    {
        // 获取testItemId信息
        auto vecItem = DataPrivate::Instance().GetSampleTestItems(selectdbs[0]);

        for (auto& item : vecItem)
        {
            if (item->lastTestResultKey.assayTestResultId > 0)
            {
                bReCheck = true;
                break;
            }
        }

        // 直接上传首次检查
        if (!bReCheck)
        {
            DcsControlProxy::GetInstance()->PushSampleToLIS(selectdbs, tf::LisTransferType::TRANSFER_FIRST, false);
            return;
        }
    }

    // 设置不需要显示
    if (m_workSet.transferManual.first)
    {
        m_transferDialog->show();
    }
    else
    {
        int32_t transType = (m_workSet.transferManual.second == 0) ? tf::LisTransferType::TRANSFER_FIRST : tf::LisTransferType::TRANSFER_LAST;
        DcsControlProxy::GetInstance()->PushSampleToLIS(selectdbs, transType, false);
    }
}

///
/// @brief 复查样本
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月22日，新建函数
///
void QSampleDataBrowse::OnRecheckSample()
{
    if (m_recheckDialog == Q_NULLPTR)
    {
        m_recheckDialog = new QReCheckDialog(this);
        m_recheckDialog->SetMode(QReCheckDialog::Mode::dilu_mode);
    }

    if (QSampleAssayModel::Instance().GetModule() != QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        return;
    }

    // 获取当前index
    auto index = ui->sample_list->currentIndex();
    if (!index.isValid())
    {
        return;
    }

    auto sampleProxyMoudel = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
    if (sampleProxyMoudel == nullptr)
    {
        return;
    }

    auto sourceData = sampleProxyMoudel->GetSampleByIndex(index);
    if (!sourceData)
    {
        return;
    }

    m_sampleInfo = sourceData;
    if (m_sampleInfo.has_value())
    {
        tf::SampleInfo sampleInfo;
        sampleInfo.__set_id(m_sampleInfo.value().id);
        m_recheckDialog->SetSample(sampleInfo);
        m_recheckDialog->show();
    }
}

void QSampleDataBrowse::OnSampleListCurrentRowChanged(const QModelIndex& current, const QModelIndex& pre)
{
    int row = current.row();
    ULOG(LOG_INFO, "%s(selectionModel row: %d)", __FUNCTION__, row);

    if (!current.isValid())
    {
        ULOG(LOG_INFO, "%s(The currentIndex is inValid)", __FUNCTION__);
        return;
    }

    if (m_filterModule != Q_NULLPTR)
    {
        bool isChanged = m_filterModule->IsRowCountChanged();
        if (isChanged)
        {
            m_filterModule->SetCountChangeFlag(false);
            return;
        }

        int rowCount = m_filterModule->rowCount();
        if (rowCount <= 1)
        {
            ULOG(LOG_INFO, "%s(row count is %d)", __FUNCTION__, rowCount);
            return;
        }
    }

    if (!CheckIndexRange(row))
    {
        ULOG(LOG_INFO, "%s(The currentrow is out of range)", __FUNCTION__);
        return;
    }

    // 第0列-checkbox
    int column = current.column();
    if (0 == column /*&& !m_pageChange*/)
    {
        return;
    }

    m_pageChange = false;
    if (current == pre)
    {
        return;
    }

    if (!m_controlModify)
    {
        ui->sample_list->clearSelection();
        QSampleAssayModel::Instance().ClearSelectedFlag(0);
    }

    DealCurrentIndex(current);

    // 只显示当前的项目
    OnAssayChanged(current, true);
}

///
/// @brief 保存修改
///
/// @param[in]  type   1:修改审核 2：修改复查
///
/// @return true:表示修改成功，反之修改失败
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月28日，新建函数
///
bool QSampleDataBrowse::OnSaveModify(tf::SampleInfo sampleInfo)
{
    // 获取复查状态
    auto sampleKey = GetCurrentIndex();
    if (!sampleKey.has_value())
    {
        return false;
    }

    auto index = std::get<0>(sampleKey.value());
    auto list = std::get<1>(sampleKey.value());
    auto sampleProxyMoudel = std::get<2>(sampleKey.value());
    if (sampleProxyMoudel == Q_NULLPTR || !index.isValid() || list == Q_NULLPTR)
    {
        return false;
    }

    // 获取选中项目数据
    auto sampleData = sampleProxyMoudel->GetSampleByIndex(index);
    if (!sampleData)
    {
        ULOG(LOG_WARN, "%s(), !sampleData", __FUNCTION__);
        return false;
    }

    ::tf::SampleInfoQueryCond sampQryCond;
    ::tf::SampleInfoQueryResp sampQryResp;
    sampQryCond.__set_id(sampleData.value().id);
    // 查询数据的时候，把对应的项目信息页查询出来
    sampQryCond.__set_containTestItems(true);
    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
        || sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || sampQryResp.lstSampleInfos.empty())
    {
        return false;
    }

    // 设置审核
    if (sampleInfo.__isset.audit)
    {
        // 反向设置
        sampleInfo.__set_audit(!sampleData.value().audit);
    }

    // 设置项目
    if (sampleInfo.__isset.testItems && sampleInfo.testItems.size() > 0)
    {
        for (auto& testItem : sampQryResp.lstSampleInfos[0].testItems)
        {
            if (testItem.assayCode == sampleInfo.testItems[0].assayCode)
            {
                // 复查
                if (testItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED
                    && testItem.rerun == false)
                {
                    testItem.__set_rerun(true);
                    testItem.__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING);
                }
                else
                {
                    testItem.__set_rerun(false);
                    //testItem.__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED);
                }
            }
        }

        sampleInfo.__set_testItems(sampQryResp.lstSampleInfos[0].testItems);
    }

    sampleInfo.__set_id(sampleData.value().id);
    // 更新样本信息
    if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
    {
        return false;
    }

    // 设置各按钮状态
    UpdateButtonStatus();
    return true;
}


///
/// @brief  重新计算
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
bool QSampleDataBrowse::OnReCalcBtnClicked()
{
    if (m_reCalcDialog == nullptr)
    {
        m_reCalcDialog = new QReCalcDialog(this);
    }

    m_reCalcDialog->show();

    return true;
}

///
/// @brief 获取样本中项目测试次数最大值（可能样本中某个项目比其它项目测试次数更多）
///
/// @param[in]  sampleId  
///
/// @return 返回最多的次数
///
/// @par History:
/// @li 8090/YeHuaNing，2023年3月9日，新建函数
///
int QSampleDataBrowse::GetMaxTestCount(int64_t sampleId)
{
    // 样本中，项目测试次数
    int maxRecheckedIndex = 0;
    auto&& testItems = DataPrivate::Instance().GetSampleTestItems(sampleId);

    for (auto& testItem : testItems)
    {
        if (testItem == nullptr)
            continue;

        switch (CommonInformationManager::GetAssayClassify(testItem->assayCode))
        {
            //其它类型暂时作为计算项目
        case tf::AssayClassify::ASSAY_CLASSIFY_OTHER:
        {
            ::tf::CalcAssayTestResultQueryResp assayTestResult;
            ::tf::CalcAssayTestResultQueryCond queryAssaycond;
            queryAssaycond.__set_testItemId(testItem->id);

            if (!DcsControlProxy::GetInstance()->QueryCalcAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstCalcAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s()", __FUNCTION__);
                continue;
            }

            maxRecheckedIndex = maxRecheckedIndex < assayTestResult.lstCalcAssayTestResult.size() ? assayTestResult.lstCalcAssayTestResult.size() : maxRecheckedIndex;
        }
        break;
        // 生化项目
        case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
        {
            ch::tf::AssayTestResultQueryCond queryAssaycond;
            ch::tf::AssayTestResultQueryResp assayTestResult;
            // 根据测试项目的ID来查询结果
            queryAssaycond.__set_testItemId(testItem->id);

            if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s()", __FUNCTION__);
                continue;
            }

            maxRecheckedIndex = maxRecheckedIndex < assayTestResult.lstAssayTestResult.size() ? assayTestResult.lstAssayTestResult.size() : maxRecheckedIndex;
        }
        break;
        // 免疫项目
        case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
        {
            im::tf::AssayTestResultQueryCond queryAssaycond;
            im::tf::AssayTestResultQueryResp assayTestResult;
            // 根据测试项目的ID来查询结果
            queryAssaycond.__set_testItemId(testItem->id);

            if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s()", __FUNCTION__);
                continue;
            }

            maxRecheckedIndex = maxRecheckedIndex < assayTestResult.lstAssayTestResult.size() ? assayTestResult.lstAssayTestResult.size() : maxRecheckedIndex;
        }
        break;
        //电解质
        case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
        {
            ise::tf::AssayTestResultQueryCond queryAssaycond;
            ise::tf::AssayTestResultQueryResp assayTestResult;
            // 根据测试项目的ID来查询结果
            queryAssaycond.__set_testItemId(testItem->id);

            if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || assayTestResult.lstAssayTestResult.empty())
            {
                ULOG(LOG_ERROR, "%s()", __FUNCTION__);
                continue;
            }

            maxRecheckedIndex = maxRecheckedIndex < assayTestResult.lstAssayTestResult.size() ? assayTestResult.lstAssayTestResult.size() : maxRecheckedIndex;
        }
        break;
        default:
            break;
        }
    }

    return maxRecheckedIndex;
}

///
/// @brief 获取当前行的信息（1：总数量，2：当前数量，3：样本id）
///
///
/// @return (1：总数量，2：当前数量，3：样本id）
///
/// @par History:
/// @li 5774/WuHongTao，2023年3月27日，新建函数
///
bool QSampleDataBrowse::GetCurrentPatientKey(std::tuple<int, int, int64_t>& data)
{
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        // 判断样本登记表当前选中项是否为有效索引
        auto index = ui->sample_list->currentIndex();
        auto proxyModule = static_cast<QSampleFilterDataModule*>(ui->sample_list->model());
        if (nullptr == proxyModule || !index.isValid())
        {
            ULOG(LOG_ERROR, "%s(index is not valid/proxymodule is not valid)", __FUNCTION__);
            return false;
        }

        auto sample = proxyModule->GetSampleByIndex(index);
        if (!sample)
        {
            ULOG(LOG_ERROR, "%s(can not get the sample)", __FUNCTION__);
            return false;
        }

        std::get<0>(data) = proxyModule->rowCount();
        std::get<1>(data) = index.row();
        std::get<2>(data) = sample.value().id;
    }
    else
    {
        // 判断样本登记表当前选中项是否为有效索引
        auto itemIndex = ui->show_assay_list->currentIndex();
        auto proxyModule = static_cast<QSampleFilterDataModule*>(ui->show_assay_list->model());
        if (nullptr == proxyModule || !itemIndex.isValid())
        {
            ULOG(LOG_ERROR, "%s(index is not valid/proxymodule is not valid)", __FUNCTION__);
            return false;
        }

        auto testItem = proxyModule->GetTestItemByIndex(itemIndex);
        if (!testItem)
        {
            ULOG(LOG_ERROR, "%s(can not get the testItem)", __FUNCTION__);
            return false;
        }

        std::get<0>(data) = proxyModule->rowCount();
        std::get<1>(data) = itemIndex.row();
        std::get<2>(data) = testItem.value().sampleInfoId;
    }

    return true;
}

///
/// @brief 查询模糊匹配的行号
///
/// @param[in]  start    开始当前行
/// @param[in]  keyWord  匹配关键字
/// @param[in]  type     类型（0,：序号，1：条码）
/// @param[in]  showTime 第几次
///
/// @return 行号
///
/// @par History:
/// @li 5774/WuHongTao，2023年3月24日，新建函数
///
boost::optional<int> QSampleDataBrowse::GetIndexSearch(bool direction, int start, QString& keyWord, int type, int showTime)
{
    if (m_filterModule == Q_NULLPTR)
    {
        return boost::none;
    }

    int column = 1;
    // 0:代表按照序号匹配--找到序号对应的列
    if (type == 0)
    {
        // 必须重新获取当前的样本号的列，因为有可能会发生变化
		boost::optional<std::pair<int, int>> seqColumn = boost::none;
		// 按样本展示
		if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
		{
			seqColumn = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::SAMPLE, int(SampleColumn::COL::SEQNO));
		}
		// 按项目展示
		else
		{
			seqColumn = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::COLASSAY::SEQNO));
		}

        if (!seqColumn)
        {
            return boost::none;
        }

        column = seqColumn.value().first;
    }
    // 1:代表按照条码排序
    else
    {
        // 必须重新获取当前的样本条码的列，因为有可能会发生变化
		boost::optional<std::pair<int, int>> barCodeColumn = boost::none;
		// 按样本展示
		if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
		{
			barCodeColumn = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::SAMPLE, int(SampleColumn::COL::BARCODE));
		}
		// 按项目展示
		else
		{
			barCodeColumn = SampleColumn::Instance().GetAttributeByType(SampleColumn::SAMPLEMODE::ASSAYLIST, int(SampleColumn::COLASSAY::BARCODE));
		}

        if (!barCodeColumn)
        {
            return boost::none;
        }

        column = barCodeColumn.value().first;
    }

    if (direction)
    {
        int findTime = 0;
		// modify by wuht for bug3508
        for (int row = (start == -1)?0:(start + 1); row < m_filterModule->rowCount(); ++row)
        {
            QModelIndex sourceIndex = m_filterModule->mapToSource(m_filterModule->index(row, column));
            QString data = QSampleAssayModel::Instance().data(sourceIndex, Qt::DisplayRole).toString();
            if (data.contains(keyWord))
            {
                QModelIndex proxyIndex = m_filterModule->index(row, 0);
                return proxyIndex.row();
            }
        }
    }
    else
    {
        int findTime = 0;
			// modify by wuht for bug3508
        for (int row = (start < 1) ? 0 : (start - 1); row >= 0; --row)
        {
            QModelIndex sourceIndex = m_filterModule->mapToSource(m_filterModule->index(row, column));
            QString data = QSampleAssayModel::Instance().data(sourceIndex, Qt::DisplayRole).toString();
            if (data.contains(keyWord))
            {
                QModelIndex proxyIndex = m_filterModule->index(row, 0);
                return proxyIndex.row();
            }
        }
    }

    return boost::none;
}

///
/// @brief 窗口位置变化的消息
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2023年4月20日，新建函数
///
void QSampleDataBrowse::OnChangedRow(int value)
{
    QTableView* view = Q_NULLPTR;
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        view = ui->sample_list;
    }
    // 按照项目展示
    else if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        view = ui->show_assay_list;
    }
    else
    {
        return;
    }

    auto firtsRow = view->verticalScrollBar()->value();
    // 获取指向QAbstractItemModel对象的指针
    QAbstractItemModel* model = view->model();
    if (model == Q_NULLPTR)
    {
        return;
    }

    // 获取第一行在视口中的位置
    int firstRowViewportPos = view->rowViewportPosition(firtsRow);

    // 计算当前可见的行数和第一个可见行的索引
    int firstVisibleRow = qMax(0, view->rowAt(firstRowViewportPos));
    //int lastVisibleRow = qMin(model->rowCount() - 1, view->rowAt(firstRowViewportPos + view->viewport()->height()));
    auto rowCount = model->rowCount() - 1;
    auto computRow = view->rowAt(firstRowViewportPos + view->viewport()->height());
    int lastVisibleRow = qMin(rowCount, computRow);
    if (lastVisibleRow < 0)
    {
        lastVisibleRow = rowCount;
    }

    DataPrivate::Instance().SetRange(firstVisibleRow, lastVisibleRow);
}

///
/// @brief 处理样本上传Lis状态改变消息
///
/// @param[in]  vecSampleID		发生改变的样本ID
/// @param[in]  bStatus			发生改变的样本当前状态
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年5月29日，新建函数
///
void QSampleDataBrowse::OnAuditChanged(class std::vector<__int64, class std::allocator<__int64>> vecSampleID, bool bStatus)
{
	std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (pLoginUserInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't get UserInfo.");
		return;
	}

	std::vector<int64_t> needUpdateVec;
    for (int index = 0; index < vecSampleID.size(); ++index)
    {
        auto sampleID = vecSampleID[index];
        ::tf::SampleInfo sampleInfo;
        sampleInfo.__set_id(sampleID);
        sampleInfo.__set_audit(bStatus);

        // 更新样本信息
        if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
        {
            ULOG(LOG_ERROR, "ModifySampleInfo failed!");
            continue;
        }

		// 更新审核者信息	
		if (!WorkpageCommon::UpdateAduitInfo(sampleInfo.patientInfoId, sampleInfo.id, pLoginUserInfo->username))
		{
			ULOG(LOG_ERROR, "audit name, patient id: %id, sample id: %id failed!", pLoginUserInfo->username, sampleInfo.patientInfoId, sampleInfo.id);
			continue;
		}

		needUpdateVec.push_back(sampleID);
    }

	// 是否启用审核以后自动上传(bug3441)-历史数据
	// 避免上传两次
	/*
	if (m_cps.bUploadResults && m_cps.bUploadAfterAudit)
	{
		// 审核以后自动上传
		DcsControlProxy::GetInstance()->PushSampleToLIS(needUpdateVec, tf::LisTransferType::TRANSFER_AUTO, true);
	}*/
}

///
/// @brief 更新工作页面的按钮状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月15日，新建函数
///
void QSampleDataBrowse::OnUpdateButtonStatus()
{
    if (!DictionaryQueryManager::GetPageset(m_workSet))
    {
        ULOG(LOG_ERROR, "Failed to get workSet.");
        return;
    }

	// 更新标识
	if (m_statusShow != Q_NULLPTR)
	{
		QString tips = CommonInformationManager::GetInstance()->GetTipsContent(m_workSet.aiRecognition);
		m_statusShow->setToolTip(tips);
	}

	if (m_assayButton != Q_NULLPTR)
	{
		QString tips = CommonInformationManager::GetInstance()->GetTipsContent(m_workSet.aiRecognition);
		m_assayButton->setToolTip(tips);
	}

	// 设置工作设置状态
	QDataItemInfo::Instance().SetWorkSet(m_workSet);
    auto fSetVisable = [&](QPushButton* button, bool flag)
    {
        if (flag)
        {
            button->show();
        }
        else
        {
            button->hide();
        }
    };


    // 根据类型判断是否显示某些按钮
    auto softType = CommonInformationManager::GetInstance()->GetSoftWareType();
    // 导出
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    fSetVisable(ui->selectCheck, m_workSet.filter);
    ui->widget_2->setVisible(m_workSet.filter);
    fSetVisable(ui->result_detail_btn, m_workSet.detail);
    fSetVisable(ui->flat_examine, m_workSet.audit);
    fSetVisable(ui->flat_print, m_workSet.print);
    fSetVisable(ui->flat_manual_transfer, m_workSet.transfer);

    bool bShowExportBySample = userPms->IsPermissionShow(PSM_EXPORT_DATABROWER_SAMPLE);
    bool bShowExportByQc = userPms->IsPermissionShow(PSM_EXPORT_DATABROWER_QC);
    bool bShowExportByCali = userPms->IsPermissionShow(PSM_EXPORT_DATABROWER_CALI);
    fSetVisable(ui->export_btn, (bShowExportBySample || bShowExportByQc || bShowExportByCali) ? m_workSet.exported : false);
    fSetVisable(ui->patient_btn, m_workSet.patient);
    fSetVisable(ui->flat_recheck, m_workSet.recheck);
    fSetVisable(ui->del_Button, m_workSet.deleteData);
    fSetVisable(ui->flat_recalculate, (softType == SOFTWARE_TYPE::IMMUNE) ? m_workSet.reComput : false);

    m_filterModule->SetSampleShows(m_workSet.sampleShows);
	// 刷新显示
	if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
	{
		OnShowByAssay();
	}

	if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
	{
		OnShowBySample();
	}

    // 重新计算 PMS_RECAULCULATE(生免联机的时候也必须支持-0013619)
    if (softType == SOFTWARE_TYPE::IMMUNE || softType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
    {
        ui->flat_recalculate->setVisible(m_workSet.reComput && userPms->IsPermissionShow(PMS_RECAULCULATE));
    }
}

///
/// @brief 打印完毕更新状态
///
/// @param[in]  bIsPrinted: true：表示打印完成；false：表示预览，不更新状态  
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月21日，新建函数
/// @li 6889/ChenWei，2023年9月8日，添加参数 bIsPrinted
///
void QSampleDataBrowse::OnPrintFinsihed(bool bIsPrinted)
{
    if (!bIsPrinted)
        return;

    auto GetCurrentSample = [&](int64_t db)->std::shared_ptr<tf::SampleInfo>
    {
        ::tf::SampleInfoQueryCond sampQryCond;
        ::tf::SampleInfoQueryResp sampQryResp;
        sampQryCond.__set_id(db);
        sampQryCond.__set_containTestItems(true);

        // 执行查询条件
        if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
            || sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || sampQryResp.lstSampleInfos.empty())
        {
            return Q_NULLPTR;
        }

        return std::make_shared<tf::SampleInfo>(sampQryResp.lstSampleInfos[0]);
    };

    auto selecteds = DataPrivate::Instance().GetAllSelectFlag(0);
    for (const auto& db : selecteds)
    {
        auto spSample = GetCurrentSample(db);
        if (spSample == Q_NULLPTR)
        {
            continue;
        }
        spSample->__set_printed(true);

        // 更新样本信息
        if (!DcsControlProxy::GetInstance()->ModifySampleInfo(*spSample))
        {
            ULOG(LOG_ERROR, "ModifySampleInfo failed!");
            continue;
        }
    }
}

void QSampleDataBrowse::OnSeachSample(bool direction, bool isFirst)
{
	m_lastDirection = direction;
    QString keyword = ui->lineEdit->text();
    if (keyword.isEmpty())
    {
        TipDlg(tr("提示"), tr("查找关键字不能为空！")).exec();
        return;
    }

    QTableView* view = Q_NULLPTR;
    if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::DATABROWSE)
    {
        view = ui->sample_list;
    }
    // 按照项目展示
    else
    {
        view = ui->show_assay_list;
    }

	// modify by wuht for bug3508
    int startrow = -1;
    auto currentIndex = view->currentIndex();
	auto selectedSampels = DataPrivate::Instance().GetAllSelectFlag(0);
    if (currentIndex.isValid() 
		&& selectedSampels.size() != 0
		&& !isFirst)//不能是“查询按钮”，true：表示查询按钮
    {
		// modify for bug0011971 by wuht
        startrow = currentIndex.row();
    }

	QString warning = tr("当前为第一个查找结果，无法向上查找！");
	if (direction)
	{
		warning = tr("当前为最后一个查找结果，无法向下查找！");
	}

	// 对于查询按钮显示的警告内容
	if (isFirst)
	{
		warning = tr("未查询到符合条件的样本！");
	}

    auto row = this->GetIndexSearch(direction, startrow, keyword, ui->comboBox->currentData().toInt(), m_searchTime);
    if (!row || m_filterModule == Q_NULLPTR)
    {
		// 	0021448: [工作] 数据浏览界面查找输入框输入不存在的样本号，点击“▲”无合理提示不存在对应的样本 modify by chenjialin 20230921
        if (m_searchTime <= 0)
        {
			auto row = this->GetIndexSearch(true, -1, keyword, ui->comboBox->currentData().toInt(), m_searchTime);
			if (!row)
			{
				warning = tr("未查询到符合条件的样本！");
			}

            TipDlg(tr("提示"), warning).exec();
        }
		else
		{
			TipDlg(tr("提示"), warning).exec();
		}

        m_searchTime = 0;
        return;
    }

	// modify by wuht for bug3508
	if (row.value() == startrow)
	{
		TipDlg(tr("提示"), warning).exec();
		return;
	}

	// modify by wuht for bug3508
	if (currentIndex.isValid() && currentIndex.row() == row.value())
	{
		// bug0013849
		view->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		auto sourceIndex = m_filterModule->mapToSource(currentIndex);
		if (!sourceIndex.isValid())
		{
			return;
		}

		QSampleAssayModel::Instance().SetSelectFlag(0, sourceIndex.row(), true);
		return;
	}

	if (m_lastDirection == direction)
	{
		m_searchTime++;
	}
	else
	{
		m_searchTime = 0;
	}

    QModelIndex index = view->model()->index(row.value(), 1);
	// modify by wuht for bug3508
    view->setCurrentIndex(index);
	//QSampleAssayModel::Instance().SetSelectFlag(0, row.value(), true);
    view->scrollTo(index, QAbstractItemView::PositionAtCenter);
}

///
/// @brief 处理复查结果更新消息（历史数据）
///  
/// @param[in]  ri  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月14日，新建函数
///
void QSampleDataBrowse::OnRetestResultChanged(std::string deviceSN, int64_t sampleID, ::tf::HistoryBaseDataUserUseRetestUpdate ri)
{
    auto deviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSN);
    if (deviceInfo == nullptr)
    {
        return;
    }

    ::tf::TestItemQueryCond qryCond;
    qryCond.__set_sampleId(sampleID);
    ::tf::TestItemQueryResp qryResp;
    auto bRet = DcsControlProxy::GetInstance()->QueryTestItems(qryCond, qryResp);
    if (!bRet || qryResp.lstTestItems.empty())
    {
        return;
    }

    // 找到修改的测试项
    auto it = std::find_if(qryResp.lstTestItems.cbegin(), qryResp.lstTestItems.cend(), [ri](const tf::TestItem& item)
    {
        return item.id == ri.itemId;
    });
    if (it == qryResp.lstTestItems.cend())
    {
        return;
    }
    tf::TestItem testItem = *it;

    tf::TestResultKeyInfo testKey;
    testKey.__set_deviceType(deviceInfo->deviceType);
    testKey.__set_assayTestResultId(ri.userUseRetestResultId);
    testItem.__set_lastTestResultKey(testKey);
    testItem.__set_retestConc(std::stod(ri.reTestResult));
    testItem.__set_resultStatusCodes(ri.reResultStatu);

    if (!DcsControlProxy::GetInstance()->ModifyTestItemInfo(testItem))
    {
        return;
    }

}

QSampleFilterDataModule::QSampleFilterDataModule(QObject* parent /*= nullptr*/)
    :QSortFilterProxyModel(parent)
    , m_filterSamples(0)
    , m_preRowCount(0)
    , m_countChanged(false)
    , m_enableFilter(false)
	, m_iOrder(SortHeaderView::NoOrder)
{
    m_sampleFilterMap.clear();
    m_samplePriotityMap.clear();

    // 数据更新,查看数据量是否变化
    connect(this, &QSortFilterProxyModel::dataChanged, this,
        [&](QModelIndex indexStart, QModelIndex indexEnd, QVector<int>)
    {
        if (m_preRowCount != rowCount())
        {
            m_preRowCount = rowCount();
            m_countChanged = true;
        }
    });
}

QSampleFilterDataModule::~QSampleFilterDataModule()
{
}

bool QSampleFilterDataModule::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	// 按项目展示
	if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
	{
		std::shared_ptr<tf::TestItem> testItemData = DataPrivate::Instance().GetTestItemByRow(sourceRow);
		if (testItemData == Q_NULLPTR)
		{
			return false;
		}

		// 检查该样本类型是否设置为显示
		auto spSample = DataPrivate::Instance().GetSampleInfoByDb(testItemData->sampleInfoId);
		if (!const_cast<QSampleFilterDataModule*>(this)->CheckIsSetToShow(spSample))
		{
			return false;
		}

		// 若是计算项目（判断需要显示与否）
		if (CommonInformationManager::IsCalcAssay(testItemData->assayCode))
		{
			std::shared_ptr<::tf::CalcAssayInfo> pCalcItem = CommonInformationManager::GetInstance()->GetCalcAssayInfo(testItemData->assayCode);
			if (pCalcItem && !pCalcItem->display)
			{
				return false;
			}
		}

		// 在开启筛选的情况下
		if (!m_enableFilter)
		{
			return true;
		}

		return m_filterCondNew.IsPass(testItemData);
	}
	else
	{
		auto sampleData = DataPrivate::Instance().GetSampleByRow(sourceRow);
		if (!sampleData)
		{
			return false;
		}

		std::shared_ptr<tf::SampleInfo> spSample = std::make_shared<tf::SampleInfo>(sampleData.value());
		// 检查该样本类型是否设置为显示
		if (!const_cast<QSampleFilterDataModule*>(this)->CheckIsSetToShow(spSample))
		{
			return false;
		}

		// 在开启筛选的情况下
		if (!m_enableFilter)
		{
			return true;
		}

		bool isShow = m_filterCondNew.IsPass(spSample);
		// 当前行不显示的时候，检查是否需要隐藏选中的项目
		if (!isShow)
		{
			auto sourceData = QSampleDetailModel::Instance().GetCurrentSample();
			if (sourceData && sourceData.value().id == spSample->id)
			{
				QSampleDetailModel::Instance().SetData(boost::optional<tf::SampleInfo>());
			}
		}

		return isShow;
	}
}

bool QSampleFilterDataModule::IsRowCountChanged()
{
	ULOG(LOG_INFO, "%s(preCount: %d count: %d)", __FUNCTION__, m_preRowCount, rowCount());
    if (m_preRowCount != rowCount())
    {
        m_preRowCount = rowCount();
        return true;
    }

    return false;
}

///
/// @brief 通过位置获取样本数据
///
/// @param[in]  index  位置信息
///
/// @return 样本数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月25日，新建函数
///
boost::optional<tf::SampleInfo> QSampleFilterDataModule::GetSampleByIndex(const QModelIndex &index)
{
    auto sourceModule = qobject_cast<QSampleAssayModel*>(this->sourceModel());
    auto sourceIndex = this->mapToSource(index);
    sourceModule->SetCurrentSampleRow(sourceIndex.row());
    return DataPrivate::Instance().GetSampleByRow(sourceIndex.row());
}

///
/// @brief 根据位置获取项目数据
///
/// @param[in]  index  位置信息
///
/// @return 项目数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
boost::optional<tf::TestItem> QSampleFilterDataModule::GetTestItemByIndex(const QModelIndex &index)
{
    auto sourceModule = qobject_cast<QSampleAssayModel*>(this->sourceModel());
    auto sourceIndex = this->mapToSource(index);
    sourceModule->SetCurrentSampleRow(sourceIndex.row());

    auto testItemValue = DataPrivate::Instance().GetTestItemByRow(sourceIndex.row());
    if (testItemValue != Q_NULLPTR)
    {
        return (*DataPrivate::Instance().GetTestItemByRow(sourceIndex.row()));
    }
    else
    {
        return  boost::none;
    }
}

///
///  @brief 排序规则
///
bool QSampleFilterDataModule::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
	// 获取源model
	QSampleAssayModel* pSouceModel = dynamic_cast<QSampleAssayModel*>(sourceModel());
	if (pSouceModel == nullptr)
	{
		return false;
	}

	// 获取源model的样本类型
	auto optLeftType = pSouceModel->GetSampleTypeByIndex(source_left);
	auto optRightType = pSouceModel->GetSampleTypeByIndex(source_right);
	if (!optLeftType.has_value() || !optRightType.has_value())
	{
		return false;
	}

	{
		QMutexLocker lock(&m_priotityMapLock);

		auto leftPriotity = m_samplePriotityMap.find(optLeftType.value());
		if (leftPriotity == m_samplePriotityMap.end())
		{
			return false;
		}

		auto rightPriotity = m_samplePriotityMap.find(optRightType.value());
		if (rightPriotity == m_samplePriotityMap.end())
		{
			return false;
		}

		// 如果样本类型不同，先排序类型（这也是实现分组的关键）
		if (optLeftType.value() != optRightType.value())
		{
			bool bRet = leftPriotity->second < rightPriotity->second;
			return sortOrder() == Qt::AscendingOrder ? bRet : !bRet;
		}
	}

	// 如果是原序,按照数据库主键分类排序
	if (m_iOrder == SortHeaderView::NoOrder)
	{
		auto optLeftId = pSouceModel->GetIdByIndex(source_left);
		auto optRightId = pSouceModel->GetIdByIndex(source_right);

		if (!optLeftId.has_value() || !optRightId.has_value())
		{
			return false;
		}

		return optLeftId.value() < optRightId.value();
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
///  @brief 设置显示设置界面的配置
///
///
///  @param[in]   sampleShows  显示配置
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月27日，新建函数
///
void QSampleFilterDataModule::SetSampleShows(const std::vector<std::pair<bool, int>>& sampleShows)
{
    for (int i = 0; i < sampleShows.size(); ++i)
    {
        // 加锁防止多线程访问
        {
            QMutexLocker lock(&m_priotityMapLock);
            m_samplePriotityMap[::tf::SampleType::type(sampleShows[i].second)] = i;
        }

        {
            QMutexLocker lock(&m_filterMapLock);
            m_sampleFilterMap[::tf::SampleType::type(sampleShows[i].second)] = sampleShows[i].first;
        }
    }
}

///
///  @brief 检查该类型的样本是否需要显示
///
///
///  @param[in]   spSampleInfo  样本信息指针
///
///  @return	true 显示 false 隐藏
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月27日，新建函数
///
bool QSampleFilterDataModule::CheckIsSetToShow(const std::shared_ptr<tf::SampleInfo> spSampleInfo)
{
    QMutexLocker lock(&m_filterMapLock);
    if (spSampleInfo == nullptr)
    {
        return false;
    }
    auto iterMap = m_sampleFilterMap.find(spSampleInfo->sampleType);
    if (iterMap != m_sampleFilterMap.end())
    {
        return iterMap->second;
    }
    return true;
}

///
/// @brief 全选
///
/// @param[in]  mode  模式
///
/// @return true:选中成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月14日，新建函数
///
bool QSampleFilterDataModule::SetSelectedAll(int mode)
{
    auto sourceModule = qobject_cast<QSampleAssayModel*>(this->sourceModel());
    if (sourceModule == Q_NULLPTR)
    {
        return false;
    }

    int rowCount = this->rowCount();
    for (int row = 0; row < rowCount; row++)
    {
        auto index = this->index(row, 0);
        auto sourceIndex = this->mapToSource(index);
        if (!sourceIndex.isValid())
        {
            continue;
        }

		sourceModule->SetSelectFlag(mode, sourceIndex.row(), true, true);
	}

    return true;
}

///
/// @brief  设置排序方式
///
/// @param[in]  order  排序方式
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月7日，新建函数
///
void QSampleFilterDataModule::SetSortOrder(const int order)
{
	m_iOrder = order;
}

///
///  @brief 检查字符串是否是纯数字
///
///
///  @param[in]    data  数据
///  @param[out]   value  是数字就输出转换后的数字
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月27日，新建函数
///
bool QSampleFilterDataModule::IsNumber(const QVariant & data, double & value) const
{
    bool ok = false;
    value = data.toDouble(&ok);
    return ok;
}




