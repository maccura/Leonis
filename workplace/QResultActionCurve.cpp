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
/// @file     QResultActionCurve.h
/// @brief    反应曲线
///
/// @author   5774/WuHongTao
/// @date     2022年5月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QResultActionCurve.h"
#include "ui_QResultActionCurve.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"

QResultActionCurve::QResultActionCurve(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::QResultActionCurve();
	ui->setupUi(this);
	SetTitleName(tr("反应曲线"));
	// 显示常规曲线信息
	connect(ui->regular_btn, SIGNAL(clicked()), this, SLOT(OnShowRegularCurveInfo()));
	// 显示复查曲线信息
	connect(ui->recheck_btn, SIGNAL(clicked()), this, SLOT(OnShowRecheckCurveInfo()));
	// 关闭对话框
	connect(ui->close_btn, &QPushButton::clicked, this, [&]() {this->close(); });
}

QResultActionCurve::~QResultActionCurve()
{
}

///
/// @brief 显示项目反应曲线等信息
///
/// @param[in]  testItem  项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QResultActionCurve::ShowAssayCurve(tf::TestItem& testItem)
{
    ui->widget->ClearContent();
    m_resultContainer.clear();
	// 查找初测结果
	if (testItem.__isset.firstTestResultKey)
	{
		// 查询项目测试结果
		::ch::tf::AssayTestResultQueryCond resultQueryCond;
		::ch::tf::AssayTestResultQueryResp resultQueryResp;
		resultQueryCond.__set_id(testItem.firstTestResultKey.assayTestResultId);

		// 执行查询条件
		if (ch::c1005::LogicControlProxy::QueryAssayTestResult(resultQueryResp, resultQueryCond)
			&& resultQueryResp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
			&& !resultQueryResp.lstAssayTestResult.empty())
		{
			m_resultContainer[0] = resultQueryResp.lstAssayTestResult[0];
		}
	}

	// 查找复查结果
	if (testItem.__isset.lastTestResultKey)
	{
		// 查询项目测试结果
		::ch::tf::AssayTestResultQueryCond resultQueryCond;
		::ch::tf::AssayTestResultQueryResp resultQueryResp;
		resultQueryCond.__set_id(testItem.lastTestResultKey.assayTestResultId);

		// 执行查询条件
		if (ch::c1005::LogicControlProxy::QueryAssayTestResult(resultQueryResp, resultQueryCond)
			&& resultQueryResp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
			&& !resultQueryResp.lstAssayTestResult.empty())
		{
			m_resultContainer[1] = resultQueryResp.lstAssayTestResult[0];
		}
	}

	ui->regular_btn->setChecked(true);
	// 显示初测结果
	if (m_resultContainer.count(0) > 0)
	{
		ui->widget->ShowAssayResultReactionProcess(m_resultContainer[0]);
	}
}

///
/// @brief 显示常规曲线信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QResultActionCurve::OnShowRegularCurveInfo()
{
	ui->assay_edit->setText("");
	ui->assay_result->setText("");
	ui->widget->ClearContent();

	if (m_resultContainer.count(0) > 0)
	{
		ui->assay_edit->setText("1");

		// 若修改过，则使用修改过的数据
		if (m_resultContainer[0].__isset.concEdit)
		{
			ui->assay_result->setText(QString::fromStdString(m_resultContainer[0].concEdit));
		}
		else
		{
			ui->assay_result->setText(QString::number(m_resultContainer[0].conc));
		}

		// 显示曲线
		ui->widget->ShowAssayResultReactionProcess(m_resultContainer[0]);
	}
}

///
/// @brief 显示复查曲线信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QResultActionCurve::OnShowRecheckCurveInfo()
{
	ui->assay_edit->setText("");
	ui->assay_result->setText("");
	ui->widget->ClearContent();

	if (m_resultContainer.count(1) > 0)
	{
		ui->assay_edit->setText("1");

		// 若修改过，则使用修改过的数据
		if (m_resultContainer[1].__isset.concEdit)
		{
			ui->assay_result->setText(QString::fromStdString(m_resultContainer[1].concEdit));
		}
		else
		{
			ui->assay_result->setText(QString::number(m_resultContainer[1].conc));
		}

		// 显示曲线
		ui->widget->ShowAssayResultReactionProcess(m_resultContainer[1]);
	}
}
