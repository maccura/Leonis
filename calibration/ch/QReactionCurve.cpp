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
/// @file     QReactionCurve.h
/// @brief    曲线
///
/// @author   5774/WuHongTao
/// @date     2022年3月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QReactionCurve.h"
#include "ui_QReactionCurve.h"
#include <QFileDialog>
#include <QDateTime>
#include <QStandardItemModel>

#include "CaliBrateCommom.h"
#include "shared/CommonInformationManager.h"
#include "shared/ReagentCommon.h"
#include "shared/tipdlg.h"
#include "shared/FileExporter.h"
#include "shared/datetimefmttool.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "QReactionCurveCtrl.h"

QReactionCurve::QReactionCurve(QWidget *parent)
    : BaseDlg(parent),
    m_PrimCurveId(-1),
    m_SecCurveId(-1),
    m_PrimSubSecSecCurveId(-1),
    m_CurvePointMode(nullptr)
{
    ui = new Ui::QReactionCurve();
    ui->setupUi(this);
    m_curvePlot = new QReactionCurveCtrl(ui->assay_result_show);

    // 关闭按钮
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(close()));
	// 打印按钮被点击
	connect(ui->print_btn_none_flat, &QPushButton::clicked, this, &QReactionCurve::OnPrintBtnClicked);
	// 导出按钮被点击
	connect(ui->export_btn_none_flat, &QPushButton::clicked, this, &QReactionCurve::OnExportBtnClicked);
    // 恢复原图比例
	connect(ui->defaultScale, &QPushButton::clicked, this, [this] {
        emit ui->times->currentIndexChanged(ui->times->currentIndex());
	});
    // 选择第几次校准
	connect(ui->times, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QReactionCurve::SelectNewCaliCurve);
    // 反应曲线
    SetTitleName(tr("反应曲线"));
}

QReactionCurve::~QReactionCurve()
{
    delete m_curvePlot;
    m_curvePlot = nullptr;
}

///
/// @brief
///     显示反应曲线信息
///
/// @param[in]  caliCurve  反应曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月28日，新建函数
///
void QReactionCurve::ShowReactionCurve(const ch::tf::CaliCurve& caliCurve)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_CalibrateResult.clear();
    m_CurveData = caliCurve;
    ch::tf::AssayTestResultQueryCond qryCond;

    for (const auto& result : caliCurve.assayTestResults)
    {
        // 测试结果主键
        qryCond.__set_id(result.second);
        ch::tf::AssayTestResultQueryResp qryResp;
        if (!ch::c1005::LogicControlProxy::QueryAssayTestResult(qryResp, qryCond) || qryResp.lstAssayTestResult.empty())
        {
            continue;
        }

        // 校准结果
        if (m_CalibrateResult.count(result.first) == 0)
        {
            std::vector<ch::tf::AssayTestResult> resultTmp;
            resultTmp.push_back(qryResp.lstAssayTestResult[0]);
            m_CalibrateResult.insert(AssayRetmap::value_type(result.first, resultTmp));
        }
        else
        {
            m_CalibrateResult[result.first].push_back(qryResp.lstAssayTestResult[0]);
        }
    }

    auto spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(caliCurve.assayCode);
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", caliCurve.assayCode);
        return;
    }

    SetComBoxContent(m_CalibrateResult);

}

///
/// @brief
///     显示曲线列表
///
/// @param[in]  curves  曲线列表
/// @param[in]  type    页面类型
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月4日，新建函数
///
void QReactionCurve::ShowCurveList(std::shared_ptr<ch::tf::CaliCurve> curve)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->export_btn_none_flat->setVisible(userPms->IsPermisson(PSM_CALI_EXPORT_CURVE));

	InitPageInfos();

    // 首先判断曲线是否为空
    if (curve == nullptr || curve->caliMode == tf::CaliMode::CALI_MODE_INTELLIGENT)
    {
        ULOG(LOG_INFO, "curve is nullptr.");
        // 没有校准曲线，也要清空波形图数据
        if (m_curvePlot != nullptr)
            m_curvePlot->DisplayCurve(ch::tf::AssayTestResult());
        SetPrintAndExportEnabled(false);
        m_CurveData = ch::tf::CaliCurve();
        return;
    }

    ULOG(LOG_INFO, "curve=%s.", ToString(*curve));

    SetPrintAndExportEnabled(true);

    m_CurveData = *curve;

    // 显示曲线数据
    SetCalibrateInfos(m_CurveData);

    // 首先显示当前的页面
    ShowReactionCurve(m_CurveData);
}

///
/// @brief
///     根据测试结果设置选择框
///
/// @param[in]  CalibrateResult  测试结果
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月3日，新建函数
///
void QReactionCurve::SetComBoxContent(AssayRetmap& CalibrateResult)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (CalibrateResult.empty())
    {
        return;
    }

	// 组装校准品次数
	QStringList items;
	ui->times->clear();

	for (const auto& calibrate : CalibrateResult)
	{
		QString CompostionString = tr("校准品") + QString::number(calibrate.first);

		if (calibrate.second.size() == 2)
		{
			ui->times->addItem(CompostionString + tr("第一次"), QString("%1|%2").arg(calibrate.first).arg(0));
			ui->times->addItem(CompostionString + tr("第二次"), QString("%1|%2").arg(calibrate.first).arg(1));
		}
		else if (calibrate.second.size() == 1)
		{
			ui->times->addItem(CompostionString + tr("第一次"), QString("%1|%2").arg(calibrate.first).arg(0));
		}
	}

	emit ui->times->currentIndexChanged(0);
}

///
/// @brief 初始化页面内容
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月8日，新建函数
///
void QReactionCurve::InitPageInfos()
{
	ClearViewContent();
	InitLabels();
	InitPlot();

    //QFont cosFont/* = ui->plotTitle_XBottom->font()*/;
    //cosFont.setFamily("HarmonyOS Sans SC");
    //cosFont.setPixelSize(14);
    //ui->plotTitle_XBottom->setFont(ui->lb_assName->font());
	//ui->assay_result_show->ClearContent();
}

///
/// @brief 初始化顶部标签
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月8日，新建函数
///
void QReactionCurve::InitLabels()
{
	ui->lb_assName->setText(QString(tr("项目名称：")));
	ui->lb_assLot->setText(QString(tr("试剂批号：")));
	ui->lb_mudule->setText(QString(tr("模块：")) + " ");
	ui->lb_caliDate->setText(QString(tr("校准时间：")));
	ui->lb_caliName->setText(QString(tr("校准品名称：")));
	ui->lb_assBot->setText(QString(tr("试剂瓶号：")));
	ui->lb_tupple->setText(QString(tr("杯号：")));
	ui->lb_resultState->setText(QString(tr("结果状态：")));
	ui->lb_same->setText(QString(tr("同杯前反应：")));
    ui->lb_con->setText(QString(tr("结果：")));
    ui->lb_caliMode->setText(QString(tr("执行方法：")));
}

///
/// @brief 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月8日，新建函数
///
void QReactionCurve::InitPlot()
{
	ui->times->clear();
}

///
/// @brief 清空测光点的表格内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QReactionCurve::ClearViewContent()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (m_CurvePointMode == nullptr)
	{
		m_CurvePointMode = new QStandardItemModel(this);
		ui->result_process_view->setModel(m_CurvePointMode);
		ui->result_process_view->setSelectionBehavior(QAbstractItemView::SelectRows);
		ui->result_process_view->setSelectionMode(QAbstractItemView::SingleSelection);
		ui->result_process_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	m_CurvePointMode->clear();
	QStringList CurveTypeShowList;
	CurveTypeShowList << tr("测光点") << tr("主波长") << tr("次波长") << tr("主-次");
	m_CurvePointMode->setHorizontalHeaderLabels(CurveTypeShowList);
	m_CurvePointMode->setRowCount(0);

	ui->result_process_view->setColumnWidth(0, 80);
	ui->result_process_view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui->result_process_view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	ui->result_process_view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

///
/// @brief
///     显示曲线的检测点
///
/// @param[in]  result  曲线检测点
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
void QReactionCurve::ShowDetectPointView(ch::tf::AssayTestResult& result)
{
	int primWave = -1;
	int subWave = -1;

	ClearViewContent();

	// 仅有主波长
	if (result.primarySubWaves.size() == 1)
	{
		primWave = result.primarySubWaves[0];
	}
	// 主次波长都有
	else if (result.primarySubWaves.size() >= 2)
	{
		primWave = result.primarySubWaves[0];
		subWave = result.primarySubWaves[1];
	}

	// 依次显示内容
	int row = 0;
	for (auto detectPoint : result.detectPoints)
	{
		int column = 0;
		// 测光点编号
		m_CurvePointMode->setItem(row, column++, CenterAligmentItem(QString::number(detectPoint.pointSN)));
		// 主波长
		if (primWave == -1)
		{
			m_CurvePointMode->setItem(row, column++, CenterAligmentItem(""));
		}
		else
		{
			m_CurvePointMode->setItem(row, column++, CenterAligmentItem(QString::number(detectPoint.ods[primWave])));
		}

		if (subWave == -1)
		{
			m_CurvePointMode->setItem(row, column++, CenterAligmentItem(""));
			m_CurvePointMode->setItem(row, column++, CenterAligmentItem(""));
		}
		else
		{
			// 次波长
			m_CurvePointMode->setItem(row, column++, CenterAligmentItem(QString::number(detectPoint.ods[subWave])));
			// 主-次
			m_CurvePointMode->setItem(row, column++, CenterAligmentItem(QString::number(detectPoint.ods[primWave] - detectPoint.ods[subWave])));
		}

		row++;
	}

    // 设置表格行数
    m_CurvePointMode->setRowCount(row);
}

///
/// @brief 设置曲线对应的校准品信息
///
/// @param[in]  result  项目测试结果
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月12日，新建函数
///
void QReactionCurve::SetCalibrateInfos(const ch::tf::CaliCurve & curve)
{
	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(curve.assayCode);

	ui->lb_assName->setText(QString(tr("项目名称：")) + QString::fromStdString(spAssayInfo->assayName));
	ui->lb_assLot->setText(QString(tr("试剂批号：")) + QString::fromStdString(curve.reagentLot));
	ui->lb_caliDate->setText(QString(tr("校准时间：")) + ToCfgFmtDateTime(QString::fromStdString(curve.caliTime)));
	ui->lb_assBot->setText(QString(tr("试剂瓶号：")) + QString::fromStdString(curve.reagentKeyInfos[0].sn));
	ui->lb_mudule->setText(QString(tr("模块：")) + QString::fromStdString(CommonInformationManager::GetDeviceName(curve.deviceSN)));
    ui->lb_caliName->setText(QString(tr("校准品名称：")) + QString::fromStdString(curve.calibratorName));
    ui->lb_caliMode->setText(QString(tr("执行方法：")) + CaliBrateCommom::GetCaliBrateMode(curve.caliMode));
}

///
/// @brief 校准品曲线变化
///
/// @param[in]  index  发生变化项目的索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月13日，新建函数
///
void QReactionCurve::SelectNewCaliCurve(int index)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	QString data = ui->times->itemData(index).toString();

	if (data.isEmpty() || data.split("|").size() != 2)
	{
		return;
	}


	// 获取校准品的水平
	int level = data.split("|").at(0).toInt();

	// 查找校准品
	if (m_CalibrateResult.count(level) == 0)
	{
		ULOG(LOG_ERROR, "%s()", "can not find the calibrate level");
		return;
	}

	int timeStatus = data.split("|").at(1).toInt();
	// 次数错误
	if (timeStatus > m_CalibrateResult[level].size() - 1)
	{
		ULOG(LOG_ERROR, "%s()", "the times is error");
		return;
	}

	//ui->assay_result_show->ShowAssayResultReactionProcess(m_CalibrateResult[level][timeStatus]);

	// 显示波长的数值
	ShowDetectPointView(m_CalibrateResult[level][timeStatus]);
    m_curvePlot->DisplayCurve(m_CalibrateResult[level][timeStatus]);

    try
    {
        const auto& calibrateCurv = ch::CreateCaliCurve(m_CurveData.curveData);
        if (calibrateCurv)
        {
            const auto& points = calibrateCurv->GetAlgorithmConfigs().analyPoint;
            std::vector<int> anaPoints;
            if (points.m)
                anaPoints.push_back(*(points.m));
            if (points.n)
                anaPoints.push_back(*(points.n));
            if (points.p)
                anaPoints.push_back(*(points.p));
            if (points.q)
                anaPoints.push_back(*(points.q));
            m_curvePlot->SetDetectPoints(anaPoints, calibrateCurv->GetAlgorithmConfigs().analysis_method);
        }
    }
    catch (const std::exception&)
    {
        ULOG(LOG_ERROR, "Exception!");
    }

	// 展示当前校准品的部分信息
	ui->lb_tupple->setText(QString(tr("杯号：")) + QString::number(m_CalibrateResult[level][timeStatus].cupSN));
	ui->lb_same->setText(QString(tr("同杯前反应：") + GetPreAssayName(m_CalibrateResult[level][timeStatus].previousAssayCode)));
	ui->lb_con->setText(QString(tr("结果：")) + QString::number(m_CalibrateResult[level][timeStatus].conc));
    ui->lb_caliDate->setText(QString(tr("校准时间：")) + ToCfgFmtDateTime(QString::fromStdString(m_CalibrateResult[level][timeStatus].endTime)));
    //ui->lb_caliName->setText(QString(tr("校准品名称：")) + QString::fromStdString(m_CalibrateResult[level][timeStatus].resultCaliInfo.calibratorName));
	ui->lb_resultState->setText(QString(tr("结果状态：")) + QString::fromStdString(m_CalibrateResult[level][timeStatus].resultStatusCodes));
}

///
/// @brief 打印按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
void QReactionCurve::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ReactCurveData Info;
    GetReactionCurvePrintData(Info);
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strPrintTime = strPrintTime.toStdString();

    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
}

///
/// @brief 导出按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
void QReactionCurve::OnExportBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!m_CurveData.__isset.id ||
        m_CurveData.id <= 0 ||
        (m_CurveData.__isset.isEditCurve &&
            m_CurveData.isEditCurve))
    {
        TipDlg(tr("无可用数据导出!")).exec();
        return;
    }

	// 弹出保存文件对话框
	QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
	if (fileName.isNull() || fileName.isEmpty())
	{
		//std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存文件错误！")));
		//pTipDlg->exec();
		return;
    }

    QFileInfo file(fileName);
    bool bRet = true;

    if (file.suffix() == "pdf")
    {
        ReactCurveData Info;
        GetReactionCurvePrintData(Info);
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        Info.strExportTime = strPrintTime.toStdString();

        std::string strInfo = GetJsonString(Info);

        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliReactCurve.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRet = true;
    }
    else
    {
        QStringList strExportTextList;

        // 反应曲线信息
        if (!SetReactionCurveInfo(strExportTextList))
        {
            // 弹框提示导出失败
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据获取失败！")));
            pTipDlg->exec();

            return;
        }

        // 换行
        strExportTextList.push_back("");

        // 曲线校准信息
        if (!SetCurveCaliInfo(strExportTextList))
        {
            // 弹框提示导出失败
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据获取失败！")));
            pTipDlg->exec();

            return;
        }

        // 导出文件
        FileExporter fileExporter;
        bRet = fileExporter.ExportInfoToFile(strExportTextList, fileName);
    }

	// 弹框提示导出失败
	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")));
	pTipDlg->exec();

}

///
/// @brief 设置反应曲线（导出使用）
///
/// @param[in] strExportTextList   导出的数据
///
/// @return true:设置成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
bool QReactionCurve::SetReactionCurveInfo(QStringList& strExportTextList)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 创建表头
	QString strHeader("");
	strHeader += QObject::tr("项目名称") + "\t";
    strHeader += QObject::tr("模块") + "\t";
    strHeader += tr("校准品名称") + "\t";
	strHeader += QObject::tr("试剂批号") + "\t";
    strHeader += QObject::tr("试剂瓶号") + "\t";
    strHeader += tr("执行方法") + "\t";
	strExportTextList.push_back(std::move(strHeader));

	QString rowData = "";

	// 1:项目名称
	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(m_CurveData.assayCode);
	rowData += QString::fromStdString((spAssay == nullptr) ? "" : spAssay->assayName);
	rowData += "\t";
	// 2:模块
	rowData += QString::fromStdString(CommonInformationManager::GetDeviceName(m_CurveData.deviceSN));
    rowData += "\t";
    // 3:校准品名称
    rowData += QString::fromStdString(m_CurveData.calibratorName) + "\t";
	// 4:试剂批号
	rowData += QString::fromStdString(m_CurveData.reagentLot);
	rowData += "\t";
	// 5:试剂瓶号
	rowData += QString::fromStdString((m_CurveData.reagentKeyInfos.empty()) ? "" : m_CurveData.reagentKeyInfos[0].sn);
    rowData += "\t";
    // 6:执行方法
    rowData += CaliBrateCommom::GetCaliBrateMode(m_CurveData.caliMode) + "\t";
	
	strExportTextList.push_back(std::move(rowData));

	return true;

}

///
/// @brief 设置曲线校准信息（导出使用）
///
/// @param[in] strExportTextList   导出的数据
///
/// @return true:设置成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
bool QReactionCurve::SetCurveCaliInfo(QStringList& strExportTextList)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_CalibrateResult.empty())
	{
		return false;
	}

	// 填充结果基本信息
	auto setBaseResultInfo = [&strExportTextList](const QString& calibartorTimes, const ch::tf::AssayTestResult& testResult)
	{
		// 填充标题
		strExportTextList.push_back(calibartorTimes);

		// 填充基本信息
		QString strBaseResultHeader("");
		strBaseResultHeader += QObject::tr("结果") + "\t";
		strBaseResultHeader += QObject::tr("结果状态") + "\t";
		strBaseResultHeader += QObject::tr("校准时间") + "\t";
		strBaseResultHeader += QObject::tr("杯号") + "\t";
		strBaseResultHeader += QObject::tr("同杯前反应") + "\t";
		strExportTextList.push_back(std::move(strBaseResultHeader));

		QString baseResultRowData = "";

		// 1:结果
		baseResultRowData += QString::number(testResult.conc);
		baseResultRowData += "\t";
		// 2:结果状态
		baseResultRowData += QString::fromStdString(testResult.resultStatusCodes);
		baseResultRowData += "\t";
		// 3:校准品名称
		//baseResultRowData += QString::fromStdString(testResult.resultCaliInfo.calibratorName);
		//baseResultRowData += "\t";
		// 4:校准时间
		baseResultRowData += QString::fromStdString(testResult.endTime);
		baseResultRowData += "\t";
		// 5:杯号
		baseResultRowData += QString::number(testResult.cupSN);
		baseResultRowData += "\t";
		// 6:同杯前反应
		baseResultRowData += GetPreAssayName(testResult.previousAssayCode);
		baseResultRowData += "\t";

		strExportTextList.push_back(std::move(baseResultRowData));

	};

	// 填充测光点信息
	auto setLightPointInfo = [&strExportTextList](const ch::tf::AssayTestResult& testResult)
	{
		// 填充基本信息表头
		QString strLightPointHeader("");
		strLightPointHeader += QObject::tr("测光点") + "\t";
        QString strLightPointPrimary("");
        strLightPointPrimary += QObject::tr("主波长") + "\t";
        QString strLightPointSub("");
        strLightPointSub += QObject::tr("次波长") + "\t";
        QString strLightPointMinus("");
        strLightPointMinus += QObject::tr("主-次波长") + "\t";
		//strExportTextList.push_back(std::move(strLightPointHeader));

		int primWave = -1;
		int subWave = -1;

		// 仅有主波长
		if (testResult.primarySubWaves.size() == 1)
		{
			primWave = testResult.primarySubWaves[0];
		}
		// 主次波长都有
		else if (testResult.primarySubWaves.size() >= 2)
		{
			primWave = testResult.primarySubWaves[0];
			subWave = testResult.primarySubWaves[1];
		}

		for (auto detectPoint : testResult.detectPoints)
		{
			QString lightPointRowData = "";

			// 1:测光点编号
            strLightPointHeader += QString::number(detectPoint.pointSN);
            strLightPointHeader += "\t";
			// 2:主波长
            strLightPointPrimary += (primWave == -1 ?"": QString::number(detectPoint.ods[primWave]));
            strLightPointPrimary += "\t";
			// 3:次波长
            strLightPointSub += (subWave == -1 ? "" : QString::number(detectPoint.ods[subWave]));
            strLightPointSub += "\t";
			// 4:主-次波长
            strLightPointMinus += (subWave == -1 ? "" : QString::number(detectPoint.ods[primWave]-detectPoint.ods[subWave]));
            strLightPointMinus += "\t";
        }

        strExportTextList.push_back(std::move(strLightPointHeader));
        strExportTextList.push_back(std::move(strLightPointPrimary));
        strExportTextList.push_back(std::move(strLightPointSub));
        strExportTextList.push_back(std::move(strLightPointMinus));
	};
	
	// 遍历填充信息（由于map是无序的，所以用size来遍历）
	for (int level = 1; level <= m_CalibrateResult.size(); level++)
	{
		// 查找校准品
		if (m_CalibrateResult.count(level) == 0)
		{
			continue;
		}

		// 遍历次数
		int times = 1;
		for (const auto& result : m_CalibrateResult[level])
		{
			QString calibartorTimes = QObject::tr("校准品") + QString::number(level) + 
									  QObject::tr("  第") + QString::number(times)+QObject::tr("次");
			// 填充结果基本信息
			setBaseResultInfo(calibartorTimes, result);
			// 填充测光点信息
			setLightPointInfo(result);
            // 反应曲线图
			
			// 空行
			strExportTextList.push_back("");
			times++;
		}

		// 空行
		strExportTextList.push_back("");
	}

	return true;
}


///
/// @brief 获取反应曲线打印数据（打印使用）
///
/// @param[in] rceactionCurveInfo   反应曲线打印数据
///
/// @return true:获取成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
bool QReactionCurve::GetReactionCurvePrintData(ReactCurveData& rceactionCurveInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    /******反应曲线信息******/
    // 1:项目名称
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(m_CurveData.assayCode);
    if (spAssay == nullptr)
        return {};

	rceactionCurveInfo.strReagentName = spAssay->assayName;
	// 2:模块
	rceactionCurveInfo.strModelName = CommonInformationManager::GetDeviceName(m_CurveData.deviceSN);
    // 校准品名称

	// 3:试剂批号
	rceactionCurveInfo.strReagentLot = m_CurveData.reagentLot;
	// 4:试剂瓶号
	rceactionCurveInfo.strReagentSN = m_CurveData.reagentKeyInfos[0].sn;
    // 执行方法
    rceactionCurveInfo.strCaliMode = CaliBrateCommom::GetCaliBrateMode(m_CurveData.caliMode).toStdString();

    std::vector<QString> chineseNum({ tr("一"),tr("二"),tr("三"),tr("四"),tr("五"),tr("六"),tr("七"),tr("八"),tr("九"),tr("十"), });
    ushort code = QChar(tr("一")[0]).unicode();

	/******每个校准品的测试数据******/
	// 遍历填充信息（由于map是无序的，所以用size来遍历）
	for (int level = 1; level <= m_CalibrateResult.size(); level++)
	{
        CaliCurveTimesData timesData;
		// 查找校准品
		if (m_CalibrateResult.count(level) == 0)
		{
			continue;
		}

        // 遍历次数
        int i = 0;
		for (const auto& result : m_CalibrateResult[level])
		{
			// 1:结果
			//pCTI.dResult = result.conc;
			// 2:结果状态
			//pCTI.strResultStatus = result.resultStatusCodes;
			// 3:校准品名称
            rceactionCurveInfo.strCaliName = result.resultCaliInfo.calibratorName;
            
            // 校准品次序
            QString times = (i < chineseNum.size() ? chineseNum[i] : QString::number(i + 1));
            timesData.strCaliSeqNo = tr("校准品%1第%2次").arg(level).arg(times).toStdString();
			// 4:校准时间
			//pCTI.strCalibrateDate = result.endTime;
			// 5:杯号
			//pCTI.uCupNumber = result.cupSN;
			// 6:同杯前反应
			//pCTI.strPreviousCupReaction = GetPreAssayName(result.previousAssayCode).toStdString();
			
			/******测光点数据******/
			int primWave = -1;
			int subWave = -1;

			// 仅有主波长
			if (result.primarySubWaves.size() == 1)
			{
				primWave = result.primarySubWaves[0];
			}
			// 主次波长都有
			else if (result.primarySubWaves.size() >= 2)
			{
				primWave = result.primarySubWaves[0];
				subWave = result.primarySubWaves[1];
			}

            timesData.vecCaliLvData.clear();
			// 遍历测光点
			for (auto detectPoint : result.detectPoints)
			{
                CaliCurveLvData  pCTI;

				// 1:测光点编号
                pCTI.strPonitIndex = to_string(detectPoint.pointSN);
				// 2:主波长
                pCTI.strPrimaryWave = to_string(detectPoint.ods[primWave]);
				// 3:次波长
                if (subWave != -1)
                    pCTI.strSubWave =to_string(detectPoint.ods[subWave]);
                // 4:主-次波长
                if (subWave != -1)
                    pCTI.strWaveDiff = to_string(detectPoint.ods[primWave] - detectPoint.ods[subWave]);

                timesData.vecCaliLvData.push_back(std::move(pCTI));
			}

            ++i;
            rceactionCurveInfo.vecCaliTimesData.push_back(timesData);
		}
	}

	return true;
}

void QReactionCurve::SetPrintAndExportEnabled(bool hasCurve)
{
    // 暂时只是用是否有当前工作曲线
    ui->print_btn_none_flat->setEnabled(hasCurve);
    ui->export_btn_none_flat->setEnabled(hasCurve);
}
