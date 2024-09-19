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
/// @file     QCupBlankCalcDlg.cpp
/// @brief    杯空白计算对话框
///
/// @author   8580/GongZhiQiang
/// @date     2024年7月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QCupBlankCalcDlg.h"
#include "ui_QCupBlankCalcDlg.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <QDateTime>
#include "src/common/Mlog/mlog.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "shared/tipdlg.h"
#include "shared/FileExporter.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "thrift/ch/ChLogicControlProxy.h"
#include "datetimefmttool.h"
#include "QCupBlankTestResultDlg.h"

QCupBlankCalcDlg::QCupBlankCalcDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    // 初始化ui对象
    ui = new Ui::QCupBlankCalcDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

QCupBlankCalcDlg::~QCupBlankCalcDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief 设置当前设备信息
///
/// @param[in]  devInfo 设备信息（模块名称和SN）
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
void QCupBlankCalcDlg::SetDevSnInfo(const QPair<QString, QString>& devInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	m_devNameOfSN = devInfo;
}

///
/// @brief 显示前初始化
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
///
void QCupBlankCalcDlg::InitBeforeShow()
{
    // 设置标题
    SetTitleName(tr("计算"));

	// 初始化表格控件
	InitTableCtrl();

	// 初始化时间下拉框
	InitTimeComboBox();
}

// 初始化表格控件
void QCupBlankCalcDlg::InitTableCtrl()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 初始化表格 
	m_pStdModel = new QStandardItemModel();
	ui->tableView_result->setModel(m_pStdModel);

	// 隐藏垂直表头
	ui->tableView_result->verticalHeader()->setVisible(false);

	// 设置表格选中模式为行选中，不可多选，不可编辑
	ui->tableView_result->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_result->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView_result->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// 设置试剂信息表默认行数和列数
	m_pStdModel->setRowCount(REACTION_DISK_NUMBER_OF_CUPS);
	m_pStdModel->setColumnCount(DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE);

	// 设置表头
	QStringList strHeaderList;
	strHeaderList << tr(u8"反应杯号") << tr(u8"340nm") << tr(u8"380nm") << tr(u8"405nm") << tr(u8"450nm") << tr(u8"480nm")
		<< tr(u8"505nm") << tr(u8"546nm") << tr(u8"570nm") << tr(u8"600nm") << tr(u8"660nm") << tr(u8"700nm") << tr(u8"750nm") << tr(u8"800nm");
	m_pStdModel->setHorizontalHeaderLabels(strHeaderList);

	 // 设置列宽
	for (int colIndex = 0; colIndex < DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE; colIndex++)
	{
		ui->tableView_result->setColumnWidth(colIndex, ui->tableView_result->width() / DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE);
	}


	//ui->tableView_result->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	// 设置行高
	for (int rowIndex = 0; rowIndex < DEFAULT_ROW_CNT_OF_BLANK_RESULT_TABLE; rowIndex++)
	{
		ui->tableView_result->setRowHeight(rowIndex, ui->tableView_result->height() / (DEFAULT_ROW_CNT_OF_BLANK_RESULT_TABLE + 1));
	}

}

///
/// @brief 初始化时间下拉框
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
void QCupBlankCalcDlg::InitTimeComboBox()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空
	ui->comboBox_exeTime_1->clear();
	ui->comboBox_exeTime_2->clear();
}

///
/// @brief 更新时间下拉框
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
void QCupBlankCalcDlg::UpdateTimeComboBox()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询杯空白维护结果
	ch::tf::MaintainItemResultInfoQueryCond miriqc;
	miriqc.__set_deviceSN(m_devNameOfSN.second.toStdString());
	miriqc.__set_itemType(tf::MaintainItemType::MAINTAIN_ITEM_CH_WATER_BLANK);

	ch::tf::MaintainItemResultInfoQueryResp miriqr;
	ch::LogicControlProxy::QueryMaintainItemResultInfo(miriqr, miriqc);
	if (miriqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
		miriqr.lstMaintainItemResultInfo.empty())
	{
		ULOG(LOG_ERROR, "QueryMaintainItemResultInfo Failed !");
		return;
	}

	// 遍历添加
	int index = 0;
	for (auto resultInfo : miriqr.lstMaintainItemResultInfo)
	{
		// 添加到显示
		ui->comboBox_exeTime_1->addItem(ToCfgFmtDateTime(QString::fromStdString(resultInfo.exeTime)), QString::fromStdString(resultInfo.exeTime));
		ui->comboBox_exeTime_2->addItem(ToCfgFmtDateTime(QString::fromStdString(resultInfo.exeTime)), QString::fromStdString(resultInfo.exeTime));
	}

	// 设置为最后一条数据
	ui->comboBox_exeTime_1->setCurrentIndex(miriqr.lstMaintainItemResultInfo.size() - 1);
	ui->comboBox_exeTime_2->setCurrentIndex(miriqr.lstMaintainItemResultInfo.size() - 1);
}

void QCupBlankCalcDlg::InitAfterShow()
{
    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

void QCupBlankCalcDlg::InitConnect()
{
    // 关闭按钮
    connect(ui->btn_close, SIGNAL(clicked()), this, SLOT(reject()));

	connect(ui->comboBox_exeTime_1, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		[=](int index) { 

		// 时间改变则清空
		std::string time1 = ui->comboBox_exeTime_1->itemData(ui->comboBox_exeTime_1->currentIndex()).toString().toStdString();
		if (m_calculateData.time1 != time1 && !m_calculateData.time1.empty())
		{
			ClearData();
		}
	});

	connect(ui->comboBox_exeTime_2, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		[=](int index) {
		
		// 时间改变则清空
		std::string time2 = ui->comboBox_exeTime_2->itemData(ui->comboBox_exeTime_2->currentIndex()).toString().toStdString();
		if (m_calculateData.time2 != time2 && !m_calculateData.time2.empty())
		{
			ClearData();
		}

	});

    // 计算按钮
    connect(ui->btn_calculate, SIGNAL(clicked()), this, SLOT(OnBtnCalculateClicked()));

	// 导出按钮
	connect(ui->btn_export, SIGNAL(clicked()), this, SLOT(OnBtnExportClicked()));

	// 打印按钮
	connect(ui->btn_print, SIGNAL(clicked()), this, SLOT(OnBtnPrintClicked()));
}

///
/// @brief 初始化子控件
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
///
void QCupBlankCalcDlg::InitChildCtrl()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 更新时间下拉框
	UpdateTimeComboBox();

	// 清空数据
	ClearData();
}

///
/// @brief 计算按钮点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
///
void QCupBlankCalcDlg::OnBtnCalculateClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空数据
	ClearData();

	// 时间判断
	if (ui->comboBox_exeTime_1->currentIndex() == ui->comboBox_exeTime_2->currentIndex())
	{
		TipDlg(tr("测定时间1不能与测定时间2相同！")).exec();
		return;
	}

	// 获取测定时间1结果
	std::string time1 = ui->comboBox_exeTime_1->itemData(ui->comboBox_exeTime_1->currentIndex()).toString().toStdString();
	QMap<int, QVector<int>> data1;
	if (!GetWaterBlankResult(time1, data1))
	{
		TipDlg(tr("获取测定时间1数据失败！")).exec();
		return;
	}

	// 获取测定时间2结果
	std::string time2 = ui->comboBox_exeTime_2->itemData(ui->comboBox_exeTime_2->currentIndex()).toString().toStdString();
	QMap<int, QVector<int>> data2;
	if (!GetWaterBlankResult(time2, data2))
	{
		TipDlg(tr("获取测定时间2数据失败！")).exec();
		return;
	}

	// 计算
	if (!CalculateAbsData(data1, data2, m_calculateData))
	{
		TipDlg(tr("计算失败！")).exec();
		return;
	}

	// 缓存时间
	m_calculateData.time1 = std::move(time1);
	m_calculateData.time2 = std::move(time2);

	// 刷新表格
	UpdateTableInfo();
}

///
/// @brief 获取杯空白测定结果
///
/// @param[in]  time  测定时间  
/// @param[in]  adcData  结果数据（杯号，Vec(各个波长ADC值）)
///
/// @return true：获取成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
bool QCupBlankCalcDlg::GetWaterBlankResult(const std::string& time, QMap<int, QVector<int>>& adcData)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 参数检查
	if (time.empty())
	{
		ULOG(LOG_ERROR, "Time is empty!");
		return false;
	}

	// 查询指定设备时间杯空白测定数据
	ch::tf::MaintainItemResultInfoQueryCond miriqc;
	miriqc.__set_deviceSN(m_devNameOfSN.second.toStdString());
	miriqc.__set_itemType(tf::MaintainItemType::MAINTAIN_ITEM_CH_WATER_BLANK);
	miriqc.__set_exeTime(time);

	// 查询
	ch::tf::MaintainItemResultInfoQueryResp miriqr;
	ch::LogicControlProxy::QueryMaintainItemResultInfo(miriqr, miriqc);
	if (miriqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
		miriqr.lstMaintainItemResultInfo.size() != 1)
	{
		ULOG(LOG_ERROR, "QueryMaintainItemResultInfo Failed !");
		return false;
	}

	// 结果
	auto resultInfo = miriqr.lstMaintainItemResultInfo[0];
	// 解析JSON
	if (resultInfo.params.empty())
	{
		ULOG(LOG_ERROR, "MAINTAIN_ITEM_CH_WATER_BLANK result is empty!");
		return false;
	}

	// 解析维护项参数
	try
	{
		Document jsonDoc;
		if (!RapidjsonUtil::ParseRapidjson(resultInfo.params, jsonDoc))
		{
			ULOG(LOG_ERROR, "Failed to parse json string: %s", resultInfo.params);
			return false;
		}

		const Value& vResultSet = RapidjsonUtil::GetArrayForKey(jsonDoc, "waterBlankData");
		// 杯号索引
		int cupNumIndex = 1;
		// 波长索引
		int waveIndex = 0;
		QVector<int> cupAdcTemp;
		for (auto& rt : vResultSet.GetArray())
		{
			cupAdcTemp.clear();
			const Value& cupAdc = RapidjsonUtil::GetArrayForKey(rt, QString::number(cupNumIndex).toUtf8().constData());
			// 索引初始化
			waveIndex = 0;
			for (auto& adc : cupAdc.GetArray())
			{
				// 直接过滤掉保留波长
				if (waveIndex == CUPBLANK_WAVE_INDEX::WAVE_RESERVE1_INDEX ||
					waveIndex == CUPBLANK_WAVE_INDEX::WAVE_RESERVE2_INDEX ||
					waveIndex == CUPBLANK_WAVE_INDEX::WAVE_RESERVE3_INDEX)
				{
					waveIndex++;
					continue;
				}

				// ADC值
				int adcVal = adc.GetInt();
				cupAdcTemp.push_back(adcVal);
				waveIndex++;
			}

			// 存储数据
			adcData.insert(cupNumIndex, cupAdcTemp);
			cupNumIndex++;
		}

	}
	catch (exception& ex)
	{
		ULOG(LOG_ERROR, "exception: %s", ex.what());
		return false;
	}

	return true;
}

///
/// @brief 计算吸光度
///
/// @param[in]  time1Data 测定时间1结果数据
/// @param[in]  time2Data 测定时间2结果数据
/// @param[in]  rData	  返回的计算结果
///
/// @return true：计算成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
bool QCupBlankCalcDlg::CalculateAbsData(const QMap<int, QVector<int>>& time1Data, const QMap<int, QVector<int>>& time2Data, CalculateData& rData)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 参数判断
	if (time1Data.size() != REACTION_DISK_NUMBER_OF_CUPS || 
		time1Data.size() != REACTION_DISK_NUMBER_OF_CUPS)
	{
		ULOG(LOG_ERROR, "time1Data or time2Data size error!");
		return false;
	}

	// 计算值
	try
	{
		QMap<int, QVector<int>> calAbsData;
		for (int cupIndex = 1; cupIndex <= REACTION_DISK_NUMBER_OF_CUPS; cupIndex++)
		{
			// 索引判断
			if (!time1Data.contains(cupIndex) || 
				!time2Data.contains(cupIndex))
			{
				ULOG(LOG_ERROR, "time1Data or time2Data don't contains cup(%d) adc value!", cupIndex);
				return false;
			}

			// 数据判断
			if (time1Data[cupIndex].size() != time2Data[cupIndex].size())
			{
				ULOG(LOG_ERROR, "time1Data and time2Data cup(%d) adc size don't equal!", cupIndex);
				return false;
			}

			// 遍历计算吸光度
			QVector<int> absDataTemp;
			for (int waveIndex = 0; waveIndex < time1Data[cupIndex].size(); waveIndex++)
			{
				// 计算吸光度
				auto absTemp = time2Data[cupIndex][waveIndex] <= 0 ? 0 : ch::CalcSinglePointAbs(time1Data[cupIndex][waveIndex], time2Data[cupIndex][waveIndex], CUP_BLANK_OPTICAL_PATH_LENGTH);
				absDataTemp.push_back(std::move(absTemp));
			}

			// 添加进缓存
			calAbsData.insert(cupIndex, std::move(absDataTemp));
		}

		rData.absValue = std::move(calAbsData);
	}
	catch (exception& ex)
	{
		ULOG(LOG_ERROR, "exception: %s", ex.what());
		return false;
	}
	return true;
}

///
/// @brief 更新表格信息
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
void QCupBlankCalcDlg::UpdateTableInfo()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空表格内容
	m_pStdModel->setRowCount(0);

	if (m_calculateData.absValue.size() != REACTION_DISK_NUMBER_OF_CUPS)
	{
		ULOG(LOG_ERROR, "m_calculateData size error!");
		return;
	}

	auto getStandardItem = [&](QString displayText) {
		QStandardItem* item = new QStandardItem(displayText);
		item->setTextAlignment(Qt::AlignCenter);
		return item;
	};

	// 刷新表格
	int rowIndex = 0;
	QList<QStandardItem *> rowItems;
	for (int cupIndex = 1; cupIndex <= REACTION_DISK_NUMBER_OF_CUPS; cupIndex++)
	{
		// 初始化当前行
		rowItems.clear();

		// 杯号
		rowItems.append(getStandardItem(QString::number(cupIndex)));

		// 是否存在对应杯号数据判断
		if (!m_calculateData.absValue.contains(cupIndex))
		{
			continue;
		}

		// 列索引
		int colIndex = 1;
		for (auto value : m_calculateData.absValue[cupIndex])
		{
			// 如果超出界面显示波长的数量，则退出
			if (colIndex > (DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE - 1))
			{
				break;
			}

			rowItems.append(getStandardItem(QString::number(value)));

			colIndex++;
		}

		m_pStdModel->insertRow(rowIndex, rowItems);
		rowIndex++;
	}

}

///
/// @brief 显示事件处理
///
/// @param[in]  event  事件
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
///
void QCupBlankCalcDlg::showEvent(QShowEvent *event)
{
	// 让基类处理事件
	BaseDlg::showEvent(event);

	// 第一次显示时初始化
	if (!m_bInit)
	{
		m_bInit = true;
		InitAfterShow();
	}
}

///
/// @brief 导出按钮点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
///
void QCupBlankCalcDlg::OnBtnExportClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 参数判断
	if (m_calculateData.absValue.empty())
	{
		TipDlg(tr("计算结果为空，不可导出！")).exec();
		return;
	}

	// 弹出保存文件对话框
	QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
	if (fileName.isNull() || fileName.isEmpty())
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存文件错误！")));
		pTipDlg->exec();
		return;
	}

	bool bRet = false;
	QFileInfo FileInfo(fileName);
	QString strSuffix = FileInfo.suffix();
	if (strSuffix == "pdf")
	{
		ChCupBlankCalculateInfo Info;
		GetPrintExportInfo(Info);
		std::string strInfo = GetJsonString(Info);
		ULOG(LOG_INFO, "Print datas : %s", strInfo);
		std::string unique_id;
		QString strDirPath = QCoreApplication::applicationDirPath();
		QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportChCupCalculateResult.lrxml";
		int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
		ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
		bRet = true;
	}
	else
	{
		QStringList strExportTextList;
		GetExportExcelData(strExportTextList);

		// 导出文件
		FileExporter fileExporter;
		bRet = fileExporter.ExportInfoToFile(strExportTextList, fileName);

	}

	// 弹框提示导出失败
	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")));
	pTipDlg->exec();
}

///
/// @brief 打印按钮点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
///
void QCupBlankCalcDlg::OnBtnPrintClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 参数判断
	if (m_calculateData.absValue.empty())
	{
		TipDlg(tr("计算结果为空，不可打印！")).exec();
		return;
	}

	ChCupBlankCalculateInfo Info;
	GetPrintExportInfo(Info);
	std::string strInfo = GetJsonString(Info);
	ULOG(LOG_INFO, "Print datas : %s", strInfo);
	std::string unique_id;
	int irect = printcom::printcom_async_print(strInfo, unique_id);
	ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
}

///
/// @brief 获取打印导出数据
///
/// @param[in]  info  杯空白计算打印导出数据
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
void QCupBlankCalcDlg::GetPrintExportInfo(ChCupBlankCalculateInfo& info)
{
	// 模块
	info.strModel = m_devNameOfSN.first.toStdString();

	// 打印导出时间
	info.strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss").toStdString();

	// 测定时间1
	info.strTestTime1 = m_calculateData.time1;

	// 测定时间2
	info.strTestTime2 = m_calculateData.time2;

	// 计算结果
	for (int cupIndex = 1; cupIndex <= m_calculateData.absValue.size(); cupIndex++)
	{
		if (!m_calculateData.absValue.contains(cupIndex))
		{
			continue;
		}

		ChCupTestResult cupTemp;
		// 杯号
		cupTemp.strCupNum = std::to_string(cupIndex);
		// 吸光度
		auto values = m_calculateData.absValue[cupIndex];
		if (values.size() != (DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE - 1))
		{
			continue;
		}
		cupTemp.str340Value = std::to_string((int)values[0]);
		cupTemp.str380Value = std::to_string((int)values[1]);
		cupTemp.str405Value = std::to_string((int)values[2]);
		cupTemp.str450Value = std::to_string((int)values[3]);
		cupTemp.str480Value = std::to_string((int)values[4]);
		cupTemp.str505Value = std::to_string((int)values[5]);
		cupTemp.str546Value = std::to_string((int)values[6]);
		cupTemp.str570Value = std::to_string((int)values[7]);
		cupTemp.str600Value = std::to_string((int)values[8]);
		cupTemp.str660Value = std::to_string((int)values[9]);
		cupTemp.str700Value = std::to_string((int)values[10]);
		cupTemp.str750Value = std::to_string((int)values[11]);
		cupTemp.str800Value = std::to_string((int)values[12]);

		info.vecCupResult.push_back(cupTemp);
	}
}

///
/// @brief 获取导出表格数据
///
/// @param[in]  strList  表格数据
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
void QCupBlankCalcDlg::GetExportExcelData(QStringList& strList)
{
	QStringList strExportTextList;

	// 创建表头
	QString strHeaderInfo("");
	strHeaderInfo += tr("模块") + "\t";
	strHeaderInfo += tr("测定时间1") + "\t";
	strHeaderInfo += tr("测定时间2");

	strExportTextList.push_back(std::move(strHeaderInfo));

	// 数据1
	QString dataInfo = "";

	// 模块名称
	dataInfo += m_devNameOfSN.first + "\t";

	// 测定时间1
	dataInfo += QString::fromStdString(m_calculateData.time1) + "\t";;

	// 测定时间2
	dataInfo += QString::fromStdString(m_calculateData.time2);

	strExportTextList.push_back(dataInfo);

	// 创建表头
	QString strHeaderRes("");
	strHeaderRes += tr("反应杯号") + "\t";
	strHeaderRes += tr("340nm") + "\t";
	strHeaderRes += tr("380nm") + "\t";
	strHeaderRes += tr("405nm") + "\t";
	strHeaderRes += tr("450nm") + "\t";
	strHeaderRes += tr("480nm") + "\t";
	strHeaderRes += tr("505nm") + "\t";
	strHeaderRes += tr("546nm") + "\t";
	strHeaderRes += tr("570nm") + "\t";
	strHeaderRes += tr("600nm") + "\t";
	strHeaderRes += tr("660nm") + "\t";
	strHeaderRes += tr("700nm") + "\t";
	strHeaderRes += tr("750nm") + "\t";
	strHeaderRes += tr("800nm");
	strExportTextList.push_back(std::move(strHeaderRes));

	// 数据
	QString rowData = "";
	QStringList cupResultInfo;
	QMapIterator<int, QVector<int>> iter(m_calculateData.absValue);
	while (iter.hasNext()) {
		iter.next();

		rowData = "";

		// 添加每行数据
		// 杯号
		rowData += QString::number(iter.key()) + "\t";

		// 吸光度
		for (auto val : iter.value())
		{
			rowData += QString::number(val) + "\t";
		}

		// 移除最后一个tab
		if (!rowData.isEmpty()) {
			rowData.remove(rowData.size() - 1, 1);
		}

		cupResultInfo.push_back(rowData);
	}

	// 添加结果信息
	strExportTextList += cupResultInfo;

	strList = std::move(strExportTextList);
}

///
/// @brief 清空数据
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
///
void QCupBlankCalcDlg::ClearData()
{
	m_pStdModel->setRowCount(0);
	m_calculateData.Clear();

}
