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

#include "QAssayResultView.h"
#include "ui_QAssayResultView.h"
#include <QStandardItem>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QDoubleValidator>

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
#include "QSampleAssayModel.h"
#include "datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include "shared/mccustomscaledraw.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/uidef.h"
#include "shared/ReagentCommon.h"
#include "WorkpageCommon.h"

#include "src/common/StringUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "src/common/common.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"

QReactionCurveData::QReactionCurveData()
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

void QReactionCurveData::showCurve(DetectCurveData& curveData)
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
	tmpMarker->setBrush(QBrush(QColor(0,0,255,128)));
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

void QReactionCurveData::reset()
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

double QReactionCurveData::GetAvge(QVector<double>& resultVec)
{
	double min = 0;
	double max = 0;
	for (auto result : resultVec)
	{
		if (min > result)
		{
			min = result;
		}
		else if(result > max)
		{
			max = result;
		}
	}

	return (max - min) / 10;
}

bool QReactionCurveData::showDataDetail(double x, QString showMessage)
{
	if (m_plotMessageMarker != Q_NULLPTR)
	{
		// modify bug0012425 by wuht
		m_plotMessageMarker->hide();
		m_plotMessageMarkerText->hide();
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
		if (x >= (m_curveData.absorbValues.size() -1) && (iValXRight > width))
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

void QReactionCurveData::showMarker(double x, double y)
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
		if ((x <= 0) || (x > 34) || x > m_curveData.absorbValues.size())
		{
			return;
		}

		m_plotInnerMarker->setYValue(m_curveData.absorbValues[x - 1]);
		m_plotInnerMarker->show();
	}
}

QString QReactionCurveData::GetCurrentWaveAbsorbs(double x)
{
	// modify bug0012425 by wuht
	if ((x < 0) || (x > 34) || x >= m_curveData.absorbValues.size())
	{
		return "";
	}
	else
	{
		QString ret = m_curveData.curveName;
		return ((ret += QString::number(m_curveData.absorbValues[x], 'f', 2)) + '\n');
	}
}

QAssayResultView::QAssayResultView(QWidget *parent)
	: QWidget(parent)
	, m_plotMagnifier(Q_NULLPTR)
	, m_plotPanner(Q_NULLPTR)
	, m_pSplitGrid(Q_NULLPTR)
    , m_mode(DataMode::CHMODE)
	, m_isReset(false)
	, m_allowedEdit(false)
	, m_controlModify(false)
	, m_shiftKey(false)
{
    ui = new Ui::QAssayResultView();
    ui->setupUi(this);
    Init();
}

QAssayResultView::~QAssayResultView()
{
}

///
/// @brief 设置项目结果id
///
/// @param[in]  reuslt		当前显示的结果Id
/// @param[in]  testItem    当前显示的项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月17日，新建函数
///
void QAssayResultView::SetAssayResult(int64_t reuslt, tf::TestItem& testItem)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	auto IsLHI = [&](int assayCode)->bool
	{
		if (assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_L
			|| assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_H
			|| assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_I)
		{
			return true;
		}

		return false;
	};

    ClearView();
	// 首先将结果作废，当没有结果的时候，去掉曲线的效果
	m_assayResult.__set_id(-1);
	m_currentTestItem = testItem;
	// LHI页面
	if (IsLHI(testItem.assayCode))
	{
		ui->stackedWidget->setCurrentWidget(ui->page_2);
		ui->stackedWidget_2->setCurrentWidget(ui->page_4);
	}
	// 生化页面
	else
	{
		ui->stackedWidget->setCurrentWidget(ui->page);
		ui->stackedWidget_2->setCurrentWidget(ui->page_4);
	}

    ch::tf::AssayTestResultQueryResp qryResp;
    ch::tf::AssayTestResultQueryCond qryCond;
    qryCond.__set_id(reuslt);
    if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(qryResp, qryCond)
        || qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstAssayTestResult.empty())
    {
        return;
    }

    auto assayResult = qryResp.lstAssayTestResult[0];
    // 保存当前结果信息
    m_assayResult = assayResult;

    // 获取项目信息
    auto spAssayConfig = CommonInformationManager::GetInstance()->GetAssayInfo(assayResult.assayCode);
    if (spAssayConfig == Q_NULLPTR)
    {
        return;
    }

    // 生化信息
    if (spAssayConfig->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
    {
        if (IsLHI(assayResult.assayCode))
        {
			ui->result_Edit_3->setEnabled(m_allowedEdit);
			ui->result_status_3->setEnabled(m_allowedEdit);
			ui->result_show_3->setEnabled(m_allowedEdit);
            DisplayLHIResultInfo(assayResult);
        }
        else
        {
			ui->result_Edit->setEnabled(m_allowedEdit);
			ui->result_show->setEnabled(m_allowedEdit);
			ui->result_status->setEnabled(m_allowedEdit);
            DisplayResultInfo(assayResult);
        }
    }

    DisplayCurve(assayResult);
}

///
/// @brief 设置当前显示的testItem
///
/// @param[in]  testItem  当前item
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月16日，新建函数
///
void QAssayResultView::SetCurrentTestItem(tf::TestItem& testItem)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_currentTestItem = testItem;
	// modify bug0010650 by wuht
	if (m_mode == DataMode::ISEMODE)
	{
		if (testItem.__isset.lastTestResultKey && m_assayResult.__isset.id)
		{
			if (testItem.lastTestResultKey.assayTestResultId == m_assayIseResult.id)
			{
				m_isReset = true;
			}
			else
			{
				m_isReset = false;
			}
		}
	}
	else
	{
		if (testItem.__isset.lastTestResultKey && m_assayResult.__isset.id)
		{
			if (testItem.lastTestResultKey.assayTestResultId == m_assayResult.id)
			{
				m_isReset = true;
			}
			else
			{
				m_isReset = false;
			}
		}
	}
	QDoubleValidator validator;
	validator.setNotation(QDoubleValidator::StandardNotation);
	ui->result_Edit->setValidator(&validator);
	ui->result_Edit_3->setValidator(&validator);
	ui->result_Edit_4->setValidator(&validator);
}

///
/// @brief 获取当前显示的testItem（数据浏览）
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月14日，新建函数
///
const tf::TestItem& QAssayResultView::GetCurrentTestItem()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	return m_currentTestItem;
}

///
/// @brief 获取当前结果的测试完成时间
///
///
/// @return 完成时间
///
/// @par History:
/// @li 5774/WuHongTao，2023年9月25日，新建函数
///
QString QAssayResultView::GetTestTime()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (m_assayResult.__isset.endTime)
	{
		return QString::fromStdString(m_assayResult.endTime);
	}
	else
	{
		return QString();
	}
}

///
/// @brief 将当前页面复查结果设置为复查结果
///
///
/// @return true:设置成功，false:设置失败
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月16日，新建函数
///
bool QAssayResultView::SetCurrentResult()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (m_mode == DataMode::ISEMODE)
	{
		auto deviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(m_assayIseResult.deviceSN);
		if (deviceInfo == nullptr)
		{
			return false;
		}

		tf::TestResultKeyInfo testKey;
		testKey.__set_deviceType(deviceInfo->deviceType);
		testKey.__set_assayTestResultId(m_assayIseResult.id);
		m_currentTestItem.__set_lastTestResultKey(testKey);
		m_currentTestItem.__set_retestConc(m_assayIseResult.conc);
		m_currentTestItem.__set_retestRefRangeJudge(m_assayIseResult.refRangeJudge);
		m_currentTestItem.__set_retestCrisisRangeJudge(m_assayIseResult.crisisRangeJudge);

		double concEdit;
		if (m_assayIseResult.__isset.concEdit
			&& !m_assayIseResult.concEdit.empty()
			&& stringutil::IsDouble(m_assayIseResult.concEdit, concEdit))
		{
			m_currentTestItem.__set_retestConc(concEdit);
			m_currentTestItem.__set_retestEditConc(true);
		}
		else
		{
			m_currentTestItem.__set_retestEditConc(false);
		}

		// modify bug0013260 by wuht
		m_currentTestItem.__set_retestResultStatusCodes(m_assayIseResult.resultStatusCodes);
		m_currentTestItem.__set_retestEndTime(m_assayIseResult.endTime);

		if (!DcsControlProxy::GetInstance()->ModifyTestItemInfo(m_currentTestItem))
		{
			return false;
		}

		m_isReset = true;
	}
	else
	{
		auto deviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(m_assayResult.deviceSN);
		if (deviceInfo == nullptr)
		{
			return false;
		}

		tf::TestResultKeyInfo testKey;
		testKey.__set_deviceType(deviceInfo->deviceType);
		testKey.__set_assayTestResultId(m_assayResult.id);
		m_currentTestItem.__set_lastTestResultKey(testKey);
		m_currentTestItem.__set_retestConc(m_assayResult.conc);
		m_currentTestItem.__set_retestRefRangeJudge(m_assayResult.refRangeJudge);
		m_currentTestItem.__set_retestCrisisRangeJudge(m_assayResult.crisisRangeJudge);

		double concEdit;
		if (m_assayResult.__isset.concEdit
			&& !m_assayResult.concEdit.empty()
			&& stringutil::IsDouble(m_assayResult.concEdit, concEdit))
		{
			m_currentTestItem.__set_retestConc(concEdit);
			m_currentTestItem.__set_retestEditConc(true);
		}
		else
		{
			m_currentTestItem.__set_retestEditConc(false);
		}

		// modify bug0013260 by wuht
		m_currentTestItem.__set_retestResultStatusCodes(m_assayResult.resultStatusCodes);
		m_currentTestItem.__set_retestEndTime(m_assayResult.endTime);

		if (!DcsControlProxy::GetInstance()->ModifyTestItemInfo(m_currentTestItem))
		{
			return false;
		}

		m_isReset = true;
	}

	emit dataChanged();
	return true;
}

///
/// @brief 当前日期是否过期
///
/// @param[in]  expiryDate  当前日期
///
/// @return true:过期
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月9日，新建函数
///
bool QAssayResultView::IsExpired(const std::string& leftDate, const std::string& rightDate)
{
	if (leftDate.empty() || rightDate.empty())
	{
		return false;
	}

	auto durationTime = TimeStringToPosixTime(leftDate) - TimeStringToPosixTime(rightDate);
	// modify bug0011325  by wuht
	if (durationTime.hours() >= 0)
	{
		return false;
	}

	return true;
}

///
/// @brief 设置样本信息
///
/// @param[in]  sampleInfo  样本信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月7日，新建函数
///
void QAssayResultView::SetSampleInfo(tf::SampleInfo& sampleInfo, bool isAllowedModify)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_currentSample = sampleInfo;

	// 是否允许修改检测结果
	// modify bug0011341 by wuht
	bool isAllowedEdit = UserInfoManager::GetInstance()->IsPermisson(PSM_IM_RESULT_DETAIL_MANUALEDIT);
    auto ResultEdits = findChildren<QLineEdit*>();
    for (auto& edit : ResultEdits)
    {
        // 正在测试的，校准，质控的样本不允许编辑,已经审核的样本不允许修改
        if (m_currentSample.__isset.status && m_currentSample.status == tf::SampleStatus::type::SAMPLE_STATUS_TESTING
            || m_currentSample.sampleType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR
            || m_currentSample.sampleType == tf::SampleType::SAMPLE_TYPE_QC
            || m_currentSample.__isset.audit && m_currentSample.audit
			// 若用户权限不允许修改检测结果
			|| !isAllowedEdit
			// 是否允许修改结果信息（历史页面不允许修改）
			|| !isAllowedModify)
        {
            // 不允许编辑
            edit->setEnabled(false);
        }
    }

	// 是否允许修改
	m_allowedEdit = (isAllowedEdit && isAllowedModify);

	// 当前用户的权限是否允许查看原始数据
	// modify bug0011339 by wuht
	bool IsShowRawData = UserInfoManager::GetInstance()->IsPermissionShow(PMS_RESULT_DETIAL_ORIGINVALUE);
	if (IsShowRawData)
	{
		ui->result_show->show();
		ui->result_show_3->show();
		ui->result_show_4->show();
		ui->label->show();
		ui->label_31->show();
		ui->label_38->show();
	}
	else
	{
		ui->result_show->hide();
		ui->result_show_3->hide();
		ui->result_show_4->hide();
		ui->label->hide();
		ui->label_31-> hide();
		ui->label_38->hide();
	}

	// 校准,质控样本不显示稀释倍数
	// 校准,质控样本不显示（样本量，手工稀释，机内稀释）
	if (m_currentSample.sampleType == tf::SampleType::SAMPLE_TYPE_CALIBRATOR
		|| m_currentSample.sampleType == tf::SampleType::SAMPLE_TYPE_QC)
	{
		ui->device_duli_times->hide();
		ui->manual_duli_times->hide();
		ui->module_index_9->hide();
		ui->label_42->hide();
		ui->label_2->hide();
		ui->label_6->hide();
		ui->label->hide();
		ui->result_show->hide();
		ui->label_38->hide();
		ui->result_show_4->hide();
		ui->label_32->hide();
		ui->manual_duli_times_4->hide();
	}

	// 仅有校准才显示校准原因和执行方法
	if (m_currentSample.sampleType != tf::SampleType::SAMPLE_TYPE_CALIBRATOR)
	{
		ui->t_abel_43->hide();
		ui->t_abel_44->hide();
		ui->t_label_45->hide();
		ui->t_label_46->hide();
		ui->label_84->hide();
		ui->cali_reason->hide();
	}
	else
	{
		ui->label_14->hide();
		ui->curve_date->hide();
		ui->label_83->hide();
		ui->cali_expireDate->hide();
	}
}

///
/// @brief 恢复原图
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月2日，新建函数
///
void QAssayResultView::ResetCurve()
{
    for (auto& curveData : m_reactionDatas)
    {
        if (curveData != Q_NULLPTR)
        {
			curveData->reset();
        }
    }

	m_reactionDatas.clear();
    DisplayCurve(m_assayResult);
}

///
/// @brief 显示ISE的结果
///
/// @param[in]  iseResult  结果信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月30日，新建函数
///
void QAssayResultView::SetIseAssayResult(ise::tf::AssayTestResult iseResult)
{
	ui->result_Edit_4->setEnabled(m_allowedEdit);
	ui->result_status_4->setEnabled(m_allowedEdit);
	ui->result_show_4->setEnabled(m_allowedEdit);
    DisplayISEResultInfo(iseResult);
}

void QAssayResultView::SetItemName(QLineEdit *label, const QString& strItemName)
{
	if (nullptr == label)
	{
		return;
	}

	int width = label->width();
	// 获取当前字体，用于计算在当前字体情况下需要多宽
	QFontMetrics fm(label->font());
	int dataWidth = fm.width(strItemName) + 33;
	// 若显示长度大于字符宽度，那么冒泡
	if (width > 0 && dataWidth >= width)
	{
		label->setToolTip(strItemName);
	}

	label->setText(strItemName);
	label->setStyleSheet("QToolTip { color: white; backgroud-color: #2a2a2a;}");
}

///
/// @brief 初始化
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月17日，新建函数
///
void QAssayResultView::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 初始化的时候，先设置为无效
	m_assayResult.__set_id(-1);
	ui->result_status_4->setReadOnly(true);
	ui->result_status_3->setReadOnly(true);
	ui->result_status->setReadOnly(true);
    // 定时更新吸光点
    connect(&m_timer, &QTimer::timeout, this, [&]() 
    {
        ch::tf::AssayTestResultQueryResp qryResp;
        ch::tf::AssayTestResultQueryCond qryCond;

        qryCond.__set_id(m_assayResult.id);
        if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(qryResp, qryCond)
            || qryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || qryResp.lstAssayTestResult.empty())
        {
			// modify bug 0011898 by wuht
			m_timer.stop();
            return;
        }

        auto assayResult = qryResp.lstAssayTestResult[0];
        // 保存当前结果信息
        m_assayResult = assayResult;
        // 若结果被设置，说明测试完成，定时器退出
        if ((m_assayResult.__isset.endTime && !m_assayResult.endTime.empty())
            || !this->isVisible())
        {
            m_timer.stop();
        }

        ResetCurve();
    });

	std::vector<QLineEdit*> edits = { ui->result_Edit, ui->result_Edit_3, ui->result_Edit_4};
	for (auto& edit : edits)
	{
		edit->setMaxLength(11);
		connect(edit, &QLineEdit::textEdited, this,
			[&](const QString& text)
		{
			emit this->resultChanged();
		});
	}



    // plot自动更新
    ui->qwtPlot->setAutoReplot(true);
    // 设置背景色
    ui->qwtPlot->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

    // 初始化网格分割线
	m_pSplitGrid = std::make_shared<CustomGrid>();
	m_pSplitGrid->setMinorPen(Qt::transparent);
	m_pSplitGrid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1, Qt::DashLine);
	m_pSplitGrid->attach(ui->qwtPlot);

    // 初始化标记选中线
    m_plotMarker = new QwtPlotMarker();
    m_plotMarker->setLineStyle(QwtPlotMarker::VLine);
    m_plotMarker->setLinePen(QColor(0xfa, 0x64, 0x31,128), 2);
    m_plotMarker->attach(ui->qwtPlot);
    m_plotMarker->setZ(20);

    // 安装事件过滤器
    ui->qwtPlot->installEventFilter(this);
	QFont Font;
	Font.setPixelSize(14);
	QwtText title;
	title.setFont(Font);
	title.setText(tr("吸光度") + "\n");
	ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, title);
	ui->qwtPlot->setStyleSheet("font-size: 14px;");

	ui->qwtPlot->setAxisLabelAlignment(QwtPlot::yLeft, Qt::AlignLeft);
	ui->qwtPlot->setContentsMargins(3, 3, 3, 3);

    // 横坐标只能为整型
    McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    ui->qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);

    // 纵坐标刻度值为彩色
    McCustomColorScaleDraw* pScaleDrawYLeft = new McCustomColorScaleDraw();
    pScaleDrawYLeft->SetLabelColor(QColor(0x1b, 0x7d, 0xf5));
    ui->qwtPlot->setAxisScaleDraw(QwtPlot::yLeft, pScaleDrawYLeft);

    // 坐标无刻度
    ui->qwtPlot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->qwtPlot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->qwtPlot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->qwtPlot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    // 设置边框
    ui->qwtPlot->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");
    // 插入图例
    //ui->qwtPlot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
    // 放大缩小
    m_plotMagnifier = new QwtPlotMagnifier(ui->qwtPlot->canvas());
    // 平移
    m_plotPanner = new QwtPlotPanner(ui->qwtPlot->canvas());
}

///
/// @brief 保存当前的结果修改
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月19日，新建函数
///
void QAssayResultView::SaveAssayResultEdit()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 获取当前显示单位对于主单位的倍率，修改结果时需要除以倍率
	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo;
	std::shared_ptr<::tf::CalcAssayInfo> spCalcInfo;
	// 获取当前单位与主单位的转化倍率
	double factor = 1.0;

	// 根据类型获取项目编号
	int assayCode = m_assayResult.assayCode;
	if (m_mode == DataMode::ISEMODE)
	{
		assayCode = m_assayIseResult.assayCode;
	}

	// 获取当前单位，按倍率转换并显示
	if (!CommonInformationManager::IsCalcAssay(assayCode))
	{
		spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
		if (spAssayInfo != Q_NULLPTR)
		{
			for (const auto& unit : spAssayInfo->units)
			{
				if (unit.isCurrent)
				{
					factor = unit.factor;
					break;
				}
			}
		}
	}

	if (spAssayInfo == Q_NULLPTR)
	{
		return;
	}

	// modify bug0010623 buy wuht
	QRegularExpression regex(UI_REG_POSITIVE_FLOAT);
    if (m_mode == DataMode::CHMODE)
    {
        // 保存修改结果
        ch::tf::AssayTestResult modifyAssayResult;
        modifyAssayResult.__set_id(m_assayResult.id);
		// modify bug3383
		if (ui->result_Edit->text().isEmpty())
		{
			return;
		}

		// 若数据不符合规则
		auto match = regex.match(ui->result_Edit->text());
		if (!match.hasMatch())
		{
			TipDlg(tr("保存"), tr("编辑浓度格式错误")).exec();
			return;
		}

		// modify bug0010623 buy wuht
		QString rawData = ui->result_Edit->text();
		auto edit = rawData.toDouble() / factor;
		QDoubleValidator validator;
		validator.setNotation(QDoubleValidator::StandardNotation);
		ui->result_Edit->setValidator(&validator);
        modifyAssayResult.__set_concEdit(std::to_string(edit));
        if (!ch::c1005::LogicControlProxy::ModifyAssayTestResult(modifyAssayResult))
        {
            return;
        }

		// modify bug0013752
		m_assayResult.__set_concEdit(std::to_string(edit));
		QString logOpt = QString(tr("编辑样本结果, 样本号: %1, 条码: %2, 申请时间: %3, 项目: %4, 原始结果: %5, 编辑结果: %6"))
			.arg(QString::fromStdString(m_currentSample.seqNo))
			.arg(QString::fromStdString(m_currentSample.barcode))
			.arg(QString::fromStdString(m_currentSample.registerTime))
			.arg(QString::fromStdString(spAssayInfo->assayName))
			.arg(ui->result_show->text())
			.arg(QString::number(edit));
		AddOptLog(::tf::OperationType::type::MOD, logOpt);
    }
    // 血清指数
    else if (m_mode == DataMode::LHIMODE)
    {
        ch::tf::AssayTestResult modifyAssayResult;
        modifyAssayResult.__set_id(m_assayResult.id);
		// modify bug3383
		if (ui->result_Edit_3->text().isEmpty())
		{
			return;
		}

		// 若数据不符合规则
		auto match = regex.match(ui->result_Edit_3->text());
		if (!match.hasMatch())
		{
			TipDlg(tr("保存"), tr("编辑浓度格式错误")).exec();
			return;
		}

		// modify bug0010623 buy wuht
		QString rawData = ui->result_Edit_3->text();
		auto edit = rawData.toDouble() / factor;
		QDoubleValidator validator;
		validator.setNotation(QDoubleValidator::StandardNotation);
		ui->result_Edit_3->setValidator(&validator);
		modifyAssayResult.__set_concEdit(std::to_string(edit));
        if (!ch::c1005::LogicControlProxy::ModifyAssayTestResult(modifyAssayResult))
        {
            return;
        }

		// modify bug0013752
		m_assayResult.__set_concEdit(std::to_string(edit));
		QString logOpt = QString(tr("编辑样本结果, 样本号: %1, 条码: %2, 申请时间: %3, 项目: %4, 原始结果: %5, 编辑结果: %6"))
			.arg(QString::fromStdString(m_currentSample.seqNo))
			.arg(QString::fromStdString(m_currentSample.barcode))
			.arg(QString::fromStdString(m_currentSample.registerTime))
			.arg(QString::fromStdString(spAssayInfo->assayName))
			.arg(ui->result_show_3->text())
			.arg(QString::number(edit));
		AddOptLog(::tf::OperationType::type::MOD, logOpt);
    }
    // ISE
    else if (m_mode == DataMode::ISEMODE)
    {
		spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_assayIseResult.assayCode);
		if (spAssayInfo == Q_NULLPTR)
		{
			return;
		}

        ise::tf::AssayTestResult modifyAssayResult;
        modifyAssayResult.__set_id(m_assayIseResult.id);
		// modify bug3383
		if (ui->result_Edit_4->text().isEmpty())
		{
			return;
		}

		// 若数据不符合规则
		auto match = regex.match(ui->result_Edit_4->text());
		if (!match.hasMatch())
		{
			TipDlg(tr("保存"), tr("编辑浓度格式错误")).exec();
			return;
		}

		// modify bug0010623 buy wuht
		QString rawData = ui->result_Edit_4->text();
		auto edit = rawData.toDouble() / factor;
		QDoubleValidator validator;
		validator.setNotation(QDoubleValidator::StandardNotation);
		ui->result_Edit_4->setValidator(&validator);
		modifyAssayResult.__set_concEdit(std::to_string(edit));
        if (!ise::ise1005::LogicControlProxy::ModifyAssayTestResult(modifyAssayResult))
        {
            return;
        }

		// modify bug0013752
		m_assayIseResult.__set_concEdit(std::to_string(edit));
		QString logOpt = QString(tr("编辑样本结果, 样本号: %1, 条码: %2, 申请时间: %3, 项目: %4, 原始结果: %5, 编辑结果: %6"))
			.arg(QString::fromStdString(m_currentSample.seqNo))
			.arg(QString::fromStdString(m_currentSample.barcode))
			.arg(QString::fromStdString(m_currentSample.registerTime))
			.arg(QString::fromStdString(spAssayInfo->assayName))
			.arg(ui->result_show_4->text())
			.arg(QString::number(edit));
		AddOptLog(::tf::OperationType::type::MOD, logOpt);
    }
    else
    {
        return;
    }

    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 数据被修改，通知上层刷新数据
    emit dataChanged();
}

///
/// @brief 显示反应过程
///
/// @param[in]  detectPointer  显示过程的点
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月17日，新建函数
///
void QAssayResultView::ShowProcessCurve(DetectCurve& detectPointer)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    std::shared_ptr<QwtPlotCurve> Curve(new QwtPlotCurve());
    m_waveCurves.push_back(Curve);
    Curve->attach(ui->qwtPlot);
    Curve->setZ(30);
    // 设置坐标轴
    Curve->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
    Curve->setTitle(detectPointer.curveName);
    // 设置图例属性
    // Curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, !detectPointer.curveName.isEmpty());
    // 设置曲线点符号
    QwtSymbol* pCircleSymbol2 = new QwtSymbol(QwtSymbol::Ellipse);
    pCircleSymbol2->setBrush(QBrush(detectPointer.color));
    pCircleSymbol2->setPen(QPen(Qt::NoPen));
    pCircleSymbol2->setSize(10);
    Curve->setSymbol(pCircleSymbol2);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
    ui->qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
    // 设置曲线样式为点
    Curve->setStyle(QwtPlotCurve::NoCurve);
    Curve->setSamples(detectPointer.detectPointers, detectPointer.absorbValues);
}

///
/// @brief 显示结果信息
///
/// @param[in]  result  结果信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月17日，新建函数
///
void QAssayResultView::DisplayResultInfo(ch::tf::AssayTestResult& result)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_mode = DataMode::CHMODE;
    // 生化页面
    ui->stackedWidget->setCurrentWidget(ui->page);
    ui->stackedWidget_2->setCurrentWidget(ui->page_4);

	// 清空
	{
		ui->device_duli_times->clear();
		ui->result_show->clear();
		ui->result_status->clear();
		ui->manual_duli_times->clear();
		ui->module_index->clear();
		ui->module_index_9->clear();
		ui->last_assay->clear();
		ui->cup_number->clear();
		ui->reagent_lot->clear();
		ui->reagent_sn->clear();
		ui->expired_day->clear();
		ui->open_day->clear();
		ui->cali_name->clear();
		ui->cali_lot->clear();
		ui->t_label_43->clear();
		ui->curve_date->clear();
		ui->t_label_44->clear();
	}


    // 小数点后保留几位小数(默认2位)
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

    QString conc = "";
	if (WorkpageCommon::IsConcValid(false, result.resultStatusCodes, result.conc))
    {
        conc = QString::number(result.conc*factor, 'f', left);
    }

	// 显示非浓度的情况下，不需要显示单位
	QString unit = " ";
	SampleShowSet sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	bool isShowConc = WorkpageCommon::IsShowConc(sampleSet, m_currentSample.sampleType);
	if (isShowConc)
	{
		QString strAssay = CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(result.assayCode);
		if (!QStringList({ "H", "L", "I" }).contains(strAssay))
		{
			unit += CommonInformationManager::GetInstance()->GetCurrentUnit(result.assayCode);
		}
	}
	ui->label_result_edit_unit->setText(unit);

	// 显示浓度的情况下才有边界标志
    QString editConcData = "";
	if (isShowConc)
	{
		// modify bug0011353 by wuht
		if (result.__isset.concEdit)
		{
			QString flag = "";
			auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
			if (userInfo != Q_NULLPTR && userInfo->type >= tf::UserType::type::USER_TYPE_ADMIN)
			{
				flag = QString::fromStdString(sampleSet.editflag);
			}

			editConcData = QString::fromStdString(result.concEdit);
			double editConc;
			if (stringutil::IsDouble(result.concEdit, editConc))
			{
				editConcData = flag + QString::number(editConc*factor, 'f', left);
			}
		}
	}

	// 结果提示
	QString dangerFlag = "";
	const ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();
	ui->label_result_edit_bk->setProperty("bks", "result_normal");
	ui->result_Edit->setStyleSheet("color: black;");
	if (resultTips.enabledCritical && isShowConc)
	{
		if (result.__isset.crisisRangeJudge && result.crisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		{
			ui->label_result_edit_bk->setProperty("bks", "result_highest");
			ui->result_Edit->setStyleSheet("color: #FA3741;");

			// modify bug0011817 by wuht
			if (resultTips.outRangeMarkType == 1)
			{
				dangerFlag = "↑!";
			}
			else if (resultTips.outRangeMarkType == 2)
			{
				dangerFlag = "H!";
			}
		}
		else if (result.__isset.crisisRangeJudge && result.crisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		{
			ui->label_result_edit_bk->setProperty("bks", "result_lowest");
			ui->result_Edit->setStyleSheet("color: #459FFF;");
			// modify bug0011817 by wuht
			if (resultTips.outRangeMarkType == 1)
			{
				dangerFlag = "↓!";
			}
			else if (resultTips.outRangeMarkType == 2)
			{
				dangerFlag = "L!";
			}
		}
	}

	QString refFlag;
	QStringList refFlags;
	if (resultTips.enabledOutRange && isShowConc)
	{
		if (resultTips.outRangeMarkType == 1)
		{
			refFlags << "↑" << "↓";
		}
		else if (resultTips.outRangeMarkType == 2)
		{
			refFlags << "H" << "L";
		}
	}

	if (result.__isset.refRangeJudge && (refFlags.size() == 2))
	{
		if (result.refRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		{
			ui->result_Edit->setStyleSheet("color: #FA3741;");
			refFlag = refFlags[0];
		}
		else if (result.refRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		{
			ui->result_Edit->setStyleSheet("color: #459FFF;");
			refFlag = refFlags[1];
		}
	}

	// 用户编辑的结果
	QString showResult = "";
	// 显示浓度
	if (isShowConc)
	{
		QString flag = dangerFlag.isEmpty()? refFlag: dangerFlag;
		if (result.__isset.concEdit && !result.concEdit.empty())
		{
			showResult = editConcData + flag;
		}
		else
		{
			showResult = conc + flag;
		}

		// 原始结果
		if (WorkpageCommon::IsConcValid(false, result.resultStatusCodes, result.conc))
		{
			ui->result_show->setText(conc);
		}
		// 若浓度无法显示，并且浓度未编辑，那么显示为空（编辑以后要显示编辑后的浓度）
		else if(!result.__isset.concEdit)
		{
			showResult = "";
		}
	}
	// 显示吸光度
	else
	{
		ui->result_Edit->setEnabled(false);
		auto rawResult = QString::number(result.abs);
		QString flag = dangerFlag.isEmpty() ? refFlag : dangerFlag;
		showResult = rawResult + flag;
		// 吸光度是否符合显示标准
		if (WorkpageCommon::IsRLUValid(result.abs))
		{
			ui->result_show->setText(rawResult);
		}
		else
		{
			showResult = "";
		}
	}

	// 若显示内容为空，则不能修改 modify bug0011240 by wuht
	if (showResult.isEmpty())
	{
		ui->result_Edit->setEnabled(false);
	}
	ui->result_Edit->setText(showResult);

	// 结果状态码
	auto shileCodemap = DictionaryQueryManager::GetInstance()->GetShileDataAlarm();
	QString statusCodeDetail;
	auto statusCodes =  QString::fromStdString(result.resultStatusCodes).split(";");
	// 是否允许Cali.M
	bool showCaliM = UserInfoManager::GetInstance()->IsPermissionShow(PMS_DATAREVIEW_RESULTCODE_CALIM);
	for (const auto statusCode : statusCodes)
	{
		// modify bug0010645 by wuht
		if (statusCode.isEmpty())
		{
			continue;
		}

		// 若被禁止显示
		if (shileCodemap.count(statusCode) > 0)
		{
			continue;
		}

		// 不允许Cali.M
		if (statusCode.contains("Cali.M") && !showCaliM)
		{
			continue;
		}

		auto detail = CommonInformationManager::GetInstance()->GetResultStatusDetail(statusCode);
		statusCodeDetail += QString("%1：%2\n").arg(statusCode, detail);
	}
    ui->result_status->setText(statusCodeDetail);

	// 手工稀释倍数
	if (result.__isset.preDilutionFactor && result.preDilutionFactor > 1)
	{
		ui->manual_duli_times->setText(QString::number(result.preDilutionFactor));
	}
    // 稀释倍数（机内）
    else
    {
		// 机内稀释
		if (result.__isset.dilutionFactor && result.dilutionFactor > 1)
		{
			ui->device_duli_times->setText(QString::number(result.dilutionFactor));
		}
		// 未稀释？
		else if(result.__isset.suckVolType)
		{
			if (result.suckVolType == tf::SuckVolType::SUCK_VOL_TYPE_DEC)
			{
				ui->module_index_9->setText(QString(tr("减量")));
			}
			else if (result.suckVolType == tf::SuckVolType::SUCK_VOL_TYPE_INC)
			{
				ui->module_index_9->setText(QString(tr("增量")));
			}
			else if(result.suckVolType == tf::SuckVolType::SUCK_VOL_TYPE_STD)
			{
				ui->module_index_9->setText(QString(tr("常量")));
			}
		}
	}

    // 模块
    auto device = CommonInformationManager::GetInstance()->GetDeviceInfo(result.deviceSN);
    if (device)
    {
        ui->module_index->setText(QString::fromStdString(CommonInformationManager::GetDeviceName(result.deviceSN)));
    }

    // 反应杯号
    ui->cup_number->setText(QString::number(result.cupSN));
    // 同杯前反应
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(result.previousAssayCode);
    if (spAssay)
    {
        ui->last_assay->setText(QString::fromStdString(spAssay->assayName));
    }

    // 项目完成时间
    ui->assay_finished_time->setText(ToCfgFmtDateTime(QString::fromStdString(result.endTime)));

	// 试剂信息
	if (result.__isset.resultSupplyInfo)
	{
		auto reagentInfo = result.resultSupplyInfo;
		// 批号
		QString reagentLot = "";
		reagentLot = QString::fromStdString(reagentInfo.reagentLot);
		ui->reagent_lot->setText(reagentLot);

		// 瓶号
		QString reagentSn = "";
		reagentSn = QString::fromStdString(reagentInfo.reagentSn);
		ui->reagent_sn->setText(reagentSn);

		// 失效日期
		QString reagentexpiryDate = "";
		bool isExpired = true;
		// modify bug0012372 by wuht
		if (!IsExpired(reagentInfo.reagentexpiryDate, result.endTime))
		{
			isExpired = false;
		}
		reagentexpiryDate = QString::fromStdString(reagentInfo.reagentexpiryDate);

		// 显示红色，超时
		if (isExpired)
		{
			ui->expired_day->setStyleSheet("color: red;");
			ui->label_11->setStyleSheet("color: red;");
		}

		ui->expired_day->setText(ToCfgFmtDate(reagentexpiryDate));

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

			// modify bug 0010655 by wuht
			if (expiryTime <= 0)
			{
				ui->label_10->setStyleSheet("color: red;");
				ui->open_day->setStyleSheet("color: red;");
			}
		}
		ui->open_day->setText(reagentopenBottleExpiryTime);
	}

	// 校准信息
	if (result.__isset.resultCaliInfo)
	{
		auto resultCaliInfo = result.resultCaliInfo;

		//校准名称
		QString calibratorName = "";
		calibratorName = QString::fromStdString(resultCaliInfo.calibratorName);
		ui->cali_name->setText(calibratorName);

		//校准批号
		QString calibratorLot = "";
		calibratorLot = QString::fromStdString(resultCaliInfo.calibratorLot);
		ui->cali_lot->setText(calibratorLot);

		bool isExpired = true;
		//失效日期
		QString calibratorExpiryDate = "";
		// modify bug0011868 by wuht
		auto durationTime = TimeStringToPosixTime(resultCaliInfo.caliDate) - TimeStringToPosixTime(resultCaliInfo.calibratorExpiryDate);
		if (durationTime.hours() <= 0)
		{
			isExpired = false;
		}

		// 当为空的时候，不能显示红色
		if (resultCaliInfo.calibratorExpiryDate.empty())
		{
			isExpired = false;
		}

		calibratorExpiryDate = QString::fromStdString(resultCaliInfo.calibratorExpiryDate);

		// 失效日期
		if (isExpired)
		{
			ui->t_abel_42->setStyleSheet("color: red;");
			ui->t_label_43->setStyleSheet("color: red;");
		}
		ui->t_label_43->setText(ToCfgFmtDate(calibratorExpiryDate));

		// 校准原因
		if (resultCaliInfo.__isset.caliReason)
		{
			ui->t_label_45->setText(GetCaliBrateReasonInfo(resultCaliInfo.caliReason));
		}
		// 执行方法

		if (resultCaliInfo.__isset.caliMode)
		{
			ui->t_label_46->setText(GetCaliBrateMethodInfo(resultCaliInfo.caliMode));
		}
		//校准时间
		QString caliDate = "";
		caliDate = QString::fromStdString(resultCaliInfo.caliDate);
		ui->t_label_44->setText(ToCfgFmtDateTime(caliDate));

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
		auto chemistryAssayInfo = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(result.assayCode, device->deviceType);
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
			case tf::CurveSource::CURVE_SOURCE_INTELLIGENT:
				caliCurveExpiryTime = "AC";
				break;
			default:
				break;
		}

		// modify bug 0010655 by wuht
		// modify bug3093 by wuht
		if (isRed)
		{
			ui->label_14->setStyleSheet("color: red;");
			ui->curve_date->setStyleSheet("color: red;");
		}
		ui->curve_date->setText(caliCurveExpiryTime);
	}

	ui->label_result_edit_bk->style()->unpolish(ui->label_result_edit_bk);
	ui->label_result_edit_bk->style()->polish(ui->label_result_edit_bk);
}

///
/// @brief显示结果信息(血清指数)
///
/// @param[in]  result  结果信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月30日，新建函数
///
void QAssayResultView::DisplayLHIResultInfo(ch::tf::AssayTestResult& result)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_mode = DataMode::LHIMODE;
    // LHI页面
    ui->stackedWidget->setCurrentWidget(ui->page_2);
    ui->stackedWidget_2->setCurrentWidget(ui->page_4);

	// 首先清楚信息
	ui->device_dulition->clear();
	ui->dulition->clear();
	ui->module_index_3->clear();
	ui->cup_number_3->clear();
	ui->last_assay_3->clear();
	ui->sample_register_times_3->clear();

	int iPrecision = 2; // 小数点后保留几位小数(默认2位)	
	double factor = 1.0; // 获取当前单位与主单位的转化倍率
	std::string unitName = "";
	WorkpageCommon::GetPrecFactorUnit(result.assayCode, iPrecision, factor, unitName);

    QString conc = "";
    if (result.__isset.conc)
    {
        conc = QString::number(result.conc, 'f', iPrecision);
    }

	// 显示非浓度的情况下，不需要显示单位
	QString unit = " ";
	const SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	bool isShowConc = WorkpageCommon::IsShowConc(sampleSet, m_currentSample.sampleType);
	if (isShowConc)
	{
		QString strAssay = CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(result.assayCode);
		if (!QStringList({ "H", "L", "I" }).contains(strAssay))
		{
			unit += CommonInformationManager::GetInstance()->GetCurrentUnit(result.assayCode);
		}
	}
	ui->label_unit_2->setText(unit);

	// 结果提示
	QString dangerFlag = "";
	const ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();
	ui->label_result_edit_bk_3->setProperty("bks", "result_normal");
	ui->result_Edit_3->setStyleSheet("color: black;");
	ui->result_Edit_3->setProperty("bks", "result_normal");
	if (resultTips.enabledCritical && isShowConc)
	{
		if (result.__isset.crisisRangeJudge && result.crisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		{
			ui->label_result_edit_bk_3->setProperty("bks", "result_highest");
			ui->result_Edit_3->setStyleSheet("color: #FA3741;");
			ui->result_Edit_3->setProperty("bks", "result_highest");
			// modify bug0011817 by wuht
			if (resultTips.outRangeMarkType == 1)
			{
				dangerFlag = "↑!";
			}
			else if (resultTips.outRangeMarkType == 2)
			{
				dangerFlag = "H!";
			}
		}
		else if (result.__isset.crisisRangeJudge && result.crisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		{
			ui->label_result_edit_bk_3->setProperty("bks", "result_lowest");
			ui->result_Edit_3->setStyleSheet("color: #459FFF;");
			ui->result_Edit_3->setProperty("bks", "result_lowest");
			// modify bug0011817 by wuht
			if (resultTips.outRangeMarkType == 1)
			{
				dangerFlag = "↓!";
			}
			else if (resultTips.outRangeMarkType == 2)
			{
				dangerFlag = "L!";
			}
		}
	}

	QString refFlag;
	QStringList refFlags;
	if (resultTips.enabledOutRange && isShowConc)
	{
		if (resultTips.outRangeMarkType == 1)
		{
			refFlags << "↑" << "↓";
		}
		else if (resultTips.outRangeMarkType == 2)
		{
			refFlags << "H" << "L";
		}
	}

	if (result.__isset.refRangeJudge && (refFlags.size() == 2))
	{
		if (result.refRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		{
			ui->result_Edit_3->setStyleSheet("color: #FA3741;");
			refFlag = refFlags[0];
		}
		else if (result.refRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		{
			ui->result_Edit_3->setStyleSheet("color: #459FFF;");
			refFlag = refFlags[1];
		}
	}

	QString flagCombin = dangerFlag.isEmpty() ? refFlag : dangerFlag;
	// 增加++++
	if (result.__isset.qualitativeJudge && tf::QualJudge::type::FuzzyRegion < result.qualitativeJudge)
	{
		int start = result.qualitativeJudge - tf::QualJudge::CH_SIND_LEVEL1;
		auto status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(result.assayCode, start);
		flagCombin = status + flagCombin;
	}

	if (isShowConc)
	{
		QString editConcData = "";
		if (result.__isset.concEdit)
		{
			QString flag = "";
			auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
			if (userInfo != Q_NULLPTR && userInfo->type >= tf::UserType::type::USER_TYPE_ADMIN)
			{
				flag = QString::fromStdString(sampleSet.editflag);
			}

			editConcData = QString::fromStdString(result.concEdit);
			double editConc;
			if (stringutil::IsDouble(result.concEdit, editConc))
			{
				editConcData = flag + QString::number(editConc) + flagCombin;
			}
		}

		// 是否显示浓度
		bool  isShowConc = WorkpageCommon::IsConcValid(false, result.resultStatusCodes, result.conc);
		if (isShowConc)
		{
			// 原始结果
			ui->result_show_3->setText(conc);
		}

		// 加工后的结果
		if (!conc.isEmpty())
		{
			conc = conc + flagCombin;
		}

		QString showText;
		if (result.__isset.concEdit && !result.concEdit.empty())
		{
			showText = editConcData;
		}
		// 若不是编辑后的浓度，需要判断初始浓度是否符合显示标准
		else if(isShowConc)
		{
			showText = conc;
		}

		// 若显示内容为空，则不能修改 modify bug0011240 by wuht
		if (showText.isEmpty())
		{
			ui->result_Edit_3->setEnabled(false);
		}
		// 显示浓度
		ui->result_Edit_3->setText(showText);

	}
	else
	{
		ui->result_Edit_3->setEnabled(false);
		auto rawData = QString::number(result.abs);
		QString data = rawData + flagCombin;

		if (WorkpageCommon::IsRLUValid(result.abs))
		{
			ui->result_Edit_3->setText(data);
			// 原始结果
			ui->result_show_3->setText(rawData);
		}
	}

	// 结果状态码
	auto shileCodemap = DictionaryQueryManager::GetInstance()->GetShileDataAlarm();
	// 结果状态码
	QString statusCodeDetail;
	auto statusCodes = QString::fromStdString(result.resultStatusCodes).split(";");
	for (const auto statusCode : statusCodes)
	{
		// modify bug0010645 by wuht
		if (statusCode.isEmpty())
		{
			continue;
		}

		// 若被禁止显示
		if (shileCodemap.count(statusCode) > 0)
		{
			continue;
		}

		auto detail = CommonInformationManager::GetInstance()->GetResultStatusDetail(statusCode);
		statusCodeDetail += QString("%1：%2\n").arg(statusCode, detail);
	}

	ui->result_status_3->setText(statusCodeDetail);
    
	// 手工稀释倍数
	if (result.__isset.preDilutionFactor && result.preDilutionFactor > 1)
	{
		ui->dulition->setText(QString::number(result.preDilutionFactor));
	}
	// 稀释倍数
	 else if (result.__isset.dilutionFactor)
	{
		ui->device_dulition->setText(QString::number(result.dilutionFactor));
	}


    // 模块
    auto device = CommonInformationManager::GetInstance()->GetDeviceInfo(result.deviceSN);
    if (device)
    {
        ui->module_index_3->setText(QString::fromStdString(CommonInformationManager::GetDeviceName(result.deviceSN)));
    }

    ui->cup_number_3->setText(QString::number(result.cupSN));

    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(result.previousAssayCode);
    if (spAssay)
    {
        ui->last_assay_3->setText(QString::fromStdString(spAssay->assayName));
    }

    // 项目完成时间
    ui->sample_register_times_3->setText(ToCfgFmtDateTime(QString::fromStdString(result.endTime)));

	ui->label_result_edit_bk_3->style()->unpolish(ui->label_result_edit_bk_3);
	ui->label_result_edit_bk_3->style()->polish(ui->label_result_edit_bk_3);
}

///
/// @brief 显示ISE的结果信息
///
/// @param[in]  result  结果信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月30日，新建函数
///
void QAssayResultView::DisplayISEResultInfo(ise::tf::AssayTestResult& result)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_mode = DataMode::ISEMODE;
    m_assayIseResult = result;
	if (result.id < 0)
	{
		ui->result_status_4->setEnabled(false);
		ui->result_show_4->setEnabled(false);
	}

	ui->result_status_4->clear();
	ui->result_show_4->clear();

    // 校准信息清空
    {
        ui->cali_lot_7->clear();
        ui->cali_name_7->clear();
        ui->curve_date_7->clear();
        ui->cali_time->clear();
        ui->cali_status->clear();
		ui->cali_expireDate->clear();
		ui->curve_date_7->setStyleSheet("color: #565656;");
		ui->cali_expireDate->setStyleSheet("color: #565656;");
		ui->label_80->setStyleSheet("color: #565656;");
		ui->label_83->setStyleSheet("color: #565656;");
    }

    // 试剂信息清空
    {
        ui->supply_expired_day->clear();
        ui->supply_expired_day_2->clear();
        ui->supply_lot->clear();
        ui->supply_lot_2->clear();
        ui->supply_open_day->clear();
        ui->supply_open_day_2->clear();
        ui->supply_sn->clear();
        ui->supply_sn_2->clear();
        ui->supply_status->clear();
        ui->supply_status_2->clear();
    }

    // ISE页面
    ui->stackedWidget->setCurrentWidget(ui->page_3);
    ui->stackedWidget_2->setCurrentWidget(ui->page_5);

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

	// 显示非浓度的情况下，不需要显示单位
	QString unit = " ";
	SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	bool isShowConc = WorkpageCommon::IsShowConc(sampleSet, m_currentSample.sampleType);
	if (isShowConc)
	{
		QString strAssay = CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(result.assayCode);
		if (!QStringList({ "H", "L", "I" }).contains(strAssay))
		{
			unit += CommonInformationManager::GetInstance()->GetCurrentUnit(result.assayCode);
		}
	}
	ui->label_unit_3->setText(unit);

    QString conc = "";
    if (result.__isset.conc)
    {
        conc = QString::number(result.conc*factor, 'f', left);
    }

	// 结果提示
	QString dangerFlag = "";
	ResultPrompt &resultTips = DictionaryQueryManager::GetInstance()->GetResultTips();
	ui->label_result_edit_bk_4->setProperty("bks", "result_normal");
	ui->result_Edit_4->setProperty("bks", "result_normal");
	ui->result_Edit_4->setStyleSheet("color: black;");
	if (resultTips.enabledCritical && isShowConc)
	{
		if (result.__isset.crisisRangeJudge && result.crisisRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		{
			ui->label_result_edit_bk_4->setProperty("bks", "result_highest");
			ui->result_Edit_4->setProperty("bks", "result_highest");
			ui->result_Edit_4->setStyleSheet("color: #FA3741;");
			// modify bug0011817 by wuht
			if (resultTips.outRangeMarkType == 1)
			{
				dangerFlag = "↑!";
			}
			else if (resultTips.outRangeMarkType == 2)
			{
				dangerFlag = "H!";
			}
		}
		else if (result.__isset.crisisRangeJudge && result.crisisRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		{
			ui->label_result_edit_bk_4->setProperty("bks", "result_lowest");
			ui->result_Edit_4->setProperty("bks", "result_lowest");
			ui->result_Edit_4->setStyleSheet("color: #459FFF;");
			// modify bug0011817 by wuht
			if (resultTips.outRangeMarkType == 1)
			{
				dangerFlag = "↓!";
			}
			else if (resultTips.outRangeMarkType == 2)
			{
				dangerFlag = "L!";
			}
		}
	}

	QString refFlag;
	QStringList refFlags;
	if (resultTips.enabledOutRange && isShowConc)
	{
		if (resultTips.outRangeMarkType == 1)
		{
			refFlags << "↑" << "↓";
		}
		else if (resultTips.outRangeMarkType == 2)
		{
			refFlags << "H" << "L";
		}
	}

	if (result.__isset.refRangeJudge && (refFlags.size() == 2))
	{
		if (result.refRangeJudge == ::tf::RangeJudge::ABOVE_UPPER)
		{
			ui->result_Edit_4->setStyleSheet("color: #FA3741;");
			refFlag = refFlags[0];
		}
		else if (result.refRangeJudge == ::tf::RangeJudge::LESS_LOWER)
		{
			ui->result_Edit_4->setStyleSheet("color: #459FFF;");
			refFlag = refFlags[1];
		}
	}

	QString flagCombin = dangerFlag.isEmpty() ? refFlag : dangerFlag;
	// 显示浓度
	if (isShowConc)
	{
		QString editConcData = "";
		if (result.__isset.concEdit)
		{
			SampleShowSet &sampleSet = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
			QString flag = "";
			auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
			if (userInfo != Q_NULLPTR && userInfo->type >= tf::UserType::type::USER_TYPE_ADMIN)
			{
				flag = QString::fromStdString(sampleSet.editflag);
			}

			editConcData = QString::fromStdString(result.concEdit);
			double editConc;
			if (stringutil::IsDouble(result.concEdit, editConc))
			{
				editConcData = flag + QString::number(editConc*factor, 'f', left);
			}
		}

		if (!editConcData.isEmpty())
		{
			editConcData = editConcData + flagCombin;
		}

		QString rawData;
		if (!conc.isEmpty())
		{
			rawData = conc;
			conc = conc + flagCombin;
		}

		// 用户编辑的结果
		QString showText;
		if (result.__isset.concEdit && !result.concEdit.empty())
		{
			showText = editConcData;
		}
		else if(WorkpageCommon::IsConcValid(false, result.resultStatusCodes, result.conc))
		{
			showText = conc;
		}

		// 若显示内容为空，则不能修改 modify bug0011240 by wuht
		if (showText.isEmpty())
		{
			ui->result_Edit_4->setEnabled(false);
		}

		ui->result_Edit_4->setText(showText);
		// 原始结果
		ui->result_show_4->setText(rawData);
	}
	// 显示电动势
	else
	{
		ui->result_Edit_4->setEnabled(false);
		QString emfValue;
		QString rawData;
		if (result.__isset.testEMF)
		{
			rawData = QString::number(result.testEMF.sampleEMF, 'f', left) + "/" + QString::number(result.testEMF.baseEMF, 'f', left);
			emfValue = rawData + flagCombin;
		}

		// 用户编辑的结果
		SetItemName(ui->result_Edit_4, emfValue);
		// 原始结果
		ui->result_show_4->setText(rawData);
	}

	// 结果状态码
	auto shileCodemap = DictionaryQueryManager::GetInstance()->GetShileDataAlarm();
	// 结果状态码
	QString statusCodeDetail;
	auto statusCodes = QString::fromStdString(result.resultStatusCodes).split(";");
	for (const auto statusCode : statusCodes)
	{
		// modify bug0010645 by wuht
		if (statusCode.isEmpty())
		{
			continue;
		}

		// 若被禁止显示
		if (shileCodemap.count(statusCode) > 0)
		{
			continue;
		}

		auto detail = CommonInformationManager::GetInstance()->GetResultStatusDetail(statusCode);
		statusCodeDetail += QString("%1: %2\n").arg(statusCode, detail);
	}
	ui->result_status_4->setText(statusCodeDetail);
    
	// 手工稀释
	if (result.__isset.preDilutionFactor && result.preDilutionFactor > 1)
	{
		// 稀释倍数
		ui->manual_duli_times_4->setText(QString::number(result.preDilutionFactor));
	}
	// 机内稀释（修改bug0013742,改为空）
	else if (result.__isset.dilutionFactor)
	{
		ui->device_dulition_times->setText("");
	}

    // 模块
    auto device = CommonInformationManager::GetInstance()->GetDeviceInfo(result.deviceSN);
    if (device)
    {
		auto devName = CommonInformationManager::GetDeviceName(result.deviceSN, result.moduleIndex);
        ui->module_index_2->setText(QString::fromStdString(devName));
    }

    // 项目完成时间
    ui->assay_finished_time_4->setText(ToCfgFmtDateTime(QString::fromStdString(result.endTime)));

    // 耗材信息
    // 内部标准液
    ui->supply_lot->setText(QString::fromStdString(result.resultSupplyInfo.IS_Lot));
    ui->supply_sn->setText(QString::fromStdString(result.resultSupplyInfo.IS_Sn));
    // 缓冲液
    ui->supply_lot_2->setText(QString::fromStdString(result.resultSupplyInfo.diluent_Lot));
    ui->supply_sn_2->setText(QString::fromStdString(result.resultSupplyInfo.diluent_Sn));

    // 校准信息
    ui->cali_lot_7->setText(QString::fromStdString(result.resultCaliInfo.calibratorLot));
    // 校准品名称
    ui->cali_name_7->setText(QString::fromStdString(result.resultCaliInfo.calibratorName));
    
	// 失效日期
	// modify bug 0010655 by wuht
	QString reagentexpiryDate = "";
	bool isExpired = true;
	//if (result.resultCaliInfo.__isset.calibratorExpiryDate)
	{
		// modify bug0011868 by wuht
		//if (result.resultCaliInfo.caliDate)
		{
			auto durationTime = TimeStringToPosixTime(result.resultCaliInfo.caliDate) - TimeStringToPosixTime(result.resultCaliInfo.calibratorExpiryDate);
			if (durationTime.hours() <= 0)
			{
				isExpired = false;
			}
		}

		if (result.resultCaliInfo.calibratorExpiryDate.empty())
		{
			isExpired = false;
		}

		reagentexpiryDate = ToCfgFmtDate(QString::fromStdString(result.resultCaliInfo.calibratorExpiryDate));
		if (isExpired)
		{
			ui->curve_date_7->setStyleSheet("color: red;");
			ui->label_80->setStyleSheet("color: red;");
		}
	}
    ui->curve_date_7->setText(reagentexpiryDate);

    // 校准曲线有效期
	if (result.__isset.resultCaliInfo && result.resultCaliInfo.__isset.caliCurveExpiryTime)
	{
		// bug0014295 modify by wuht
		int32_t caliCurveExpiryTime = result.resultCaliInfo.caliCurveExpiryTime;
		if (caliCurveExpiryTime < 0)
		{
			caliCurveExpiryTime = 0;
		}

		// modify bug0011895 by wuht
		QString expireDateString = QString::number(caliCurveExpiryTime);
		// modify bug 0010655 by wuht
		if (caliCurveExpiryTime <= 0)
		{
			ui->label_83->setStyleSheet("color: red;");
			ui->cali_expireDate->setStyleSheet("color: red;");
		}

		ui->cali_expireDate->setText(expireDateString);
	}
	else
	{
		ui->cali_expireDate->setText("");
	}

	// 校准原因
	ui->cali_reason->setText(GetCaliBrateReasonInfo(result.resultCaliInfo.caliReason));
    // 校准时间
    ui->cali_time->setText(ToCfgFmtDateTime(QString::fromStdString(result.resultCaliInfo.caliDate)));
	ui->label_result_edit_bk_4->style()->unpolish(ui->label_result_edit_bk_4);
	ui->label_result_edit_bk_4->style()->polish(ui->label_result_edit_bk_4);
}

void QAssayResultView::keyPressEvent(QKeyEvent* event)
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

void QAssayResultView::keyReleaseEvent(QKeyEvent* event)
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
/// @li 5774/WuHongTao，2022年11月17日，新建函数
///
bool QAssayResultView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != ui->qwtPlot)
    {
        return QWidget::eventFilter(obj, event);
    }

    // 获取鼠标点击事件
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;
        QwtScaleWidget *yAxisWdgt = ui->qwtPlot->axisWidget(QwtPlot::yLeft);
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
            int iValX = ui->qwtPlot->invTransform(QwtPlot::xBottom, posX) + 0.5;
			double iValY = ui->qwtPlot->invTransform(QwtPlot::yLeft, posY);
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

			// 显示text的浮窗
			ShowTextDetail(iValX, iValY);
            McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
            pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
            pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
            pScaleDrawX->SetSelLabelColor(iValX, QColor(0xfa, 0x64, 0x31));
            ui->qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
        }
    }

    return QWidget::eventFilter(obj, event);
}

void QAssayResultView::ShowTextDetail(double xPointer, double yPointer)
{
	if (m_plotMarker == Q_NULLPTR)
	{
		return;
	}

	QString message = "";
	m_plotMarker->setXValue(xPointer);
	for (const auto& data : m_reactionDatas)
	{
		if (data == Q_NULLPTR)
		{
			continue;
		}

		// modify bug0012425 by wuht
		auto tmpMessage = data->GetCurrentWaveAbsorbs(xPointer - 1);
		if (tmpMessage.isEmpty())
		{
			continue;
		}

		message += tmpMessage;
		data->showMarker(xPointer, yPointer);
	}

	if (!message.isEmpty())
	{
		message.chop(1);
	}

	// 选择第一个
	if (m_reactionDatas.size() != 0)
	{
		m_reactionDatas[0]->showDataDetail(xPointer, message);
	}
}

///
/// @brief 页面显示时，处理事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 5774/WuHongTao，2023年4月24日，新建函数
///
void QAssayResultView::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 定时器未启动，同时endtime未被设置
	// modify bug 0011898 by wuht
    if (!m_timer.isActive() 
		&& m_assayResult.endTime.empty()
		&& m_assayResult.id != -1)
    {
        m_timer.setInterval(1000);
        m_timer.start();
    }
}

///
/// @brief 清除界面
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月19日，新建函数
///
void QAssayResultView::ClearView()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置结果不能输入字符串的正则表达式
    auto labels = findChildren<QLineEdit*>(QRegExp("result_Edit*"));
    for (auto& edit : labels)
    {
        edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_FLOAT), this));
    }

	ui->result_Edit->setEnabled(false);
	ui->result_show->setEnabled(false);
	ui->result_status->setEnabled(false);
	ui->result_Edit_3->setEnabled(false);
	ui->result_status_3->setEnabled(false);
	ui->result_show_3->setEnabled(false);
    ui->result_Edit->clear();
    ui->result_show->clear();
    ui->result_status->clear();
    ui->device_duli_times->clear();
	ui->manual_duli_times->clear();
    ui->module_index->clear();
    ui->last_assay->clear();
    ui->expired_day->clear();
    ui->open_day->clear();
    ui->reagent_lot->clear();
    ui->reagent_sn->clear();
    ui->cali_lot->clear();
    ui->cali_name->clear();
    ui->curve_date->clear();
    ui->cup_number->clear();
	ui->module_index_3->clear();
	ui->dulition->clear();
	ui->device_dulition->clear();
	ui->cup_number_3->clear();
	ui->last_assay_3->clear();
	ui->sample_register_times_3->clear();
	ui->result_Edit_3->clear();
	ui->result_status_3->clear();
	ui->result_show_3->clear();
	ui->t_label_43->clear();
	ui->module_index_9->clear();
	ui->t_label_44->clear();
	ui->assay_finished_time->clear();
	
	ui->expired_day->setStyleSheet("color: #565656;");
	ui->label_11->setStyleSheet("color: #565656;");
	ui->t_abel_42->setStyleSheet("color: #565656;");
	ui->t_label_43->setStyleSheet("color: #565656;");
	ui->label_14->setStyleSheet("color: #565656;");
	ui->curve_date->setStyleSheet("color: #565656;");
	ui->label_10->setStyleSheet("color: #565656;");
	ui->open_day->setStyleSheet("color: #565656;");

    if (m_plotMagnifier != Q_NULLPTR)
    {
        m_plotMagnifier->setKeyFactor(1);
    }
}

///
/// @brief 显示曲线
///
/// @param[in]  assayResult  结果
///
/// @return true成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月6日，新建函数
///
bool QAssayResultView::DisplayCurve(ch::tf::AssayTestResult& assayResult)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (assayResult.__isset.id && assayResult.id <= 0)
	{
		ULOG(LOG_INFO, "%s(parameter error)", __FUNCTION__);
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
	QReactionCurveData::DetectCurveData primaryCurve;
	QReactionCurveData::DetectCurveData secondCurve;
	QReactionCurveData::DetectCurveData primarySecondCurve;
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
		// modify bug10625 by wuht
        primaryCurve.curveName = QString(tr("主波长"));
		primaryCurve.qwtPlot = ui->qwtPlot;
		auto spCurve = std::make_shared<QReactionCurveData>();
		spCurve->showCurve(primaryCurve);
		m_reactionDatas.push_back(spCurve);
        //ShowProcessCurve(primaryCurve);
    }

    // 次波长
    if (!secondCurve.detectPointers.empty())
    {
        secondCurve.color = QColor("#fa9016");
		secondCurve.markerColor = QColor("#b7dbfd");
		// modify bug10625 by wuht
        secondCurve.curveName = QString(tr("次波长"));
		secondCurve.qwtPlot = ui->qwtPlot;
		auto spCurve = std::make_shared<QReactionCurveData>();
		spCurve->showCurve(secondCurve);
		m_reactionDatas.push_back(spCurve);
        //ShowProcessCurve(secondCurve);
    }

    // 主-次波长
    if (!primarySecondCurve.detectPointers.empty())
    {
        primarySecondCurve.color = QColor("#41c46a");
		primarySecondCurve.markerColor = QColor("#b7dbfd");
		// modify bug10625/0011022 by wuht
        primarySecondCurve.curveName = QString(tr("主-次波长"));
		primarySecondCurve.qwtPlot = ui->qwtPlot;
		auto spCurve = std::make_shared<QReactionCurveData>();
		spCurve->showCurve(primarySecondCurve);
		m_reactionDatas.push_back(spCurve);
        //ShowProcessCurve(primarySecondCurve);
    }

    return true;
}

bool QAssayResultView::DisplayAnalyPointInfo(const ch::tf::AssayTestResult& assayResult, std::shared_ptr<CustomGrid>& spCurve)
{
	ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(assayResult));
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

void CustomGrid::draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
    QwtPlotGrid::draw(p, xMap, yMap, canvasRect);

    for (const auto& shleid : m_shiledMap)
    {
        double x1 = shleid.first; // 开始段的X坐标
        double x2 = shleid.second; // 结束段的X坐标
        p->fillRect(xMap.transform(x1), qRound(canvasRect.top()), xMap.transform(x2) - xMap.transform(x1), qRound(canvasRect.height()), QBrush(QColor("#b7dbfd")));
    }
}
