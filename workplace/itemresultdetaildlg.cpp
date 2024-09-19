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
/// @file     itemresultdetaildlg.cpp
/// @brief    结果详情对话框
///
/// @author   4170/TangChuXian
/// @date     2022年10月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "itemresultdetaildlg.h"
#include "ui_QHistoryItemResultDetailDlg.h"

#include "imassayresultinfowidget.h"
#include "ui_imassayresultinfowidget.h"
#include "QAssayResultView.h"
#include "QSampleAssayModel.h"

#include "src/common/StringUtil.h"
#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/datetimefmttool.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "QHistoryAIIdentifyDlg.h"
#include "CReadOnlyDelegate.h"
#include "manager/OperationLogManager.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "WorkpageCommon.h"

#include <QKeyEvent>


#define ROW_CNT_OF_ASSAY_RESULT_TABLE                        (44)         // 项目结果表行数
#define COL_CNT_OF_ASSAY_RESULT_TABLE                        (3)          // 项目结果表列数

#define  STACK_WIDGET_INDEX_CH                               (0)          // 生化项目详情界面索引
#define  STACK_WIDGET_INDEX_IM                               (1)          // 免疫项目详情界面索引

#define  TAB_WIDGET_INDEX_FIRST_RLT                          (0)          // 初测结果详情界面tab索引
#define  TAB_WIDGET_INDEX_LAST_RLT                           (1)          // 复查结果详情界面tab索引

#define  COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE           (0)          // 项目名所在列索引
#define  COL_INDEX_FIRST_RESULT_OF_ITEM_RESULT_TABLE         (1)          // 初测结果所在列索引
#define  COL_INDEX_LAST_RESULT_OF_ITEM_RESULT_TABLE          (2)          // 复查结果所在列索引

Q_DECLARE_METATYPE(tf::TestItem)
Q_DECLARE_METATYPE(im::tf::AssayTestResult)
Q_DECLARE_METATYPE(ch::tf::AssayTestResult)
Q_DECLARE_METATYPE(ise::tf::AssayTestResult)
Q_DECLARE_METATYPE(::tf::HistoryBaseDataUserUseRetestUpdate)

ItemResultDetailDlg::ItemResultDetailDlg(QWidget *parent)
    : BaseDlg(parent),
    m_lSampId(-1),
    m_lItemId(-1),
    m_actionDiglog(Q_NULLPTR),
	m_modifyData(false),
    m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化UI对象
    ui = new Ui::QHistoryItemResultDetailDlg();

    ui->setupUi(this);
    SetTitleName(tr("结果详情"));
    // 界面显示前初始化
    InitBeforeShow();
}

ItemResultDetailDlg::~ItemResultDetailDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief 设置需要显示的样本信息和项目信息
///
/// @param[in]  sampId      样本信息的主键
/// @param[in]  testItems   项目信息的主键列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
void ItemResultDetailDlg::SetSampleId(int64_t sampId, std::vector<int64_t>& testItems, bool isFirst, bool isLast)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->PrevBtn->setEnabled(true);
    ui->NextBtn->setEnabled(true);
    ClearContents();

    // 若是第一
    if (isFirst)
    {
        ui->PrevBtn->setDisabled(true);
    }

    // 若是最后一个
    if (isLast)
    {
        ui->NextBtn->setDisabled(true);
    }

    m_testItems = testItems;
	if (m_lSampId != sampId)
	{
		m_curRow = boost::none;
		m_curColumn = boost::none;
	}

    m_lSampId = sampId;
    UpdateSampleInfo();
}

///
/// @brief
///     清空内容
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，新建函数
///
void ItemResultDetailDlg::ClearContents()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空样本结果表
    ui->AssayResTbl->clearContents();

    // 清空病人信息
    ClearPatientInfo();
}

///
/// @brief 按键事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 5774/WuHongTao，2023年7月3日，新建函数
///
void ItemResultDetailDlg::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        return;
    }
}

///
/// @brief
///     清空选中生化项目项目详情信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月20日，新建函数
///
void ItemResultDetailDlg::ClearSelChItemDetail()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取生化tab页面
	RFlagTabWidget* pChTabWgt = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
    if (pChTabWgt == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s()", __FUNCTION__);
        return;
    }

    pChTabWgt->Clear();
}

///
/// @brief
///     清空选中生化项目项目详情信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月20日，新建函数
///
void ItemResultDetailDlg::ClearSelImItemDetail()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	auto FuncClearItemDeltail = [this](int tabIdx) {
		RFlagTabWidget* pChTabWgt = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(tabIdx));
		if (pChTabWgt == Q_NULLPTR)
		{
			ULOG(LOG_WARN, "Not exist tabwidget.");
			return;
		}
		pChTabWgt->Clear();
	};

	// 清空选中免疫项目项目详情信息
	FuncClearItemDeltail(STACK_WIDGET_INDEX_IM);
}

///
/// @brief
///     清空选中项目病人信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月20日，新建函数
///
void ItemResultDetailDlg::ClearPatientInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空病人信息
    ui->label_6->clear();
    ui->label_endtime->clear();
    ui->NameValLab->clear();
    ui->GenderValLab->clear();
    ui->AgeValLab->clear();
    ui->BarcodeValLab->clear();
    ui->SampNoValLab->clear();
    ui->CaseNoValLab->clear();
    //ui->BedNoValLab->clear();
    ui->DepartmentValLab->clear();
    ui->VisitingDoctorValLab->clear();
    ui->DiagNosisValLab->clear();
    ui->RemarkValLab->clear();
    ui->label_2->clear();
    ui->label_4->clear();
}

bool ItemResultDetailDlg::eventFilter(QObject* target, QEvent* event)
{
	return BaseDlg::eventFilter(target, event);
	// qtabview和focusOut同时满足
	if (target == ui->AssayResTbl->viewport())
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent != Q_NULLPTR)
			{
				auto index = ui->AssayResTbl->indexAt(mouseEvent->pos());
				if (index.isValid())
				{
					m_curColumn = 0;
					OnItemTblCurIndexChanged(index);
					if (m_sampleInfo.__isset.audit)
					{
						ui->DelBtn->setEnabled(!m_sampleInfo.audit && m_delete);
					}
				}
			}
		}
	}

	return BaseDlg::eventFilter(target, event);
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void ItemResultDetailDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	m_curRow = boost::none;
	m_curColumn = boost::none;
    // 插入生化项目详情信息
    RFlagTabWidget* pChTabWgt = new RFlagTabWidget();
	connect(pChTabWgt, &RFlagTabWidget::SignalCurrentTabIsRFlag, this, [&](bool d) {
		ui->button_setRetest->setEnabled(!d && m_reset);
	});
    ui->AssayDetailStkWgt->insertWidget(STACK_WIDGET_INDEX_CH, pChTabWgt);
	connect(pChTabWgt, &QTabWidget::tabBarClicked, this, [&](int index){
		ui->SaveBtn->setEnabled(false);
		auto tabWidget = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
		if (tabWidget == Q_NULLPTR)
		{
			return;
		}
		m_curColumn = index;

		// 若页面编号是0-表示初测
		if (index == 0)
		{
			ui->button_setRetest->setEnabled(false);
			return;
		}

		auto widget = tabWidget->widget(index);
		if (widget == Q_NULLPTR)
		{
			return;
		}

		auto currentView = qobject_cast<QAssayResultView*>(widget);
		if (currentView == Q_NULLPTR)
		{
			return;
		}

		if (currentView->GetIsReset())
		{
			tabWidget->SetFlag(std::vector<int>{index});
			ui->button_setRetest->setEnabled(false);
		}
		else
		{
			if (m_sampleInfo.__isset.audit)
			{
				ui->button_setRetest->setEnabled(!m_sampleInfo.audit && m_reset);
			}
		}
	});

	// bug0011297
	// 联机的情况下，AI打开才显示
	bool isAi =	QDataItemInfo::Instance().IsAiRecognition();
	// 是否联机
	bool isPipel = DictionaryQueryManager::GetInstance()->GetPipeLine();
	if (isAi && isPipel)
	{
		ui->button_AI->setVisible(true);
	}
	else
	{
		ui->button_AI->setVisible(false);
	}

    // 插入免疫项目详情信息
    RFlagTabWidget* pImTabWgt = new RFlagTabWidget();
    pImTabWgt->setElideMode(Qt::TextElideMode::ElideRight);
	connect(pImTabWgt, &RFlagTabWidget::SignalCurrentTabIsRFlag, this, [&](bool d) {
		ui->button_setRetest->setEnabled(!d && m_reset);
	});
    pImTabWgt->AddTab(new ImAssayResultInfoWidget(), tr("检测结果"));
    pImTabWgt->AddTab(new ImAssayResultInfoWidget(), tr("复查结果"));
    ui->AssayDetailStkWgt->insertWidget(STACK_WIDGET_INDEX_IM, pImTabWgt);

    // 默认显示生化
    ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_CH);

	// zhu
	ui->AssayResTbl->viewport()->installEventFilter(this);
    // 初始化项目表
    ui->AssayResTbl->setRowCount(ROW_CNT_OF_ASSAY_RESULT_TABLE);
	// 默认列数为3，modifybug0011257 by wuht
    ui->AssayResTbl->setColumnCount(COL_CNT_OF_ASSAY_RESULT_TABLE);
    ui->AssayResTbl->verticalHeader()->setVisible(false);
    ui->AssayResTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // 清空信息
    ClearContents();
    connect(ui->button_setRetest, &QPushButton::clicked, this, &ItemResultDetailDlg::OnSetCurrentRetestResult);

    // 数据浏览结果详情界面无AI识别按钮
    ui->button_AI->setHidden(true);
}

void ItemResultDetailDlg::CutTipstring(const QString& src, QString& dst, int width)
{
	if (width <= 0)
	{
		dst = src;
		return;
	}

	QFont font("Source Han Sans CN Regular");
	font.setPixelSize(16);
	QFontMetrics fm(font);
	int currentWidth = 0;
    int row = 0;
	for (auto& data : src)
	{
		int charWidth = fm.width(data);
        if ((currentWidth + charWidth) > width)
        {
            row++;
            if (row > 1)
            {
                dst += "...";
                break;
            }
			dst += "\n";
			currentWidth = 0;           
		}

		currentWidth += charWidth;
		dst += data;
	}

	if (dst.endsWith("\n"))
	{
		dst.chop(1);
	}
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void ItemResultDetailDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void ItemResultDetailDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置表头
    QStringList strHeaderList;
    strHeaderList << tr("项目名称") << tr("检测结果") << tr("复查结果");
    ui->AssayResTbl->setHorizontalHeaderLabels(strHeaderList);

    // 设置表格选中模式为行选中，不可多选，不可编辑
    ui->AssayResTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->AssayResTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->AssayResTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void ItemResultDetailDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 上一条按钮被点击
    connect(ui->PrevBtn, SIGNAL(clicked()), this, SIGNAL(SigPrevBtnClicked()));

    // 下一条按钮被点击
    connect(ui->NextBtn, SIGNAL(clicked()), this, SIGNAL(SigNextBtnClicked()));

    // 删除按钮被点击
    connect(ui->DelBtn, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));

    // 保存按钮被点击
	// ui->SaveBtn->setEnabled(false);
    connect(ui->SaveBtn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    // 恢复图例
    connect(ui->reset_curve, SIGNAL(clicked()), this, SLOT(OnResetCurve()));

    // 视觉识别结果
    connect(ui->button_AI, SIGNAL(clicked()), this, SLOT(OnAIIdentify()));

    // 项目结果表选中项改变(bug3151)
    QItemSelectionModel* selItemModel = ui->AssayResTbl->selectionModel();
    if (selItemModel != Q_NULLPTR)
    {
        // 项目信息更新
        connect(selItemModel, SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(OnItemTblCurIndexChanged(const QModelIndex&)));
    }
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
QString ItemResultDetailDlg::GetItemResult(const tf::TestResultKeyInfo& stuResultKey, QVariant* pVarRlt /*= Q_NULLPTR*/)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 构造项目结果保存临时变量
    QVariant varRltTemp;
    if (pVarRlt == Q_NULLPTR)
    {
        pVarRlt = &varRltTemp;
    }

	// 判断显示浓度还是吸光度
	SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	bool isShowConc = WorkpageCommon::IsShowConc(sampleSet, m_sampleInfo.sampleType);

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
            ULOG(LOG_ERROR, "Failed to query assaytestresult by id:%lld.", stuResultKey.assayTestResultId);
            return QString("");
        }

        auto result = assayTestResult.lstAssayTestResult[0];
		QString data;
		// 显示浓度
		if (isShowConc)
		{
			data = CommonInformationManager::GetInstance()->GetImDisplyResultStr(result.assayCode, result);
		}
		// 显示吸光度
		else if(result.__isset.RLU)
		{
			data = QString::number(result.RLU);
		}

        // 保存结果信息
        pVarRlt->setValue<im::tf::AssayTestResult>(result);

        return data;
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
            ULOG(LOG_ERROR, "Failed to query assaytestresult by id:%lld.", stuResultKey.assayTestResultId);
            return QString("");
        }

        // 保存结果信息
        pVarRlt->setValue<ch::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);
        auto result = assayTestResult.lstAssayTestResult[0];
        int left = 2;
		// 单位倍率
		double factor = 1.0;
        auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(result.assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            left = spAssayInfo->decimalPlace;
			for (const auto& unit : spAssayInfo->units)
			{
				if (unit.isCurrent)
				{
					factor = unit.factor;
                    break;
                }
			}
        }

        auto concRaw = QString::number(result.conc*factor, 'f', left);
        QString editConcData = QString::fromStdString(result.concEdit);
        double editConc;
        if (stringutil::IsDouble(result.concEdit, editConc))
        {
			editConcData = "*";
            editConcData += QString::number(editConc*factor, 'f', left);
        }

		QString data;
		// 显示浓度
		if (isShowConc)
		{
			data = (result.__isset.concEdit && !result.concEdit.empty()) ? editConcData : concRaw;
		}
		// 显示吸光度
		else if(result.__isset.abs)
		{
			data = QString::number(result.abs);
		}

        return data;
    }

    if (stuResultKey.deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
    {
        ULOG(LOG_INFO, "%s(), stuResultKey.deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005", __FUNCTION__);
        // 构造查询条件
        ise::tf::AssayTestResultQueryCond queryAssaycond;
        ise::tf::AssayTestResultQueryResp assayTestResult;
        queryAssaycond.__set_id(stuResultKey.assayTestResultId);

        if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
            || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || assayTestResult.lstAssayTestResult.empty())
        {
            ULOG(LOG_ERROR, "Failed to query assaytestresult by id:%lld.", stuResultKey.assayTestResultId);
            return QString("");
        }

        pVarRlt->setValue<ise::tf::AssayTestResult>(assayTestResult.lstAssayTestResult[0]);

        auto result = assayTestResult.lstAssayTestResult[0];
        int left = 2;
		// 单位倍率
		double factor = 1.0;
        auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(result.assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            left = spAssayInfo->decimalPlace;
			for (const auto& unit : spAssayInfo->units)
			{
				if (unit.isCurrent)
				{
					factor = unit.factor;
                    break;
                }
			}
        }

        auto concRaw = QString::number(result.conc*factor, 'f', left);
        QString editConcData = QString::fromStdString(result.concEdit);
        double editConc;
        if (stringutil::IsDouble(result.concEdit, editConc))
        {
			editConcData = "*";
            editConcData += QString::number(editConc*factor, 'f', left);
        }

		QString data;
		// 显示浓度
		if (isShowConc)
		{
			data = (result.__isset.concEdit && !result.concEdit.empty()) ? editConcData : concRaw;
		}
		// 显示吸光度
		else if(result.__isset.testEMF)
		{
			data = QString::number(result.testEMF.sampleEMF, 'f', left) 
				+ "/"+ QString::number(result.testEMF.baseEMF, 'f', left);
		}

		return data;

    }

    return QString("");
}

///
/// @brief
///     更新选中生化项目详情信息
///
/// @param[in]  curIndex        选中项索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月21日，新建函数
///
void ItemResultDetailDlg::UpdateSelChItemDetailInfo(const QModelIndex& curIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_pChTabWidget = Q_NULLPTR;
	// 当修改结果或者设置复查的时候不需要再次刷新结果详情
	if (m_modifyData)
	{
		m_modifyData = false;
		return;
	}

    // 检查参数
    if (!curIndex.isValid() || (ui->AssayResTbl->item(curIndex.row(), COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE) == Q_NULLPTR))
    {
        // 如果选中索引无效或选中空行，清空选中项信息
        ULOG(LOG_WARN, "Invalid sel index or empty row.");
        return;
    }

    // 获取选项
    auto item = ui->AssayResTbl->item(curIndex.row(), COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE);
    if (item == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null item by row %d.", curIndex.row());
        return;
    }
	m_curRow = curIndex.row();
    auto stuTestItem = item->data(Qt::UserRole).value<tf::TestItem>();
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuTestItem.assayCode);
    if (nullptr == spAssayInfo)
    {
        ULOG(LOG_WARN, "Not eixst generalassyainfo, assayCode:%d.", stuTestItem.assayCode);
        return;
    }

    // 判断是否ISE
    auto IsISE = [&](int assayCode)->bool
    {
        if (assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_NA
            || assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_K
            || assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
        {
            return true;
        }

        return false;
    };

    // 若是ISE项目
    if (IsISE(stuTestItem.assayCode))
    {
		ui->reset_curve->setVisible(false);
        ise::tf::AssayTestResultQueryCond queryAssaycond;
        ise::tf::AssayTestResultQueryResp assayTestResult;
        // 根据测试项目的ID来查询结果
        queryAssaycond.__set_testItemId(stuTestItem.id);
		queryAssaycond.__set_assayCode(stuTestItem.assayCode);

        if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
            || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
            /*|| assayTestResult.lstAssayTestResult.empty()*/)
        {
            ULOG(LOG_ERROR, "Failed query ise testresult, by id:%lld assayCode:%d.", stuTestItem.id, stuTestItem.assayCode);
            return;
        }

        auto pChTabWgt = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
        if (pChTabWgt == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "%s(), pImTabWgt == Q_NULLPTR,", __FUNCTION__);
            return;
        }

		// 根据需求生成对应的样本bug12904
		auto iseResultVec = GennerateAssayResult(assayTestResult.lstAssayTestResult);

		pChTabWgt->Clear();
		// 依次显示结果
		int index = 1;
		QString TableName(tr("检测结果"));
        for (const auto& assayResult : iseResultVec)
        {
			if (!assayResult.__isset.endTime)
			{
				continue;
			}

			QAssayResultView* actionDiglog = qobject_cast<QAssayResultView*>(pChTabWgt->widget(index -1));
			if (actionDiglog == Q_NULLPTR)
			{
				actionDiglog = new QAssayResultView(this);
				connect(actionDiglog, &QAssayResultView::dataChanged, this, [&]()
				{
					m_modifyData = true;
					emit dataChanged();
				});
				// modify bug0012697 by wuht
				connect(actionDiglog, &QAssayResultView::resultChanged, this, [&]()
				{
					ui->SaveBtn->setEnabled(true);
				});

				pChTabWgt->AddTab(actionDiglog, TableName);
			}

			actionDiglog->SetSampleInfo(m_sampleInfo);
			actionDiglog->SetIseAssayResult(assayResult);
			actionDiglog->ResetCurve();
			actionDiglog->SetSampleInfo(m_sampleInfo);
			actionDiglog->SetCurrentTestItem(stuTestItem);


			if (actionDiglog->GetIsReset() && TableName != tr("检测结果"))
			{
				pChTabWgt->SetFlag(std::vector<int>{index - 1});
			}

			// modify bug3302 by wuhongtao 
            TableName = tr("复查结果") + QString::number(index++);
        }

		std::string endTimeStr;
		std::for_each(assayTestResult.lstAssayTestResult.cbegin(), assayTestResult.lstAssayTestResult.cend(), [&endTimeStr](const ise::tf::AssayTestResult& result)
		{
			if (endTimeStr < result.endTime)
			{
				endTimeStr = result.endTime;
			}
		});
		ui->label_endtime->setText(ToCfgFmtDateTime(QString::fromStdString(endTimeStr)));

		if (m_curColumn)
		{
			int column = m_curColumn.value();
			if (column < 0 || column >= pChTabWgt->count())
			{
				ui->button_setRetest->setEnabled(false);
				return;
			}

			pChTabWgt->setCurrentIndex(column);
            ui->button_setRetest->setEnabled(column == 0 ? false : m_reset);
		}
		else
		{
			ui->button_setRetest->setEnabled(false);
		}

		QAssayResultView* actionDiglog = qobject_cast<QAssayResultView*>(pChTabWgt->currentWidget());
		if (actionDiglog == Q_NULLPTR)
		{
			return;
		}

		// 当前页面是复查页面则置灰
		if (actionDiglog->GetIsReset())
		{
			ui->button_setRetest->setEnabled(false);
		}
    }
    else
    {
		ui->reset_curve->setVisible(true);
        // 判断项目的设备类型
        if (spAssayInfo->assayClassify != tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
        {
            ULOG(LOG_WARN, "%s(), device type is not matched,", __FUNCTION__);
            return;
        }

        // 生化项目
        auto pChTabWgt = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
        if (pChTabWgt == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "%s(), pImTabWgt == Q_NULLPTR,", __FUNCTION__);
            return;
        }

        ch::tf::AssayTestResultQueryCond queryAssaycond;
        ch::tf::AssayTestResultQueryResp assayTestResult;
        // 根据测试项目的ID来查询结果
        queryAssaycond.__set_testItemId(stuTestItem.id);
		queryAssaycond.__set_assayCode(stuTestItem.assayCode);

        if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
            || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            ULOG(LOG_ERROR, "Failed to query ch testresult, by id:%lld, assayCode:%d.", stuTestItem.id, stuTestItem.assayCode);
            return;
        }

		// 根据需求生成对应的样本bug12904
		auto chResultVec = GennerateAssayResult(assayTestResult.lstAssayTestResult);
		pChTabWgt->Clear();
        // 依次显示结果
        int index = 1;
        QString TableName(tr("检测结果"));
        for (const auto& assayResult : chResultVec)
        {
			if (!assayResult.__isset.endTime)
			{
				continue;
			}

			bool isNeedAdd = false;
			QAssayResultView* actionDiglog = qobject_cast<QAssayResultView*>(pChTabWgt->widget(index - 1));
			if (actionDiglog == Q_NULLPTR)
			{
				actionDiglog = new QAssayResultView(pChTabWgt);
				//connect(actionDiglog, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
				connect(actionDiglog, &QAssayResultView::dataChanged, this, [&]()
				{
					m_modifyData = true;
					emit dataChanged();
				});
				connect(actionDiglog, &QAssayResultView::resultChanged, this, [&]()
				{
					ui->SaveBtn->setEnabled(true);
				});

				isNeedAdd = true;
			}

			actionDiglog->SetSampleInfo(m_sampleInfo);
			actionDiglog->SetAssayResult(assayResult.id, stuTestItem);
			actionDiglog->ResetCurve();
			// modify bug0011173 by wuht
			actionDiglog->SetSampleInfo(m_sampleInfo);
			actionDiglog->SetCurrentTestItem(stuTestItem);
			// 初测结果不能显示
			if (actionDiglog->GetIsReset() && TableName != tr("检测结果"))
			{
				pChTabWgt->SetFlag(std::vector<int>{index - 1});
			}

			if (isNeedAdd)
			{
				pChTabWgt->AddTab(actionDiglog, TableName);
			}

			// modify bug3302 by wuhongtao 
            TableName = tr("复查结果") + QString::number(index++);
        }

		std::string endTimeStr;
		std::for_each(assayTestResult.lstAssayTestResult.cbegin(), assayTestResult.lstAssayTestResult.cend(), [&endTimeStr](const ch::tf::AssayTestResult& result)
		{
			if (endTimeStr < result.endTime)
			{
				endTimeStr = result.endTime;
			}
		});
		ui->label_endtime->setText(ToCfgFmtDateTime(QString::fromStdString(endTimeStr)));

		if (m_curColumn)
		{
			int column = m_curColumn.value();
			if (column < 0 || column >= pChTabWgt->count())
			{
				ui->button_setRetest->setEnabled(false);
				return;
			}

			pChTabWgt->setCurrentIndex(column);
			if (column == 0)
			{
				ui->button_setRetest->setEnabled(false);
			}
			else
			{
				ui->button_setRetest->setEnabled(m_reset);
			}
		}
		else
		{
			ui->button_setRetest->setEnabled(false);
		}

		QAssayResultView* actionDiglog = qobject_cast<QAssayResultView*>(pChTabWgt->currentWidget());
		if (actionDiglog == Q_NULLPTR)
		{
			return;
		}

		// 当前页面是复查页面则置灰
		if (actionDiglog->GetIsReset())
		{
			ui->button_setRetest->setEnabled(false);
		}
    }
}

void ItemResultDetailDlg::ShowTestResultItem(const std::vector<im::tf::AssayTestResult>& resultItems, 
    const tf::TestItem& stuTestItem, bool bVolEditEnable)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto pImTabWgt = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_IM));
    if (pImTabWgt == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "Null im tab widget at tab index %d.", STACK_WIDGET_INDEX_IM);
        return;
    }
    int iCurIndx = pImTabWgt->currentIndex();

    pImTabWgt->Clear();
    // 无正常结果保存按钮不可用bug28790
    if (resultItems.size() > 0 && resultItems[0].id > 0)
    {
        ui->SaveBtn->setEnabled(true);
    }   
    std::vector<int> vecFlag;

    for (int index = 0; index < resultItems.size(); ++index)
    {
        auto wgt = new ImAssayResultInfoWidget();
        connect(wgt, &ImAssayResultInfoWidget::dataChanged, this, [&]()
        {
            m_modifyData = true;
            emit dataChanged();
        });
        QString title;
        if (index == 0)
        {
            title = tr("检测结果");
        }
        else
        {
            title = (index > 0) ? (tr("复查结果") + QString::number(index)) : tr("复查结果");
            if (vecFlag.empty() && resultItems[index].id == stuTestItem.lastTestResultKey.assayTestResultId)
            {
                vecFlag.push_back(index);
            }
        }
        bool bIsFirst = index == 0 ? true : false;
        bool bIsLastReTest = (index == resultItems.size() - 1 && index > 0) ? true : false;

        // 病人样本可编辑，校准、质控不可编辑
        bVolEditEnable = resultItems.at(index).sampType == ::tf::SampleType::SAMPLE_TYPE_PATIENT ? bVolEditEnable : false;
        wgt->UpdateDetailData(resultItems.at(index), bVolEditEnable, bIsFirst, bIsLastReTest);
        wgt->SetCurrentTestItem(stuTestItem);
        pImTabWgt->AddTab(wgt, title);
    }

    std::string endTimeStr;
    std::for_each(resultItems.cbegin(), resultItems.cend(), [&endTimeStr](const im::tf::AssayTestResult& result)
    {
        if (endTimeStr < result.endTime)
        {
            endTimeStr = result.endTime;
        }
    });
    ui->label_endtime->setText(ToCfgFmtDateTime(QString::fromStdString(endTimeStr)));

    pImTabWgt->SetFlag(vecFlag);
    pImTabWgt->setCurrentIndex(iCurIndx);
}

///
/// @brief
///     更新选中免疫项目详情信息
///
/// @param[in]  curIndex        选中项索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月21日，新建函数
///
void ItemResultDetailDlg::UpdateSelImItemDetailInfo(const QModelIndex& curIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 检查参数; 如果选中索引无效或选中空行，清空选中项信息
    if (!curIndex.isValid() || (ui->AssayResTbl->item(curIndex.row(), COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE) == Q_NULLPTR))
    {
        ULOG(LOG_WARN, "Sel invalid index or empty row.");
        return;
    }
    m_curRow = curIndex.row();

    // 获取选项
    auto item = ui->AssayResTbl->item(curIndex.row(), COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE);
    if (item == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null item cell at row:%d, col:%d.", curIndex.row(), COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE);
        return;
    }

    // 获取当前选中项目的详情信息
    auto stuTestItem = item->data(Qt::UserRole).value<tf::TestItem>();

    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuTestItem.assayCode);
    if (Q_NULLPTR == spAssayInfo)
    {
        ULOG(LOG_WARN, "Faild to take generalassayinfo from commrg, assayCode:%d.", stuTestItem.assayCode);
        return;
    }

    // 判断项目的设备类型; 生化项目在此流程非法
    if (spAssayInfo->assayClassify != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        ULOG(LOG_WARN, "Invalid assay type, %d.", spAssayInfo->assayClassify);
        return;
    }

    // 查询样本审核状态
    bool bVolEditEnable = false;
    {
        ::tf::SampleInfoQueryCond sampQryCond;
        ::tf::SampleInfoQueryResp sampQryResp;
        sampQryCond.__set_id(stuTestItem.sampleInfoId);
        // 执行查询条件
        if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
            || sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || sampQryResp.lstSampleInfos.empty())
        {
            ULOG(LOG_WARN, "Failed to query resultitem by id:%lld.", stuTestItem.sampleInfoId);
            return;
        }
		// 审核后不可编辑
        bVolEditEnable = !sampQryResp.lstSampleInfos[0].audit;
    }

    // 根据测试项目的ID来查询结果
    im::tf::AssayTestResultQueryCond queryAssaycond;
    im::tf::AssayTestResultQueryResp assayTestResult;
    queryAssaycond.__set_testItemId(stuTestItem.id);

    im::tf::AssayTestResult nullItem;
    nullItem.__set_dilutionFactor(stuTestItem.dilutionFactor > 1?stuTestItem.dilutionFactor:1);
    nullItem.__set_preDilutionFactor(stuTestItem.preDilutionFactor > 1?stuTestItem.preDilutionFactor:1);

    if (!im::i6000::LogicControlProxy::QueryAssayTestResult(assayTestResult, queryAssaycond)
        || assayTestResult.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || assayTestResult.lstAssayTestResult.empty())
    {
        ShowTestResultItem({ nullItem }, stuTestItem, bVolEditEnable); // 显示空白信息页面
        ULOG(LOG_WARN, "Failed query im resultitem by id:%lld.", stuTestItem.id);
        return;
    }

	// 当存在多次测试时，需要去除无结果的测试记录 add by wzx 20240402
	if (assayTestResult.lstAssayTestResult.size() > 1)
	{
		for (auto it = assayTestResult.lstAssayTestResult.begin(); it != assayTestResult.lstAssayTestResult.end(); )
		{
			if (it->conc < 0)
			{
				it = assayTestResult.lstAssayTestResult.erase(it);
			}
			else
			{
				it++;
			}

            // 如果只剩余一个，则不管是否存在结果，均保留，避免空白页面显示
            if (assayTestResult.lstAssayTestResult.size() == 1)
            {
                break;
            }
		}
	}

    // 增加结果详细显示控件并进行显示
    ShowTestResultItem(assayTestResult.lstAssayTestResult, stuTestItem, bVolEditEnable);
}

///
/// @brief
///     更新选中项目详情信息
///
/// @param[in]  curIndex        选中项索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月21日，新建函数
///
void ItemResultDetailDlg::UpdateSelItemDetailInfo(const QModelIndex& curIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const int iColItemTableIdx = COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE;
    // 检查参数
    if (!curIndex.isValid() || (ui->AssayResTbl->item(curIndex.row(), iColItemTableIdx) == Q_NULLPTR))
    {
        // 如果选中索引无效或选中空行，清空选中项信息
        ULOG(LOG_WARN, "Invalid sel index or empty row.");
        return;
    }

    // 获取当前选中项目的详情信息
    auto TestItem = ui->AssayResTbl->item(curIndex.row(), iColItemTableIdx);
    if (TestItem == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null item by row:%d col:%d.", curIndex.row(), iColItemTableIdx);
        return;
    }

    auto stuTestItem = TestItem->data(Qt::UserRole).value<tf::TestItem>();
		
    // 清空选中项信息
    if (m_lItemId != stuTestItem.id)
    {
        ClearSelImItemDetail();
        m_lItemId = stuTestItem.id;
    }

    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuTestItem.assayCode);
    if (Q_NULLPTR == spAssayInfo)
    {
        ULOG(LOG_WARN, "Invalid assay code,%d", stuTestItem.assayCode);
        return;
    }
    ui->reset_curve->setVisible(true);
    // 判断项目的设备类型
    if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
    {
        // 生化项目
        ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_CH);
        UpdateSelChItemDetailInfo(curIndex);
    }
    else if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        // 免疫项目
        // 免疫界面没有恢复比例按钮
        ui->reset_curve->setVisible(false);

        UpdateSelImItemDetailInfo(curIndex);
        ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_IM);
    }
    else if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE)
    {
		ui->reset_curve->setVisible(false);
        ui->AssayDetailStkWgt->setCurrentIndex(STACK_WIDGET_INDEX_CH);
        UpdateSelChItemDetailInfo(curIndex);
    }
    else
    {
        ULOG(LOG_WARN, "%s(), Unhandle device type,", __FUNCTION__);
        return;
    }
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void ItemResultDetailDlg::showEvent(QShowEvent* event)
{
    // 基类处理
    BaseDlg::showEvent(event);

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
        UpdateSampleInfo();
        m_bInit = true;
    }
}


///
/// @brief  设置lable显示
///
/// @param[in]  lab  lab控件
/// @param[in]  labTitle  lab控件(标题)
/// @param[in]  strItemName  字符串
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2024年9月2日，新建函数
///
void ItemResultDetailDlg::SetItemName(QLabel *lab, QLabel *labTitle, const QString& strItemName)
{
    if (nullptr == lab)
    {
        return;
    }
    int widthTitle = 0;
    if (nullptr != labTitle)
    {
        widthTitle = labTitle->width() + 2;
    }
    int width = ui->RemarkLab->width() - widthTitle;
    lab->setText(strItemName);
    lab->setToolTip(strItemName);
    QFontMetrics fm(lab->font());
    if (width > 0 && (fm.width(strItemName) >= width))
    {
        // 判断是否长度过长需要显示省略号
        QString newText = fm.elidedText(strItemName, Qt::ElideRight, width);       
        lab->setText(newText);
    }
}

///
/// @brief 显示病人信息
///
/// @param[in]  sample  样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
void ItemResultDetailDlg::DisplayPatientInfo(const tf::SampleInfo& sample)
{
    if (sample.__isset.patientInfoId && sample.patientInfoId < 0)
    {
        return;
    }

    // 查询病人信息
    ::tf::PatientInfoQueryResp ptQryResp;
    ::tf::PatientInfoQueryCond ptQryCond;
    ptQryCond.__set_id(sample.patientInfoId);

    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QueryPatientInfo(ptQryResp, ptQryCond)
        || ptQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || ptQryResp.lstPatientInfos.empty())
    {
        ULOG(LOG_ERROR, "%s(), QueryPatientInfo failed, by patientid:%lld.", __FUNCTION__, sample.patientInfoId);
        return;
    }

    // 将病人信息加载到界面
    tf::PatientInfo& stuPatientInfo = ptQryResp.lstPatientInfos[0];
    SetItemName(ui->NameValLab, ui->NameLab, QString::fromStdString(stuPatientInfo.name));
    // 性别不合法时为未赋值
    if (stuPatientInfo.gender == -1)
    {
        ui->GenderValLab->clear();
    }
    else
    {
        ui->GenderValLab->setText(ConvertTfEnumToQString(stuPatientInfo.gender));
    }
    // 年龄单位不合法时为未赋值
    if (stuPatientInfo.ageUnit == -1)
    {
        ui->AgeValLab->clear();
    }
    else
    {
        ui->AgeValLab->setText(QString::number(stuPatientInfo.age) + ConvertTfEnumToQString(stuPatientInfo.ageUnit));
    }

    SetItemName(ui->CaseNoValLab, ui->CaseNoLab, QString::fromStdString(stuPatientInfo.medicalRecordNo));
    SetItemName(ui->DepartmentValLab, ui->DepartmentLab, QString::fromStdString(stuPatientInfo.department));
    SetItemName(ui->VisitingDoctorValLab, ui->VisitingDoctorLab, QString::fromStdString(stuPatientInfo.attendingDoctor));

	QString showData;
	CutTipstring(QString::fromStdString(stuPatientInfo.diagnosis), showData, ui->DiagNosisValLab->width()-40);
    ui->DiagNosisValLab->setText(showData);
	ui->DiagNosisValLab->setToolTip(QString::fromStdString(stuPatientInfo.diagnosis));

    SetItemName(ui->RemarkValLab, nullptr, QString::fromStdString(stuPatientInfo.comment));
}

///
/// @brief 显示项目信息
///
/// @param[in]  testItemInfo  项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月18日，新建函数
///
void ItemResultDetailDlg::DisplayTestItem(const tf::TestItem& testItemInfo, int iRow)
{
    // 默认显示项目编号
    QString strAssayName(QString::number(testItemInfo.assayCode));
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(testItemInfo.assayCode);
    if (spAssayInfo != Q_NULLPTR)
    {
        // 转换项目编号为项目名称
        strAssayName = QString::fromStdString(spAssayInfo->assayName);
    }

    // 行数自适应
    if (iRow >= ui->AssayResTbl->rowCount())
    {
        ui->AssayResTbl->setRowCount(iRow + 1);
    }

    // 获取项目结果
	QString refFlag("");
    QString strFirstResult("");
    QString strLastResult("");
    QVariant varItemFirstRltInfo;
    QVariant varItemLastRltInfo;
	// 初测结果
	QTableWidgetItem* firstResultItem = new QTableWidgetItem();
	firstResultItem->setTextAlignment(Qt::AlignCenter);
	// 复查结果
	QTableWidgetItem* lastResultItem = new QTableWidgetItem();
	lastResultItem->setTextAlignment(Qt::AlignCenter);
	// 显示设置
	ResultPrompt resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();

    if (testItemInfo.__isset.firstTestResultKey)
    {
		refFlag = QDataItemInfo::Instance().ResultRangeFlag(false, testItemInfo, resultTips);
		// 超范围
		auto spSample = std::make_shared<tf::SampleInfo>(m_sampleInfo);
		strFirstResult = QDataItemInfo::Instance().GetAssayResult(false, testItemInfo, spSample);
		firstResultItem->setText(strFirstResult);
		// 设置当显示内容过长的时候，显示tooltips
		firstResultItem->setToolTip(strFirstResult);
		// 显示浓度的时候才显示
		SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
		if (WorkpageCommon::IsShowConc(sampleSet, spSample->sampleType))
		{
			if (refFlag.contains("↑") || refFlag.contains("H"))
			{
				firstResultItem->setData(Qt::UserRole + 1, 0);
				firstResultItem->setTextColor(QColor("#FA3741"));
			}
			else if (refFlag.contains("↓") || refFlag.contains("L"))
			{
				firstResultItem->setData(Qt::UserRole + 1, 1);
				firstResultItem->setTextColor(QColor("#459FFF"));
			}

			// 危机值
			if (resultTips.enabledCritical)
			{
				if (testItemInfo.__isset.firstCrisisRangeJudge && testItemInfo.firstCrisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
				{
					firstResultItem->setData(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS, QColor("#FFDAD6"));
				}
				else if (testItemInfo.__isset.firstCrisisRangeJudge && testItemInfo.firstCrisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
				{
					firstResultItem->setData(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS, QColor("#E5F2FF"));
				}
			}
		}
    }

    if (testItemInfo.__isset.lastTestResultKey)
    {
		// 获取状态标签
		refFlag = QDataItemInfo::Instance().ResultRangeFlag(true, testItemInfo, resultTips);
		// 获取值
		auto spSample = std::make_shared<tf::SampleInfo>(m_sampleInfo);
		strLastResult = QDataItemInfo::Instance().GetAssayResult(true, testItemInfo, spSample);
		// 标签与值的组合
		lastResultItem->setText(strLastResult);
		// 设置当显示内容过长的时候，显示tooltips
		lastResultItem->setToolTip(strLastResult);
		// 显示浓度的时候才显示
		SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
		if (WorkpageCommon::IsShowConc(sampleSet, spSample->sampleType))
		{
			// 设置字体颜色
			if (refFlag.contains("↑") || refFlag.contains("H"))
			{
				lastResultItem->setData(Qt::UserRole + 1, 0);
				lastResultItem->setTextColor(QColor("#FA3741"));
			}
			else if (refFlag.contains("↓") || refFlag.contains("L"))
			{
				lastResultItem->setData(Qt::UserRole + 1, 1);
				lastResultItem->setTextColor(QColor("#459FFF"));
			}

			// 危机值
			if (resultTips.enabledCritical)
			{
				if (testItemInfo.__isset.retestCrisisRangeJudge && testItemInfo.retestCrisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
				{
					lastResultItem->setData(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS, QColor("#FFDAD6"));
				}
				else if (testItemInfo.__isset.retestCrisisRangeJudge && testItemInfo.retestCrisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
				{
					lastResultItem->setData(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS, QColor("#E5F2FF"));
				}
			}
		}
    }

	// 校准和质控类型，没有复查，modifybug0011257 by wuht
	if (m_sampleInfo.sampleType != tf::SampleType::type::SAMPLE_TYPE_PATIENT)
	{
		// modify bug0010942 by wuht
		auto nameItem = new QTableWidgetItem(strAssayName);
		nameItem->setTextAlignment(Qt::AlignCenter);
		ui->AssayResTbl->setItem(iRow, 0, nameItem);
		ui->AssayResTbl->setItem(iRow, 1, firstResultItem);
		ui->AssayResTbl->item(iRow, 0)->setData(Qt::UserRole, QVariant::fromValue<tf::TestItem>(testItemInfo));
		ui->AssayResTbl->item(iRow, 1)->setData(Qt::UserRole, varItemFirstRltInfo);
	}
	else
	{
		// 将数据插入表格中
		// modify bug0010942 by wuht
		auto nameItem = new QTableWidgetItem(strAssayName);
		nameItem->setTextAlignment(Qt::AlignCenter);
		ui->AssayResTbl->setItem(iRow, 0, nameItem);
		ui->AssayResTbl->setItem(iRow, 1, firstResultItem);
		ui->AssayResTbl->setItem(iRow, 2, lastResultItem);
		ui->AssayResTbl->item(iRow, 0)->setData(Qt::UserRole, QVariant::fromValue<tf::TestItem>(testItemInfo));
		ui->AssayResTbl->item(iRow, 1)->setData(Qt::UserRole, varItemFirstRltInfo);
		ui->AssayResTbl->item(iRow, 2)->setData(Qt::UserRole, varItemLastRltInfo);
	}
}

///
/// @brief
///     删除按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void ItemResultDetailDlg::OnDelBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // bug21732 优先进行判定，防止提示过多
    if (m_sampleInfo.sampleType == tf::SampleType::type::SAMPLE_TYPE_CALIBRATOR
        || m_sampleInfo.sampleType == tf::SampleType::type::SAMPLE_TYPE_QC)
    {
        TipDlg(tr("错误"), tr("质控和校准样本不允许删除数据")).exec();
        return;
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("确定需要删除项目?"), TipDlgType::TWO_BUTTON));
    if (pTipDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel delete!");
        return;
    }

    if (m_sampleInfo.__isset.status && m_sampleInfo.status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING)
    {
        TipDlg(tr("错误"), tr("样本正在检测中，不可删除！")).exec();
        return;
    }

    // 查找登记表中对应的选中模型和标准模型
    QItemSelectionModel* slectmodule = ui->AssayResTbl->selectionModel();
    QModelIndexList selectIndexs = slectmodule->selectedRows();
    // 如果模型为空，则返回
    if (slectmodule == Q_NULLPTR || selectIndexs.count() <= 0)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未选中项目信息.")));
        pTipDlg->exec();
        return;
    }

    std::set<int64_t>  itemMap;
    std::set<int, std::greater<int>> rowMap;
    for (auto& index : selectIndexs)
    {
        auto TestItem = ui->AssayResTbl->item(index.row(), COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE);
        if (TestItem == Q_NULLPTR)
        {
            return;
        }

        rowMap.insert(index.row());
        auto stuTestItem = TestItem->data(Qt::UserRole).value<tf::TestItem>();
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuTestItem.assayCode);
        if (spAssayInfo != Q_NULLPTR)
        {
            itemMap.insert(stuTestItem.id);
        }
    }

    tf::SampleInfo sampleInfo;
    // 能找到则修改
    sampleInfo.__set_id(m_sampleInfo.id);
    std::vector<tf::TestItem>  testItems, delItems;
	auto spTestItemVec = DataPrivate::Instance().GetSampleTestItems(m_sampleInfo.id);
    for (const auto& spItem : spTestItemVec)
    {
		if (spItem == Q_NULLPTR)
		{
			continue;
		}

		auto item = *spItem;
        // 是否计算项目
        bool isCalcAssay = false;
        if (tf::AssayCodeRange::CALC_RANGE_MIN <= item.assayCode
            && item.assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX)
        {
            isCalcAssay = true;
        }

        // 不在删除列表的项目
        if (itemMap.count(item.id) <= 0 && !isCalcAssay)
        {
            testItems.push_back(item);
        }
		else
		{
			// 如果不是计算项目
			if (!isCalcAssay)
			{
				delItems.push_back(item);
			}
		}
    }

	// 根据现有项目重新生成删除项目以后的初测完成时间和复查完成时间
	// 初测完成时间是第一次完成项目的时间，复查完成时间是最后一次完成复查项目的时间
	// modify bug1589 by wuht
	std::string firstFinishedTime;
	std::string lastFinishedTime;
	for (const auto& item : testItems)
	{
		auto times = WorkpageCommon::GetItemTestTime(item.id, item.assayCode);

		if (firstFinishedTime.empty())
		{
			firstFinishedTime = times.first;
		}

		if (lastFinishedTime.empty())
		{
			lastFinishedTime = times.second;
		}

		// bug0013209 modify
		if (firstFinishedTime < times.first)
		{
			firstFinishedTime = times.first;
		}

		if (lastFinishedTime < times.second)
		{
			lastFinishedTime = times.second;
		}
	}
	sampleInfo.__set_endTestTime(firstFinishedTime);
	sampleInfo.__set_endRetestTime(lastFinishedTime);
    sampleInfo.__set_testItems(testItems);
    // 更新样本信息
    if (!DcsControlProxy::GetInstance()->ModifySampleInfo(sampleInfo))
    {
        TipDlg(tr("错误"), tr("删除项目信息失败")).exec();
        return;
    }
	// 添加操作日志 add by chenjianlin 20231215
	for (const auto& atVal : delItems)
	{
		COperationLogManager::GetInstance()->AddTestItemOperationLog(tf::OperationType::type::DEL, m_sampleInfo.seqNo, m_sampleInfo.barcode, atVal.assayCode);
	}
    // 清空数据信息
    if (testItems.empty())
    {
        // 清空生化的结果详细信息
        m_pChTabWidget = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
        // 清空免疫的结果详细信息
        auto tabWidget = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_IM));
        if (tabWidget != Q_NULLPTR)
        {
            tabWidget->Clear();
        }
    }

    // 删除行
    for (auto row : rowMap)
    {
        if (row < ui->AssayResTbl->rowCount())
        {
            ui->AssayResTbl->removeRow(row);
        }
    }

    // 若不空，则显示第一行
    if (ui->AssayResTbl->rowCount() != 0)
    {
        ui->AssayResTbl->selectRow(0);
    }
	// modify forbug0011003 by wuht
	else
	{
		ClearSelChItemDetail();
		ClearSelImItemDetail();
		ui->button_setRetest->setEnabled(false);
		ui->reset_curve->setEnabled(false);
	}
}

///
/// @brief
///     保存按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建函数
///
void ItemResultDetailDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const int iColIdxTabResult = COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE;

    // 检查参数
    auto curIndex = ui->AssayResTbl->currentIndex();
    if (!curIndex.isValid() || (ui->AssayResTbl->item(curIndex.row(), iColIdxTabResult) == Q_NULLPTR))
    {
        // 如果选中索引无效或选中空行，清空选中项信息
        ULOG(LOG_WARN, "Invalid sel index or empty row.");
        return;
    }

    // 获取当前选中项目的详情信息
    auto TestItem = ui->AssayResTbl->item(curIndex.row(), iColIdxTabResult);
    if (TestItem == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null item by row:%d col:%d.", curIndex.row(), iColIdxTabResult);
        return;
    }

    auto stuTestItem = TestItem->data(Qt::UserRole).value<tf::TestItem>();
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuTestItem.assayCode);
    if (Q_NULLPTR == spAssayInfo)
    {
        ULOG(LOG_WARN, "Invalid assay code,%d", stuTestItem.assayCode);
        return;
    }

    // 判断是否ISE
    auto IsISE = [&](int assayCode)->bool
    {
        if (assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_NA
            || assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_K
            || assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_CL)
        {
            return true;
        }

        return false;
    };

    // 生化项目
    if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
        || IsISE(stuTestItem.assayCode))
    {
        QTabWidget* pChTabWidget = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
        if (pChTabWidget == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "%s(), pChTabWidget == Q_NULLPTR,", __FUNCTION__);
            return;
        }

        auto assayResult = static_cast<QAssayResultView*>(pChTabWidget->currentWidget());
        if (assayResult == Q_NULLPTR)
        {
            return;
        }

        // 保存数据
        assayResult->SaveAssayResultEdit();
    }
    // 免疫项目
    else if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        QTabWidget* pImTabWgt = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_IM));
        if (pImTabWgt == Q_NULLPTR)
        {
            return;
        }

        auto assayResult = static_cast<ImAssayResultInfoWidget*>(pImTabWgt->currentWidget());
        if (assayResult == Q_NULLPTR)
        {
            return;
        }

        // 保存数据
		assayResult->Save();
    }
    UpdateSampleInfo();
}

///
/// @brief 恢复图例
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月6日，新建函数
///
void ItemResultDetailDlg::OnResetCurve()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const int iColIdxTabResult = COL_INDEX_ASSAY_NAME_OF_ITEM_RESULT_TABLE;
    // 检查参数
    auto curIndex = ui->AssayResTbl->currentIndex();
    if (!curIndex.isValid() || (ui->AssayResTbl->item(curIndex.row(), iColIdxTabResult) == Q_NULLPTR))
    {
        // 如果选中索引无效或选中空行，清空选中项信息
        ULOG(LOG_INFO, "%s(), sel incalid index or empty row.", __FUNCTION__);
        return;
    }

    // 获取当前选中项目的详情信息
    auto TestItem = ui->AssayResTbl->item(curIndex.row(), iColIdxTabResult);
    if (TestItem == Q_NULLPTR)
    {
        return;
    }

    auto stuTestItem = TestItem->data(Qt::UserRole).value<tf::TestItem>();
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuTestItem.assayCode);
    if (Q_NULLPTR == spAssayInfo)
    {
        ULOG(LOG_WARN, "Invalid assay code, %d", stuTestItem.assayCode);
        return;
    }

    // 生化项目
    if (spAssayInfo->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
    {
        QTabWidget* pChTabWidget = qobject_cast<QTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
        if (pChTabWidget == Q_NULLPTR)
        {
            ULOG(LOG_ERROR, "%s(), pChTabWidget == Q_NULLPTR,", __FUNCTION__);
            return;
        }

        auto assayResult = static_cast<QAssayResultView*>(pChTabWidget->currentWidget());
        if (assayResult == Q_NULLPTR)
        {
            return;
        }

        // 重置曲线
        assayResult->ResetCurve();
    }
}

///
/// @bref
///		视觉识别结果按钮点击槽函数
///
/// @par History:
/// @li 8276/huchunli, 2023年7月28日，新建函数
///
void ItemResultDetailDlg::OnAIIdentify()
{
    QHistoryAIIdentifyDlg aiIdenty;
    aiIdenty.exec();
}

///
/// @brief
///     项目结果表当前选中项改变
///
/// @param[in]  curIndex 当前选中项索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月20日，新建函数
///
void ItemResultDetailDlg::OnItemTblCurIndexChanged(const QModelIndex& curIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 更新选中项目详情
	ui->SaveBtn->setEnabled(false);
    UpdateSelItemDetailInfo(curIndex);
}

void ItemResultDetailDlg::OnSetCurrentRetestResult()
{
    auto index = ui->AssayDetailStkWgt->currentIndex();
    // 免疫
    if (index == STACK_WIDGET_INDEX_IM)
    {
        auto tabWidget = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_IM));
        if (tabWidget == Q_NULLPTR)
        {
            return;
        }

        auto rCurIndex = tabWidget->currentIndex();
        auto currentView = qobject_cast<ImAssayResultInfoWidget*>(tabWidget->currentWidget());
        if (currentView == Q_NULLPTR)
        {
            return;
        }

        // 设置当前结果为界面显示复查结果
        auto bRet = currentView->SetCurrentResult();
        if (bRet && rCurIndex > 0)
        {
            for (int index = 0; index < tabWidget->Count(); index++)
            {
                if (index == rCurIndex)
                {
                    continue;
                }

                auto widget = tabWidget->widget(index);
                if (widget != Q_NULLPTR)
                {
                    auto currentView = qobject_cast<QAssayResultView*>(widget);
                    if (currentView != Q_NULLPTR)
                    {
                        currentView->ClearReset();
                    }
                }
            }
            // 设置成功，应该将“设置复查结果”的按钮置灰
            ui->button_setRetest->setEnabled(false);
            tabWidget->SetFlag(std::vector<int>{rCurIndex});

            // 通知历史数据修改复查结果
            ::tf::HistoryBaseDataUserUseRetestUpdate ri;
            auto item = currentView->GetCurrentTestItem();
            ri.__set_itemId(item.id);
            ri.__set_userUseRetestResultId(item.lastTestResultKey.assayTestResultId);
            ri.__set_reTestResult(std::to_string(item.retestConc));
            ri.__set_reResultStatu(item.retestResultStatusCodes);
            POST_MESSAGE(MSG_ID_WORK_PAGE_RETEST_RESULT_UPDATE, ri);
        }
    }
    // 生化
    else if (index == STACK_WIDGET_INDEX_CH)
    {
        auto tabWidget = qobject_cast<RFlagTabWidget*>(ui->AssayDetailStkWgt->widget(STACK_WIDGET_INDEX_CH));
        if (tabWidget == Q_NULLPTR)
        {
            return;
        }

        auto rCurIndex = tabWidget->currentIndex();
        auto currentView = qobject_cast<QAssayResultView*>(tabWidget->currentWidget());
        if (currentView == Q_NULLPTR)
        {
            return;
        }

        // 设置当前结果为界面显示复查结果
        auto bRet = currentView->SetCurrentResult();
        if (bRet && rCurIndex > 0)
        {
            // bug3385
            for (int index = 0; index < tabWidget->Count(); index++)
            {
                if (index == rCurIndex)
                {
                    continue;
                }

                auto widget = tabWidget->widget(index);
                if (widget != Q_NULLPTR)
                {
                    auto currentView = qobject_cast<QAssayResultView*>(widget);
                    if (currentView != Q_NULLPTR)
                    {
                        currentView->ClearReset();
                    }
                }
            }

            // 设置成功，应该将“设置复查结果”的按钮置灰
            ui->button_setRetest->setEnabled(false);
            tabWidget->SetFlag(std::vector<int>{rCurIndex});
            // 通知历史数据修改复查结果
            ::tf::HistoryBaseDataUserUseRetestUpdate ri;

            auto item = currentView->GetCurrentTestItem();
            ri.__set_itemId(item.id);
            ri.__set_userUseRetestResultId(item.lastTestResultKey.assayTestResultId);
            ri.__set_reTestResult(std::to_string(item.retestConc));
            ri.__set_reResultStatu(item.retestResultStatusCodes);
            POST_MESSAGE(MSG_ID_WORK_PAGE_RETEST_RESULT_UPDATE, ri);
        }
    }
}

///
/// @brief
///     更新样本信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月26日，新建函数
///
void ItemResultDetailDlg::UpdateSampleInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ui->AssayResTbl->setRowCount(0);
    // 根据样本ID查询样本
	auto spSampleInfo = DataPrivate::Instance().GetSampleInfoByDb(m_lSampId);
	if (spSampleInfo == Q_NULLPTR)
	{
		return;
	}

	// modify bug0011353 by wuht
	ui->SaveBtn->hide();
	auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (userInfo != Q_NULLPTR && userInfo->type >=  tf::UserType::type::USER_TYPE_ADMIN)
	{
		ui->SaveBtn->show();
	}

	ui->SaveBtn->setEnabled(false);
    // 获取样本信息
    const tf::SampleInfo stuSelSampleInfo = *spSampleInfo;
    m_sampleInfo = *spSampleInfo;

	QStringList strHeaderList;
	// 校准和质控类型，没有复查，modifybug0011257 by wuht
	if (m_sampleInfo.sampleType != tf::SampleType::type::SAMPLE_TYPE_PATIENT)
	{
		ui->AssayResTbl->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
		strHeaderList << tr("项目名称") << tr("检测结果");
	}
	else
	{
		ui->AssayResTbl->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));
		ui->AssayResTbl->setItemDelegateForColumn(2, new CReadOnlyDelegate(this));
		strHeaderList << tr("项目名称") << tr("检测结果") << tr("复查结果");
	}

	// modify bug0011928 by wuht
	ui->AssayResTbl->setColumnCount(strHeaderList.size());
	ui->AssayResTbl->setHorizontalHeaderLabels(strHeaderList);

    std::vector<std::shared_ptr<tf::TestItem>> testItemvec;
    {

        auto&& testItems = DataPrivate::Instance().GetSampleTestItemMap(m_lSampId);
        if (!testItems)
        {
            return;
        }

        auto& testItemValue = testItems.value();
		std::map<int, int> showOrderMap{};
		if (!QSampleDetailModel::Instance().SortItem(testItemvec, testItemValue, showOrderMap))
        {
            return;
        }
    }

	// 初始设置为不能使用，只有当有项目的时候才能使用
	ui->DelBtn->setEnabled(false);
	ui->reset_curve->setEnabled(false);
	ui->button_setRetest->setEnabled(false);
	ui->button_AI->setEnabled(false);

	// modify bug0011341 by wuht(仅某些用户可以删除项目信息)
	m_delete = UserInfoManager::GetInstance()->IsPermisson(PSM_IM_RESULT_DETAIL_DELETE_SAMPLE);
    // bug26568 by zhangjing 修改为隐藏
	ui->DelBtn->setVisible(m_delete);
	// modify bug0011341 by wuht(仅某些用户可以设置当前结果为复查结果)
	m_reset = UserInfoManager::GetInstance()->IsPermisson(PSM_IM_RESULT_DETAIL_ASERCHECK);
    // bug26568 by zhangjing 修改为隐藏
	ui->button_setRetest->setVisible(m_reset);

	bool bReRun = false;
    int iRow = 0;
    for (auto& testItem : testItemvec)
    {
        if (testItem == Q_NULLPTR)
        {
            continue;
        }
		// 当前处于复查状态或者该样本已做复查
		if ((testItem->__isset.rerun && testItem->rerun) || \
			(testItem->__isset.lastTestResultKey && testItem->lastTestResultKey.assayTestResultId >= 0))
		{
			bReRun = true;
		}

        // 计算项目不显示
        if (CommonInformationManager::IsCalcAssay(testItem->assayCode))
        {
            continue;
        }

        // 按照项目展示的时候，只显示一个项目
        if (m_testItems.size() > 0)
        {
            if (testItem->id == m_testItems[0])
            {
                DisplayTestItem(*testItem, iRow);
                iRow++;
                break;
            }

            continue;
        }

        DisplayTestItem(*testItem, iRow);
        // 行号自增
        ++iRow;
    }

	// 若已经审核，则不能删除项目
	if (m_sampleInfo.__isset.audit)
	{
		ui->DelBtn->setEnabled(!m_sampleInfo.audit && m_delete);
		ui->button_setRetest->setEnabled(!m_sampleInfo.audit && m_reset);

		// modify forbug0011003 by wuht,必须要有项目
		if (testItemvec.empty())
		{
			ui->DelBtn->setEnabled(false);
			ui->button_setRetest->setEnabled(false);
		}
	}

    // 如果行数大于0,默认选中第一行
    if (iRow > 0)
    {
		ui->reset_curve->setEnabled(true);
		ui->button_AI->setEnabled(true);
		if (m_curRow)
		{
			int row = m_curRow.value();
			ui->AssayResTbl->selectRow(row);
		}
		else
		{    
			ui->AssayResTbl->selectRow(0);
			UpdateSelItemDetailInfo(ui->AssayResTbl->currentIndex());
			ui->AssayResTbl->selectRow(0);
		}

		ui->AssayResTbl->setFocus();
    }

    // 只有病人样本信息才显示病人
    if (m_sampleInfo.sampleType == tf::SampleType::type::SAMPLE_TYPE_PATIENT)
    {
        for (auto& item : ui->PatienInfoFrame->findChildren<QLabel*>())
        {
            item->show();
        }

        for (auto& item : ui->frame->findChildren<QLabel*>())
        {
            item->show();
        }

		// 已上传
		if (stuSelSampleInfo.__isset.uploaded && stuSelSampleInfo.uploaded)
		{
			ui->label_2->setText(tr("已传输"));
		}
        // 该UI显示和病人信息无关，和样本有关
		QString strStatus;
		if (bReRun)
		{
			strStatus += QString(" ") + tr("复查");
		}        
		// 样本检测状态
		strStatus += QString(" ") + ThriftEnumTrans::GetSampleTestStatus(stuSelSampleInfo.status);
		if (stuSelSampleInfo.uploaded)
		{
			strStatus += QString(" ") + tr("已传输");
		}
		strStatus = strStatus.trimmed();
		ui->label_2->setText(strStatus);

        // 样本类型
        ui->label_4->setText(ThriftEnumTrans::GetSourceTypeName(stuSelSampleInfo.sampleSourceType));

        // 架号/位置号
        QString strRack = stuSelSampleInfo.rack.empty() ? "" : (QString::fromStdString(stuSelSampleInfo.rack) + "-" + QString::number(stuSelSampleInfo.pos));
        ui->label_6->setText(strRack);

        // 样本号、条码
		auto barCode = QString::fromStdString(stuSelSampleInfo.barcode);
		ui->BarcodeValLab->setToolTip(barCode);
        ui->BarcodeValLab->setText(barCode);
        ui->SampNoValLab->setText(QString::fromStdString(stuSelSampleInfo.seqNo));

        // 无病人信息直接返回
        if (!stuSelSampleInfo.__isset.patientInfoId)
        {
            ULOG(LOG_INFO, "%s(), no patient info.", __FUNCTION__);
            return;
        }

        DisplayPatientInfo(stuSelSampleInfo);
    }
    else
    {
        for (auto& item : ui->PatienInfoFrame->findChildren<QLabel*>())
        {
            item->hide();
        }

        for (auto& item : ui->frame->findChildren<QLabel*>())
        {
            item->hide();
        }
    }
}
