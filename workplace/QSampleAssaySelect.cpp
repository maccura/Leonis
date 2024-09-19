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

#include "QSampleAssaySelect.h"
#include "ui_QSampleAssaySelect.h"
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QCompleter>
#include <QScrollBar>

#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/QComDelegate.h"
#include "shared/QAssayTabWidgets.h"
#include "thrift/DcsControlProxy.h"

#include "QSampleAssayModel.h"
#include "SortHeaderView.h"
#include "QDialogBatchRegister.h"
#include "QPatientDialog.h"
#include "QReCheckDialog.h"
#include "WorkpageCommon.h"
#include "src/common/defs.h"
#include "src/common/StringUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ch/ChCommon.h"
#include "utility/UtilityCommonFunctions.h"
#include "manager/DictionaryQueryManager.h"

#define MAXLENGTH 25
#define MAXSEQNUM 999999999999
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return;\
}

QSampleAssaySelect::QSampleAssaySelect(QWidget *parent)
    : QWorkShellPage(parent)
    , m_isSeqChanged(false)
	, m_controlModify(false)
    , m_recheckDialog(Q_NULLPTR)
    , m_filterModule(Q_NULLPTR)
    , m_statusShow(Q_NULLPTR)
{
    ui = new Ui::QSampleAssaySelect();
    ui->setupUi(this);
    // 初始化样本列表
    InitalSampleData();
    // 初始化样本属性
    InitSampleAttribute();
}

QSampleAssaySelect::~QSampleAssaySelect()
{
}

void QSampleAssaySelect::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
	POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, tr("> 工作 > ") + tr("样本申请"));
    //去掉排序三角样式
    ui->SampleList->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    //-1则还原model默认顺序
    m_filterModule->sort(-1, Qt::DescendingOrder);
    auto pSampleHeader = dynamic_cast<SortHeaderView*>(ui->SampleList->horizontalHeader());
    if (pSampleHeader != nullptr)
    {
        pSampleHeader->ResetAllIndex();
    }

	// 重刷某些修改后的属性
	{
		RefreshCornerWidgets(0);
	}

	// modify for bug1780
	ui->SampleList->horizontalScrollBar()->setValue(0);

    // 开启刷新免疫试剂校准异常标志(刷新效率优化)
    if (!CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_I6000 }).empty())
    {
        QSystemMonitor::GetInstance().SetImRgntAbnFlagUpdateRealTime(true);
    }
}

void QSampleAssaySelect::hideEvent(QHideEvent *event)
{
    // 基类处理
    QWidget::hideEvent(event);

    // 停止刷新免疫试剂校准异常标志(刷新效率优化)
    if (!CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::DEVICE_TYPE_I6000 }).empty())
    {
        QSystemMonitor::GetInstance().SetImRgntAbnFlagUpdateRealTime(false);
    }
}

///
/// @brief 初始化样本列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月9日，新建函数
///
void QSampleAssaySelect::InitalSampleData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_filterModule == nullptr)
    {
        m_filterModule = new QSampleFilter(ui->SampleList);
    }

    auto frames = findChildren<QFrame*>();
    for (const auto& frame : frames)
    {
        if (frame->objectName().contains(QStringLiteral("apply_")))
        {
            m_sampleAppFrame.push_back(frame);
            frame->hide();
        }
        else if (frame->objectName().contains(QStringLiteral("list_")))
        {
            m_sampleListFrame.push_back(frame);
            frame->show();
        }
    }
    SortHeaderView *pSampleHeader = new SortHeaderView(Qt::Horizontal, ui->SampleList);
    pSampleHeader->setStretchLastSection(true);
    ui->SampleList->setHorizontalHeader(pSampleHeader);
    ui->SampleList->setItemDelegateForColumn(0, new QStatusDelegate(this));
	ui->SampleList->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
	ui->SampleList->setItemDelegateForColumn(2, new CReadOnlyDelegate(this));
    connect(pSampleHeader, &SortHeaderView::SortOrderChanged, this, [this, pSampleHeader](int logicIndex, SortHeaderView::SortOrder order)
    {
        if (logicIndex + 1 == static_cast<int>(SampleColumn::COL::STATUS))
        {
            return;
        }

        QTableView *view = ui->SampleList;
        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            //去掉排序三角样式
            pSampleHeader->setSortIndicator(-1, Qt::DescendingOrder);
            //-1则还原model默认顺序
            m_filterModule->sort(-1, Qt::DescendingOrder);
        }
        else
        {
            switch (order)
            {
            case SortHeaderView::DescOrder:
            {
                view->sortByColumn(logicIndex, Qt::DescendingOrder);
            }
            break;
            case SortHeaderView::AscOrder:
            {
                view->sortByColumn(logicIndex, Qt::AscendingOrder);
            }
            break;
            }
        }
    });

    // 按样本展示中复选框和？跟随
    connect(ui->SampleList->horizontalScrollBar(), &QScrollBar::valueChanged, this, [&](int value)
    {
        auto header = ui->SampleList->horizontalHeader();
        if (header == Q_NULLPTR)
        {
            return;
        }

        if (m_statusShow == Q_NULLPTR)
        {
            return;
        }

        m_statusShow->setGeometry(header->sectionViewportPosition(0) + header->sectionSize(0) - 10, 14, 50, header->height());
    });

    //去掉排序三角样式
    ui->SampleList->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    //-1则还原model默认顺序
    m_filterModule->sort(-1, Qt::DescendingOrder);
    pSampleHeader->ResetAllIndex();

    QSampleAssayModel::Instance().SetSampleModuleType(QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT);
    m_filterModule->setSourceModel(&QSampleAssayModel::Instance());
    ui->SampleList->setModel(m_filterModule);
    ui->SampleList->setSortingEnabled(true);
    // 注册事件处理对象
    ui->SampleList->viewport()->installEventFilter(this);
    InstallEventFilterRecursively(this);

    // 样本编号改变（序号模式和样本架模式）
    connect(ui->sample_no, &QLineEdit::textEdited, this, [&]() 
	{ 
		if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
		{
			m_isSeqChanged = true;
			m_assaySelected = false;
		}
	 });

    // 样本编号改变
    connect(ui->sample_bar, &QLineEdit::textEdited, this, [&]() 
	{ 
		if (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
		{
			m_isSeqChanged = true;
			m_assaySelected = false;
		}
	});
    // 设置点击样本列表事件的处理函数
    connect(ui->SampleList, &QTableView::clicked, this, [&](const QModelIndex& current)
    {
        ULOG(LOG_INFO, "%s()", __FUNCTION__);

		if (!m_controlModify && !m_shiftKey)
		{
			ui->SampleList->clearSelection();
		}

        OnSampleClicked(current);
		ui->SampleList->selectionModel()->select(current, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        UpdateButtonStatus();
    });

    // 选择模式，增加移动
    connect(ui->SampleList->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex& current, const QModelIndex& pre)
    {
        ULOG(LOG_INFO, "%s()", __FUNCTION__);
		if (!m_controlModify && !m_shiftKey)
		{
			ui->SampleList->clearSelection();
		}

        ui->SampleList->selectionModel()->select(current, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		OnSampleClicked(current);
    });

	// 样本申请的项目被选中的信号处理
	connect(ui->AssaySelectTabContainer, SIGNAL(assaySelected(int, int)), this, SLOT(OnAssaySelected(int, int)));

    // 复查按钮
    connect(ui->flat_recheck, SIGNAL(clicked()), this, SLOT(OnRecheck()));
    // 刷新页面信息
    connect(&DataPrivate::Instance(), &DataPrivate::sampleChanged, this, [&](DataPrivate::ACTIONTYPE mode, int start, int end)
    {
        ULOG(LOG_INFO, "%s(sample changed mode:%d, start:%d, end:%d)", __FUNCTION__, mode, start, end);
		// 不在样本申请页面不处理项目信息
		if (IsUpdateData())
		{
			return;
		}

        auto curModIdx = ui->SampleList->currentIndex();
		if (!curModIdx.isValid())
		{
			ULOG(LOG_INFO, "%s(The current Index is Invalid-mode:%d, start:%d, end:%d)", __FUNCTION__, mode, start, end);
			return;
		}

		// 获取代理model
		if (m_filterModule == Q_NULLPTR)
		{
			ULOG(LOG_INFO, "%s(m_filterModule is null)", __FUNCTION__);
			return;
		}

		// 查找对应的index
		auto sourceIndex = m_filterModule->mapToSource(curModIdx);
		if (!sourceIndex.isValid())
		{
			ULOG(LOG_INFO, "%s(curModIdx is invlaid)", __FUNCTION__);
			return;
		}

		int row = sourceIndex.row();
        if (row > end || row < start)
        {
            ULOG(LOG_INFO, "%s(out of range row:%d  mode:%d, start:%d, end:%d)", __FUNCTION__, row, mode, start, end);
            return;
        }

		if (mode == DataPrivate::ACTIONTYPE::ADDSAMPLE)
		{
			ULOG(LOG_INFO, "%s(The mode is not right mode:%d, start:%d, end:%d)", __FUNCTION__, mode, start, end);
			return;
		}

		// modify for bug3055 by wuht(删除样本的情况特殊处理)
		int rowCount = m_filterModule->rowCount();
		if (rowCount == 1 && mode == DataPrivate::ACTIONTYPE::DELETESAMPLE)
		{
			if (!m_isSeqChanged)
			{
				SetDefaultSampleDetail(true);
				ui->sample_no->setText("1");
			}
		}
		else
		{
			OnSampleClicked(curModIdx);
		}
    });

    std::vector<QPushButton*> functionButtons;
    functionButtons.push_back(ui->sampel_btn);
    functionButtons.push_back(ui->sampleApp_btn);
    functionButtons.push_back(ui->patient_btn);
    functionButtons.push_back(ui->flat_recheck);
    functionButtons.push_back(ui->flat_pre);
    functionButtons.push_back(ui->flat_next);
    functionButtons.push_back(ui->del_Button);
    //functionButtons.push_back(ui->batch_btn);
    //functionButtons.push_back(ui->flat_save);
    for (const auto& button : functionButtons)
    {
        connect(button, &QPushButton::clicked, this, [&]()
        {
            if (m_isSeqChanged && m_sampleTestMode != tf::TestMode::type::BARCODE_MODE)
            {
                OnSampleNoEditReturnPress();
            }

            if (m_isSeqChanged && m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
            {
                OnSampleBarEditReturnPress();
            }
        });
    }

    std::vector<QLineEdit*> edits;
    edits.push_back(ui->sample_no);
    edits.push_back(ui->sample_bar);
    edits.push_back(ui->sampel_rack);
    edits.push_back(ui->dilution_edit);
    edits.push_back(ui->sample_use_volume);
    edits.push_back(ui->comment_edit);
    for (const auto& edit : edits)
    {
        edit->installEventFilter(this);
    }

	// 样本无条码modify by wuhongtao for bug3273
	ui->sample_barcode_control->installEventFilter(this);

    std::vector<QComboBox*> selects;
    selects.push_back(ui->sample_postion);
    selects.push_back(ui->sample_type);
    selects.push_back(ui->sample_tube_type);
    selects.push_back(ui->dilution_box);
    for (const auto& select : selects)
    {
        select->installEventFilter(this);
    }

	// 限制备注输入字符数为15个
	ui->comment_edit->setMaxLength(15);
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
bool QSampleAssaySelect::eventFilter(QObject* target, QEvent* event)
{
    // qtabview和focusOut同时满足
	// 是否选中样本条码，当选中到样本条码的时候，跳转到对应的样本信息，而不是做ReturnPress
	// selectItem，用来控制
	bool selectItem = false;
    if (target == ui->SampleList->viewport())
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent != Q_NULLPTR)
            {
                auto index = ui->SampleList->indexAt(mouseEvent->pos());
                if (index.isValid())
				{
					selectItem = true;
				}
            }
        }
		// 冒泡显示 modify bug1615 by wuht
		else if (event->type() == QEvent::ToolTip)
		{
			QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
			if (helpEvent != Q_NULLPTR)
			{
				auto index = ui->SampleList->indexAt(helpEvent->pos());
				if (index.isValid())
				{
					auto rawData = ui->SampleList->model()->data(index, Qt::DisplayRole);
					QString tipContent = rawData.toString();
					QToolTip::showText(helpEvent->globalPos(), tipContent);
				}
			}
		}
    }

	// 回车(样本号)
	// 因为样本中号的输入中认为未输入任何字符属于非法，但是测试需要提示，故做特殊处理，具体Bug2554
	// 特殊处理，样本号为空的时候输出错误提示信息
	if (target == ui->sample_no
		&& event->type() == QEvent::KeyPress
		&& m_isSeqChanged
		&& ui->sample_no->text().isEmpty()
		&& (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE
			|| m_sampleTestMode == tf::TestMode::type::RACK_MODE))
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent != Q_NULLPTR 
			&& keyEvent->key() == Qt::Key_Return)
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未输入样本号，请重新输入！")));
			pTipDlg->exec();
			m_isSeqChanged = false;
			return QWorkShellPage::eventFilter(target, event);
		}
	}

    // 鼠标点击(样本号)
    if (target != ui->sample_no
        && event->type() == QEvent::MouseButtonPress
        && m_isSeqChanged
		&& m_sampleTestMode != tf::TestMode::type::BARCODE_MODE)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent != Q_NULLPTR
            && mouseEvent->button() == Qt::LeftButton)
        {
			if (!selectItem)
			{
				OnSampleNoEditReturnPress();
				m_isSeqChanged = false;
				if (m_assaySelected)
				{
					ui->AssaySelectTabContainer->Redo();
				}
				m_assaySelected = false;
			}
        }
    }

	// 回车(样本条码)
	// 因为样本中条码的输入中认为未输入任何字符属于非法，但是测试需要提示，故做特殊处理，具体Bug2554
	// 特殊处理，样本条码为空的时候输出错误提示信息
	if (target == ui->sample_bar
		&& event->type() == QEvent::KeyPress
		&& m_isSeqChanged
		&& ui->sample_bar->text().isEmpty()
		&& m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent != Q_NULLPTR
			&& keyEvent->key() == Qt::Key_Return)
		{
			std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("样本条码不能为空")));
			pTipDlg->exec();
			m_isSeqChanged = false;
			return QWorkShellPage::eventFilter(target, event);
		}
	}

    // 鼠标点击(样本条码)
    if (target != ui->sample_bar
        && event->type() == QEvent::MouseButtonPress
        && m_isSeqChanged
		&& m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent != Q_NULLPTR
            && mouseEvent->button() == Qt::LeftButton)
        {
			if (!selectItem)
			{
				OnSampleBarEditReturnPress();
				m_isSeqChanged = false;
				if (m_assaySelected)
				{
					ui->AssaySelectTabContainer->Redo();
				}
				m_assaySelected = false;
			}
        }
    }

// 	auto index = ui->SampleList->currentIndex();
// 	if (index.isValid())
// 	{
// 		ui->SampleList->setCurrentIndex(index);
// 	}

    return QWorkShellPage::eventFilter(target, event);
}

///
/// @brief 更新按钮状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月5日，新建函数
///
void QSampleAssaySelect::UpdateButtonStatus()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->patient_btn->setEnabled(false);
    ui->flat_recheck->setEnabled(false);
    ui->flat_pre->setEnabled(false);
    ui->flat_next->setEnabled(false);
    ui->del_Button->setEnabled(false);
    ui->batch_btn->setEnabled(false);
    ui->flat_save->setEnabled(true);

    QItemSelectionModel* selectModule = ui->SampleList->selectionModel();
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (Q_NULLPTR == sampleProxyMoudel || selectModule == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(sampleProxyMoudel or selectModule is nullptr)", __FUNCTION__);
        return;
    }

    // 没有任何样本的时候，样本号默认为1(并且要求用户修改的样本好不能被覆盖)
    if (!m_isSeqChanged 
		&& (sampleProxyMoudel->rowCount() == 0) 
		&& m_sampleTestMode != tf::TestMode::type::BARCODE_MODE)
    {
        ui->sample_no->setText("1");
    }

    ui->batch_btn->setEnabled(true);
    // 获取选中的最后一个值
    QModelIndexList selectIndexs = selectModule->selectedRows();
    if (selectIndexs.count() <= 0)
    {
        return;
    }

    auto index = selectIndexs[selectIndexs.count() - 1];
    auto sample = sampleProxyMoudel->GetSampleByIndex(index);
    // 若没有值，则返回
    if (!sample)
    {
        return;
    }

    // 判断模式是否一致
    bool isModeSame = false;
    if ((m_sampleTestMode == tf::TestMode::type::BARCODE_MODE) && (sample->testMode == tf::TestMode::type::BARCODE_MODE)
        || (m_sampleTestMode != tf::TestMode::type::BARCODE_MODE) && (sample->testMode != tf::TestMode::type::BARCODE_MODE))
    {
        isModeSame = true;
    }

    // 模式不一致，批量添加不允许
    if (!isModeSame)
    {
        ui->batch_btn->setEnabled(false);
    }

    int totalCount = sampleProxyMoudel->rowCount();

    // 只有一条数据
    if (totalCount == 1)
    {
        ui->flat_pre->setEnabled(false);
        ui->flat_next->setEnabled(false);
    }
    // 已经是第一条了
    else if (index.row() == 0)
    {
        ui->flat_pre->setEnabled(false);
        ui->flat_next->setEnabled(true);
    }
    // 已经是最后一条了
    else if (index.row() >= (totalCount - 1))
    {
        ui->flat_pre->setEnabled(true);
        ui->flat_next->setEnabled(false);
    }
    else
    {
        ui->flat_pre->setEnabled(true);
        ui->flat_next->setEnabled(true);
    }

	// 当多选的时候，患者信息置灰（bug3295 modify by wuhongtao）
	if (selectIndexs.count() == 1)
	{
		ui->patient_btn->setEnabled(true);
	}
    ui->del_Button->setEnabled(true);

    // 已经审核的样本不允许被修改
    if (sample->__isset.audit && sample->audit)
    {
        ui->flat_save->setEnabled(false);
    }
    else
    {
        ui->flat_save->setEnabled(true);
    }

    auto testItems = DataPrivate::Instance().GetSampleTestItems(sample->id);

    bool hasFinished = false;
    for (const auto testItem : testItems)
    {
        if (testItem->__isset.status
            && (testItem->status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED
                || testItem->status == tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING
                && testItem->rerun))
        {
            hasFinished = true;
            break;
        }
    }

	// 有复查请求的样本不允许作为批量的样本
	if (hasFinished)
	{
		ui->batch_btn->setEnabled(false);
	}

    // 若有已经完成的项目则可以复查(同时要求样本不在测试中)，同时已经设置复查的样本也可以改变复查架号
    if (hasFinished
        && (sample->__isset.status && sample->status != tf::SampleStatus::SAMPLE_STATUS_TESTING)
        // 不允许已经审核的样本复查
        && (sample->__isset.audit && !sample->audit)
		// 必须在同等模式下才能修改，包括复查(modify bug0012029 by wuht)
		&& (sample->__isset.testMode && (sample->testMode == m_sampleTestMode)))
    {
        ui->flat_recheck->setEnabled(true);
    }

	// modify bug 0011767 by wuht
	if (ui->SampleList->isVisible())
	{
		ui->del_Button->setEnabled(true);
	}
	else
	{
		ui->del_Button->setEnabled(false);
	}
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
boost::optional<tf::SampleInfo> QSampleAssaySelect::GetCurrentSample()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取当前样本
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return boost::none;
    }

    auto sample = sampleProxyMoudel->GetSampleByIndex(ui->SampleList->currentIndex());
    if (!sample)
    {
        return boost::none;
    }

    ::tf::SampleInfoQueryCond sampQryCond;
    ::tf::SampleInfoQueryResp sampQryResp;
    sampQryCond.__set_id(sample.value().id);
    sampQryCond.__set_containTestItems(true);

    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
        || sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || sampQryResp.lstSampleInfos.empty())
    {
        return boost::none;
    }

    return boost::make_optional(sampQryResp.lstSampleInfos[0]);
}

///
/// @brief 获取下一个合适的空序号（序号模式）
///
/// @param[in]  start  序号起点（终点是最大序号，在其中找空置的序号使用）
///
/// @return 空置可用序号
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月3日，新建函数
///
cpp_int QSampleAssaySelect::GetRightSeqNo(cpp_int start)
{
    ULOG(LOG_INFO, "%s(start: %s)", __FUNCTION__, start.str());
    // 当序号大于最大值的时候，返回最大值
    if (start >= cpp_int(MAXSEQNUM))
    {
        return cpp_int(MAXSEQNUM);
    }

    // 获取最大样本号，做为终点
    ::tf::ResultStr  rst;
    if (!DcsControlProxy::GetInstance()->GetMaxSampleSeqno(rst, ui->emerge_btn->isChecked(), m_sampleTestMode)
        || rst.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "GetMaxSeqNo() failed!");
        return (start + 1);
    }

    // 必须首先检查是否是纯数字，如果不是，则返回
    if (!stringutil::IsPureDigit(rst.value))
    {
        return (start + 1);
    }

    auto endSeq = cpp_int(rst.value);
    if (start > endSeq)
    {
        return start;
    }

    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        if (endSeq >= cpp_int(MAXSEQNUM))
        {
            return cpp_int(MAXSEQNUM);
        }

        return (endSeq + 1);
    }

    QSampleAssayModel* assayMoudel = static_cast<QSampleAssayModel*>(sampleProxyMoudel->sourceModel());
    if (assayMoudel == nullptr)
    {
        if (endSeq >= cpp_int(MAXSEQNUM))
        {
            return cpp_int(MAXSEQNUM);
        }

        return (endSeq + 1);
    }

    auto i = start;
    while (i < endSeq)
    {
        // 查找当前序号是否被占用（病人样本类型）
        auto sampleData = DataPrivate::Instance().GetSampleBySeq(m_sampleTestMode, i.str(), tf::SampleType::SAMPLE_TYPE_PATIENT, ui->emerge_btn->isChecked());
        if (!sampleData)
        {
            // 没有被占用则返回
            return i;
        }

        i++;
    }

    if (endSeq >= cpp_int(MAXSEQNUM))
    {
        return cpp_int(MAXSEQNUM);
    }

    return (endSeq + 1);
}

///
/// @brief 鼠标点击事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
void QSampleAssaySelect::mousePressEvent(QMouseEvent* event)
{
	return;
    if (!ui->sample_no->geometry().contains(event->pos()) && m_isSeqChanged && m_sampleTestMode != tf::TestMode::type::BARCODE_MODE)
    {
        OnSampleNoEditReturnPress();
    }

    if (!ui->sample_bar->geometry().contains(event->pos()) && m_isSeqChanged && m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
    {
        OnSampleBarEditReturnPress();
    }
}

///
/// @brief 安装（递归）
///
/// @param[in]  object  对象
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
void QSampleAssaySelect::InstallEventFilterRecursively(QObject *object)
{
    if (object != Q_NULLPTR)
    {
        object->installEventFilter(this);
        for (auto& child : object->children())
        {
            child->installEventFilter(object);
        }
    }
}

void QSampleAssaySelect::keyPressEvent(QKeyEvent* event)
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

void QSampleAssaySelect::keyReleaseEvent(QKeyEvent* event)
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

///
/// @brief 初始化样本属性选项
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月9日，新建函数
///
void QSampleAssaySelect::InitSampleAttribute()
{
    // 初始化样本类型下拉框(如果存在生化设备，则多添加 脑脊液、浆膜腔积液 两种类型)
    UiCommon::Instance()->AssignSampleCombox(ui->sample_type);
    connect(ui->sample_type, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this,
        [&](int index)
    {
        // 若类型没有改变，不需要提醒
        auto type = ui->sample_type->currentData().toInt();
        if (type == m_currentSampleType)
        {
            return;
        }

        // 获取样本类型，保证已经测试的样本不能修改
        auto sampleInfo = GetCurrentSample();
        if (sampleInfo.has_value())
        {
            auto sampleData = sampleInfo.value();
            for (auto& testItem : sampleData.testItems)
            {
                if (testItem.status != tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING)
                {
                    ui->sample_type->setCurrentIndex(ui->sample_type->findData(m_currentSampleType));
                    TipDlg(tr("警告"), tr("当前样本不允许修改样本类型！")).exec();
                    return;
                }
            }
        }

        // 是否需要提醒(若没有选择项目则不需要提醒，否则需要提醒)
        bool isNeedWarning = false;
        std::vector<int64_t>			profileLst;				    // 组合项目列表
        std::vector<::tf::TestItem>		testItemLst;                // 非组合项目列表
		auto keyData = ui->AssaySelectTabContainer->GetSampleTestItemInfo(testItemLst);
        if (keyData.first != 0)
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("警告"), tr("切换样本类型会清空已选择的项目,继续吗?"), TipDlgType::TWO_BUTTON));
            if (pTipDlg->exec() == QDialog::Rejected)
            {
                // 不改变类型
                ui->sample_type->setCurrentIndex(ui->sample_type->findData(m_currentSampleType));
                return;
            }
        }

        // 设置当前样本类型
        m_currentSampleType = type;
        auto config = DictionaryQueryManager::GetInstance()->GetBackupOrder();
		if (config.autoTestSind && m_currentSampleType == tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ)
		{
			tf::TestItem testItem;
			testItem.__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING);
			testItem.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_L);
			std::vector<std::shared_ptr<tf::TestItem>> testItems;
			testItems.push_back(std::make_shared<tf::TestItem>(testItem));
			testItem.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_H);
			testItems.push_back(std::make_shared<tf::TestItem>(testItem));
			testItem.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_I);
			testItems.push_back(std::make_shared<tf::TestItem>(testItem));
			ui->AssaySelectTabContainer->UpdateTestItemStatus(testItems);
		}
		else
		{
			ui->AssaySelectTabContainer->RefreshCard();
		}

		ui->AssaySelectTabContainer->SetCurrentSampleType(type);
        // 更新NA,K,CL的状态
        QSystemMonitor::GetInstance().SetCurrentType(type);
        QSystemMonitor::GetInstance().OnUpdateSampleType();
    });

    // 初始化样本杯下拉框
    AddTfEnumItemToComBoBox(ui->sample_tube_type, ::tf::TubeType::TUBE_TYPE_NORMAL);
    AddTfEnumItemToComBoBox(ui->sample_tube_type, ::tf::TubeType::TUBE_TYPE_MICRO);
    AddTfEnumItemToComBoBox(ui->sample_tube_type, ::tf::TubeType::TUBE_TYPE_ORIGIN);
    AddTfEnumItemToComBoBox(ui->sample_tube_type, ::tf::TubeType::TUBE_TYPE_STORE);

    // 位置下拉框初始化
    ui->sample_postion->addItem(QStringLiteral(""), 0);
    for (int i = 1; i <= 5; i++)
    {
        ui->sample_postion->addItem(QString::number(i), i);
    }

    // 样本号编辑回车键槽函数
    connect(ui->dilution_box, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDilution(int)));
    // 稀释编辑回调
    connect(ui->dilution_edit, SIGNAL(textChanged(QString)), this, SLOT(OnDilutionText(QString)));
    // 选中
    connect(ui->AssaySelectTabContainer, SIGNAL(deviceTypeChanged(int)), this, SLOT(OnDilutionChanged(int)));

	// 仅当只有免疫的时候，设置为免疫的稀释倍数
	auto softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();
	if (softwareType == SOFTWARE_TYPE::IMMUNE)
	{
		OnDilutionChanged(1);
	}
	else
	{
		OnDilutionChanged(0);
	}

    // 初始化按钮状态
    UpdateButtonStatus();
    // 设置类型
    //ui->AssaySelectTabContainer->StartContainer(ASSAYTYPE1_WORKPAGE);
	KeyDataIn setData;
	setData.assayType = AssayTypeNew::ASSAYTYPE1_WORKPAGE;
	ui->AssaySelectTabContainer->SetKeyStatus(setData);
	// 初始化为标准
	OnDilution(SampleSize::STD_SIZE);
    // 样本号的限制
    ui->sample_no->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    // 样本架的限制
    ui->sampel_rack->setValidator(new QRegExpValidator(QRegExp(UI_REG_RACK_NUM), this));

    ui->SampleList->horizontalHeader()->setHighlightSections(false);
    ui->SampleList->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->SampleList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->SampleList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->SampleList->setColumnWidth(0, 63);
    ui->SampleList->setColumnWidth(1, 140);

    ui->SampleList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui->SampleList->horizontalHeader()->setStretchLastSection(true);
    ui->sample_use_volume->setEnabled(false);

    DetectionSetting detectionSetting;
    if (!DictionaryQueryManager::GetDetectionConfig(detectionSetting))
    {
        ULOG(LOG_ERROR, "Failed to get detectionsettings.");
        return;
    }

	// 改变模式
	m_sampleTestMode = tf::TestMode::type(detectionSetting.testMode);
    // 条码模式
    if (detectionSetting.testMode == ::tf::TestMode::BARCODE_MODE)
    {
        // 样本条码的限制(条码模式)
        ui->sample_no->setValidator(Q_NULLPTR);
        ui->sample_no->setMaxLength(MAXLENGTH);
        ui->sample_bar->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));
    }
    // 样本架模式
    else if (detectionSetting.testMode == ::tf::TestMode::RACK_MODE)
    {
        ui->sample_bar->setValidator(Q_NULLPTR);
        ui->sample_bar->setMaxLength(MAXLENGTH);
        ui->sample_no->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    }
    // 序号模式
    else
    {
        ui->sample_bar->setValidator(Q_NULLPTR);
        ui->sample_bar->setMaxLength(MAXLENGTH);
        ui->sample_no->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    }

    // 根据模式设置条码或者序号
    OnChangeMode(detectionSetting);

    m_paTientDialog = Q_NULLPTR;
    // 样本架模式
    if (m_sampleTestMode == tf::TestMode::type::RACK_MODE)
    {
        ui->sampel_rack->setEnabled(true);
        ui->sample_postion->setEnabled(true);
    }
    else
    {
        ui->sampel_rack->setEnabled(false);
        ui->sample_postion->setEnabled(false);
    }

    // 序号模式或者样本架模式下，隐藏
    if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
    {
        ui->sample_barcode_control->setVisible(false);
    }

    m_batchDialog = nullptr;

    // 样本号编辑回车键槽函数
    connect(ui->sample_no, SIGNAL(returnPressed()), this, SLOT(OnSampleNoEditReturnPress()));
    // 样本编码编辑回车键槽函数
    connect(ui->sample_bar, SIGNAL(returnPressed()), this, SLOT(OnSampleBarEditReturnPress()));
    // 样本号编辑完成
    // connect(ui->sample_no, SIGNAL(editingFinished()), this, SLOT(OnSampleNoEditReturnPress()));
    // 样本条码编辑完成
    // connect(ui->sample_bar, SIGNAL(editingFinished()), this, SLOT(OnSampleBarEditReturnPress()));
    // 显示样本列表
    connect(ui->sampel_btn, &QPushButton::clicked, this, [&]()
    {
        if (!ui->SampleList->isVisible())
        {
            ui->sampel_btn->setStyleSheet("color: #025bc7; background: url(:/Leonis/resource/image/icon-sampel-hv.png) left center no-repeat;");
            ui->del_Button->setEnabled(true);
            ui->SampleList->setFocus();
            ui->SampleList->show();
            for (const auto& frame : m_sampleListFrame)
            {
                frame->show();
            }

            for (const auto& frame : m_sampleAppFrame)
            {
                frame->hide();
            }

            UpdateButtonStatus();
        }
        else
        {
            ui->sampel_btn->setStyleSheet("color: #565656; background: url(:/Leonis/resource/image/icon-sampel.png) left center no-repeat;");
            ui->del_Button->setEnabled(false);
            ui->SampleList->hide();
            for (const auto& frame : m_sampleListFrame)
            {
                frame->hide();
            }

            for (const auto& frame : m_sampleAppFrame)
            {
                frame->show();
            }

            UpdateButtonStatus();
            // 展开的时候不能删除当前样本（bug0018477）
            ui->del_Button->setEnabled(false);
        }

    });
    ui->sampel_btn->setStyleSheet("color: #025bc7; background: url(:/Leonis/resource/image/icon-sampel-hv.png) left center no-repeat;");

    // 保存样本
    connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveSampleInfo()));
    // 批量输入
    connect(ui->batch_btn, SIGNAL(clicked()), this, SLOT(OnBatchInputBtnClicked()));
    // 样本申请按钮的处理槽函数
    connect(ui->sampleApp_btn, SIGNAL(clicked()), this, SLOT(OnClickNewBtn()));
    // 常规按钮
    connect(ui->normal_btn, &QPushButton::clicked, this, [&]()
    {
        // 序号模式下的常规
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE)
        {
            ui->sampel_rack->setEnabled(false);
            ui->sample_postion->setEnabled(false);
        }

        ui->sample_no_mark->setStyleSheet("color : black;");
        ui->sample_no_mark->setText("N");
        OnClickNewBtn();
    });

    // 急诊按钮
    connect(ui->emerge_btn, &QPushButton::clicked, this, [&]()
    {
        // 序号模式下的急诊需要使用架号和位置号来定位样本
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE)
        {
            ui->sampel_rack->setEnabled(true);
            ui->sample_postion->setEnabled(true);
        }

        ui->sample_no_mark->setStyleSheet("color : red;");
        ui->sample_no_mark->setText("E");
        OnClickNewBtn();
    });

    ui->sample_no_mark->setStyleSheet("color : black;");
    // 上一条按钮被按下
    connect(ui->flat_pre, SIGNAL(clicked()), this, SLOT(OnPreviousBtnClicked()));
    // 下一条按钮被按下
    connect(ui->flat_next, SIGNAL(clicked()), this, SLOT(OnNextBtnClicked()));
    // 删除按钮被按下
    connect(ui->del_Button, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));
    // 病人按钮被按下
    connect(ui->patient_btn, SIGNAL(clicked()), this, SLOT(OnPatientInfoBtn()));
    // 样本无条码被点击
    connect(ui->sample_barcode_control, SIGNAL(toggled(bool)), this, SLOT(OnSampleNoBarCodeBtn(bool)));
    // 更新项目选择信息
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, ui->AssaySelectTabContainer, OnUpdateView);
    // 条码/序号模式切换
    REGISTER_HANDLER(MSG_ID_DETECTION_UPDATE, this, OnChangeMode);
    // 工作页面的设置更新
    REGISTER_HANDLER(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE, this, OnUpdateButtonStatus);
    // 初始化按钮状态
    OnUpdateButtonStatus();

	auto headView = ui->SampleList->horizontalHeader();
	m_statusShow = new CustomButton("", headView);
	m_statusShow->setObjectName(QStringLiteral("tooltip_btn"));
	QString tips = CommonInformationManager::GetInstance()->GetTipsContent(m_workSet.aiRecognition);
	m_statusShow->setToolTip(tips);
	// 设置按钮的位置
	m_statusShow->setGeometry(headView->sectionViewportPosition(0) + headView->sectionSize(0) - 10, 14, 50, headView->height());
}

///
/// @brief 改变稀释倍数类型
///
/// @param[in]  type  0：代表生化-1代表免疫
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月22日，新建函数
///
void QSampleAssaySelect::OnDilutionChanged(int type)
{
    ULOG(LOG_INFO, "%s(type: %d)", __FUNCTION__, type);
	// modify bug3456 by wuht
	if (type < 0 || type >1)
	{
		return;
	}

	bool isMannual = false;
	// 手工稀释的选择
	int indexManual = -1;
    QCompleter* completer = Q_NULLPTR;
    // 生化类型
    if (type == 0)
    {
        QStringList valueList = { tr("常量"), tr("减量"), tr("增量"), tr("手工稀释"), tr("自动稀释")
        , "3","5", "10","20","50" };
		indexManual = 3;
        completer = new QCompleter(valueList, this);

		int index = ui->dilution_box->currentIndex();
		if (index == 1)
		{
			isMannual = true;
		}
		ui->dilution_box->clear();

        int i = 0;
        for (const auto& value : valueList)
        {
            if (i <= 4)
            {
                ui->dilution_box->addItem(value, i);
            }
            // 对于3，特殊处理
            else if (value == "3")
            {
                ui->dilution_box->addItem(value, -1);
            }
            else
            {
                ui->dilution_box->addItem(value, value.toInt());
            }

            i++;
        }
    }
    else
    {
		// 	0019578: [工作] 样本申请界面稀释倍数存在原倍与1倍重复，bug修复 modify by chenjianlin 20240425
        QStringList valueList = { tr("原倍"), tr("手工稀释"), /*"1",*/ "2", "4", "5", "10","20", "40", "50",
            "80", "100","200", "400", "500", "800", "1000","1600", "2000", "3000" ,"3200", "4000", "6400" };
		indexManual = 1;
        completer = new QCompleter(valueList, this);

		int index = ui->dilution_box->currentIndex();
		if (index == 3)
		{
			isMannual = true;
		}
		ui->dilution_box->clear();

        int i = 0;
        for (const auto& value : valueList)
        {
            // 常量--特殊处理
            if (i == 0)
            {
                ui->dilution_box->addItem(value, 0);
            }
            // 手工稀释--特殊处理
            else if (i == 1)
            {
                ui->dilution_box->addItem(value, 3);
            }
            // 对于“1”的特殊处理 需要大于SampleSize::USER_SIZE
		    /* else if (i == 2)
			{
				ui->dilution_box->addItem(value, 6);
			}*/
            // 对于“2”的特殊处理
            else if (i == 2)
            {
                ui->dilution_box->addItem(value, 7);
            }
            // 对于“4”的特殊处理
            else if (i == 3)
            {
                ui->dilution_box->addItem(value, 8);
            }
            else
            {
                ui->dilution_box->addItem(value, value.toInt());
            }

            i++;
        }
    }

    ui->dilution_box->setEditable(true);
    ui->dilution_box->setCompleter(completer);
    ui->dilution_box->setInsertPolicy(QComboBox::NoInsert);

	if (isMannual)
	{
		auto saveData = ui->dilution_edit->text();
		ui->dilution_box->setCurrentIndex(indexManual);
		ui->dilution_edit->setText(saveData);
	}
}

///
/// @brief 复查按钮
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月23日，新建函数
///
void QSampleAssaySelect::OnRecheck()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_recheckDialog == Q_NULLPTR)
    {
        m_recheckDialog = new QReCheckDialog(this);
        m_recheckDialog->SetMode(QReCheckDialog::Mode::simple_mode);
    }

    // 获取当前样本
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return;
    }

    auto sample = sampleProxyMoudel->GetSampleByIndex(ui->SampleList->currentIndex());
    if (!sample)
    {
        return;
    }

    // 获取选中样本的db_no
    tf::SampleInfo sampleInfo;
    GetCurrentSampleInfo(sampleInfo, false);
    sampleInfo.__set_id(sample.value().id);

    m_recheckDialog->SetSample(sampleInfo);
    m_recheckDialog->show();
}

void QSampleAssaySelect::OnAssaySelected(int postion, int assayCode)
{
	m_assaySelected = true;
}

///
/// @brief 改变检测模式
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月24日，新建函数
///
void QSampleAssaySelect::OnChangeMode(DetectionSetting detectionSetting)
{
    ULOG(LOG_INFO, "%s(testMode: %d)", __FUNCTION__, detectionSetting.testMode);
	// 改变模式
	m_sampleTestMode = tf::TestMode::type(detectionSetting.testMode);
    // 序号模式
    if (detectionSetting.testMode == ::tf::TestMode::SEQNO_MODE)
    {
        ui->sample_bar->setValidator(Q_NULLPTR);
        ui->sample_bar->setMaxLength(MAXLENGTH);
        ui->sample_no->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    }
    // 架号模式
    else if (detectionSetting.testMode == ::tf::TestMode::RACK_MODE)
    {
        ui->sample_bar->setValidator(Q_NULLPTR);
        ui->sample_bar->setMaxLength(MAXLENGTH);
        ui->sample_no->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    }
    // 条码模式
    else
    {
        ui->sample_bar->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));
        ui->sample_no->setValidator(Q_NULLPTR);
        ui->sample_no->setMaxLength(MAXLENGTH);
    }

    if (m_sampleTestMode == ::tf::TestMode::SEQNO_MODE)
    {
        ui->sample_barcode_control->setVisible(false);
        if (ui->normal_btn->isChecked())
        {
            ui->sample_no_mark->setStyleSheet("color : black;");
            ui->sample_no_mark->setText("N");
            ui->sampel_rack->setEnabled(false);
            ui->sample_postion->setEnabled(false);
        }
        else
        {
            ui->sample_no_mark->setStyleSheet("color : red;");
            ui->sample_no_mark->setText("E");
            ui->sampel_rack->setEnabled(true);
            ui->sample_postion->setEnabled(true);
        }
    }
    else if (m_sampleTestMode == ::tf::TestMode::BARCODE_MODE)
    {
        ui->sample_barcode_control->setVisible(true);
        if (ui->sample_barcode_control->isChecked())
        {
            ui->sampel_rack->setEnabled(true);
            ui->sample_postion->setEnabled(true);
        }
        else
        {
            ui->sampel_rack->setEnabled(false);
            ui->sample_postion->setEnabled(false);
        }
    }
    // 样本架模式架号和位置都是长期打开
    else
    {
        ui->sample_barcode_control->setVisible(false);
        ui->sampel_rack->setEnabled(true);
        ui->sample_postion->setEnabled(true);
    }

    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (Q_NULLPTR == sampleProxyMoudel)
    {
        return;
    }

    if ((sampleProxyMoudel->rowCount() == 0) && m_sampleTestMode != ::tf::TestMode::BARCODE_MODE)
    {
        ui->sample_no->setText("1");
    }
}

void QSampleAssaySelect::OnUpdateButtonStatus()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
    if (!DictionaryQueryManager::GetPageset(m_workSet))
    {
        ULOG(LOG_ERROR, "Failed to get workpage set.");
        return;
    }

	// 更新标识
	if (m_statusShow != Q_NULLPTR)
	{
		QString tips = CommonInformationManager::GetInstance()->GetTipsContent(m_workSet.aiRecognition);
		m_statusShow->setToolTip(tips);
	}

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

    fSetVisable(ui->patient_btn, m_workSet.patient);
    fSetVisable(ui->flat_recheck, m_workSet.recheck);
    fSetVisable(ui->del_Button, m_workSet.deleteData);
}

///
/// @brief 显示样本详细信息
///
/// @param[in]  sampleInfo  样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
void QSampleAssaySelect::ShowSampleDetail(tf::SampleInfo& sampleInfo)
{
    ULOG(LOG_INFO, "%s(sample_id : %d)", __FUNCTION__, sampleInfo.id);
    // 样本号
    QString seqNo = QString::fromStdString(sampleInfo.seqNo);
    ui->sample_no->setText(seqNo);
    // 条码号
    QString barCode = QString::fromStdString(sampleInfo.barcode);
    ui->sample_bar->setText(barCode);
    // 样本类型（尿，血，浆膜液等）
    ui->sample_type->setCurrentIndex(ui->sample_type->findData(sampleInfo.sampleSourceType));
    m_currentSampleType = sampleInfo.sampleSourceType;
    ui->AssaySelectTabContainer->SetCurrentSampleType(m_currentSampleType);
    // 设置架号
    QString rack = QString::fromStdString(sampleInfo.rack);
    ui->sampel_rack->setText(rack);
    // 加样量
    int volumn = GetSampleVolumn(sampleInfo);
    ui->sample_use_volume->setText(QString::number(double(double(volumn) / 10)));

    // 设置备注
    ui->comment_edit->setText(QString::fromStdString(sampleInfo.comment));
    // 设置加样量
    ui->sample_use_volume->setEnabled(false);
    // 设置位置号
    (sampleInfo.pos >= 1) ? ui->sample_postion->setCurrentIndex(sampleInfo.pos) : ui->sample_postion->setCurrentIndex(0);
    // 设置杯类型
    ui->sample_tube_type->setCurrentIndex(ui->sample_tube_type->findData(sampleInfo.tubeType));
    ui->emerge_btn->setChecked(false);
    ui->normal_btn->setChecked(false);

    // 样本无条码
    if (sampleInfo.__isset.barcodeException)
    {
        ui->sample_barcode_control->setChecked(sampleInfo.barcodeException);
    }

    // 急诊
    if (sampleInfo.stat)
    {
        ui->sample_no_mark->setStyleSheet("color : red;");
        ui->sample_no_mark->setText("E");
        ui->emerge_btn->setChecked(true);
        // 序号模式下的急诊需要使用架号和位置号来定位样本
        if (m_sampleTestMode == ::tf::TestMode::SEQNO_MODE)
        {
            ui->sampel_rack->setEnabled(true);
            ui->sample_postion->setEnabled(true);
        }
    }
    else
    {
        ui->sample_no_mark->setStyleSheet("color : black;");
        ui->sample_no_mark->setText("N");
        ui->normal_btn->setChecked(true);
        // 序号模式下的常规
        if (m_sampleTestMode == ::tf::TestMode::SEQNO_MODE)
        {
            ui->sampel_rack->setEnabled(false);
            ui->sample_postion->setEnabled(false);
        }
    }
}

///
/// @brief 设置空样本默认页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QSampleAssaySelect::SetDefaultSampleDetail(bool isDefault)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 样本号
    ui->sample_no->setText("");
    // 条码号
    ui->sample_bar->setText("");

    if (isDefault)
    {
        auto sampleSet = DictionaryQueryManager::GetInstance()->GetSampleShowSet();

        // 样本类型（尿，血，浆膜液等）
        ui->sample_type->setCurrentIndex(0);
        // 设置杯类型
        ui->sample_tube_type->setCurrentIndex(0);

        // 默认样本管类型
        if (sampleSet.size() > 0)
        {
            if (sampleSet[0] > 0)
            {
                ui->sample_tube_type->setCurrentIndex(sampleSet[0] - 1);
            }
        }

        // 默认样本类型
        if (sampleSet.size() > 1)
        {
            if (sampleSet[1] == 0)
            {
                ui->sample_type->setCurrentIndex(5);
            }
            else
            {
                ui->sample_type->setCurrentIndex(sampleSet[1] - 1);
            }
        }

		// modify for bug1784 by wuht
		// 设置样本无条码取消
		ui->sample_barcode_control->setChecked(false);
		// 设置位置号
		ui->sample_postion->setCurrentIndex(0);
		// 设置架号
		ui->sampel_rack->setText("");
    }

    // 获取当样本类型
    m_currentSampleType = ui->sample_type->currentData().toInt();
    // 设置备注
    ui->comment_edit->setText("");
    // 设置默认的稀释倍数
    ui->dilution_box->setCurrentIndex(0);
    ui->dilution_edit->setText("");
    // 设置加样量
    ui->sample_use_volume->setText("");
    ui->sample_use_volume->setEnabled(false);
    auto config = DictionaryQueryManager::GetInstance()->GetBackupOrder();
    if (config.autoTestSind && m_currentSampleType == tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ)
    {
		tf::TestItem testItem;
		testItem.__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING);
		testItem.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_L);
		std::vector<std::shared_ptr<tf::TestItem>> testItems;
		testItems.push_back(std::make_shared<tf::TestItem>(testItem));
		testItem.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_H);
		testItems.push_back(std::make_shared<tf::TestItem>(testItem));
		testItem.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_I);
		testItems.push_back(std::make_shared<tf::TestItem>(testItem));
		ui->AssaySelectTabContainer->UpdateTestItemStatus(testItems);
    }
	else
	{
		ui->AssaySelectTabContainer->RefreshCard();
	}

	// modify bug 0011893
	// 取消之前的审核
	ui->AssaySelectTabContainer->SetAudit(false);
	// 设置样本类型
	ui->AssaySelectTabContainer->SetCurrentSampleType(m_currentSampleType);
}

std::pair<int, int> QSampleAssaySelect::GetIncreasePostion(std::pair<int, int> samplePostion, bool isEmerge, bool isRetest)
{
	ULOG(LOG_INFO, "%s(%s, %s, rack:%d, postion:%d)", __FUNCTION__, isEmerge?"emerge":"Unemerge", isRetest?"reTest":"test", samplePostion.first, samplePostion.second);	
	int rack = samplePostion.first;
	int postion = samplePostion.second;
	if (postion >= MAX_SLOT_NUM)
	{
		tf::RackType::type rackType = ::tf::RackType::type::RACK_TYPE_ROUTINE;
		// 样本属于复查(必须在复查架范围）
		if (isRetest)
		{
			rackType = ::tf::RackType::type::RACK_TYPE_RERUN;
		}
		// 急诊样本
		else if (isEmerge)
		{
			rackType = ::tf::RackType::type::RACK_TYPE_STAT;
		}
		// 常规
		else
		{
			rackType = ::tf::RackType::type::RACK_TYPE_ROUTINE;
		}

		// 判断是否在架范围内才递增
		if (!CommonInformationManager::GetInstance()->IsRackRangOut(rackType, rack + 1))
		{
			return std::make_pair(rack + 1, 1);
		}

		return std::make_pair(rack, postion);
	}
	else
	{
		return std::make_pair(rack, ++postion);
	}
}

///
/// @brief 获取当前样本的信息
///
/// @param[out]  sampleInfo  样本信息
///
/// @return true获取成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
bool QSampleAssaySelect::GetCurrentSampleInfo(tf::SampleInfo& sampleInfo, bool isWarning)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 样本序号
    string seqNo = ui->sample_no->text().toStdString();
    // 样本条码
    string barCode = ui->sample_bar->text().toStdString();
    // 样本架号
    string rack = ui->sampel_rack->text().toStdString();
    // 样本加样量
    double sampleVolumn = ui->sample_use_volume->text().toDouble();
    // 样本位置号
    int postion = ui->sample_postion->currentData().toInt();
    // 备注
    string comments = ui->comment_edit->text().toStdString();
    // 样本杯类型
    tf::TubeType::type tubeType = static_cast<::tf::TubeType::type>(ui->sample_tube_type->currentData().toInt());
    ::tf::SampleSourceType::type sourcetype = static_cast<::tf::SampleSourceType::type>(ui->sample_type->currentData().toInt());
    // 样本类型
    sampleInfo.__set_sampleType(::tf::SampleType::SAMPLE_TYPE_PATIENT);
    sampleInfo.__set_tubeType(tubeType);
    sampleInfo.__set_sampleSourceType(sourcetype);
    // 样本无条码
    sampleInfo.__set_barcodeException(ui->sample_barcode_control->isChecked() ? true : false);

	// 条码模式下, 条码不能为空, 否则返回
	if (m_sampleTestMode == ::tf::TestMode::BARCODE_MODE && barCode.empty())
	{
		ULOG(LOG_INFO, "%s(The barCode is empty)", __FUNCTION__);
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("条码模式下，样本条码不能为空！")));
		pTipDlg->exec();
		return false;
	}

	// 序号模式和样本架模式下, 序号不能为空, 否则返回
	if (m_sampleTestMode == ::tf::TestMode::SEQNO_MODE && seqNo.empty())
	{
		ULOG(LOG_INFO, "%s(The seqNo is empty)", __FUNCTION__);
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("样本号模式、样本架模式下，样本号不能为空！")));
		pTipDlg->exec();
		return false;
	}

	sampleInfo.__set_seqNo(seqNo);
	sampleInfo.__set_barcode(barCode);

    if (!rack.empty())
    {
        sampleInfo.__set_rack(rack);
    }

    if (postion >= 1)
    {
        sampleInfo.__set_pos(postion);
    }

    // 手动稀释
    if (ui->dilution_box->currentData().toInt() == (int)SampleSize::MANUAL_SIZE)
    {
        // 设置稀释倍数
        sampleInfo.__set_preDilutionFactor(ui->dilution_edit->text().toInt());
    }

    sampleInfo.__set_stat(ui->emerge_btn->isChecked());
    sampleInfo.__set_comment(comments);

    std::vector<int64_t>			profileLst;				    // 组合项目列表
    std::vector<::tf::TestItem>		testItemLst;                // 非组合项目列表
	auto keyData = ui->AssaySelectTabContainer->GetSampleTestItemInfo(testItemLst);
	std::set<int64_t> runCodeMap;
    // 是否使用复查架
    for (auto& testItem : testItemLst)
    {
        // 若有复查项目，则使用复查架
        if (testItem.__isset.rerun && testItem.rerun)
        {
			// 同一复查项目不重复复查
			if (runCodeMap.count(testItem.assayCode))
			{
				testItem.__set_rerun(false);
				testItem.__set_status(tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED);
				continue;
			}

			runCodeMap.insert(testItem.assayCode);
            sampleInfo.__set_useRetestRack(true);
        }
    }

	ui->AssaySelectTabContainer->GetProfiles(profileLst);
    sampleInfo.__set_preDilutionFactor(keyData.second);
    sampleInfo.__set_testItems(testItemLst);
    sampleInfo.__set_profiles(profileLst);

    // 若不需要做判断
    if (!isWarning)
    {
        return true;
    }

	// 允许进行原架复查的
	auto IsTestPrePostion = [&](std::string rack)->bool
	{
		// 若是追加
		auto sampleValue = GetCurrentSample();
		if (!sampleValue)
		{
			return false;
		}

		// 是否有项目完成测试（原架复查的话，必须是复查）
		bool isReRun = false;
		for (const auto testItem : sampleValue.value().testItems)
		{
			if (testItem.__isset.conc)
			{
				isReRun = true;
				break;
			}
		}

		// 不是复查，直接返回
		if (!isReRun)
		{
			return false;
		}

		int rackRaw;
		if (!stringutil::IsInteger(sampleValue.value().rack, rackRaw))
		{
			return false;
		}

		int rackData;
		if (!stringutil::IsInteger(rack, rackData))
		{
			return false;
		}

		// 样本架类型
		auto rackType = CommonInformationManager::GetInstance()->GetRackTypeFromRack(rackRaw);
		if (CommonInformationManager::GetInstance()->IsRackRangOut(rackType, rackData))
		{
			return false;
		}

		return true;
	};

    // 急诊模式下，和条码模式下选择样本无条码的情况，样本架模式下，当有相同架号和位置号的时候，需要提醒
	bool isEmerge = ui->emerge_btn->isChecked();
    if (isEmerge 
		|| (m_sampleTestMode == ::tf::TestMode::BARCODE_MODE && ui->sample_barcode_control->isChecked())
        || m_sampleTestMode == ::tf::TestMode::RACK_MODE)
    {
        if (!rack.empty() && postion >= 1)
        {
			// 1:判断是否在对应的样本架范围之内
            // 急诊模式只能放在急诊架号和位置号内
            if (ui->emerge_btn->isChecked())
            {
                int rackData = stoi(rack);
                if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_STAT, rackData))
                {
					if (!IsTestPrePostion(rack))
					{
						RackRangeOneType rr = CommonInformationManager::GetInstance()->GetRackRange(::tf::RackType::type::RACK_TYPE_STAT);
						TipDlg(tr("保存"), tr("架号超出急诊架号允许范围（%1-%2），请重新设置！").arg(rr.iLowerLmt).arg(rr.iUpperLmt)).exec();
						return false;
					}
                }
            }
            // 若不是急诊，则是常规的样本
            else
            {
                int rackData = stoi(rack);
                if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_ROUTINE, rackData))
                {
					if (!IsTestPrePostion(rack))
					{
						RackRangeOneType rr = CommonInformationManager::GetInstance()->GetRackRange(::tf::RackType::type::RACK_TYPE_ROUTINE);
						TipDlg(tr("保存"), tr("架号超出常规架号允许范围（%1-%2），请重新设置！").arg(rr.iLowerLmt).arg(rr.iUpperLmt)).exec();
						return false;
					}
                }
            }

			// 2:位置是否冲突
			// 根据模式来查找对应的样本对象
            auto IndexSample = GetSampleByPostion(rack, postion, m_sampleTestMode);
			if (!IndexSample)
			{
				return true;
			}

			// 查出来的是非无条码样本也不需要考虑冲突与否bug21608
			// 必须是在条码模式下bug21608
			// 允许无条码样本的位置占用条码样本的位置
			if (m_sampleTestMode == ::tf::TestMode::BARCODE_MODE && ui->sample_barcode_control->isChecked())
			{
				if (!IndexSample->barcodeException)
				{
					return true;
				}
			}

			// 添加样本和，查出来的样本都是无条码样本才考虑不冲突
			auto currentSample = GetCurrentSample();
			// 判断是不是样本本身
			if (!currentSample || currentSample && currentSample->id != IndexSample->id)
			{
				TipDlg(tr("保存"), tr("输入架号位置号被占用，请重新输入！")).exec();
				return false;
			}
        }
    }

    return true;
}

///
/// @brief 新增按钮槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QSampleAssaySelect::OnClickNewBtn()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取最大样本号，并让最大样本号加1
    ::tf::ResultStr  rst;
    if (!DcsControlProxy::GetInstance()->GetMaxSampleSeqno(rst, ui->emerge_btn->isChecked(), m_sampleTestMode)
        || rst.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "GetMaxSeqNo() failed!");
        return;
    }

    // 必须首先检查是否是纯数字，如果不是，则返回
    if (!stringutil::IsPureDigit(rst.value))
    {
        ULOG(LOG_ERROR, "the max seq number is abnormal!");
        return;
    }

    m_isSeqChanged = false;
    cpp_int maxSeq(rst.value);
    // 保存当前样本序号
    m_sampleSeq = QString::fromStdString((++maxSeq).str());
    // 初始化样本参数
    SetDefaultSampleDetail(true);
    ui->SampleList->setCurrentIndex(QModelIndex());
    UpdateButtonStatus();

	// 序号模式/样本架模式下才跳到最大值
	if (m_sampleTestMode == ::tf::TestMode::SEQNO_MODE || m_sampleTestMode == ::tf::TestMode::RACK_MODE)
	{
		// 若超过最大值，则回退到最大值
		if (maxSeq > cpp_int(MAXSEQNUM))
		{
			maxSeq = maxSeq - 1;
			m_sampleSeq = QString::fromStdString(maxSeq.str());
			ui->sample_no->setText(m_sampleSeq);
			emit ui->sample_no->returnPressed();
		}
		else
		{
			ui->sample_no->setText(m_sampleSeq);
		}

	}
}

///
/// @brief 上一条记录被选中
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QSampleAssaySelect::OnPreviousBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 判断是否选中有效索引
    if (!ui->SampleList->currentIndex().isValid())
    {
        ULOG(LOG_INFO, "Select invalid item");
        return;
    }

    // 获取选中项前一行
    int iPrevRow = ui->SampleList->currentIndex().row() - 1;
    if (iPrevRow >= 0)
    {
        ui->SampleList->selectRow(iPrevRow);
        // 刷新当前内容
        OnSampleClicked(ui->SampleList->currentIndex());
    }
}

///
/// @brief 下一条记录被选中
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QSampleAssaySelect::OnNextBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取登记表的当前索引和标准模型
    auto curModIdx = ui->SampleList->currentIndex();
    // 判断是否选中有效索引
    if (!curModIdx.isValid())
    {
        ULOG(LOG_ERROR, "Select invalid item");
        return;
    }

    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return;
    }

    // 获取选中项后一行
    int iNextRow = curModIdx.row() + 1;
    if (iNextRow < sampleProxyMoudel->rowCount())
    {
        ui->SampleList->selectRow(iNextRow);
        // 刷新当前内容
        OnSampleClicked(ui->SampleList->currentIndex());
    }
}

///
/// @brief 删除被选中的样本
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QSampleAssaySelect::OnDelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 查找登记表中对应的选中模型和标准模型
    QItemSelectionModel* slectmodule = ui->SampleList->selectionModel();
    QModelIndexList selectIndexs = slectmodule->selectedRows();
    // 如果模型为空，则返回
    if (slectmodule == Q_NULLPTR || selectIndexs.count() <= 0)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未选中记录信息.")));
        pTipDlg->exec();
        return;
    }

    // 弹框提示是否确认删除
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除"), tr("确定删除选中样本？"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel delete!");
            return;
        }
    }

    // 删除选中样本信息
    if (!DelSelSampInfo())
    {
        return;
    }

	QSampleAssayModel::Instance().SetCurrentSampleRow(-1);
	ui->SampleList->setCurrentIndex(QModelIndex());
	QSampleAssayModel::Instance().ClearSelectedFlag(0);

    SetDefaultSampleDetail(true);
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
bool QSampleAssaySelect::DelSelSampInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 查找登记表中对应的选中模型和标准模型
    QItemSelectionModel* selectModule = ui->SampleList->selectionModel();
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel || selectModule == nullptr)
    {
        return false;
    }

    // 是否允许删除的样本
    int allowDelete = false;
    // 获取选中范围
    QModelIndexList selectIndexs = selectModule->selectedRows();
    // 逐行获取要删除的样本信息的数据库主键
    std::vector<int64_t> deleVec;
	std::vector<tf::SampleInfo> sampleVec;
    for (const auto& index : selectIndexs)
    {
        auto sample = sampleProxyMoudel->GetSampleByIndex(index);
        if (!sample.has_value())
        {
            continue;
        }
        // 样本值
        auto sampleValue = sample.value();
        // 存在样本处于测试中（P状态）
        if (!allowDelete && sampleValue.status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
        {
            if (selectIndexs.count() > 1)
            {
                std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("选中样本中正在测试的样本不可删除，是否继续删除其他样本？"), TipDlgType::TWO_BUTTON));
                if (pTipDlg->exec() == QDialog::Rejected)
                {
                    ULOG(LOG_INFO, "Cancel delete!");
                    return false;
                }

                allowDelete = true;
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

        deleVec.push_back(sampleValue.id);
		sampleVec.push_back(sampleValue);
    }

    // 删除选中样本
    if (!DcsControlProxy::GetInstance()->RemoveSampleInfos(deleVec))
    {
        ULOG(LOG_ERROR, "RemoveSampleInfos failed!");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除失败.")));
        pTipDlg->exec();
        return false;
    }

	// 依次增加操作日志
	for (const auto& sample : sampleVec)
	{
		WorkpageCommon::DeleteSampleOptLog(sample);
	}

    return true;
}

///
/// @brief 刷新右上坐标控件
///
/// @param[in]  index  页面序号
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QSampleAssaySelect::RefreshCornerWidgets(int index)
{
    ULOG(LOG_INFO, "%s(index: %d)", __FUNCTION__, index);
    if (m_filterModule == nullptr)
    {
        m_filterModule = new QSampleFilter(ui->SampleList);
    }

    SetDefaultSampleDetail(false);
    QSampleAssayModel::Instance().SetSampleModuleType(QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT);
    m_filterModule->setSourceModel(&QSampleAssayModel::Instance());
    ui->SampleList->setModel(m_filterModule);
    auto row = QSampleAssayModel::Instance().GetCurrentRow();
    auto sampleIndex = m_filterModule->mapFromSource(QSampleAssayModel::Instance().index(row, 0));

    if (!sampleIndex.isValid() && (row != -1))
    {
		auto sourceIndex = QSampleAssayModel::Instance().index(m_filterModule->rowCount() - 1, 0);
		auto index = m_filterModule->mapFromSource(sourceIndex);
        ui->SampleList->setCurrentIndex(index);
    }
    else
    {
        ui->SampleList->setCurrentIndex(sampleIndex);
    }

    if (row == -1)
    {
        ui->SampleList->clearSelection();
    }

	// modify bug0012450 by wuht
	auto currentIndex = ui->SampleList->currentIndex();
	if (!currentIndex.isValid())
	{
		SetDefaultSampleDetail(true);
	}

    m_parent->setCornerWidget(nullptr);
    UpdateButtonStatus();
}

///
/// @brief 样本被选中的处理函数
///
/// @param[in]  index  位置信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
void QSampleAssaySelect::OnSampleClicked(const QModelIndex& index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->dilution_box->setCurrentIndex(0);
    ui->dilution_edit->setText("");

    if (!index.isValid())
    {
        ULOG(LOG_INFO, "%s(index invalid)", __FUNCTION__);
        return;
    }

    if (QSampleAssayModel::Instance().GetModule() != QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT)
    {
        ULOG(LOG_INFO, "%s(mode invalid)", __FUNCTION__);
        return;
    }

    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel || index.row() >= sampleProxyMoudel->rowCount())
    {
        return;
    }

    int totalCount = sampleProxyMoudel->rowCount();

    // 只有一条数据
    if (totalCount == 1)
    {
        ui->flat_pre->setEnabled(false);
        ui->flat_next->setEnabled(false);
    }
    // 已经是第一条了
    else if (index.row() == 0)
    {
        ui->flat_pre->setEnabled(false);
        ui->flat_next->setEnabled(true);
    }
    // 已经是最后一条了
    else if (index.row() >= (totalCount - 1))
    {
        ui->flat_pre->setEnabled(true);
        ui->flat_next->setEnabled(false);
    }
    else
    {
        ui->flat_pre->setEnabled(true);
        ui->flat_next->setEnabled(true);
    }

    auto sample = sampleProxyMoudel->GetSampleByIndex(index);
    if (!sample)
    {
        return;
    }

    m_isSeqChanged = false;
    // 样本序号
    m_sampleSeq = QString::fromStdString(sample->seqNo);
    // 更新样本状态
    auto sampleInfo = sample.value();
    ui->AssaySelectTabContainer->SetAudit(sampleInfo.audit);
    // 获取需测试的项目列表
    auto testItems = DataPrivate::Instance().GetSampleTestItems(sampleInfo.id);
	ui->AssaySelectTabContainer->UpdateTestItemStatus(testItems);
	ui->AssaySelectTabContainer->UpdateAssayProfileStatus(sampleInfo.profiles);
	ShowSampleDetail(sampleInfo);
    UpdateButtonStatus();
}

///
/// @brief 样本号回车键槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QSampleAssaySelect::OnSampleNoEditReturnPress()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 条码模式下在序号编辑框上点击回车，不做反应
    if (m_sampleTestMode == ::tf::TestMode::BARCODE_MODE)
    {
        return;
    }

    // 获取编辑框中的样本号
    QString seqNo = ui->sample_no->text();
    // 判断样本号是否为空
    if (seqNo.isEmpty())
    {
		m_isSeqChanged = false;
        ULOG(LOG_INFO, "Sample number could not be empty.");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("样本号模式、样本架模式下，样本号不能为空！")));
        pTipDlg->exec();
        return;
    }

    // 样本架号
    int rack = -1;
    if (!ui->sampel_rack->text().isEmpty())
    {
        rack = ui->sampel_rack->text().toInt();
    }
    // 样本位置号
    int postion = ui->sample_postion->currentData().toInt();

    // 回车第一步刷新界面
    SetDefaultSampleDetail(false);
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return;
    }

    QSampleAssayModel* assayMoudel = static_cast<QSampleAssayModel*>(sampleProxyMoudel->sourceModel());
    if (assayMoudel == nullptr)
    {
        return;
    }

	// 修改bug3216-当仅仅使用序号的时候使用当前测试模式，当有选中样本的时候使用选中样本的模式来查找
	/// tf::TestMode::type testMode = GetMode(m_sampleTestMode);（去掉模式的转化，直接使用模式，20231207）
	// 需求更新18952，因为新需求，当修改样本后，不要求显示当前修改的样本，要求显示新增页面或者显示下一个样本
	// 同时要求在不同的模式下不能再修改不同模式下的样本了
	// 对于Bug3216的情况在新需求下不存在了
	/*auto currenSample = GetCurrentSample();
	if (currenSample)
	{
		testMode = currenSample->testMode;
	}*/

    m_sampleSeq = seqNo;
    // 设置样本默认行无效
    assayMoudel->SetCurrentSampleRow(-1);
    // 根据类型，和是否急诊样本来查找
    auto sampleData = DataPrivate::Instance().GetSampleBySeq(m_sampleTestMode, seqNo.toStdString(), tf::SampleType::SAMPLE_TYPE_PATIENT, ui->emerge_btn->isChecked());
    if (!sampleData)
    {
		// 1：急诊模式
		// 2：样本架模式
		if (ui->emerge_btn->isChecked() || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
		{
			// 若为找到样本，同时当前模式需要输入样本号，同时未修改过样本号
			if (!m_isSeqChanged)
			{
				// 递增架号位置号
				if (rack != -1)
				{
					auto samplePostion = GetIncreasePostion({ rack , postion }, ui->emerge_btn->isChecked(), false);
					ui->sampel_rack->setText(QString::number(samplePostion.first));
					ui->sample_postion->setCurrentIndex(samplePostion.second);
				}
			}
		}
		// 序号模式下，常规
		// modify bug0012053 by wuht
		else
		{
			// 设置位置号
			ui->sample_postion->setCurrentIndex(0);
			// 设置架号
			ui->sampel_rack->setText("");
		}

        ui->SampleList->setCurrentIndex(QModelIndex());
        UpdateButtonStatus();
        ui->sample_no->setText(seqNo);
        m_isSeqChanged = false;
        return;
    }

    m_isSeqChanged = false;
    auto rowOptional = DataPrivate::Instance().GetSampleByDb(sampleData.value().id);
    if (rowOptional.has_value())
    {
        // 获取代理行
        auto proxyIndex = sampleProxyMoudel->mapFromSource(assayMoudel->index(rowOptional.value(), 0));
        // 选中对应行
        ui->SampleList->selectRow(proxyIndex.row());
        // 刷新当前内容
        OnSampleClicked(ui->SampleList->currentIndex());
    }

    UpdateButtonStatus();
}

///
/// @brief 样本编码回车键槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QSampleAssaySelect::OnSampleBarEditReturnPress()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 序号/样本架模式下点击条码编辑框不做反应
    if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
    {
        return;
    }

    // 获取编辑框中的条码号
    QString barcode = ui->sample_bar->text();
    // 判断条码号是否为空
    if (barcode.isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("条码模式下，样本条码不能为空！")));
        pTipDlg->exec();
        return;
    }

    // 回车第一步刷新界面
    SetDefaultSampleDetail(false);

    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return;
    }

    QSampleAssayModel* assayMoudel = static_cast<QSampleAssayModel*>(sampleProxyMoudel->sourceModel());
    if (assayMoudel == nullptr)
    {
        return;
    }

    m_isSeqChanged = false;
    // 设置样本默认行无效
    assayMoudel->SetCurrentSampleRow(-1);

    // 根据类型，和是否急诊样本来查找
    auto sampleData = DataPrivate::Instance().GetSampleByBarCode(m_sampleTestMode, barcode.toStdString(), tf::SampleType::SAMPLE_TYPE_PATIENT);
    if (!sampleData)
    {
		// 条码模式下常规
		if (!(ui->emerge_btn->isChecked() || ui->sample_barcode_control->isChecked()))
		{
			ui->sample_postion->setCurrentIndex(0);
			// 设置架号
			ui->sampel_rack->setText("");
		}

        ui->sample_bar->setText(barcode);
        ui->SampleList->setCurrentIndex(QModelIndex());
        UpdateButtonStatus();
        return;
    }

    auto rowOptional = DataPrivate::Instance().GetSampleByDb(sampleData.value().id);
    if (rowOptional.has_value())
    {
        // 刷新当前内容
        OnSampleClicked(ui->SampleList->currentIndex());
        // 获取代理行
        auto proxyIndex = sampleProxyMoudel->mapFromSource(assayMoudel->index(rowOptional.value(), 0));
        // 选中对应行
        ui->SampleList->selectRow(proxyIndex.row());
    }

    UpdateButtonStatus();
}

///
/// @brief 保存样本信息的槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QSampleAssaySelect::OnSaveSampleInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	m_assaySelected = false;
    // 获取数据模型
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return;
    }

    QSampleAssayModel* assayMoudel = static_cast<QSampleAssayModel*>(sampleProxyMoudel->sourceModel());
    if (assayMoudel == nullptr)
    {
        return;
    }

    // 新增的样本
    if (!GetCurrentSample())
    {
        // 序号模式下和样本架模式
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
        {
            // 序号模式下和样本架模式下必须先点击回车，才能保存
            if (m_isSeqChanged)
            {
                ReportErr(true, tr("修改样本号后请先点击回车，再保存"));
            }
            SaveSampleInSeqMode();
        }
        // 条码模式
        else
        {
            // 条码模式下必须先点击回车，才能保存
            if (m_isSeqChanged)
            {
                ReportErr(true, tr("修改条码后请先点击回车，再保存"));
            }
            SaveSampleInBarcodeMode();
        }
    }
    // 修改样本
    else
    {
        // 序号模式下
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
        {
            // 序号模式下必须先点击回车，才能保存
            if (m_isSeqChanged)
            {
                ReportErr(true, tr("修改样本号后请先点击回车，再保存"));
            }
        }
        // 条码模式
        else
        {
            // 条码模式下必须先点击回车，才能保存
            if (m_isSeqChanged)
            {
                ReportErr(true, tr("修改条码后请先点击回车，再保存"));
            }
        }

        // 获取当前样本信息
        auto sampleData = GetCurrentSample();
        // 正在测试的样本不能修改
        if (sampleData.value().status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
        {
			OnSampleClicked(ui->SampleList->currentIndex());
            ReportErr(true, tr("样本正在检测中，不可修改！"));
        }

		// 去掉模式转换20231207
		if (m_sampleTestMode != sampleData.value().testMode)
		{
			OnSampleClicked(ui->SampleList->currentIndex());
			ReportErr(true, tr("不能修改非当前模式下的样本"));
		}

        // 已经审核的样本不允许修改
        if (sampleData.value().audit)
        {
			OnSampleClicked(ui->SampleList->currentIndex());
            ReportErr(true, tr("已经审核的样本不允许修改"));
        }

        tf::SampleInfo sampleInfo;
        if (!GetCurrentSampleInfo(sampleInfo))
        {
            return;
        }

        // 修改样本类型的时候，不能有已经测试完成的项目
        if (sampleInfo.sampleSourceType != sampleData.value().sampleSourceType)
        {
            for (const auto& testItem : sampleData.value().testItems)
            {
                // 已经有测试完成的项目
                if (testItem.__isset.status && testItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED)
                {
					OnSampleClicked(ui->SampleList->currentIndex());
                    ReportErr(true, tr("已经有测试完成的项目，不能修改样本类型"));
                }
            }
        }

        // 序号模式下的急诊
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE && ui->emerge_btn->isChecked())
        {
            if (!sampleInfo.__isset.rack || !sampleInfo.__isset.pos)
            {
                ULOG(LOG_WARN, "check the parameter");
				OnSampleClicked(ui->SampleList->currentIndex());
                ReportErr(true, tr("急诊下,架号和位置号不能为空"));
            }
        }

        // 条码模式下，选择了样本无条码时，架号位置号不能为空
        if (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE && ui->sample_barcode_control->isChecked())
        {
            if (!sampleInfo.__isset.rack || !sampleInfo.__isset.pos)
            {
                ULOG(LOG_WARN, "check the parameter");
				OnSampleClicked(ui->SampleList->currentIndex());
                ReportErr(true, tr("条码模式下,架号和位置号不能为空"));
            }
        }

        // 样本架模式下的急诊，架号位置号不能为空
        if (m_sampleTestMode == tf::TestMode::type::RACK_MODE)
        {
            if (!sampleInfo.__isset.rack || !sampleInfo.__isset.pos)
            {
                ULOG(LOG_WARN, "check the parameter");
				OnSampleClicked(ui->SampleList->currentIndex());
                ReportErr(true, tr("样本架模式下,架号和位置号不能为空"));
            }
        }

        sampleInfo.__set_id(sampleData.value().id);
        // 更新样本信息
        if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
        {
			OnSampleClicked(ui->SampleList->currentIndex());
            ReportErr(true, tr("保存失败"));
        }

        TipDlg(tr("保存"), tr("保存成功")).exec();

        // 序号模式或者样本架修改成功
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
        {
			// 空直接返回
			if (ui->sample_no->text().isEmpty())
			{
				return;
			}

			// 若样本号不是纯数字，则返回
			auto seqNo = ui->sample_no->text().toStdString();
			if (!stringutil::IsPureDigit(seqNo))
			{
				emit ui->sample_no->returnPressed();
				return;
			}

			// 递增与越界处理
			cpp_int newSeq(seqNo);
			newSeq++;
			if (newSeq >= cpp_int(MAXSEQNUM))
			{
				newSeq = cpp_int(MAXSEQNUM);
			}

			ui->sample_no->setText(QString::fromStdString(newSeq.str()));
			emit ui->sample_no->returnPressed();
			return;
        }
        // 条码模式修改成功
		// 条码模式下修改条码样本，点击保存按钮，跳转到新增界面(for bug18952)
        else
        {
			// 修改无条码样本，跳转到新增界面，无条码自动被勾选，架号位置号自动递增
			// 样本架号
			int rack = -1;
			int postion;
			bool isCheck = ui->sample_barcode_control->isChecked();
			if (isCheck)
			{
				if (!ui->sampel_rack->text().isEmpty())
				{
					rack = ui->sampel_rack->text().toInt();
				}

				// 样本位置号
				postion = ui->sample_postion->currentData().toInt();
			}

			// 不选择任何样本
			ui->SampleList->selectionModel()->clearCurrentIndex();
			ui->SampleList->clearSelection();
			SetDefaultSampleDetail(true);
			if (isCheck)
			{
				ui->sample_barcode_control->setChecked(isCheck);
				// 递增架号位置号
				if (rack != -1)
				{
					auto samplePostion = GetIncreasePostion({ rack , postion }, ui->emerge_btn->isChecked(), false);
					ui->sampel_rack->setText(QString::number(samplePostion.first));
					ui->sample_postion->setCurrentIndex(samplePostion.second);
				}
			}
        }
    }
}

///
/// @brief 序号模式下保存样本
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月3日，新建函数
///
void QSampleAssaySelect::SaveSampleInSeqMode()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取数据模型
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return;
    }

    QSampleAssayModel* assayMoudel = static_cast<QSampleAssayModel*>(sampleProxyMoudel->sourceModel());
    if (assayMoudel == nullptr)
    {
        return;
    }

    // 序号模式下必须有序号
    if (ui->sample_no->text().isEmpty())
    {
        ULOG(LOG_WARN, "check the parameter");
        ReportErr(true, tr("当前模式下样本号不能为空！"));
    }

    tf::SampleInfo sampleInfo;
    if (!GetCurrentSampleInfo(sampleInfo))
    {
        return;
    }

    // 急诊情况和样本架模式下
    if (ui->emerge_btn->isChecked() || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
    {
        if (!sampleInfo.__isset.rack || !sampleInfo.__isset.pos)
        {
            ULOG(LOG_WARN, "check the parameter");
            ReportErr(true, tr("未输入架号位置号信息，请重新输入！"));
        }
    }

    // 根据类型，和是否急诊样本来查找(若查不到表示新增，否在修改)
    boost::optional<tf::SampleInfo> sampleData = boost::none;
    if (!ui->sample_no->text().isEmpty())
    {
        sampleData = DataPrivate::Instance().GetSampleBySeq(m_sampleTestMode, ui->sample_no->text().toStdString(), tf::SampleType::SAMPLE_TYPE_PATIENT, ui->emerge_btn->isChecked());
    }

    // 1: 不存在样本，则新增
    if (!sampleData.has_value())
    {
        ::tf::ResultListInt64 lResult;
        if (!DcsControlProxy::GetInstance()->AddSampleInfo(lResult, sampleInfo, 1) || lResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "AddSampleInfo() failed");
            auto spDiglog = std::make_shared<QCustomDialog>(this, 1000, tr("保存失败"));
            if (spDiglog != Q_NULLPTR)
            {
                spDiglog->SetCenter();
                spDiglog->exec();
            }
            return;
        }
        // 必须首先检查是否是纯数字，如果不是，则返回
        auto inputSeq = ui->sample_no->text().toStdString();
        if (!stringutil::IsPureDigit(inputSeq))
        {
            ULOG(LOG_INFO, "%s(the input seq is : %s)", __FUNCTION__, inputSeq);
            QCustomDialog tipDlg(this, 1000, tr("保存成功"));
            tipDlg.exec();
            return;
        }
        // 获取下一个样本序号
        auto startChartNo = GetRightSeqNo(++cpp_int(inputSeq));
        ui->sample_no->setText(QString::fromStdString((startChartNo).str()));
        QCustomDialog tipDlg(this, 1000, tr("保存成功"));
        tipDlg.exec();

        // 模拟回车
        emit ui->sample_no->returnPressed();
        return;
    }
    // 2：存在样本则表示修改
    else
    {
        // 正在检测的样本不允许修改
        if (sampleData.value().status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
        {
            ReportErr(true, tr("样本正在检测中，不可修改！"));
        }

        // 能找到则修改
        sampleInfo.__set_id(sampleData.value().id);
        // 更新样本信息
        if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
        {
            ReportErr(true, tr("保存失败"));
        }

        emit ui->sample_no->returnPressed();
        ReportErr(true, tr("保存成功"));
    }
}

///
/// @brief 样本模式下保存样本
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月24日，新建函数
///
void QSampleAssaySelect::SaveSampleInBarcodeMode()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取数据模型
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel)
    {
        return;
    }

    QSampleAssayModel* assayMoudel = static_cast<QSampleAssayModel*>(sampleProxyMoudel->sourceModel());
    if (assayMoudel == nullptr)
    {
        return;
    }

    if (ui->sample_bar->text().isEmpty())
    {
        ULOG(LOG_WARN, "check the parameter");
        ReportErr(true, tr("未输入样本条码，请重新输入！"));
    }

    tf::SampleInfo sampleInfo;
    if (!GetCurrentSampleInfo(sampleInfo))
    {
        return;
    }

    // 样本无条码开启
    if (ui->sample_barcode_control->isChecked())
    {
        if (!sampleInfo.__isset.rack || !sampleInfo.__isset.pos)
        {
            ULOG(LOG_WARN, "check the parameter");
            ReportErr(true, tr("未输入架号位置号信息，请重新输入！"));
        }
    }

    // 条码
    boost::optional<tf::SampleInfo> sampleBarData = boost::none;
    if (!ui->sample_bar->text().isEmpty())
    {
        sampleBarData = DataPrivate::Instance().GetSampleByBarCode(m_sampleTestMode, ui->sample_bar->text().toStdString(), tf::SampleType::SAMPLE_TYPE_PATIENT);
    }

    //1：样本的条码不存在--新增
    if (!sampleBarData.has_value())
    {
        ::tf::ResultListInt64 lResult;
        if (!DcsControlProxy::GetInstance()->AddSampleInfo(lResult, sampleInfo, 1) || lResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "AddSampleInfo() failed");
            ReportErr(true, tr("保存失败"));
        }

        bool no_barcode = false;
        if (ui->sample_barcode_control->isChecked())
        {
            no_barcode = true;
        }

		// modify0024796 by wuht
		QCustomDialog tipDlg(this, 1000, tr("保存成功"));
		tipDlg.exec();

        // 模拟回车
        //emit ui->sample_bar->returnPressed();
        // 刷新界面为默认
        SetDefaultSampleDetail(false);
        // 架号递增
        if (sampleInfo.__isset.rack && sampleInfo.__isset.pos)
        {
            int postion = sampleInfo.pos;
            int rack = stoi(sampleInfo.rack);
            if ((rack > 0 && postion > 0))
            {
                if (postion >= MAX_SLOT_NUM)
                {
					tf::RackType::type rackType = ::tf::RackType::type::RACK_TYPE_ROUTINE;
                    // 样本属于复查(必须在复查架范围）
                    if (sampleInfo.useRetestRack)
                    {
                        rackType = ::tf::RackType::type::RACK_TYPE_RERUN;
                    }
                    // 急诊样本
                    else if (ui->emerge_btn->isChecked())
                    {
                        rackType = ::tf::RackType::type::RACK_TYPE_STAT;
                    }
                    // 常规
                    else
                    {
                        rackType = ::tf::RackType::type::RACK_TYPE_ROUTINE;
                    }

                    // 判断是否在架范围内才递增
                    if (!CommonInformationManager::GetInstance()->IsRackRangOut(rackType, rack + 1))
                    {
                        ui->sampel_rack->setText(QString::number(++rack));
                        ui->sample_postion->setCurrentIndex(1);
                    }
                }
                else
                {
                    ui->sampel_rack->setText(QString::number(rack));
                    ui->sample_postion->setCurrentIndex(++postion);
                }
            }
        }

        ui->sample_barcode_control->setChecked(no_barcode);
        return;
    }
    // 修改信息
    else
    {
        // 正在检测的样本不允许修改
        if (sampleBarData.value().status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
        {
            ReportErr(true, tr("样本正在检测中，不可修改！"));
        }

        // 能找到则修改
        sampleInfo.__set_id(sampleBarData.value().id);
        // 更新样本信息
        if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
        {
            ReportErr(true, tr("保存失败"));
        }

        emit ui->sample_bar->returnPressed();
        ReportErr(true, tr("保存成功"));
    }
}

///
/// @brief 批量输入按钮被按下
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QSampleAssaySelect::OnBatchInputBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (ui->sample_no->text().isEmpty() && m_sampleTestMode == tf::TestMode::type::SEQNO_MODE)
    {
        ReportErr(true, tr("样本号模式下样本号不能为空！"));
    }

    if (ui->sample_bar->text().isEmpty() && m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
    {
        ReportErr(true, tr("未输入样本条码，请重新输入！"));
    }

    if (ui->sample_no->text().isEmpty() && m_sampleTestMode == tf::TestMode::type::RACK_MODE)
    {
        ReportErr(true, tr("样本架模式下样本号不能为空！"));
    }

    // 条码模式下，条码不是纯数字，则不能做为批量添加的模板
    if (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE && !stringutil::IsPureDigit(ui->sample_bar->text().toStdString()))
    {
        ReportErr(true, tr("样本条码为非纯数字的样本无法进行批量添加"));
    }

    // 急诊，序号模式下必须有架号位置号/样本架模式也必须有架号位置号
	// 条码模式下，勾选样本无条码的情况（bug3321）
    if ((ui->emerge_btn->isChecked() && m_sampleTestMode == tf::TestMode::type::SEQNO_MODE)
		|| (m_sampleTestMode == tf::TestMode::type::RACK_MODE)
		|| (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE && ui->sample_barcode_control->isChecked()))
    {
        string rack = ui->sampel_rack->text().toStdString();
        // 样本位置号
        int postion = ui->sample_postion->currentData().toInt();

        if (rack.empty() || postion <= 0)
        {
            ULOG(LOG_WARN, "check the parameter");
            ReportErr(true, tr("当前模式下,架号和位置号不能为空"));
        }
    }

    // 在选中的样本条件下的批量添加
    auto sample = GetCurrentSample();
    if (sample)
    {

        // 1: 第一种情况（正在测试和已经审核的样本的不能做为模板）
        if (sample.value().__isset.audit && sample.value().audit
            || sample.value().__isset.status && sample.value().status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
        {
            auto seqNo = QDataItemInfo::Instance().GetSeqNo(sample.value());
            auto message = QString(tr("当前样本%1不能进行批量输入")).arg(seqNo);
            TipDlg(tr("批量添加"), message).exec();
            return;
        }

        // 2：有测试项目完成的样本不能作为模板
        bool hasFinished = false;
        for (const auto testItem : sample.value().testItems)
        {
            if (testItem.__isset.status
                && testItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED)
            {
                hasFinished = true;
                break;
            }
        }

        if (hasFinished)
        {
            auto seqNo = QDataItemInfo::Instance().GetSeqNo(sample.value());
            auto message = QString(tr("已有完成测试项目的样本%1不能进行批量输入")).arg(seqNo);
            TipDlg(tr("批量添加"), message).exec();
            return;
        }
    }

    if (m_batchDialog == nullptr)
    {
        m_batchDialog = new QDialogBatchRegister(this);
        connect(m_batchDialog, SIGNAL(batchParameterInputed()), this, SLOT(OnDealBatchInputSample()));
    }

    // 序号模式/样本架模式
    if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
    {
        auto inputSeq = ui->sample_no->text().toStdString();
        if (!stringutil::IsPureDigit(inputSeq))
        {
            ULOG(LOG_INFO, "%s(the input seq is : %s)", __FUNCTION__, inputSeq);
            return;
        }

        m_batchDialog->SetSefaultStopNo(++cpp_int(inputSeq), false);
    }
    // 条码模式
    else
    {
        auto inputBar = ui->sample_bar->text().toStdString();
        if (!stringutil::IsPureDigit(inputBar))
        {
            ULOG(LOG_INFO, "%s(the input bar is : %s)", __FUNCTION__, inputBar);
            return;
        }

        m_batchDialog->SetSefaultStopNo(++cpp_int(inputBar), true);
    }

    m_batchDialog->show();
}

///
/// @brief 处理批量输入
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QSampleAssaySelect::OnDealBatchInputSample()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    auto fSetTestTimes = [&](tf::SampleInfo& sampleInfo, int testTimes)->bool
    {
        if (!GetCurrentSampleInfo(sampleInfo))
        {
            return false;
        }

        // 依次更新次数
        for (auto item : sampleInfo.testItems)
        {
            ui->AssaySelectTabContainer->UpdateTestItemTimes(item.assayCode, testTimes);
        }

        // 再次获取项目的数目（上一步增加了项目数目）
        if (!GetCurrentSampleInfo(sampleInfo))
        {
            return false;
        }

        // 新增必须将所有的项目的id = -1
        for (auto& item : sampleInfo.testItems)
        {
            item.id = -1;
            item.__isset.id = false;
        }

        return true;
    };

    // 判断样本是否覆盖架号位置号（true-表示需要覆盖，false-表示不会覆盖）
    auto f = [&](tf::SampleInfo& sampleInfo, int count)->bool
    {
		// 样本号模式下，常规样本，批量添加样本不需要判断架号位置号是否覆盖
		// bug0011709
		if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE 
			&& sampleInfo.__isset.stat
			&& !sampleInfo.stat)
		{
			return false;
		}

        // 没有设置架号位置号，则不需判断是否覆盖
        if (!sampleInfo.__isset.rack || !sampleInfo.__isset.pos)
        {
            return false;
        }

        auto rack = sampleInfo.rack;
        auto pos = sampleInfo.pos;
        while (count > 0)
        {
            if (pos >= MAX_SLOT_NUM)
            {
                auto rackNum = stoi(rack);
                rack = to_string(++rackNum);
                pos = 1;
            }

			// 必须是待测样本占用了对应的位置，否则可以回收使用
			// 首先递增位置号的原因是，自己的位置不存在覆盖
			auto spSample = DataPrivate::Instance().GetSampleByRackAndPsotion(m_sampleTestMode, rack, pos);
			if (spSample == nullptr)
			{
				count--;
				continue;
			}

			// 条码模式下，若非无条码模式，样本架号无要求（13515）
			if (m_sampleTestMode == tf::TestMode::BARCODE_MODE 
				&& spSample->__isset.barcodeException
				&& !spSample->barcodeException)
			{
				count--;
				continue;
			}

			if (spSample->status == tf::SampleStatus::type::SAMPLE_STATUS_PENDING)
			{
				return true;
			}

            count--;
			// modify bug0014472 by wuht
			pos++;
        }

        return false;
    };

    if (m_batchDialog == nullptr)
    {
        return;
    }

    // 获取终止样本和样本数量和每个项目测试次数
    cpp_int endKey = 0;
    int count = -1;
    int times = -1;
    QDialogBatchRegister::BATCHMODE  batchMode = m_batchDialog->GetBatchInputPatater(endKey, count, times);

    tf::SampleInfo sampleInfo;
    if (!fSetTestTimes(sampleInfo, times))
    {
        ULOG(LOG_ERROR, "AddSampleInfo() failed");
		// modify bug0024892 by wuht 
		return;
    }

    cpp_int startKey = 0;
    // 序号模式下-序号/样本架模式
    if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
    {
        auto inputSeq = ui->sample_no->text().toStdString();
        if (!stringutil::IsPureDigit(inputSeq))
        {
            ULOG(LOG_INFO, "%s(the input seq is : %s)", __FUNCTION__, inputSeq);
            return;
        }

        startKey = cpp_int(inputSeq);
    }

    // 条码模式下-条码
    if (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
    {
        auto inputBar = ui->sample_bar->text().toStdString();
        if (!stringutil::IsPureDigit(inputBar))
        {
            ULOG(LOG_INFO, "%s(the input bar is : %s)", __FUNCTION__, inputBar);
            return;
        }

        startKey = cpp_int(inputBar);
    }

    // 样本数量进行批量添加
    if (batchMode == QDialogBatchRegister::SAMPLECOUNT)
    {
        if (count <= 0)
        {
            ReportErr(true, tr("批量添加样本数目需大于0"));
        }

        // 判断是否样本架覆盖
        if (f(sampleInfo, count))
        {
			// 条码模式/样本号模式/样本架模式批量输入过程中遇到样本检测完成、待测、被占用等情况，
			// 给出提示告知本次批量输入失败，让用户重新进行批量添加（日立逻辑）
			// 修改bug21625,根据讨论以后的最新逻辑修改
			// 0024005: bug修改，消息提示问号改叹号 modify by chenjianlin 20231215
			TipDlg(tr("提示"), tr("输入架号位置号被占用，添加失败，请重新输入！")).exec();
			return;
        }

        // 序号模式下/样本架模式
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
        {
            // 判断是否存在被覆盖的样本
            cpp_int startNo = startKey;
            for (int i = 0; i < count; i++)
            {
                auto sampleData = DataPrivate::Instance().GetSampleBySeq(m_sampleTestMode, startNo.str(), tf::SampleType::SAMPLE_TYPE_PATIENT, ui->emerge_btn->isChecked());
                if (sampleData.has_value())
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("存在相同的样本号，会覆盖样本，同意?"), TipDlgType::TWO_BUTTON));
                    if (pTipDlg->exec() == QDialog::Rejected)
                    {
                        return;
                    }
                    else
                    {
                        break;
                    }
                }

                startNo++;

                // 序号不能比最大编号大
                if (startNo >= cpp_int(MAXSEQNUM))
                {
                    // 强制减小能够批量添加的样本数目
                    // count = i;
                    break;
                }
            }

            // 设置序号
            sampleInfo.__set_seqNo(startKey.str());
            ::tf::ResultListInt64 lResult;
            if (!DcsControlProxy::GetInstance()->AddSampleInfo(lResult, sampleInfo, count) || lResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "AddSampleInfo() failed");
                ReportErr(true, tr("批量添加样本失败"));
            }

            // 当返回的添加/修改的样本数目小于需要增加的
            if (lResult.value.size() < count)
            {
                ReportErr(true, tr("%1个样本添加失败").arg(count - lResult.value.size()));
            }
        }

        // 条码模式
        if (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
        {
            // 判断是否存在被覆盖的样本
            cpp_int startBar = startKey;
            for (int i = 0; i < count; i++)
            {
                auto sampleData = DataPrivate::Instance().GetSampleByBarCode(m_sampleTestMode, startBar.str(), tf::SampleType::SAMPLE_TYPE_PATIENT);
                if (sampleData.has_value())
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("存在相同的条码，会覆盖样本，同意?"), TipDlgType::TWO_BUTTON));
                    if (pTipDlg->exec() == QDialog::Rejected)
                    {
                        return;
                    }
                    else
                    {
                        break;
                    }
                }

                startBar++;
            }

            // 设置条码
            sampleInfo.__set_barcode(startKey.str());
            ::tf::ResultListInt64 lResult;
            if (!DcsControlProxy::GetInstance()->AddSampleInfo(lResult, sampleInfo, count) || lResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "AddSampleInfo() failed");
                ReportErr(true, tr("批量添加样本失败"));
            }

            // 当返回的添加/修改的样本数目小于需要增加的
            if (lResult.value.size() < count)
            {
                ReportErr(true, tr("%1个样本添加失败").arg(count - lResult.value.size()));
            }
        }
    }
    // 样本终止号批量添加
    else if (batchMode == QDialogBatchRegister::STOPSAMPLENO)
    {
        // 不管是序号还是条码都不能小于0
        if (endKey <= 0)
        {
            ReportErr(true, tr("样本号超出范围"));
        }

        if (startKey > endKey)
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("开始样本号不能大于结束样本号")));
            pTipDlg->exec();
            return;
        }

        // 获取需要添加多少个样本
        int count = 0;
        for (cpp_int startNo = startKey; startNo <= endKey; startNo++)
        {
            count++;
        }

        // 判断是否样本架覆盖
        if (f(sampleInfo, count))
        {
			// 条码模式/样本号模式/样本架模式批量输入过程中遇到样本检测完成、待测、被占用等情况，
			// 给出提示告知本次批量输入失败，让用户重新进行批量添加（日立逻辑）
			// 修改bug21625,根据讨论以后的最新逻辑修改
			// 0024005: bug修改，消息提示问号改叹号 modify by chenjianlin 20231215
			TipDlg(tr("提示"), tr("输入架号位置号被占用，添加失败，请重新输入！")).exec();
			return;
        }

        // 序号模式/样本架模式
        if (m_sampleTestMode == tf::TestMode::type::SEQNO_MODE || m_sampleTestMode == tf::TestMode::type::RACK_MODE)
        {
            cpp_int startNo = startKey;
            while (startNo <= endKey)
            {
                auto sampleData = DataPrivate::Instance().GetSampleBySeq(m_sampleTestMode, startNo.str(), tf::SampleType::SAMPLE_TYPE_PATIENT, ui->emerge_btn->isChecked());
                if (sampleData.has_value())
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("存在相同的样本号，会覆盖样本，同意?"), TipDlgType::TWO_BUTTON));
                    if (pTipDlg->exec() == QDialog::Rejected)
                    {
                        return;
                    }
                    else
                    {
                        break;
                    }
                }

                startNo++;
            }

            sampleInfo.__set_seqNo(startKey.str());
            // 新增样本信息
            ::tf::ResultListInt64 lResult;
            cpp_int countString = endKey - startKey + 1;
            if (countString < 0)
            {
                ULOG(LOG_ERROR, "%s(the start key:%s > end key:%s)", __FUNCTION__, startKey.str(), endKey.str());
                ReportErr(true, tr("开始样本号 > 结束样本号"));
            }

            // 转化为int整数
            int count = 0;
            try
            {
                count = countString.convert_to<int>();
            }
            catch (overflow_error* e)
            {
                ULOG(LOG_ERROR, "%s(convert %s error)", __FUNCTION__, countString.str());
                ReportErr(true, tr("转化失败"));
            }

            if (count > 200 || count <= 0)
            {
                ULOG(LOG_ERROR, "%s(failed %d)", __FUNCTION__, count);
                ReportErr(true, tr("最大添加样本不能超过200"));
            }

            if (!DcsControlProxy::GetInstance()->AddSampleInfo(lResult, sampleInfo, count) || lResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "AddSampleInfo() failed");
                ReportErr(true, tr("批量添加样本失败"));
            }

            // 当返回的添加/修改的样本数目小于需要增加的
            if (lResult.value.size() < count)
            {
                ReportErr(true, tr("%1个样本添加失败").arg(count - lResult.value.size()));
            }
        }

        // 条码模式
        if (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
        {
            cpp_int startBar = startKey;
            while (startBar <= endKey)
            {
                auto sampleData = DataPrivate::Instance().GetSampleByBarCode(m_sampleTestMode, startBar.str(), tf::SampleType::SAMPLE_TYPE_PATIENT);
                if (sampleData.has_value())
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("存在相同的样本条码，会覆盖样本，同意?"), TipDlgType::TWO_BUTTON));
                    if (pTipDlg->exec() == QDialog::Rejected)
                    {
                        return;
                    }
                    else
                    {
                        break;
                    }
                }

                startBar++;
            }

            sampleInfo.__set_barcode(startKey.str());
            // 新增样本信息
            ::tf::ResultListInt64 lResult;
            cpp_int countString = endKey - startKey + 1;
            if (countString < 0)
            {
                ULOG(LOG_ERROR, "%s(the start key:%s > end key:%s)", __FUNCTION__, startKey.str(), endKey.str());
                ReportErr(true, tr("开始样本号 > 结束样本号"));
            }

            // 转化为int整数
            int count = 0;
            try
            {
                count = countString.convert_to<int>();
            }
            catch (overflow_error* e)
            {
                ULOG(LOG_ERROR, "%s(convert %s error)", __FUNCTION__, countString.str());
                ReportErr(true, tr("转化失败"));
            }

            if (count > 200 || count <= 0)
            {
                ULOG(LOG_ERROR, "%s(failed %d)", __FUNCTION__, count);
                ReportErr(true, tr("最大添加样本不能超过200"));
            }

            if (!DcsControlProxy::GetInstance()->AddSampleInfo(lResult, sampleInfo, count) || lResult.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "AddSampleInfo() failed");
                ReportErr(true, tr("批量添加样本失败"));
            }

            if (lResult.value.size() < count)
            {
                ReportErr(true, tr("%1个样本添加失败").arg(count - lResult.value.size()));
            }
        }
    }
    else
    {
        return;
    }

    // 刷新到新增样本界面
	UpdateButtonStatus();
    OnClickNewBtn();
}

///
/// @brief 设置稀释方式
///
/// @param[in]  index  0，标准，1减量，2增量，3自定义
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月14日，新建函数
///
void QSampleAssaySelect::OnDilution(int index)
{
    ULOG(LOG_INFO, "%s(index: %d)", __FUNCTION__, index);
    int dilutions = -1;
    SampleSize sampleType;
    // 手工稀释--对样本的稀释（项目稀释为默认，不改变）
    if (ui->dilution_box->currentData().toInt() == (int)SampleSize::MANUAL_SIZE)
    {
        ui->dilution_edit->clear();
        ui->dilution_edit->setText("");
        ui->dilution_edit->setValidator(new QRegExpValidator(QRegExp(UI_MANUAL_DILUTION), this));
        ui->dilution_edit->show();
        sampleType = SampleSize::MANUAL_SIZE;
        dilutions = ui->dilution_edit->text().toInt();
    }
    // 自动稀释--用户自定义的稀释--对项目的稀释
    else if (ui->dilution_box->currentData().toInt() == (int)SampleSize::USER_SIZE)
    {
        ui->dilution_edit->clear();
        ui->dilution_edit->setText("");
        sampleType = SampleSize::USER_SIZE;
        ui->dilution_edit->setValidator(new QRegExpValidator(QRegExp(UI_DILUTION), this));
        ui->dilution_edit->show();
        dilutions = ui->dilution_edit->text().toInt();
    }
    // 快捷稀释
    else if (ui->dilution_box->currentData().toInt() == -1 || ui->dilution_box->currentData().toInt() > (int)SampleSize::USER_SIZE)
    {
        sampleType = SampleSize::USER_SIZE;
        dilutions = ui->dilution_box->currentData().toInt();
        // 对3的特殊处理
        if (dilutions == -1)
        {
            dilutions = 3;
        }

        // 对于“1”的特殊处理
//         if (dilutions == 6)
//         {
//             dilutions = 1;
//         }

        // 对于“2”的特殊处理
        if (dilutions == 7)
        {
            dilutions = 2;
        }

        // 对于“4”的特殊处理
        if (dilutions == 8)
        {
            dilutions = 4;
        }

        ui->dilution_edit->hide();
    }
    // 增量，减量，常量
    else
    {
        ui->dilution_edit->hide();
        dilutions = 1;
        sampleType = SampleSize(ui->dilution_box->currentData().toInt());
    }

    ui->AssaySelectTabContainer->SetDilution(sampleType, dilutions);
}

///
/// @brief 设置用户自定义稀释倍数
///
/// @param[in]  text  自定义稀释倍数
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月14日，新建函数
///
void QSampleAssaySelect::OnDilutionText(QString text)
{
    // 用户自定义稀释前项目
    if (ui->dilution_box->currentData().toInt() == (int)SampleSize::USER_SIZE)
    {
        ui->AssaySelectTabContainer->SetDilution(SampleSize::USER_SIZE, text.toInt());
    }

    if (ui->dilution_box->currentData().toInt() == (int)SampleSize::MANUAL_SIZE)
    {
        ui->AssaySelectTabContainer->SetDilution(SampleSize::MANUAL_SIZE, text.toInt());
    }
}

///
/// @brief 查看病人信息按钮被按下
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月19日，新建函数
///
void QSampleAssaySelect::OnPatientInfoBtn()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_paTientDialog == nullptr)
    {
        m_paTientDialog = new QPatientDialog(this);
        // 上一条下一条
        connect(m_paTientDialog, SIGNAL(MoveButtonPressed(int)), this, SLOT(MovePatientInfo(int)));
    }

    // 判断样本登记表当前选中项是否为有效索引
    auto curModIdx = ui->SampleList->currentIndex();
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel || !curModIdx.isValid())
    {
        return;
    }

    auto sample = sampleProxyMoudel->GetSampleByIndex(curModIdx);
    if (!sample)
    {
        return;
    }

    // 获取选中样本的db_no
    auto sampleId = sample.value().id;
    int totalCount = sampleProxyMoudel->rowCount();

    // 只有一条数据
    if (totalCount == 1)
    {
        m_paTientDialog->UpdatePatientData(sampleId, true, true);
    }
    // 已经是第一条了
    else if (curModIdx.row() == 0)
    {
        m_paTientDialog->UpdatePatientData(sampleId, true, false);
    }
    // 已经是最后一条了
    else if (curModIdx.row() >= (totalCount - 1))
    {
        m_paTientDialog->UpdatePatientData(sampleId, false, true);
    }
    else
    {
        m_paTientDialog->UpdatePatientData(sampleId, false, false);
    }

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
void QSampleAssaySelect::MovePatientInfo(int orient)
{
    ULOG(LOG_INFO, "%s(derection : %d)", __FUNCTION__, orient);
    if (m_paTientDialog == nullptr)
    {
        return;
    }

    if (orient > 0)
    {
        OnNextBtnClicked();
    }
    else
    {
        OnPreviousBtnClicked();
    }

    // 判断样本登记表当前选中项是否为有效索引
    auto curModIdx = ui->SampleList->currentIndex();
    auto sampleProxyMoudel = static_cast<QSampleFilter*>(ui->SampleList->model());
    if (nullptr == sampleProxyMoudel || !curModIdx.isValid())
    {
        return;
    }

    auto sample = sampleProxyMoudel->GetSampleByIndex(curModIdx);
    if (!sample)
    {
        return;
    }

    // 获取选中样本的db_no
    std::int64_t iSampDbNo = sample.value().id;
    int totalCount = sampleProxyMoudel->rowCount();
    // 只有一条数据
    if (totalCount == 1)
    {
        m_paTientDialog->UpdatePatientData(iSampDbNo, true, true);
    }
    // 已经是第一条了
    else if (curModIdx.row() == 0)
    {
        m_paTientDialog->UpdatePatientData(iSampDbNo, true, false);
    }
    // 已经是最后一条了
    else if (curModIdx.row() >= (totalCount - 1))
    {
        m_paTientDialog->UpdatePatientData(iSampDbNo, false, true);
    }
    else
    {
        m_paTientDialog->UpdatePatientData(iSampDbNo, false, false);
    }
}

///
/// @brief 样本无条码的槽函数
///
/// @param[in]  checked  是否被选中
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月20日，新建函数
///
void QSampleAssaySelect::OnSampleNoBarCodeBtn(bool checked)
{
    if (m_sampleTestMode == tf::TestMode::type::BARCODE_MODE)
    {
        if (checked)
        {
            ui->sampel_rack->setEnabled(true);
            ui->sample_postion->setEnabled(true);
        }
        else
        {
            ui->sampel_rack->setEnabled(false);
            ui->sample_postion->setEnabled(false);
        }
    }
}

void QSampleAssaySelect::OnRefreshAssayCard()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 变更单项目行数
//    ui->AssaySelectTabContainer->ResetSingleAssayRow(CommonInformationManager::GetInstance()->GetSingleAssayRowAmount());

    // 刷新项目选择页面
 //   ui->AssaySelectTabContainer->RefreshAssayComplete();
    ui->SampleList->selectRow(ui->SampleList->currentIndex().row());
}

bool QSampleAssaySelect::IsUpdateData()
{
	if (!this->isVisible())
	{
		return true;
	}

	if (QSampleAssayModel::Instance().GetModule() == QSampleAssayModel::VIEWMOUDLE::ASSAYSELECT)
	{
		return false;
	}

	return true;
}

///
/// @brief 根据位置信息获取样本
///
/// @param[in]  rack    架号
/// @param[in]  postion 位置号 
/// @param[in]  testMode 样本所属测试模式 
///
/// @return 样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月23日，新建函数
///
boost::optional<tf::SampleInfo> QSampleAssaySelect::GetSampleByPostion(const std::string& rack, int postion, tf::TestMode::type testMode)
{
    tf::SampleInfoQueryCond queryCond;
    queryCond.__set_rack(rack);
    queryCond.__set_pos(postion);
    queryCond.__set_testMode(testMode);
	// bug21608对于架号和位置号，只有待测状态的样本不能重复
	// bug0012061与需求周晓峰商量，当样本只要不是在已完成状态都不允许重复（所以测试状态也不行）
    queryCond.__set_statuses({ tf::SampleStatus::type::SAMPLE_STATUS_PENDING, tf::SampleStatus::type::SAMPLE_STATUS_TESTING });
    tf::SampleInfoQueryResp sampleResult;

    if (!DcsControlProxy::GetInstance()->QuerySampleInfo(queryCond, sampleResult)
        || sampleResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || sampleResult.lstSampleInfos.empty())
    {
        return boost::none;
    }

    return boost::make_optional(sampleResult.lstSampleInfos[0]);
}

///
/// @brief 获取样本的加样量
///
/// @param[in]  sampleInfo  样本信息
///
/// @return 加样量大小
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月15日，新建函数
///
int QSampleAssaySelect::GetSampleVolumn(tf::SampleInfo& sampleInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int retVolumn = 0;
    auto spManager = CommonInformationManager::GetInstance();
    if (spManager == Q_NULLPTR)
    {
        return retVolumn;
    }

    // 设置项目的测试次数（LHI和ISE）
    auto fmap = [](std::map<int, int>& insertMap, int iAssayCode)
    {
        auto iter = insertMap.find(iAssayCode);
        if (iter == insertMap.end())
        {
            insertMap[iAssayCode] = 1;
        }
        else
        {
            insertMap[iAssayCode]++;
        }
    };

    // 获取最大的测试次数
    auto fGetMaxTimes = [](std::map<int, int>& insertMap)->int
    {
        int maxTimes = 0;
        for (const auto& item : insertMap)
        {
            if (item.second > maxTimes)
            {
                maxTimes = item.second;
            }
        }

        return maxTimes;
    };

    // 存放LHI和ISE的次数
    std::map<int, int> ISEmap;
    int iseVolumn = -1;
    std::map<int, int> SINDmap;
    int sindVolumn = -1;

    // 普通项目的加样量
    auto testItmes = DataPrivate::Instance().GetSampleTestItems(sampleInfo.id);
    for (const auto& testItem : testItmes)
    {
        if (testItem == Q_NULLPTR)
        {
            continue;
        }

        if (spManager->IsAssaySIND(testItem->assayCode))
        {
            fmap(SINDmap, testItem->assayCode);
            if (sindVolumn == -1)
            {
				// 修改Bug3326,因为LHI中只有SIND有加样量信息
				tf::TestItem tmpTestItem = *testItem;
				tmpTestItem.__set_assayCode(::ch::tf::g_ch_constants.ASSAY_CODE_SIND);
                sindVolumn = GetTestItemVolumn(tmpTestItem, sampleInfo.sampleSourceType);
            }
            continue;
        }

        if (spManager->IsAssayISE(testItem->assayCode))
        {
            fmap(ISEmap, testItem->assayCode);
            if (iseVolumn == -1)
            {
                iseVolumn = GetTestItemVolumn(*testItem, sampleInfo.sampleSourceType);
            }
            continue;
        }

        retVolumn += GetTestItemVolumn(*testItem, sampleInfo.sampleSourceType);
    }

    // 存在血清指数项目
    auto sindTimes = fGetMaxTimes(SINDmap);
    if (sindTimes != 0 && sindVolumn != -1)
    {
        retVolumn += sindTimes*sindVolumn;
    }

    // 存在Ise项目
    auto iseTimes = fGetMaxTimes(ISEmap);
    if (iseTimes != 0 && iseVolumn != -1)
    {
        retVolumn += iseTimes*iseVolumn;
    }

    return retVolumn;
}

///
/// @brief 根据稀释倍数获取加样量
///
/// @param[in]  ratio  稀释倍数
///
/// @return 加样量（若找不到，则返回0）
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月16日，新建函数
///
int QSampleAssaySelect::GetSampleRatioVolumn(int ratio)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    static std::map<int, int> mapRatio2SampleVol = GetRatio2SampleVolMap();

    int volumn = 0;
    // 越界检测(查看稀释倍数是否被赋值)
    if (mapRatio2SampleVol.count(ratio) <= 0)
    {
        return volumn;
    }

    return mapRatio2SampleVol[ratio];
}

///
/// @brief 获取项目的加样量
///
/// @param[in]  testTtem  项目信息
/// @param[in]  sampleType样本类型
///
/// @return 项目加样量，注意：单位为 0.1ul
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月15日，新建函数
///
int QSampleAssaySelect::GetTestItemVolumn(tf::TestItem& testItem, int sampleType)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, testItem.assayCode);

    int retVolumn = 0;
    if (testItem.__isset.status && testItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_TESTED)
    {
        return retVolumn;
    }

    auto spManager = CommonInformationManager::GetInstance();
    if (spManager == Q_NULLPTR)
    {
        return retVolumn;
    }

    // 是否生化项目
    auto spVecChAssay = spManager->GetChemistrySpecialAssayInfo(testItem.assayCode,
        ::tf::DeviceType::DEVICE_TYPE_C1000, sampleType);
    if (!spVecChAssay.empty())
    {
        auto spChAssay = spVecChAssay[0];
        if (spChAssay == Q_NULLPTR)
        {
            return retVolumn;
        }

        // 若是血清指数
        if (spManager->IsAssaySIND(testItem.assayCode) && !spChAssay->sampleAspirateVols.empty())
        {
            retVolumn = spChAssay->sampleAspirateVols[0].originalSample;
        }
        // 普通项目
        else
        {
			// 机外稀释默认的加样量是常量的加样量
			if (testItem.__isset.preDilutionFactor && testItem.preDilutionFactor > 1)
			{
				int size = spChAssay->sampleAspirateVols.size();
				if (size < 1)
				{
					return retVolumn;
				}
				retVolumn = spChAssay->sampleAspirateVols[0].originalSample;
			}
            // 增量，减量，常量的稀释规则
            else if (testItem.__isset.dilutionFactor && testItem.dilutionFactor == 1)
            {
				int size = spChAssay->sampleAspirateVols.size();
                switch (testItem.suckVolType)
                {
                    // 常量
                case 0:
					if (size < 1)
					{
						return retVolumn;
					}
                    retVolumn = spChAssay->sampleAspirateVols[0].originalSample;
                    break;
                case 1:
					if (size < 2)
					{
						return retVolumn;
					}
                    retVolumn = spChAssay->sampleAspirateVols[1].originalSample;
                    break;
                case 2:
					if (size < 3)
					{
						return retVolumn;
					}
                    retVolumn = spChAssay->sampleAspirateVols[2].originalSample;
                    break;
                default:
                    retVolumn = 0;
                    break;
                }

                return retVolumn;
            }
            // 机内稀释
            else
            {
                return GetSampleRatioVolumn(testItem.dilutionFactor);
            }
        }
    }

    // 是否ISE
    if (spManager->IsAssayISE(testItem.assayCode))
    {
        return ::ise::tf::g_ise_constants.ISE_SAMPLE_VOLUMN;
    }

    // 免疫设备
    auto spImmuneAssay = spManager->GetImmuneAssayInfo(testItem.assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
    if (spImmuneAssay != Q_NULLPTR)
    {
        return spImmuneAssay->reactInfo.suckVol * 10;
    }

    return retVolumn;
}

///
/// @brief 当前位置是否在范围内
///
/// @param[in]  isRerun		是否复查
/// @param[in]  mode		模式
/// @param[in]  isEmerge	是否急诊
/// @param[in]  rack		架号
/// @param[in]  postion		位置号
///
/// @return true:范围内
///
/// @par History:
/// @li 5774/WuHongTao，2023年10月11日，新建函数
///
bool QSampleAssaySelect::IsPostionProperty(bool isRerun, int mode, bool isEmerge, std::string rack, int postion)
{
	ULOG(LOG_INFO, "%s(rack: %s - Pos: %d)", __FUNCTION__, rack, postion);
	// 当前样本
	auto currentSample = GetCurrentSample();
	// 判断是否原位复查
	if (isRerun && currentSample.has_value())
	{
		// 原位复查直接返回
		auto sample = currentSample.value();
		if (sample.__isset.rack 
			&& sample.__isset.pos
			&& sample.rack == rack
			&& sample.pos == postion)
		{
			return true;
		}
	}

	// 样本号模式
	if (mode == 0)
	{
		// 样本号模式下只有急诊情况下需要检查架号，位置号
		if (isEmerge)
		{
			// 首先检查入口参数，架号位置号必须在范围内
			if (!rack.empty() && postion >= 1)
			{
				return false;
			}

			int rackData = stoi(rack);
			if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_STAT, rackData))
			{
				RackRangeOneType rr = CommonInformationManager::GetInstance()->GetRackRange(::tf::RackType::type::RACK_TYPE_STAT);
				TipDlg(tr("保存"), tr("架号超出急诊架号允许范围（%1-%2），请重新设置！").arg(rr.iLowerLmt).arg(rr.iUpperLmt)).exec();
				return false;
			}
		}
		// 样本号模式下无需检测架号位置号
		else
		{
			return true;
		}
	}
	// 样本架模式
	else if (mode == 2)
	{
		// 样本号模式，架号，位置受到限制
		if (isEmerge)
		{
			int rackData = stoi(rack);
			if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_STAT, rackData))
			{
				RackRangeOneType rr = CommonInformationManager::GetInstance()->GetRackRange(::tf::RackType::type::RACK_TYPE_STAT);
				TipDlg(tr("保存"), tr("架号超出急诊架号允许范围（%1-%2），请重新设置！").arg(rr.iLowerLmt).arg(rr.iUpperLmt)).exec();
				return false;
			}
		}
		// 若不是急诊，则是常规的样本
		else
		{
			int rackData = stoi(rack);
			if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_ROUTINE, rackData))
			{
				RackRangeOneType rr = CommonInformationManager::GetInstance()->GetRackRange(::tf::RackType::type::RACK_TYPE_ROUTINE);
				TipDlg(tr("保存"), tr("架号超出常规架号允许范围（%1-%2），请重新设置！").arg(rr.iLowerLmt).arg(rr.iUpperLmt)).exec();
				return false;
			}
		}
	}
	// 条码模式-只要不是在质控，校准，清洗架即可使用
	else
	{
		// 条码模式下只有样本无条码和急诊模式下才有架号位置号
		if (ui->sample_barcode_control->isChecked() || isEmerge)
		{
			int rackData = stoi(rack);
			if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_CALI, rackData))
			{
				TipDlg(tr("保存"), tr("非校准(样本)不允许使用在校准(架)")).exec();
				return false;
			}

			if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_QC, rackData))
			{
				TipDlg(tr("保存"), tr("非质控(样本)不允许使用在质控(架)")).exec();
				return false;
			}

			if (CommonInformationManager::GetInstance()->IsRackRangOut(::tf::RackType::type::RACK_TYPE_WASH, rackData))
			{
				TipDlg(tr("保存"), tr("不能使用清洗架")).exec();
				return false;
			}
		}
		else
		{
			return true;
		}
	}

	auto IndexSample = GetSampleByPostion(rack, postion, m_sampleTestMode);
	if (IndexSample)
	{
		auto currentSample = GetCurrentSample();
		// 判断是不是样本本身
		if (!currentSample || currentSample && currentSample->id != IndexSample->id)
		{
			TipDlg(tr("保存"), tr("输入架号位置号被占用，请重新输入！")).exec();
			return false;
		}
	}

	return true;
}

QSampleFilter::QSampleFilter(QObject* parent /*= nullptr*/)
{

}

QSampleFilter::~QSampleFilter()
{

}

bool QSampleFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{

    auto sample = DataPrivate::Instance().GetSampleByRow(sourceRow);
    if (sample.has_value() && sample->sampleType == tf::SampleType::type::SAMPLE_TYPE_PATIENT)
    {
        return true;
    }
    else
    {
        return false;
    }

}

boost::optional<tf::SampleInfo> QSampleFilter::GetSampleByIndex(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return boost::none;
    }

    auto sourceModule = qobject_cast<QSampleAssayModel*>(this->sourceModel());
    auto sourceIndex = this->mapToSource(index);
    sourceModule->SetCurrentSampleRow(sourceIndex.row());
    return DataPrivate::Instance().GetSampleByRow(sourceIndex.row());
}

bool QSampleFilter::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
    // 获取源model
    QSampleAssayModel* pSouceModel = dynamic_cast<QSampleAssayModel*>(sourceModel());
    if (pSouceModel == nullptr)
    {
        return false;
    }
    QVariant leftData = pSouceModel->data(source_left);
    QVariant rightData = pSouceModel->data(source_right);

    double leftValue = 0;
    double rightValue = 0;
    if (IsNumber(leftData, leftValue) && IsNumber(rightData, rightValue))
    {
        return leftValue < rightValue;
    }
    return leftData.toString() < rightData.toString();
}

///
///  @brief 检查字符串是否是纯数字
///
bool QSampleFilter::IsNumber(const QVariant & data, double & value) const
{
    bool ok = false;
    value = data.toDouble(&ok);
    return ok;
}
