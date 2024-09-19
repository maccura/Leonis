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
/// @file    QPhotoCheckResultDlg.cpp
/// @brief 	 光度计检查结果
///
/// @author  8580/GongZhiQiang
/// @date    2023年8月2日
/// @version 0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li GongZhiQiang，2023年8月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QPhotoCheckResultDlg.h"
#include "ui_QPhotoCheckResultDlg.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "uidcsadapter/uidcsadapter.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/FileExporter.h"
#include "shared/tipdlg.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "datetimefmttool.h"

QPhotoCheckResultDlg::QPhotoCheckResultDlg(QWidget *parent)
	: BaseDlg(parent),
	m_bInit(false)
{
    ui = new Ui::QPhotoCheckResultDlg();
	ui->setupUi(this);

	// 显示前初始化
	InitBeforeShow();
}

QPhotoCheckResultDlg::~QPhotoCheckResultDlg()
{
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QPhotoCheckResultDlg::InitBeforeShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	SetTitleName(tr("光度计检查结果"));

	// 初始化表格控件
	InitTableCtrl();

	// 初始化模块下拉选择框
	InitComboBoxModule();

	// 初始化其他控件
	ui->comboBox_historyExeTime->setEditable(false);
	ui->comboBox_module->setEditable(false);
	ui->lineEdit_user->setEnabled(false);
}

///
/// @brief
///     初始化数据表格控件
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QPhotoCheckResultDlg::InitTableCtrl()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初始化表格 
	m_pCurStdModel = new QStandardItemModel(ui->tableView_currentResult);
	m_pHisStdModel = new QStandardItemModel(ui->tableView_historyResult);
	ui->tableView_currentResult->setModel(m_pCurStdModel);
	ui->tableView_historyResult->setModel(m_pHisStdModel);

	// 隐藏垂直表头
	ui->tableView_currentResult->verticalHeader()->setVisible(false);
	ui->tableView_historyResult->verticalHeader()->setVisible(false);

	// 设置表格选中模式为行选中，不可多选，不可编辑
	ui->tableView_currentResult->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_currentResult->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui->tableView_currentResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView_currentResult->horizontalHeader()->setStretchLastSection(true);

	ui->tableView_historyResult->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_historyResult->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui->tableView_historyResult->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView_historyResult->horizontalHeader()->setStretchLastSection(true);

	// 设置试剂信息表默认行数和列数
	m_pCurStdModel->setRowCount(DEFAULT_ROW_CNT_OF_PHOTO_CHECK_TABLE);
	m_pCurStdModel->setColumnCount(DEFAULT_COL_CNT_OF_PHOTO_CHECK_TABLE);

	m_pHisStdModel->setRowCount(DEFAULT_ROW_CNT_OF_PHOTO_CHECK_TABLE);
	m_pHisStdModel->setColumnCount(DEFAULT_COL_CNT_OF_PHOTO_CHECK_TABLE);

	// 设置表头
	QStringList strHeaderList;
	strHeaderList << tr("波长") << tr("均值") << tr("极差");
	m_pCurStdModel->setHorizontalHeaderLabels(strHeaderList);
	m_pHisStdModel->setHorizontalHeaderLabels(strHeaderList);

	// 显示代理
	ui->tableView_currentResult->setItemDelegateForColumn(COLUNM_ADC, new CReadOnlyDelegate(ui->tableView_currentResult));
	ui->tableView_currentResult->setItemDelegateForColumn(COLUNM_RANGE, new CReadOnlyDelegate(ui->tableView_currentResult));

	ui->tableView_historyResult->setItemDelegateForColumn(COLUNM_ADC, new CReadOnlyDelegate(ui->tableView_historyResult));
	ui->tableView_historyResult->setItemDelegateForColumn(COLUNM_RANGE, new CReadOnlyDelegate(ui->tableView_historyResult));

	// 调整表格
	int tableWidth = ui->tableView_currentResult->width() / DEFAULT_COL_CNT_OF_PHOTO_CHECK_TABLE;
	int tableHeight = ui->tableView_currentResult->height() / (DEFAULT_ROW_CNT_OF_PHOTO_CHECK_TABLE+1);

	for (int colIndex = 0; colIndex < DEFAULT_COL_CNT_OF_PHOTO_CHECK_TABLE; colIndex++)
	{
		ui->tableView_currentResult->setColumnWidth(colIndex, tableWidth);
		ui->tableView_historyResult->setColumnWidth(colIndex, tableWidth);
	}

	for (int rowIndex = 0; rowIndex < DEFAULT_ROW_CNT_OF_PHOTO_CHECK_TABLE; rowIndex++)
	{
		ui->tableView_currentResult->setRowHeight(rowIndex, tableHeight);
		ui->tableView_historyResult->setRowHeight(rowIndex, tableHeight);
	}

}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QPhotoCheckResultDlg::InitAfterShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初始化信号槽连接
	InitConnect();

	// 更新界面
	UpdatePage();
}

///
/// @brief
///     刷新前初始化
///
/// @param[in]  isTimeChanged  是否是测定时间下拉框改变
///
/// @return		true:初始化成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
bool QPhotoCheckResultDlg::InitBeforeUpdate(bool isTimeChanged)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 模块选择改变，则测定时间清空
	if (!isTimeChanged)
	{
		// 当前测定时间
		ui->label_currentTestTime->clear();
		m_pCurStdModel->setRowCount(0);

		// 阻塞信号发出
		ui->comboBox_historyExeTime->blockSignals(true);
		// 设置测定时间下拉框
		ui->comboBox_historyExeTime->clear();
		// 阻塞信号发出解除
		ui->comboBox_historyExeTime->blockSignals(false);
	}

	// 清空当前表格
	m_pHisStdModel->setRowCount(0);

	// 操作人员清空
	ui->lineEdit_user->clear();

	return true;
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QPhotoCheckResultDlg::InitConnect()
{
	// 关闭按钮
	connect(ui->close_btn, &QPushButton::clicked, this, [&] {this->close(); });

	// 导出功能
	connect(ui->btn_export, &QPushButton::clicked, this, &QPhotoCheckResultDlg::OnExportBtnClicked);

	// 打印功能
	connect(ui->btn_print, &QPushButton::clicked, this, &QPhotoCheckResultDlg::OnPrintBtnClicked);

	// 模块下拉框
	connect(ui->comboBox_module, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, [&] {
		UpdatePage();
	});

	// 测定时间下拉框
	connect(ui->comboBox_historyExeTime, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, [&] {
		UpdatePage(true);
	});
}

///
/// @brief
///     更新界面
///
/// @param[in]  isTimeChanged  是否是测定时间下拉框改变
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QPhotoCheckResultDlg::UpdatePage(bool isTimeChanged)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 刷新前初始化
	if (!InitBeforeUpdate(isTimeChanged))
	{
		ULOG(LOG_ERROR, "InitBeforeUpdate Failed !");
		return;
	}

	if (!UpdatePhotoCheckData(isTimeChanged))
	{
		ULOG(LOG_ERROR, "UpdatePhotoCheckData Failed !");
		return;
	}
}

///
/// @brief
///     初始化模块下拉选择框
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QPhotoCheckResultDlg::InitComboBoxModule()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 阻塞信号发出
	ui->comboBox_module->blockSignals(true);

	// 清空下拉框
	ui->comboBox_module->clear();
	m_devNameOfSN.clear();

	// 获取全部生化设备
	QStringList strDevNameList;
	if (!gUiAdapterPtr()->GetGroupDevNameListWithTrack(strDevNameList))
	{
		return;
	}

	// 依次添加具体设备
	for (const auto& strDevName : strDevNameList)
	{

		// 判断是否为设备组
		if (gUiAdapterPtr()->WhetherContainOtherSubDev(strDevName))
		{
			// 获取子设备
			QStringList strSubDevList;
			if (!gUiAdapterPtr()->GetSubDevNameList(strDevName, strSubDevList) || strSubDevList.isEmpty())
			{
				continue;
			}

			// 根据子设备列表创建单选框
			for (const QString& strSubDevName : strSubDevList)
			{
				// 过滤掉非生化设备
				if (gUiAdapterPtr()->GetDeviceClasssify(strSubDevName, strDevName) != DEVICE_CLASSIFY_CHEMISTRY)
				{
					continue;
				}

				// 名称和sn
				QString strNewSubDevName = strDevName + strSubDevName;
				m_devNameOfSN.insert(strNewSubDevName, gUiAdapterPtr()->GetDevSnByName(strSubDevName, strDevName));
				ui->comboBox_module->addItem(strNewSubDevName);
			}
		}
		else
		{
			// 过滤掉非生化设备
			if (gUiAdapterPtr()->GetDeviceClasssify(strDevName) != DEVICE_CLASSIFY_CHEMISTRY)
			{
				continue;
			}

			// 名称和sn
			ui->comboBox_module->addItem(strDevName);
			m_devNameOfSN.insert(strDevName, gUiAdapterPtr()->GetDevSnByName(strDevName));
		}
	}

	// 默认第一个模块
	if (ui->comboBox_module->count() != 0)
	{
		ui->comboBox_module->setCurrentIndex(0);
	}

	// 如果只有一个模块则隐藏
	if (ui->comboBox_module->count() <= 1)
	{
		ui->label_module->hide();
		ui->comboBox_module->hide();
	}

	// 阻塞信号发出解除
	ui->comboBox_module->blockSignals(false);
}


///
/// @brief
///     更新数据
///
/// @param[in]  isTimeChanged  是否是测定时间下拉框改变
///
/// @return		true:更新成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
bool QPhotoCheckResultDlg::UpdatePhotoCheckData(bool isTimeChanged)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询指定设备光度计检查数据
	ch::tf::MaintainItemResultInfoQueryCond miriqc;
	miriqc.__set_deviceSN(m_devNameOfSN[ui->comboBox_module->currentText()].toStdString());
	miriqc.__set_itemType(tf::MaintainItemType::MAINTAIN_ITEM_CH_PHOTOMETER_CHECK);
	// 测定时间筛选改变
	if (isTimeChanged) miriqc.__set_exeTime(ui->comboBox_historyExeTime->itemData(ui->comboBox_historyExeTime->currentIndex()).toString().toStdString());

	ch::tf::MaintainItemResultInfoQueryResp miriqr;
	ch::LogicControlProxy::QueryMaintainItemResultInfo(miriqr, miriqc);
	if (miriqr.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
		miriqr.lstMaintainItemResultInfo.empty())
	{
		ULOG(LOG_ERROR, "QueryMaintainItemResultInfo Failed !");
		return false;
	}

	// 非测定时间筛选改变，则初始化测定时间下拉框
	if (!isTimeChanged)
	{
		// 阻塞信号发出
		ui->comboBox_historyExeTime->blockSignals(true);

		// 设置测定时间下拉框
		ui->comboBox_historyExeTime->clear();
		for (auto resultInfo : miriqr.lstMaintainItemResultInfo)
		{
			ui->comboBox_historyExeTime->addItem(ToCfgFmtDateTime(QString::fromStdString(resultInfo.exeTime)), QString::fromStdString(resultInfo.exeTime));
		}
		// 设置时间为最后一条数据
		ui->comboBox_historyExeTime->setCurrentIndex(miriqr.lstMaintainItemResultInfo.size() - 1);

		// 阻塞信号发出解除
		ui->comboBox_historyExeTime->blockSignals(false);
	}

	// 更新为查找到的最后一条数据
	auto resultInfo = miriqr.lstMaintainItemResultInfo[miriqr.lstMaintainItemResultInfo.size() - 1];
	QVector<PhotoCheckData> selectedResult;
	if (!GetCheckDataFromJson(resultInfo.params, selectedResult))
	{
		ULOG(LOG_ERROR, "MaintainItemResultInfo Parse Error!");
		return false;
	}

	// 非测定时间筛选改变,更新最新数据
	if (!isTimeChanged) {
		if (!UpdateResultTable(m_pCurStdModel, selectedResult))
		{
			ULOG(LOG_ERROR, "Update Current ResultTable  Error!");
			return false;
		}
		ui->label_currentTestTime->setText(ToCfgFmtDateTime(QString::fromStdString(resultInfo.exeTime)));
	}

	// 更新操作人员
	ui->lineEdit_user->setText(QString::fromStdString(resultInfo.user));

	// 更新历史数据
	if (!UpdateResultTable(m_pHisStdModel, selectedResult))
	{
		ULOG(LOG_ERROR, "Update History ResultTable  Error!");
		return false;
	}

	return true;

}

///
/// @brief
///     更新表格
///
/// @param[in]  spStdModel 表格模型（当前数据||历史数据）
/// @param[in]  result	 吸光度数据
///
/// @return		true:更新成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
bool QPhotoCheckResultDlg::UpdateResultTable(QStandardItemModel* const &spStdModel, const QVector<PhotoCheckData> &result)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 参数判断
	if (spStdModel == nullptr || result.isEmpty())
	{
		ULOG(LOG_ERROR, "Params Error!");
		return false;
	}

	// 缓存数据(打印导出使用)
	if (spStdModel == m_pCurStdModel)
	{
		m_currentResult = result;
	}
	else if (spStdModel == m_pHisStdModel)
	{
		m_historyResult = result;
	}

	// 每种波长的吸光度都有，并且按照约定顺序
	QStringList wavelength;
	wavelength << tr(u8"340nm") << tr(u8"380nm") << tr(u8"405nm") << tr(u8"450nm") << tr(u8"480nm") << tr(u8"505nm") << tr(u8"546nm")
		<< tr(u8"570nm") << tr(u8"600nm") << tr(u8"660nm") << tr(u8"700nm") << tr(u8"750nm") << tr(u8"800nm");
	if (result.size() <= 0)
	{
		ULOG(LOG_ERROR, "Wavelength  Error!");
		return false;
	}

	// 清空表格
	spStdModel->setRowCount(0);
	
	auto getStandardItem = [&](QString displayText) {
		QStandardItem* item = new QStandardItem(displayText);
		item->setTextAlignment(Qt::AlignCenter);
		return item;
	};

	// 更新表格
	int rowIndex = 0;
	QList<QStandardItem *> rowItems;
	// 反向获取tableView
	auto tableview =  qobject_cast<QTableView*>(spStdModel->parent());
	for(auto data : result)
	{
		// 如果超出界面显示的数量，则退出
		if (rowIndex >= wavelength.size())
		{
			break;
		}

		rowItems.clear();
		rowItems.append(getStandardItem(wavelength[rowIndex]));

		// ADC值
		auto adcItem = getStandardItem(QString::number(data.adcValue));
		if (!data.adcIsNormal)
		{
			adcItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::ForegroundRole);
			//adcItem->setData(UI_REAGENT_WARNCOLOR, Qt::UserRole + 5);
		}
		rowItems.append(adcItem);

		// 极差值
		auto rangeItem = getStandardItem(QString::number(data.rangeValue));
		if (!data.rangeIsNormal)
		{
			rangeItem->setData(QColor(UI_REAGENT_WARNFONT), Qt::ForegroundRole);
			//rangeItem->setData(UI_REAGENT_WARNCOLOR, Qt::UserRole + 5);
		}
		rowItems.append(rangeItem);

		spStdModel->insertRow(rowIndex, rowItems);
		if (tableview != nullptr)
		{
			tableview->setRowHeight(rowIndex, tableview->height()/(DEFAULT_ROW_CNT_OF_PHOTO_CHECK_TABLE+1));
		}
		rowIndex++;

	}

	return true;
}

///
/// @brief
///     将JSON格式的数据结果转换为vector形式的数据
///
/// @param[in] jsonResult 字符串格式的测定结果
/// @param[in] selectedResult 吸光度数据
///
/// @return		true:转换成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
bool QPhotoCheckResultDlg::GetCheckDataFromJson(const std::string &jsonResult, QVector<PhotoCheckData> &selectedResult)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 判空
	if (jsonResult.empty())
	{
		ULOG(LOG_ERROR, "MaintainItemResultInfo is empty!");
		return false;
	}

	// 清空
	selectedResult.clear();

	// 解析维护项参数
	try
	{
		Document jsonDoc;
		if (!RapidjsonUtil::ParseRapidjson(jsonResult, jsonDoc))
		{
			ULOG(LOG_ERROR, "Failed to parse json string: %s", jsonResult);
			return false;
		}

		const Value& vResultSet = RapidjsonUtil::GetArrayForKey(jsonDoc, "checkResult");
		
		// 索引初始化
		int waveIndex = 0;
		PhotoCheckData pcdTemp;
		for (auto& rt : vResultSet.GetArray())
		{
			// 直接过滤掉保留波长
			if (waveIndex == PHOTOCHECK_WAVE_INDEX::WAVE_RESERVE1_INDEX ||
				waveIndex == PHOTOCHECK_WAVE_INDEX::WAVE_RESERVE2_INDEX ||
				waveIndex == PHOTOCHECK_WAVE_INDEX::WAVE_RESERVE3_INDEX)
			{
				waveIndex++;
				continue;
			}

			const Value& data = RapidjsonUtil::GetArrayForKey(rt, QString::number(waveIndex+1).toUtf8().constData());
			if (data.GetArray().Size() < 4)
			{
				continue;
			}

			pcdTemp.adcValue = data.GetArray()[0].GetInt();
			pcdTemp.adcIsNormal = (data.GetArray()[1].GetInt() == 0 ? false : true);
			pcdTemp.rangeValue = data.GetArray()[2].GetInt();
			pcdTemp.rangeIsNormal = (data.GetArray()[3].GetInt() == 0 ? false : true);

			selectedResult.push_back(pcdTemp);

			waveIndex++;
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
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QPhotoCheckResultDlg::showEvent(QShowEvent *event)
{
	// 让基类处理事件
	BaseDlg::showEvent(event);

	// 第一次显示时初始化
	if (!m_bInit)
	{
		InitAfterShow();
		m_bInit = true;
	}
}

///
/// @brief
///     导出功能
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年01月16日，新建函数
///
void QPhotoCheckResultDlg::OnExportBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

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
		ChPhotoCheckResultInfo Info;
		GetPrintExportInfo(Info);
		std::string strInfo = GetJsonString(Info);
		ULOG(LOG_INFO, "Print datas : %s", strInfo);
		std::string unique_id;
		QString strDirPath = QCoreApplication::applicationDirPath();
		QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportChPhotoCheckResult.lrxml";
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
/// @brief
///     打印功能
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年03月20日，新建函数
///
void QPhotoCheckResultDlg::OnPrintBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	ChPhotoCheckResultInfo Info;
	GetPrintExportInfo(Info);
	std::string strInfo = GetJsonString(Info);
	ULOG(LOG_INFO, "Print datas : %s", strInfo);
	std::string unique_id;
	int irect = printcom::printcom_async_print(strInfo, unique_id);
	ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
}

///
/// @brief 获取打印数据
///
/// @param[in]  info  光度计检查测试记录
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
///
void QPhotoCheckResultDlg::GetPrintExportInfo(ChPhotoCheckResultInfo& info)
{
	// 模块
	info.strModel = ui->comboBox_module->currentText().toStdString();

	// 打印导出时间
	info.strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss").toStdString();

	// 当前测定时间
	info.strCurTestTime = ui->label_currentTestTime->text().toStdString();

	// 历史测定时间
	info.strHisTestTime = ui->comboBox_historyExeTime->currentText().toStdString();

	// 结果转换
	auto ConvertResult = [](std::vector<ChPhotoCheckResult>& outResult,QVector<PhotoCheckData>& inResult) {
		QStringList waveList;
		waveList << tr(u8"340nm") << tr(u8"380nm") << tr(u8"405nm") << tr(u8"450nm") << tr(u8"480nm") << tr(u8"505nm") << tr(u8"546nm")
			<< tr(u8"570nm") << tr(u8"600nm") << tr(u8"660nm") << tr(u8"700nm") << tr(u8"750nm") << tr(u8"800nm");

		// 当前测定结果
		int waveIndex = 0;
		for (auto result : inResult)
		{
			ChPhotoCheckResult curResultTemp;

			// 波长
			if (waveIndex < waveList.size())
			{
				curResultTemp.strWave = waveList[waveIndex].toStdString();
			}

			// 均值
			curResultTemp.strMeanValue = std::to_string((int)result.adcValue);
			if (!result.adcIsNormal)
			{
				curResultTemp.strMeanValue += "*";
			}
			// 极差
			curResultTemp.strRangeValue = std::to_string((int)result.rangeValue);
			if (!result.rangeIsNormal)
			{
				curResultTemp.strRangeValue += "*";
			}

			outResult.push_back(curResultTemp);
			waveIndex++;
		}

	};

	// 当前数据结果
	ConvertResult(info.vecCurResult, m_currentResult);

	// 历史数据结果
	ConvertResult(info.vecHisResult, m_historyResult);
}

///
/// @brief 获取导出表格数据
///
/// @param[in]  strList  表格数据
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月21日，新建函数
///
void QPhotoCheckResultDlg::GetExportExcelData(QStringList& strList)
{
	// 模块
	QString model = tr("模块:") + "\t" + ui->comboBox_module->currentText();
	strList.push_back(std::move(model));

	// 结果转换
	auto ConvertResult = [](QStringList& strList, QVector<PhotoCheckData>& inResult) {
		QString strHeaderRes("\t");
		strHeaderRes += tr("波长") + "\t";
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
		strList.push_back(std::move(strHeaderRes));

		// 当前测定结果
		QString meanValue("\t");
		meanValue += tr("均值") + "\t";

		QString rangeValue("\t");
		rangeValue += tr("极差") + "\t";

		for (auto result : inResult)
		{
			// 均值
			QString mV = QString::number((int)result.adcValue);
			if (!result.adcIsNormal)
			{
				mV += "*";
			}
			meanValue += mV + "\t";

			// 极差
			QString rV = QString::number((int)result.rangeValue);
			if (!result.rangeIsNormal)
			{
				rV += "*";
			}
			rangeValue += rV + "\t";
		}

		strList.push_back(std::move(meanValue));
		strList.push_back(std::move(rangeValue));
	};

	// 本次测定结果
	QString curTestTime = tr("本次结果") + "\t";
	curTestTime += tr("测定时间:") + "\t";
	curTestTime += ui->label_currentTestTime->text();
	strList.push_back(std::move(curTestTime));

	ConvertResult(strList, m_currentResult);
	
	// 历史结果
	QString hisTestTime = tr("历史结果") + "\t";
	hisTestTime += tr("测定时间:") + "\t";
	hisTestTime += ui->comboBox_historyExeTime->currentText();
	strList.push_back(std::move(hisTestTime));

	ConvertResult(strList, m_historyResult);
}
