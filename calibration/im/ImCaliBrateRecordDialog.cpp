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

/// @file     ImCaliBrateRecordDialog.h
/// @brief    校准中新增弹出对话框
///
/// @author   5774/WuHongTao
/// @date     2022年2月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "ImCaliBrateRecordDialog.h"
#include "ui_ImCaliBrateRecordDialog.h"

#include "QVariant"
#include <QStandardItemModel>
#include <QTimer>

#include "thrift/im/ImLogicControlProxy.h"
#include "shared/QPostionEdit.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"

#include "boost/shared_ptr.hpp"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"

#include "src/leonis/manager/OperationLogManager.h"

///
/// @brief 新增对话框的构造函数
///     
///
/// @param[in]  parent  
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
ImCaliBrateRecordDialog::ImCaliBrateRecordDialog(QWidget *parent)
    : BaseDlg(parent),
    m_CalibrateConcMode(nullptr),
	m_CalibrateDownMode(nullptr),
    m_press(false),
    m_inputType(InputType::Input_Scan),
    m_iOverTimeCnt(SCAN_TIMER_MAX_OVER_TIME_COUNT)
{
    ui = new Ui::ImCaliBrateRecordDialog();
    SetTitleName(tr("登记"));
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
    ui->lotedit->setValidator(new QIntValidator(0, 9999999));
    ui->lotedit->setMaxLength(7);
    m_caliGrp.__set_assayCode(0);
    
    // 初始化扫码定时器
    m_pScanTimer = new QTimer(this);
    m_pScanTimer->setInterval(SCAN_TIMER_INTERVAL);

	Init();
    ui->radioButton_scanModel->setChecked(true);
    SetScanInputMode();
}

///
/// @brief
///     重置（查看/输入）对话框的table表头
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月10日，新建函数
///
void ImCaliBrateRecordDialog::ResetRecordAssayTable()
{
    if (m_CalibrateConcMode == nullptr)
    {
        m_CalibrateConcMode = new QStandardItemModel(this);
        ui->calibrateBrowse->verticalHeader()->hide();
		ui->calibrateBrowse->horizontalHeader()->setHighlightSections(false);
        ui->calibrateBrowse->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->calibrateBrowse->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->calibrateBrowse->setItemDelegate(new PositiveNumDelegate(this));
        ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN, new CReadOnlyDelegate(this));
        ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_CALIBRATOR_1_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateBrowse->setItemDelegateForColumn(caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN, new CheckBoxDelegate(this));
        ui->calibrateBrowse->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->calibrateBrowse->setModel(m_CalibrateConcMode);
    }

	if (m_CalibrateDownMode == nullptr)
	{
		m_CalibrateDownMode = new QStandardItemModel(this);
		ui->calibrateDown->verticalHeader()->hide();
		ui->calibrateDown->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui->calibrateDown->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
		ui->calibrateDown->setItemDelegate(new PositiveNumDelegate(this));
		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_CALIBRATOR_NAME_COLUMN, new CReadOnlyDelegate(this));
		ui->calibrateDown->setItemDelegateForColumn(caliDownColumnIndex::DOWN_SELECT_COLUMN, new CheckBoxDelegate(this));
        ui->calibrateDown->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui->calibrateDown->setModel(m_CalibrateDownMode);
	}

    m_CalibrateConcMode->clear();
	m_CalibrateDownMode->clear();

    //设置表头
    QStringList headerList;
    headerList << tr("项目名称") << tr("校准品1") << tr("校准品2") << tr("校准品3") << tr("校准品4")
        << tr("校准品5") << tr("校准品6") << tr("单位") << tr("启用");
    m_CalibrateConcMode->setHorizontalHeaderLabels(headerList);
    m_CalibrateConcMode->setRowCount(1);
    m_CalibrateConcMode->setColumnCount(headerList.size());
	//ui->calibrateBrowse->horizontalHeader()->setStyleSheet("QHeaderView::section:last { color: rgb(2,91,199);}");
    m_CalibrateConcMode->setHeaderData(headerList.size() - 1, Qt::Horizontal, QColor::fromRgb(2, 91, 199), Qt::ForegroundRole);

	QStringList headerList_down;
	headerList_down << tr("校准品名称") << tr("校准品批号") << tr("失效日期") << tr("项目名称") << tr("试剂批号") << tr("选择");
	m_CalibrateDownMode->setHorizontalHeaderLabels(headerList_down);
	m_CalibrateDownMode->setRowCount(12);
	m_CalibrateDownMode->setColumnCount(headerList_down.size());
	//ui->calibrateDown->horizontalHeader()->setStyleSheet("QHeaderView::section:last { color: rgb(2,91,199);}");
    m_CalibrateDownMode->setHeaderData(headerList_down.size() - 1, Qt::Horizontal, QColor::fromRgb(2, 91, 199), Qt::ForegroundRole);

    // 列宽自适应
    ResizeTblColToContent(ui->calibrateDown);
    ResizeTblColToContent(ui->calibrateBrowse);
}


///
/// @brief 对话框的初始化操作
///     
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
void ImCaliBrateRecordDialog::Init()
{
	ui->groupBox_input->setStyleSheet("#groupBox_input{ border: 1px solid rgb(181,181,181);}");
	ui->groupBox_out->setStyleSheet("#groupBox_out{ border: 1px solid rgb(181,181,181);}");
	ui->groupBox_screen->setStyleSheet("#groupBox_out{ border: 1px solid rgb(181,181,181);}");

    //设置表头
    ResetRecordAssayTable();
    //初始化按钮
    connect(ui->yes, SIGNAL(clicked()), this, SLOT(addNewCaliIntoDatabase()));
    //退出
	connect(ui->no, &QPushButton::clicked, this, [=] {
		CleanBeforeQuit();
		this->close();
	});

    // 设置当前时间
    //ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->setNull();
    ui->scanEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

	connect(ui->radioButton_scanModel, &QRadioButton::clicked, this, [=]() {
		setInputMode(InputType::Input_Scan);
	});

	connect(ui->radioButton_downModel, &QRadioButton::clicked, this, [=]() {
		setInputMode(InputType::Input_Down);
	});
}


///
/// @brief 退出之前清理
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void ImCaliBrateRecordDialog::CleanBeforeQuit()
{
	// 清除扫描检测
	if (m_barCodeTimer && m_barCodeTimer->isActive())
	{
		m_barCodeTimer->stop();
		m_barCodeTimer.reset();
		m_barCodeTimer = nullptr;
	}

	// 清除扫描框的事件
	if (m_inputType == InputType::Input_Scan)
	{
		disconnect(ui->scanEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnBarCodeEditChanged(const QString&)));
        disconnect(m_pScanTimer, SIGNAL(timeout()), this, SLOT(OnScanTimer()));
		disconnect(ui->scanEdit, SIGNAL(editingFinished()), this, SLOT(OnBarFinished()));      
	}
}

///
/// @brief 添加新样品
///      
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
void ImCaliBrateRecordDialog::addNewCaliIntoDatabase()
{
    // 首先确定是否需要插入
    if ( ui->nameEdit->text().isEmpty() || ui->lotedit->text().isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("校准品名称或者批号不能为空。")));
        pTipDlg->exec();
        return;
    }

    if (m_caliGrp.assayCode == 0)
    {
        return;
    }
    ::tf::ResultLong _return;
    im::LogicControlProxy::AddCaliDocGroup(_return, m_caliGrp);
    if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return;
    }
    m_caliGrp.__set_id(_return.value);
    ::im::tf::CaliDocGroup caliGrp;
    ::im::tf::CaliDocGroupQueryCond qry;
    ::im::tf::CaliDocGroupQueryResp resp;
    qry.__set_id(_return.value);
    if (im::LogicControlProxy::QueryCaliDocGroup(resp, qry) && resp.lstCaliDocGroups.size() == 1)
    {
        emit FinishCaliInfo(resp.lstCaliDocGroups[0]);
    }

	// 添加操作日志
	COperationLogManager::GetInstance()->AddCaliOperationLog(tf::OperationType::type::ADD, m_caliGrp);
    
    CleanBeforeQuit();
    close();
}


///
/// @brief 析构函数
///     
/// @par History:
/// @li 5774/WuHongTao，2020年5月9日，新建函数
///
ImCaliBrateRecordDialog::~ImCaliBrateRecordDialog()
{
}

///
/// @brief
///     显示新增校准品组界面（包括手动输入和扫描输入）
///
/// @param[in]  &  InputType 新增输入校准品的方式
///
/// @par History:
/// @li 8090/YeHuaNing，2021年8月24日，新建函数
///
void ImCaliBrateRecordDialog::setInputMode(const InputType & type)
{
	switch (type)
	{
	case InputType::Input_Scan:		// 扫描输入
		this->SetScanInputMode();
		break;
	case InputType::Input_Down:		// 下载输入
		this->SetDownInputMode();
		break;
	default:
		break;
	}	
}


///
/// @brief 下载输入模式
///
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void ImCaliBrateRecordDialog::SetDownInputMode()
{
	// 设置模式
	m_inputType = InputType::Input_Down;

	// 初始化界面
	ui->stackedWidget->setCurrentWidget(ui->download_page);
	ui->label_symbol->setVisible(true);
    ui->label_symbolName->setVisible(true);
	ui->yes->setText(tr("下载"));
	
    disconnect(ui->scanEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnBarcodeChanged()));
    disconnect(m_pScanTimer, SIGNAL(timeout()), this, SLOT(OnScanTimer()));
    disconnect(ui->scanEdit, SIGNAL(editingFinished()), this, SLOT(OnBarFinished()));
}

///
/// @brief 扫描输入模式
///
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void ImCaliBrateRecordDialog::SetScanInputMode()
{
	// 设置模式
	m_inputType = InputType::Input_Scan;
	// 初始化界面
	ui->stackedWidget->setCurrentWidget(ui->scan_hand_page);
	ui->scan->show();
	ui->scanEdit->setFocus();
	ui->scanEdit->setText("");
	ui->label_errorshow->setText("");
	ui->label_errorshow->show();
	ui->scanEdit->show();
	//ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->setNull();

	ui->nameEdit->setDisabled(true);
	ui->nameEdit->setText("");
	ui->lotedit->setDisabled(true);
	ui->lotedit->setText("");
	ui->dateEdit->setDisabled(true);
	if (!ui->radioButton_scanModel->isChecked())  ui->radioButton_scanModel->setChecked(true);
	ui->label_symbol->setVisible(false);
    ui->label_symbolName->setVisible(false);
    m_CalibrateConcMode->removeRow(0);
    m_CalibrateConcMode->setRowCount(1);

	ui->yes->setText(tr("确定"));

    connect(ui->scanEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnBarcodeChanged()));
    connect(m_pScanTimer, SIGNAL(timeout()), this, SLOT(OnScanTimer()));
    connect(ui->scanEdit, SIGNAL(editingFinished()), this, SLOT(OnBarFinished()));
}


///
/// @brief
///     条码改变（表明正在输入条码）
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void ImCaliBrateRecordDialog::OnBarcodeChanged()
{
    // 重置超时次数
    m_iOverTimeCnt = SCAN_TIMER_MAX_OVER_TIME_COUNT;

    // 如果定时器未开启，则代表条码输入第一个字符
    if (!m_pScanTimer->isActive() && !ui->scanEdit->text().isEmpty())
    {
        // 开启定时器
        m_pScanTimer->start();
        ui->no->setEnabled(false);
        ui->yes->setEnabled(false);
        m_caliGrp.__set_assayCode(0);
        ui->nameEdit->setText(QString::fromStdString(""));
        ui->lotedit->setText(QString::fromStdString(""));
        //ui->dateEdit->setDate(QDate::currentDate());
        ui->dateEdit->setNull();
        m_CalibrateConcMode->removeRow(0);
        m_CalibrateConcMode->setRowCount(1);
    }
}

///
/// @brief 设置界面扫描状态
///
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年5月5日，新建函数
///
void ImCaliBrateRecordDialog::SetScanStatusNoticeInfo(bool isSuccess, QString noticeInfo)
{
	ui->label_errorshow->setText(noticeInfo);

	if (isSuccess)
	{
		// 清空扫描输入框
		ui->scanEdit->clear();
		ui->label_errorshow->setStyleSheet("color:green;font-size:16;");
	}
	else
	{
		ui->label_errorshow->setStyleSheet("color:red;font-size:16;");
	}
}


void ImCaliBrateRecordDialog::OnScanTimer()
{
    // 倒计时结束仍未检测到输入
    if ((--m_iOverTimeCnt) > 0)
    {
        return;
    }
    m_pScanTimer->stop();
    ui->no->setEnabled(true);
    ui->yes->setEnabled(true);
    m_iOverTimeCnt = SCAN_TIMER_MAX_OVER_TIME_COUNT;
    if (ui->scanEdit->text().isEmpty())
    {
        return;
    }
    std::string strBarcode = ui->scanEdit->text().toStdString();

    m_caliGrp.__set_assayCode(0);
    m_caliGrp.caliDocs.clear();
    if (!im::LogicControlProxy::ParseCaliGrpBarcodeInfo(m_caliGrp, strBarcode) || m_caliGrp.assayCode == 0 || m_caliGrp.caliDocs.size() <= 0)
    {
        SetScanStatusNoticeInfo(false, tr("扫描结果失败！"));
        ui->scanEdit->setText("");
        m_caliGrp.__set_assayCode(0);
        return;
    }
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_caliGrp.assayCode);
    if (Q_NULLPTR == spAssayInfo)
    {
        SetScanStatusNoticeInfo(false, tr("该通道号未找到已下载项目信息。"));
        ui->scanEdit->setText("");
        m_caliGrp.__set_assayCode(0);
        return;
    }
    ::im::tf::CaliDocGroupQueryResp _return;
    ::im::tf::CaliDocGroupQueryCond cdgq;
    cdgq.__set_assayCode(m_caliGrp.assayCode);
    cdgq.__set_lot(m_caliGrp.lot);
    im::LogicControlProxy::QueryCaliDocGroup(_return, cdgq);
    // 已经存在该校准品
    if (_return.lstCaliDocGroups.size() > 0)
    {
        SetScanStatusNoticeInfo(false, tr("该校准品已存在。"));
        ui->scanEdit->setText("");
        m_caliGrp.__set_assayCode(0);
        return;
    }
    m_caliGrp.__set_name(spAssayInfo->assayName);
    m_caliGrp.__set_factorySource(1);
    ui->nameEdit->setText(QString::fromStdString(m_caliGrp.name));
    ui->lotedit->setText(QString::fromStdString(m_caliGrp.lot));
    ui->dateEdit->setNull();
    ui->dateEdit->setDate(QDateTime::fromString(QString::fromStdString(m_caliGrp.expiryTime), "yyyy-MM-dd hh:mm:ss").date());

    // 默认所有项目均启用
    QStandardItem* item = new QStandardItem();
    item->setData(true);
    m_CalibrateConcMode->setItem(0, caliBrowseColumnIndex::BROW_ENABLE_SELECT_COLUMN, item);
    // 设置项目名称
    m_CalibrateConcMode->setItem(0, caliBrowseColumnIndex::BROW_ASSAY_NAME_COLUMN, new QStandardItem(QString::fromStdString(m_caliGrp.name)));
    std::string mainUnit = "";
    for (const auto& unit : spAssayInfo->units)
    {
        if (unit.isMain)
        {
            mainUnit = unit.name;
            break;
        }
    }
    // 设置项目单位
    m_CalibrateConcMode->setItem(0, caliBrowseColumnIndex::BROW_MAIN_UNIT_COLUMN, new QStandardItem(QString::fromStdString(mainUnit)));

    int startInd = caliBrowseColumnIndex::BROW_CALIBRATOR_1_COLUMN;
    for (const auto& caliDoc: m_caliGrp.caliDocs)
    {
        m_CalibrateConcMode->setItem(0, startInd++, new QStandardItem(QString::fromStdString(caliDoc.conc)));
    }
    // 列宽自适应
    ResizeTblColToContent(ui->calibrateBrowse);
    // 单元格内容居中显示
    SetTblTextAlign(m_CalibrateConcMode, Qt::AlignCenter);

    SetScanStatusNoticeInfo(true, tr("扫描结果成功！"));
    ui->scanEdit->setText("");
}
