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
/// @file     QAssayResultDetail.cpp
/// @brief    结果详情
///
/// @author   5774/WuHongTao
/// @date     2022年5月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssayResultDetail.h"
#include "ui_QAssayResultDetail.h"
#include <QStandardItemModel>
#include <QList>
#include <boost/assign.hpp>
#include "QResultActionCurve.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/DcsControlProxy.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_ui_control_constants.h"

QAssayResultDetail::QAssayResultDetail(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::QAssayResultDetail();
	ui->setupUi(this);
	m_testItemModel = nullptr;
	m_actionDiglog = nullptr;
	Init();
}

QAssayResultDetail::~QAssayResultDetail()
{
}

///
/// @brief 显示样本测试结果详情
///
/// @param[in]  sampleDb  样本的数据库主键
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QAssayResultDetail::ShowSampleResultDetail(int64_t sampleDb)
{
	// 首先刷新一下界面
	SetDefaultStatus();

	// 查询样本
	::tf::SampleInfoQueryCond sampQryCond;
	::tf::SampleInfoQueryResp sampQryResp;
	sampQryCond.__set_id(sampleDb);
	sampQryCond.__set_containTestItems(true);

	// 执行查询条件
	if (!DcsControlProxy::GetInstance()->QuerySampleInfo(sampQryCond, sampQryResp)
		|| sampQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| sampQryResp.lstSampleInfos.empty())
	{
		return;
	}

	tf::SampleInfo sampleInfo = sampQryResp.lstSampleInfos[0];
	m_sampleInfo = sampleInfo;
	// 显示样本的基本信息
	ShowSampleBasicInfo(sampleInfo);

	// 显示测试项目的列表
	int row = 0;
	for (const auto& testItem : sampleInfo.testItems)
	{
		int column = 0;
		QString strAssayName("");
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(testItem.assayCode);
		if (nullptr == spAssayInfo)
		{
			continue;
		}

		// 插入到map列表中，将项目信息
		m_testItems.insert(std::make_pair(testItem.id, testItem));
		// 项目名称
		strAssayName = QString::fromStdString(spAssayInfo->assayName);
		QStandardItem* assayNameItem = new QStandardItem(strAssayName);
        assayNameItem->setData(static_cast<qlonglong>(testItem.id));
		m_testItemModel->setItem(row++, column++, assayNameItem);
	}
}

///
/// @brief 初始化结果页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QAssayResultDetail::Init()
{
	SetDefaultStatus();
	ui->tableView->horizontalHeader()->setHighlightSections(false);
	ui->tableView->verticalHeader()->setVisible(false);
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	connect(ui->close_btn, &QPushButton::clicked, this, [&]() {this->close(); });
	// 反应曲线显示
	connect(ui->reaction_curve_btn, SIGNAL(clicked()), this, SLOT(OnShowReactCurve()));
	// 项目被点击
	connect(ui->tableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnAssayClicked(const QModelIndex&)));
}

///
/// @brief 设置页面的默认状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QAssayResultDetail::SetDefaultStatus()
{
	if (m_testItemModel == nullptr)
	{
		m_testItemModel = new QStandardItemModel(ui->tableView);
	}

	m_testItemModel->clear();
	QStringList headers;
	headers <<tr("项目");
	m_testItemModel->setHorizontalHeaderLabels(headers);
	ui->tableView->setModel(m_testItemModel);

	// 设置病人信息所有显示为空
	QList<QLabel*> memberLables = ui->patientDetial->findChildren<QLabel*>();
	std::for_each(memberLables.begin(), memberLables.end(), [](QLabel* meber) {meber->setText(""); });

	// 设置样本信息所有显示为空
	QList<QLabel*> sampleLables = ui->widget_2->findChildren<QLabel*>();
	std::for_each(sampleLables.begin(), sampleLables.end(), [](QLabel* meber) {meber->setText(""); });

}

///
/// @brief 显示样本基本信息
///
/// @param[in]  sampleInfo  样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QAssayResultDetail::ShowSampleBasicInfo(tf::SampleInfo& sampleInfo)
{
	// 显示样本基本信息、
	ui->medical_record_label->setText(QString::number(1));
	ui->gender_label->setText("man");
	// 样本id
	ui->sample_id_label->setText(QString::fromStdString(sampleInfo.seqNo));
	// 样本编号
	ui->sample_barcode_label->setText(QString::fromStdString(sampleInfo.barcode));
	// 反应杯号
//	ui->tube_no_label->setText(QString::number(sampleInfo.aliquotNo));
	// 登记时间
	ui->register_time_label->setText(QString::fromStdString(sampleInfo.registerTime));
	// 完成时间
	ui->finished_time_label->setText(QString::fromStdString(sampleInfo.endTestTime));
}

///
/// @brief 获取当前项目信息的列表
///
/// @param[in]  sampleInfo   样本信息
/// @param[in]  testitemInfo  项目信息
///
/// @return 项目信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月27日，新建函数
///
QStringList QAssayResultDetail::GetCurrentTestInfo(int type, tf::SampleInfo sampleInfo, tf::TestItem testitemInfo)
{
	QStringList CurrentResultList;
	
	// 样本量
	if (testitemInfo.__isset.dilutionFactor && testitemInfo.dilutionFactor != 1)
	{
		CurrentResultList << tr("自定义");
		CurrentResultList << QString::number(testitemInfo.dilutionFactor);
	}
	else
	{
		switch (testitemInfo.suckVolType)
		{
			// 减量
			case tf::SuckVolType::type::SUCK_VOL_TYPE_DEC:
				CurrentResultList << tr("减量");
				break;
				// 减量
			case tf::SuckVolType::type::SUCK_VOL_TYPE_STD:
				CurrentResultList << tr("标准量");
				break;
				// 减量
			case tf::SuckVolType::type::SUCK_VOL_TYPE_INC:
				CurrentResultList << tr("增量");
				break;
			default:
				CurrentResultList << tr("自定义");
				break;
		}

		CurrentResultList << "";
	}

	// 位置
	if (sampleInfo.__isset.rack && sampleInfo.__isset.pos)
	{
		CurrentResultList << QString::fromStdString(sampleInfo.rack) + QString::number(sampleInfo.pos);
	}
	else
	{
		CurrentResultList << "";
	}

	// 若是初测结果
	if (type == 0 && testitemInfo.__isset.firstTestResultKey)
	{
		if (auto assayResult = GetAssayResultById(testitemInfo.firstTestResultKey.assayTestResultId))
		{
			// 原始结果
			CurrentResultList << QString::number(assayResult.value().conc, 'f', 3);
			// 当前结果
			if (testitemInfo.__isset.firstTestResultKey)
			{
				CurrentResultList << QString::fromStdString(assayResult.value().concEdit);
			}
			else
			{
				CurrentResultList << QString::number(assayResult.value().conc, 'f', 3);
			}
		}
	}
	// 若是复查结果
	else if(type == 1 && testitemInfo.__isset.lastTestResultKey)
	{
		if (auto assayResult = GetAssayResultById(testitemInfo.lastTestResultKey.assayTestResultId))
		{
			// 原始结果
			CurrentResultList << QString::number(assayResult.value().conc, 'f', 3);
			// 当前结果
			if (testitemInfo.__isset.lastTestResultKey)
			{
				CurrentResultList << QString::fromStdString(assayResult.value().concEdit);
			}
			else
			{
				CurrentResultList << QString::number(assayResult.value().conc, 'f', 3);
			}
		}
	}
	else
	{
		// 填空
		CurrentResultList << ""<<"";
	}

	// 完成时间
	if (sampleInfo.__isset.endTestTime)
	{
		CurrentResultList << QString::fromStdString(sampleInfo.endTestTime);
	}
	else
	{
		CurrentResultList << "";
	}

	// 温度（待完善）
	CurrentResultList << "37";
	// 待完善
	CurrentResultList << "M2";
/*	CommonInformationManager::GetInstance()->GetDeviceFromType(testitemInfo.lastTestResultKey.deviceType);*/

	return CurrentResultList;
}

///
/// @brief 根据数据库主键，获取项目测试结果
///
/// @param[in]  id  数据库主键
///
/// @return 成功则返回
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月11日，新建函数
///
boost::optional<ch::tf::AssayTestResult> QAssayResultDetail::GetAssayResultById(int64_t id)
{
	if (id < 0)
	{
		return boost::none;
	}

	ch::tf::AssayTestResultQueryResp qryResp;
	ch::tf::AssayTestResultQueryCond qryCond;
	qryCond.__set_id(id);
	if (ch::c1005::LogicControlProxy::QueryAssayTestResult(qryResp, qryCond) || qryResp.lstAssayTestResult.empty())
	{
		return boost::none;
	}

	return boost::make_optional(qryResp.lstAssayTestResult[0]);
}

///
/// @brief 根据测试项目结果信息获取试剂信息
///
/// @param[in]  testitemInfo  测试结果
///
/// @return 试剂信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月27日，新建函数
///
QStringList QAssayResultDetail::GetReagentInfo(int testResultId)
{
	QStringList resultList{"","","",""};
	// 查询项目测试结果
	::ch::tf::AssayTestResultQueryCond resultQueryCond;
	::ch::tf::AssayTestResultQueryResp resultQueryResp;
	resultQueryCond.__set_id(testResultId);

	// 执行查询条件
	if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(resultQueryResp, resultQueryCond)
		|| resultQueryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| resultQueryResp.lstAssayTestResult.empty())
	{
		return resultList;
	}

	if (resultQueryResp.lstAssayTestResult[0].reagentKeyInfos.empty())
	{
		return resultList;
	}

	int seqNum = 0;
	// 获取试剂关键信息
	::ch::tf::ReagentKeyInfo keyInfo = resultQueryResp.lstAssayTestResult[0].reagentKeyInfos[0];
	// 瓶号
	resultList[seqNum++] = QString::fromStdString(keyInfo.sn);
	// 批号
	resultList[seqNum++] = QString::fromStdString(keyInfo.lot);

	// 查询耗材信息(根据批号和瓶号查找，理论上不存在批号和瓶号相同的试剂)
	::ch::tf::SuppliesInfoQueryCond supplyQueryCond;
	::ch::tf::SuppliesInfoQueryResp supplyQueryResp;
	supplyQueryCond.__set_lot(keyInfo.lot);
	supplyQueryCond.__set_sn(keyInfo.sn);

	// 执行查询条件
	if (!ch::c1005::LogicControlProxy::QuerySuppliesInfo(supplyQueryResp, supplyQueryCond)
		|| supplyQueryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| supplyQueryResp.lstSuppliesInfo.empty())
	{
		return resultList;
	}

	// 开瓶有效期
	resultList[seqNum++] = QString::fromStdString(supplyQueryResp.lstSuppliesInfo[0].openBottleExpiryTime);
	// 有效期
	resultList[seqNum++] = QString::fromStdString(supplyQueryResp.lstSuppliesInfo[0].expiryTime);
	return resultList;
}

///
/// @brief 项目被点击，槽函数
///
/// @param[in]  index  点击位置
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QAssayResultDetail::OnAssayClicked(const QModelIndex& index)
{
	if (!index.isValid())
	{
		return;
	}

	// 获取对应的项目结果数据库主键
	int64_t dbIten = m_testItemModel->itemData(index)[Qt::UserRole+1].toLongLong();
	if (dbIten < 0)
	{
		return;
	}

	// 找不到直接返回
	if (m_testItems.count(dbIten) <= 0)
	{
		return;
	}

	// 找到对应的项目信息
	tf::TestItem item = m_testItems[dbIten];
	// 获取当前的数据框
	QResultDataShowWidget* dataShow = qobject_cast<QResultDataShowWidget*>(ui->tabWidget->currentWidget());
	if (dataShow == nullptr)
	{
		return;
	}

	dataShow->ClearWidgetContent();

	// 初测结果--并且有初测结果
	if (ui->tabWidget->currentIndex() == 0 && item.__isset.firstTestResultKey)
	{
		auto assayResult = GetAssayResultById(item.firstTestResultKey.assayTestResultId);
		// 0： 设置当前结果
        if (assayResult)
        {
            dataShow->SetModuleInfo(QResultDataShowWidget::CURRENT_RESULT, QStringList{ QString::number(assayResult->conc,'f',3) });
        }

		// 1： 设置项目信息
		dataShow->SetModuleInfo(QResultDataShowWidget::ITEM_SHOW, GetCurrentTestInfo(0, m_sampleInfo, item));
		// 2： 试剂信息
		dataShow->SetModuleInfo(QResultDataShowWidget::REAGENT_SHOW, GetReagentInfo(item.firstTestResultKey.assayTestResultId));
	}
	// 复查结果
	else if (ui->tabWidget->currentIndex() == 1 && item.__isset.lastTestResultKey)
	{
		auto assayResult = GetAssayResultById(item.lastTestResultKey.assayTestResultId);
        if (assayResult)
        {
            dataShow->SetModuleInfo(QResultDataShowWidget::CURRENT_RESULT, QStringList{ QString::number(assayResult->conc,'f',3) });
        }
        
        // 1： 设置项目信息
		dataShow->SetModuleInfo(QResultDataShowWidget::ITEM_SHOW, GetCurrentTestInfo(1, m_sampleInfo, item));
		// 2： 试剂信息
		dataShow->SetModuleInfo(QResultDataShowWidget::REAGENT_SHOW, GetReagentInfo(item.lastTestResultKey.assayTestResultId));
	}
}

///
/// @brief 显示反应曲线
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QAssayResultDetail::OnShowReactCurve()
{
	if (m_actionDiglog == nullptr)
	{
		m_actionDiglog = new QResultActionCurve(this);
	}
	
	QModelIndex index = ui->tableView->currentIndex();
	if (!index.isValid())
	{
		return;
	}

	// 获取对应的项目结果数据库主键
	int64_t dbIten = m_testItemModel->itemData(index)[Qt::UserRole + 1].toLongLong();
	if (dbIten < 0)
	{
		return;
	}

	// 找不到直接返回
	if (m_testItems.count(dbIten) <= 0)
	{
		return;
	}

	// 显示曲线
	m_actionDiglog->ShowAssayCurve(m_testItems[dbIten]);
	m_actionDiglog->show();
}
