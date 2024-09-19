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
/// @brief 	  结果统计界面
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
#include "QResultStatistics.h"
#include "ui_QResultStatistics.h"
#include "QResultStatisticsRangeSetDlg.h"
#include "QResultRangeCard.h"
#include "QResultStatisticsBarChart.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include <QGridLayout>
#include <QStandardItemModel>
#include <QListView>

QResultStatistics::QResultStatistics(QWidget *parent)
	: QStatisBaseWgt(parent)
	, m_pSetDlg(nullptr)
{
	ui = new Ui::QResultStatistics();
	ui->setupUi(this);
	InitBeforeShow();
}

QResultStatistics::~QResultStatistics()
{
	delete ui;
}

void QResultStatistics::showEvent(QShowEvent * event)
{
	// 更新模块下拉列表
	//UpdateDevList();

	// 更新项目名称下拉列表
	//UpdateAssayItemList();

	// 更新试剂批号下拉列表
	//UpdateReagentLotList();
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
void QResultStatistics::InitBeforeShow()
{
	// 设置图标
	ui->icon_normal->setPixmap(QPixmap(":/Leonis/resource/image/icon_normal_bar_chart.png"));
	ui->icon_recheck->setPixmap(QPixmap(":/Leonis/resource/image/icon_recheck_bar_chart.png"));

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
	m_pStatisType = tf::StatisType::type::STATIS_RESULT;

	// 初始化模块下拉列表
	InitDevComboBox();

	// 初始化项目名称下拉列表
	InitAssayItemComboBox();

	// 初始化试剂批号下拉列表
	InitReagentLotComboBox();

	// 初始化小卡片控件
	InitResultRangeCard();

	// 初始化条形统计图控件
	InitBarChart();

	// 连接所有信号槽
	ConnectSlots();

	// 重置控件
	OnResetBtnClicked();

	CreateStimulateData();
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
void QResultStatistics::InitDevComboBox()
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
void QResultStatistics::InitAssayItemComboBox()
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
void QResultStatistics::InitReagentLotComboBox()
{
	// 试剂批号下拉列表数据
	m_pReagView = new QListView(ui->comboBox_reag_batch);
	m_pReagModel = new QStandardItemModel(ui->comboBox_reag_batch);
	ui->comboBox_reag_batch->setView(m_pReagView);
	ui->comboBox_reag_batch->setModel(m_pReagModel);
}

///
/// @brief  初始化小卡片控件
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
///
void QResultStatistics::InitResultRangeCard()
{
	// 初始化小卡片
	QGridLayout* gLayout = new QGridLayout(ui->widget_cards);
	gLayout->setSpacing(5);
	for (int i = 0; i < 10; ++i)
	{
		auto pCard = new QResultRangeCard(i, ui->widget_cards);
		pCard->SetRecheckVisible(false);
		m_resultRangeCardsMap[i] = (pCard);
		gLayout->addWidget(pCard, i / 2, i % 2);
		gLayout->setSpacing(5);
	}
}

///
/// @brief  初始化条形统计图
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
///
void QResultStatistics::InitBarChart()
{
	// 初始化条形统计图
	m_pBarChart = new QResultStatisticsBarChart(ui->graphic);
	QVBoxLayout *vLayout = new QVBoxLayout(ui->graphic);
	vLayout->setMargin(0);
	vLayout->addWidget(m_pBarChart);
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
void QResultStatistics::ConnectSlots()
{
	QStatisBaseWgt::ConnectSlots();
	// 结果分段设置按钮
	connect(ui->resultRangeBtn, &QPushButton::clicked, this, [&]() {
		if (m_pSetDlg == nullptr)
		{
			m_pSetDlg = new QResultStatisticsRangeSetDlg();
		}
        m_pSetDlg->SetSpinBoxFocus();
		m_pSetDlg->exec();
	});

	// 开启复查统计复选框
	connect(ui->checkBox_recheck_enable, &QCheckBox::stateChanged, this, [&](int checked) {
		OpenRecheckStatistics(checked == Qt::Checked);
	});
}

///
///  @brief 设置统计数据
///
///
///  @param[in]   vecData  统计数据列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
///
void QResultStatistics::SetData(QVector<QVector<double>>& vecData)
{
	double iNormalTotal = 0;
	double iRecheckTotal = 0;

	for (auto data : vecData)
	{
		if (data.size() > 0)
		{
			iNormalTotal += data[0];
		}
		if (data.size() > 1)
		{
			iRecheckTotal += data[1];
		}
	}

	ui->label_3->setText(QString("%1").arg(ui->checkBox_recheck_enable->isChecked() ? iNormalTotal + iRecheckTotal : iNormalTotal));

	for (int i = 0; i < vecData.size(); ++i)
	{
		auto iter = m_resultRangeCardsMap.find(i);
		if (iter == m_resultRangeCardsMap.end())
		{
			continue;
		}

		if (i < vecData.size() - 1)
		{
			iter->second->SetNormalRange(i * 10, (i + 1) * 10);
			iter->second->SetRecheckRange(i * 10, (i + 1) * 10);
		}
		else
		{
			iter->second->SetNormalRange(i * 10, -1);
			iter->second->SetRecheckRange(i * 10, -1);
		}

		if (vecData[i].size() > 0)
		{
			iter->second->SetNormalPer(vecData[i][0] / iNormalTotal * 100);
		}
		if (vecData[i].size() > 1)
		{
			iter->second->SetRecheckPer(vecData[i][1] / iRecheckTotal * 100);
		}
	}

	m_pBarChart->SetSamples(vecData);
}

///
///  @brief 复查统计使能
///
///
///  @param[in]   visible  true 打开 false 关闭
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
///
void QResultStatistics::OpenRecheckStatistics(const bool visible)
{
	for (auto it = m_resultRangeCardsMap.begin(); it != m_resultRangeCardsMap.end(); ++it)
	{
		it->second->SetRecheckVisible(visible);
	}

	QVector<QVector<double>> data = m_data;
	for (auto &it : data)
	{
		if (!visible)
		{
			it.removeAt(1);
		}
	}
	m_pBarChart->SetSpacing(visible ? 60 : 100);
	SetData(data);
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
void QResultStatistics::CreateStimulateData()
{
	m_data =
	{
		{160,90},
		{190,110},
		{190,120},
		{246,130},
		{246,120},
		{300,160},
		{246,120},
		{246,110},
		{246,100},
		{180,120}
	};

	QVector<QVector<double>> data = m_data;
	for (auto &it : data)
	{
		if (!ui->checkBox_recheck_enable->isChecked())
		{
			it.removeAt(1);
		}
	}
	m_pBarChart->SetSpacing(ui->checkBox_recheck_enable->isChecked() ? 60 : 100);
	SetData(data);

	// 模块下拉列表
	ui->comboBox_dev->clear();
	auto devView = new QListView(ui->comboBox_dev);
	auto devModel = new QStandardItemModel(ui->comboBox_dev);
	ui->comboBox_dev->setView(devView);
	ui->comboBox_dev->setModel(devModel);
	devModel->insertRow(0, new QStandardItem(""));
	devModel->insertRow(1, new QStandardItem(""));

	devView->setIndexWidget(devModel->index(0, 0), new QCheckBox(tr("全部"), devView));
	devView->setIndexWidget(devModel->index(1, 0), new QCheckBox("M1", devView));

	// 项目名称下拉列表
	ui->comboBox_item_name->clear();
	auto consumView = new QListView(ui->comboBox_item_name);
	auto consumModel = new QStandardItemModel(ui->comboBox_item_name);
	ui->comboBox_item_name->setView(consumView);
	ui->comboBox_item_name->setModel(consumModel);
	consumModel->insertRow(0, new QStandardItem(""));
	consumModel->insertRow(1, new QStandardItem(""));
	consumModel->insertRow(2, new QStandardItem(""));
	consumModel->insertRow(3, new QStandardItem(""));

	consumView->setIndexWidget(consumModel->index(0, 0), new QCheckBox(tr("全部"), consumView));
	consumView->setIndexWidget(consumModel->index(1, 0), new QCheckBox("FT3", consumView));
	consumView->setIndexWidget(consumModel->index(2, 0), new QCheckBox("AFP", consumView));
	consumView->setIndexWidget(consumModel->index(3, 0), new QCheckBox("HBsAg", consumView));

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
void QResultStatistics::OnQueryBtnClicked()
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

	std::string strStartTime = ui->lower_date_edit->text().toStdString() + " 00:00:00";
	std::string strSendTime = ui->upper_date_edit->text().toStdString() + " 23:59:59";

	// TODO 此函数暂时不响应查询，待ui完善后再实现
	return;

	std::vector<std::string> vecDeviceSN = GetVecDeviceSN();
	if (vecDeviceSN.empty())
	{
		TipDlg(tr("值错误"), tr("请选择模块！")).exec();
		return;
	}

	std::vector<int> vecAssayCode = GetVecAssayCode();
	if (vecAssayCode.empty())
	{
		TipDlg(tr("值错误"), tr("请选择项目！")).exec();
		return;
	}

	std::vector<std::string> vecReagentLot = GetVecReagentLot();
	if (vecReagentLot.empty())
	{
		TipDlg(tr("值错误"), tr("请选择试剂批号！")).exec();
		return;
	}

	bool rangIsValid = false;
	std::vector<::tf::RangQueryCond> lstRangQueryCond = m_pSetDlg->GetRangQueryConds();
	for (auto it : lstRangQueryCond)
	{
		if (it.bUse)
		{
			rangIsValid = true;
		}
	}
	if (!rangIsValid)
	{
		TipDlg(tr("值错误"), tr("请设置合理的结果分段！")).exec();
		return;
	}

	::tf::ResultStatisCurveQueryCond queryCond;
	::tf::ResultStatisCurveQueryResp ret;
	queryCond.__set_startTime(std::move(strStartTime));
	queryCond.__set_endTime(std::move(strSendTime));
	queryCond.__set_vecDeviceSN(std::move(vecDeviceSN));
	queryCond.__set_vecAssayCode(std::move(vecAssayCode));
	queryCond.__set_vecReagentLot(std::move(vecReagentLot));
	queryCond.__set_lstRangQueryCond(lstRangQueryCond);

	if (!DcsControlProxy::GetInstance()->QueryResultStatisCurve(ret, queryCond))
	{
		ULOG(LOG_ERROR, "QueryResultStatisCurve Failed");
		return;
	}
	if (ret.lstResultStatisCurveDatas.empty())
	{
		ULOG(LOG_ERROR, "lstResultStatisCurveDatas is null");
		return;
	}
	auto &vecData = ret.lstResultStatisCurveDatas;
}
