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
/// @brief 	  校准统计界面
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
#include "QCalibrationStatistics.h"
#include "ui_QCalibrationStatistics.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include <QStandardItemModel>
#include <QCheckBox>
#include <QListView>

QCalibrationStatistics::QCalibrationStatistics(QWidget *parent)
	: QStatisBaseWgt(parent)
{
	ui = new Ui::QCalibrationStatistics();
	ui->setupUi(this);
	InitBeforeShow();
}

QCalibrationStatistics::~QCalibrationStatistics()
{
	delete ui;
}

void QCalibrationStatistics::showEvent(QShowEvent * event)
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
void QCalibrationStatistics::InitBeforeShow()
{
	// 将控件指针传给基类
	m_pLowerDateEit = ui->lower_date_edit;
	m_pUpperDateEit = ui->upper_date_edit;
	m_pDevComboBox = ui->comboBox_dev;
	m_pAssayComboBox = ui->comboBox_item_name;
	m_pReagComboBox = ui->comboBox_reag_batch;
	m_pQueryBtn = ui->query_btn;
	m_pResetBtn = ui->reset_btn;

	// 绑定起止日期编辑框(间隔时间不超过12个月)
	BindDateEdit(ui->lower_date_edit, ui->upper_date_edit, 12);

	// 设置统计类型
	m_pStatisType = tf::StatisType::type::STATIS_CALI;

	// 初始化模块下拉列表
	InitDevComboBox();

	// 初始化项目名称下拉列表
	InitAssayItemComboBox();

	// 初始化试剂批号下拉列表
	InitReagentLotComboBox();

	// 初始化校准统计表
	InitCaliStatisTab();

	// 初始化校准失败原因统计表
	InitFailCauseTab();

	// 连接所有信号槽
	ConnectSlots();

	// 重置控件
	OnResetBtnClicked();
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
void QCalibrationStatistics::InitDevComboBox()
{
	// 模块下拉列表数据
	m_pDevView = new QListView(ui->comboBox_dev);
	m_pDevModel = new QStandardItemModel(ui->comboBox_dev);
	ui->comboBox_dev->setView(m_pDevView);
	ui->comboBox_dev->setModel(m_pDevModel);
}

///
/// @brief  初始化项目名称下拉列表
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
///
void QCalibrationStatistics::InitAssayItemComboBox()
{
	// 项目名称下拉列表数据
	m_pAssayView = new QListView(ui->comboBox_item_name);
	m_pAssayModel = new QStandardItemModel(ui->comboBox_item_name);
	ui->comboBox_item_name->setView(m_pAssayView);
	ui->comboBox_item_name->setModel(m_pAssayModel);
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
void QCalibrationStatistics::InitReagentLotComboBox()
{
	// 试剂批号下拉列表数据
	m_pReagView = new QListView(ui->comboBox_reag_batch);
	m_pReagModel = new QStandardItemModel(ui->comboBox_reag_batch);
	ui->comboBox_reag_batch->setView(m_pReagView);
	ui->comboBox_reag_batch->setModel(m_pReagModel);
}

///
/// @brief  初始化校准统计表
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
///
void QCalibrationStatistics::InitCaliStatisTab()
{
	m_pStatisticsModel = new QStandardItemModel(ui->tableView_statistics);
	m_pStatisticsModel->setHorizontalHeaderLabels({ tr("模块"),tr("项目名称"),tr("试剂批号"),tr("校准次数"),tr("校准通过次数"),tr("校准失败次数"),tr("校准通过率") });
	ui->tableView_statistics->setModel(m_pStatisticsModel);
	// 隐藏校准通过次数
	ui->tableView_statistics->setColumnHidden(CALI_SUCCEED_STEPS, true);
	// 隐藏校准失败次数
	ui->tableView_statistics->setColumnHidden(CALI_FAIL_STEPS, true);
	// 隐藏校准通过率
	ui->tableView_statistics->setColumnHidden(CALI_SUCCEED_RATIO, true);
	ui->tableView_statistics->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableView_statistics->verticalHeader()->hide();
	ui->tableView_statistics->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_statistics->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

///
/// @brief  初始化失败原因统计表
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
///
void QCalibrationStatistics::InitFailCauseTab()
{
	m_pReasonModel = new QStandardItemModel(ui->tableView_reason);
	m_pReasonModel->setHorizontalHeaderLabels({ tr("校准失败原因"),tr("次数"),tr("原因占比") });
	ui->tableView_reason->setModel(m_pReasonModel);
	ui->tableView_reason->setColumnWidth(0, 400);
	ui->tableView_reason->setColumnWidth(0, 200);
	ui->tableView_reason->horizontalHeader()->setStretchLastSection(true);
	ui->tableView_reason->verticalHeader()->hide();
	ui->tableView_reason->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_reason->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView_reason->hide();
}

///
///  @brief 连接信号槽
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
///
void QCalibrationStatistics::ConnectSlots()
{
	QStatisBaseWgt::ConnectSlots();
	//connect(ui->tableView_statistics, &QTableView::clicked, this, &QCalibrationStatistics::OnCaliStatisTabCliked);
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
void QCalibrationStatistics::OnQueryBtnClicked()
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

	std::vector<int> vecAssayCode = GetVecAssayCode();
	if (vecAssayCode.empty())
	{
		TipDlg(tr("值错误"), tr("请选择项目名称！")).exec();
		return;
	}

	std::vector<std::string> vecReagentLot = GetVecReagentLot();

	m_pStatisticsModel->clear();
	m_pStatisticsModel->setHorizontalHeaderLabels({ tr("模块"),tr("项目名称"),tr("试剂批号"),tr("校准次数"),tr("校准通过次数"),tr("校准失败次数"),tr("校准通过率") });
	// 隐藏校准通过次数
	ui->tableView_statistics->setColumnHidden(CALI_SUCCEED_STEPS, true);
	// 隐藏校准失败次数
	ui->tableView_statistics->setColumnHidden(CALI_FAIL_STEPS, true);
	// 隐藏校准通过率
	ui->tableView_statistics->setColumnHidden(CALI_SUCCEED_RATIO, true);
	
	::tf::StatisCaliTabQueryCond queryCond;
	::tf::StatisCaliTabQueryResp ret;
	queryCond.__set_startTime(strStartTime);
	queryCond.__set_endTime(strSendTime);
	queryCond.__set_vecDeviceSN(vecDeviceSN);
	queryCond.__set_vecAssayCode(vecAssayCode);
	if (!vecReagentLot.empty())
	{
		queryCond.__set_vecReagentLot(vecReagentLot);
	}
	
	if (!DcsControlProxy::GetInstance()->QueryStatisCaliTab(ret, queryCond))
	{
		ULOG(LOG_ERROR, "QueryStatisCaliTab Failed");
		return;
	}
	if (ret.lstStatisCaliTab.empty())
	{
		ULOG(LOG_ERROR, "lstStatisCaliTab is null");
		return;
	}
	auto &vecData = ret.lstStatisCaliTab;

	for (int i = 0; i < vecData.size(); ++i)
	{
		auto item = new QStandardItem(QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(vecData[i].deviceSN)));
		item->setData(QString::fromStdString(vecData[i].deviceSN), Qt::UserRole + 1);
		item->setTextAlignment(Qt::AlignCenter);
		m_pStatisticsModel->setItem(i, DEVICE_SN, item);

		item = new QStandardItem(CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(vecData[i].assayCode));
		item->setData(vecData[i].assayCode, Qt::UserRole + 1);
		item->setTextAlignment(Qt::AlignCenter);
		m_pStatisticsModel->setItem(i, ASSAY_CODE, item);

		item = new QStandardItem(vecReagentLot.empty() ? "" : QString::fromStdString(vecData[i].reagentLot));
		item->setTextAlignment(Qt::AlignCenter);
		m_pStatisticsModel->setItem(i, REAGENT_LOT, item);

		item = new QStandardItem(QString::number(vecData[i].caliSteps));
		item->setTextAlignment(Qt::AlignCenter);
		m_pStatisticsModel->setItem(i, CALI_STEPS, item);

		item = new QStandardItem(QString::number(vecData[i].caliSucceedSteps));
		item->setTextAlignment(Qt::AlignCenter);
		m_pStatisticsModel->setItem(i, CALI_SUCCEED_STEPS, item);

		item = new QStandardItem(QString::number(vecData[i].caliFailSteps));
		item->setTextAlignment(Qt::AlignCenter);
		m_pStatisticsModel->setItem(i, CALI_FAIL_STEPS, item);

		item = new QStandardItem(QString::number(vecData[i].caliSucceedratio) + "%");
		item->setTextAlignment(Qt::AlignCenter);
		m_pStatisticsModel->setItem(i, CALI_SUCCEED_RATIO, item);
	}
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
void QCalibrationStatistics::OnResetBtnClicked()
{
	QStatisBaseWgt::OnResetBtnClicked();

	m_pStatisticsModel->clear();
	m_pStatisticsModel->setHorizontalHeaderLabels({ tr("模块"),tr("项目名称"),tr("试剂批号"),tr("校准次数"),tr("校准通过次数"),tr("校准失败次数"),tr("校准通过率") });
	
	// 隐藏校准通过次数
	ui->tableView_statistics->setColumnHidden(CALI_SUCCEED_STEPS, true);
	
	// 隐藏校准失败次数
	ui->tableView_statistics->setColumnHidden(CALI_FAIL_STEPS, true);
	
	// 隐藏校准通过率
	ui->tableView_statistics->setColumnHidden(CALI_SUCCEED_RATIO, true);

	// 更新模块下拉列表
	UpdateDevList(true);

	// 更新项目名称下拉列表
	UpdateAssayItemList(true);

	// 更新试剂批号下拉列表
	UpdateReagentLotList(true);
}

///
/// @brief  点击选中校准统计表的一行
///
/// @param[in]  index  选中行所在的索引
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
///
void QCalibrationStatistics::OnCaliStatisTabCliked(const QModelIndex & index)
{
	if (!index.isValid())
	{
		ULOG(LOG_ERROR, "!index.isValid()");
		return;
	}

	std::string strStartTime = ui->lower_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00";
	std::string strSendTime = ui->upper_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59";
	std::string deviceSN = m_pStatisticsModel->item(index.row(), DEVICE_SN)->data(Qt::UserRole + 1).toString().toStdString();
	int assayCode = m_pStatisticsModel->item(index.row(), ASSAY_CODE)->data(Qt::UserRole + 1).toInt();
	std::string reagentLot = m_pStatisticsModel->item(index.row(), REAGENT_LOT)->text().toStdString();

	m_pReasonModel->clear();
	m_pReasonModel->setHorizontalHeaderLabels({ tr("校准失败原因"),tr("次数"),tr("原因占比") });
	ui->tableView_reason->setColumnWidth(0, 400);
	ui->tableView_reason->setColumnWidth(0, 200);

	::tf::StatisCaliFailCauseTabQueryCond queryCond;
	::tf::StatisCaliFailCauseTabQueryResp ret;
	queryCond.__set_startTime(strStartTime);
	queryCond.__set_endTime(strSendTime);
	queryCond.__set_deviceSN(deviceSN);
	queryCond.__set_assayCode(assayCode);
	queryCond.__set_reagentLot(reagentLot);

	if (!DcsControlProxy::GetInstance()->QueryStatisCaliFailCauseTab(ret, queryCond))
	{
		ULOG(LOG_ERROR, "QueryStatisCaliFailCauseTab Failed");
		return;
	}
	if (ret.lstStatisCaliFailCauseTab.empty())
	{
		ULOG(LOG_ERROR, "lstStatisCaliFailCauseTab is null");
		return;
	}
	auto &vecData = ret.lstStatisCaliFailCauseTab;

	for (int i = 0; i < vecData.size(); ++i)
	{
		auto item = new QStandardItem(QString::fromStdString(vecData[i].caliFailCause));
		item->setTextAlignment(Qt::AlignCenter);
		m_pReasonModel->setItem(i, CALI_FAIL_CAUSE, item);

		item = new QStandardItem(QString::number(vecData[i].caliFailnum));
		item->setTextAlignment(Qt::AlignCenter);
		m_pReasonModel->setItem(i, CALI_FAIL_NUM, item);

		item = new QStandardItem(QString::number(vecData[i].failCauseRatio) + "%");
		item->setTextAlignment(Qt::AlignCenter);
		m_pReasonModel->setItem(i, FAIL_CAUSE_RATIO, item);
	}
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
bool QCalibrationStatistics::GetStatisticsInfo(CaliStatisticsInfo& Info)
{
	if (m_pStatisticsModel->rowCount() == 0)
	{
		ULOG(LOG_ERROR, "data is null");
		return false;
	}

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    Info.strDate = (ui->lower_date_edit->text() + " - " + ui->upper_date_edit->text()).toStdString();
    Info.strModule = ui->comboBox_dev->currentText().toStdString();
    Info.strItemName = ui->comboBox_item_name->currentText().toStdString();
    Info.strReagentLot = ui->comboBox_reag_batch->currentText().toStdString();

    bool bHasRecord = false;
    for (int i = 0; i < m_pStatisticsModel->rowCount(); ++i)
    {
        CaliStatistics Record;
        QStandardItem* pSN = m_pStatisticsModel->item(i, DEVICE_SN);
        if (pSN != nullptr)
        {
            Record.strModule = pSN->text().toStdString();
            bHasRecord = true;
        }

        QStandardItem* pAC = m_pStatisticsModel->item(i, ASSAY_CODE);
        if (pAC != nullptr)
        {
            Record.strItemName = pAC->text().toStdString();
            bHasRecord = true;
        }

        QStandardItem* pRL = m_pStatisticsModel->item(i, REAGENT_LOT);
        if (pRL != nullptr)
        {
            Record.strReagentLot = pRL->text().toStdString();
            bHasRecord = true;
        }

        QStandardItem* pCS = m_pStatisticsModel->item(i, CALI_STEPS);
        if (pCS != nullptr)
        {
            Record.strCaliNum = pCS->text().toStdString();
            bHasRecord = true;
        }

        Info.vecRecord.push_back(Record);
    }

    return bHasRecord;
}

