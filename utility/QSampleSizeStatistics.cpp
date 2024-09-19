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
/// @file     QCalibrationStatistics.h
/// @brief 	  样本量统计界面
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSampleSizeStatistics.h"
#include "ui_QSampleSizeStatistics.h"
#include "shared/QSamplesDonutPie.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"

#include <QStandardItemModel>
#include <QListView>
#include <QCheckBox>

QSampleSizeStatistics::QSampleSizeStatistics(QWidget *parent)
	: QStatisBaseWgt(parent)
{
	ui = new Ui::QSampleSizeStatistics();
	ui->setupUi(this);
	InitBeforeShow();
}

QSampleSizeStatistics::~QSampleSizeStatistics()
{
	delete ui;
}

void QSampleSizeStatistics::showEvent(QShowEvent * event)
{
	// 重置控件
	//OnResetBtnClicked();
}

///
///  @brief 显示前初始化
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
///
void QSampleSizeStatistics::InitBeforeShow()
{
	// 将控件指针传给基类
	m_pLowerDateEit = ui->lower_date_edit;
	m_pUpperDateEit = ui->upper_date_edit;
	m_pDevComboBox = ui->comboBox_dev;
	m_pReagComboBox = ui->comboBox_reag_batch;
	m_pQueryBtn = ui->query_btn;
	m_pResetBtn = ui->reset_btn;

    m_headerData = QStringList({ tr("模块") ,tr("试剂批号") ,tr("项目名称") ,tr("样本量") ,
        tr("常规"), tr("急诊"), tr("质控"), tr("校准") });

	// 绑定起止日期编辑框(间隔时间不超过12个月)
	BindDateEdit(ui->lower_date_edit, ui->upper_date_edit, 12);

	// 设置统计类型
	m_pStatisType = tf::StatisType::type::STATIS_SAMPLE;

	// 初始化模块下拉列表
	InitDevComboBox();

	// 初始化试剂批号下拉列表
	InitReagentLotComboBox();

	// 初始化饼状图
	InitPie();

	// 初始化表格
	InitTable();

	// 连接所有信号槽
	ConnectSlots();

	// 重置控件
	OnResetBtnClicked();
}

///
/// @brief  初始化饼状图控件
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSampleSizeStatistics::InitPie()
{
	m_pQSamplesDonutPie = new QSamplesDonutPie();
	QVBoxLayout* pHLayout = new QVBoxLayout();
	pHLayout->setSpacing(10);
	pHLayout->setMargin(0);
	pHLayout->addWidget(m_pQSamplesDonutPie);
	ui->graphic->setLayout(pHLayout);
	/*
	QColor(27, 125, 245, 255)  常规 （蓝色）
	QColor(31, 181, 88, 255)   急诊 （绿色）
	QColor(124, 63, 209, 255)  紫色  (质控) 
	QColor(19, 191, 191, 255)  校准 （青蓝色）
	*/
	m_pQSamplesDonutPie->SetColors({ QColor(27, 125, 245, 255), QColor(31, 181, 88, 255), QColor(124, 63, 209, 255), QColor(19, 191, 191, 255) });
}

///
/// @brief  初始化表格
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSampleSizeStatistics::InitTable()
{
	m_pTableModel = new QStandardItemModel(ui->tableView);
	m_pTableModel->setHorizontalHeaderLabels(m_headerData);

	ui->tableView->setModel(m_pTableModel);
	ui->tableView->verticalHeader()->hide();
	//ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView->horizontalHeader()->setTextElideMode(Qt::ElideLeft);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(100);
}

///
/// @brief  初始化模块下拉列表
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSampleSizeStatistics::InitDevComboBox()
{
	// 模块下拉列表数据
	m_pDevView = new QListView(ui->comboBox_dev);
	m_pDevModel = new QStandardItemModel(ui->comboBox_dev);
	ui->comboBox_dev->setView(m_pDevView);
	ui->comboBox_dev->setModel(m_pDevModel);
}

///
/// @brief  初始化试剂批号下拉列表
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSampleSizeStatistics::InitReagentLotComboBox()
{
	// 试剂批号下拉列表数据
	m_pReagView = new QListView(ui->comboBox_reag_batch);
	m_pReagModel = new QStandardItemModel(ui->comboBox_reag_batch);
	ui->comboBox_reag_batch->setView(m_pReagView);
	ui->comboBox_reag_batch->setModel(m_pReagModel);
}

///
///  @brief 创建模拟数据（仅展示界面效果使用，非后台真实数据）
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
///
void QSampleSizeStatistics::CreateStimulateData()
{
	// 试剂批号下拉列表数据
	ui->comboBox_reag_batch->clear();
	auto reagBatchView = new QListView(ui->comboBox_reag_batch);
	auto reagBatchModel = new QStandardItemModel(ui->comboBox_reag_batch);
	ui->comboBox_reag_batch->setView(reagBatchView);
	ui->comboBox_reag_batch->setModel(reagBatchModel);
	reagBatchModel->insertRow(0, new QStandardItem(""));
	reagBatchModel->insertRow(1, new QStandardItem(""));
	reagBatchModel->insertRow(2, new QStandardItem(""));
	reagBatchModel->insertRow(3, new QStandardItem(""));
	reagBatchModel->insertRow(4, new QStandardItem(""));

	reagBatchView->setIndexWidget(reagBatchModel->index(0, 0), new QCheckBox(tr("全部"), reagBatchView));
	reagBatchView->setIndexWidget(reagBatchModel->index(1, 0), new QCheckBox("123456", reagBatchView));
	reagBatchView->setIndexWidget(reagBatchModel->index(2, 0), new QCheckBox("123457", reagBatchView));
	reagBatchView->setIndexWidget(reagBatchModel->index(3, 0), new QCheckBox("123458", reagBatchView));
	reagBatchView->setIndexWidget(reagBatchModel->index(4, 0), new QCheckBox("123459", reagBatchView));

	// 模块下拉列表数据
	ui->comboBox_dev->clear();
	auto devView = new QListView(ui->comboBox_dev);
	auto devModel = new QStandardItemModel(ui->comboBox_dev);
	ui->comboBox_dev->setView(devView);
	ui->comboBox_dev->setModel(devModel);
	devModel->insertRow(0, new QStandardItem(""));
	devModel->insertRow(1, new QStandardItem(""));

	devView->setIndexWidget(devModel->index(0, 0), new QCheckBox(tr("全部"), devView));
	devView->setIndexWidget(devModel->index(1, 0), new QCheckBox("M1", devView));

	// 统计表格
	QVector<QStringList> statisticsData = {
		{ "M1", "123456", "FT3", "150", "-", "-", "15", "10%", "-", "-" },
		{ "M1", "123456", "AFP", "150", "0", "0", "30", "20%", "0", "0" },
		{ "M1", "123456", "HBsAg", "100", "10", "10%", "0", "0", "0", "0" },
	};

	for (int i = 0; i < statisticsData.size(); ++i)
	{
		auto item = new QStandardItem(statisticsData[i][0]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 0, item);
		item = new QStandardItem(statisticsData[i][1]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 1, item);
		item = new QStandardItem(statisticsData[i][2]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 2, item);
		item = new QStandardItem(statisticsData[i][3]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 3, item);
		item = new QStandardItem(statisticsData[i][4]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 4, item);
		item = new QStandardItem(statisticsData[i][5]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 5, item);
		item = new QStandardItem(statisticsData[i][6]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 6, item);
		item = new QStandardItem(statisticsData[i][7]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 7, item);
		item = new QStandardItem(statisticsData[i][8]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 8, item);
		item = new QStandardItem(statisticsData[i][9]);
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, 9, item);
	}

	m_pQSamplesDonutPie->SetData({ 150000,90000,210000,150000 });
}

///
/// @brief  查询饼状图数据
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSampleSizeStatistics::QueryPieData()
{
	std::string strStartTime = ui->lower_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00";
	std::string strSendTime = ui->upper_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59";

	std::vector<std::string> vecDeviceSN = GetVecDeviceSN();
	std::vector<std::string> vecReagentLot = GetVecReagentLot();
	::tf::SampleStatisPieQueryCond queryCond;
	queryCond.__set_startTime(strStartTime);
	queryCond.__set_endTime(strSendTime);
	queryCond.__set_vecDeviceSN(vecDeviceSN);
	if (!vecReagentLot.empty())
	{
		queryCond.__set_vecReagentLot(vecReagentLot);
	}
	::tf::SampleStatisPieQueryResp ret;
	if (!DcsControlProxy::GetInstance()->QuerySampleStatisPie(ret, queryCond))
	{
		ULOG(LOG_ERROR, "QuerySampleStatisPie Failed");
		return;
	}
	if (ret.lstSampleStatisPieDatas.empty())
	{
		ULOG(LOG_ERROR, "Pie Query Resp is null");
		return;
	}

	m_pieData = std::make_shared<::tf::SampleStatisPieData>(std::move(ret.lstSampleStatisPieDatas.front()));
	QVector<double> vecPieData;
	vecPieData.append(m_pieData->totalNormalSamples);
	vecPieData.append(m_pieData->totalEmerSamples);
	vecPieData.append(m_pieData->totalQcSamples);
	vecPieData.append(m_pieData->totalCaliSamples);
	m_pQSamplesDonutPie->SetData(vecPieData);
}

///
/// @brief  筛选按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSampleSizeStatistics::OnQueryBtnClicked()
{
	QDateTime startTime(ui->lower_date_edit->date(), QTime(0, 0, 0));
	QDateTime endTime(ui->upper_date_edit->date(), QTime(0, 0, 0));

	if (startTime > endTime)
	{
		TipDlg(tr("值错误"), tr("起始日期不能大于截止日期！")).exec();
		return;
	}

	if (startTime.addYears(1) < endTime)
	{
		TipDlg(tr("值错误"), tr("查找最大时间跨度不得超过一年！")).exec();
		return;
	}

	std::string strStartTime = ui->lower_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00";
	std::string strSendTime = ui->upper_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59";

	std::vector<std::string> vecDeviceSN = GetVecDeviceSN();
	if (vecDeviceSN.empty())
	{
		TipDlg(tr("值错误"), tr("请选择模块！")).exec();
		return;
	}

	std::vector<std::string> vecReagentLot = GetVecReagentLot();

	m_pTableModel->clear();
	m_pTableModel->setHorizontalHeaderLabels(m_headerData);
	::tf::SampleStatisTabDataQueryCond queryCond;
	::tf::SampleStatisTabQueryResp ret;
	queryCond.__set_startTime(strStartTime);
	queryCond.__set_endTime(strSendTime);
	queryCond.__set_vecDeviceSN(vecDeviceSN);
	if (!vecReagentLot.empty())
	{
		queryCond.__set_vecReagentLot(vecReagentLot);
	}

	if (!DcsControlProxy::GetInstance()->QuerySampleStatisTab(ret, queryCond))
	{
		ULOG(LOG_ERROR, "QuerySampleStatisTab Failed");
		return;
	}
	if (ret.lstSampleStatisTabDatas.empty())
	{
		ULOG(LOG_ERROR, "lstSampleStatisTabDatas is null");
		return;
	}
	auto &vecData = ret.lstSampleStatisTabDatas;

	for (int i = 0; i < vecData.size(); ++i)
	{
		auto item = new QStandardItem(QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(vecData[i].deviceSN)));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, DEVICE_SN, item);

		item = new QStandardItem(vecReagentLot.empty() ? "" : QString::fromStdString(vecData[i].reagentLot));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, REAGENT_LOT, item);

		item = new QStandardItem(CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(vecData[i].assayCode));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, ASSAY_CODE, item);

		item = new QStandardItem(QString::number(vecData[i].totalSamples));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, TOTAL_SAMPLES, item);

		item = new QStandardItem(QString::number(vecData[i].norSamples));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, NOR_SAMPLES, item);

		item = new QStandardItem(QString::number(vecData[i].emerSamples));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, EMER_SAMPLES, item);

		item = new QStandardItem(QString::number(vecData[i].qcSamples));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, QC_SAMPLES, item);

		item = new QStandardItem(QString::number(vecData[i].caliSamples));
		item->setTextAlignment(Qt::AlignCenter);
		m_pTableModel->setItem(i, CALI_SAMPLES, item);
	}

	QueryPieData();
}

///
/// @brief  重置按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QSampleSizeStatistics::OnResetBtnClicked()
{
	QStatisBaseWgt::OnResetBtnClicked();

	m_pTableModel->clear();
	m_pTableModel->setHorizontalHeaderLabels(m_headerData);
	m_pQSamplesDonutPie->SetData({0,0,0,0});
	m_pieData = nullptr;

	// 更新模块下拉列表
	UpdateDevList(true);

	// 更新试剂批号下拉列表
	UpdateReagentLotList(true);
}

///
///  @brief 获取打印数据
///
///
///
///  @return	
///
///  @par History: 
///  @li 6889/ChenWei，2024年1月9日，新建函数
///
bool QSampleSizeStatistics::GetStatisticsInfo(SampleStatisticsInfo& Info)
{
	if (m_pieData == nullptr)
	{
		ULOG(LOG_ERROR, "data is null!");
		return false;
	}

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    Info.strDate = (ui->lower_date_edit->text() + " - " + ui->upper_date_edit->text()).toStdString();
    Info.strModule = ui->comboBox_dev->currentText().toStdString();
    Info.strReagentLot = ui->comboBox_reag_batch->currentText().toStdString();
    SampleStatisticsSum Quantity;
    Quantity.strType = tr("数量").toStdString();
    double dTotalOrders = m_pieData->totalNormalSamples + m_pieData->totalEmerSamples + m_pieData->totalQcSamples + m_pieData->totalCaliSamples;
    Quantity.strTotalOrder = QString::number(dTotalOrders).toStdString();
    Quantity.strConvSample = QString::number(m_pieData->totalNormalSamples).toStdString();
    Quantity.strEmSample = QString::number(m_pieData->totalEmerSamples).toStdString();
    Quantity.strQc = QString::number(m_pieData->totalQcSamples).toStdString();
    Quantity.strCali = QString::number(m_pieData->totalCaliSamples).toStdString();
    SampleStatisticsSum Proportion;
    Proportion.strType = tr("占比").toStdString();
    if (dTotalOrders > 0)
    {
        Proportion.strTotalOrder = "100%";
        Proportion.strConvSample = (QString::number(qRound(m_pieData->totalNormalSamples / dTotalOrders * 100)) + tr("%")).toStdString();
        Proportion.strEmSample = (QString::number(qRound(m_pieData->totalEmerSamples / dTotalOrders * 100)) + tr("%")).toStdString();
        Proportion.strQc = (QString::number(qRound(m_pieData->totalQcSamples / dTotalOrders * 100)) + tr("%")).toStdString();
        Proportion.strCali = (QString::number(qRound(m_pieData->totalCaliSamples / dTotalOrders * 100)) + tr("%")).toStdString();
    }

    Info.vecRecordSum.push_back(Quantity);
    Info.vecRecordSum.push_back(Proportion);
    for (int i = 0; i < m_pTableModel->rowCount(); ++i)
    {
        SampleStatistics Record;
        QStandardItem* pSN = m_pTableModel->item(i, DEVICE_SN);
        if (pSN != nullptr)
        {
            Record.strModule = pSN->text().toStdString();
        }

        QStandardItem* pLOT = m_pTableModel->item(i, REAGENT_LOT);
        if (pLOT != nullptr)
        {
            Record.strReagentLot = pLOT->text().toStdString();
        }

        QStandardItem* pCODE = m_pTableModel->item(i, ASSAY_CODE);
        if (pCODE != nullptr)
        {
            Record.strItemName = pCODE->text().toStdString();
        }

        QStandardItem* pTS = m_pTableModel->item(i, TOTAL_SAMPLES);
        if (pTS != nullptr)
        {
            Record.strTotal = pTS->text().toStdString();
        }

        QStandardItem* pNS = m_pTableModel->item(i, NOR_SAMPLES);
        if (pNS != nullptr)
        {
            Record.strConvSample = pNS->text().toStdString();
        }

        QStandardItem* pES = m_pTableModel->item(i, EMER_SAMPLES);
        if (pES != nullptr)
        {
            Record.strEmSample = pES->text().toStdString();
        }

        QStandardItem* pQS = m_pTableModel->item(i, QC_SAMPLES);
        if (pQS != nullptr)
        {
            Record.strQc = pQS->text().toStdString();
        }

        QStandardItem* pCS = m_pTableModel->item(i, CALI_SAMPLES);
        if (pCS != nullptr)
        {
            Record.strCali = pCS->text().toStdString();
        }

        Info.vecRecord.push_back(Record);
    }

    return true;
}

