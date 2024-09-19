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
/// @file     SpecialWashDlg.cpp
/// @brief    交叉污染配置
///
/// @author   7951/LuoXin
/// @date     2022年9月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SpecialWashDlg.h"
#include "ui_SpecialWashDlg.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QStandardItemModel>
#include <QRadioButton>
#include <QKeyEvent>
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/QComDelegate.h"
#include "shared/CommonInformationManager.h"

#include "AddWashCupDlg.h"
#include "AddWashReagentProbeDlg.h"
#include "AddWashSampleProbeDlg.h"
#include "AddWashImSampleProbeDlg.h"

#include "manager/DictionaryQueryManager.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "src/public/ch/ChConfigDefine.h"
#include "src/public/im/ImConfigDefine.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/gen-cpp/defs_types.h"

#define  VOLUME_FACTOR	                (10)			    // 清洗液用量倍率
#define	REAGRNT_WIDGET_NAME		        ("tab_reagent")		// 试剂针页面名称
#define	CUP_WIDGET_NAME			        ("tab_cup")	        // 反应杯页面名称
#define	SAMPLE_WIDGET_NAME				("tab_sample")	    // 样本针页面名称


// 试剂针配置表格各信息所在列
enum ReagentTableViewColunm
{
	RG_COLUNM_NUM,						// 序号
	RG_COLUNM_REAGENT_NAME,				// 试剂针
	RG_COLUNM_FROM_ASSAY_NAME,			// 造成影响的项目名字
	RG_COLUNM_TO_ASSAY_NAME,			// 受到影响的项目名字
	RG_COLUNM_DETERGEN_NAME,			// 清洗剂类型
	RG_COLUNM_ID						// 数据库主键
};

// 反应杯配置表格各信息所在列
enum CupTableViewColunm
{
	CUP_COLUNM_NUM,						// 序号
	CUP_COLUNM_ASSAY_NAME,				// 项目名字
	CUP_COLUNM_DETERGEN,				// 清洗剂类型
	CUP_COLUNM_ID						// 数据库主键
};

// 样本针配置表格各信息所在列
enum SPTableViewColunm
{
	SP_COLUNM_NUM,						// 序号
	SP_COLUNM_ASSAY_NAME,				// 项目名字
	SP_COLUNM_SAMPLE_SOURCE_NAME,		// 样本源名字
	SP_COLUNM_DETERGEN_NAME,			// 清洗剂类型
	SP_COLUNM_ID						// 数据库主键
};

// 样本针配置表格各信息所在列(免疫)
enum IMSPTableViewColunm
{
	IM_SP_COLUNM_NUM,					// 序号
	IM_SP_COLUNM_ASSAY_NAME,			// 项目名字
	IM_SP_COLUNM_SAMPLE_SOURCE_NAME,	// 样本源名字
    IM_SP_COLUNM_VOLUME,			    // 加样量（单位：0.1ul）
	IM_SP_COLUNM_SELECT,				// 勾选框
	IM_SP_COLUNM_ID						// 数据库主键
};

// 生化和免疫样本针配置页面索引
enum IM_AND_CH_INDEX
{
    CH_INDEX,							// 生化
    IM_INDEX,							// 免疫
};

SpecialWashDlg::SpecialWashDlg(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::SpecialWashDlg)
	, m_addWashCupDlg(new AddWashCupDlg(this))
	, m_addWashReagentProbeDlg(new AddWashReagentProbeDlg(this))
	, m_addWashSampleProbeDlg(new AddWashSampleProbeDlg(this))
	, m_addWashImSampleProbeDlg(new AddWashImSampleProbeDlg(this))
	, m_reagentProbeMode(new QStandardItemModel)
	, m_sampleProbeMode(new QStandardItemModel)
	, m_imSampleProbeMode(new QStandardItemModel)
	, m_cupMode(new QStandardItemModel)
{
	ui->setupUi(this);

	// 初始化
	InitCtrls();
}

SpecialWashDlg::~SpecialWashDlg()
{

}

void SpecialWashDlg::InitCtrls()
{
	// 隐藏保存按钮
	ui->flat_save->hide();

	// 限制间隔清洗次数100-10000
	ui->interval_test_times_edit->setValidator(new QIntValidator(100, 10000, ui->interval_test_times_edit));

	// 初始化清洗液下拉框
    AddTfEnumItemToComBoBox(ui->lot_test_combox, ::tf::DetergentType::DET_TYPE_ACIDITY);
    AddTfEnumItemToComBoBox(ui->interval_test_times_liquid_combox, ::tf::DetergentType::DET_TYPE_ACIDITY);
    AddTfEnumItemToComBoBox(ui->lot_test_combox, ::tf::DetergentType::DET_TYPE_ALKALINITY);
    AddTfEnumItemToComBoBox(ui->interval_test_times_liquid_combox, ::tf::DetergentType::DET_TYPE_ALKALINITY);
    AddTfEnumItemToComBoBox(ui->lot_test_combox, ::tf::DetergentType::DET_TYPE_WATER);
    AddTfEnumItemToComBoBox(ui->interval_test_times_liquid_combox, ::tf::DetergentType::DET_TYPE_WATER);
    AddTfEnumItemToComBoBox(ui->lot_test_combox, ::tf::DetergentType::DET_TYPE_ULTRASONIC);
    AddTfEnumItemToComBoBox(ui->interval_test_times_liquid_combox, ::tf::DetergentType::DET_TYPE_ULTRASONIC);

	// 1：右上角部件的显示
	m_cornerItem = new QWidget(ui->tabWidget);
	QHBoxLayout* hlayout = new QHBoxLayout(m_cornerItem);
	m_cornerItem->setLayout(hlayout);
	QButtonGroup* pButtonGroup = new QButtonGroup(this);

	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->setSpacing(0);
	m_cornerItem->setObjectName("bgCornerShow");

	// 设置互斥
	pButtonGroup->setExclusive(true);
	QRadioButton *pButton_1 = new QRadioButton(tr("生化"));
    pButton_1->setObjectName(QStringLiteral("showByChBtn"));
    pButton_1->setCheckable(true);
	connect(pButton_1, SIGNAL(clicked()), this, SLOT(OnShowByCh()));
    pButton_1->setChecked(true);
	pButtonGroup->addButton(pButton_1);
	hlayout->addWidget(pButton_1);

    QRadioButton *pButton_2 = new QRadioButton(tr("免疫"));
    pButton_2->setObjectName(QStringLiteral("showByImBtn"));
	connect(pButton_2, SIGNAL(clicked()), this, SLOT(OnShowByIm()));
	pButtonGroup->addButton(pButton_2);
	hlayout->addWidget(pButton_2);
	ui->tabWidget->setCornerWidget(m_cornerItem);
	m_cornerItem->hide();

	ui->lineEdit_interval_steps->setEnabled(false);
	connect(ui->checkBox_interval_steps, &QCheckBox::clicked, this, [&](bool checked) { ui->lineEdit_interval_steps->setEnabled(checked); });

	// 设置试剂针表格表头
	QStringList headerListString;
	headerListString << tr("序号") << tr("试剂针") << tr("污染物") << tr("受污染物") 
					 << tr("清洗方式") << tr("id");
	m_reagentProbeMode->setHorizontalHeaderLabels(headerListString);

	// 设置反应杯表格表头
	headerListString.clear();
	headerListString << tr("序号")<< tr("项目名称") << tr("清洗方式") << tr("id");
	m_cupMode->setHorizontalHeaderLabels(headerListString);

	// 设置表格的model
	ui->reagent_tableView->setModel(m_reagentProbeMode);
	ui->cup_tableView->setModel(m_cupMode);
	ui->sample_tableView->setModel(m_sampleProbeMode);
	ui->imSample_tableView->setModel(m_imSampleProbeMode);

	// 设置样本针表格表头（生化）
	headerListString.clear();
	headerListString << tr("序号") << tr("项目名称")<< tr("样本类型")<< tr("清洗方式") << tr("id");
	m_sampleProbeMode->setHorizontalHeaderLabels(headerListString);

	// 设置样本针表格表头(免疫)
	ui->lineEdit_interval_steps->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT_NOT_ZERO_1000), this));
	headerListString.clear();
	headerListString << tr("序号") << tr("项目名称") << tr("样本类型") << tr("加样量(μL)") << tr("选择") << tr("id");
	m_imSampleProbeMode->setHorizontalHeaderLabels(headerListString);

	// 隐藏表格的id的列
	ui->reagent_tableView->hideColumn(RG_COLUNM_ID);
	ui->cup_tableView->hideColumn(CUP_COLUNM_ID);
	ui->sample_tableView->hideColumn(SP_COLUNM_ID);
	ui->imSample_tableView->hideColumn(IM_SP_COLUNM_ID);

	// 隐藏勾选框列（后续可能删除勾选框）
	ui->imSample_tableView->hideColumn(IM_SP_COLUNM_SELECT);
	ui->imSample_tableView->setMouseTracking(true);
	ui->imSample_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->imSample_tableView->horizontalHeader()->setTextElideMode(Qt::ElideRight);

	connect(ui->btn_new, SIGNAL(clicked()), this, SLOT(OnAddBtnClicked()));
	connect(ui->btn_del, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));
	connect(ui->btn_edit, SIGNAL(clicked()), this, SLOT(OnModifyBtnClicked()));
	connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

	connect(m_addWashReagentProbeDlg, &AddWashReagentProbeDlg::WashReagentProbeChanged, this, &SpecialWashDlg::LoadReagentDataToTableview);
	connect(m_addWashCupDlg, &AddWashCupDlg::WashCupChanged, this, &SpecialWashDlg::LoadCupDataToTableview);
    connect(m_addWashSampleProbeDlg, &AddWashSampleProbeDlg::WashSampleProbeChanged, this, [&] 
    {
        LoadSampleDataToTableview(false);
    });
	connect(m_addWashImSampleProbeDlg, &AddWashImSampleProbeDlg::WashImSampleProbeChanged, this, [&](const bool pageChanged, const bool isAdd) {
		auto selectRows = ui->imSample_tableView->selectionModel()->selectedRows();

		LoadImSampleDataToTableview(pageChanged);
		if (isAdd)
		{
			ui->imSample_tableView->selectRow(m_imSampleProbeMode->rowCount() - 1);
			return;
		}

		if (selectRows.isEmpty())
		{
			return;
		}

		ui->imSample_tableView->selectRow(selectRows.first().row());
	});

	// 点击勾选（后续可能删除勾选框）
	connect(ui->imSample_tableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnSaveTableViewSelect(const QModelIndex&)));

    // 未选中按项目强化清洗，刷新界面
    connect(ui->assay_test_ckbox, &QCheckBox::stateChanged, this, &SpecialWashDlg::OnUpdateCtrls);

	// 未选中项目测试结束，刷新界面
	connect(ui->checkBox_item_finish, &QCheckBox::stateChanged, this, &SpecialWashDlg::OnUpdateCtrls);

	connect(ui->cup_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SpecialWashDlg::OnTableViewRowChanged);
	connect(ui->reagent_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SpecialWashDlg::OnTableViewRowChanged);
	connect(ui->sample_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SpecialWashDlg::OnTableViewRowChanged);
    connect(ui->imSample_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SpecialWashDlg::OnTableViewRowChanged);

	ui->imSample_tableView->setItemDelegateForColumn(IM_SP_COLUNM_SELECT, new CheckBoxDelegate(this));

	connect(ui->imSample_tableView, &QTableView::doubleClicked, this, [&](const QModelIndex& index)
	{
		if (index.column() == IM_SP_COLUNM_SELECT)
		{
			m_imSampleProbeMode->item(index.row(), index.column())->setData(
				!m_imSampleProbeMode->item(index.row(), index.column())->data().toBool(), Qt::UserRole + 1);
		}
	});

    // 设置按钮的状态
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [=] 
    {
        LoadDataToDlg();
    });

    // 设置生化免疫按钮是否可见
    SOFTWARE_TYPE type = CommonInformationManager::GetInstance()->GetSoftWareType();
    if (type == CHEMISTRY)
    {
        ui->stackedWidget->setCurrentIndex(CH_INDEX);
    } 
    else if(type == IMMUNE)
    {
        ui->tabWidget->removeTab(0);
        ui->tabWidget->removeTab(0);
        ui->flat_save->show();
        ui->stackedWidget->setCurrentIndex(IM_INDEX);
		ui->flat_save->setVisible(true);
    }

    pButton_1->setVisible(type == CHEMISTRY_AND_IMMUNE);
    pButton_2->setVisible(type == CHEMISTRY_AND_IMMUNE);
    ui->ch_im_same_tube->setVisible(type == CHEMISTRY_AND_IMMUNE);
}

void SpecialWashDlg::OnUpdateCtrls()
{
    if (ui->tabWidget->count() -1 == ui->tabWidget->currentIndex())
    {
        bool boxStatus;
        // 生化样本针
        if (ui->stackedWidget->currentIndex() == 0)
        {
            boxStatus = ui->assay_test_ckbox->isChecked();
        }
        // 免疫样本针
        else
        {
            boxStatus = ui->checkBox_item_finish->isChecked();
        }

        auto view = ui->stackedWidget->currentWidget()->findChild<QTableView*>();
        QModelIndexList rowList = view->selectionModel()->selectedRows();

        view->setEnabled(boxStatus);
        ui->btn_edit->setEnabled(boxStatus && !rowList.empty());
        ui->btn_del->setEnabled(boxStatus && !rowList.empty());
        ui->btn_new->setEnabled(boxStatus);
        ui->flat_save->show();
        m_cornerItem->setVisible(ui->tabWidget->count());
    } 
    else
    {
        auto view = ui->tabWidget->currentWidget()->findChild<QTableView*>();
        QModelIndexList rowList = view->selectionModel()->selectedRows();
        ui->btn_edit->setEnabled(!rowList.empty());
        ui->btn_del->setEnabled(!rowList.empty());
        ui->btn_new->setEnabled(true);
        ui->flat_save->hide();
        m_cornerItem->hide();
    }
}

void SpecialWashDlg::LoadDataToDlg()
{
	LoadCupDataToTableview();
	LoadSampleDataToTableview();
	LoadImSampleDataToTableview();
	LoadReagentDataToTableview();
}

void SpecialWashDlg::showEvent(QShowEvent *event)
{
	// 基类先处理
	QWidget::showEvent(event);

	LoadDataToDlg();

    ui->tabWidget->setCurrentIndex(0);
}

bool SpecialWashDlg::eventFilter(QObject * obj, QEvent * event)
{
	if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent == nullptr)
		{
			return QWidget::eventFilter(obj, event);
		}

		if (keyEvent->key() == Qt::Key_Control || keyEvent->key() == Qt::Key_Shift)
		{
			ui->imSample_tableView->setSelectionMode(event->type() == QEvent::KeyPress ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);
		}
	}
	return QWidget::eventFilter(obj, event);
}

bool SpecialWashDlg::CheckUserData(bool AddNewFalg)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	return false;
}

void SpecialWashDlg::LoadReagentDataToTableview()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    int selectRow = ui->reagent_tableView->currentIndex().row();

	// 清空表格
	m_reagentProbeMode->removeRows(0, m_reagentProbeMode->rowCount());

	// 查询试剂针配置信息
	ch::tf::SpecialWashReagentProbeQueryCond srqc;
	ch::tf::SpecialWashReagentProbeQueryResp srqr;
	ch::LogicControlProxy::QuerySpecialWashReagentProbe(srqr, srqc);
	if (srqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySpecialWashReagentProbe Failed !");
		return;
	}

	// 加载试剂针配置信息到表格
	int iRow = 0;
	for (const auto& item : srqr.lstSpecialWashItems)
    {
        auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item.fromAssay);
        if (assayInfo == nullptr)
        {
            ULOG(LOG_WARN, "GetChemistryAssayInfo By AssayCode[%d] Failed !", item.fromAssay);
            continue;
        }
		// 序号
        AddTextToTableView(m_reagentProbeMode,iRow, RG_COLUNM_NUM, QString::number(iRow + 1))

		// 试剂针
        AddTextToTableView(m_reagentProbeMode, iRow, RG_COLUNM_REAGENT_NAME, "R" + QString::number(item.probeNum))

        // 污染物
        AddTextToTableView(m_reagentProbeMode, iRow, RG_COLUNM_FROM_ASSAY_NAME, QString::fromStdString(assayInfo->assayName))
	
		// 受污染物
		auto toAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item.toAssay);
		if (toAssayInfo == nullptr)
		{
			ULOG(LOG_WARN, "GetChemistryAssayInfo By AssayCode[%d] Failed !", item.toAssay);
			continue;
		}
        AddTextToTableView(m_reagentProbeMode, iRow, RG_COLUNM_TO_ASSAY_NAME, QString::fromStdString(toAssayInfo->assayName))

        // 清洗剂
        AddTextToTableView(m_reagentProbeMode, iRow, RG_COLUNM_DETERGEN_NAME, ConvertTfEnumToQString(item.detergentType))
	
		// id
		AddTextToTableView(m_reagentProbeMode,iRow, RG_COLUNM_ID, QString::number(item.id))
		
		iRow++;
	}

	ui->reagent_tableView->setColumnWidth(0, 105);
	ui->reagent_tableView->horizontalHeader()->setStretchLastSection(true);
	ui->reagent_tableView->horizontalHeader()->setMinimumSectionSize(80);

	ui->cup_tableView->setColumnWidth(0, 105);
	ui->cup_tableView->horizontalHeader()->setStretchLastSection(true);
	ui->cup_tableView->horizontalHeader()->setMinimumSectionSize(80);

	ui->sample_tableView->setColumnWidth(0, 105);
	ui->sample_tableView->horizontalHeader()->setStretchLastSection(true);
	ui->sample_tableView->horizontalHeader()->setMinimumSectionSize(80);

	ui->imSample_tableView->setColumnWidth(0, 105);
    ui->imSample_tableView->horizontalHeader()->setStretchLastSection(true);
    ui->imSample_tableView->horizontalHeader()->setMinimumSectionSize(80);

    OnUpdateCtrls();

    if (selectRow < m_reagentProbeMode->rowCount())
    {
        ui->reagent_tableView->selectRow(0);
    }
    else
    {
        ui->reagent_tableView->selectRow(m_reagentProbeMode->rowCount() - 1);
    }
}

void SpecialWashDlg::LoadSampleProbeWashConfig()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    int selectRow = ui->reagent_tableView->currentIndex().row();

    SampleProbeWashConfig data;
    if (!DictionaryQueryManager::GetSampleProbeWashConfigCh(data))
    {
        ULOG(LOG_ERROR, "Failed to get ch sampleprobewash config.");
        return;
    }

	// 显示到界面
	if (data.iIntervalTimes >= 0)
	{
		ui->interval_test_times_edit->setText(QString::number(data.iIntervalTimes));
	}

    ui->interval_test_times_ckbox->setChecked(data.bIntervalTimesEndable);
    ui->interval_test_times_liquid_combox->setCurrentText(ConvertTfEnumToQString((::tf::DetergentType::type)data.iIntervalTimesWashLiquid));
    ui->lot_test_ckbox->setChecked(data.bLotTestEndable);
    ui->lot_test_combox->setCurrentText(ConvertTfEnumToQString((::tf::DetergentType::type)data.iLotTestWashLiquid));
    ui->assay_test_ckbox->setChecked(data.bAssayIntensiveWash);
    ui->ch_im_same_tube->setChecked(data.bChImSameTube);

    OnUpdateCtrls();

    if (selectRow < m_sampleProbeMode->rowCount())
    {
        ui->sample_tableView->selectRow(0);
    }
    else
    {
        ui->sample_tableView->selectRow(m_sampleProbeMode->rowCount() - 1);
    }
}

void SpecialWashDlg::LoadImSampleProbeWashConfig()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    ImSampleProbeWashConfig data;
    if (!DictionaryQueryManager::GetSampleProbeWashConfigIm(data))
    {
        ULOG(LOG_ERROR, "Failed to get im sampleprobewash config.");
        return;
    }

	ui->lineEdit_interval_steps->setEnabled(data.bIntervalTimesEndable);
	ui->lineEdit_interval_steps->setText(data.iIntervalTimes > 0?QString::number(data.iIntervalTimes):"");
	ui->checkBox_interval_steps->setChecked(data.bIntervalTimesEndable);
	ui->checkBox_batch_finish->setChecked(data.bLotTestEndable);
	ui->checkBox_item_finish->setChecked(data.bItemTestEndable);

    OnUpdateCtrls();
}

void SpecialWashDlg::LoadCupDataToTableview()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    int selectRow = ui->reagent_tableView->currentIndex().row();

	// 清空表格
	m_cupMode->removeRows(0, m_cupMode->rowCount());

	//查询反应杯配置信息
	ch::tf::SpecialWashCupQueryCond scqc;
	ch::tf::SpecialWashCupQueryResp scqr;
	ch::LogicControlProxy::QuerySpecialWashCup(scqr, scqc);
	if (scqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySpecialWashReagentProbe Failed !");
		return;
	}

	// 加载反应杯配置信息到表格
	int iRow = 0;
	for (auto item : scqr.lstSpecialWashCup)
    {
        auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item.assayCode);
        if (assayInfo == nullptr)
        {
            ULOG(LOG_WARN, "GetChemistryAssayInfo By AssayCode[%d] Failed !", item.assayCode);
            continue;
        }
		// 序号
        AddTextToTableView(m_cupMode,iRow, CUP_COLUNM_NUM, QString::number(iRow + 1))

        // 项目名称
		AddTextToTableView(m_cupMode, iRow, CUP_COLUNM_ASSAY_NAME, QString::fromStdString(assayInfo->assayName))

		// 清洗剂
		AddTextToTableView(m_cupMode, iRow, CUP_COLUNM_DETERGEN, ConvertTfEnumToQString(item.washItem.detergentType))

		// id
		AddTextToTableView(m_cupMode, iRow, CUP_COLUNM_ID, QString::number(item.id))

		iRow++;
	}

    OnUpdateCtrls();

    if (selectRow < m_cupMode->rowCount())
    {
        ui->cup_tableView->selectRow(0);
    }
    else
    {
        ui->cup_tableView->selectRow(m_cupMode->rowCount() - 1);
    }
}

void SpecialWashDlg::SaveChSampleProbeWashConfig()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    for (const auto& spDev : CommonInformationManager::GetInstance()->
        GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000, ::tf::DeviceType::DEVICE_TYPE_ISE1005 }))
    {
        if (devIsRun(*spDev))
        {
            TipDlg(tr("保存失败"), tr("仪器运行中，不能修改样本针特殊清洗配置")).exec();
            return;
        }
    }

	SampleProbeWashConfig data;

    data.iIntervalTimes = ui->interval_test_times_edit->text().toInt();
	if (ui->interval_test_times_ckbox->isChecked() 
        && (data.iIntervalTimes < 100 || data.iIntervalTimes > 10000))
	{
        TipDlg(tr("保存失败"),tr("值错误：100≤间隔测试数≤10000")).exec();
        return;
	}

    if (ui->interval_test_times_ckbox->isChecked() 
        && ui->interval_test_times_liquid_combox->currentIndex() == -1)
    {
        TipDlg(tr("保存失败"), tr("请选择间隔测试次数的清洗液")).exec();
        return;
    }

    if (ui->lot_test_ckbox->isChecked()
        && ui->lot_test_combox->currentIndex() == -1)
    {
        TipDlg(tr("保存失败"), tr("请选择批测试完成的清洗液")).exec();
        return;
    }

	data.bIntervalTimesEndable = ui->interval_test_times_ckbox->isChecked();
	data.iIntervalTimesWashLiquid = ui->interval_test_times_liquid_combox->currentData().toInt();
	data.bLotTestEndable = ui->lot_test_ckbox->isChecked();
	data.iLotTestWashLiquid = ui->lot_test_combox->currentData().toInt();
    data.bAssayIntensiveWash = ui->assay_test_ckbox->isChecked();
    data.bChImSameTube = ui->ch_im_same_tube->isChecked();

    SampleProbeWashConfig olDwashConfig;
    if (!DictionaryQueryManager::GetSampleProbeWashConfigCh(olDwashConfig))
    {
        ULOG(LOG_ERROR, "Failed get ch sampleprobwash config!");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    if (olDwashConfig == data)
    {
        ULOG(LOG_INFO, "The data has not changed!");
        return;
    }

    // 保存显示设置
    if (!DictionaryQueryManager::SaveSampleProbeWashConfigCh(data))
    {
        ULOG(LOG_ERROR, "Failed save ch sampleprobwash config!");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    // 记录编辑样本针日志
    QString optStr = (data.bAssayIntensiveWash ? tr("启用") : tr("关闭")) + tr("按项目强化清洗");
    optStr += (data.bIntervalTimesEndable ? tr("，启用") : tr("，关闭")) + tr("间隔测试数");
    optStr += tr("，次数：") + QString::number(data.iIntervalTimes);
    optStr += tr("，清洗方式：") + ConvertTfEnumToQString((::tf::DetergentType::type)data.iIntervalTimesWashLiquid);
    optStr += (data.bLotTestEndable ? tr("，启用") : tr("，关闭")) + tr("批测试完成");
    optStr += tr("，清洗方式：") + ConvertTfEnumToQString((::tf::DetergentType::type)data.iLotTestWashLiquid);
    optStr += (data.bChImSameTube ? tr("，启用") : tr("，关闭")) + tr("生免同管");

    if (!AddOptLog(::tf::OperationType::DEL, tr("编辑样本针特殊清洗:") + optStr))
    {
        ULOG(LOG_ERROR, "Add delete special wash sample probe operate log failed !");
    }
}

///
/// @brief 保存免疫样本针清洗配置信息
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年10月26日，添加注释
///
void SpecialWashDlg::SaveImSampleProbeWashConfig()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
	int iIntervalTimes = 0;
	if (ui->checkBox_interval_steps->isChecked())
	{
		if (ui->lineEdit_interval_steps->text().isEmpty())
		{
			ULOG(LOG_ERROR, "Failed to save im sampleprobwash config!");
			TipDlg(tr("保存失败"), tr("间隔测试数不能为空")).exec();
			return;
		}

		iIntervalTimes = ui->lineEdit_interval_steps->text().toInt();
		if (iIntervalTimes <= 0)
		{
			ULOG(LOG_ERROR, "iIntervalTimes input data is invalid");
			TipDlg(tr("保存失败"), tr("间隔测试数输入值必须为正整数")).exec();
			return;
		}
	}

	ImSampleProbeWashConfig data;
	data.bIntervalTimesEndable = ui->checkBox_interval_steps->isChecked();
	data.iIntervalTimes = ui->lineEdit_interval_steps->text().toInt();
	data.bLotTestEndable = ui->checkBox_batch_finish->isChecked();
	data.bItemTestEndable = ui->checkBox_item_finish->isChecked();

    // 保存显示设置
    if (!DictionaryQueryManager::SaveSampleProbeWashConfigIm(data))
    {
        ULOG(LOG_ERROR, "Failed to save im sampleprobwash config!");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    // 添加操作日志，
    if (AddOptLog(::tf::OperationType::MOD, tr("修改样本针特殊清洗")))
    {
        ULOG(LOG_ERROR, "Add modify special wash sample probe operate log failed !");
    }
}

///
/// @brief 检查是否允许保存免疫样本针清洗配置信息
///
///
/// @return ture:允许保存 false:不允许保存
///
/// @par History:
/// @li 1556/Chenjianlin，2023年10月26日，新建函数
///
bool SpecialWashDlg::EnableSaveImSampleProbeWashConfig()
{
	// 判断所有免疫设备是否在运行
	bool bRun = CommonInformationManager::GetInstance()->IsExistDeviceRuning(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE);
	// 遍历设备进行分类
	if (bRun)
	{
		// 消息提示
		TipDlg(tr("保存失败"), tr("仪器处于运行状态或加样停状态不能进行保存操作！")).exec();
		return false;
	}
	return true;
}

void SpecialWashDlg::LoadSampleDataToTableview(bool pageChanged /*= true*/)
{
	ULOG(LOG_INFO, "%s pageChanged:%d.", __FUNCTION__, pageChanged);

	// 重新加载界面
    if (pageChanged)
    {
        ui->interval_test_times_ckbox->setChecked(false);
        ui->interval_test_times_edit->clear();
        ui->interval_test_times_liquid_combox->setCurrentIndex(-1);
        ui->lot_test_ckbox->setChecked(false);
        ui->lot_test_combox->setCurrentIndex(-1);
        ui->assay_test_ckbox->setChecked(false);

        LoadSampleProbeWashConfig();
    }

	// 查询样本针配置信息
	ch::tf::SpecialWashSampleProbeQueryCond ssqc;
	ch::tf::SpecialWashSampleProbeQueryResp ssqr;
	ch::LogicControlProxy::QuerySpecialWashSampleProbe(ssqr, ssqc);
	if (ssqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySpecialWashSampleProbe Failed !");
		return;
	}

	// 加载样本针配置信息到表格
    m_sampleProbeMode->removeRows(0, m_sampleProbeMode->rowCount());
	int iRow = 0;
	for (auto item : ssqr.lstSpecialWashSampleProbe)
	{
        // 已经删除的项目不再显示
        if (CommonInformationManager::GetInstance()->GetAssayInfo(item.assayCode) == nullptr)
        {
            ULOG(LOG_WARN, "Not exist assayinfo, %d.", item.assayCode);
            continue;
        }

		// 序号
		AddTextToTableView(m_sampleProbeMode, iRow, SP_COLUNM_NUM, QString::number(iRow + 1))

		// 项目名称
        QString assayName;
        const auto& CIM = CommonInformationManager::GetInstance();
        if (item.assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_ISE)
        {
            for (int i = ::ise::tf::g_ise_constants.ASSAY_CODE_NA; i <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL; i++)
            {
                const auto& tempName = QString::fromStdString(CIM->GetAssayNameByCode(i));
                assayName += assayName.isEmpty() ? tempName : ("、" + tempName);
            }
        } 
        else
        {
            assayName = QString::fromStdString(CIM->GetAssayNameByCode(item.assayCode));
        }

		AddTextToTableView(m_sampleProbeMode,iRow, SP_COLUNM_ASSAY_NAME, assayName)

		// 样本源类型
		AddTextToTableView(m_sampleProbeMode, iRow, SP_COLUNM_SAMPLE_SOURCE_NAME, ConvertTfEnumToQString(item.sampleSourceType))

		// 清洗剂
		AddTextToTableView(m_sampleProbeMode, iRow, SP_COLUNM_DETERGEN_NAME, ConvertTfEnumToQString(item.detergentType))
	
		// id
		AddTextToTableView(m_sampleProbeMode, iRow, SP_COLUNM_ID, QString::number(item.id))

		iRow++;
	}

	// 默认选中第一行
	ui->sample_tableView->selectRow(0);

    OnUpdateCtrls();
}

///
/// @brief
///     获取免疫的通道号-吸样量 map
///
/// @param[in]   vecAsssyCode,通道号集合
/// @param[out]   mapAssaycodeSuckVol,通道号-吸样量 map
///
/// @return 
///
/// @par History:
/// @li 7685/likai，2023年10月27日，新建函数
bool SpecialWashDlg::GetImSuckVolMap(std::set<int> vecAsssyCode, std::map<int, int> & mapAssaycodeSuckVol)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    //先查数据库获取项目吸样量
    ::im::tf::GeneralAssayInfoQueryResp _return;
    ::im::tf::GeneralAssayInfoQueryCond gaiq;
    //gaiq.__set_usingFlag(true); //usingFlag并不是代表在用的版本，如果一个通道号有多条项目信息记录，取最高版本的即可
    ::im::LogicControlProxy::QueryImGeneralAssayInfo(_return, gaiq);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryImGeneralAssayInfo Failed !");
        return false;
    }

    //若有多个版本只取最高版本
    std::map<int, ::im::tf::GeneralAssayInfo> mapAssayCodeAssayInfos;
    for (auto assay : _return.lstAssayConfigInfos)
    {
        auto tmpAssay = mapAssayCodeAssayInfos.find(assay.assayCode);
        if (tmpAssay != mapAssayCodeAssayInfos.end())
        {
            //已经存在则取高版本
            if (assay.version > tmpAssay->second.version)
            {
                tmpAssay->second = assay;
                mapAssaycodeSuckVol[assay.assayCode] = assay.reactInfo.suckVol;
            }
        }
        else
        {
            //未存在直接插入
            mapAssayCodeAssayInfos.insert(std::make_pair(assay.assayCode, assay));
            mapAssaycodeSuckVol.insert(std::make_pair(assay.assayCode, assay.reactInfo.suckVol));
        }
    }
    return true;
}

void SpecialWashDlg::LoadImSampleDataToTableview(const bool pageChanged /* = true*/)
{
	ULOG(LOG_INFO, "%s(pageChanged:%d.", __FUNCTION__, pageChanged);

    if (pageChanged)
    {
        // 清空单选开关
        ui->checkBox_interval_steps->setChecked(false);
        ui->lineEdit_interval_steps->clear();
        ui->checkBox_batch_finish->setChecked(false);
        ui->checkBox_item_finish->setChecked(false);

        LoadImSampleProbeWashConfig();
    }
	
	// 清空表格
	m_imSampleProbeMode->removeRows(0, m_imSampleProbeMode->rowCount());

	// 查询样本针配置信息
	im::tf::SpecialWashSampleProbeQueryCond ssqc;
	im::tf::SpecialWashSampleProbeQueryResp ssqr;
	im::LogicControlProxy::QuerySpecialWashSampleProbe(ssqr, ssqc);
	if (ssqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySpecialWashSampleProbe Failed !");
		return;
	}

	// 加载样本针配置信息到表格
	int iRow = 0;

    //获取通道号-吸样量map
    std::map<int, int> mapAssaycodeSuckVol;
    std::set<int> vecAsssyCode;
    for (const auto& item : ssqr.lstSpecialWashSampleProbe)
    {
        vecAsssyCode.insert(item.assayCode);
    }

    if (!GetImSuckVolMap(vecAsssyCode, mapAssaycodeSuckVol))
    {
        ULOG(LOG_ERROR, "GetImSuckVolMap Failed !");//查不到后续填0
    }

	for (auto item : ssqr.lstSpecialWashSampleProbe)
    {
        auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item.assayCode);
        if (assayInfo == nullptr)
        {
            ULOG(LOG_WARN, "GetChemistryAssayInfo By AssayCode[%d] Failed !", item.assayCode);
            continue;
        }
		// 序号
        auto ImSampleProbeOrder = new QStandardItem(QString::number(iRow + 1));
        ImSampleProbeOrder->setTextAlignment(Qt::AlignCenter);
		m_imSampleProbeMode->setItem(iRow, IM_SP_COLUNM_NUM, ImSampleProbeOrder);

		// 项目名称
        auto ImSampleProbeName = new QStandardItem(QString::fromStdString(assayInfo->assayName));
        ImSampleProbeName->setTextAlignment(Qt::AlignCenter);
		m_imSampleProbeMode->setItem(iRow, IM_SP_COLUNM_ASSAY_NAME, ImSampleProbeName);

		// 样本源类型
        auto ImSampleProbeType = new QStandardItem(ConvertTfEnumToQString(item.sampleSourceType));
        ImSampleProbeType->setTextAlignment(Qt::AlignCenter);
		m_imSampleProbeMode->setItem(iRow, IM_SP_COLUNM_SAMPLE_SOURCE_NAME, ImSampleProbeType);

        // 加样量(ul)
        int suckVol = 0;
        auto iter = mapAssaycodeSuckVol.find(item.assayCode);
        if (mapAssaycodeSuckVol.end() != iter)
        {
            suckVol = iter->second;
        }
        auto ImSampleProbeVolume = new QStandardItem(QString::number(suckVol));
        ImSampleProbeVolume->setTextAlignment(Qt::AlignCenter);
        m_imSampleProbeMode->setItem(iRow, IM_SP_COLUNM_VOLUME, ImSampleProbeVolume);

		// id
        auto ImSampleProbeID = new QStandardItem(QString::number(item.id));
        ImSampleProbeID->setTextAlignment(Qt::AlignCenter);
		m_imSampleProbeMode->setItem(iRow, IM_SP_COLUNM_ID, ImSampleProbeID);

		// 勾选框
		auto si = new QStandardItem("");
		si->setData(item.enable, Qt::UserRole + 1);
		m_imSampleProbeMode->setItem(iRow, IM_SP_COLUNM_SELECT, si);
		iRow++;
	}
	// 默认选中第一行
	ui->imSample_tableView->selectRow(0);
}

void SpecialWashDlg::OnAddBtnClicked()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	if (ui->tabWidget->currentWidget()->objectName() == REAGRNT_WIDGET_NAME)
	{
		m_addWashReagentProbeDlg->SetTitleName(tr("新增"));
		m_addWashReagentProbeDlg->ClearCtrls();
		m_addWashReagentProbeDlg->LoadNumToCtrls(QString::number(m_reagentProbeMode->rowCount()+1));
		m_addWashReagentProbeDlg->show();
	}
	else if (ui->tabWidget->currentWidget()->objectName() == CUP_WIDGET_NAME)
	{
		m_addWashCupDlg->SetTitleName(tr("新增"));
		m_addWashCupDlg->ClearCtrls();
		m_addWashCupDlg->LoadNumToCtrls(QString::number(m_cupMode->rowCount() + 1));
		m_addWashCupDlg->show();
	}
	else if (ui->tabWidget->currentWidget()->objectName() == SAMPLE_WIDGET_NAME)
	{
		if (ui->stackedWidget->currentIndex() == CH_INDEX)
		{
			m_addWashSampleProbeDlg->SetTitleName(tr("新增"));
			m_addWashSampleProbeDlg->ClearCtrls();
			m_addWashSampleProbeDlg->LoadNumToCtrls(QString::number(m_sampleProbeMode->rowCount() + 1));
			m_addWashSampleProbeDlg->show();
		}
		else if (ui->stackedWidget->currentIndex() == IM_INDEX)
		{
			m_addWashImSampleProbeDlg->SetTitleName(tr("新增"));
			m_addWashImSampleProbeDlg->ClearCtrls();
			m_addWashImSampleProbeDlg->LoadNumToCtrls(QString::number(m_imSampleProbeMode->rowCount() + 1));
			m_addWashImSampleProbeDlg->show();
		}
		
	}
}

void SpecialWashDlg::OnModifyBtnClicked()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	if (ui->tabWidget->currentWidget()->objectName() == REAGRNT_WIDGET_NAME)
	{
		// 设置标题
		m_addWashReagentProbeDlg->SetTitleName(tr("修改"));

		wrp::CtrlsData data;
		int currentRow		= ui->reagent_tableView->currentIndex().row();
		// 序号
		data.num			= m_reagentProbeMode->data(m_reagentProbeMode->index(currentRow, RG_COLUNM_NUM)).toString();
		// 试剂针
		data.probe			= m_reagentProbeMode->data(m_reagentProbeMode->index(currentRow, RG_COLUNM_REAGENT_NAME)).toString();
		// 污染物
		data.fromAssay		= m_reagentProbeMode->data(m_reagentProbeMode->index(currentRow, RG_COLUNM_FROM_ASSAY_NAME)).toString();
		// 受污染物
		data.toAssay		= m_reagentProbeMode->data(m_reagentProbeMode->index(currentRow, RG_COLUNM_TO_ASSAY_NAME)).toString();
		// 清洗剂
		data.detergentType	= m_reagentProbeMode->data(m_reagentProbeMode->index(currentRow, RG_COLUNM_DETERGEN_NAME)).toString();
		// 数据库主键
		data.id				= m_reagentProbeMode->data(m_reagentProbeMode->index(currentRow, RG_COLUNM_ID)).toInt();

		m_addWashReagentProbeDlg->LoadDataToCtrls(data);
		m_addWashReagentProbeDlg->show();
	}
	else if (ui->tabWidget->currentWidget()->objectName() == CUP_WIDGET_NAME)
	{
		// 设置标题
		m_addWashCupDlg->SetTitleName(tr("修改"));

		wc::CtrlsData data;
		int currentRow		= ui->cup_tableView->currentIndex().row();
		// 序号
		data.num			= m_cupMode->data(m_cupMode->index(currentRow, CUP_COLUNM_NUM)).toString();
		// 项目名称
		data.assayName		= m_cupMode->data(m_cupMode->index(currentRow, CUP_COLUNM_ASSAY_NAME)).toString();
		// R1清洗剂
		data.detergent		= m_cupMode->data(m_cupMode->index(currentRow, CUP_COLUNM_DETERGEN)).toString();
		// 数据库主键
		data.id				= m_cupMode->data(m_cupMode->index(currentRow, CUP_COLUNM_ID)).toInt();

		m_addWashCupDlg->LoadDataToCtrls(data);
		m_addWashCupDlg->show();
	}
	else if (ui->tabWidget->currentWidget()->objectName() == SAMPLE_WIDGET_NAME)
	{
		if (ui->stackedWidget->currentIndex() == CH_INDEX)
		{
			// 设置标题
			m_addWashSampleProbeDlg->SetTitleName(tr("修改"));

			wsp::CtrlsData data;
			int currentRow = ui->sample_tableView->currentIndex().row();
			// 序号
			data.num = m_sampleProbeMode->data(m_sampleProbeMode->index(currentRow, SP_COLUNM_NUM)).toString();
			// 项目名称
			data.assayName = m_sampleProbeMode->data(m_sampleProbeMode->index(currentRow, SP_COLUNM_ASSAY_NAME)).toString();
			// 清洗剂
			data.detergentType = m_sampleProbeMode->data(m_sampleProbeMode->index(currentRow, SP_COLUNM_DETERGEN_NAME)).toString();
			// 样本源
			data.sampleSourceType = m_sampleProbeMode->data(m_sampleProbeMode->index(currentRow, SP_COLUNM_SAMPLE_SOURCE_NAME)).toString();
			// 数据库主键
			data.id = m_sampleProbeMode->data(m_sampleProbeMode->index(currentRow, SP_COLUNM_ID)).toInt();

			m_addWashSampleProbeDlg->LoadDataToCtrls(data);
			m_addWashSampleProbeDlg->show();
		}
		else if(ui->stackedWidget->currentIndex() == IM_INDEX)
		{
			// 设置标题
			m_addWashImSampleProbeDlg->SetTitleName(tr("修改"));

			wsp::ImCtrlsData data;
			int currentRow = ui->imSample_tableView->currentIndex().row();
			// 序号
			data.num = m_imSampleProbeMode->data(m_imSampleProbeMode->index(currentRow, IM_SP_COLUNM_NUM)).toString();
			// 项目名称
			data.assayName = m_imSampleProbeMode->data(m_imSampleProbeMode->index(currentRow, IM_SP_COLUNM_ASSAY_NAME)).toString();
			// 样本源
			data.sampleSourceType = m_imSampleProbeMode->data(m_imSampleProbeMode->index(currentRow, IM_SP_COLUNM_SAMPLE_SOURCE_NAME)).toString();
            // 加样量用量
            data.volume = m_imSampleProbeMode->data(m_imSampleProbeMode->index(currentRow, IM_SP_COLUNM_VOLUME)).toString();
            // 数据库主键
			data.id = m_imSampleProbeMode->data(m_imSampleProbeMode->index(currentRow, IM_SP_COLUNM_ID)).toInt();

			m_addWashImSampleProbeDlg->LoadDataToCtrls(data);
			m_addWashImSampleProbeDlg->show();
		}
		
	}
}

void SpecialWashDlg::OnDelBtnClicked()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	// 弹出提示框
	if (TipDlg(tr("删除"),tr("确定删除选中配置"),TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
	{
		return;
	}

	if (ui->tabWidget->currentWidget()->objectName() == REAGRNT_WIDGET_NAME)
	{
		// 删除勾选的试剂针清洗配置
		auto selectRows = ui->reagent_tableView->selectionModel()->selectedRows();
		for (auto index : selectRows)
		{
			// 组装删除信息
			ch::tf::SpecialWashReagentProbeQueryCond swrpq;
			swrpq.__set_id(m_reagentProbeMode->data(m_reagentProbeMode->index(index.row(), RG_COLUNM_ID)).toInt());

			if (!ch::LogicControlProxy::DeleteSpecialWashReagentProbe(swrpq))
			{
				TipDlg(tr("删除失败"), tr("删除勾选的试剂针清洗配置失败！")).exec();
			}	
            else
            {
                QString probe = m_reagentProbeMode->data(m_reagentProbeMode->index(index.row(), RG_COLUNM_REAGENT_NAME)).toString();
                QString fromAssay = m_reagentProbeMode->data(m_reagentProbeMode->index(index.row(), RG_COLUNM_FROM_ASSAY_NAME)).toString();
                QString toAssay = m_reagentProbeMode->data(m_reagentProbeMode->index(index.row(), RG_COLUNM_TO_ASSAY_NAME)).toString();
                QString cleanType = m_reagentProbeMode->data(m_reagentProbeMode->index(index.row(), RG_COLUNM_DETERGEN_NAME)).toString();
                QString optStr = tr("，试剂针：") + probe + tr("、污染物：") + fromAssay + tr("、受污染物：") + toAssay + tr("、清洗方式：") + cleanType;

                if (!AddOptLog(::tf::OperationType::DEL, tr("删除试剂针特殊清洗") + optStr))
                {
                    ULOG(LOG_ERROR, "Add delete special wash reagent probe operate log failed !");
                }
            }
		}

		// 刷新试剂针清洗配置表格
		LoadReagentDataToTableview();
	}
	else if(ui->tabWidget->currentWidget()->objectName() == CUP_WIDGET_NAME)
	{
		// 删除勾选的反应杯清洗配置
		auto selectRows = ui->cup_tableView->selectionModel()->selectedRows();
		for (auto index : selectRows)
		{
			// 组装删除信息
			ch::tf::SpecialWashCupQueryCond swcq;
			swcq.__set_id(m_cupMode->data(m_cupMode->index(index.row(), CUP_COLUNM_ID)).toInt());

			if (!ch::LogicControlProxy::DeleteSpecialWashCup(swcq))
			{
				TipDlg(tr("删除失败"), tr("删除勾选的反应杯清洗配置失败！")).exec();
			}
            else
            {
                QString assayName = m_cupMode->data(m_cupMode->index(index.row(), CUP_COLUNM_ASSAY_NAME)).toString();
                QString cleanType = m_cupMode->data(m_cupMode->index(index.row(), CUP_COLUNM_ASSAY_NAME)).toString();
                QString optStr = tr("，项目名称：") + assayName + tr("、清洗方式：") + cleanType;

                if (!AddOptLog(::tf::OperationType::DEL, tr("删除反应杯特殊清洗") + optStr))
                {
                    ULOG(LOG_ERROR, "Add delete special wash cup operate log failed !");
                }
            }
		}

		// 刷新反应杯清洗配置表格
		LoadCupDataToTableview();
	}
	else
	{
		if (ui->stackedWidget->currentIndex() == CH_INDEX)
		{
			// 删除勾选的样本针清洗配置
			auto selectRows = ui->sample_tableView->selectionModel()->selectedRows();
			for (auto index : selectRows)
			{
				// 组装删除信息
				ch::tf::SpecialWashSampleProbeQueryCond swspq;
				swspq.__set_id(m_sampleProbeMode->data(m_sampleProbeMode->index(index.row(), SP_COLUNM_ID)).toInt());

				if (!ch::LogicControlProxy::DeleteSpecialWashSampleProbe(swspq))
				{
					TipDlg(tr("删除失败"), tr("删除勾选的样本针清洗配置失败！")).exec();
				}
                else
                {
                    QString assayName = m_sampleProbeMode->data(m_sampleProbeMode->index(index.row(), SP_COLUNM_ASSAY_NAME)).toString();
                    QString sampleType = m_sampleProbeMode->data(m_sampleProbeMode->index(index.row(), SP_COLUNM_SAMPLE_SOURCE_NAME)).toString();
                    QString cleanType = m_sampleProbeMode->data(m_sampleProbeMode->index(index.row(), CUP_COLUNM_DETERGEN)).toString();
                    QString optStr = tr("，项目名称：") + assayName + tr("，样本类型：") + sampleType + tr("、清洗方式：") + cleanType;

                    if (!AddOptLog(::tf::OperationType::DEL, tr("删除样本针特殊清洗") + optStr))
                    {
                        ULOG(LOG_ERROR, "Add delete special wash sample probe operate log failed !");
                    }
                }

			}

			// 刷新样本针清洗配置表格
			LoadSampleDataToTableview(false);
		}
		else if (ui->stackedWidget->currentIndex() == IM_INDEX)
		{
			// 删除勾选的样本针清洗配置
			auto selectRows = ui->imSample_tableView->selectionModel()->selectedRows();
			if (selectRows.isEmpty())
			{
				TipDlg(tr("提示"), tr("请选择需要删除的行！")).exec();
			}

			for (auto index : selectRows)
			{
				// 组装删除信息
				im::tf::SpecialWashSampleProbeQueryCond swspq;
				swspq.__set_id(m_imSampleProbeMode->data(m_imSampleProbeMode->index(index.row(), IM_SP_COLUNM_ID)).toInt());

				if (!im::LogicControlProxy::DeleteSpecialWashSampleProbe(swspq))
				{
					TipDlg(tr("删除失败"), tr("删除勾选的样本针清洗配置失败！")).exec();
				}
			}

			// 刷新样本针清洗配置表格
			LoadImSampleDataToTableview(false);

			ui->imSample_tableView->selectRow(selectRows.first().row() - 1);
		}
	}
}

void SpecialWashDlg::OnTableViewRowChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTableView* view;

    auto wid = ui->tabWidget->currentWidget();

    if (wid->objectName() == REAGRNT_WIDGET_NAME)
    {
        view = ui->reagent_tableView;
    }

    else if (wid->objectName() == CUP_WIDGET_NAME)
    {
        view = ui->cup_tableView;
    }

    else if (wid->objectName() == SAMPLE_WIDGET_NAME)
    {
        view = ui->stackedWidget->currentIndex() == CH_INDEX ? ui->sample_tableView : ui->imSample_tableView;
    }

    QModelIndexList rowList = view->selectionModel()->selectedRows();
	ui->btn_edit->setEnabled(!rowList.empty());
	ui->btn_del->setEnabled(!rowList.empty());
}

void SpecialWashDlg::OnSaveTableViewSelect(const QModelIndex& index)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	if (ui->stackedWidget->currentIndex() == IM_INDEX && index.column() == IM_SP_COLUNM_SELECT)
	{
        im::tf::SpecialWashSampleProbe swsp;
        
        swsp.__set_id(m_imSampleProbeMode->index(index.row(), IM_SP_COLUNM_ID).data().toInt());
        swsp.__set_enable(m_imSampleProbeMode->item(index.row(), index.column())->data().toBool());
        
        if (!im::LogicControlProxy::ModifySpecialWashSampleProbe(swsp))
        {
        	TipDlg(tr("保存失败"), tr("保存勾选状态失败！")).exec();
        }
	}
}

///
/// @brief 保存样本针清洗配置信息
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年10月26日，添加注释
///
void SpecialWashDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
	// 如果当前页不是最后一页则返回（样本针清洗界面）
	if (ui->tabWidget->currentIndex() != ui->tabWidget->count() - 1)
	{
        ULOG(LOG_WARN, "Not last page return.");
		return;
	}
	// 如果是生化的样本针清洗
	if (ui->stackedWidget->currentIndex() == CH_INDEX)
	{
		SaveChSampleProbeWashConfig();
	}
	else if (ui->stackedWidget->currentIndex() == IM_INDEX) // 如果是免疫的样本针清洗
	{
		// 检查是否允许保存免疫样本针清洗配置信息
		if (EnableSaveImSampleProbeWashConfig())
		{
			 // 允许保存免疫样本针清洗配置信息
			SaveImSampleProbeWashConfig();
		}
	}
	else
	{
		ULOG(LOG_INFO, "Do not data to save %s", __FUNCTION__);
        return;
	}
}

///
///  @brief 样本针界面切换到生化界面
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
///
void SpecialWashDlg::OnShowByCh()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
	ui->stackedWidget->setCurrentIndex(CH_INDEX);

    if (ui->tabWidget->currentWidget()->objectName() == SAMPLE_WIDGET_NAME)
    {
        OnUpdateCtrls();
    }
}

///
///  @brief 样本针界面切换到免疫界面
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
///
void SpecialWashDlg::OnShowByIm()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
	ui->stackedWidget->setCurrentIndex(IM_INDEX);

    if (ui->tabWidget->currentWidget()->objectName() == SAMPLE_WIDGET_NAME)
    {
        OnUpdateCtrls();
    }
}

