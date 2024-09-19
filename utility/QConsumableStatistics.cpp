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
/// @brief 	  试剂/耗材统计界面
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
#include "QConsumableStatistics.h"
#include "ui_QConsumableStatistics.h"
#include "shared/QConsumableCurve.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/gen-cpp/statis_types.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "thrift/DcsControlProxy.h"

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_panner.h>

#include <QScrollBar>
#include <QStandardItemModel>
#include <QListView>
#include <QBuffer>
#define CURVE_NUM (7)   // 曲线数目

QConsumableStatistics::QConsumableStatistics(QWidget *parent)
	: QStatisBaseWgt(parent)
{
	ui = new Ui::QConsumableStatistics();
	ui->setupUi(this);
	InitBeforeShow();
}

QConsumableStatistics::~QConsumableStatistics()
{
	delete ui;
}

void QConsumableStatistics::showEvent(QShowEvent * event)
{
	// 重置界面
	//OnResetBtnClicked();
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年7月26日，新建函数
///
void QConsumableStatistics::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;
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
void QConsumableStatistics::InitBeforeShow()
{
	// 将控件指针传给基类
	m_pLowerDateEit = ui->lower_date_edit;
	m_pUpperDateEit = ui->upper_date_edit;
	m_pDevComboBox = ui->comboBox_dev;
	m_pAssayComboBox = ui->combox_consum_name;
	m_pReagRadioBtn = ui->radioButton_reag;
	m_pQueryBtn = ui->query_btn;
	m_pResetBtn = ui->reset_btn;

	// 绑定起止日期编辑框(间隔时间不超过12个月)
	BindDateEdit(ui->lower_date_edit, ui->upper_date_edit, 12);

	// 设置统计类型
	m_pStatisType = ui->radioButton_reag->isChecked() ? tf::StatisType::type::STATIS_REAG : tf::StatisType::type::STATIS_SUB;

	// 初始化图标
	ui->icon_cali_work->setPixmap(QPixmap(":/Leonis/resource/image/icon_cali_work.png"));
	ui->icon_qc_work->setPixmap(QPixmap(":/Leonis/resource/image/icon_qc_work.png"));
	ui->icon_reag_work->setPixmap(QPixmap(":/Leonis/resource/image/icon_reag_work.png"));
	ui->icon_recheck_work->setPixmap(QPixmap(":/Leonis/resource/image/icon_recheck_work.png"));
	ui->icon_sample_work->setPixmap(QPixmap(":/Leonis/resource/image/icon_sample_work.png"));
	ui->icon_con_size->setPixmap(QPixmap(":/Leonis/resource/image/icon_reag_con.png"));

	// 初始化模块下拉列表
	InitDevComboBox();

	// 初始化试剂/耗材名称下拉列表
	InitSupComboBox();

	// 初始化表格
	InitSupTab();

	// 初始化曲线图
	InitCurve();

	// 连接所有信号槽
	ConnectSlots();

	// 切换到试剂显示模式
	SwitchMode(REAGENT);

	// 重置控件
	OnResetBtnClicked();

	// 隐藏试剂和耗材切换单选框
	ui->widget_reag_or_sup->hide();
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
void QConsumableStatistics::InitDevComboBox()
{
	// 模块下拉列表数据
	m_pDevView = new QListView(ui->comboBox_dev);
	m_pDevModel = new QStandardItemModel(ui->comboBox_dev);
	ui->comboBox_dev->setView(m_pDevView);
	ui->comboBox_dev->setModel(m_pDevModel);
}

///
/// @brief  初始化试剂/耗材下拉列表
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
///
void QConsumableStatistics::InitSupComboBox()
{
	m_pAssayView = new QListView(ui->combox_consum_name);
	m_pAssayModel = new QStandardItemModel(ui->combox_consum_name);
	ui->combox_consum_name->setView(m_pAssayView);
	ui->combox_consum_name->setModel(m_pAssayModel);
}

///
/// @brief  初始化表格
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
///
void QConsumableStatistics::InitSupTab()
{
	m_pModel = new QStandardItemModel(ui->tableView);
	m_pModel->setHorizontalHeaderLabels({ tr("模块"),tr("试剂名称") });
	ui->tableView->setModel(m_pModel);
	ui->tableView->setColumnWidth(0, 79);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);
}

///
/// @brief  初始化曲线图
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
///
void QConsumableStatistics::InitCurve()
{
	// 初始化曲线图
	m_pConsumableCurve = new QConsumableCurve();
	QVBoxLayout* pHLayout = new QVBoxLayout();
	m_pScrollBar = new QScrollBar(Qt::Horizontal);
	UpdateScrollBarRange();
	m_pScrollBar->setSingleStep(1);
	pHLayout->setSpacing(10);
	pHLayout->setMargin(0);
	pHLayout->addWidget(m_pConsumableCurve);
	pHLayout->addWidget(m_pScrollBar);
	ui->graphic->setLayout(pHLayout);
	m_pConsumableCurve->SetShowMode(::tf::GroupByType::type::GROUP_BY_DAY);
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
void QConsumableStatistics::ConnectSlots()
{
	QStatisBaseWgt::ConnectSlots();
	// 点击试剂单选按钮
	connect(ui->radioButton_reag, &QRadioButton::clicked, this, [&](bool checked) {
		SwitchMode(REAGENT);
	});

	// 点击耗材单选按钮
	connect(ui->radioButton_consum, &QRadioButton::clicked, this, [&](bool checked) {
		SwitchMode(CONSUMEABLE);
	});

	// 试剂工作量复选框状态
	connect(ui->checkBox_reag_work, &QCheckBox::stateChanged, this, [&](int checked) {
		bool visibleFlag = checked == Qt::Checked;
		ui->icon_reag_work->setVisible(visibleFlag);
		ui->label_reag_work->setVisible(visibleFlag);
		m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_REAG_WORK, visibleFlag);
	});

	// 样本工作量复选框状态
	connect(ui->checkBox_sample_work, &QCheckBox::stateChanged, this, [&](int checked) {
		bool visibleFlag = checked == Qt::Checked;
		ui->icon_sample_work->setVisible(visibleFlag);
		ui->label_sample_work->setVisible(visibleFlag);
		m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_SAMPLE_WORK, visibleFlag);
	});

	// 复查工作量复选框状态
	connect(ui->checkBox_recheck_work, &QCheckBox::stateChanged, this, [&](int checked) {
		bool visibleFlag = checked == Qt::Checked;
		ui->icon_recheck_work->setVisible(visibleFlag);
		ui->label_recheck_work->setVisible(visibleFlag);
		m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_RECHECK_WORK, visibleFlag);
	});

	// 质控工作量复选框状态
	connect(ui->checkBox_qc_work, &QCheckBox::stateChanged, this, [&](int checked) {
		bool visibleFlag = checked == Qt::Checked;
		ui->icon_qc_work->setVisible(visibleFlag);
		ui->label_qc_work->setVisible(visibleFlag);
		m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_QC_WORK, visibleFlag);
	});

	// 校准工作量复选框状态
	connect(ui->checkBox_cali_work, &QCheckBox::stateChanged, this, [&](int checked) {
		bool visibleFlag = checked == Qt::Checked;
		ui->icon_cali_work->setVisible(visibleFlag);
		ui->label_cali_work->setVisible(visibleFlag);
		m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_CALI_WORK, visibleFlag);
	});

	// 按天显示
	connect(ui->show_by_day, &QRadioButton::clicked, this, [&](bool checked) {
		if (checked)
		{
			ShowModeChanged(::tf::GroupByType::type::GROUP_BY_DAY);
		}
	});

	// 按周显示
	connect(ui->show_by_week, &QRadioButton::clicked, this, [&](bool checked) {
		if (checked)
		{
			ShowModeChanged(::tf::GroupByType::type::GROUP_BY_WEEK);
		}
	});

	// 按月显示
	connect(ui->show_by_mon, &QRadioButton::clicked, this, [&](bool checked) {
		if (checked)
		{
			ShowModeChanged(::tf::GroupByType::type::GROUP_BY_MON);
		}
	});

	// 点击表格的一行
	connect(ui->tableView, &QTableView::clicked, this, &QConsumableStatistics::OnSupTabCliked);

	// 折线图水平滑动条
	connect(m_pScrollBar, &QScrollBar::valueChanged, m_pConsumableCurve, &QConsumableCurve::OnSliderValueChanged);

	// 连接平移信号
	connect(m_pConsumableCurve, &QConsumableCurve::Panned, this, &QConsumableStatistics::HandlePanned);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
///  @brief 切换显示模式
///
///
///  @param[in]   mode  显示模式
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
///
void QConsumableStatistics::SwitchMode(TABLE_SHOW_MODE mode)
{
	m_pStatisType = mode == REAGENT ? tf::StatisType::type::STATIS_REAG : tf::StatisType::type::STATIS_SUB;
	m_pConsumableCurve->SwitchStatisType(m_pStatisType);
	bool visibleFlag = mode == REAGENT;
	ui->tableView->model()->setHeaderData(1, Qt::Horizontal, visibleFlag ? tr("试剂名称") : tr("耗材名称"));
	ui->widget_reag->setVisible(visibleFlag);

	ui->icon_reag_work->setVisible(visibleFlag ? ui->checkBox_reag_work->isChecked() : visibleFlag);
	ui->label_reag_work->setVisible(visibleFlag ? ui->checkBox_reag_work->isChecked() : visibleFlag);
	m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_REAG_WORK, visibleFlag ? ui->checkBox_reag_work->isChecked() : visibleFlag);

	ui->icon_sample_work->setVisible(visibleFlag ? ui->checkBox_sample_work->isChecked() : visibleFlag);
	ui->label_sample_work->setVisible(visibleFlag ? ui->checkBox_sample_work->isChecked() : visibleFlag);
	m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_SAMPLE_WORK, visibleFlag ? ui->checkBox_sample_work->isChecked() : visibleFlag);

	ui->icon_recheck_work->setVisible(visibleFlag ? ui->checkBox_recheck_work->isChecked() : visibleFlag);
	ui->label_recheck_work->setVisible(visibleFlag ? ui->checkBox_recheck_work->isChecked() : visibleFlag);
	m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_RECHECK_WORK, visibleFlag ? ui->checkBox_recheck_work->isChecked() : visibleFlag);

	ui->icon_qc_work->setVisible(visibleFlag ? ui->checkBox_qc_work->isChecked() : visibleFlag);
	ui->label_qc_work->setVisible(visibleFlag ? ui->checkBox_qc_work->isChecked() : visibleFlag);
	m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_QC_WORK, visibleFlag ? ui->checkBox_qc_work->isChecked() : visibleFlag);

	ui->icon_cali_work->setVisible(visibleFlag ? ui->checkBox_cali_work->isChecked() : visibleFlag);
	ui->label_cali_work->setVisible(visibleFlag ? ui->checkBox_cali_work->isChecked() : visibleFlag);
	m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_CALI_WORK, visibleFlag ? ui->checkBox_cali_work->isChecked() : visibleFlag);

	ui->icon_con_size->setVisible(!visibleFlag);
	ui->label_con_size->setVisible(!visibleFlag);
	m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_CONSUMPTION, !visibleFlag);
	m_pConsumableCurve->SetCurveVisable(CURVE_ID::ID_REAG_CONSUMPTION, !visibleFlag);

	// 更新模块下拉列表
	UpdateDevList();

	// 更新试剂耗材名称下拉列表
	UpdateAssayItemList();

	// 更新表格数据
	UpdateTable();

	// 查询第一行的数据
	auto selectedIndexs = ui->tableView->selectionModel()->selectedRows();
	if (selectedIndexs.empty())
	{
		return;
	}
	OnSupTabCliked(selectedIndexs.first());
}

///
/// @brief  更新表格数据
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月14日，新建函数
///
void QConsumableStatistics::UpdateTable()
{
	// 先清除表格内容
	ClearTableData();

	// 准备数据
	std::vector<std::string> vecDeviceSN = GetVecDeviceSN();
	std::vector<int> vecAssayCode = GetVecAssayCode();
	int row = 0;
	for (int i = 0; i < vecDeviceSN.size(); ++i)
	{
		// 获取设备名称
		QString devName = QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(vecDeviceSN[i]));
		if (devName.isEmpty())
		{
			ULOG(LOG_INFO, "The device name is empty. device sn=%s", vecDeviceSN[i]);
			continue;
		}

		for (int j = 0; j < vecAssayCode.size(); ++j)
		{
			QStandardItem * devNameObj = new QStandardItem(devName);
			devNameObj->setTextAlignment(Qt::AlignCenter);
			devNameObj->setData(QString::fromStdString(vecDeviceSN[i]), Qt::UserRole + 1);
			m_pModel->setItem(row, 0, devNameObj);

			QString itemName = ui->radioButton_reag->isChecked() ? CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(vecAssayCode[j]) : ConvertTfEnumToQString(static_cast<::tf::ConsumablesType::type> (vecAssayCode[j]));
			QStandardItem * itemNameObj = new QStandardItem(itemName);
			itemNameObj->setTextAlignment(Qt::AlignCenter);
			itemNameObj->setData(vecAssayCode[j], Qt::UserRole + 1);
			m_pModel->setItem(row, 1, itemNameObj);
			++row;
		}
	}
	if (m_pModel->rowCount() > 0)
	{
		ui->tableView->selectRow(0);
	}
}

///
/// @brief  查询选中的行的曲线数据
///
/// @param[in]  row  选中的行
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月14日，新建函数
///
void QConsumableStatistics::QueryDataByRow(int row)
{
	if (row < 0 || row >= m_pModel->rowCount())
	{
		ULOG(LOG_ERROR, "row is out of range!");
		return;
	}

	std::string strStartTime = ui->lower_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00";
	std::string strSendTime = ui->upper_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59";
	std::string deviceSN = m_pModel->data(m_pModel->index(row, 0), Qt::UserRole + 1).toString().toStdString();
	::tf::GroupByType::type groupByType = m_pConsumableCurve->GetShowMode();

	QVector<QVector<QPointF>> outCurveData;
	if (ui->radioButton_reag->isChecked())
	{
		int assayCode = m_pModel->data(m_pModel->index(row, 1), Qt::UserRole + 1).toInt();
		::tf::StatisReagentCurveQueryCond queryCond;
		::tf::StatisReagentCurveQueryResp ret;
		queryCond.__set_assayCode(assayCode);
		queryCond.__set_deviceSN(deviceSN);
		queryCond.__set_startTime(strStartTime);
		queryCond.__set_endTime(strSendTime);
		queryCond.__set_groupBy(groupByType);

		if (!DcsControlProxy::GetInstance()->QueryStatisReagentCurve(ret, queryCond))
		{
			ULOG(LOG_ERROR, "QueryStatisReagentCurve Failed");
			m_pConsumableCurve->ClearData();
			return;
		}

		if (ret.lstStatisReagentCurve.empty())
		{
			ULOG(LOG_INFO, "lstStatisReagentCurve is null");
			m_pConsumableCurve->ClearData();
			return;
		}

		DecodeReagData(outCurveData, groupByType, ret.lstStatisReagentCurve);
	}
	else
	{
		int subType = m_pModel->data(m_pModel->index(row, 1), Qt::UserRole + 1).toInt();
		::tf::StatisSupCurveQueryCond queryCond;
		::tf::StatisSupCurveQueryResp ret;
		queryCond.__set_supType(subType);
		queryCond.__set_deviceSN(deviceSN);
		queryCond.__set_startTime(strStartTime);
		queryCond.__set_endTime(strSendTime);
		queryCond.__set_groupBy(groupByType);

		if (!DcsControlProxy::GetInstance()->QueryStatisSupCurve(ret, queryCond))
		{
			ULOG(LOG_ERROR, "QueryStatisSupCurve Failed");
			m_pConsumableCurve->ClearData();
			return;
		}
		if (ret.lstStatisSupCurve.empty())
		{
			ULOG(LOG_ERROR, "lstStatisSupCurve is null");
			m_pConsumableCurve->ClearData();
			return;
		}

		DecodeSupData(outCurveData, groupByType, ret.lstStatisSupCurve);
	}

	m_pConsumableCurve->SetData(outCurveData);
}

///
/// @brief  计算X轴对应的值
///
/// @param[in]  groupByType           分组类型
/// @param[in]  statisReagentCurve    曲线数据
///
/// @return X 轴对应的值
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年3月1日，新建函数
///
int QConsumableStatistics::CalCuXValue(const ::tf::GroupByType::type groupByType, const::tf::StatisReagentCurve & statisReagentCurve)
{
	int timeX = -1;
	QDate startDate = ui->lower_date_edit->date();
	switch (groupByType)
	{
	case tf::GroupByType::GROUP_BY_DAY:
	{
		QDate curDate = QDate::fromString(QString::fromStdString(statisReagentCurve.useDate), "yyyy-MM-dd");
		timeX = startDate.daysTo(curDate) + 1;
		break;
	}
	case tf::GroupByType::GROUP_BY_WEEK:
	{
		QDate curDate = QDate(statisReagentCurve.year, 1, 1);
		curDate = curDate.addDays(statisReagentCurve.week * 7);
		timeX = startDate.daysTo(curDate) / 7 + 1;
		break;
	}
	case tf::GroupByType::GROUP_BY_MON:
	{
		QDate curDate = QDate(statisReagentCurve.year, statisReagentCurve.month, 1);
		int yearDiff = curDate.year() - startDate.year();
		int monthDiff = curDate.month() - startDate.month();
		timeX = yearDiff * 12 + monthDiff + 1;
		break;
	}
	default:
		break;
	}
	return timeX;
}

///
/// @brief  解码试剂统计曲线数据
///
/// @param[in]  outCurveData  曲线数据返回值
/// @param[in]  groupByType   分组类型
/// @param[in]  curveData     数据库查询出的数据
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
///
void QConsumableStatistics::DecodeReagData(QVector<QVector<QPointF>> &outCurveData, ::tf::GroupByType::type groupByType, const std::vector<::tf::StatisReagentCurve>& curveData)
{
	for (int i = 0; i < CURVE_NUM; ++i)
	{
		outCurveData.append(QVector<QPointF>());
	}

	for (int i = 0; i < curveData.size(); ++i)
	{
		int timeX = CalCuXValue(groupByType, curveData[i]);

		outCurveData[CURVE_ID::ID_REAG_CONSUMPTION].append(QPointF(timeX, curveData[i].totalConut));
		outCurveData[CURVE_ID::ID_REAG_WORK].append(QPointF(timeX, curveData[i].totalNormalCount + curveData[i].totalRetestCount + curveData[i].totalQcCount + curveData[i].totalCaliCount));
		outCurveData[CURVE_ID::ID_SAMPLE_WORK].append(QPointF(timeX, curveData[i].totalNormalCount));
		outCurveData[CURVE_ID::ID_RECHECK_WORK].append(QPointF(timeX, curveData[i].totalRetestCount));
		outCurveData[CURVE_ID::ID_QC_WORK].append(QPointF(timeX, curveData[i].totalQcCount));
		outCurveData[CURVE_ID::ID_CALI_WORK].append(QPointF(timeX, curveData[i].totalCaliCount));
	}
}

///
/// @brief  解码耗材统计曲线数据
///
/// @param[in]  outCurveData  曲线数据返回值
/// @param[in]  groupByType   分组类型
/// @param[in]  curveData     数据库查询出的数据
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
///
void QConsumableStatistics::DecodeSupData(QVector<QVector<QPointF>> &outCurveData, ::tf::GroupByType::type groupByType, const std::vector<::tf::StatisSupCurve>& curveData)
{
	for (int i = 0; i < CURVE_NUM; ++i)
	{
		outCurveData.append(QVector<QPointF>());
	}

	for (int i = 0; i < curveData.size(); ++i)
	{
		outCurveData[CURVE_ID::ID_CONSUMPTION].append(QPointF(i + 1, curveData[i].totalVol));
	}
}

///
/// @brief  计算X轴坐标范围
///
/// @param[in]  mode		显示模式（按周、按月、按天）
/// @param[in]  startDate   起始日期
/// @param[in]  endDate     截止日期
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
///
int QConsumableStatistics::GetXRange(const::tf::GroupByType::type mode, const QDate & startDate, const QDate & endDate)
{
	if (!startDate.isValid() || !endDate.isValid())
	{
		// 如果日期不合法，返回默认的刻度数12
		return 12;
	}
	switch (mode)
	{
	case tf::GroupByType::GROUP_BY_DAY:
		return startDate.daysTo(endDate) + 1;
	case tf::GroupByType::GROUP_BY_WEEK:
		return int(startDate.daysTo(endDate) / 7 + 0.5) + 1;
	case tf::GroupByType::GROUP_BY_MON:
		return (endDate.year() - startDate.year()) * 12 + endDate.month() - startDate.month() + 1;
	default:
		return 12;
	}
}

///
/// @brief  更新水平滑动条范围
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
///
void QConsumableStatistics::UpdateScrollBarRange()
{
	m_pScrollBar->blockSignals(true);
	int iXRange = GetXRange(m_pConsumableCurve->GetShowMode(), ui->lower_date_edit->date(), ui->upper_date_edit->date());
	m_pScrollBar->setRange(0, iXRange);
	m_pScrollBar->blockSignals(false);
    m_pConsumableCurve->UpdateAxisScaleX(iXRange);
}

///
/// @brief  切换曲线图显示模式（按周、按月、按天）
///
/// @param[in]  mode  显示模式
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
///
void QConsumableStatistics::ShowModeChanged(const::tf::GroupByType::type mode)
{
	m_pConsumableCurve->SetShowMode(mode);
	auto selectedIndexs = ui->tableView->selectionModel()->selectedRows();
	if (selectedIndexs.empty())
	{
		return;
	}
	UpdateScrollBarRange();
	OnSupTabCliked(selectedIndexs.first());
}

///
/// @brief		清除表格内容
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月30日，新建函数
///
void QConsumableStatistics::ClearTableData()
{
	// 清除表格内容，不清除标题
	if (nullptr != m_pModel)
	{
		int iRowCount = m_pModel->rowCount();
		if (iRowCount > 0  )
		{
			m_pModel->removeRows(0, iRowCount);
		}
	}
	// 同时清除表格数据
	if (nullptr != m_pConsumableCurve)
	{
		m_pConsumableCurve->ClearData();
	}
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
void QConsumableStatistics::OnQueryBtnClicked()
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

	std::vector<std::string> vecDeviceSN = GetVecDeviceSN();
	if (vecDeviceSN.empty())
	{
		TipDlg(tr("值错误"), tr("请选择模块！")).exec();
		return;
	}

	std::vector<int> vecSup = GetVecAssayCode();
	if (vecSup.empty())
	{
		TipDlg(tr("值错误"), tr("请选择试剂名称！")).exec();
		return;
	}

	::tf::DeviceSNQueryResp retDev;
	::tf::StatisComboBoxQueryCond queryCondDev;
	queryCondDev.__set_startTime(m_pLowerDateEit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00");
	queryCondDev.__set_endTime(m_pUpperDateEit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59");
	queryCondDev.__set_statisType(static_cast<::tf::StatisType::type> (m_pStatisType));
	if (!DcsControlProxy::GetInstance()->QueryDeviceSN(retDev, queryCondDev))
	{
		ULOG(LOG_ERROR, "QueryDeviceSN Failed");
		TipDlg(tr("查询失败"), tr("查询模块失败！")).exec();
		return;
	}

	if (retDev.lstDeviceSN.empty())
	{
		ULOG(LOG_INFO, "QueryDeviceSN lstDeviceSN is null");
		TipDlg(tr("值错误"), tr("当前日期没有找到使用过试剂的模块，请重新选择日期！")).exec();
		return;
	}

	::tf::AssayCodeQueryResp retAssay;
	::tf::StatisComboBoxQueryCond queryCondAssay;
	queryCondAssay.__set_startTime(m_pLowerDateEit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00");
	queryCondAssay.__set_endTime(m_pUpperDateEit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59");
	queryCondAssay.__set_statisType(static_cast<::tf::StatisType::type> (m_pStatisType));
	if (!DcsControlProxy::GetInstance()->QueryAssayCode(retAssay, queryCondAssay))
	{
		ULOG(LOG_ERROR, "QueryAssayCode Failed");
		TipDlg(tr("值错误"), tr("查询试剂名称失败！")).exec();
		return;
	}

	if (retAssay.lstAssayCode.empty())
	{
		ULOG(LOG_INFO, "QueryAssayCode lstAssayCode is null");
		TipDlg(tr("值错误"), tr("当前日期没有找到试剂试剂的使用记录，请重新选择日期！")).exec();
		return;
	}

	// 更新表格数据
	UpdateTable();

	// 默认选中第一行
	ui->tableView->selectRow(0);

	// 查询第一行的数据
	QueryDataByRow(0);
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
void QConsumableStatistics::OnResetBtnClicked()
{
	QStatisBaseWgt::OnResetBtnClicked();
	m_pConsumableCurve->SetDateInterval(ui->lower_date_edit->date(), ui->upper_date_edit->date());

	// 更新模块下拉列表
	UpdateDevList(true);

	// 更新试剂名称下拉列表
	UpdateAssayItemList(true);

	// 清空表格内容
	ClearTableData();
}

///
/// @brief  点击耗材表的一行
///
/// @param[in]  index  选中行所在的索引
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
///
void QConsumableStatistics::OnSupTabCliked(const QModelIndex & index)
{
	if (!index.isValid())
	{
		ULOG(LOG_ERROR, "index is invalid!");
		return;
	}

	QueryDataByRow(index.row());
}

///
/// @brief  起止日期改变
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月27日，新建函数
///
void QConsumableStatistics::DateEditChanged()
{
	QStatisBaseWgt::DateEditChanged();

	m_pConsumableCurve->SetDateInterval(ui->lower_date_edit->date(), ui->upper_date_edit->date());
	UpdateScrollBarRange();
	
	if (m_pModel->rowCount() == 0)
	{
		return;
	}
}

///
/// @brief  平移处理函数
///
/// @param[in]  dx  x 轴偏移量
/// @param[in]  dy  y 轴偏移量
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
///
void QConsumableStatistics::HandlePanned(int dx, int dy)
{
	int max = m_pScrollBar->maximum();
	int min = m_pScrollBar->minimum();
	int curValue = m_pScrollBar->value();
	int deltaValue = curValue - dx;
	int wid = m_pConsumableCurve->GetXAxisScaleIntervalWidth();
	deltaValue = wid == 0 ? deltaValue : deltaValue / wid;
	if (deltaValue < min)
	{
		m_pScrollBar->blockSignals(true);
		m_pScrollBar->setValue(min);
		m_pScrollBar->blockSignals(false);
		return;
	}

	if (deltaValue > max)
	{
		m_pScrollBar->blockSignals(true);
		m_pScrollBar->setValue(max);
		m_pScrollBar->blockSignals(false);
		return;
	}

	m_pScrollBar->blockSignals(true);
	m_pScrollBar->setValue(deltaValue);
	m_pScrollBar->blockSignals(false);
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
bool QConsumableStatistics::GetStatisticsInfo(ReagentStatisticsInfo& Info)
{
	if (m_pModel->rowCount() == 0)
	{
		ULOG(LOG_ERROR, "data is null!");
		return false;
	}

    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    Info.strDate = (ui->lower_date_edit->text() + " - " + ui->upper_date_edit->text()).toStdString();
    QModelIndex Index = ui->tableView->currentIndex();
    QStandardItem* pModule = m_pModel->item(Index.row(), 0);
    if (pModule != nullptr)
    {
        Info.strModule = pModule->text().toStdString();
    }

    QStandardItem* pName = m_pModel->item(Index.row(), 1);
    if (pName != nullptr)
    {
        Info.strReagentName = pName->text().toStdString();
    }

    
    const QVector<QVector<QPointF>>& Data = m_pConsumableCurve->GetData();
    if (Data.size() < CURVE_NUM)
    {
        return false;
    }

    for (int i = 0; i < Data[CURVE_ID::ID_REAG_WORK].size(); i++)
    {
        ReagentStatistics Record;
        Record.strIndex = m_pConsumableCurve->label(Data[CURVE_ID::ID_REAG_WORK].at(i).x()).toStdString();
        if (ui->checkBox_reag_work->isChecked())
        {
            Record.strReagentCount = QString::number(Data[CURVE_ID::ID_REAG_WORK].at(i).y()).toStdString();
        }
            
        if (ui->checkBox_sample_work->isChecked())
        {
            Record.strSampleCount = QString::number(Data[CURVE_ID::ID_SAMPLE_WORK].at(i).y()).toStdString();          // 样本工作量
        }

        if (ui->checkBox_recheck_work->isChecked())
        {
            Record.strRetestCount = QString::number(Data[CURVE_ID::ID_RECHECK_WORK].at(i).y()).toStdString();          // 复查工作量
        }
        
        if (ui->checkBox_qc_work->isChecked())
        {
            Record.strQcCount = QString::number(Data[CURVE_ID::ID_QC_WORK].at(i).y()).toStdString();              // 质控工作量
        }
        
        if (ui->checkBox_cali_work->isChecked())
        {
            Record.strCaliCount = QString::number(Data[CURVE_ID::ID_CALI_WORK].at(i).y()).toStdString();            // 校准工作量
        }

        Info.vecRecord.push_back(Record);
    }

    QImage printImg(m_pConsumableCurve->size(), QImage::Format_RGB32);
    m_pConsumableCurve->render(&printImg);

    QByteArray ba;
    QBuffer buff(&ba);
    buff.open(QIODevice::WriteOnly);
    printImg.save(&buff, "PNG");
    QString strPrintImg(ba.toBase64());
    Info.strImage = strPrintImg.toStdString();

    return true;
}
