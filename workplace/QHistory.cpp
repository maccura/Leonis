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
/// @file     QHistory.h
/// @brief    数据浏览页面
///
/// @author   5774/WuHongTao
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
#include "QHistory.h"
#include "ui_QHistory.h"

#include <QButtonGroup>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QCheckBox>
#include <QScrollBar>

#include <thread>
#include "QPushSampleToRemote.h"
#include "QReCheckDialog.h"
#include "QHistoryFilterSample.h"
#include "QAssayResultDetail.h"
#include "QHistoryItemResultDetailDlg.h"
#include "printsetdlg.h"
#include "SortHeaderView.h"
#include "WorkpageCommon.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "QDialogAi.h"
#include "QSampleAssayModel.h"

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
#include "shared/datetimefmttool.h"
#include "src/common/TimeUtil.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryKeyName.h"
#include "src/public/ch/ChCommon.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

///
///  @brief 获取状态字符串
///
///  @return	状态字符串
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年5月19日，新建函数
///
template <typename T>
QString GetStatus(const T& data)
{
    QString str(" F");
    if (data.m_retest)
    {
        str += " R";
    }
    if (data.m_sendLis)
    {
        str += " H";
    }

    return str.trimmed();
}

std::string GetResult(const std::shared_ptr<HistoryBaseDataByItem>& pItem, bool isRecheck)
{
    // 样本显示的设置
    const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();

    std::string strResult;
    if (!WorkpageCommon::IsShowConc(sampleSet, (::tf::HisSampleType::type)pItem->m_sampleType))
    {
        strResult = isRecheck ? pItem->m_reTestRLU : pItem->m_RLU;
    }
    else
    {
        int iPrecision = 2;
        double factor = 1.0;
        WorkpageCommon::GetPrecFactorUnit(pItem->m_assayCode, iPrecision, factor, pItem->m_unit);

        double dReslt = isRecheck ? (std::stod(pItem->m_reTestResult) * factor) : (pItem->m_testResult * factor);
        QString strTemp = QString::number(dReslt, 'f', iPrecision);
        strResult = strTemp.toStdString();				// 用于打印的结果
    }

    return strResult;
}


Q_DECLARE_METATYPE(::tf::HistoryBaseDataQueryCond)
QStringList g_searchList;
QHistory::QHistory(QWidget *parent)
    : QWorkShellPage(parent),
    m_sampleDetailDialog(Q_NULLPTR),
    m_sampleInfo(boost::none),
    m_recheckDialog(Q_NULLPTR),
    m_transferDialog(Q_NULLPTR),
    m_lastFilterObj(Q_NULLPTR),
    m_preventRow(-1), 
    m_preventRowAssaylist(-1),
    m_click(false),
    m_selectCheckBtn(Q_NULLPTR),
    m_statusShow(Q_NULLPTR),
    m_selectAssayCheckBtn(Q_NULLPTR),
    m_assayButton(Q_NULLPTR),
	m_dialogAi(Q_NULLPTR),
	m_status(Q_NULLPTR)
{
    ui = new Ui::QHistory();
    m_sCheckBtnText = tr("审核");
    m_sUnCheckBtnText = tr("取消审核");
    ui->setupUi(this);
    g_searchList = QStringList{ tr("样本号"),tr("样本条码") };
    m_pCommAssayMgr = CommonInformationManager::GetInstance();

    m_pPrintSetDlg = nullptr;
    Init();

    m_currentHighLightRow = 0;
}

QHistory::~QHistory()
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
void QHistory::RefreshCornerWidgets(int index)
{
    // 句柄检查（不能为空）
    if (m_parent == nullptr || m_cornerItem == nullptr)
    {
        return;
    }

    // 设置切换句柄
    m_parent->setCornerWidget(m_cornerItem);
    m_cornerItem->show();

    if (ui->stackedWidget->currentIndex() == 0)
    {
        gHisSampleAssayModel.SetSampleModuleType(QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE);
    }
    else
    {
        gHisSampleAssayModel.SetSampleModuleType(QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE);
    }

	m_selectCheckBtn->setChecked(false);
	m_selectAssayCheckBtn->setChecked(false);
    // 设置页面允许功能状态
    PageAllowFun();
}

///
/// @brief 初始化页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QHistory::Init()
{
    // 硬件暂不支持视觉识别，暂时隐藏对应按钮——mod_tcx
    ui->flat_Ai->setVisible(false);

    // 1：右上角部件的显示
    m_cornerItem = new QWidget(m_parent);
    QHBoxLayout* hlayout = new QHBoxLayout(m_cornerItem);
    m_cornerItem->setLayout(hlayout);

    m_controlModify = false;
    m_shiftKey = false;
    m_showResultDetail = nullptr;
    m_selectSampleModel = nullptr;
    m_filterDialog = nullptr;
    m_filterDialog = new QHistoryFilterSample(this);
    connect(m_filterDialog, SIGNAL(finished()), this, SLOT(OnShowFilterSampleInfo()));
    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    // 设置互斥
    pButtonGroup->setExclusive(true);
    QRadioButton *pButton = new QRadioButton(tr("按样本"));
    pButton->setObjectName(QStringLiteral("showBySample_btn"));
    pButton->setCheckable(true);
    connect(pButton, SIGNAL(clicked()), this, SLOT(OnShowBySample()));
    pButton->setChecked(true);
    pButtonGroup->addButton(pButton);
    hlayout->addWidget(pButton);

    pButton = new QRadioButton(tr("按项目"));
    pButton->setObjectName(QStringLiteral("showByAssay_btn"));
    connect(pButton, SIGNAL(clicked()), this, SLOT(OnShowByAssay()));
    pButtonGroup->addButton(pButton);
    hlayout->addWidget(pButton);
    m_cornerItem->hide();

    // 设置数据模型
    ResetSampleSelectModel();
    // 数据改变
    connect(&gHisSampleAssayModel, SIGNAL(sampleChanged()), this, SLOT(OnDataChanged()));
    // 审核按钮按钮
    connect(ui->flat_examine, SIGNAL(clicked()), this, SLOT(OnCheckSample()));
    // 手工传输
    connect(ui->flat_manual_transfer, SIGNAL(clicked()), this, SLOT(OnTransfer()));
    // 复查按钮
    connect(ui->flat_recheck, SIGNAL(clicked()), this, SLOT(OnRecheckSample()));
    // 选择模式，增加移动
    connect(ui->sample_list->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QHistory::OnSampleListSelChanged);
    // 选择模式，增加移动
    connect(ui->show_assay_list->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QHistory::OnAssayListSelChanged);

    // 查询
    connect(ui->pushButton_search, SIGNAL(clicked()), this, SLOT(OnSearch()));
    // 向上查找
    connect(ui->btn_pre, SIGNAL(clicked()), this, SLOT(OnSearchPre()));
    // 向下查找
    connect(ui->btn_next, SIGNAL(clicked()), this, SLOT(OnSearchNext()));

    // 样本信息表日期列设置代理
    ui->sample_list->setItemDelegateForColumn((int)QHistorySampleAssayModel::COL::ENDTIME, new CReadOnlyDelegate(this));
    ui->sample_list->setItemDelegateForColumn((int)QHistorySampleAssayModel::COL::RECHECKENDTIME, new CReadOnlyDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn(int(QHistorySampleAssayModel::COLASSAY::ENDTIME), new CReadOnlyDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn(int(QHistorySampleAssayModel::COLASSAY::RECHECKENDTIME), new CReadOnlyDelegate(this));

    ui->stackedWidget->setCurrentIndex(0);
    // 显示结果详情
    connect(ui->result_detail_btn, SIGNAL(clicked()), this, SLOT(OnShowResultDetail()));
    // 显示筛选结果对话框
    connect(ui->selectCheck, SIGNAL(clicked()), this, SLOT(OnShowFilterDialog()));
    //打印按钮被点击
    connect(ui->flat_print, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));
    // 数据导出按钮被选中
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    connect(this, SIGNAL(CloseFilterLable()), this, SLOT(OnCloseFilter()));

    for (const auto& searchType : g_searchList)
    {
        ui->comboBox->addItem(searchType);
    }
	// 按样本、按条码方式发生改变时清空输入的查询内容
	connect(ui->comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [&]() {
		ui->lineEdit->clear();
	});
    connect(m_filterDialog, &QHistoryFilterSample::fastFilterFinished, this, &QHistory::OnfastFilterFinished);
    OnfastFilterFinished(::tf::HistoryBaseDataQueryCond());
    connect(ui->pushButton_fastfilter_1, &QPushButton::clicked, this, &QHistory::OnFastFilterSampleInfo);
    connect(ui->pushButton_fastfilter_2, &QPushButton::clicked, this, &QHistory::OnFastFilterSampleInfo);
    connect(ui->pushButton_fastfilter_3, &QPushButton::clicked, this, &QHistory::OnFastFilterSampleInfo);
    connect(ui->pushButton_fastfilter_4, &QPushButton::clicked, this, &QHistory::OnFastFilterSampleInfo);
    connect(ui->pushButton_fastfilter_5, &QPushButton::clicked, this, &QHistory::OnFastFilterSampleInfo);

    connect(&gHisSampleAssayModel, &QHistorySampleAssayModel::HightLightChanged, this, &QHistory::OnHightLightChanged);

	// 视觉识别结果
	connect(ui->flat_Ai, &QPushButton::clicked, this, &QHistory::OnAIVisonResultIdentifyClicked);

    connect(ui->sample_list, &QTableView::clicked, this, &QHistory::OnSampleListClick);
    connect(ui->show_assay_list, &QTableView::clicked, this, &QHistory::OnAssayListClick);

    // 显示后执行
    connect(this, SIGNAL(afterShow()), this, SLOT(OnAfterShow()));

    // 设置排序
    InitHeader(ui->sample_list, m_selectCheckBtn, m_statusShow);
    InitHeader(ui->show_assay_list, m_selectAssayCheckBtn, m_assayButton);

    SetSampleBrowseModel();
    // 按样本展示中复选框和？跟随
    connect(ui->sample_list->horizontalScrollBar(), &QScrollBar::valueChanged, this, [&](int value) {
        ProcScrollBarValueChanged(ui->sample_list, m_selectCheckBtn, m_statusShow);
    });
    emit ui->sample_list->horizontalScrollBar()->valueChanged(0);
    // 按项目展示中复选框和？跟随
    connect(ui->show_assay_list->horizontalScrollBar(), &QScrollBar::valueChanged, this, [&](int value) {
        ProcScrollBarValueChanged(ui->show_assay_list, m_selectAssayCheckBtn, m_assayButton);
    });
    emit ui->show_assay_list->horizontalScrollBar()->valueChanged(0);

    // 框架的int64_t的名称会转换为__int64导致和槽函数的类型不匹配，因此这里先手动注册一下
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<QVector<int64_t>>("QVector<int64_t>");
    // 处理上传Lis消息
    REGISTER_HANDLER(MSG_ID_SAMPLE_LIS_UPDATE, this, OnLisChanged);
    // 处理审核消息
    REGISTER_HANDLER(MSG_ID_SAMPLE_AUDIT_UPDATE_DATABROWSE, this, OnAuditChanged);
    // 工作页面的设置更新
    REGISTER_HANDLER(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE, this, OnUpdateButtonStatus);

    // 处理复查结果更新消息
    REGISTER_HANDLER(MSG_ID_WORK_PAGE_RETEST_RESULT_UPDATE, this, OnRetestResultChanged);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

    // 刷新当前行
    REGISTER_HANDLER(MSG_ID_SAMPLE_INFO_UPDATE, this, OnRefreshRow);
	// 条码/序号模式切换
	REGISTER_HANDLER(MSG_ID_DETECTION_UPDATE, this, OnChangeMode);
    // 项目配置信息更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnDataChanged);
	// 同步delegate的顺序
    REGISTER_HANDLER(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE, this, OnUpdateColumn);

    OnUpdateButtonStatus();
	AssignSampleComb(ui->comboBox);

    // 注册事件过滤
    ui->sample_list->viewport()->installEventFilter(this);
    ui->assay_list->viewport()->installEventFilter(this);
    ui->show_assay_list->viewport()->installEventFilter(this);

    //多语言处理
    QString /*tmpStr = ui->result_detail_btn->fontMetrics().elidedText(ui->result_detail_btn->text(), Qt::ElideRight, 80);
    ui->result_detail_btn->setToolTip(ui->result_detail_btn->text());
    ui->result_detail_btn->setText(tmpStr);

    tmpStr = ui->flat_manual_transfer->fontMetrics().elidedText(ui->flat_manual_transfer->text(), Qt::ElideRight, 80);
    ui->flat_manual_transfer->setToolTip(ui->flat_manual_transfer->text());
    ui->flat_manual_transfer->setText(tmpStr); */

    tmpStr = ui->flat_recalculate->fontMetrics().elidedText(ui->flat_recalculate->text(), Qt::ElideRight, 80);
    ui->flat_recalculate->setToolTip(ui->flat_recalculate->text());
    ui->flat_recalculate->setText(tmpStr);
	UpdateButtonStatus();
    
}

void QHistory::OnSampleListClick(const QModelIndex& current)
{
    m_currentHighLightRow = current.row();
    // 实现shift健的功能
    if (m_shiftKey)
    {
        if (CheckIndexRange(m_preventRow) && CheckIndexRange(current.row()))
        {
            bool bPrevLessThanCurr = (m_preventRow < current.row());
            int iBegin = bPrevLessThanCurr ? m_preventRow : current.row();
            int iEnd = bPrevLessThanCurr ? current.row() : m_preventRow;

            for (int index = iBegin; index <= iEnd; index++)
            {
                QModelIndex indexSet = gHisSampleAssayModel.index(index, 1);
                gHisSampleAssayModel.SetCheckBoxStatus(indexSet, true);
            }
        }
    }
    else
    {
        // modify for bug3254 
        if (!m_click)
        {
            gHisSampleAssayModel.SetCheckBoxStatus(current, m_controlModify);
        }
        m_preventRow = current.row();
    }

    m_click = false;
    // 点击后需要选中勾选行
    if (current.column() != 0)
    {
        ui->sample_list->clearSelection();
    }

    auto checkedIndexs = gHisSampleAssayModel.GetCheckedModelIndex();
    for (const auto& index : checkedIndexs)
    {
        ui->sample_list->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    m_selectCheckBtn->setChecked(gHisSampleAssayModel.IsSelectAll());

    UpdateButtonStatus();
    OnShowSampleDetail(current);
}

void QHistory::OnAssayListClick(const QModelIndex& current)
{
    QHistorySampleAssayModel& insAs = gHisSampleAssayModel;
    m_currentHighLightRow = current.row();
    // 实现shift健的功能
    if (m_shiftKey)
    {
        bool bPrevLessThanCurr = (m_preventRowAssaylist < current.row());
        int iBegin = bPrevLessThanCurr ? m_preventRowAssaylist : current.row();
        int iEnd = bPrevLessThanCurr ? current.row() : m_preventRowAssaylist;

        for (int index = iBegin; index <= iEnd; index++)
        {
            QModelIndex tmpIdx = insAs.index(index, 0);
            insAs.SetCheckBoxStatus(tmpIdx, true);
            ui->show_assay_list->selectionModel()->select(tmpIdx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
    else
    {
        // modify for bug3254 
        if (!m_click)
        {
            insAs.SetCheckBoxStatus(current, m_controlModify);
        }
        // 点击后需要选中勾选行
        if (current.column() != 0)
        {
            ui->show_assay_list->clearSelection();
        }
        QModelIndexList checkedIndexs = insAs.GetCheckedModelIndex();
        for (const auto& index : checkedIndexs)
        {
            ui->show_assay_list->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }

    m_click = false;
    m_selectAssayCheckBtn->setChecked(insAs.IsSelectAll());
    m_preventRowAssaylist = current.row();
    UpdateButtonStatus();
    OnShowSampleDetail(current);
}

void QHistory::OnSampleListSelChanged(const QModelIndex& current, const QModelIndex& pre)
{
	ProSelect(ui->sample_list, current);
}

void QHistory::OnAssayListSelChanged(const QModelIndex& current, const QModelIndex& pre)
{
	ProSelect(ui->show_assay_list, current);
}

void QHistory::ProcScrollBarValueChanged(QTableView* qTb, QCheckBox* qCheck, QPushButton* qBtn)
{
    if (qTb == Q_NULLPTR || qCheck == Q_NULLPTR || qBtn == Q_NULLPTR)
    {
        return;
    }

	auto header = qTb->horizontalHeader();
    qCheck->setGeometry(header->sectionViewportPosition(0) + 5, 0, 50, header->height());
    qBtn->setGeometry(header->sectionViewportPosition(1) + header->sectionSize(1) - 10, 15, 50, header->height());
}

void QHistory::ProSelect(QTableView* tableView, const QModelIndex& current)
{
    if (!current.isValid() || tableView == Q_NULLPTR)
    {
        return;
    }
    QModelIndex kepCur = current; // selectRow会修改current指向，所以暂缓存起来
    if (!m_controlModify)
    {
        if (current.column() != 0)
        {
            tableView->clearSelection();
            gHisSampleAssayModel.SetOrClearAllCheck(false);
        }
    }

    if (!m_shiftKey)
    {
        // modify for bug3254 
        gHisSampleAssayModel.SetCheckBoxStatus(kepCur, m_controlModify);
        tableView->selectionModel()->select(kepCur, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        m_click = true;

        // 使用键盘上下键选择时作用
        OnShowSampleDetail(current);
        UpdateButtonStatus();
    }
}

///
/// @brief 更新按钮状态
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月18日，新建函数
///
void QHistory::UpdateButtonStatus()
{
    ui->patient_btn->setEnabled(false);
    ui->result_detail_btn->setEnabled(false);
    ui->flat_recheck->setEnabled(false);
    ui->flat_examine->setText(m_sCheckBtnText);
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

    const QHistorySampleAssayModel &insHisSmpModel = gHisSampleAssayModel;
    if (insHisSmpModel.Empty())
    {
        ULOG(LOG_INFO, "Empty history sample assaymodel.");
        return;
    }
    QModelIndexList checkedIndexs = insHisSmpModel.GetCheckedModelIndex();
    std::sort(checkedIndexs.begin(), checkedIndexs.end(), [&](const QModelIndex& a, const QModelIndex& b) {
        return a.row() < b.row();
    });

    bool onlyOne = (checkedIndexs.size() == 1) ? true : false;
    bool selectEmpty = (checkedIndexs.empty()) ? true : false;

    // 使用是否审核来判断打印按钮的使能
    auto funcEnablePrintBtnByAudit = [&]() {
        if (!selectEmpty)
        {
            ui->flat_examine->setEnabled(true);
            if (m_workSet.audit)
            {
                if (insHisSmpModel.GetCheck(checkedIndexs[0]) == 1)
                {
                    ui->flat_examine->setText(m_sUnCheckBtnText);
                    ui->flat_print->setEnabled(true);
                }
            }
            else
            {
                // 关闭审核后都可打印
                ui->flat_print->setEnabled(true);
            }
            ui->export_btn->setEnabled(true);
			ui->flat_Ai->setEnabled(true);
        }
    };

    // 按项目展示的处理
    if (insHisSmpModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        if (onlyOne)
        {
            auto assayCode = insHisSmpModel.GetAssayItemData(checkedIndexs[0])->m_assayCode;
            // 1:计算项目不能显示结果详情 2:只能选中一个项目
            if (!CommonInformationManager::IsCalcAssay(assayCode) && onlyOne)
            {
                ui->result_detail_btn->setEnabled(true);
            }
        }

        // 按项目展示，都不能使用手工传输（modify for bug3220）
        ui->flat_manual_transfer->setEnabled(false);
        // 审核功能
        funcEnablePrintBtnByAudit();
		// 按项目展示审核功能需置灰（bug 0013657）
		ui->flat_examine->setEnabled(false);

        return;
    }
    // 按样本显示的判断
    else
    {
        // 患者信息和结果详情，都只能选中一个的时候才能使能
        if (onlyOne)
        {
            // 病人信息
            ui->patient_btn->setEnabled(true);
            // 结果详情
            ui->result_detail_btn->setEnabled(true);
        }
        // 审核功能
        funcEnablePrintBtnByAudit();

        // 校准的样本无法手工传输
        std::vector<std::shared_ptr<HistoryBaseDataSample>> items;
        insHisSmpModel.GetSelectedSamples(checkedIndexs, items);
        if (items.empty() || items[0]->m_sampleType == tf::HisSampleType::SAMPLE_SOURCE_CL)
        {
            ui->flat_manual_transfer->setEnabled(false);
        }
        else
        {
            ui->flat_manual_transfer->setEnabled(true);
        }
    }
}

void QHistory::AssignSampleComb(QComboBox* pComb)
{
	if (pComb == nullptr)
	{
		return;
	}

	DetectionSetting detectionSetting;
	if (!DictionaryQueryManager::GetDetectionConfig(detectionSetting))
	{
		ULOG(LOG_ERROR, "Failed to get inJectionModelConfig.");
		return;
	}

	pComb->clear();
	if (detectionSetting.testMode == ::tf::TestMode::BARCODE_MODE)
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
/// @brief 重置项目详情列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QHistory::ResetAssayModel()
{
    ui->assay_list->setModel(&QHistorySampleModel_Assay::Instance());

    QStringList assayHeadList = { tr("复查"), tr("项目"), tr("检测结果"),
        tr("结果状态"), tr("复查结果"), tr("复查数据报警"), tr("单位") , tr("模块") };

    ui->assay_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->assay_list->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->assay_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	/*
    ui->assay_list->setItemDelegateForColumn((int)QHistorySampleModel_Assay::COL::ASSAY, new QResultDelegate(this));
    ui->assay_list->setItemDelegateForColumn((int)QHistorySampleModel_Assay::COL::RESULT, new CReadOnlyDelegate(this));
    ui->assay_list->setItemDelegateForColumn((int)QHistorySampleModel_Assay::COL::RESULTSTATUS, new CReadOnlyDelegate(this));
    ui->assay_list->setItemDelegateForColumn((int)QHistorySampleModel_Assay::COL::RECHECKRESULT, new CReadOnlyDelegate(this));
    ui->assay_list->setItemDelegateForColumn((int)QHistorySampleModel_Assay::COL::RECHECKSTATUS, new CReadOnlyDelegate(this));
    */
    ui->assay_list->horizontalHeader()->setHighlightSections(false);
    ui->assay_list->verticalHeader()->setVisible(false);
    ui->assay_list->horizontalHeader()->setStretchLastSection(true);
    ui->assay_list->horizontalHeader()->setTextElideMode(Qt::ElideLeft);

    ui->assay_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->assay_list->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->assay_list->setColumnWidth(0, 200);
    ui->assay_list->setColumnWidth(1, 100);
    ui->assay_list->setColumnWidth(2, 100);
    ui->assay_list->setColumnWidth(3, 100);
    ui->assay_list->setColumnWidth(4, 110);
    ui->assay_list->setColumnWidth(5, 100);
	OnUpdateColumn();
}

///
/// @brief 初始化表头
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月4日，新建函数
///
void QHistory::InitHeader(QTableView* tableView, QCheckBox*& selectCheckBtn, QPushButton*& statusBtn)
{
    if (tableView == nullptr)
    {
        return;
    }

    auto header = new SortHeaderView(Qt::Horizontal, tableView);
    header->setSortIndicatorShown(true);
    tableView->setSortingEnabled(true);
    header->setSortIndicator(-1, Qt::DescendingOrder);
    tableView->setHorizontalHeader(header);
    // 全选对话框
    {
        auto header = tableView->horizontalHeader();
        selectCheckBtn = new QCheckBox(header);
        connect(selectCheckBtn, &QCheckBox::clicked, this,
            [this, tableView]()
        {
            auto spCheckBox = qobject_cast<QCheckBox*>(sender());
            if (spCheckBox == Q_NULLPTR)
            {
                return;
            }

            bool isCheck = spCheckBox->isChecked();
            // 全选
            if (isCheck)
            {
                gHisSampleAssayModel.SetOrClearAllCheck(true);
				tableView->selectAll();
            }
            // 全部取消
            else
            {
                tableView->clearSelection();
                gHisSampleAssayModel.SetOrClearAllCheck(false);
            }
            UpdateButtonStatus();
        });

        // 设置按钮的位置
        selectCheckBtn->setGeometry(header->sectionViewportPosition(0) /*+ header->sectionSize(0)*/ + 5, 5, 50, header->height());
    }

    // 设置状态列旁边的帮助按钮
    {
        auto headView = tableView->horizontalHeader();
		statusBtn = new CustomButton("", headView);
		statusBtn->setObjectName(QStringLiteral("tooltip_btn"));
		QString tipsData = m_pCommAssayMgr->GetTipsContent(m_workSet.aiRecognition);
		statusBtn->setToolTip(tipsData);
        // 设置按钮的位置
		statusBtn->setGeometry(headView->sectionViewportPosition(0) + headView->sectionSize(0) - 2, 15, 50, headView->height());
    }

    connect(header, &SortHeaderView::SortOrderChanged, this, [this, header](int logicIndex, SortHeaderView::SortOrder order)
    {
        std::vector<int> vecSortRow_databrowse = {
            static_cast<int>(QHistorySampleAssayModel::COL::SEQNO),
            static_cast<int>(QHistorySampleAssayModel::COL::BARCODE),
            static_cast<int>(QHistorySampleAssayModel::COL::TYPE),
            static_cast<int>(QHistorySampleAssayModel::COL::POS),
            static_cast<int>(QHistorySampleAssayModel::COL::ENDTIME),
            static_cast<int>(QHistorySampleAssayModel::COL::RECHECKENDTIME) };

        auto it_browse = std::find(vecSortRow_databrowse.begin(), vecSortRow_databrowse.end(), logicIndex);
        if (it_browse == vecSortRow_databrowse.end() && \
            gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
        {
            return;
        }

        std::vector<int> vecSortRow_assay = {
            static_cast<int>(QHistorySampleAssayModel::COLASSAY::SEQNO),
            static_cast<int>(QHistorySampleAssayModel::COLASSAY::BARCODE),
            static_cast<int>(QHistorySampleAssayModel::COLASSAY::TYPE),
            static_cast<int>(QHistorySampleAssayModel::COLASSAY::POS),
            static_cast<int>(QHistorySampleAssayModel::COLASSAY::ENDTIME),
            static_cast<int>(QHistorySampleAssayModel::COLASSAY::RECHECKENDTIME) };
        auto it_assay = std::find(vecSortRow_assay.begin(), vecSortRow_assay.end(), logicIndex);

        if (it_assay == vecSortRow_assay.end() && \
            gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
        {
            return;
        }

        //无效index或NoOrder就设置为默认未排序状态
        if (order == SortHeaderView::NoOrder) {
            //去掉排序三角样式
            header->setSortIndicator(-1, Qt::DescendingOrder);
            //-1则还原model默认顺序
            gHisSampleAssayModel.Refush();
        }
        else
        {
            auto qtorder = order == SortHeaderView::SortOrder::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            header->setSortIndicator(logicIndex, qtorder);

            gHisSampleAssayModel.Sort(logicIndex, order);
        }
    });
}

///
/// @brief 重置数据浏览样本筛选数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QHistory::ResetSampleSelectModel()
{
    ui->sample_list->setModel(&gHisSampleAssayModel);
    ui->show_assay_list->setModel(&gHisSampleAssayModel);
    gHisSampleAssayModel.SetSampleModuleType(QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE);
}

void QHistory::OnUpdateColumn()
{
	auto funcAssignAttri = [this](QTableView *qTb, const std::vector<int>& vecColumns, SampleColumn::SAMPLEMODE modeType)
	{
		int iResultAssayCol;
		if (modeType == SampleColumn::SAMPLEMODE::SAMPLEASSAY)
		{
			iResultAssayCol = (int)SampleColumn::SAMPLEASSAY::ASSAY;
		}
		else if (modeType == SampleColumn::SAMPLEMODE::ASSAYLIST)
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
				int column = attribute.first;
				if (modeType == SampleColumn::SAMPLEMODE::SAMPLEASSAY)
				{
					column = column - 1;
				}

				// 设置列
				if (iCol == iResultAssayCol)
				{
					qTb->setItemDelegateForColumn(column, new QResultDelegate(this));
				}
				else
				{
					qTb->setItemDelegateForColumn(column, new CReadOnlyDelegate(this));
				}

				// 设置宽度
				if (attribute.second > 0)
				{
					qTb->setColumnWidth(column, attribute.second);
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

///
/// @brief 设置样本浏览模型
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QHistory::SetSampleBrowseModel()
{
    ResetAssayModel();
    // 设置表格选中模式为行选中，不可多选
    ui->sample_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->sample_list->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->sample_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // modify for bug3189
    ui->sample_list->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->sample_list->horizontalHeader()->setHighlightSections(false);
    ui->sample_list->verticalHeader()->setVisible(false);

    // 设置触摸屏的拖动模式（平移推动模式/滚动条拖动模式）
    ui->sample_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->sample_list->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->sample_list->horizontalHeader()->setMinimumSectionSize(45);

    ui->sample_list->setColumnWidth(0, 45);
    ui->sample_list->setColumnWidth(1, 63);
    ui->sample_list->setColumnWidth(2, 110);
    ui->sample_list->setColumnWidth(3, 200);
    ui->sample_list->setColumnWidth(4, 110);
    ui->sample_list->setColumnWidth(5, 100);
    ui->sample_list->setColumnWidth(6, 185);
    ui->sample_list->setColumnWidth(7, 185);
    ui->sample_list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->sample_list->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);

    ui->sample_list->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));
    ui->sample_list->setItemDelegateForColumn(1, new QStatusDelegate(this));
    ui->sample_list->horizontalHeader()->setTextElideMode(Qt::ElideLeft);
    ui->sample_list->horizontalHeader()->setStretchLastSection(true);

    // 设置表格选中模式为行选中，不可多选
    ui->show_assay_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->show_assay_list->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->show_assay_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // modify for bug3189
    ui->show_assay_list->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->show_assay_list->horizontalHeader()->setHighlightSections(false);
	/*
    ui->show_assay_list->setItemDelegateForColumn(int(QHistorySampleAssayModel::COLASSAY::ASSAY), new QResultDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn(int(QHistorySampleAssayModel::COLASSAY::RESULT), new CReadOnlyDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn(int(QHistorySampleAssayModel::COLASSAY::RECHECKRESULT), new CReadOnlyDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn((int)QHistorySampleAssayModel::COLASSAY::RESULTSTATUS, new CReadOnlyDelegate(this));
    ui->show_assay_list->setItemDelegateForColumn((int)QHistorySampleAssayModel::COLASSAY::RECHECKSTATUS, new CReadOnlyDelegate(this));
	*/
    ui->show_assay_list->verticalHeader()->setVisible(false);
    ui->show_assay_list->horizontalHeader()->setStretchLastSection(true);
    // 设置触摸屏的拖动模式（平移推动模式/滚动条拖动模式）
    ui->show_assay_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->show_assay_list->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->show_assay_list->horizontalHeader()->setMinimumSectionSize(45);

    ui->show_assay_list->setColumnWidth(0, 45);
    ui->show_assay_list->setColumnWidth(1, 63);
    ui->show_assay_list->setColumnWidth(2, 110);
    ui->show_assay_list->setColumnWidth(3, 200);
    ui->show_assay_list->setColumnWidth(4, 110);
    ui->show_assay_list->setColumnWidth(5, 100);
    ui->show_assay_list->setColumnWidth(6, 185);
    ui->show_assay_list->setColumnWidth(7, 185);
    ui->show_assay_list->setColumnWidth(8, 200);
    ui->show_assay_list->setColumnWidth(12, 110);
    ui->show_assay_list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->show_assay_list->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);

    ui->show_assay_list->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));
    ui->show_assay_list->setItemDelegateForColumn(1, new QStatusDelegate(this));
    ui->show_assay_list->horizontalHeader()->setTextElideMode(Qt::ElideLeft);

	OnUpdateColumn();
    // 设置页面允许功能状态
    PageAllowFun();
}

void QHistory::UpdateCheckAllBoxStatus()
{
	QHistorySampleAssayModel::VIEWMOUDLE curModle = gHisSampleAssayModel.GetModule();
	if (curModle == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
    {
        m_selectCheckBtn->setChecked(gHisSampleAssayModel.IsSelectAll());
	}
	else if (curModle == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        m_selectAssayCheckBtn->setChecked(gHisSampleAssayModel.IsSelectAll());
	}
}

void QHistory::OnAfterShow()
{
    QHistorySampleAssayModel::VIEWMOUDLE curModle = gHisSampleAssayModel.GetModule();
	UpdateCheckAllBoxStatus();

    auto rIter = m_rowSelectedBeforeHide.find(curModle);
    if (rIter == m_rowSelectedBeforeHide.end() || rIter->second == -1)
    {
        ULOG(LOG_INFO, "No row selected before hide.");
        return;
    }

    // 恢复之前的选中
    if (curModle == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
    {
        QModelIndex curIdx = ui->sample_list->model()->index(rIter->second, 1);
        OnSampleListSelChanged(curIdx, curIdx);
    }
    else if (curModle == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        QModelIndex curIdx = ui->show_assay_list->model()->index(rIter->second, 1);
        OnAssayListSelChanged(curIdx, curIdx);
    }
}

///
/// @brief 查询历史数据
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2022年12月29日，新建函数
///
void QHistory::OnSearch()
{
    if (ui->lineEdit->text().isEmpty())
    {
        TipDlg(tr("提示"), tr("查找关键字不能为空！")).exec();
        return;
    }

    // 从筛选结果中模糊匹配
    QString strTargFiled = ui->comboBox->currentText();
    int iFindRes = gHisSampleAssayModel.LikeSearch(ui->lineEdit->text(), strTargFiled);
    if (iFindRes < 0)
    {
        TipDlg(tr("提示"), tr("未查询到符合条件的样本！")).exec();
    }
}

void QHistory::OnChangeMode(DetectionSetting detectionSetting)
{
	AssignSampleComb(ui->comboBox);
}

void QHistory::CheckAndSearch(int direction)
{
    if (ui->lineEdit->text().isEmpty())
    {
        TipDlg(tr("提示"), tr("查找关键字不能为空")).exec();
        return;
    }

    // 从筛选结果中模糊匹配
    int iRet = gHisSampleAssayModel.LikeSearch(ui->lineEdit->text(), ui->comboBox->currentText(), direction, m_currentHighLightRow);

    // 使选中行当前屏可见
    QHistorySampleAssayModel::VIEWMOUDLE curViewModle = gHisSampleAssayModel.GetModule();
    QTableView* curTab = (curViewModle == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE) ? ui->sample_list : ui->show_assay_list;
    QModelIndex index = curTab->model()->index(m_currentHighLightRow, 1);
    curTab->scrollTo(index, QAbstractItemView::PositionAtCenter);

    if (iRet == -1)// 向上越界
    {
        TipDlg(tr("当前为第一个查找结果，无法向上查找！")).exec();
    }
    else if (iRet == -2)// 向下越界
    {
        TipDlg(tr("当前为最后一个查找结果，无法向下查找！")).exec();
    }
}

void QHistory::GetImAssayTestResult(int64_t sampleId, int64_t itemId, const boost::posix_time::ptime& retime, 
    std::shared_ptr<::im::tf::AssayTestResult>& pFirst, std::shared_ptr<::im::tf::AssayTestResult>& pReTest)
{
    // 获取测试项目的首次和复查结果
    ::im::tf::AssayTestResultQueryResp queryAssaycond;
    ::im::tf::AssayTestResultQueryCond assayTestResult;
    assayTestResult.__set_sampleInfoId(sampleId);
    assayTestResult.__set_testItemId(itemId);

    ::im::i6000::LogicControlProxy::QueryAssayTestResult(queryAssaycond, assayTestResult);
    if (queryAssaycond.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS || queryAssaycond.lstAssayTestResult.empty())
    {
        ULOG(LOG_ERROR, "Failed to query im::assaytestresult by sampleid:%lld, itemid:%lld.", sampleId, itemId);
    }
    else
    {
        std::string strReMaxTime = PosixTimeToTimeString(retime);
        bool bFlagAlreadyFirst = false;
        for (const im::tf::AssayTestResult& testResult : queryAssaycond.lstAssayTestResult)
        {
            // 根据时间匹配复查结果
            if (testResult.endTime == strReMaxTime)
            {
                pReTest = std::make_shared<::im::tf::AssayTestResult>(testResult);
            }
            // 如果conc小于0，被认为是测试失败结果，则跳过该结果
            else if (!bFlagAlreadyFirst && testResult.conc >= 0)
            {
                pFirst = std::make_shared<::im::tf::AssayTestResult>(testResult);
                bFlagAlreadyFirst = true;
            }
        }
    }
}

///
/// @brief 向上查询历史数据
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2022年12月29日，新建函数
///
void QHistory::OnSearchPre()
{
    CheckAndSearch(1);
}

///
/// @brief 向下查询历史数据
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2022年12月29日，新建函数
///
void QHistory::OnSearchNext()
{
    CheckAndSearch(2);
}

///
/// @brief 显示样本的详细信息
///
/// @param[in]  index  样本的位置
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QHistory::OnShowSampleDetail(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    // 如果选中的当前行
    if (gHisSampleAssayModel.IsRowChecked(index.row()))
    {
        auto sampleID = gHisSampleAssayModel.GetSampleID(index);
        QHistorySampleModel_Assay::Instance().Update(sampleID, gHisSampleAssayModel.GetQueryCond());
    }
    else
    {
        int64_t curSampleId = gHisSampleAssayModel.GetSelectedSampleID();
        if (curSampleId != -1)
        {
            QHistorySampleModel_Assay::Instance().Update(curSampleId, gHisSampleAssayModel.GetQueryCond());
        }
    }
}

///
/// @brief 显示结果详情
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QHistory::OnShowResultDetail()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 创建结果详情对话框
    if (m_sampleDetailDialog == Q_NULLPTR)
    {
        m_sampleDetailDialog = new QHistoryItemResultDetailDlg(this);
        connect(m_sampleDetailDialog, &QHistoryItemResultDetailDlg::SigPrevBtnClicked, this, [&]() {OnMoveBtn(false); });
        connect(m_sampleDetailDialog, &QHistoryItemResultDetailDlg::SigNextBtnClicked, this, [&]() {OnMoveBtn(true); });
        connect(m_sampleDetailDialog, SIGNAL(dataChanged()), this, SLOT(OnDataChanged()));
        // 复查结果变化时更新历史数据界面
        connect(m_sampleDetailDialog, &QHistoryItemResultDetailDlg::SigRetestResultChanged, this, &QHistory::OnUIRetestResultChanged);
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
void QHistory::OnShowFilterDialog()
{
    if (m_filterDialog == nullptr)
    {
        ULOG(LOG_WARN, "Null filter dialog.");
        return;
    }

	UpdateButtonStatus();
    m_filterDialog->show();
}

///
/// @brief 筛选范围页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QHistory::OnReturnPage()
{
    SetSampleBrowseModel();
	UpdateCheckAllBoxStatus();
}

///
/// @brief 刷新过滤样本
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QHistory::OnShowFilterSampleInfo()
{
    // 发送筛选消息
    auto oldCond = gHisSampleAssayModel.GetQueryCond();
    ::tf::HistoryBaseDataQueryCond qryCond;
    m_filterDialog->GetQueryConditon(qryCond);
    qryCond.__set_orderBy(oldCond.orderBy);
    m_filterDialog->setProperty("cond", QVariant::fromValue(qryCond));
    m_lastFilterObj = m_filterDialog;

    UnCheckedFastFilterBtn(qryCond);

    emit ShowTipMessage(QueryCondToStr(qryCond));

    gHisSampleAssayModel.Update(qryCond);
	UpdateCheckAllBoxStatus();
}

///
///
/// @brief 快捷筛选
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月14日，新建函数
///
void QHistory::OnFastFilterSampleInfo()
{
    QObject * senderObj = sender();
    if (nullptr == senderObj)
    {
        return;
    }
    auto qryCond = senderObj->property("cond").value<::tf::HistoryBaseDataQueryCond>();
    // 选中和取消与数据浏览界面操作逻辑保持一致
    if (senderObj == m_lastFilterObj)
    {
        m_lastFilterObj = nullptr;
        emit SignalHideFilterLable();
        // 取消快捷筛选按钮的check状态
        OnCloseFilter();
    }
    else
    {
        emit ShowTipMessage(QueryCondToStr(qryCond));
        m_lastFilterObj = senderObj;
        gHisSampleAssayModel.Update(qryCond);
    }
}

void QHistory::OnHightLightChanged(int index)
{
    m_currentHighLightRow = index;
    QHistorySampleAssayModel::VIEWMOUDLE curViewModle = gHisSampleAssayModel.GetModule();
    if (curViewModle == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
    {
        ui->sample_list->clearSelection();
        m_selectCheckBtn->setChecked(false);
        QModelIndex curIdx = ui->sample_list->model()->index(index, 1);
        OnSampleListSelChanged(curIdx, curIdx);
    }
    else if (curViewModle == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        ui->show_assay_list->clearSelection();
        m_selectAssayCheckBtn->setChecked(false);
        QModelIndex curIdx = ui->show_assay_list->model()->index(index, 1);
        OnAssayListSelChanged(curIdx, curIdx);
    }
}

void QHistory::OnAIVisonResultIdentifyClicked()
{
    qint64 aiRecognizeResultId = -1;
    bool bIsSampleModel = (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE);
    if (bIsSampleModel)
    {
        // 获取样本的测试项目
        int currentRow = m_currentHighLightRow;
        auto index = ui->sample_list->currentIndex();
        if (index.isValid())
        {
            currentRow = index.row();
        }

        auto currentIndex = gHisSampleAssayModel.index(currentRow, 0);
        aiRecognizeResultId = gHisSampleAssayModel.GetAiRecognizeResultId(currentIndex);
    }
    else
    {
        int currentRow = m_currentHighLightRow;
        auto index = ui->show_assay_list->currentIndex();
        if (index.isValid())
        {
            currentRow = index.row();
        }

        auto currentIndex = gHisSampleAssayModel.index(currentRow, 0);
        aiRecognizeResultId = gHisSampleAssayModel.GetAiRecognizeResultId(currentIndex);
    }

    if (m_dialogAi == Q_NULLPTR)
    {
        m_dialogAi = new QDialogAi(this);
    }

    m_dialogAi->SetAiRecognizeId(aiRecognizeResultId);
    m_dialogAi->show();
}

void QHistory::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    bool isChemistry = (m_pCommAssayMgr->GetSoftWareType() == CHEMISTRY);

    // 导出
    (userPms->IsPermisson(PSM_HISTORY_EXPORT) && m_workSet.exported) ? ui->export_btn->show() : ui->export_btn->hide();
}

///
/// @brief 初始化快捷筛选按钮
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月14日，新建函数
///
void QHistory::OnfastFilterFinished(const ::tf::HistoryBaseDataQueryCond& qryCond)
{
    std::vector<::tf::HistoryBaseDataQueryCond> vecQryCond;
    m_filterDialog->GetFastFilterQueryConditon(vecQryCond);
    ui->pushButton_fastfilter_1->hide();
    ui->pushButton_fastfilter_2->hide();
    ui->pushButton_fastfilter_3->hide();
    ui->pushButton_fastfilter_4->hide();
    ui->pushButton_fastfilter_5->hide();
    ui->label->hide();

    int index = 0;
    for (const auto& cond : vecQryCond)
    {
        if (cond.__isset.isUsed && cond.isUsed)
        {
            if (ui->label->isHidden())
            {
                ui->label->show();
            }

            // modify for bug3446 by wuht
            index = cond.filterIndex;
            QPushButton* btn = nullptr;
            switch (index)
            {
            case 1:
                btn = ui->pushButton_fastfilter_1;
                break;
            case 2:
                btn = ui->pushButton_fastfilter_2;
                break;
            case 3:
                btn = ui->pushButton_fastfilter_3;
                break;
            case 4:
                btn = ui->pushButton_fastfilter_4;
                break;
            case 5:
                btn = ui->pushButton_fastfilter_5;
                break;
            default:
                return;
            }
            auto txt = cond.filterName.empty() ? QString(tr("快捷筛选%1")).arg(cond.filterIndex + 1) : QString::fromUtf8(cond.filterName.c_str());
            btn->setProperty("cond", QVariant::fromValue(cond));
            btn->show();
            // 如果文字过长，则添加省略号
            UiCommon::SetButtonTextWithEllipsis(btn, txt);
            btn->setToolTip(txt);
        }
    }
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
bool QHistory::DealSeriesSampIe(int method)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // TODO

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
bool QHistory::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QTableView* targetView = Q_NULLPTR;
        if (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
        {
            targetView = (target == ui->sample_list->viewport()) ? ui->sample_list : (
                target == ui->assay_list->viewport() ? ui->assay_list : Q_NULLPTR);
        }
        else
        {
            targetView = (target == ui->show_assay_list->viewport() ? ui->show_assay_list : Q_NULLPTR);
        }

        // 添加tooltip
        if (targetView != Q_NULLPTR)
        {
            QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
            if (helpEvent != Q_NULLPTR)
            {
                auto index = targetView->indexAt(helpEvent->pos());
                if (index.isValid())
                {
                    auto rawData = targetView->model()->data(index, Qt::DisplayRole);
                    QToolTip::showText(helpEvent->globalPos(), rawData.toString());
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
/// @brief 刷新结果详情页面
///
/// @param[in]  dialog  结果详情的对话框
///
/// @return true,刷新成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
bool QHistory::ShowSampleDetail(QHistoryItemResultDetailDlg* dialog)
{
    if (dialog == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null history item result detail dialog.");
        return false;
    }

    std::vector<std::shared_ptr<HistoryBaseDataItem>> vecDetailTestItem;
    int64_t sampleID;
    bool bIsPatient = false;	// 是否是病人样本(常规、急诊)
    bool bIsSampleModel = (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE);
    if (bIsSampleModel)
    {
        // 获取样本的测试项目
		int currentRow = m_currentHighLightRow;
		auto index = ui->sample_list->currentIndex();
		if (index.isValid())
		{
			currentRow = index.row();
		}

		auto currentIndex = gHisSampleAssayModel.index(currentRow, 0);
        auto sampleType = gHisSampleAssayModel.GetSampleType(currentIndex);
        bIsPatient = (sampleType == ::tf::HisSampleType::SAMPLE_SOURCE_NM || \
            sampleType == ::tf::HisSampleType::SAMPLE_SOURCE_EM) ? true : false;

        auto assays = QHistorySampleModel_Assay::Instance().GetAssay();
        sampleID = QHistorySampleModel_Assay::Instance().GetSampleID();

        for (const auto& data : assays)
        {
            vecDetailTestItem.push_back(data);
        }
    }
    else
    {
		int currentRow = m_currentHighLightRow;
		auto index = ui->show_assay_list->currentIndex();
		if (index.isValid())
		{
			currentRow = index.row();
		}

		auto currentIndex = gHisSampleAssayModel.index(currentRow, 0);
        auto assay = gHisSampleAssayModel.GetAssayItemData(currentIndex);
        auto sampleType = gHisSampleAssayModel.GetSampleType(currentIndex);
        bIsPatient = (sampleType == ::tf::HisSampleType::SAMPLE_SOURCE_NM || \
            sampleType == ::tf::HisSampleType::SAMPLE_SOURCE_EM) ? true : false;

        sampleID = assay->m_sampleId;
        vecDetailTestItem.emplace_back(assay);
    }    
    
    // 上一条、下一条 按钮状态更新
    QTableView* pTableView = bIsSampleModel ? ui->sample_list : ui->show_assay_list;
    int curRow = pTableView->currentIndex().row();
    dialog->SetNextBtnEnable(curRow + 1 < pTableView->model()->rowCount());
    dialog->SetPreBtnEnable(curRow > 0);

    // 样本ID不合法，则直接退出
    if (sampleID < 0)
    {
        ULOG(LOG_WARN, "Invalid sample ID, %lld.", sampleID);
        return false;
    }

	std::vector<std::shared_ptr<HistoryBaseDataItem>> itemDatas;
	for (const auto& data : vecDetailTestItem)
	{
		// 滤掉计算项目
		if (CommonInformationManager::IsCalcAssay(data->m_assayCode))
		{
			continue;
		}

		itemDatas.push_back(data);
	}

    dialog->Update(std::make_tuple(sampleID, itemDatas, bIsPatient));
    return true;
}

///
/// @brief 页面允许功能
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月29日，新建函数
///
void QHistory::PageAllowFun()
{
    ui->patient_btn->setVisible(false);
    ui->flat_recheck->setVisible(false);
    ui->del_Button->setVisible(false);
    ui->flat_del_all->setVisible(false);
    ui->flat_recalculate->setVisible(false);
    ui->flat_examine->setText(m_sCheckBtnText);

    // 过滤掉不属于历史页面的按钮,bug2726 by wuhongtao
    UpdateButtonStatus();
}

void QHistory::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
	POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, tr("> 工作 > ") + tr("历史数据"));

    // 显示时显示筛选条件，隐藏时不显示
    // 发送筛选消息
    if (m_lastFilterObj != nullptr)
    {
        auto qryCond = m_lastFilterObj->property("cond").value<::tf::HistoryBaseDataQueryCond>();
        emit ShowTipMessage(QueryCondToStr(qryCond));
    }

    // 切回历史数据界面时需要回复默认显示，目前的做法是通过数据库排序，在数据量大时可能会卡顿 add by wzx-20231206
    {
        auto header = qobject_cast<SortHeaderView*>(ui->sample_list->horizontalHeader());
        if (header != nullptr)
        {
            header->ResetAllIndex();
            //去掉排序三角样式
            header->setSortIndicator(-1, Qt::DescendingOrder);
        }
    }
    {
        auto header = qobject_cast<SortHeaderView*>(ui->show_assay_list->horizontalHeader());
        if (header != nullptr)
        {
            header->ResetAllIndex();
            //去掉排序三角样式
            header->setSortIndicator(-1, Qt::DescendingOrder);
        }
    }
    gHisSampleAssayModel.Refush();

	// 0028556: [应用] 应用-显示设置取消启用“导出”并保存，数据浏览和历史数据无”导出“按钮，退出软件重新启动，历史数据界面出现”导出“按钮 added by zhang.changjiang-20240603
	OnUpdateButtonStatus();

	// 恢复之前的选中
    QHistorySampleAssayModel::VIEWMOUDLE curModle = gHisSampleAssayModel.GetModule();
    QTableView* tbView = (curModle == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE) ? ui->sample_list : 
        ((curModle == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE) ? ui->show_assay_list : Q_NULLPTR);
    if (tbView != Q_NULLPTR)
    {
        int currentRow = m_currentHighLightRow;
        auto index = tbView->currentIndex();
        if (!index.isValid())
        {
            auto currentIndex = tbView->model()->index(currentRow, 1);
            tbView->setCurrentIndex(currentIndex);
        }
    }
}

void QHistory::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);

    // 记录切换页面前的选中状态
    m_rowSelectedBeforeHide[QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE] = ui->sample_list->currentIndex().row();
    m_rowSelectedBeforeHide[QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE] = ui->show_assay_list->currentIndex().row();
}

void QHistory::keyPressEvent(QKeyEvent *event)
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

void QHistory::keyReleaseEvent(QKeyEvent* event)
{
    if (event == Q_NULLPTR)
    {
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Control:
        m_controlModify = false;
        break;
    case Qt::Key_Shift:
        m_shiftKey = false;
        break;
    default:
        break;
    }

    QWorkShellPage::keyReleaseEvent(event);
}

///
/// @brief 获取单样本的打印数据
///
/// @param[out]  vecSampleDatas  组装好的打印数据
/// @param[in]   QueryCondition   查询条件，类型待定
///
/// @return true 设置正确
///
/// @par History:
/// @li 6889/ChenWei，2023年3月24日，新建函数
///
bool QHistory::GetPrintData(SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)
{
    sampIds.clear();
    if (m_pCommAssayMgr == nullptr)
    {
        TipDlg(tr("创建打印数据失败！")).exec();
        return false;
    }
    auto selectedIndexs = gHisSampleAssayModel.GetCheckedModelIndex();
    if (selectedIndexs.isEmpty())
    {
        return false;
    }
    std::vector<std::shared_ptr<HistoryBaseDataSample>> selectedSamples;
    gHisSampleAssayModel.GetSelectedSamples(selectedIndexs, selectedSamples);

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    for (const auto& sample : selectedSamples)
    {
        // 审核判断
        if (m_workSet.audit && !sample->m_bCheck)
        {
            continue;
        }

        SampleInfo info;
        std::shared_ptr<tf::PatientInfo> ptPatient = nullptr;
        if (sample->m_patientId > 0)
        {
            ::tf::PatientInfo patientInfo = QueryPatientInfoById(sample->m_patientId);
            info.strPatientName = patientInfo.name;                                 // 姓名

			// 性别
			if (patientInfo.gender >= 0)
			{
				info.strGender = ConvertTfEnumToQString(patientInfo.gender).toStdString();
			}
            
            // 年龄
            if (patientInfo.age > 0)
            {
                info.strAge = std::to_string(patientInfo.age);
                if (patientInfo.ageUnit != -1)
                {
                    info.strAge += " " + ConvertTfEnumToQString(patientInfo.ageUnit).toStdString();
                }
            }
            info.strCaseNo = patientInfo.medicalRecordNo;							// 病历号
            info.strDepartment = patientInfo.department;							// 科室
            info.strDoctor = patientInfo.attendingDoctor;							// 医生
            info.strBedNo = patientInfo.bedNo;										// 病床号
            info.strInpatientWard = patientInfo.inpatientArea;						// 病区
            info.strClinicalDiagnosis = patientInfo.diagnosis;						// 临床诊断
            info.strInspectionPersonnel = patientInfo.doctor;						// 检验人员
            info.strReviewers = patientInfo.auditor;								// 审核人员
            ptPatient = std::make_shared<tf::PatientInfo>(patientInfo);
        }
                
        info.strPrintTime = strPrintTime.toStdString();                         // 打印时间       
        info.strSourceType = ThriftEnumTrans::GetSourceTypeName(sample->m_sampleSourceType).toStdString();// 样本类型
        info.strSequenceNO = GetSeqNo((::tf::HisSampleType::type)sample->m_sampleType, (::tf::TestMode::type)sample->m_testMode,sample->m_seqNo).toStdString();		// 样本号
        info.strSampleID = sample->m_barcode;									// 样本ID（样本条码）     
        
        info.strDateAudit = " ";//PosixTimeToTimeString(sample->m_endTestTimeMax);	// 审核日期       
		info.strInstrumentModel = " ";		// 仪器型号
		auto softType = m_pCommAssayMgr->GetSoftWareType();
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

        const auto& devInfos = m_pCommAssayMgr->GetDeviceMaps();
        int iIndex = 0;

        std::vector<std::shared_ptr<HistoryBaseDataItem>> itemsInSelectedSample;
        QHistorySampleModel_Assay::Instance().GetItems(sample->m_sampleId, gHisSampleAssayModel.GetQueryCond(), itemsInSelectedSample);
        for (const auto& it : itemsInSelectedSample)
        {
			if (it->m_sampleId != sample->m_sampleId)
			{
				continue;
			}

            ItemInfo item;
            // 序号
            item.strIndex = QString::number(++iIndex).toStdString();
            // 项目名称
            auto pAssay = m_pCommAssayMgr->GetAssayInfo(it->m_assayCode);
            item.strItemName = (pAssay == nullptr) ? it->m_itemName : pAssay->printName;
            // 参考范围
            if (m_pCommAssayMgr->IsCalcAssay(it->m_assayCode))
            {
                std::shared_ptr<::tf::CalcAssayInfo> calcAssayInfo = m_pCommAssayMgr->GetCalcAssayInfo(it->m_assayCode);
                if (calcAssayInfo != nullptr)
                {
                    item.strItemName = calcAssayInfo->printName;
                }
                int iDecimalDigit = calcAssayInfo == nullptr ? 2 : calcAssayInfo->decimalDigit; // default 2

                // 计算项目参考范围单独获取
                ::tf::AssayReferenceItem matchRef;
                if (m_pCommAssayMgr->GetCalcAssayResultReference(it->m_assayCode, sample->m_sampleSourceType, ptPatient, matchRef))
				{
                    item.strRefRange = (matchRef.fUpperRefRang == DBL_MAX ? "" : QString::number(matchRef.fLowerRefRang, 'f', iDecimalDigit) + "-" +
                    QString::number(matchRef.fUpperRefRang, 'f', iDecimalDigit)).toStdString();
				}
			}
            else
            {
                auto opAssayClassify = QueryAssayClassifyBySn(it->m_deviceSN);
                if (!opAssayClassify.has_value())
                {
                    continue;
                }
                switch (opAssayClassify.value())
                {
                case ::tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE:
                {
                    QString strRefRange = QString::fromStdString(QueryRefRange(it->m_itemId));
                    QStringList RefRangeList = strRefRange.split(QRegExp(";|；"));
                    strRefRange = RefRangeList.join(";\n");
                    item.strRefRange = strRefRange.toStdString();				// 参考区间
                }
                    
                    break;
                case ::tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY:
                    // TODO
                    break;
                case ::tf::AssayClassify::type::ASSAY_CLASSIFY_ISE:
                    // TODO
                    break;
                }

                // 此处不能使用assayName进行项目查找，因为t_historybasedata中记录的itemName为简称还没有跟随简称的修改而修改
                auto assayInfo = m_pCommAssayMgr->GetAssayInfo(it->m_assayCode);
                if (assayInfo != nullptr)
                {
                    item.strItemName = assayInfo->printName;
                    item.strShortName = assayInfo->assayName;           // 简称
                }
            }

            stAssayResult firstResult, retestResult;
            QHistorySampleAssayModel::GetAssayResult(*it, false, firstResult);
            QHistorySampleAssayModel::GetAssayResult(*it, true, retestResult);

            std::shared_ptr<HistoryBaseDataByItem> pBaseItem = gHisSampleAssayModel.GetItemInSelectedSample(sample->m_sampleId, it->m_itemId);
            if (pBaseItem != nullptr)
            {
                item.strUnit = QHistorySampleAssayModel::GetResultUnit(*pBaseItem).toStdString();	// 结果单位
                item.strResultState = QHistorySampleAssayModel::GetDisplayStatuCode(*pBaseItem, pBaseItem->m_retest).toStdString();
            }
			if (!it->m_retest)// 首查
			{
                info.strCompTime = PosixTimeToTimeString(sample->m_endTestTimeMax);		// 检测完成时间
				item.strResult = firstResult.m_result.toStdString();				// 用于打印的结果
			}
			else // 复查
			{
                info.strCompTime = PosixTimeToTimeString(sample->m_reTestEndTimeMax);		// 检测完成时间
				item.strResult = retestResult.m_result.toStdString();							// 用于打印的结果
			}
            
            item.strModule = devInfos.count(it->m_deviceSN) > 0 ? devInfos.at(it->m_deviceSN)->name : tr("-").toStdString();// 检测模块
            info.vecItems.push_back(item);
        }

        vecSampleDatas.push_back(info);
        sampIds.push_back(sample->m_sampleId);
    }

    return true;
}

///
/// @brief 获取按样本打印的打印数据
///
/// @param[out]  vecSampleDatas  组装好的打印数据
/// @param[in]   QueryCondition   查询条件，类型待定
///
/// @return true 设置正确
///
/// @par History:
/// @li 6889/ChenWei，2023年3月24日，新建函数
///
bool QHistory::GetPrintDataBySample(SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)
{
    sampIds.clear();
    if (m_pCommAssayMgr == nullptr)
    {
        TipDlg(tr("创建打印数据失败！")).exec();
        return false;
    }

    const auto& devInfos = m_pCommAssayMgr->GetDeviceMaps();
    auto selectedIndexs = gHisSampleAssayModel.GetCheckedModelIndex();
    if (selectedIndexs.isEmpty())
    {
        ULOG(LOG_WARN, "Empty selected indexes.");
        return false;
    }
    std::vector<std::shared_ptr<HistoryBaseDataSample>> selectedSamples;
    gHisSampleAssayModel.GetSelectedSamples(selectedIndexs, selectedSamples);
    std::vector<std::shared_ptr<HistoryBaseDataByItem>> itemsInSelectedSample;
    gHisSampleAssayModel.GetItemsInSelectedSamples(selectedSamples, itemsInSelectedSample);

    SimpleSampleInfo info;
    info.strInstrumentModel = "i 6000";
	auto softType = m_pCommAssayMgr->GetSoftWareType();
	if (SOFTWARE_TYPE::IMMUNE == softType)
	{
		info.strInstrumentModel = "i 6000";
	}
	else if (SOFTWARE_TYPE::CHEMISTRY == softType)
	{
		info.strInstrumentModel = "C1005";
	}
	else
	{
		info.strInstrumentModel = "";
	}

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    info.strPrintTime = strPrintTime.toStdString();
    for (const auto& sample : selectedSamples)
    {
        // 审核判断
        if (m_workSet.audit && !sample->m_bCheck)
        {
            continue;
        }

        std::vector<ItemResult> vecItemResults;
        ItemResult item;
        
        item.strSequenceNO = GetSeqNo((::tf::HisSampleType::type)sample->m_sampleType, (::tf::TestMode::type)sample->m_testMode, sample->m_seqNo).toStdString();								// 样本号
        item.strSampleID = sample->m_barcode;								// 样本ID（样本条码）

        for (const auto& it : itemsInSelectedSample)
        {
            if (it->m_sampleId != sample->m_sampleId)
            {
                continue;
            }

            item.strItemName = it->m_itemName;                              // 项目名称
            if (m_pCommAssayMgr->IsCalcAssay(it->m_assayCode))
            {
                auto calcAssayInfo = m_pCommAssayMgr->GetCalcAssayInfo(it->m_assayCode);
                if (calcAssayInfo != nullptr)
                {
                    item.strItemName = calcAssayInfo->printName;
                }
            }
            else
            {
                auto assayInfo = m_pCommAssayMgr->GetAssayInfo(it->m_assayCode);
                if (assayInfo != nullptr)
                {
                    item.strItemName = assayInfo->printName;
                }
            }

            stAssayResult firstResult, retestResult;
            QHistorySampleAssayModel::GetAssayResult(*it, false, firstResult);
            QHistorySampleAssayModel::GetAssayResult(*it, true, retestResult);
			if (!it->m_retest)// 首查
			{
                item.strCompTime = PosixTimeToTimeString(sample->m_endTestTimeMax);	// 检测时间
				item.strResult = firstResult.m_result.toStdString();				// 用于打印的结果
			}
			else // 复查
			{
                item.strCompTime = PosixTimeToTimeString(sample->m_reTestEndTimeMax);	// 检测时间
				item.strResult = retestResult.m_result.toStdString();							// 用于打印的结果
			}

            item.strModule = devInfos.count(it->m_deviceSN) > 0 ? devInfos.at(it->m_deviceSN)->name : tr("-").toStdString();// 检测模块
            info.vecItemResults.push_back(std::move(item));
        }

        // 没结果时特殊处理
        if(itemsInSelectedSample.size() == 0)
            info.vecItemResults.push_back(std::move(item));

        sampIds.push_back(sample->m_sampleId);
    }

    vecSampleDatas.push_back(info);
    return true;
}

///
/// @brief 获取按项目打印的打印数据
///
/// @param[out]  vecSampleDatas  组装好的打印数据
/// @param[in]   QueryCondition   查询条件，类型待定
/// @param[in]   assayCode        要打印项目的编号，后续可根据业务需求换成其他标识
///
/// @return true 设置正确
///
/// @par History:
/// @li 6889/ChenWei，2023年3月24日，新建函数
///
bool QHistory::GetPrintItemData(ItemSampleInfoVector& vecItemDatas, EM_RESULT_MODE ResultMode)
{
    ULOG(LOG_INFO, "%s, dataSize:%d, resultModel:%d.",  __FUNCTION__, vecItemDatas.size(), ResultMode);

    if (m_pCommAssayMgr == nullptr)
    {
        TipDlg(tr("创建打印数据失败！")).exec();
        return false;
    }

    const auto& devInfos = m_pCommAssayMgr->GetDeviceMaps();
    auto selectedIndexs = gHisSampleAssayModel.GetCheckedModelIndex();
    if (selectedIndexs.isEmpty())
    {
        ULOG(LOG_WARN, "Empty selected indexes.");
        return false;
    }
    std::vector<std::shared_ptr<HistoryBaseDataByItem>> selectedItems = gHisSampleAssayModel.GetSelectedItems(selectedIndexs);

    ItemSampleInfo info;
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");    // 打印时间
    info.strPrintTime = strPrintTime.toStdString();
    info.strFactory = std::string(tr("maccura").toStdString());								// 试剂厂家
    info.strInstrumentModel = " ";						// 仪器型号
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

    info.strTemperature = std::string(tr("    - ℃").toStdString());								// 实验室温度
    info.strHumidness = std::string(tr("    - %rh").toStdString());								// 实验室湿度
    for (const auto& it : selectedItems)
    {
        // 审核判断
        if (m_workSet.audit && !it->m_bCheck)
        {
            continue;
        }

		// 将查询到的项目数据组装成打印数据
        ItemSampleResult item;

        // 过滤掉没有对应结果的情况
        if (!(it->m_retest) && (ResultMode == EM_RESULT_MODE::PRINT_RESULT_MODE_RERUN))
        {
            continue;
        }

        int curTestType = gHisSampleAssayModel.GetHistorySample(it->m_sampleId);
        item.strSampleNumber = GetSeqNo((::tf::HisSampleType::type)it->m_sampleType, (::tf::TestMode::type)curTestType, it->m_seqNo).toStdString();									// 样本号
        item.strSampleID = it->m_barcode; // 样本ID（样本条码）

        // 项目名称
        auto pAssay = m_pCommAssayMgr->GetAssayInfo(it->m_assayCode);
        item.strItemName = (pAssay == nullptr) ? it->m_itemName : pAssay->printName;

        // 查询设备类型
        std::string strFRLU;
        std::string strRRLU;
        auto opAssayClassify = QueryAssayClassifyBySn(it->m_deviceSN);
        if (opAssayClassify.has_value())
        {
            // 根据设备类型填写打印数据
            switch (opAssayClassify.value())
            {
            case ::tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE:
            {
                strRRLU = it->m_reTestRLU; // 复查, 信号值
                strFRLU = it->m_RLU; // 首查, 信号值
            }
            break;
            case ::tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY:
            {
                ch::tf::AssayTestResultQueryCond queryAssaycond;
                ch::tf::AssayTestResultQueryResp assayTestResult;
                // 根据测试项目的ID来查询结果
                queryAssaycond.__set_testItemId(it->m_itemId);

                if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                    || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                    || assayTestResult.lstAssayTestResult.empty())
                {
                    ULOG(LOG_ERROR, "Failed to query assaytest result by itemid:%lld.", it->m_itemId);
                    continue;
                }
                item.strRLU = tr("-").toStdString();												// 信号值
            }
            break;
            case ::tf::AssayClassify::type::ASSAY_CLASSIFY_ISE:
            {
                ise::tf::AssayTestResultQueryCond queryAssaycond;
                ise::tf::AssayTestResultQueryResp assayTestResult;
                // 根据测试项目的ID来查询结果
                queryAssaycond.__set_testItemId(it->m_itemId);

                if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
                    || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
                    || assayTestResult.lstAssayTestResult.empty())
                {
                    ULOG(LOG_ERROR, "Failed to query assaytest result by itemid:%lld.", it->m_itemId);
                    continue;
                }
                item.strRLU = tr("-").toStdString();												// 信号值
            }
            break;
            }
        }
        else if (m_pCommAssayMgr->IsCalcAssay(it->m_assayCode))
        {
            auto calcAssayInfo = m_pCommAssayMgr->GetCalcAssayInfo(it->m_assayCode);
            if (calcAssayInfo != nullptr)
            {
                item.strItemName = calcAssayInfo->printName;
            }
        }
        else
        {
            continue;
        }

        item.strUnit = QHistorySampleAssayModel::GetResultUnit(*it).toStdString();											// 单位
		//item.strModule = devInfos.count(it->m_deviceSN) > 0 ? devInfos.at(it->m_deviceSN)->name : tr("-").toStdString();// 检测模块

        if ((ResultMode & EM_RESULT_MODE::PRINT_RESULT_MODE_FIRST))// 首查
        {
            stAssayResult firstResult;
            QHistorySampleAssayModel::GetAssayResult(*it, false, firstResult);
            item.strResult = firstResult.m_result.toStdString();                  // 用于打印的结果
            item.strDetectionTime = PosixTimeToTimeString(it->m_endTestTime);   // 检测时间
            item.strRLU = strFRLU;
            info.vecResult.push_back(item);
        }

        if ((it->m_retest) && (ResultMode & EM_RESULT_MODE::PRINT_RESULT_MODE_RERUN))
        {
            stAssayResult retestResult;
            QHistorySampleAssayModel::GetAssayResult(*it, true, retestResult);
            ItemSampleResult RetestItem = item;
            RetestItem.strResult = retestResult.m_result.toStdString() + std::string(" R");                     // 用于打印的结果
            RetestItem.strDetectionTime = PosixTimeToTimeString(it->m_reTestEndTimeMax);      // 检测时间
            RetestItem.strRLU = strRRLU;
            info.vecResult.push_back(RetestItem);
        }
    }

    vecItemDatas.push_back(info);

    return true;
}

///
///  @brief 查询病人信息
///
///
///  @param[in]   patientId  病人信息id
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月24日，新建函数
///
::tf::PatientInfo QHistory::QueryPatientInfoById(int64_t patientId)
{
    ::tf::PatientInfoQueryResp _return;
    ::tf::PatientInfoQueryCond piqc;
    piqc.__set_id(patientId);
    DcsControlProxy::GetInstance()->QueryPatientInfo(_return, piqc);
    if (_return.result == ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS && _return.lstPatientInfos.size() > 0)
    {
        return std::move(_return.lstPatientInfos.at(0));
    }
    return std::move(::tf::PatientInfo());
}

///
///  @brief 根据设备序列号查询设备类型
///
///
///  @param[in]   deviceSN  设备序列号
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
///
boost::optional<::tf::AssayClassify::type> QHistory::QueryAssayClassifyBySn(const std::string& deviceSN)
{
    ::tf::DeviceInfoQueryResp _deviceReturn;
    ::tf::DeviceInfoQueryCond deviceDiqc;
    deviceDiqc.__set_deviceSN(deviceSN);
    DcsControlProxy::GetInstance()->QueryDeviceInfo(_deviceReturn, deviceDiqc);
    // 查询设备失败时退出
    if (_deviceReturn.result != ::tf::ThriftResult::type::THRIFT_RESULT_SUCCESS || _deviceReturn.lstDeviceInfos.size() != 1)
    {
        return boost::none;
    }
    if (_deviceReturn.lstDeviceInfos.empty())
    {
        return boost::none;
    }
    return _deviceReturn.lstDeviceInfos[0].deviceClassify;
}

///
/// @brief  参考范围
///
/// @param[in]  resultid  结果id
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年8月9日，新建函数
///
std::string QHistory::QueryRefRange(int64_t resultid)
{
    im::tf::AssayTestResultQueryCond qryCond;
    qryCond.__set_testItemId(resultid);
    im::tf::AssayTestResultQueryResp resp;
    if (im::i6000::LogicControlProxy::QueryAssayTestResult(resp, qryCond))
    {
        for (int i = resp.lstAssayTestResult.size() - 1; i >= 0; --i)
        {
            if (resp.lstAssayTestResult[i].referenceStr.empty())
            {
                continue;
            }
            return resp.lstAssayTestResult[i].referenceStr;
        }
    }

    return "";
}

///
/// @brief 取消快捷筛选按钮的check状态
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月16日，新建函数
///
void QHistory::UnCheckedFastFilterBtn()
{
	QList<QPushButton*> listBtns = ui->widget_2->findChildren<QPushButton*>();
	// 取消选中
	for (const auto& atBtn : listBtns)
	{
		if (nullptr == atBtn)
		{
			continue;
		}
		bool bAutoExcl = atBtn->autoExclusive();
		atBtn->setAutoExclusive(false);
		atBtn->setChecked(false);
		atBtn->clearFocus();
		atBtn->setAutoExclusive(bAutoExcl);
	}
}

///
/// @brief 根据筛选条件取消快捷筛选按钮的check状态
///
/// @param[in]  cond  筛选条件
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2024年1月30日，新建函数
///
void QHistory::UnCheckedFastFilterBtn(const ::tf::HistoryBaseDataQueryCond& cond)
{
    QList<QPushButton*> listBtns = ui->widget_2->findChildren<QPushButton*>();
    // 取消选中
    for (const auto& atBtn : listBtns)
    {
        if (nullptr == atBtn)
        {
            continue;
        }
		// 把当前条件的按钮设置为按下状态
		if (atBtn->property("cond").value<::tf::HistoryBaseDataQueryCond>() == cond)
		{
			atBtn->setChecked(true);
			m_lastFilterObj = atBtn;
			continue;
		}

        bool bAutoExcl = atBtn->autoExclusive();
        atBtn->setAutoExclusive(false);
        atBtn->setChecked(false);
        atBtn->clearFocus();
        atBtn->setAutoExclusive(bAutoExcl);
    }
}

///
/// @brief 查询条件转换为字符串
///
/// @param[in]    cond
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月23日，新建函数
///
QString QHistory::QueryCondToStr(const ::tf::HistoryBaseDataQueryCond& cond)
{
    int conditionCnt = 0;
    QString str;
	QString endstr = "\n";
	QString split = "/";
    // 检测日期
    if ((cond.__isset.reTestStartTime || cond.__isset.startTime) && (cond.__isset.reTestEndtime || cond.__isset.endtime))
    {
        str += tr("检测日期：");
        QString strStart = ToCfgFmtDate(QString::fromStdString(cond.__isset.reTestStartTime ? cond.reTestStartTime : cond.startTime));
        QString strStop = ToCfgFmtDate(QString::fromStdString(cond.__isset.reTestEndtime ? cond.reTestEndtime : cond.endtime));
        str += (strStart + "-" + strStop + endstr);
        conditionCnt++;
    }
    // 样本号
    if (cond.__isset.seqNoList && cond.seqNoList.size() == 2)
    {
        str += tr("样本号：");
        if (!cond.seqNoList[0].empty() && !cond.seqNoList[1].empty())
        {
            str += QString::fromStdString(cond.seqNoList[0]) + "-" + QString::fromStdString(cond.seqNoList[1]) + endstr;
            conditionCnt++;
        }
        else if (cond.seqNoList[0].empty() && cond.seqNoList[1].empty()) {}
        else
        {
            auto seq = !cond.seqNoList[0].empty() ? cond.seqNoList[0] : cond.seqNoList[1];
            str += QString::fromStdString(seq) + endstr;
            conditionCnt++;
        }
    }
    // 样本条码
    if (cond.__isset.sampleBarcodeList && cond.sampleBarcodeList.size() == 2)
    {
        if (!cond.sampleBarcodeList[0].empty() && !cond.sampleBarcodeList[1].empty())
        {
            str += tr("样本条码：") + QString::fromStdString(cond.sampleBarcodeList[0]) + "-" + QString::fromStdString(cond.sampleBarcodeList[1]) + endstr;
            conditionCnt++;
        }
        else if (cond.sampleBarcodeList[0].empty() && cond.sampleBarcodeList[1].empty()) {}
        else
        {
            auto barCode = !cond.sampleBarcodeList[0].empty() ? cond.sampleBarcodeList[0] : cond.sampleBarcodeList[1];
            str += tr("样本条码：") + QString::fromStdString(barCode) + endstr;
            conditionCnt++;
        }
    }
    // 模块
    if (cond.__isset.moduleList && cond.moduleList.size() >= 1)
    {
        auto deviceInfo = m_pCommAssayMgr->GetDeviceInfo(cond.moduleList[0]);
		std::string deviceType;
		if (deviceInfo != Q_NULLPTR)
		{
			QString strAssayClassfyName = ThriftEnumTrans::GetAssayClassfiyName(deviceInfo->deviceClassify);
			if (!strAssayClassfyName.isEmpty())
			{
				deviceType = strAssayClassfyName.toStdString();
			}

			auto name = deviceInfo != nullptr ? (deviceInfo->groupName + deviceInfo->name) : cond.moduleList[0];
			name = deviceType + "-" + name;
			str += tr("模块：") + QString::fromStdString(name) + endstr;
		}
        conditionCnt++;
    }
    if (cond.__isset.caseNo)
    {
        str += tr("患者ID：") + QString::fromStdString(cond.caseNo) + endstr;
        conditionCnt++;
    }
    if (cond.__isset.patientName)
    {
        str += tr("患者姓名：") + QString::fromStdString(cond.patientName) + endstr;
        conditionCnt++;
    }
    if (cond.__isset.reagentLot)
    {
        str += tr("试剂批号：") + QString::fromStdString(cond.reagentLot) + endstr;
        conditionCnt++;
    }
    if (cond.__isset.assayCodeList && cond.assayCodeList.size() >= 1)
    {
        QString strCode;
        for (auto assayCode : cond.assayCodeList)
        {
			bool isCalc = m_pCommAssayMgr->IsCalcAssay(assayCode);
			if (isCalc)
			{
				auto spAssayInfo = m_pCommAssayMgr->GetCalcAssayInfo(assayCode);
				if (spAssayInfo == nullptr)
				{
					continue;
				}

				strCode += (QString::fromStdString(spAssayInfo->name) + "、");
				continue;
			}

            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = m_pCommAssayMgr->GetAssayInfo(assayCode);
            if (spAssayInfo == nullptr)
            {
                continue;
            }
            strCode += (QString::fromStdString(spAssayInfo->assayName) + split);
        }
        strCode.chop(1);

        str += tr("项目名：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.sampleTypeList)
    {
        QString strCode;
        for (auto type : cond.sampleTypeList)
        {
            switch ((::tf::HisSampleType::type)type)
            {
            case ::tf::HisSampleType::type::SAMPLE_SOURCE_NM:
                strCode += (tr("常规") + split);
                break;
            case ::tf::HisSampleType::type::SAMPLE_SOURCE_EM:
                strCode += (tr("急诊") + split);
                break;
            case ::tf::HisSampleType::type::SAMPLE_SOURCE_CL:
                strCode += (tr("校准") + split);
                break;
            case ::tf::HisSampleType::type::SAMPLE_SOURCE_QC:
                strCode += (tr("质控") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("订单类型：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.sendLISList)
    {
        QString strCode;
        for (auto type : cond.sendLISList)
        {
            switch (type)
            {
            case 1:
                strCode += (tr("已传输") + split);
                break;
            case 0:
                strCode += (tr("未传输") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("传输：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.checkList)
    {
        QString strCode;
        for (auto type : cond.checkList)
        {
            switch (type)
            {
            case 1:
                strCode += (tr("已审核") + split);
                break;
            case 0:
                strCode += (tr("未审核") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("审核：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.printList)
    {
        QString strCode;
        for (auto type : cond.printList)
        {
            switch (type)
            {
            case 1:
                strCode += (tr("已打印") + split);
                break;
            case 0:
                strCode += (tr("未打印") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("打印：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.retestList)
    {
        QString strCode;
        for (auto type : cond.retestList)
        {
            switch (type)
            {
            case 1:
                strCode += (tr("有复查") + split);
                break;
            case 0:
                strCode += (tr("无复查") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("复查：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.diluteStatuList)
    {
        QString strCode;
        for (auto type : cond.diluteStatuList)
        {
            switch ((::tf::HisDataDiluteStatu::type)type)
            {
            case ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_NONE:
                strCode += (tr("无") + split);
                break;
            case ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_AUTO:
                strCode += (tr("机内稀释") + split);
                break;
			case ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_INCREASE:
				strCode += (tr("机内增量") + split);
				break;
            case ::tf::HisDataDiluteStatu::type::DILUTE_STATU_MANUAL:
                strCode += (tr("手工稀释") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("稀释状态：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.sampleSourceTypeList)
    {
        QString strCode;
        for (auto type : cond.sampleSourceTypeList)
        {
            switch ((::tf::SampleSourceType::type)type)
            {
            case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_XQXJ:
                strCode += (tr("血清/血浆") + split);
                break;
            case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_QX:
                strCode += (tr("全血") + split);
                break;
            case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NY:
                strCode += (tr("尿液") + split);
                break;
            case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_JMQJY:
                strCode += (tr("浆膜腔积液") + split);
                break;
            case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NJY:
                strCode += (tr("脑脊液") + split);
                break;
            case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_OTHER:
                strCode += (tr("其他") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("样本类型：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.warnList)
    {
        QString strCode;
        for (auto type : cond.warnList)
        {
            switch (type)
            {
            case 1:
                strCode += (tr("有报警") + split);
                break;
            case 0:
                strCode += (tr("无报警") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("数据报警：") + strCode + endstr;
        conditionCnt++;
    }
    if (cond.__isset.qualitativeResultList)
    {
        QString strCode;
        for (auto type : cond.qualitativeResultList)
        {
            switch (type)
            {
            case ::tf::QualJudge::Positive:
                strCode += (tr("阳性") + split);
                break;
            case ::tf::QualJudge::Negative:
                strCode += (tr("阴性") + split);
                break;
            default:
                break;
            }
        }
        strCode.chop(1);
        str += tr("定性结果：") + strCode + endstr;
        conditionCnt++;
    }

    // 组装显示字符串
    QString strTip = QString(tr("筛选：%1个筛选条件|")).arg(conditionCnt) + str;

    return strTip;
}

///
/// @brief  响应打印按钮
///
/// @return 
///
/// @par History:
/// @li 6889/ChenWei，2023年3月23日，新建函数
///
void QHistory::OnPrintBtnClicked()
{
    // 构造数据的查询条件
    PrintSetDlg::PrintMode mode = PrintSetDlg::UNKNOMN;
    if (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
        mode = PrintSetDlg::PRINTBYSAMPLE;
    else if (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
        mode = PrintSetDlg::PRINTBYITEM;

    if (m_pPrintSetDlg == nullptr)
    {
        m_pPrintSetDlg = std::shared_ptr<PrintSetDlg>(new PrintSetDlg(true, this));
    }
    m_pPrintSetDlg->SetModel(mode);
    // 设置数据获取接口
    m_pPrintSetDlg->SetSampleGetFun([&](SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)->bool {
        return GetPrintData(vecSampleDatas, sampIds);
    });

    m_pPrintSetDlg->SetSimpleSampleGetFun([&](SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)->bool {
        return GetPrintDataBySample(vecSampleDatas, sampIds);
    });

    m_pPrintSetDlg->SetItemDataGetFun([&](ItemSampleInfoVector& vecSampleDatas)->bool {
        EM_RESULT_MODE ResultMode = (EM_RESULT_MODE)m_pPrintSetDlg->GetResultType(); // 结果类型
        return GetPrintItemData(vecSampleDatas, ResultMode);
    });

    m_pPrintSetDlg->SetFuncPrinted([&](const std::vector<int64_t>& vecSampleIds) {
        DcsControlProxy::GetInstance()->UpdateHistoryBaseDataPrint(vecSampleIds, true);

		for (const auto& sampleid : vecSampleIds)
		{
			tf::SampleInfo modifySample;
			modifySample.__set_id(sampleid);
			modifySample.__set_printed(true);
			// 修改样本为已打印状态
			DcsControlProxy::GetInstance()->ModifySampleInfo(modifySample);
		}

        gHisSampleAssayModel.UpdatePrintFlag(vecSampleIds, true);
    });

    if (m_pPrintSetDlg->exec() == QDialog::Rejected || !m_pPrintSetDlg->IsPrintFinished())
    {
        return;
    }
}

///
/// @brief
///     数据导出按钮被选中
///
/// @par History:
/// @li 5774/WuHongTao，2022年4月6日，新建函数
///
void QHistory::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    switch (gHisSampleAssayModel.GetModule())
    {
    case QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE:
        ExportSample();
        break;
    case QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE:
        ExPortItem();
        break;
    default:
        break;
    }
}

std::vector<ch::tf::AssayTestResult> QHistory::GetChAssayTestResult(int64_t itemId)
{
    if (itemId < 0)
        return vector<ch::tf::AssayTestResult>();

    ch::tf::AssayTestResultQueryCond queryAssaycond;
    ch::tf::AssayTestResultQueryResp assayTestResult;
    // 根据测试项目的ID来查询结果
    queryAssaycond.__set_testItemId(itemId);

    if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
    }
    else
        return assayTestResult.lstAssayTestResult;

    return vector<ch::tf::AssayTestResult>();
}

std::vector<ise::tf::AssayTestResult> QHistory::GetIseAssayTestResult(int64_t itemId)
{
    if (itemId < 0)
        return vector<ise::tf::AssayTestResult>();

    ise::tf::AssayTestResultQueryCond queryAssaycond;
    ise::tf::AssayTestResultQueryResp assayTestResult;
    // 根据测试项目的ID来查询结果
    queryAssaycond.__set_testItemId(itemId);

    if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        ULOG(LOG_ERROR, "%s()", __FUNCTION__);
    }
    else
        return assayTestResult.lstAssayTestResult;

    return vector<ise::tf::AssayTestResult>();
}

std::shared_ptr<tf::PatientInfo> QHistory::GetPatientInfoById(int64_t id)
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
bool QHistory::ExportSample()
{
	std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (pLoginUserInfo == nullptr)
	{
		ULOG(LOG_ERROR, "Can't get UserInfo.");
		return false;
	}

	if (pLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)   // 普通用户不能导出
	{
        ULOG(LOG_INFO, "Operator user not allowed to export.");
		return false;
	}

	auto softwareType = m_pCommAssayMgr->GetSoftWareType();
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

		// 在线程里面依次导出生化和免疫的信息
		auto exportAll = [this](QString fileName, int type)->void
		{
			// 生化项目导出
			ExportSampleData(FileExporter::GetFileName(true, fileName));
			// 免疫项目导出
			{
				// 获取选中的索引
				auto selectedIndexs = ui->sample_list->selectionModel()->selectedRows();
				if (selectedIndexs.isEmpty())
				{
					TipDlg(tr("未选择样本数据！")).exec();
					return;
				}

				std::vector<std::shared_ptr<HistoryBaseDataSample>> selectedSamples;
				gHisSampleAssayModel.GetSelectedSamples(selectedIndexs, selectedSamples);
				if (selectedSamples.empty())
				{
					ULOG(LOG_WARN, "Empty selected samples.");
					return;
				}
				ExportData_im(FileExporter::GetFileName(false, fileName), tf::UserType::type(type), selectedSamples);
			}
		};

		// 耗时操作放在线程里完成
		std::thread thExport(exportAll, fileName, pLoginUserInfo->type);
		thExport.detach();
    }
	// 单独免疫
	else if (softwareType == SOFTWARE_TYPE::IMMUNE)
	{
		return ExportSample_im();
	}
	// 单独生化
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
            ULOG(LOG_WARN, "Null file name.");
			return false;
		}

		// 生化项目导出
		std::thread th(std::bind(&QHistory::ExportSampleData, this, fileName));
		th.detach();
	}
	return true;
}

bool QHistory::ExportSample_im()
{
    std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if (pLoginUserInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can't get UserInfo.");
        return false;
    }

    if (pLoginUserInfo->type < tf::UserType::USER_TYPE_ADMIN)   // 普通用户不能导出
    {
        ULOG(LOG_INFO, "Operator user are not allowed to export.");
        return false;
    }

    // 弹出保存文件对话框
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (!fileName.isNull())
    {
        if (fileName.isEmpty())
        {
            TipDlg(tr("文件名为空！")).exec();
            return false;
        }
    }
    else
    {
        ULOG(LOG_WARN, "Null file name.");
        return false;
    }

    // 获取选中的索引
    auto selectedIndexs = ui->sample_list->selectionModel()->selectedRows();
    if (selectedIndexs.isEmpty())
    {
        TipDlg(tr("未选择样本数据！")).exec();
        return false;
    }

    // 获取选中的样本数据
    std::vector<std::shared_ptr<HistoryBaseDataSample>> selectedSamples;
    gHisSampleAssayModel.GetSelectedSamples(selectedIndexs, selectedSamples);

    // 此处不能使用gHisSampleAssayModel.GetItemsInSelectedSamples来获取，因为获取不全。
    // 应与界面保持一致，而界面来源于QHistorySampleModel_Assay,其不进行时间筛选，且需要重新查询，所以被注释掉。
#if 0
    // 获取选中的样数据包含的项目数据
    std::vector<std::shared_ptr<HistoryBaseDataByItem>> itemsInSelectedSample;
    gHisSampleAssayModel.GetItemsInSelectedSamples(selectedSamples, itemsInSelectedSample);
#else
    const ::tf::HistoryBaseDataQueryCond& cond = gHisSampleAssayModel.GetQueryCond();
    std::vector<std::shared_ptr<HistoryBaseDataByItem>> itemsInSelectedSample;
    for (const auto& sp : selectedSamples)
    {
        std::vector<std::shared_ptr<HistoryBaseDataByItem>> itemForSamples;
        QHistorySampleAssayModel::GetHistoryDataBySampleId(sp->m_sampleId, cond, itemForSamples);
        if (!itemForSamples.empty())
        {
            itemsInSelectedSample.insert(itemsInSelectedSample.end(), itemForSamples.begin(), itemForSamples.end());
        }
    }
#endif

    if (selectedSamples.empty())
    {
        ULOG(LOG_INFO, "Empty selected samples.");
        return false;
    }

    // 耗时操作放在线程里完成
    std::thread th(std::bind(&QHistory::ExportData_im, this, fileName, pLoginUserInfo->type, selectedSamples));
    th.detach();

    return true;
}

bool QHistory::ExportData_im(QString fileName, tf::UserType::type UType, std::vector<std::shared_ptr<HistoryBaseDataSample>>& vecBaseDataSample)
{
    ULOG(LOG_ERROR, "%s()", __FUNCTION__);
    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);

    // 导出文本列表
    ExpSampleInfoVector vecSamples;

    // 写入数据元组
    for (const std::shared_ptr<HistoryBaseDataSample>& sample : vecBaseDataSample)
    {
        if (sample == nullptr)
            continue;

        SampleExportInfo SampleInfo;

        // 样本号
        SampleInfo.strSampleNo = GetSeqNo((::tf::HisSampleType::type)sample->m_sampleType, 
		(::tf::TestMode::type)sample->m_testMode, sample->m_seqNo);

        // 样本条码
        SampleInfo.strBarcode = QString::fromStdString(sample->m_barcode);

        // 样本类型
        SampleInfo.strSampleSourceType = ConvertTfEnumToQString((::tf::SampleSourceType::type)sample->m_sampleSourceType);

        // 复查完成时间
        // 如果复查结束时间和首查时间一致，则没有做复查
        // 1970-1-1 0:0:0为未设值的时间，这里处理一下显示
        bool bReTest = true;
        if ((sample->m_reTestEndTimeMax == sample->m_endTestTimeMax)
            || (sample->m_reTestEndTimeMax == boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1), boost::posix_time::time_duration(0, 0, 0))))
        {
            bReTest = false;
        }

        // 获取选中的样数据包含的项目数据
        std::vector<std::shared_ptr<HistoryBaseDataItem>> itemsInSelectedSample;
        QHistorySampleModel_Assay::Instance().GetItems(sample->m_sampleId, gHisSampleAssayModel.GetQueryCond(), itemsInSelectedSample);

        QMap<int, int> mapItemCounts;           // 重复次数
        QMap<int, int> mapReviewItemCounts;     // 复查重复次数
        for (auto it = itemsInSelectedSample.begin(); it != itemsInSelectedSample.end(); ++it)
        {
            if ((*it)->m_sampleId != sample->m_sampleId)
            {
                continue;
            }

            bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay((*it)->m_assayCode);
			// 是否含有免疫项目，对于联机版免疫页面导出，当含有免疫项目的时候，才导出对应的计算项目
			if (isCalc)
			{
				bool isHaveIm = CommonInformationManager::GetInstance()->IsCaClHaveChorIM((*it)->m_assayCode, false);
				if (!isHaveIm)
				{
					continue;
				}
			}

			auto spGenneral = CommonInformationManager::GetInstance()->GetAssayInfo((*it)->m_assayCode);
			if (spGenneral == nullptr && !isCalc)
			{
				continue;
			}

			// 只导出免疫项目(0013739)
			if (!(isCalc || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
			{
				continue;
			}
            std::shared_ptr<HistoryBaseDataByItem> pBaseItem = gHisSampleAssayModel.GetItemInSelectedSample(sample->m_sampleId, (*it)->m_itemId);
            if (pBaseItem == nullptr)
            {
                continue;
            }

            // 获取测试项目的首次和复查结果
            std::shared_ptr<::im::tf::AssayTestResult> pFirstResult = nullptr;
            std::shared_ptr<::im::tf::AssayTestResult> pReTestResult = nullptr;
            GetImAssayTestResult(sample->m_sampleId, pBaseItem->m_itemId, pBaseItem->m_reTestEndTimeMax, pFirstResult, pReTestResult);

            // 统计项目重复次数
            auto item = mapItemCounts.find((*it)->m_assayCode);
            if (item == mapItemCounts.end())
                mapItemCounts[(*it)->m_assayCode] = 1;
            else
                item.value() += 1;

            // 统计项目重复次数
            if (bReTest && pReTestResult != nullptr)
            {
                auto LastTestItem = mapReviewItemCounts.find(pReTestResult->assayCode);
                if (LastTestItem == mapReviewItemCounts.end())
                    mapReviewItemCounts[pReTestResult->assayCode] = 1;
                else
                    LastTestItem.value() += 1;
            }

            MakeExportItemResult(true, SampleInfo, sample, pBaseItem, pFirstResult, pReTestResult);

            // 项目重复次数
            SampleInfo.strFirstTestCounts = QString::number(mapItemCounts[(*it)->m_assayCode]);

            // 复查项目重复次数
            if (pReTestResult != nullptr)
            {
                auto LastTestItem = mapReviewItemCounts.find(pReTestResult->assayCode);
                if (LastTestItem != mapReviewItemCounts.end())
                {
                    SampleInfo.strReTestCounts = QString::number(mapReviewItemCounts[pReTestResult->assayCode]);
                }
            }

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

    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条

    // 弹框提示导出失败
    //TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")).exec();
    return bRet;
}

bool QHistory::ExportSampleData(const QString & path)
{
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    // 生化常规、急诊、质控导出表头(每个波长吸光度需要动态添加)
	QVariantList title_ch_normal;
    // 生化常规、急诊、质控 动态波长的表头
	QVariantList title_ch_cali;
    // ise导出表头（常规、急诊、质控）
	QVariantList title_ise_normal;
    // ise生化校准导出表头
	QVariantList title_ise_cali;
    // K:样本主键  
    //V:样本中包含哪些校准项目以及测试当前测试次数（K:项目编号，V:当前累计测试次数）
    std::map<int64_t, std::map<int, int>> caliSampleId;
    // 每一行的数据(每一个string为一行数据，‘\t’分隔)
    QVariantList content_ch;
    QVariantList content_ch_cali;
    QVariantList content_ise;
    QVariantList content_ise_cali;

    // 获取选中的索引
    const auto& selectedIndexs = ui->sample_list->selectionModel()->selectedRows();
    if (selectedIndexs.isEmpty())
    {
        TipDlg(tr("未选择样本数据！")).exec();
        return false;
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

    // 获取选中的样本数据
    std::vector<std::shared_ptr<HistoryBaseDataSample>> selectedSamples;
    gHisSampleAssayModel.GetSelectedSamples(selectedIndexs, selectedSamples);

    // 获取选中的样数据包含的项目数据
    std::vector<std::shared_ptr<HistoryBaseDataByItem>> itemsInSelectedSample;
    gHisSampleAssayModel.GetItemsInSelectedSamples(selectedSamples, itemsInSelectedSample);

	// 转换数据为中间数据
	ConstructHisExportInfo exportHisData;
	exportHisData.ProcessData(itemsInSelectedSample);

	// 将中间数据导出为需要的文档数据
	ConvertDataToExportString exportDocment;
	auto exportDataMap = exportHisData.GetExportMap();
	auto exportAbsTitleMap = exportHisData.GetExtendTitleMap();
	exportDocment.GenData(isXlsx, exportDataMap, exportAbsTitleMap);

	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 30);
    // 导出文件
    FileExporter fileExporter;
    auto& classify = fileExporter.GetDataClassify();
    classify.clear();
	content_ch = exportDocment.GetContent(ExportType::CH_NORMAL_AND_QC_TYPE);
    if (!content_ch.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("生化_常规"));
		title_ch_normal = exportDocment.GetTitle(ExportType::CH_NORMAL_AND_QC_TYPE);
        exTask.title = std::move(title_ch_normal);
        exTask.contents = std::move(content_ch);
        classify.append(exTask);
    }

	content_ch_cali = exportDocment.GetContent(ExportType::CH_CALI_TYPE);
    if (!content_ch_cali.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("生化_校准"));
		title_ch_cali = exportDocment.GetTitle(ExportType::CH_CALI_TYPE);
        exTask.title = std::move(title_ch_cali);
        exTask.contents = std::move(content_ch_cali);
        classify.append(exTask);
    }

	content_ise = exportDocment.GetContent(ExportType::CH_ISE_SAM_AND_QC_TYPE);
    if (!content_ise.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("电解质_常规"));
		title_ise_normal = exportDocment.GetTitle(ExportType::CH_ISE_SAM_AND_QC_TYPE);
        exTask.title = std::move(title_ise_normal);
        exTask.contents = std::move(content_ise);
        classify.append(exTask);
    }

	content_ise_cali = exportDocment.GetContent(ExportType::CH_ISE_CALI_TYPE);
    if (!content_ise_cali.isEmpty())
    {
        ExTaskType exTask;
        exTask.sheetName = std::move(tr("电解质_校准"));
		title_ise_cali = exportDocment.GetTitle(ExportType::CH_ISE_CALI_TYPE);
        exTask.title = std::move(title_ise_cali);
        exTask.contents = std::move(content_ise_cali);
        classify.append(exTask);
    }

	// 获取数据
	ExpSampleInfoVector vecSamples;
	for (const auto& sample : exportDataMap)
	{
		vecSamples.insert(vecSamples.begin(), sample.second.begin(), sample.second.end());
	}

	fileExporter.ExportInfoToFileByClassify(path, classify, vecSamples);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE, false);
	return true;
}

bool QHistory::ExportSample_ch(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& dynamicTitle, QVariantList& rowDatas)
{
    // 如果没有测试结果主键（初查或复查），直接返回
    if (item->m_itemId < 0)
        return false;
    QVariantList dyTitles;

    auto spGeneralAssayInfo = m_pCommAssayMgr->GetAssayInfo(item->m_assayCode);
    std::shared_ptr<ch::tf::GeneralAssayInfo>  spChGeneralAssayInfo = nullptr;
    if (item->m_assayCode == ch::tf::g_ch_constants.ASSAY_CODE_H || item->m_assayCode == ch::tf::g_ch_constants.ASSAY_CODE_L || item->m_assayCode == ch::tf::g_ch_constants.ASSAY_CODE_I)
        spChGeneralAssayInfo = m_pCommAssayMgr->GetChemistryAssayInfo(ch::tf::g_ch_constants.ASSAY_CODE_SIND, tf::DeviceType::DEVICE_TYPE_C1000);
    else
        spChGeneralAssayInfo = m_pCommAssayMgr->GetChemistryAssayInfo(item->m_assayCode, tf::DeviceType::DEVICE_TYPE_C1000);

    if (spGeneralAssayInfo == nullptr || spChGeneralAssayInfo == nullptr)
        return false;

    auto&& assayTestResults = GetChAssayTestResult(item->m_itemId);
    if (assayTestResults.empty())
        return false;

    // 获取样本的病人信息
    auto spPatient = GetPatientInfoById(item->m_patientId);

    int resultSize = assayTestResults.size();
    for (size_t i = 0; i < resultSize; ++i)
    {
        auto& result = assayTestResults[i];
        if (i > 0)
        {
            // 没有设置复查结果，则需要跳过中间结果，直接获取最后一个结果为复查结果
            if (item->m_userUseRetestResultID == -1 && i < resultSize - 1)
                continue;
            else if (item->m_userUseRetestResultID != -1 && result.id != item->m_userUseRetestResultID)
                continue;
        }

        QVariantList rowData;
        // 样本类型
        rowData << ThriftEnumTrans::GetOrderSampleType((tf::HisSampleType::type)(item->m_sampleType));
        // 检测模式
        QString testMode = " ";

        rowData << testMode;
        // 样本类型
        rowData << ThriftEnumTrans::GetSourceTypeName(item->m_sampleSourceType, " ");
        // 样本杯类型
        rowData << " ";
        // 预稀释
        rowData << result.preDilutionFactor;
        // 样本条码号
        rowData << ToExQString(item->m_barcode);
        // 样本号
        QString qc;
        if (item->m_sampleType == tf::HisSampleType::SAMPLE_SOURCE_QC)
        {
            qc = "QC";
        }
        rowDatas << (qc + ToExQString(item->m_seqNo));
        // 架号
        rowData << ToExQString(item->m_pos);
        // 位置号
        rowData << ToExQString(item->m_pos);
        // 年龄
        rowData << ThriftEnumTrans::GetAge(spPatient);
        // 性别
        rowData << (spPatient == nullptr ? " " : ThriftEnumTrans::GetGender(spPatient->gender, " "));
        // 加R1时间
        rowData << " ";
        // 加S时间
        rowData << " ";
        // 混匀时间1
        rowData << " ";
        // 加R2时间
        rowData << " ";
        // 混匀时间2
        rowData << " ";
        // 项目检测完成时间
        rowData << ToExQString(result.endTime);
        // 通道号
        rowData << QString::number(result.assayCode);
        // 项目名称
        rowData << ToExQString(spGeneralAssayInfo->assayName);
        // 结果
        rowData << QString::number(result.conc, 'f', spGeneralAssayInfo->decimalPlace).toDouble();
        // 计算吸光度
        rowData << result.abs;
        // 结果状态
        rowData << ToExQString(result.resultStatusCodes);
        // 单位
        rowData << QString::fromStdString(item->m_unit);;
        // 样本量/稀释倍数
        rowData << m_pCommAssayMgr->GetDilutionFactor(std::make_shared<ch::tf::AssayTestResult>(result));
        // R1批号
        rowData << ((result.reagentKeyInfos.size() > 0 && result.reagentKeyInfos[0].__isset.lot) ? ToExQString(result.reagentKeyInfos[0].lot) : " ");
        // R1瓶号
        rowData << ((result.reagentKeyInfos.size() > 0 && result.reagentKeyInfos[0].__isset.sn) ? ToExQString(result.reagentKeyInfos[0].sn) : " ");
        // R1位置
        rowData << " ";
        // R2批号
        rowData << ((result.reagentKeyInfos.size() > 1 && result.reagentKeyInfos[1].__isset.lot) ? ToExQString(result.reagentKeyInfos[1].lot) : " ");
        // R2瓶号
        rowData << ((result.reagentKeyInfos.size() > 1 && result.reagentKeyInfos[1].__isset.sn) ? ToExQString(result.reagentKeyInfos[1].sn) : " ");
        // R2位置
        rowData << " ";
        // 试剂失效日期
        rowData << (result.resultSupplyInfo.__isset.reagentexpiryDate ? ToExQString(result.resultSupplyInfo.reagentexpiryDate) : " ");
        // 开瓶有效期
        rowData << (result.resultSupplyInfo.__isset.reagentopenBottleExpiryTime && result.resultSupplyInfo.reagentopenBottleExpiryTime >= 0 ? QString::number(result.resultSupplyInfo.reagentopenBottleExpiryTime) : "0");
        // 校准品名称
        rowData << (result.resultCaliInfo.__isset.calibratorName ? ToExQString(result.resultCaliInfo.calibratorName) : " ");
        // 校准品批号
        rowData << (result.resultCaliInfo.__isset.calibratorLot ? ToExQString(result.resultCaliInfo.calibratorLot) : " ");
        // 校准曲线失效日期
        rowData << (result.resultCaliInfo.__isset.calibratorExpiryDate ? ToExQString(result.resultCaliInfo.calibratorExpiryDate) : " ");
        // 校准时间
        rowData << (result.resultCaliInfo.__isset.caliDate ? ToExQString(result.resultCaliInfo.caliDate) : " ");
        // 反应杯号
        rowData << QString::number(result.cupSN);
        // 分析方法
        rowData << QString::number(spChGeneralAssayInfo->analysisMethod);
        // 主波长
        rowData << (!result.primarySubWaves.empty() ? QString::number(result.primarySubWaves[0]) : " ");
        // 副波长
        rowData << ((result.primarySubWaves.size() > 1) ? QString::number(result.primarySubWaves[1]) : " ");
        // 反应时间
        rowData << ((result.detectPoints.size() == 34) ? 10 : 5);
        // 测光点(总数，非m,n)
        rowData << static_cast<qulonglong>(result.detectPoints.size());

        // 水空白波长
        std::vector<std::map<int32_t, int32_t>> waterBlanks;
        if (DecodeWaterBlankOds(result.waterBlanks, waterBlanks) && !waterBlanks.empty())
        {
            for (auto& wb : waterBlanks[0])
            {
                rowData << wb.second;
                dyTitles << (QString::number(wb.first) + tr("波长水空白"));
            }
        }

        // 测光点的吸光度
        {
            std::map<int, std::vector<int>> wlClassify;
            int iDetectPointCount = (int)result.detectPoints.size();
            for (int pointIdx = 0; pointIdx < 34; pointIdx++)
            {
                // 检查测光点是否越界
                if (pointIdx >= iDetectPointCount)
                {
                    continue;
                }

                ::ch::tf::DetectPoint& dp = result.detectPoints[pointIdx];
                // 将每个波长的吸光度都分类出来
                for (auto& od : dp.ods)
                    wlClassify[od.first].push_back(od.second);
            }

            // 导出主副波长
            int loop = 0;
            do
            {
                QString waveName;
                if (loop == 0)
                    waveName = tr("主波长吸光度");
                else
                    waveName = tr("副波长吸光度");

                for (size_t o_i = 0; o_i < 34; )
                {
                    if (loop >= result.primarySubWaves.size() || wlClassify.find(result.primarySubWaves[loop]) == wlClassify.end())
                    {
                        rowData << " ";
                    }
                    else
                    {
                        auto& odsVec = wlClassify[result.primarySubWaves[loop]];
                        // 按照最多34个点导出，不足的，导出为" ";
                        if (o_i < odsVec.size())
                        {
                            rowData << odsVec[o_i];
                        }
                        else
                        {
                            rowData << " ";
                        }
                    }
                    dyTitles << (waveName + QString::number(++o_i));
                }
            } while (++loop < 2);

            // 重新组织波长和吸光度数据
            for (auto& wl : wlClassify)
            {
                for (size_t o_i = 0; o_i < 34; )
                {
                    // 按照最多34个点导出，不足的，导出为" ";
                    if (o_i < wl.second.size())
                    {
                        rowData << wl.second[o_i];
                        dyTitles << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
                    }
                    else
                    {
                        rowData << " ";
                        dyTitles << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
                    }
                }
            }
        }

        rowDatas << rowData;
    }

    // 插入标题
    if (dynamicTitle.isEmpty())
        dynamicTitle << dyTitles;

    return true;
}

bool QHistory::ExportSample_ch_cali(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& dynamicTitle, QVariantList& rowDatas, int times)
{
    // 如果没有测试结果主键（初查或复查），直接返回
    if (item->m_itemId < 0)
        return false;
    QVariantList dyTitles;

    auto spGeneralAssayInfo = m_pCommAssayMgr->GetAssayInfo(item->m_assayCode);
    std::shared_ptr<ch::tf::GeneralAssayInfo>  spChGeneralAssayInfo = nullptr;
    if (item->m_assayCode == ch::tf::g_ch_constants.ASSAY_CODE_H || item->m_assayCode == ch::tf::g_ch_constants.ASSAY_CODE_L || item->m_assayCode == ch::tf::g_ch_constants.ASSAY_CODE_I)
        spChGeneralAssayInfo = m_pCommAssayMgr->GetChemistryAssayInfo(ch::tf::g_ch_constants.ASSAY_CODE_SIND, tf::DeviceType::DEVICE_TYPE_C1000);
    else
        spChGeneralAssayInfo = m_pCommAssayMgr->GetChemistryAssayInfo(item->m_assayCode, tf::DeviceType::DEVICE_TYPE_C1000);

    if (spGeneralAssayInfo == nullptr || spChGeneralAssayInfo == nullptr)
        return false;

    auto&& assayTestResults = GetChAssayTestResult(item->m_itemId);
    if (assayTestResults.empty())
        return false;

    int resultSize = assayTestResults.size();
    for (size_t i = 0; i < resultSize; ++i)
    {
        auto& result = assayTestResults[i];
        if (i > 0)
        {
            // 没有设置复查结果，则需要跳过中间结果，直接获取最后一个结果为复查结果
            if (item->m_userUseRetestResultID == -1 && i < resultSize - 1)
                continue;
            else if (item->m_userUseRetestResultID != -1 && result.id != item->m_userUseRetestResultID)
                continue;
        }

        QVariantList rowData;
        // 次数
        rowData << QString::number(times);
        // 校准方法
        rowData << " ";
        // 校准点数
        rowData << " ";
        // 架号
        rowData << ToExQString(item->m_pos);
        // 位置号
        rowData << ToExQString(item->m_pos);
        // 反应杯号
        rowData << QString::number(result.cupSN);
        // 校准品名称
        rowData << (result.resultCaliInfo.__isset.calibratorName ? ToExQString(result.resultCaliInfo.calibratorName) : " ");
        // 校准品批号
        rowData << (result.resultCaliInfo.__isset.calibratorLot ? ToExQString(result.resultCaliInfo.calibratorLot) : " ");
        // 项目名称
        rowData << ToExQString(spGeneralAssayInfo->assayName);
        // 通道号
        rowData << QString::number(result.assayCode);
        //样本量 / 稀释倍数
        rowData << m_pCommAssayMgr->GetDilutionFactor(std::make_shared<ch::tf::AssayTestResult>(result));
        // 计算吸光度
        rowData << result.abs;
        // 结果状态
        rowData << ToExQString(result.resultStatusCodes);
        // 校准水平
        rowData << (item->m_seqNo.empty() ? " " : QString(QChar::fromLatin1(item->m_seqNo.back())));
        //浓度
        rowData << QString::number(result.conc, 'f', spGeneralAssayInfo->decimalPlace).toDouble();
        //单位
        rowData << QString::fromStdString(item->m_unit);;
        // 检测日期
        rowData << ToExQString(result.endTime);
        // 时间
        rowData << ToExQString(result.endTime);
        // R1批号
        rowData << ((result.reagentKeyInfos.size() > 0 && result.reagentKeyInfos[0].__isset.lot) ? ToExQString(result.reagentKeyInfos[0].lot) : " ");
        // R1瓶号
        rowData << ((result.reagentKeyInfos.size() > 0 && result.reagentKeyInfos[0].__isset.sn) ? ToExQString(result.reagentKeyInfos[0].sn) : " ");
        // R1位置
        rowData << " ";
        // R2批号
        rowData << ((result.reagentKeyInfos.size() > 1 && result.reagentKeyInfos[1].__isset.lot) ? ToExQString(result.reagentKeyInfos[1].lot) : " ");
        // R2瓶号
        rowData << ((result.reagentKeyInfos.size() > 1 && result.reagentKeyInfos[1].__isset.sn) ? ToExQString(result.reagentKeyInfos[1].sn) : " ");
        // R2位置
        rowData << " ";
        // 样本号
        rowData << ToExQString(item->m_seqNo);
        // 主波长
        rowData << (!result.primarySubWaves.empty() ? QString::number(result.primarySubWaves[0]) : " ");
        // 副波长
        rowData << ((result.primarySubWaves.size() > 1) ? QString::number(result.primarySubWaves[1]) : " ");
        // 反应时间
        rowData << ((result.detectPoints.size() == 34) ? 10 : 5);
        // 测光点(总数，非m,n)
        rowData << static_cast<qulonglong>(result.detectPoints.size());

        // 水空白波长
        std::vector<std::map<int32_t, int32_t>> waterBlanks;
        if (DecodeWaterBlankOds(result.waterBlanks, waterBlanks) && !waterBlanks.empty())
        {
            for (auto& wb : waterBlanks[0])
            {
                rowData << wb.second;
                dyTitles << (QString::number(wb.first) + tr("波长水空白"));
            }
        }

        // 测光点的吸光度
        {
            std::map<int, std::vector<int>> wlClassify;
            int iDetectPointCount = (int)result.detectPoints.size();
            for (int pointIdx = 0; pointIdx < 34; pointIdx++)
            {
                // 检查测光点是否越界
                if (pointIdx >= iDetectPointCount)
                {
                    continue;
                }

                ::ch::tf::DetectPoint& dp = result.detectPoints[pointIdx];
                // 将每个波长的吸光度都分类出来
                for (auto& od : dp.ods)
                    wlClassify[od.first].push_back(od.second);
            }

            // 导出主副波长
            int loop = 0;
            do
            {
                QString waveName;
                if (loop == 0)
                    waveName = tr("主波长吸光度");
                else
                    waveName = tr("副波长吸光度");

                for (size_t o_i = 0; o_i < 34; )
                {
                    if (loop >= result.primarySubWaves.size() || wlClassify.find(result.primarySubWaves[loop]) == wlClassify.end())
                    {
                        rowData << " ";
                    }
                    else
                    {
                        auto& odsVec = wlClassify[result.primarySubWaves[loop]];
                        // 按照最多34个点导出，不足的，导出为" ";
                        if (o_i < odsVec.size())
                        {
                            rowData << odsVec[o_i];
                        }
                        else
                        {
                            rowData << " ";
                        }
                    }
                    dyTitles << (waveName + QString::number(++o_i));
                }
            } while (++loop < 2);

            // 重新组织波长和吸光度数据
            for (auto& wl : wlClassify)
            {
                for (size_t o_i = 0; o_i < 34; )
                {
                    // 按照最多34个点导出，不足的，导出为" ";
                    if (o_i < wl.second.size())
                    {
                        rowData << wl.second[o_i];
                        dyTitles << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
                    }
                    else
                    {
                        rowData << " ";
                        dyTitles << (QString::number(wl.first) + "_" + /*tr("波长吸光度") +*/ QString::number(++o_i));
                    }
                }
            }
        }

        rowDatas << rowData;
    }

    // 插入标题
    if (dynamicTitle.isEmpty())
        dynamicTitle << dyTitles;

    return true;
}

bool QHistory::ExportSample_ise(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& rowDatas)
{
    // 如果没有测试结果主键（初查或复查），直接返回
    if (item->m_itemId < 0)
        return false;

    auto spGeneralAssayInfo = m_pCommAssayMgr->GetAssayInfo(item->m_assayCode);
    auto spChGeneralAssayInfo = m_pCommAssayMgr->GetIseAssayInfo(item->m_assayCode, tf::DeviceType::DEVICE_TYPE_ISE1005);

    if (spGeneralAssayInfo == nullptr || spChGeneralAssayInfo == nullptr)
        return false;

    auto&& assayTestResults = GetIseAssayTestResult(item->m_itemId);
    if (assayTestResults.empty())
        return false;

    // 获取样本的病人信息
    auto spPatient = GetPatientInfoById(item->m_patientId);

    int resultSize = assayTestResults.size();
    for (size_t i = 0; i < resultSize; ++i)
    {
        auto& result = assayTestResults[i];
        if (i > 0)
        {
            // 没有设置复查结果，则需要跳过中间结果，直接获取最后一个结果为复查结果
            if (item->m_userUseRetestResultID == -1 && i < resultSize - 1)
                continue;
            else if (item->m_userUseRetestResultID != -1 && result.id != item->m_userUseRetestResultID)
                continue;
        }

        QVariantList rowData;
        // 样本类型
        rowData << ThriftEnumTrans::GetOrderSampleType((tf::HisSampleType::type)(item->m_sampleType));
        // 检测模式
        QString testMode = " ";

        rowData << testMode;
        // 样本类型
        rowData << ThriftEnumTrans::GetSourceTypeName(item->m_sampleSourceType, " ");
        // 样本杯类型
        rowData << " ";
        // 预稀释
        rowData << QString::number(result.preDilutionFactor);
        // 样本条码号
        rowData << ToExQString(item->m_barcode);
        // 样本号
        QString qc;
        if (item->m_sampleType == tf::HisSampleType::SAMPLE_SOURCE_QC)
        {
            qc = "QC";
        }
        rowDatas << (qc + ToExQString(item->m_seqNo));
        // 架号
        rowData << ToExQString(item->m_pos);
        // 位置号
        rowData << ToExQString(item->m_pos);
        // 年龄
        rowData << ThriftEnumTrans::GetAge(spPatient);
        // 性别
        rowData << (spPatient == nullptr ? " " : ThriftEnumTrans::GetGender(spPatient->gender, " "));
        // 项目检测完成时间
        rowData << ToExQString(result.endTime);
        // 通道号
        rowData << QString::number(result.assayCode);
        // 项目名称
        rowData << ToExQString(spGeneralAssayInfo->assayName);
        // 结果
        rowData << QString::number(result.conc, 'f', spGeneralAssayInfo->decimalPlace).toDouble();
        // 计算电动势
        rowData << result.testEMF.sampleEMF;
        // 结果状态
        rowData << ToExQString(result.resultStatusCodes);
        // 结果单位
        rowData << QString::fromStdString(item->m_unit);;
        // 内部标准液批号
        rowData << (result.resultSupplyInfo.__isset.IS_Lot ? ToExQString(result.resultSupplyInfo.IS_Lot) : " ");
        // 内部标准液瓶号
        rowData << (result.resultSupplyInfo.__isset.IS_Sn ? ToExQString(result.resultSupplyInfo.IS_Sn) : " ");
        // 缓冲液批号
        rowData << (result.resultSupplyInfo.__isset.diluent_Lot ? ToExQString(result.resultSupplyInfo.diluent_Lot) : " ");
        // 缓冲液瓶号
        rowData << (result.resultSupplyInfo.__isset.diluent_Sn ? ToExQString(result.resultSupplyInfo.diluent_Sn) : " ");
        // 校准品名称
        rowData << (result.resultCaliInfo.__isset.calibratorName ? ToExQString(result.resultCaliInfo.calibratorName) : " ");
        // 校准品批号
        rowData << (result.resultCaliInfo.__isset.calibratorLot ? ToExQString(result.resultCaliInfo.calibratorLot) : " ");
        // 校准曲线失效日期
        rowData << (result.resultCaliInfo.__isset.calibratorExpiryDate ? ToExQString(result.resultCaliInfo.calibratorExpiryDate) : " ");
        // 校准时间
        rowData << (result.resultCaliInfo.__isset.caliDate ? ToExQString(result.resultCaliInfo.caliDate) : " ");

        rowDatas << rowData;
    }

    return true;
}

bool QHistory::ExportSample_ise_cali(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& rowDatas, int times)
{
    if (item->m_itemId < 0)
        return false;

    auto spGeneralAssayInfo = m_pCommAssayMgr->GetAssayInfo(item->m_assayCode);
    auto spChGeneralAssayInfo = m_pCommAssayMgr->GetIseAssayInfo(item->m_assayCode, tf::DeviceType::DEVICE_TYPE_ISE1005);

    if (spGeneralAssayInfo == nullptr || spChGeneralAssayInfo == nullptr)
        return false;

    auto&& assayTestResults = GetIseAssayTestResult(item->m_itemId);
    if (assayTestResults.empty())
        return false;

    int resultSize = assayTestResults.size();
    for (size_t i = 0; i < resultSize; ++i)
    {
        auto& result = assayTestResults[i];
        if (i > 0)
        {
            // 没有设置复查结果，则需要跳过中间结果，直接获取最后一个结果为复查结果
            if (item->m_userUseRetestResultID == -1 && i < resultSize - 1)
                continue;
            else if (item->m_userUseRetestResultID != -1 && result.id != item->m_userUseRetestResultID)
                continue;
        }

        QVariantList rowData;
        // 次数
        rowData << QString::number(times);
        // 架号
        rowData << ToExQString(item->m_pos);
        // 位置号
        rowData << ToExQString(item->m_pos);
        // 校准品名称
        rowData << (result.resultCaliInfo.__isset.calibratorName ? ToExQString(result.resultCaliInfo.calibratorName) : " ");
        // 校准品批号
        rowData << (result.resultCaliInfo.__isset.calibratorLot ? ToExQString(result.resultCaliInfo.calibratorLot) : " ");
        // 项目名称
        rowData << ToExQString(spGeneralAssayInfo->assayName);
        // 通道号
        rowData << QString::number(result.assayCode);
        // 稀释率
        rowData << " ";
        // 计算电动势
        rowData << QString::number(result.testEMF.sampleEMF);
        // 结果状态
        rowData << ToExQString(result.resultStatusCodes);
        // 校准水平
        rowData << (item->m_seqNo.empty() ? " " : QString(QChar::fromLatin1(item->m_seqNo.back())));
        // 浓度
        rowData << QString::number(result.conc, 'f', spGeneralAssayInfo->decimalPlace);
        // 结果单位
        rowData << QString::fromStdString(item->m_unit);;
        // 项目检测完成日期
        rowData << ToExQString(result.endTime);
        // 项目检测完成时间
        rowData << ToExQString(result.endTime);
        // 内部标准液批号
        rowData << (result.resultSupplyInfo.__isset.IS_Lot ? ToExQString(result.resultSupplyInfo.IS_Lot) : " ");
        // 内部标准液瓶号
        rowData << (result.resultSupplyInfo.__isset.IS_Sn ? ToExQString(result.resultSupplyInfo.IS_Sn) : " ");
        // 缓冲液批号
        rowData << (result.resultSupplyInfo.__isset.diluent_Lot ? ToExQString(result.resultSupplyInfo.diluent_Lot) : " ");
        // 缓冲液瓶号
        rowData << (result.resultSupplyInfo.__isset.diluent_Sn ? ToExQString(result.resultSupplyInfo.diluent_Sn) : " ");
        // 样本号
        rowData << ToExQString(item->m_seqNo);

        rowDatas << rowData;
    }

    return true;
}

bool QHistory::ExportSample_calc(std::shared_ptr<HistoryBaseDataByItem> item, QVariantList& rowDatas)
{
    return true;
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
bool QHistory::ExPortItem()
{
	auto softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();

    // 弹框获取保存文件名
    // bug0020858 add by ldx 20230913
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
        ULOG(LOG_WARN, "Null file name.");
        return false;
    }

	// 生化
	if (softwareType == SOFTWARE_TYPE::CHEMISTRY)
    {
		// 耗时操作放在线程里完成
		std::thread th(std::bind(&QHistory::ExPortItemData, this, fileName));
		th.detach();
		return true;
    }
	// 免疫
	else if (softwareType == SOFTWARE_TYPE::IMMUNE)
	{
		// 获取选中的索引
		auto selectedIndexs = ui->show_assay_list->selectionModel()->selectedRows();
		if (selectedIndexs.isEmpty())
		{
            ULOG(LOG_INFO, "Empty selected indexes.");
			return false;
		}

		// 耗时操作放在线程里完成
		std::thread th(std::bind(&QHistory::ExportItemData_im, this, fileName, selectedIndexs));
		th.detach();
	}
	// 生免联机
	else
	{
		// 耗时操作放在线程里完成
		std::thread th(std::bind(&QHistory::ExPortItemData, this, FileExporter::GetFileName(true, fileName)));
		th.detach();

		// 免疫项目导出
		{
			// 获取选中的索引
			auto selectedIndexs = ui->show_assay_list->selectionModel()->selectedRows();
			if (selectedIndexs.isEmpty())
            {
                ULOG(LOG_INFO, "Empty selected indexes.");
				return false;
			}

			// 耗时操作放在线程里完成
			std::thread th(std::bind(&QHistory::ExportItemData_im, this, FileExporter::GetFileName(false, fileName), selectedIndexs));
			th.detach();
		}
	}

    return true;
    
}

bool QHistory::ExportItemData_im(QString fileName, const QModelIndexList &selectedIndexs)
{
    ULOG(LOG_ERROR, "%s()", __FUNCTION__);

    std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if (pLoginUserInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can't get UserInfo.");
        return false;
    }

    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."));

    // 导出文本列表
    ExpSampleInfoVector vecSamples;
    std::vector<std::shared_ptr<HistoryBaseDataByItem>> selectedItems = gHisSampleAssayModel.GetSelectedItems(selectedIndexs);
    for (std::shared_ptr<HistoryBaseDataByItem> pItem : selectedItems)
    {
        bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(pItem->m_assayCode);
		auto spGenneral = CommonInformationManager::GetInstance()->GetAssayInfo(pItem->m_assayCode);
		if (spGenneral == nullptr && !isCalc)
		{
			continue;
		}

		// 只导出免疫项目(0013739)
		if (!(isCalc || spGenneral->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
		{
			continue;
		}

        // 获取选中的样数据包含的项目数据
        std::shared_ptr<HistoryBaseDataSample> pSample;
        gHisSampleAssayModel.GetSamplesInSelectedItems(pItem, pSample);
        SampleExportInfo SampleInfo;

        // 样本号
        SampleInfo.strSampleNo = GetSeqNo((::tf::HisSampleType::type)pSample->m_sampleType, 
            (::tf::TestMode::type)pSample->m_testMode, pSample->m_seqNo);

        // 样本条码
        SampleInfo.strBarcode = QString::fromStdString(pSample->m_barcode);

        // 样本类型
        if (pSample->m_sampleSourceType >= 0)
            SampleInfo.strSampleSourceType = ConvertTfEnumToQString((::tf::SampleSourceType::type)pSample->m_sampleSourceType);

        // 获取测试项目的首次和复查结果
        std::shared_ptr<::im::tf::AssayTestResult> pFirstResult = nullptr;
        std::shared_ptr<::im::tf::AssayTestResult> pReTestResult = nullptr;
        GetImAssayTestResult(pSample->m_sampleId, pItem->m_itemId, pItem->m_reTestEndTimeMax, pFirstResult, pReTestResult);

        MakeExportItemResult(false, SampleInfo, pSample, pItem, pFirstResult, pReTestResult);
        vecSamples.push_back(SampleInfo);
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
        bRet = fileExporter.ExportSampleInfo(vecSamples, fileName, pLoginUserInfo->type);
    }

    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条

    // 弹框提示导出失败
    //TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")).exec();
    return bRet;
}

bool QHistory::ExPortItemData(const QString & path)
{
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    // 生化常规、急诊、质控导出表头(每个波长吸光度需要动态添加)
    QVariantList title_ch_normal = { tr("样本类别"), tr("检测模式"), tr("样本类型"), tr("样本杯类型"), tr("预稀释"), tr("样本条码号"), tr("样本号"), tr("架号"), tr("位置号"), tr("年龄"), tr("性别"), /*tr("操作员账号"),*/ tr("加R1时间"), tr("加S时间"), tr("混匀时间1"), tr("加R2时间"), tr("混匀时间2"), tr("项目检测完成时间"), tr("通道号"), tr("项目名称"), tr("结果"), tr("计算吸光度"), tr("结果状态"), tr("单位"), tr("样本量 / 稀释倍数"), tr("R1批号"), tr("R1瓶号"), tr("R1位置"), tr("R2批号"), tr("R2瓶号"), tr("R2位置"), tr("试剂失效日期"), tr("开瓶有效期"), tr("校准品名称"), tr("校准品批号"), tr("校准曲线失效日期"), tr("校准时间"), tr("反应杯号"), tr("分析方法"), tr("主波长"), tr("副波长"), tr("反应时间"), tr("测光点"), /*tr("340波长杯空白")*/ };
    // 生化常规、急诊、质控 动态波长的表头
    QVariantList wareLenTitle;
    // 生化校准导出表头(每个波长吸光度需要动态添加)
    QVariantList title_ch_cali = { tr("次数"), tr("校准方法"), tr("校准点数"), tr("架号"), tr("位置号"), tr("反应杯号"), tr("校准品名称"), tr("校准品批号"), tr("项目名称"), tr("通道号"), tr("样本量/稀释倍数"), tr("计算吸光度"), tr("结果状态"), tr("校准水平"), tr("浓度"), tr("单位"), tr("检测日期"), tr("时间"), tr("R1批号"), tr("R1瓶号"), tr("R1位置"), tr("R1批号"), tr("R2瓶号"), tr("R2位置"), tr("样本号"), tr("主波长"), tr("副波长"), tr("反应时间"), tr("测光点"), /*tr("340波长杯空白")*/ };
    // 生化校准 动态波长的表头
    QVariantList wareLenTitle_cali;
    // ise导出表头（常规、急诊、质控）
    QVariantList title_ise_normal = { tr("样本类别"), tr("检测模式"), tr("样本类型"), tr("样本杯类型"), tr("预稀释"), tr("样本条码号"), tr("样本号"), tr("架号"), tr("位置号"), tr("年龄"), tr("性别"), /*tr("操作员账号"),*/ tr("项目检测完成时间"), tr("通道号"), tr("项目名称"), tr("结果"), tr("计算电动势"), tr("结果状态"), tr("结果单位"), tr("内部标准液批号"), tr("内部标准液瓶号"), tr("缓冲液批号"), tr("缓冲液瓶号"), tr("校准品名称"), tr("校准品批号"), tr("校准曲线失效日期"), tr("校准时间") };
    // ise生化校准导出表头
    QVariantList title_ise_cali = { tr("次数"), tr("架号"), tr("位置号"), tr("校准品名称"), tr("校准品批号"), tr("项目名称"), tr("通道号"), tr("稀释率"), tr("计算电动势"), tr("结果状态"), tr("校准水平"), tr("浓度"), tr("结果单位"), tr("检测日期"), tr("时间"), tr("内部标准液批号"), tr("内部标准液瓶号"), tr("缓冲液批号"), tr("缓冲液瓶号"), tr("样本号") };
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

    // 获取选中的索引
    auto selectedIndexs = ui->show_assay_list->selectionModel()->selectedRows();
    if (selectedIndexs.isEmpty())
    {
        return false;
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

    std::vector<std::shared_ptr<HistoryBaseDataByItem>> selectedItems = gHisSampleAssayModel.GetSelectedItems(selectedIndexs);
	// 转换数据为中间数据
	ConstructHisExportInfo exportHisData;
	exportHisData.ProcessData(selectedItems);

	// 将中间数据导出为需要的文档数据
	ConvertDataToExportString exportDocment;
	auto exportDataMap = exportHisData.GetExportMap();
	auto exportAbsTitleMap = exportHisData.GetExtendTitleMap();
	exportDocment.GenData(isXlsx, exportDataMap, exportAbsTitleMap);

	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CHANGE, 30);
	// 导出文件
	FileExporter fileExporter;
	auto& classify = fileExporter.GetDataClassify();
	classify.clear();
	content_ch = exportDocment.GetContent(ExportType::CH_NORMAL_AND_QC_TYPE);
	if (!content_ch.isEmpty())
	{
		ExTaskType exTask;
		exTask.sheetName = std::move(tr("生化_常规"));
		title_ch_normal = exportDocment.GetTitle(ExportType::CH_NORMAL_AND_QC_TYPE);
		exTask.title = std::move(title_ch_normal);
		exTask.contents = std::move(content_ch);
		classify.append(exTask);
	}

	content_ch_cali = exportDocment.GetContent(ExportType::CH_CALI_TYPE);
	if (!content_ch_cali.isEmpty())
	{
		ExTaskType exTask;
		exTask.sheetName = std::move(tr("生化_校准"));
		title_ch_cali = exportDocment.GetTitle(ExportType::CH_CALI_TYPE);
		exTask.title = std::move(title_ch_cali);
		exTask.contents = std::move(content_ch_cali);
		classify.append(exTask);
	}

	content_ise = exportDocment.GetContent(ExportType::CH_ISE_SAM_AND_QC_TYPE);
	if (!content_ise.isEmpty())
	{
		ExTaskType exTask;
		exTask.sheetName = std::move(tr("电解质_常规"));
		title_ise_normal = exportDocment.GetTitle(ExportType::CH_ISE_SAM_AND_QC_TYPE);
		exTask.title = std::move(title_ise_normal);
		exTask.contents = std::move(content_ise);
		classify.append(exTask);
	}

	content_ise_cali = exportDocment.GetContent(ExportType::CH_ISE_CALI_TYPE);
	if (!content_ise_cali.isEmpty())
	{
		ExTaskType exTask;
		exTask.sheetName = std::move(tr("电解质_校准"));
		title_ise_cali = exportDocment.GetTitle(ExportType::CH_ISE_CALI_TYPE);
		exTask.title = std::move(title_ise_cali);
		exTask.contents = std::move(content_ise_cali);
		classify.append(exTask);
	}

	// 获取数据
	ExpSampleInfoVector vecSamples;
	for (const auto& sample : exportDataMap)
	{
		vecSamples.insert(vecSamples.begin(), sample.second.begin(), sample.second.end());
	}

	fileExporter.ExportInfoToFileByClassify(path, classify, vecSamples);
	POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE, false);
	return true;
}

void QHistory::MakeExportItemResult(bool isExportSample, SampleExportInfo& SampleInfo, const std::shared_ptr<HistoryBaseDataSample>& pSample,
    const std::shared_ptr<HistoryBaseDataByItem>& pItem, const std::shared_ptr<im::tf::AssayTestResult>& pFirstResult, 
    const std::shared_ptr<im::tf::AssayTestResult>& pReTestResult)
{
    bool isCalc = false;
    tf::HisSampleType::type sampleType = tf::HisSampleType::SAMPLE_SOURCE_NM;
    if (pItem != nullptr)
    {
        isCalc = CommonInformationManager::IsCalcAssay(pItem->m_assayCode);

        // 样本位置
        SampleInfo.strPos = QString::fromStdString(pItem->m_pos);
        // 项目名称
        SampleInfo.strItemName = QHistorySampleAssayModel::GetHistoryAssayNameByBaseItem(pItem);
        // 信号值
        SampleInfo.strRUL = QString::fromStdString(pItem->m_RLU);
        // 结果

        double dResult = 0.;
        if (pFirstResult != nullptr)
        {
            dResult = pFirstResult->concEdit;
        }

        if (isCalc)
        {
            dResult = pItem->m_testResult;
        }

        stAssayResult firstResult;
        QHistorySampleAssayModel::SetAssayResultFlag(dResult, false, *pItem, false, firstResult);
        SampleInfo.strTestResult = firstResult.m_result;

        // 结果状态
        SampleInfo.strResultStatus = QString::fromStdString(pItem->m_resultStatu);
        // 单位
        SampleInfo.strUnit = QHistorySampleAssayModel::GetResultUnit(*pItem);

        // 机内稀释倍数, 质控稀释倍数为空，因为界面不显示
        sampleType = (tf::HisSampleType::type)pItem->m_sampleType;
        if (tf::HisSampleType::SAMPLE_SOURCE_QC != sampleType && pItem->m_dilutionFactor > 0)
        {
            SampleInfo.strDilutionFactor = QString::number(pItem->m_dilutionFactor);
        }

        // 复查信号值
        SampleInfo.strReTestRUL = QString::fromStdString(pItem->m_reTestRLU);	
        if (pItem->m_retest)
        {        
            // 复查数据报警
            SampleInfo.strReResultStatus = QString::fromStdString(pItem->m_reTestResultStatu);

            
            if (pReTestResult != nullptr)
            {
                // 复查样本管类型
                SampleInfo.strReTestTubeType = ConvertTfEnumToQString((::tf::TubeType::type)pReTestResult->tubeType);
                // 复查视觉识别结果
                SampleInfo.strReAIResult = ConvertTfEnumToQString((::im::tf::AIResult::type)pReTestResult->aiResult);
                // 复查检测模式
                SampleInfo.strReTestModle = ConvertTfEnumToQString((::im::tf::DetectMode::type)pReTestResult->detectMode);
                // 复查进样模式
                SampleInfo.strReSampleSendModle = ThriftEnumTrans::GetTestMode(pReTestResult->testMode);
            }
        }
    }

    if (pFirstResult != nullptr)
    {
        // 模块
        auto spDevice = m_pCommAssayMgr->GetDeviceInfo(pFirstResult->deviceSN);
        if (spDevice == nullptr)
        {
            SampleInfo.strTestModule = " ";
        }
        else
        {
            SampleInfo.strTestModule = QString::fromStdString(spDevice->groupName + spDevice->name);
        }

        // 样本管类型
        SampleInfo.strTubeType = ConvertTfEnumToQString((::tf::TubeType::type)pFirstResult->tubeType);
        // 检测模式
        SampleInfo.strTestModle = ConvertTfEnumToQString((::im::tf::DetectMode::type)pFirstResult->detectMode);
        // 进样模式
        SampleInfo.strSampleSendModle = ThriftEnumTrans::GetTestMode(pFirstResult->testMode);

        // 原始信号值
        if (pFirstResult->RLU >= 0)
        {
            SampleInfo.strOrignialRUL = QString::number(pFirstResult->RLU, 'f', 0);
        }
            

        // 原始结果
        if (pFirstResult->conc >= 0. && !isCalc)
        {
            stAssayResult OrigResult;
            QHistorySampleAssayModel::SetAssayResultFlag(pFirstResult->conc, true, *pItem, false, OrigResult);
            SampleInfo.strOrignialTestResult = OrigResult.m_result;
        }

        // 检测完成时间
        SampleInfo.strEndTestTime = QString::fromStdString(pFirstResult->endTime);

        // 视觉识别结果
        SampleInfo.strAIResult = ConvertTfEnumToQString((::im::tf::AIResult::type)pFirstResult->aiResult);

        // 手工稀释倍数
        if (tf::HisSampleType::SAMPLE_SOURCE_QC != sampleType && pFirstResult->preDilutionFactor > 0)
        {
            SampleInfo.strPreDilutionFactor = QString::number(pFirstResult->preDilutionFactor);
        }

        // 试剂批号
        SampleInfo.strReagentBatchNo = QString::fromStdString(pFirstResult->reagentBatchNo);
        // 试剂瓶号
        SampleInfo.strReagentSerialNo = QString::fromStdString(pFirstResult->reagentSerialNo);
        // 底物液批号
        SampleInfo.strSubstrateBatchNo = QString::fromStdString(pFirstResult->supplyInfo.substrateABatchNo);
        // 底物液瓶号
        SampleInfo.strSubstrateBottleNo = QString::fromStdString(pFirstResult->supplyInfo.substrateASerialNo);
        // 清洗缓冲液批号
        SampleInfo.strCleanFluidBatchNo = QString::fromStdString(pFirstResult->supplyInfo.cleanFluidBatchNo);
        // 反应杯批号
        SampleInfo.strAssayCupBatchNo = QString::fromStdString(pFirstResult->supplyInfo.assayCupBatchNo);
        // 稀释液批号
        SampleInfo.strDiluentBatchNo = QString::fromStdString(pFirstResult->supplyInfo.diluentBatchNo);
        // 试剂上机时间
        SampleInfo.strRgtRegisterT = QString::fromStdString(pFirstResult->supplyInfo.reagentRegisterTime);
        // 试剂失效日期
        SampleInfo.strReagentExpTime = QString::fromStdString(pFirstResult->supplyInfo.reagentExpTime);
        // 校准品批号
        SampleInfo.strCaliLot = QString::fromStdString(pFirstResult->caliLot);
        // 校准曲线信息
        if (!pFirstResult->curCaliTime.empty() && tf::HisSampleType::SAMPLE_SOURCE_CL != sampleType)
        {
            SampleInfo.strFirstCurCaliTime = QString::fromStdString(pFirstResult->curCaliTime)
                + "   "
                + QString::fromStdString(pFirstResult->reagentBatchNo);
        }
    }

    if (pReTestResult != nullptr)
    {
        // 模块
        auto spDevice = m_pCommAssayMgr->GetDeviceInfo(pReTestResult->deviceSN);
        if (spDevice == nullptr)
        {
            SampleInfo.strReTestModule = " ";
        }
        else
        {
            SampleInfo.strReTestModule = QString::fromStdString(spDevice->groupName + spDevice->name);
        }

        // 复查结果
        double dretestResult = pReTestResult->concEdit;
        if (isCalc)
        {
            dretestResult = std::stod(pItem->m_reTestResult);
        }

        stAssayResult retestResult;
        QHistorySampleAssayModel::SetAssayResultFlag(dretestResult, false, *pItem, true, retestResult);
        SampleInfo.strReTestResult = retestResult.m_result;

        // 复查原始信号值
        if (pReTestResult->RLU >= 0)
        {
            SampleInfo.strReTestOriglRUL = QString::number(pReTestResult->RLU, 'f', 0);
        }

        // 复查原始结果
        if (pReTestResult->conc >= 0. && !isCalc)
        {
            stAssayResult retestOrigResult;
            QHistorySampleAssayModel::SetAssayResultFlag(pReTestResult->conc, true, *pItem, true, retestOrigResult);
            SampleInfo.strReOriglTestResult = retestOrigResult.m_result;
        }

        // 复查完成时间
        SampleInfo.strReEndTestTime = QString::fromStdString(pReTestResult->endTime);

        // 复查机内稀释倍数
        if (tf::HisSampleType::SAMPLE_SOURCE_QC != sampleType && pReTestResult->__isset.dilutionFactor)
        {
            SampleInfo.strReDilFactor = QString::number(pReTestResult->dilutionFactor);
        }

        // 复查手工稀释倍数
        if (tf::HisSampleType::SAMPLE_SOURCE_QC != sampleType && pReTestResult->__isset.preDilutionFactor)
        {
            SampleInfo.strRePreDilFactor = QString::number(pReTestResult->preDilutionFactor);
        }
            
        // 试剂批号
        SampleInfo.strReReagentBatchNo = QString::fromStdString(pReTestResult->reagentBatchNo);
        // 试剂瓶号
        SampleInfo.strReReagentSerialNo = QString::fromStdString(pReTestResult->reagentSerialNo);
        // 底物液批号
        SampleInfo.strReSubBatchNo = QString::fromStdString(pReTestResult->supplyInfo.substrateABatchNo);
        // 底物液瓶号
        SampleInfo.strReSubBottleNo = QString::fromStdString(pReTestResult->supplyInfo.substrateASerialNo);
        // 清洗缓冲液批号
        SampleInfo.strReCleanFluidBatchNo = QString::fromStdString(pReTestResult->supplyInfo.cleanFluidBatchNo);
        // 反应杯批号
        SampleInfo.strReAssayCupBatchNo = QString::fromStdString(pReTestResult->supplyInfo.assayCupBatchNo);
        // 稀释液批号
        SampleInfo.strReDiluentBatchNo = QString::fromStdString(pReTestResult->supplyInfo.diluentBatchNo);
        // 试剂上机时间
        SampleInfo.strReRgtRegisterT = QString::fromStdString(pReTestResult->supplyInfo.reagentRegisterTime);
        // 试剂失效日期
        SampleInfo.strReReagentExpTime = QString::fromStdString(pReTestResult->supplyInfo.reagentExpTime);
        // 校准品批号
        SampleInfo.strReCaliLot = QString::fromStdString(pReTestResult->caliLot);
        // 校准曲线信息
        if (!pReTestResult->curCaliTime.empty() && tf::HisSampleType::SAMPLE_SOURCE_CL != sampleType)
        {
            SampleInfo.strReTestCurCaliTime = QString::fromStdString(pReTestResult->curCaliTime)
                + "   "
                + QString::fromStdString(pReTestResult->reagentBatchNo);
        }
    }
}

void QHistory::OnShowBySample()
{
    ui->stackedWidget->setCurrentIndex(0);
    gHisSampleAssayModel.SetSampleModuleType(QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE);

	m_selectCheckBtn->setChecked(false);
	gHisSampleAssayModel.SetOrClearAllCheck(false);
    // 设置页面允许功能状态
    PageAllowFun();
    UpdateButtonStatus();
}

void QHistory::OnShowByAssay()
{
    ui->stackedWidget->setCurrentIndex(1);
    gHisSampleAssayModel.SetSampleModuleType(QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE);

	m_selectAssayCheckBtn->setChecked(false);
	gHisSampleAssayModel.SetOrClearAllCheck(false);
    // 设置页面允许功能状态
    PageAllowFun();
    UpdateButtonStatus();
}

///
/// @brief 下一条
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
void QHistory::OnMoveBtn(bool direction)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, direction);

    bool isSampleModel = (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE);
    QTableView* pTableView = isSampleModel ? ui->sample_list : ui->show_assay_list;

    QModelIndex curModIdx = pTableView->currentIndex();
    if (!curModIdx.isValid())
    {
        ULOG(LOG_ERROR, "Select invalid item.");
        return;
    }

    // 移动到下一条数据
    auto curRow = pTableView->currentIndex().row();
    int nextIndex = -1;
    if (direction)	// 下一条
    {
        if (curRow + 1 < pTableView->model()->rowCount())
            nextIndex = curRow + 1;
    }
    else	        // 上一条
    {
        if (curRow > 0)
            nextIndex = curRow - 1;
    }
    if (nextIndex != -1)
    {
        // 按项目展示时，如果下一条是计算项目，则跳过 继续下一条
        QModelIndex curModelIdx = pTableView->model()->index(nextIndex, 1);
        pTableView->setCurrentIndex(curModelIdx);
        if (!isSampleModel)
        {
            if (gHisSampleAssayModel.IsItemCalcAssay(curModelIdx))
            {
                OnMoveBtn(direction); // TODO:把递归调用修改为重复调用
                return;
            }
        }
        OnShowSampleDetail(pTableView->currentIndex());
        // 刷新结果详情
        if (m_sampleDetailDialog != Q_NULLPTR && m_sampleDetailDialog->isVisible())
        {
            ShowSampleDetail(m_sampleDetailDialog);
        }
    }

    // 设置下一条按钮状态
    if (m_sampleDetailDialog != nullptr)
    {
        auto curRow = pTableView->currentIndex().row();
        bool existNext = (curRow + 1 < pTableView->model()->rowCount());
        m_sampleDetailDialog->SetNextBtnEnable(existNext);
        m_sampleDetailDialog->SetPreBtnEnable(curRow > 0);
    }

    UpdateButtonStatus();
}

///
/// @brief 数据改变的时候刷新页面数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月19日，新建函数
///
void QHistory::OnDataChanged()
{
    // 刷新项目
    OnShowSampleDetail(ui->sample_list->currentIndex());

    // 刷新结果详情
    if (m_sampleDetailDialog != Q_NULLPTR && m_sampleDetailDialog->isVisible())
    {
        ShowSampleDetail(m_sampleDetailDialog);
    }
}

void QHistory::OnRefreshRow(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs)
{
    OnDataChanged();
}

///
/// @brief 手工传输
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月2日，新建函数
///
void QHistory::OnTransfer()
{
    if (m_transferDialog == Q_NULLPTR)
    {
        m_transferDialog = new QPushSampleToRemote(this);
        // 传输样本到Lis
        connect(m_transferDialog, &QPushSampleToRemote::selectResultIndex, this, [&](bool reCheck, int reCheckCnt)
        {
            int32_t transType = reCheck ? tf::LisTransferType::TRANSFER_LAST : tf::LisTransferType::TRANSFER_FIRST;
            DcsControlProxy::GetInstance()->PushSampleToLIS(m_transferDialog->GetSelectSamples(), transType, true);
        });
    }

    if (gHisSampleAssayModel.GetModule() != QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
    {
        ULOG(LOG_WARN, "Not support skip, not sample browse viewmoudle.");
        return;
    }

    // 获取选中范围 
    auto selectIndexs = gHisSampleAssayModel.GetCheckedModelIndex();
    // 逐行获取要删除的样本信息的数据库主键
    std::vector<int64_t> selectdbs;
    for (const auto& index : selectIndexs)
    {
        // 如果当前行为空，则跳过
        auto sampleID = gHisSampleAssayModel.GetSampleID(index);
        if (sampleID >= 0)
        {
            selectdbs.push_back(sampleID);
        }
    }

    if (!selectdbs.empty())
    {
        m_transferDialog->SetPageAttribute(selectdbs, /*selectdbs.size() == 1 ? GetMaxTestCount(selectdbs[0]) : */-1);
        m_transferDialog->SetWorkSet(m_workSet);

        // 需求：如果只选了一个样本，又没有复查，就不弹窗，直接传输首查结果
        bool bReCheck = false;
        if (selectdbs.size() == 1)
        {
            // 获取选中的样本数据
            std::vector<std::shared_ptr<HistoryBaseDataSample>> selectedSamples;
            gHisSampleAssayModel.GetSelectedSamples(selectIndexs, selectedSamples);

            // 获取选中的样数据包含的项目数据
            std::vector<std::shared_ptr<HistoryBaseDataByItem>> itemsInSelectedSample;
            gHisSampleAssayModel.GetItemsInSelectedSamples(selectedSamples, itemsInSelectedSample);

            // 确认该样本是否有复查
            for (auto item : itemsInSelectedSample)
            {
                if (item->m_retest)
                {
                    bReCheck = true;
                    break;
                }
            }

            // 直接上传首次检查
            if (!bReCheck)
            {
                DcsControlProxy::GetInstance()->PushSampleToLIS(selectdbs, tf::LisTransferType::TRANSFER_FIRST, true);
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
            DcsControlProxy::GetInstance()->PushSampleToLIS(selectdbs, transType, true);
        }
    }
}

///
/// @brief 复查样本
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月22日，新建函数
///
void QHistory::OnRecheckSample()
{
    if (m_recheckDialog == Q_NULLPTR)
    {
        m_recheckDialog = new QReCheckDialog(this);
        m_recheckDialog->SetMode(QReCheckDialog::Mode::dilu_mode);
    }

    if (m_sampleInfo.has_value())
    {
        //m_recheckDialog->SetSample(m_sampleInfo.value().id);
        m_recheckDialog->show();
    }
}

///
/// @brief 审核样本
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月5日，新建函数
///
void QHistory::OnCheckSample()
{
    QHistorySampleAssayModel &insSmpMd = gHisSampleAssayModel;

    QModelIndexList selectIndexs = insSmpMd.GetCheckedModelIndex();
    if (selectIndexs.size() == 0)
    {
        TipDlg(tr("审核"), tr("未选中记录信息.")).exec();
        return;
    }
    // 检查是否该样本还未完成，未完成则不能审核
    QModelIndexList readyToCheckIndex;
    std::vector<int64_t> readyToCheckSampleId;
    std::vector<std::string> vecNoCkSampeName;
    for (const QModelIndex& curIdx : selectIndexs)
    {
        int64_t currSampleId = insSmpMd.GetSampleID(curIdx);
        ::tf::SampleInfoQueryCond cond;
        cond.__set_id(currSampleId);
        ::tf::SampleInfoQueryResp resp;
        if (!DcsControlProxy::GetInstance()->QuerySampleInfo(cond, resp))
        {
            continue;
        }

		// 当查找样本表未找到样本，代表原样本已经删除，此时认为样本已经测试完毕(0013545)
		if (resp.lstSampleInfos.size() == 0)
		{
			readyToCheckIndex.append(curIdx);
			readyToCheckSampleId.push_back(currSampleId);
			continue;
		}

        tf::SampleInfo& currSample = resp.lstSampleInfos[0];
        if (currSample.__isset.status && currSample.status != ::tf::TestItemStatus::TEST_ITEM_STATUS_TESTED)
        {
            vecNoCkSampeName.push_back(currSample.seqNo + "/" + currSample.barcode);
        }
        else
        {
            readyToCheckIndex.append(curIdx);
            readyToCheckSampleId.push_back(currSampleId);
        }
    }

    // 存在不满足条件的样本
    if (vecNoCkSampeName.size() > 0)
    {
        QString strSampleSeq = QString::fromStdString(GetContainerJoinString(vecNoCkSampeName, ","));
        TipDlg(tr("审核"), tr("选中的如下样本记录不满足审核条件，将不被审核.\n %1").arg(strSampleSeq)).exec();
    }
    if (readyToCheckIndex.length() == 0)
    {
        return;
    }

    if (!WorkpageCommon::ConfirmVerify(ui->flat_examine))
    {
        ULOG(LOG_INFO, "Cancel audit!");
        return;
    }
    insSmpMd.CheckSample(readyToCheckIndex);

    QHistorySampleModel_Assay::Instance().Refush();
    bool bCheck = insSmpMd.GetCheck(ui->sample_list->currentIndex());
    bCheck ? ui->flat_examine->setText(m_sUnCheckBtnText) : ui->flat_examine->setText(m_sCheckBtnText);

    POST_MESSAGE(MSG_ID_SAMPLE_AUDIT_UPDATE_HIS, readyToCheckSampleId, bCheck);

    // 审核后会影响打印按钮使能，因此需要重设按钮使能
	UpdateButtonStatus();
}

///
/// @brief 获取样本中项目测试次数最大值（可能样本中某个项目比其它项目测试次数更多）
///
/// @param[in]  sampleId  
///
/// @return 返回最多的次数
///
/// @par History:
/// @li 8090/YeHuaNing，2023年3月14日，新建函数
///
int QHistory::GetMaxTestCount(int64_t sampleId)
{
    // 样本中，项目测试次数
    int maxRecheckedIndex = 0;

    auto selectedIndexs = ui->sample_list->selectionModel()->selectedRows();
    if (selectedIndexs.isEmpty())
    {
        return -1;
    }
    std::vector<std::shared_ptr<HistoryBaseDataSample>> selectedSamples;
    gHisSampleAssayModel.GetSelectedSamples(selectedIndexs.mid(0, 1), selectedSamples);
    std::vector<std::shared_ptr<HistoryBaseDataByItem>> itemsInSelectedSample;
    gHisSampleAssayModel.GetItemsInSelectedSamples(selectedSamples, itemsInSelectedSample);
    // 这里演示打印数据的构造
    for (auto item : itemsInSelectedSample)
    {
        switch (CommonInformationManager::GetAssayClassify(item->m_assayCode))
        {
            // 计算项目
        case tf::AssayClassify::ASSAY_CLASSIFY_OTHER:
        {
            ::tf::CalcAssayTestResultQueryResp assayTestResult;
            ::tf::CalcAssayTestResultQueryCond queryAssaycond;
            queryAssaycond.__set_testItemId(item->m_itemId);

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
            queryAssaycond.__set_testItemId(item->m_itemId);

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
            queryAssaycond.__set_testItemId(item->m_itemId);

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
            queryAssaycond.__set_testItemId(item->m_itemId);

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
/// @brief 清除当前筛选
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年3月16日，新建函数
///
void QHistory::OnCloseFilter()
{
    // 当前窗口切换时不清除
    if (this->isHidden())
    {
        return;
    }

    // 请除快捷筛选内容
    m_lastFilterObj = nullptr;
    gHisSampleAssayModel.Clear();
    QHistorySampleModel_Assay::Instance().Clear();
	gHisSampleAssayModel.SetOrClearAllCheck(false);
    // 取消快捷筛选按钮的check状态
    UnCheckedFastFilterBtn();
	UpdateButtonStatus();
	UpdateCheckAllBoxStatus();
}

///
/// @brief 处理样本审核状态改变消息
///
/// @param[in]  vecSampleID		发生改变的样本ID
/// @param[in]  bStatus			发生改变的样本当前状态
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年5月29日，新建函数
///
void QHistory::OnAuditChanged(class std::vector<__int64, class std::allocator<__int64> > vecSampleID, bool bStatus)
{
	// 只处理审核状态，防止上传两次
    gHisSampleAssayModel.CheckSampleBySampleID(vecSampleID, bStatus,false);
}

///
/// @brief 更新工作页面的按钮状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月15日，新建函数
///
void QHistory::OnUpdateButtonStatus()
{
    if (!DictionaryQueryManager::GetPageset(m_workSet))
    {
        ULOG(LOG_ERROR, "Failed to get workset.");
        return;
    }

	if (m_statusShow != Q_NULLPTR)
	{
		QString tipsData = m_pCommAssayMgr->GetTipsContent(m_workSet.aiRecognition);
		m_statusShow->setToolTip(tipsData);
	}

	if (m_assayButton != Q_NULLPTR)
	{
		QString tipsData = m_pCommAssayMgr->GetTipsContent(m_workSet.aiRecognition);
		m_assayButton->setToolTip(tipsData);
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

    // 此处0614版本特殊处理（上市），后续需要修改权限表，普通用户不显示导出按钮
    bool bUserCanExport = true;
    std::shared_ptr<tf::UserInfo> spLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if ((spLoginUserInfo != Q_NULLPTR) && (spLoginUserInfo->type == tf::UserType::USER_TYPE_GENERAL))
    {
        bUserCanExport = false;
    }

    fSetVisable(ui->selectCheck, m_workSet.filter);
    ui->widget_2->setVisible(m_workSet.filter);
    fSetVisable(ui->result_detail_btn, m_workSet.detail);
    fSetVisable(ui->flat_examine, m_workSet.audit);
    fSetVisable(ui->flat_print, m_workSet.print);
    fSetVisable(ui->flat_manual_transfer, m_workSet.transfer);
    fSetVisable(ui->export_btn, m_workSet.exported  && bUserCanExport);
    fSetVisable(ui->patient_btn, m_workSet.patient);
    fSetVisable(ui->flat_recheck, m_workSet.recheck);
    fSetVisable(ui->del_Button, m_workSet.deleteData);
    fSetVisable(ui->flat_recalculate, m_workSet.reComput);

    PageAllowFun();

    if (m_filterDialog != nullptr)
    {
        m_filterDialog->UpdateButtonStatus(m_workSet);
    }
}

///
/// @brief 处理样本上传Lis状态改变消息
///
/// @param[in]  sampleID  样本ID
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年5月30日，新建函数
///
void QHistory::OnLisChanged(int64_t sampleID)
{
    gHisSampleAssayModel.UpdateLisStatus(QVector<int64_t> {sampleID});
}

///
/// @brief 处理复查结果更新消息
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月14日，新建函数
///
void QHistory::OnRetestResultChanged(::tf::HistoryBaseDataUserUseRetestUpdate ri)
{
    ::tf::ResultLong _return;
    auto bRet = DcsControlProxy::GetInstance()->UpdateHistoryBaseDataUserUseRetestInfo(_return, ri);
    if (!bRet)
    {
        ULOG(LOG_ERROR, "OnRetestResultChanged Failed");
    }
    else
    {
        OnUIRetestResultChanged(std::tuple<std::string, std::string, int64_t>{ri.reTestResult, ri.reResultStatu, ri.userUseRetestResultId});
    }
}

///
/// @brief 处理复查结果更新消息
///  
/// @param[in]  vol  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月14日，新建函数
///
void QHistory::OnUIRetestResultChanged(std::tuple<std::string, std::string, int64_t> vol)
{
    if (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::SAMPLEBROWSE)
    {
        QHistorySampleModel_Assay::Instance().Refush();
    }
    else if (gHisSampleAssayModel.GetModule() == QHistorySampleAssayModel::VIEWMOUDLE::ASSAYBROWSE)
    {
        auto index = ui->show_assay_list->currentIndex();
        gHisSampleAssayModel.UpdateRetestResult(index, vol);
    }
}
