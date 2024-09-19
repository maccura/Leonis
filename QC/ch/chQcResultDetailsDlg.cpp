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
/// @file     chQcResultDetailsDlg.cpp
/// @brief    生化质控结果详情
///
/// @author   8580/GongZhiQiang
/// @date     2023年2月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年2月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "chQcResultDetailsDlg.h"
#include "ui_chQcResultDetailsDlg.h"
#include <QDateTime>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>

#define _MATH_DEFINES_DEFINED
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "shared/CommonInformationManager.h"
#include "shared/mccustomscaledraw.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/uicommon.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DataAlarmCodeDefine.h"
#include "datetimefmttool.h"

#define _MATH_DEFINES_DEFINED

CH_QC_DETAIL_PLOT::QReactionCurveData::QReactionCurveData()
	:QObject()
{
	m_curves = Q_NULLPTR;
	m_plotMarker = Q_NULLPTR;
	m_plotInnerMarker = Q_NULLPTR;
	m_plotMessageMarker = Q_NULLPTR;
	m_plotMessageMarkerText = Q_NULLPTR;
	m_markerPixmap.load(":/Leonis/resource/image/bk-curve-arrow.png");
	m_markerTextPixmap.load(":/Leonis/resource/image/bk-curve-wave.png");
}

void CH_QC_DETAIL_PLOT::QReactionCurveData::showCurve(DetectCurveData& curveData)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (curveData.qwtPlot == Q_NULLPTR)
	{
		return;
	}

	m_curveData = curveData;
	m_curves = std::make_shared<QwtPlotCurve>();
	m_curves->setZ(19);
	// 设置坐标轴
	m_curves->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	m_curves->setTitle(curveData.curveName);
	// 设置图例属性
	//m_curves->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, !curveData.curveName.isEmpty());
	// 设置曲线点符号
	QwtSymbol* pCircleSymbol = new QwtSymbol(QwtSymbol::Ellipse);
	pCircleSymbol->setBrush(QBrush(curveData.color));
	pCircleSymbol->setPen(QPen(Qt::NoPen));
	pCircleSymbol->setSize(12);
	m_curves->setSymbol(pCircleSymbol);
	// 设置曲线样式为点
	m_curves->setStyle(QwtPlotCurve::NoCurve);
	m_curves->setSamples(curveData.detectPointers, curveData.absorbValues);
	m_curves->attach(curveData.qwtPlot);

	m_plotMessageMarker = std::make_shared<QwtPlotMarker>();
	// 	m_makerSymbolInfo = new QwtSymbol(QwtSymbol::Pixmap);
	// 	m_makerSymbolInfo->setPixmap(m_markerPixmap);
	// 	m_makerSymbolInfo->setBrush(QBrush(QColor(0,0,255,128)));
	// 	//pMakerSymbolInfo->setSize(120,50);
	// 	m_plotMessageMarker->setSymbol(m_makerSymbolInfo);
	m_plotMessageMarker->attach(curveData.qwtPlot);
	m_plotMessageMarker->setZ(22);
	m_plotMessageMarker->hide();

	m_plotMessageMarkerText = std::make_shared<QwtPlotMarker>();
	QwtSymbol* tmpMarker = new QwtSymbol(QwtSymbol::Pixmap);
	tmpMarker->setPixmap(m_markerTextPixmap);
	tmpMarker->setBrush(QBrush(QColor(0, 0, 255, 128)));
	m_plotMessageMarkerText->setSymbol(tmpMarker);
	m_plotMessageMarkerText->attach(curveData.qwtPlot);
	m_plotMessageMarkerText->setZ(22);
	m_plotMessageMarkerText->hide();

	// 渐变色marker
	QwtSymbol* pMakerInnerSymbol = new QwtSymbol(QwtSymbol::Ellipse);
	pMakerInnerSymbol->setBrush(QBrush(Qt::white));
	pMakerInnerSymbol->setPen(QPen(Qt::NoPen));
	pMakerInnerSymbol->setSize(14);
	m_plotInnerMarker = std::make_shared<QwtPlotMarker>();
	m_plotInnerMarker->setSymbol(pMakerInnerSymbol);
	m_plotInnerMarker->attach(curveData.qwtPlot);
	m_plotInnerMarker->setZ(17);
	m_plotInnerMarker->hide();

	// 外层marker
	QwtSymbol* pMakerSymbol = new QwtSymbol(QwtSymbol::Ellipse);
	pMakerSymbol->setBrush(QBrush(curveData.markerColor));
	pMakerSymbol->setPen(QPen(Qt::NoPen));
	pMakerSymbol->setSize(26);
	m_plotMarker = std::make_shared<QwtPlotMarker>();
	m_plotMarker->setSymbol(pMakerSymbol);
	m_plotMarker->attach(curveData.qwtPlot);
	m_plotMarker->setZ(18);
	m_plotMarker->hide();
}

void CH_QC_DETAIL_PLOT::QReactionCurveData::reset()
{
	if (m_curves != Q_NULLPTR)
	{
		m_curves->detach();
	}


	if (m_plotMarker != Q_NULLPTR)
	{
		m_plotMarker->detach();
	}

	if (m_plotInnerMarker != Q_NULLPTR)
	{
		m_plotInnerMarker->detach();
	}

	if (m_plotMessageMarker != Q_NULLPTR)
	{
		m_plotMessageMarker->detach();
	}

	m_curves = Q_NULLPTR;
	m_plotMarker = Q_NULLPTR;
	m_plotInnerMarker = Q_NULLPTR;
	m_plotMessageMarker = Q_NULLPTR;

	if (m_curveData.qwtPlot != Q_NULLPTR)
	{
		m_curveData.qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
		m_curveData.qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
		m_curveData.qwtPlot->replot();
		m_curveData.qwtPlot->show();
	}
}

double CH_QC_DETAIL_PLOT::QReactionCurveData::GetAvge(QVector<double>& resultVec)
{
	double min = 0;
	double max = 0;
	for (auto result : resultVec)
	{
		if (min > result)
		{
			min = result;
		}
		else if (result > max)
		{
			max = result;
		}
	}

	return (max - min) / 10;
}

bool CH_QC_DETAIL_PLOT::QReactionCurveData::showDataDetail(double x, QString showMessage)
{
	if (m_plotMessageMarker != Q_NULLPTR)
	{
		m_plotMessageMarker->setXValue(x);
		m_plotMessageMarkerText->setXValue(x);
		if ((x <= 0) || (x > 34) || x > m_curveData.absorbValues.size())
		{
			return false;
		}

		int iValXLeft = m_curveData.qwtPlot->transform(QwtPlot::xBottom, x) - 180;
		if (x <= 2 && iValXLeft < 0)
		{
			m_plotMessageMarkerText->setXValue(3);
		}

		int iValXRight = m_curveData.qwtPlot->transform(QwtPlot::xBottom, x) + 180;
		int width = m_curveData.qwtPlot->width();
		if (x >= (m_curveData.absorbValues.size() - 1))
		{
			m_plotMessageMarkerText->setXValue(m_curveData.absorbValues.size() - 2);
		}

		auto y = m_curveData.absorbValues[x - 1];
		int iValY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) - 52;
		int arrowY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) - 13;
		if (iValY <= 15)
		{
			iValY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) + 52;
			arrowY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) + 13;
			QMatrix rm;
			rm.rotate(180);
			auto pixmap = m_markerPixmap.transformed(rm);
			QwtSymbol* tmp = new QwtSymbol(QwtSymbol::Pixmap);
			tmp->setPixmap(pixmap);
			tmp->setBrush(QBrush(QColor(0, 0, 255, 128)));
			m_plotMessageMarker->setSymbol(tmp);
		}
		else
		{
			QwtSymbol* tmp = new QwtSymbol(QwtSymbol::Pixmap);
			tmp->setPixmap(m_markerPixmap);
			tmp->setBrush(QBrush(QColor(0, 0, 255, 128)));
			m_plotMessageMarker->setSymbol(tmp);
		}

		y = m_curveData.qwtPlot->invTransform(QwtPlot::yLeft, iValY);
		double arrowy = m_curveData.qwtPlot->invTransform(QwtPlot::yLeft, arrowY);

		QFont font;
		font.setPixelSize(14);
		QwtText textShow;
		textShow.setFont(font);
		textShow.setColor(Qt::white);
		textShow.setText(showMessage);
		m_plotMessageMarkerText->setLabel(textShow);
		m_plotMessageMarkerText->setYValue(y);
		m_plotMessageMarker->setYValue(arrowy);
		m_plotMessageMarker->show();
		m_plotMessageMarkerText->show();
	}

	return true;
}

void CH_QC_DETAIL_PLOT::QReactionCurveData::showMarker(double x, double y)
{
	if (m_plotMarker != Q_NULLPTR)
	{
		m_plotMarker->setXValue(x);
		if ((x <= 0) || (x > 34) || x > m_curveData.absorbValues.size())
		{
			return;
		}

		m_plotMarker->setYValue(m_curveData.absorbValues[x - 1]);
		m_plotMarker->show();
	}

	if (m_plotInnerMarker != Q_NULLPTR)
	{
		m_plotInnerMarker->setXValue(x);
		if ((x <= 0) || (x > 34) || x >= m_curveData.absorbValues.size())
		{
			return;
		}

		m_plotInnerMarker->setYValue(m_curveData.absorbValues[x - 1]);
		m_plotInnerMarker->show();
	}
}

QString CH_QC_DETAIL_PLOT::QReactionCurveData::GetCurrentWaveAbsorbs(double x)
{
	QString ret = m_curveData.curveName;
	if ((x < 0) || (x > 34) || x >= m_curveData.absorbValues.size())
	{
		return ret + '\n';
	}
	else
	{
		return ((ret += QString::number(m_curveData.absorbValues[x], 'f', 2)) + '\n');
	}
}


chQcResultDetailsDlg::chQcResultDetailsDlg(QWidget *parent)
	: BaseDlg(parent),
	m_chReusltID(-1),
	m_iseResultID(-1),
	m_bCalculated(true),
	m_enQcState(QC_STATE::QC_STATE_NONE),
	m_controlModify(false),
	m_shiftKey(false),
	m_plotMagnifier(Q_NULLPTR),
	m_plotPanner(Q_NULLPTR),
	m_pSplitGrid(Q_NULLPTR)
{
    ui = new Ui::chQcResultDetailsDlg();
	ui->setupUi(this);

	SetTitleName(tr("结果详情"));

	// 初始化子控件
	initChildControl();
	// 初始化绘图
	initPlot();
	// 初始化信号槽
	initConnect();

}

chQcResultDetailsDlg::~chQcResultDetailsDlg()
{
}

///
/// @brief 初始化信号槽
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年10月26日，新建函数
///
void chQcResultDetailsDlg::initConnect()
{
	// 恢复原图比例信号槽
	connect(ui->reset_curve_Ch, &QPushButton::clicked, this, &chQcResultDetailsDlg::ResetCurve);
	// 关闭信号槽
	connect(ui->CloseBtn_Ch, &QPushButton::clicked, this, [this] { this->reject(); });
	// 关闭信号槽
	connect(ui->CloseBtn_ISE, &QPushButton::clicked, this, [this] { this->reject(); });

}

///
/// @brief 初始化子控件
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年10月26日，新建函数
///
void chQcResultDetailsDlg::initChildControl()
{

	//// 吸光度文字旋转
	//{
	//	QGraphicsScene* sence = new QGraphicsScene(ui->graphicsLabel);
	//	QFont cosFont = font();
	//	cosFont.setPixelSize(16);
	//	cosFont.setFamily("Source Han Sans CN Regular");
	//	//sence->setFont(cosFont);
	//	//sence->setForegroundBrush(QColor::fromRgb(0x56,0x56,0x56));
	//	auto textItem = sence->addText(tr("吸光度"), cosFont);
	//	textItem->setDefaultTextColor(QColor::fromRgb(0x56, 0x56, 0x56));
	//	ui->graphicsLabel->setScene(sence);
	//	ui->graphicsLabel->rotate(-90);
	//}

}

///
/// @brief 绘图初始化
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::initPlot()
{
	// plot自动更新
	ui->qwtPlot_Ch->setAutoReplot(true);
	// 设置背景色
	ui->qwtPlot_Ch->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

	// 初始化网格分割线
	m_pSplitGrid = std::make_shared<CH_QC_DETAIL_PLOT::CustomGrid>();
	m_pSplitGrid->setMinorPen(Qt::transparent);
	m_pSplitGrid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1, Qt::DashLine);
	m_pSplitGrid->attach(ui->qwtPlot_Ch);

	// 初始化标记选中线
	m_plotMarker = new QwtPlotMarker();
	m_plotMarker->setLineStyle(QwtPlotMarker::VLine);
	m_plotMarker->setLinePen(QColor(0xfa, 0x64, 0x31, 128), 2);
	m_plotMarker->attach(ui->qwtPlot_Ch);
	m_plotMarker->setZ(20);

	// 安装事件过滤器
	ui->qwtPlot_Ch->installEventFilter(this);

	// 纵坐标图标
	QFont Font;
	Font.setPixelSize(16);
	QwtText title;
	title.setFont(Font);
	title.setText(tr("吸光度") + "\n");
	ui->qwtPlot_Ch->setAxisTitle(QwtPlot::yLeft, title);
	ui->qwtPlot_Ch->setStyleSheet("font-size: 16px;");
	ui->qwtPlot_Ch->setAxisLabelAlignment(QwtPlot::yLeft, Qt::AlignLeft);
	ui->qwtPlot_Ch->setContentsMargins(3, 3, 3, 3);

	// 横坐标只能为整型
	McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
	if (pScaleDrawX)
	{
		pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
		pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	}
	ui->qwtPlot_Ch->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);

	// 纵坐标刻度值为彩色
	McCustomColorScaleDraw* pScaleDrawYLeft = new McCustomColorScaleDraw();
	pScaleDrawYLeft->SetLabelColor(QColor(0x1b, 0x7d, 0xf5));
	ui->qwtPlot_Ch->setAxisScaleDraw(QwtPlot::yLeft, pScaleDrawYLeft);

	// 坐标无刻度
	ui->qwtPlot_Ch->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	ui->qwtPlot_Ch->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	ui->qwtPlot_Ch->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	ui->qwtPlot_Ch->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

	// 设置边框
	ui->qwtPlot_Ch->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");
	// 插入图例
	//ui->qwtPlot_Ch->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
	ui->qwtPlot_Ch->setAxisAutoScale(QwtPlot::yLeft);
	ui->qwtPlot_Ch->setAxisAutoScale(QwtPlot::xBottom);
	// 放大缩小
	m_plotMagnifier = new QwtPlotMagnifier(ui->qwtPlot_Ch->canvas());
	// 平移
	m_plotPanner = new QwtPlotPanner(ui->qwtPlot_Ch->canvas());

}

///
/// @brief 设置生化质控结果id
///
/// @param[in]  chQcReusltID 生化质控结果ID  
///
/// @return true:设置成功 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
///
bool chQcResultDetailsDlg::setChAssayResultID(const int& chReusltID, const bool& bCalculated, const QC_STATE& enQcState)
{
	if (chReusltID < 0)
	{
		return false;
	}
	// 清空生化界面
	clearViewCH();
	this->m_chReusltID = chReusltID;
	m_bCalculated = bCalculated;
	m_enQcState = enQcState;

	// 当前界面设置为生化界面
	ui->Device_StackedWidget->setCurrentWidget(ui->page_Ch);

	// 查找测试结果信息
	ch::tf::AssayTestResultQueryResp assayTestQryResp;
	ch::tf::AssayTestResultQueryCond assayTestQryCond;

	assayTestQryCond.__set_id(m_chReusltID);
	if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(assayTestQryResp, assayTestQryCond)
		|| assayTestQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| assayTestQryResp.lstAssayTestResult.size() != 1)
	{
		ULOG(LOG_ERROR, "%s():QueryAssayTestResult() Failed!", __FUNCTION__);
		return false;
	}

	// 保存测试结果信息
	m_chAssayResult = assayTestQryResp.lstAssayTestResult[0];

	// 更新界面信息
	if (!updateViewCH())
	{
		ULOG(LOG_ERROR, "%s():updateViewCH Failed!", __FUNCTION__);
		return false;
	}

	return true;

}

///
/// @brief 设置ISE质控结果id
///
/// @param[in]  iseQcReusltID ISE质控结果ID  
///
/// @return true:设置成功 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
///
bool chQcResultDetailsDlg::setIseAssayResultID(const int& iseReusltID, const bool& bCalculated, const QC_STATE& enQcState)
{
	if (iseReusltID < 0)
	{
		return false;
	}

	// 清空生化界面
	clearViewISE();
	this->m_iseResultID = iseReusltID;
	m_bCalculated = bCalculated;
	m_enQcState = enQcState;

	// 当前界面设置为ISE界面
	ui->Device_StackedWidget->setCurrentWidget(ui->page_ISE);

	// 查找测试结果信息
	ise::tf::AssayTestResultQueryResp assayTestQryResp;
	ise::tf::AssayTestResultQueryCond assayTestQryCond;

	assayTestQryCond.__set_id(m_iseResultID);
	if (!ise::ise1005::LogicControlProxy::QueryAssayTestResult(assayTestQryResp, assayTestQryCond)
		|| assayTestQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| assayTestQryResp.lstAssayTestResult.size() != 1)
	{
		ULOG(LOG_ERROR, "%s():QueryAssayTestResult() Failed!", __FUNCTION__);
		return false;
	}
	// 保存测试结果信息
	m_iseAssayResult = assayTestQryResp.lstAssayTestResult[0];

	// 更新界面信息
	if (!updateViewISE())
	{
		ULOG(LOG_ERROR, "%s():updateViewISE Failed!", __FUNCTION__);
		return false;
	}

	return true;
}

///
/// @brief 获取项目模块名称
///
/// @param[in]  deviceSN  设备编号
///
/// @return 模块名称
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
QString chQcResultDetailsDlg::getModuleIndex(const std::string deviceSN)
{
	QString moduleIndex = "";
	auto device = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSN);
	if (device)
	{
		moduleIndex = QString::fromStdString(CommonInformationManager::GetDeviceName(deviceSN));
	}

	return moduleIndex;
}

///
/// @brief 获取同杯前反应
///
/// @param[in]  previousAssayCode  同杯前反应编号
///
/// @return 编号
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
QString chQcResultDetailsDlg::getLastAssay(const int previousAssayCode)
{
	QString lastAssay = "";
	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(previousAssayCode);
	if (spAssay)
	{
		lastAssay = QString::fromStdString(spAssay->assayName);
	}
    // 如果项目没有项目参数，先显示一个项目编号，和胡蓉确认
    else if (previousAssayCode >= 0)
    {
        lastAssay = QString::number(previousAssayCode);
    }

	return lastAssay;
}

///
/// @brief 更新生化界面
///
/// @return true:更新成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
///
bool chQcResultDetailsDlg::updateViewCH()
{
	// 更新质控结果
	updateQcResultCH();

	// 更新项目信息
	updateAssayInfoCH();

	// 更新试剂信息
	updateReagentInfoCH();

	// 更新校准信息
	updateCalibrationInfoCH();

	// 绘图
	if (!DisplayCurve(m_chAssayResult))
	{
		return false;
	}

	return true;
}

///
/// @brief 更新ISE界面
///
/// @return true:更新成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
///
bool chQcResultDetailsDlg::updateViewISE()
{
	// 更新质控结果
	updateQcResultISE();

	// 更新项目信息
	updateAssayInfoISE();

	// 更新耗材信息
	updateSuppliesInfoISE();

	// 更新校准信息
	updateCalibrationInfoISE();

	return true;
}

///
/// @brief 更新生化校准信息
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateCalibrationInfoCH()
{
	// 校准信息
	if (m_chAssayResult.__isset.resultCaliInfo)
	{
		auto resultCaliInfo = m_chAssayResult.resultCaliInfo;

		//校准品名称
		ui->cali_name_Ch->setText(QString::fromStdString(resultCaliInfo.calibratorName));

		//校准品批号
		ui->cali_lot_Ch->setText(QString::fromStdString(resultCaliInfo.calibratorLot));

		// 校准品失效日期
		if (!resultCaliInfo.calibratorExpiryDate.empty())
		{
			bool isExpired = true;
			QString calibratorExpiryDate = "";
			auto durationTime = TimeStringToPosixTime(resultCaliInfo.caliDate) - TimeStringToPosixTime(resultCaliInfo.calibratorExpiryDate);
			if (durationTime.hours() <= 0)
			{
				isExpired = false;
			}
			calibratorExpiryDate = QString::fromStdString(resultCaliInfo.calibratorExpiryDate);
			if (isExpired)
			{
				ui->cali_expired_day_Ch->setStyleSheet("color: red;");
				ui->label_cali_expired_day_Ch->setStyleSheet("color: red;");
			}
			ui->cali_expired_day_Ch->setText(ToCfgFmtDate(calibratorExpiryDate));
		}
		
		//校准时间
		if (!resultCaliInfo.caliDate.empty())
		{
			QString caliDate = QString::fromStdString(resultCaliInfo.caliDate);

			ui->cali_time_Ch->setText(ToCfgFmtDateTime(caliDate));
		}

		//当前曲线有效期
		QString caliCurveExpiryTime = "";
		auto GetCurveMessage = [](int timeout, int expireTime)->std::pair<bool, QString>
		{
			auto timeString = ":" + QString::number(expireTime);
			auto ret = std::make_pair(false, timeString);
			// 0 代表永不过期
			if (timeout == 0)
			{
				ret = std::make_pair(false, "");
			}
			// 当过期时间不是0，同时失效时间存在那么标红
			else if (expireTime <= 0)
			{
				ret = std::make_pair(true, timeString);
			}

			return ret;
		};

		// 负值的时候，显示为0
		auto expireTime = resultCaliInfo.caliCurveExpiryTime;
		if (expireTime < 0)
		{
			expireTime = 0;
		}

		// 是否警告色
		bool isRed = false;
		auto device = CommonInformationManager::GetInstance()->GetDeviceInfo(m_chAssayResult.deviceSN);
		if (device == nullptr)
		{
			return;
		}
		auto chemistryAssayInfo = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(m_chAssayResult.assayCode, device->deviceType);
		auto sourceType = resultCaliInfo.curveSource;
		switch (sourceType)
		{
		case tf::CurveSource::CURVE_SOURCE_NONE:
			caliCurveExpiryTime = "";
			break;
		case tf::CurveSource::CURVE_SOURCE_BOTTLE:
		{
			caliCurveExpiryTime = "B";
			if (chemistryAssayInfo == Q_NULLPTR)
			{
				break;
			}

			auto pairMessage = GetCurveMessage(chemistryAssayInfo->caliSuggest.timeoutBottle.timeout, expireTime);
			isRed = pairMessage.first;
			caliCurveExpiryTime += pairMessage.second;
		}
		break;
		case tf::CurveSource::CURVE_SOURCE_LOT:
		{
			caliCurveExpiryTime = "L";
			if (chemistryAssayInfo == Q_NULLPTR)
			{
				break;
			}

			auto pairMessage = GetCurveMessage(chemistryAssayInfo->caliSuggest.timeoutLot.timeout, expireTime);
			isRed = pairMessage.first;
			caliCurveExpiryTime += pairMessage.second;
		}
		break;
		case tf::CurveSource::CURVE_SOURCE_ASSAY:
			caliCurveExpiryTime = "T";
			break;
		default:
			break;
		}

		if (isRed)
		{
			ui->label_cali_curve_validity_Ch->setStyleSheet("color: red;");
			ui->cali_curve_validity_Ch->setStyleSheet("color: red;");
		}
		ui->cali_curve_validity_Ch->setText(caliCurveExpiryTime);
	}
}

///
/// @brief 更新生化质控结果
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateQcResultCH()
{
	// 结果数据
	if (m_chAssayResult.__isset.conc)
	{
		// 单位换算
		double result_temp = CommonInformationManager::GetInstance()->ToCurrentUnitValue(m_chAssayResult.assayCode, m_chAssayResult.conc);

		// 小数位数处理(默认2位)
		int left = 2;
		auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_chAssayResult.assayCode);
		if (spAssayInfo != Q_NULLPTR)
		{
			left = spAssayInfo->decimalPlace;
		}
		QString result = QString::number(result_temp, 'f', left);

		//// 警告提示(质控结果不进行判断)
		//QString dangerFlag = "";
		//ui->label_result_back_Ch->setProperty("bks", "result_normal");
		//ui->label_result_Ch->setProperty("bks", "result_normal");
		//ResultPrompt &resultTips = CommonInformationManager::GetInstance()->GetResultTips();
		//if (resultTips.enabledCritical)
		//{
		//	if (m_chAssayResult.__isset.crisisRangeJudge && m_chAssayResult.crisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		//	{
		//		ui->label_result_back_Ch->setProperty("bks", "result_highest");
		//		ui->label_result_Ch->setProperty("bks", "result_highest");
		//		dangerFlag = "↑!";
		//	}
		//	else if (m_chAssayResult.__isset.crisisRangeJudge && m_chAssayResult.crisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		//	{
		//		ui->label_result_back_Ch->setProperty("bks", "result_lowest");
		//		ui->label_result_Ch->setProperty("bks", "result_lowest");
		//		dangerFlag = "↓!";
		//	}
		//}

		//// 范围提示
		//QString refFlag = "";
		//QStringList refFlags;
		//if (resultTips.enabledOutRange)
		//{
		//	if (resultTips.outRangeMarkType == 1)
		//	{
		//		refFlags << "↑" << "↓";
		//	}
		//	else if (resultTips.outRangeMarkType == 2)
		//	{
		//		refFlags << "H" << "L";
		//	}
		//}

		//ui->label_result_Ch->setStyleSheet("color: black;");
		//if (m_chAssayResult.__isset.refRangeJudge && (refFlags.size() == 2))
		//{
		//	if (m_chAssayResult.refRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		//	{
		//		ui->label_result_Ch->setStyleSheet("color: red;");
		//		refFlag = refFlags[0];
		//	}
		//	else if (m_chAssayResult.refRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		//	{
		//		ui->label_result_Ch->setStyleSheet("color: blue;");
		//		refFlag = refFlags[1];
		//	}
		//}

		// 设置结果
		//ui->label_result_Ch->setText(result+ refFlag+ dangerFlag);
		ui->label_result_Ch->setText(result);
		
	}
	
	// 设置单位
	QString unit = CommonInformationManager::GetInstance()->GetCurrentUnit(m_chAssayResult.assayCode);
	ui->label_unit_Ch->setText(unit);

	// 状态码
	auto shileCodemap = DictionaryQueryManager::GetInstance()->GetShileDataAlarm();
	QString statusCodeDetail;
	auto statusCodes = QString::fromStdString(m_chAssayResult.resultStatusCodes).split(";");
	for (const auto statusCode : statusCodes)
	{
		if (statusCode.isEmpty())
		{
			continue;
		}

		// 若被禁止显示
		if (shileCodemap.count(statusCode) > 0)
		{
			continue;
		}

		// 不用结果的内容来显示计算排除点和失控
		if (statusCode == DATA_ALARM_CTRL_0 || 
			statusCode == DATA_ALARM_QC_EXCLUDE)
		{
			continue;
		}

		auto detail = CommonInformationManager::GetInstance()->GetResultStatusDetail(statusCode);
		statusCodeDetail += QString("%1：%2\n").arg(statusCode, detail);
	}

	// 计算排除点
	if (shileCodemap.count(DATA_ALARM_QC_EXCLUDE) == 0 && 
		!m_bCalculated)
	{
		statusCodeDetail += QString("%1：%2\n").arg(DATA_ALARM_QC_EXCLUDE, 
			CommonInformationManager::GetInstance()->GetResultStatusDetail(DATA_ALARM_QC_EXCLUDE));
	}

	// 失控
	if (shileCodemap.count(DATA_ALARM_CTRL_0) == 0 && 
		m_enQcState == QC_STATE_OUT_OF_CTRL)
	{
		statusCodeDetail += QString("%1：%2\n").arg(DATA_ALARM_CTRL_0,
			CommonInformationManager::GetInstance()->GetResultStatusDetail(DATA_ALARM_CTRL_0));
	}

	
	ui->result_status_Ch->setText(statusCodeDetail);
}

///
/// @brief 更新生化项目信息
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateAssayInfoCH()
{
	// 模块
	ui->module_index_Ch->setText(getModuleIndex(m_chAssayResult.deviceSN));

	//// 手工稀释倍数
	//if (m_chAssayResult.__isset.preDilutionFactor && m_chAssayResult.preDilutionFactor > 1)
	//{
	//	ui->dulition_Ch->setText(QString::number(m_chAssayResult.preDilutionFactor));
	//}
	//// 稀释倍数（机内）
	//else
	//{
	//	// 机内稀释
	//	if (m_chAssayResult.__isset.dilutionFactor && m_chAssayResult.dilutionFactor > 1)
	//	{
	//		ui->device_dulition_Ch->setText(QString::number(m_chAssayResult.dilutionFactor));
	//	}
	//	// 未稀释？
	//	else if (m_chAssayResult.__isset.suckVolType)
	//	{
	//		ui->device_dulition_Ch->setText(ConvertTfEnumToQString(m_chAssayResult.suckVolType));
	//	}
	//}

	// 反应杯号
	ui->cup_number_Ch->setText(QString::number(m_chAssayResult.cupSN));

	// 同杯前反应
	ui->last_assay_Ch->setText(getLastAssay(m_chAssayResult.previousAssayCode));

	// 项目完成时间
	ui->project_finished_time_Ch->setText(ToCfgFmtDateTime(QString::fromStdString(m_chAssayResult.endTime)));
}
	
///
/// @brief 更新生化试剂信息
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateReagentInfoCH()
{
	// 试剂信息
	if (m_chAssayResult.__isset.resultSupplyInfo)
	{
		auto reagentInfo = m_chAssayResult.resultSupplyInfo;
		// 批号
		QString reagentLot = "";
		reagentLot = QString::fromStdString(reagentInfo.reagentLot);
		ui->reagent_lot_Ch->setText(reagentLot);

		// 瓶号
		QString reagentSn = "";
		reagentSn = QString::fromStdString(reagentInfo.reagentSn);
		ui->reagent_sn_Ch->setText(reagentSn);

		// 失效日期
		QString reagentexpiryDate = "";
		bool isExpired = true;
		if (!IsExpired(reagentInfo.reagentexpiryDate, m_chAssayResult.endTime))
		{
			isExpired = false;
		}
		reagentexpiryDate = QString::fromStdString(reagentInfo.reagentexpiryDate);
		
		// 显示红色，超时
		if (isExpired)
		{
			ui->reagent_expired_day_Ch->setStyleSheet("color: red;");
			ui->label_reagent_expired_day_Ch->setStyleSheet("color: red;");
		}

		ui->reagent_expired_day_Ch->setText(ToCfgFmtDate(reagentexpiryDate));

		// 开瓶有效期
		QString reagentopenBottleExpiryTime = "";
		if (reagentInfo.__isset.reagentopenBottleExpiryTime)
		{
			int expiryTime = 0;
			if (reagentInfo.reagentopenBottleExpiryTime < 0)
			{
				expiryTime = 0;
			}
			else
			{
				expiryTime = reagentInfo.reagentopenBottleExpiryTime;
			}

			reagentopenBottleExpiryTime = QString::number(expiryTime);
			if (expiryTime <= 0)
			{
				ui->reagent_open_day_Ch->setStyleSheet("color: red;");
				ui->label_reagent_open_day_Ch->setStyleSheet("color: red;");
			}
		}
		ui->reagent_open_day_Ch->setText(reagentopenBottleExpiryTime);
	}
}

///
/// @brief 更新ISE质控结果
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateQcResultISE()
{
	// 结果数据
	if (m_iseAssayResult.__isset.conc)
	{
		// 单位换算
		double result_temp = CommonInformationManager::GetInstance()->ToCurrentUnitValue(m_iseAssayResult.assayCode, m_iseAssayResult.conc);

		// 小数位数处理(默认2位)
		int left = 2;
		auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_iseAssayResult.assayCode);
		if (spAssayInfo != Q_NULLPTR)
		{
			left = spAssayInfo->decimalPlace;
		}
		QString result = QString::number(result_temp, 'f', left);

		//// 警告提示(质控结果不进行判断)
		//QString dangerFlag = "";
		//ui->label_result_back_ISE->setProperty("bks", "result_normal");
		//ui->label_result_ISE->setProperty("bks", "result_normal");
		//ResultPrompt &resultTips = CommonInformationManager::GetInstance()->GetResultTips();
		//if (resultTips.enabledCritical)
		//{
		//	if (m_iseAssayResult.__isset.crisisRangeJudge && m_iseAssayResult.crisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		//	{
		//		ui->label_result_back_ISE->setProperty("bks", "result_highest");
		//		ui->label_result_ISE->setProperty("bks", "result_highest");
		//		dangerFlag = "↑!";
		//	}
		//	else if (m_iseAssayResult.__isset.crisisRangeJudge && m_iseAssayResult.crisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		//	{
		//		ui->label_result_back_ISE->setProperty("bks", "result_lowest");
		//		ui->label_result_ISE->setProperty("bks", "result_lowest");
		//		dangerFlag = "↓!";
		//	}
		//}

		//// 范围提示
		//QString refFlag = "";
		//QStringList refFlags;
		//if (resultTips.enabledOutRange)
		//{
		//	if (resultTips.outRangeMarkType == 1)
		//	{
		//		refFlags << "↑" << "↓";
		//	}
		//	else if (resultTips.outRangeMarkType == 2)
		//	{
		//		refFlags << "H" << "L";
		//	}
		//}

		//ui->label_result_ISE->setStyleSheet("color: black;");
		//if (m_iseAssayResult.__isset.refRangeJudge && (refFlags.size() == 2))
		//{
		//	if (m_iseAssayResult.refRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		//	{
		//		ui->label_result_ISE->setStyleSheet("color: red;");
		//		refFlag = refFlags[0];
		//	}
		//	else if (m_iseAssayResult.refRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		//	{
		//		ui->label_result_ISE->setStyleSheet("color: blue;");
		//		refFlag = refFlags[1];
		//	}
		//}

		// 设置结果
		//ui->label_result_ISE->setText(result + refFlag + dangerFlag);
		ui->label_result_ISE->setText(result);

	}

	// 设置单位
	QString unit = CommonInformationManager::GetInstance()->GetCurrentUnit(m_iseAssayResult.assayCode);
	ui->label_unit_ISE->setText(unit);

	// 状态码
	auto shileCodemap = DictionaryQueryManager::GetInstance()->GetShileDataAlarm();
	QString statusCodeDetail;
	auto statusCodes = QString::fromStdString(m_iseAssayResult.resultStatusCodes).split(";");
	for (const auto statusCode : statusCodes)
	{
		if (statusCode.isEmpty())
		{
			continue;
		}

		// 若被禁止显示
		if (shileCodemap.count(statusCode) > 0)
		{
			continue;
		}

		// 不用结果的内容来显示计算排除点和失控
		if (statusCode == DATA_ALARM_CTRL_0 ||
			statusCode == DATA_ALARM_QC_EXCLUDE)
		{
			continue;
		}

		auto detail = CommonInformationManager::GetInstance()->GetResultStatusDetail(statusCode);
		statusCodeDetail += QString("%1：%2\n").arg(statusCode, detail);
	}

	// 计算排除点
	if (shileCodemap.count(DATA_ALARM_QC_EXCLUDE) == 0 &&
		!m_bCalculated)
	{
		statusCodeDetail += QString("%1：%2\n").arg(DATA_ALARM_QC_EXCLUDE,
			CommonInformationManager::GetInstance()->GetResultStatusDetail(DATA_ALARM_QC_EXCLUDE));
	}

	// 失控
	if (shileCodemap.count(DATA_ALARM_CTRL_0) == 0 &&
		m_enQcState == QC_STATE_OUT_OF_CTRL)
	{
		statusCodeDetail += QString("%1：%2\n").arg(DATA_ALARM_CTRL_0,
			CommonInformationManager::GetInstance()->GetResultStatusDetail(DATA_ALARM_CTRL_0));
	}

	ui->result_status_ISE->setText(statusCodeDetail);

}

///
/// @brief 更新ISE耗材信息
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateSuppliesInfoISE()
{
	// 耗材信息
	if (m_iseAssayResult.__isset.resultSupplyInfo)
	{
		auto supplyInfo = m_iseAssayResult.resultSupplyInfo;

		// 内部标准液-批号和瓶号
		ui->standard_lot_ISE->setText(QString::fromStdString(supplyInfo.IS_Lot));
		ui->standard_sn_ISE->setText(QString::fromStdString(supplyInfo.IS_Sn));

		// 缓冲液-批号和瓶号
		ui->buffer_lot_ISE->setText(QString::fromStdString(supplyInfo.diluent_Lot));
		ui->buffer_sn_ISE->setText(QString::fromStdString(supplyInfo.diluent_Sn));
	}
}

///
/// @brief 更新ISE校准信息
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateCalibrationInfoISE()
{
	// 校准信息
	if (m_iseAssayResult.__isset.resultCaliInfo)
	{
		auto resultCaliInfo = m_iseAssayResult.resultCaliInfo;

		// 校准品名称
		ui->calibration_name_ISE->setText(QString::fromStdString(resultCaliInfo.calibratorName));

		// 校准品批号
		ui->calibration_lot_ISE->setText(QString::fromStdString(resultCaliInfo.calibratorLot));

		// 校准时间
		ui->calibration_time_ISE->setText(ToCfgFmtDateTime(QString::fromStdString(resultCaliInfo.caliDate)));

		// 校准品失效日期
		if (!resultCaliInfo.calibratorExpiryDate.empty())
		{
			bool isExpired = true;
			QString calibratorExpiryDate = "";
			auto durationTime = TimeStringToPosixTime(resultCaliInfo.caliDate) - TimeStringToPosixTime(resultCaliInfo.calibratorExpiryDate);
			if (durationTime.hours() <= 0)
			{
				isExpired = false;
			}
			calibratorExpiryDate = QString::fromStdString(resultCaliInfo.calibratorExpiryDate);
			if (isExpired)
			{
				ui->calibration_expireDay_ISE->setStyleSheet("color: red;");
				ui->label_calibration_expireDay_ISE->setStyleSheet("color: red;");
			}
			ui->calibration_expireDay_ISE->setText(ToCfgFmtDate(calibratorExpiryDate));
		}

		// 当前曲线有效期（ISE无标识）
		if (m_iseAssayResult.resultCaliInfo.__isset.caliCurveExpiryTime)
		{
			if (resultCaliInfo.caliCurveExpiryTime <= 0)
			{
				ui->label_curve_expirationDate_ISE->setStyleSheet("color: red;");
				ui->curve_expirationDate_ISE->setStyleSheet("color: red;");
				ui->curve_expirationDate_ISE->setText("0");
			}
			else
			{
				ui->curve_expirationDate_ISE->setText(QString::number(resultCaliInfo.caliCurveExpiryTime));
			}
		}
		else
		{
			ui->curve_expirationDate_ISE->setText("");
		}
	}
}

///
/// @brief 更新ISE项目信息
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::updateAssayInfoISE()
{
	// 模块
	QString moduleIndexName = QString::fromStdString(CommonInformationManager::GetDeviceName(m_iseAssayResult.deviceSN, m_iseAssayResult.moduleIndex));
	ui->module_index_ISE->setText(moduleIndexName);

	// 机内稀释
	if (m_iseAssayResult.__isset.dilutionFactor)
	{
		ui->device_dulition_ISE->setText(QString::number(m_iseAssayResult.dilutionFactor));
	}

	// 项目完成时间
	ui->finished_time_ISE->setText(ToCfgFmtDateTime(QString::fromStdString(m_iseAssayResult.endTime)));

}

///
/// @brief 清空生化界面
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::clearViewCH()
{
	// 质控结果清除
	ui->label_result_Ch->clear();
	ui->label_unit_Ch->clear();
	ui->result_status_Ch->clear();

	// 项目信息清除
	ui->module_index_Ch->clear();
	ui->cup_number_Ch->clear();
	ui->last_assay_Ch->clear();
	ui->project_finished_time_Ch->clear();

	// 试剂/耗材信息
	ui->reagent_lot_Ch->clear();
	ui->reagent_sn_Ch->clear();
	ui->reagent_expired_day_Ch->clear();
	ui->reagent_open_day_Ch->clear();

	// 校准信息
	ui->cali_name_Ch->clear();
	ui->cali_lot_Ch->clear();
	ui->cali_expired_day_Ch->clear();
	ui->cali_time_Ch->clear();
	ui->cali_curve_validity_Ch->clear();

}

///
/// @brief 清空ISE界面
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::clearViewISE()
{
	// 质控结果清除
	ui->label_result_ISE->clear();
	ui->label_unit_ISE->clear();
	ui->result_status_ISE->clear();

	// 项目信息清除
	ui->module_index_ISE->clear();
	ui->device_dulition_ISE->clear();
	ui->finished_time_ISE->clear();

	// 耗材信息
	ui->standard_lot_ISE->clear();
	ui->standard_sn_ISE->clear();
	ui->buffer_lot_ISE->clear();
	ui->buffer_sn_ISE->clear();

	// 校准信息
	ui->calibration_name_ISE->clear();
	ui->calibration_lot_ISE->clear();
	ui->calibration_expireDay_ISE->clear();
	ui->curve_expirationDate_ISE->clear();
	ui->calibration_time_ISE->clear();

}

///
/// @brief 是否过期
///
/// @param[in]  leftDate  左时间
/// @param[in]  rightDate 右时间
///
/// @return leftDate > rightDate = true
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年10月4日，新建函数
///
bool chQcResultDetailsDlg::IsExpired(const std::string& leftDate, const std::string& rightDate)
{
	if (leftDate.empty() || rightDate.empty())
	{
		return false;
	}

	auto durationTime = TimeStringToPosixTime(leftDate) - TimeStringToPosixTime(rightDate);

	if (durationTime.hours() >= 0)
	{
		return false;
	}

	return true;
}

///
/// @brief 显示曲线
///
/// @param[in]  assayResult  结果
///
/// @return true成功
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
bool chQcResultDetailsDlg::DisplayCurve(ch::tf::AssayTestResult& assayResult)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (assayResult.__isset.id && assayResult.id <= 0)
	{
		return false;
	}

	DisplayAnalyPointInfo(assayResult, m_pSplitGrid);

	int primaryWave = -1;
	primaryWave = assayResult.primarySubWaves[0];
	int secondWave = -1;
	if (assayResult.primarySubWaves.size() >= 2)
	{
		secondWave = assayResult.primarySubWaves[1];
	}

	m_reactionDatas.clear();
	CH_QC_DETAIL_PLOT::QReactionCurveData::DetectCurveData primaryCurve;
	CH_QC_DETAIL_PLOT::QReactionCurveData::DetectCurveData secondCurve;
	CH_QC_DETAIL_PLOT::QReactionCurveData::DetectCurveData primarySecondCurve;
	for (const auto& detectPoint : assayResult.detectPoints)
	{
		// 主波长
		if (detectPoint.ods.count(primaryWave) != 0)
		{
			// 测光点编号
			primaryCurve.detectPointers.push_back(detectPoint.pointSN);
			// 吸光度
			primaryCurve.absorbValues.push_back(detectPoint.ods.at(primaryWave));
		}

		// 次波长
		if (detectPoint.ods.count(secondWave) != 0)
		{
			// 测光点编号
			secondCurve.detectPointers.push_back(detectPoint.pointSN);
			// 吸光度
			secondCurve.absorbValues.push_back(detectPoint.ods.at(secondWave));
		}

		// 主次波长
		if (detectPoint.ods.count(primaryWave) != 0 && detectPoint.ods.count(secondWave) != 0)
		{
			// 测光点编号
			primarySecondCurve.detectPointers.push_back(detectPoint.pointSN);
			auto value = detectPoint.ods.at(primaryWave) - detectPoint.ods.at(secondWave);
			// 吸光度差值
			primarySecondCurve.absorbValues.push_back(value);
		}
	}

	// 显示主波长
	if (!primaryCurve.detectPointers.empty())
	{
		primaryCurve.color = QColor("#3b8ff6");
		primaryCurve.markerColor = QColor("#b7dbfd");
		primaryCurve.curveName = QString(tr("主波长:"));
		primaryCurve.qwtPlot = ui->qwtPlot_Ch;
		auto spCurve = std::make_shared<CH_QC_DETAIL_PLOT::QReactionCurveData>();
		spCurve->showCurve(primaryCurve);
		m_reactionDatas.push_back(spCurve);
	}

	// 次波长
	if (!secondCurve.detectPointers.empty())
	{
		secondCurve.color = QColor("#fa9016");
		secondCurve.markerColor = QColor("#b7dbfd");
		secondCurve.curveName = QString(tr("次波长:"));
		secondCurve.qwtPlot = ui->qwtPlot_Ch;
		auto spCurve = std::make_shared<CH_QC_DETAIL_PLOT::QReactionCurveData>();
		spCurve->showCurve(secondCurve);
		m_reactionDatas.push_back(spCurve);
	}

	// 主-次波长
	if (!primarySecondCurve.detectPointers.empty())
	{
		primarySecondCurve.color = QColor("#41c46a");
		primarySecondCurve.markerColor = QColor("#b7dbfd");
		primarySecondCurve.curveName = QString(tr("主-次："));
		primarySecondCurve.qwtPlot = ui->qwtPlot_Ch;
		auto spCurve = std::make_shared<CH_QC_DETAIL_PLOT::QReactionCurveData>();
		spCurve->showCurve(primarySecondCurve);
		m_reactionDatas.push_back(spCurve);
	}

	return true;
}

bool chQcResultDetailsDlg::DisplayAnalyPointInfo(const ch::tf::AssayTestResult& assayResult, 
	std::shared_ptr<CH_QC_DETAIL_PLOT::CustomGrid>& spCurve)
{
	if (spCurve == nullptr)
	{
		ULOG(LOG_INFO, "%s(bad parameter spcurve is null)", __FUNCTION__);
		return false;
	}

	// 获取测试的设备详细信息
	auto spDeviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(assayResult.deviceSN);
	if (spDeviceInfo == nullptr)
	{
		ULOG(LOG_INFO, "%s(failed to find device info, deviceSn :%s error)", __FUNCTION__, assayResult.deviceSN);
		return false;
	}

	// 获取生化的项目信息
	auto spAssay = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(assayResult.assayCode, spDeviceInfo->deviceType);
	if (spAssay == nullptr || !spAssay->__isset.analyPoints)
	{
		return false;
	}

	spCurve->clearSegment();

	// 需要单独绘点的模式
	std::set<int> caliPointType;
	caliPointType.insert(ch::tf::AnalysisMethod::ANALYSIS_METHOD_1POINT);
	caliPointType.insert(ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_END);
	caliPointType.insert(ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_RATE);

	// 连线
	if (caliPointType.count(spAssay->analysisMethod) <= 0)
	{
		if (spAssay->analyPoints.m > 0 && spAssay->analyPoints.n > 0)
		{
			spCurve->addSegment(spAssay->analyPoints.m, spAssay->analyPoints.n);
		}

		if (spAssay->analyPoints.p > 0 && spAssay->analyPoints.q > 0)
		{
			spCurve->addSegment(spAssay->analyPoints.p, spAssay->analyPoints.q);
		}
	}
	else
	{
		double delt = 0.5;
		for (const auto& data : { spAssay->analyPoints.m, spAssay->analyPoints.n,
			spAssay->analyPoints.p, spAssay->analyPoints.q })
		{
			if (data > 0)
			{
				spCurve->addSegment(data - delt, data + delt);
			}
		}
	}

	return true;
}


///
/// @brief 显示反应过程
///
/// @param[in]  detectPointer  显示过程的点
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::ShowProcessCurve(DetectCurve& detectPointer)
{
	std::shared_ptr<QwtPlotCurve> Curve(new QwtPlotCurve());
	m_waveCurves.push_back(Curve);
	Curve->attach(ui->qwtPlot_Ch);
	Curve->setZ(30);
	// 设置坐标轴
	Curve->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	Curve->setTitle(detectPointer.curveName);
	// 设置图例属性
	Curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, !detectPointer.curveName.isEmpty());
	// 设置曲线点符号
	QwtSymbol* pCircleSymbol2 = new QwtSymbol(QwtSymbol::Ellipse);
	pCircleSymbol2->setBrush(QBrush(detectPointer.color));
	pCircleSymbol2->setPen(QPen(Qt::NoPen));
	pCircleSymbol2->setSize(10);
	Curve->setSymbol(pCircleSymbol2);
	// 设置曲线样式为点
	Curve->setStyle(QwtPlotCurve::NoCurve);
	Curve->setSamples(detectPointer.detectPointers, detectPointer.absorbValues);
	ui->qwtPlot_Ch->setAxisScale(QwtPlot::xBottom, 0, 34, 1.0);
}

void chQcResultDetailsDlg::keyPressEvent(QKeyEvent* event)
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

void chQcResultDetailsDlg::keyReleaseEvent(QKeyEvent* event)
{
	if (event == Q_NULLPTR)
	{
		return;
	}

	if (event->key() == Qt::Key_Control)
	{
		m_controlModify = false;
	}

	if (event->key() == Qt::Key_Shift)
	{
		m_shiftKey = false;
	}
}

///
/// @brief 事件处理函数
///
/// @param[in]  obj   目标对象
/// @param[in]  event 事件对象
///
/// @return true表示事件已处理
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
bool chQcResultDetailsDlg::eventFilter(QObject *obj, QEvent *event)
{
	if (obj != ui->qwtPlot_Ch)
	{
		return QWidget::eventFilter(obj, event);
	}

	// 获取鼠标点击事件
	if (event->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent* mouseEvent = (QMouseEvent*)event;
		QwtScaleWidget *yAxisWdgt = ui->qwtPlot_Ch->axisWidget(QwtPlot::yLeft);
		int posX;
		int posY;
		if (yAxisWdgt != NULL)
		{
			posX = mouseEvent->x() - yAxisWdgt->width();
			posY = mouseEvent->y();
		}
		else
		{
			posX = mouseEvent->x();
			posY = mouseEvent->y();
		}

		if (posX > 0)
		{
			int iValX = ui->qwtPlot_Ch->invTransform(QwtPlot::xBottom, posX) + 0.5;
			double iValY = ui->qwtPlot_Ch->invTransform(QwtPlot::yLeft, posY);
			if ((iValX <= 0) || (iValX > 34))
			{
				return QWidget::eventFilter(obj, event);
			}

			// 调整背景色
			if (m_shiftKey)
			{
				if (m_startPostion && m_pSplitGrid != Q_NULLPTR)
				{
					m_pSplitGrid->addSegment(m_startPostion.value(), iValX);
				}
				m_startPostion = iValX;
			}

			QString message = "";
			m_plotMarker->setXValue(iValX);
			for (auto data : m_reactionDatas)
			{
				if (data == Q_NULLPTR)
				{
					continue;
				}

				message += data->GetCurrentWaveAbsorbs(iValX - 1);
				data->showMarker(iValX, iValY);
			}

			if (!message.isEmpty())
			{
				message.chop(1);
			}

			// 选择第一个
			if (m_reactionDatas.size() != 0)
			{
				m_reactionDatas[0]->showDataDetail(iValX, message);
			}


			McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
			pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
			pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
			pScaleDrawX->SetSelLabelColor(iValX, QColor(0xfa, 0x64, 0x31));
			ui->qwtPlot_Ch->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
		}
	}

	return QWidget::eventFilter(obj, event);
}

///
/// @brief 恢复原图
///
///
/// @par History:
/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
///
void chQcResultDetailsDlg::ResetCurve()
{
	for (auto& curveData : m_reactionDatas)
	{
		if (curveData != Q_NULLPTR)
		{
			curveData->reset();
		}
	}

	m_reactionDatas.clear();
	DisplayCurve(m_chAssayResult);
	
}

void CH_QC_DETAIL_PLOT::CustomGrid::draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
    QwtPlotGrid::draw(p, xMap, yMap, canvasRect);

    for (const auto& shleid : m_shiledMap)
    {
        double x1 = shleid.first; // 开始段的X坐标
        double x2 = shleid.second; // 结束段的X坐标
        p->fillRect(xMap.transform(x1), qRound(canvasRect.top()), xMap.transform(x2) - xMap.transform(x1), qRound(canvasRect.height()), QBrush(QColor("#b7dbfd")));
    }
}
