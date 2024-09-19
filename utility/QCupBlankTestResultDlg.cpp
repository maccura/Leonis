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
/// @file    QCupBlankTestResultDlg.cpp
/// @brief 	 杯空白测定结果
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

#include "QCupBlankTestResultDlg.h"
#include "ui_QCupBlankTestResultDlg.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/FileExporter.h"
#include "shared/tipdlg.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "thrift/ch/ChLogicControlProxy.h"
#include "uidcsadapter/uidcsadapter.h"
#include "datetimefmttool.h"
#include "QCupBlankCalcDlg.h"

QCupBlankTestResultDlg::QCupBlankTestResultDlg(QWidget *parent)
	: BaseDlg(parent),
	m_bInit(false)
{
    ui = new Ui::QCupBlankTestResultDlg();
	ui->setupUi(this);

	// 显示前初始化
	InitBeforeShow();
}

QCupBlankTestResultDlg::~QCupBlankTestResultDlg()
{
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QCupBlankTestResultDlg::InitBeforeShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初始化标题
	SetTitleName(tr(u8"杯空白测定结果"));

	// 初始化表格控件
	InitTableCtrl();

	// 初始化反应盘控件
	InitReactionDisk();

	// 初始化模块下拉选择框
	InitComboBoxModule();

	// 初始化其他控件
	ui->label_abnormalCupIndex->clear();
	ui->label_abnormalCupIndex->setStyleSheet("color:red;");
	ui->comboBox_exeTime->setEditable(false);
	ui->comboBox_module->setEditable(false);

	// 默认显示ADC值
	ui->rBtn_ADC->setChecked(true);
	ui->rBtn_ABS->setChecked(false);

	// 计算功能权限
	auto currentLoginUser = UserInfoManager::GetInstance()->GetLoginUserInfo();
	if (currentLoginUser != nullptr && 
		currentLoginUser->type < tf::UserType::USER_TYPE_ENGINEER)
	{
		ui->btn_calculate->setVisible(false);
	}
}

///
/// @brief
///     初始化数据表格控件
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QCupBlankTestResultDlg::InitTableCtrl()
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
	m_pStdModel->setRowCount(DEFAULT_ROW_CNT_OF_BLANK_RESULT_TABLE);
	m_pStdModel->setColumnCount(DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE);

	// 设置表头
	QStringList strHeaderList;
	strHeaderList << tr(u8"反应杯号") << tr(u8"340nm") << tr(u8"380nm") << tr(u8"405nm") << tr(u8"450nm") << tr(u8"480nm")
		<< tr(u8"505nm") << tr(u8"546nm") << tr(u8"570nm") << tr(u8"600nm") << tr(u8"660nm") << tr(u8"700nm") << tr(u8"750nm") << tr(u8"800nm");
	m_pStdModel->setHorizontalHeaderLabels(strHeaderList);

	for (int colIndex = 0; colIndex < DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE; colIndex++)
	{
		ui->tableView_result->setColumnWidth(colIndex, ui->tableView_result->width() / DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE);
	}

	for (int rowIndex = 0; rowIndex < DEFAULT_ROW_CNT_OF_BLANK_RESULT_TABLE; rowIndex++)
	{
		ui->tableView_result->setRowHeight(rowIndex, ui->tableView_result->height() / (DEFAULT_ROW_CNT_OF_BLANK_RESULT_TABLE+1));
	}
	
}

///
/// @brief
///     初始化反应盘控件
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QCupBlankTestResultDlg::InitReactionDisk()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	
	 // 加载背景图片
	QPixmap pixmap(":/Leonis/resource/image/ch/c1005/bk-reaction-disk.png");
	ui->bk_Disk->setAlignment(Qt::AlignCenter);
	ui->bk_Disk->setPixmap(pixmap.scaled(ui->bk_Disk->size().width()-160, ui->bk_Disk->size().width()-160,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
	
	ui->dev_name_label->setAlignment(Qt::AlignCenter);
	ui->dev_name_label->clear();
	
	
}

///
/// @brief
///     初始化模块下拉选择框
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QCupBlankTestResultDlg::InitComboBoxModule()
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
		ui->dev_name_label->setText(ui->comboBox_module->currentText());
	}

	// 如果只有一个模块则隐藏
	if (ui->comboBox_module->count() <= 1)
	{
		ui->widget_module->hide();
	}

	// 阻塞信号发出解除
	ui->comboBox_module->blockSignals(false);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QCupBlankTestResultDlg::InitAfterShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初始化信号槽连接
	InitConnect();

	// 更新界面
	UpdatePage();
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QCupBlankTestResultDlg::InitConnect()
{
	// 关闭按钮
	connect(ui->close_btn, &QPushButton::clicked, this, [&] {this->close(); });

	// 导出功能
	connect(ui->btn_export, &QPushButton::clicked, this, &QCupBlankTestResultDlg::OnExportBtnClicked);

	// 打印功能
	connect(ui->btn_print, &QPushButton::clicked, this, &QCupBlankTestResultDlg::OnPrintBtnClicked);

	// 计算功能
	connect(ui->btn_calculate, &QPushButton::clicked, this, &QCupBlankTestResultDlg::OnCalculateBtnClicked);

	// 模块下拉框
	connect(ui->comboBox_module, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, [&] {
		UpdatePage();
	});

	// 测定时间下拉框
	connect(ui->comboBox_exeTime, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, [&] {
		UpdatePage(true);
	});

	// 表格选中行改变
	connect(ui->tableView_result->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QCupBlankTestResultDlg::OnCurrentRowChanged);

	// 示值状态改变
	connect(ui->rBtn_ADC, &QRadioButton::toggled, this, [=](bool checked){
		int curIndex = ui->reactionDiskWidget->GetCurrentItemIndex();
		if (curIndex >= 0 && curIndex < REACTION_DISK_NUMBER_OF_UNITS)
		{
			if (!UpdateResultTable(curIndex + 1))
			{
				ULOG(LOG_ERROR, "UpdateResultTable Failed !");
				return;
			}
		}
	});

	// 反应盘选择
	connect(ui->reactionDiskWidget, &ReactionDiskWidget::SelectItemChanged, this, [=](int index) {
		if (index >= 0 && index < REACTION_DISK_NUMBER_OF_UNITS)
		{
			if (!UpdateResultTable(index + 1))
			{
				ULOG(LOG_ERROR, "UpdateResultTable Failed !");
				return;
			}
		}
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
void QCupBlankTestResultDlg::UpdatePage(bool isTimeChanged)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 刷新前初始化
	if (!InitBeforeUpdate(isTimeChanged))
	{
		ULOG(LOG_ERROR, "InitBeforeUpdate Failed !");
		return;
	}

	// 获取最新数据
	if (!UpdateCupBlankTestData(isTimeChanged))
	{
		ULOG(LOG_ERROR, "UpdateCupBlankTestData Failed !");
		return;
	}

	// 更新表格,默认第一联
	if (!UpdateResultTable(1))
	{
		ULOG(LOG_ERROR, "UpdateResultTable Failed !");
		return;
	}

	// 更新反应盘
	if (!UpdateReactionDisk())
	{
		ULOG(LOG_ERROR, "UpdateReactionDisk Failed !");
		return;
	}
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
bool QCupBlankTestResultDlg::InitBeforeUpdate(bool isTimeChanged )
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 模块选择改变，则测定时间清空
	if (!isTimeChanged)
	{
		// 阻塞信号发出
		ui->comboBox_exeTime->blockSignals(true);

		// 设置测定时间下拉框
		ui->comboBox_exeTime->clear();

		// 阻塞信号发出解除
		ui->comboBox_exeTime->blockSignals(false);

	}

	// 初始化表格
	if (m_pStdModel != nullptr)
	{
		m_pStdModel->setRowCount(0);
	}

	// 清空异常显示
	ui->label_abnormalCupIndex->clear();

	// 清空选择数据
	m_selectedResult.clear();

	// 圆盘控件状态初始化
	ui->dev_name_label->setText(ui->comboBox_module->currentText());
	ui->reactionDiskWidget->InitDiskStatus();

	return true;
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
bool QCupBlankTestResultDlg::UpdateCupBlankTestData(bool isTimeChanged)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询指定设备杯空白测定数据
	ch::tf::MaintainItemResultInfoQueryCond miriqc;
	miriqc.__set_deviceSN(m_devNameOfSN[ui->comboBox_module->currentText()].toStdString());
	miriqc.__set_itemType(tf::MaintainItemType::MAINTAIN_ITEM_CH_WATER_BLANK);
	// 测定时间筛选改变
	if (isTimeChanged) miriqc.__set_exeTime(ui->comboBox_exeTime->itemData(ui->comboBox_exeTime->currentIndex()).toString().toStdString());

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
		ui->comboBox_exeTime->blockSignals(true);

		// 设置测定时间下拉框
		ui->comboBox_exeTime->clear();
		for (auto resultInfo : miriqr.lstMaintainItemResultInfo)
		{
			ui->comboBox_exeTime->addItem(ToCfgFmtDateTime(QString::fromStdString(resultInfo.exeTime)), QString::fromStdString(resultInfo.exeTime));
		}
		// 设置时间为最后一条数据
		ui->comboBox_exeTime->setCurrentIndex(miriqr.lstMaintainItemResultInfo.size() - 1);

		// 阻塞信号发出解除
		ui->comboBox_exeTime->blockSignals(false);
	}
	
	// 更新为查找到的最后一条数据
	std::string currentResultInfo = miriqr.lstMaintainItemResultInfo[miriqr.lstMaintainItemResultInfo.size() - 1].params;
	if (!GetTestDataFromJson(currentResultInfo, m_selectedResult))
	{
		ULOG(LOG_ERROR, "MaintainItemResultInfo Parse Error!");
		return false;
	}

	return true;
}

///
/// @brief
///     将JSON格式的数据结果转换为map形式的数据
///
/// @param[in] jsonResult 字符串格式的测定结果
/// @param[in] selectedResult 按照杯号映射的数据结果
///
/// @return		true:转换成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
bool QCupBlankTestResultDlg::GetTestDataFromJson(const std::string &jsonResult, QMap<int, ShowData> &selectedResult)
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

		const Value& vResultSet = RapidjsonUtil::GetArrayForKey(jsonDoc, "waterBlankData");
		// 杯号索引
		int cupNumIndex = 1;
		// 波长索引
		int waveIndex = 0;
		QVector<int> cupAdcTemp;
		QVector<int> cupAbsTemp;
		// 参比ADC值，以第一个杯子为基准
		QMap<int, int> adcRegVal;
		bool isNormal = true;
		for (auto& rt : vResultSet.GetArray())
		{
			// 获取数据
			cupAdcTemp.clear();
			cupAbsTemp.clear();
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

				// 计算吸光度
				if (cupNumIndex == 1)
				{
					// 以第一个杯子作为参比，第一个杯子的吸光度显示为ADC值
					cupAbsTemp.push_back(adcVal);
					adcRegVal.insert(waveIndex, adcVal);
				}
				else
				{
					if (!adcRegVal.contains(waveIndex))
					{
						cupAbsTemp.push_back(0);
					}
					else
					{
						// 计算吸光度
						auto absTemp = adcVal <= 0 ? 0 : ch::CalcSinglePointAbs(adcRegVal[waveIndex], adcVal, CUP_BLANK_OPTICAL_PATH_LENGTH);
						cupAbsTemp.push_back(std::move(absTemp));
					}
				}

				waveIndex++;
			}

			// 获取是否正常
			isNormal = RapidjsonUtil::GetBoolForKey(rt, "isNormal");

			// 拿到目标数据
			selectedResult.insert(cupNumIndex, std::move(ShowData( cupNumIndex, cupAdcTemp, cupAbsTemp, isNormal )));

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
/// @brief
///     更新表格
///
/// @param[in]  unitIndex 反应盘联排编号
///
/// @return		true:更新成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
bool QCupBlankTestResultDlg::UpdateResultTable(int unitIndex)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 参数判断
	if (unitIndex < 1 || unitIndex > REACTION_DISK_NUMBER_OF_UNITS)
	{
		ULOG(LOG_ERROR, "Unit Index Error!");
		return false;
	}

	// 数据结构判断
	if (m_selectedResult.size() != REACTION_DISK_NUMBER_OF_CUPS)
	{
		ULOG(LOG_ERROR, "Cups Number Error!");
		return false;
	}

	// 计算杯号索引
	int startCupIndex = (unitIndex - 1) * REACTION_DISK_NUMBER_OF_UNIT_CUPS + 1;
	int endCupIndex = unitIndex * REACTION_DISK_NUMBER_OF_UNIT_CUPS;

	// 清空表格内容
	m_pStdModel->setRowCount(0);
	
	auto getStandardItem = [&](QString displayText) {
		QStandardItem* item = new QStandardItem(displayText);
		item->setTextAlignment(Qt::AlignCenter);
		return item;
	};

	// 更新表格
	int rowIndex = 0;
	QVector<int> unitAbnormalCups;
	QList<QStandardItem *> rowItems;
	for (int cupIndex = startCupIndex; cupIndex <= endCupIndex; cupIndex++)
	{
		rowItems.clear();
		rowItems.append(getStandardItem(QString::number(cupIndex)));
		

		// 范围判断
		if (!m_selectedResult.contains(cupIndex))
		{
			continue;
		}

		// 显示值,ADC或ABS
		auto diaplayValue = (IsShowADCValue() ? m_selectedResult[cupIndex].adcValue : m_selectedResult[cupIndex].absValue);

		int colIndex = 1;
		for(auto value : diaplayValue)
		{
			// 如果超出界面显示波长的数量，则退出
			if (colIndex > (DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE-1))	break;

			rowItems.append(getStandardItem(QString::number(value)));

			colIndex++;
		}

		m_pStdModel->insertRow(rowIndex, rowItems);
		ui->tableView_result->setRowHeight(rowIndex, ui->tableView_result->height() / (DEFAULT_ROW_CNT_OF_BLANK_RESULT_TABLE+1));
		
		// 记录异常反应杯号
		if (!m_selectedResult[cupIndex].isNormal)
		{
			unitAbnormalCups.append(cupIndex);
		}
		rowIndex++;
	}

	// 默认选中每一联的第一个
	ui->reactionDiskWidget->SetCupIsSelect(startCupIndex);
	ui->tableView_result->selectRow(0);

	// 更新异常反应杯号提示
	UpdateAbNormalCupsNotice(unitAbnormalCups);

	return true;
}

///
/// @brief
///     更新反应盘
///
///
/// @return		true:更新成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
bool QCupBlankTestResultDlg::UpdateReactionDisk()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_selectedResult.isEmpty())
	{
		ULOG(LOG_ERROR, "Data is empty!");
		return false;
	}

	QMapIterator<int, ShowData> cup(m_selectedResult);
	while (cup.hasNext()) {
		cup.next();

		ch::tf::ReactionCupStatus::type cupFlag;
		
		// 颜色正确即可
		cupFlag = cup.value().isNormal ? ch::tf::ReactionCupStatus::CUP_STATUS_ADD_SAMPLE_OR_R1 : 
									   ch::tf::ReactionCupStatus::CUP_STATUS_DISABLE;
		
		// 设置反应盘数据
		ui->reactionDiskWidget->SetCupStatus(cup.key(), cupFlag);
	}

	return true;
}

///
/// @brief
///     更新异常反应杯号显示
///
/// @param[in]  unitAbnormalCups 当前联吸光度异常的杯号
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void QCupBlankTestResultDlg::UpdateAbNormalCupsNotice(const QVector<int> &unitAbnormalCups)
{
	// 清空
	ui->label_abnormalCupIndex->clear();

	QString strIndexs = "";
	for (auto index : unitAbnormalCups)
	{
		strIndexs += QString::number(index);

		if(index!= unitAbnormalCups.last())  strIndexs += ", ";
	}

	// 显示到界面
	if (!strIndexs.isEmpty())
	{
		ui->label_abnormalCupIndex->setText(strIndexs);
	}
	
}


///
/// @brief
///     表格选中行改变
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年08月30日，新建函数
///
void QCupBlankTestResultDlg::OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (m_pStdModel == nullptr )
	{
		ULOG(LOG_ERROR, "m_pStdModel is nullptr!");
		return ;
	}

	auto item = m_pStdModel->item(current.row(), CUPBLANK_DATA_COLUNM::COLUNM_CUP_NUM);
	if (item == nullptr)
	{
		ULOG(LOG_ERROR, "item is nullptr!");
		return;
	}

	int cupNo = item->text().toInt();
	ui->reactionDiskWidget->SetCupIsSelect(cupNo);
}

///
/// @brief
///     导出功能
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年01月16日，新建函数
///
void QCupBlankTestResultDlg::OnExportBtnClicked()
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
		ChCupBlankTestInfo Info;
		GetPrintExportInfo(Info);
		std::string strInfo = GetJsonString(Info);
		ULOG(LOG_INFO, "Print datas : %s", strInfo);
		std::string unique_id;
		QString strDirPath = QCoreApplication::applicationDirPath();
		QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportChCupTestResult.lrxml";
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
void QCupBlankTestResultDlg::OnPrintBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	ChCupBlankTestInfo Info;
	GetPrintExportInfo(Info);
	std::string strInfo = GetJsonString(Info);
	ULOG(LOG_INFO, "Print datas : %s", strInfo);
	std::string unique_id;
	int irect = printcom::printcom_async_print(strInfo, unique_id);
	ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
}

///
/// @brief
///     计算功能
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年03月20日，新建函数
///
void QCupBlankTestResultDlg::OnCalculateBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取当前选中模块信息
	QString devName = ui->comboBox_module->currentText();
	if (devName.isEmpty() || !m_devNameOfSN.contains(devName))
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("未选择需要计算的模块！")));
		pTipDlg->exec();
		return;
	}

	std::shared_ptr<QCupBlankCalcDlg> spQCupBlankCalcDlg(new QCupBlankCalcDlg(this));
	spQCupBlankCalcDlg->SetDevSnInfo({ devName, m_devNameOfSN[devName] });
	spQCupBlankCalcDlg->exec();
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
void QCupBlankTestResultDlg::GetExportExcelData(QStringList& strList)
{
	QStringList strExportTextList;

	// 创建表头
	QString strHeaderInfo("");
	strHeaderInfo += tr("模块") + "\t";
	strHeaderInfo += tr("测定时间") + "\t";
	strHeaderInfo += tr("示值类型") + "\t";
	strHeaderInfo += tr("异常反应杯号");

	strExportTextList.push_back(std::move(strHeaderInfo));

	// 数据1
	QString dataInfo = "";
	// 模块名称
	dataInfo += ui->comboBox_module->currentText() + "\t";

	// 测定时间
	dataInfo += ui->comboBox_exeTime->currentText() + "\t";

	// 示值类型
	bool isAdcType = IsShowADCValue();
	dataInfo += (isAdcType ? "ADC\t" : "ABS\t");
	
	QString errorCupNum = "";
	QString rowData = "";
	QStringList cupResultInfo;

	QMapIterator<int, ShowData> iter(m_selectedResult);
	while (iter.hasNext()) {
		iter.next();

		rowData = "";

		// 异常判断
		if (!iter.value().isNormal)
		{
			errorCupNum.push_back(QString::number(iter.key()) + ",");
		}

		// 添加每行数据
		// 杯号
		rowData += QString::number(iter.key()) + "\t";

		// 遍历添加数据
		auto valueVec = isAdcType ? iter.value().adcValue : iter.value().absValue;
		for (auto val : valueVec)
		{
			rowData += QString::number(val) + "\t";
		}
		if (!rowData.isEmpty()) {
			rowData.remove(rowData.size() - 1, 1);
		}

		cupResultInfo.push_back(rowData);
	}

	// 去除最后一个分隔符
	if (!errorCupNum.isEmpty())
	{
		errorCupNum.remove(errorCupNum.size() - 1, 1);
	}

	dataInfo += errorCupNum;
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

	// 添加结果信息
	strExportTextList += cupResultInfo;

	strList = strExportTextList;
}

///
/// @brief 获取打印数据
///
/// @param[in]  info  杯空白测试记录
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
///
void QCupBlankTestResultDlg::GetPrintExportInfo(ChCupBlankTestInfo& info)
{
	// 模块
	info.strModel = ui->comboBox_module->currentText().toStdString();
	
	// 打印导出时间
	info.strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss").toStdString();

	// 测定时间
	info.strTestTime = ui->comboBox_exeTime->currentText().toStdString();

	// 数据显示类型
	bool isAdcType = IsShowADCValue();
	info.strValueType = isAdcType ? "ADC" : "ABS";

	// 遍历数据
	std::string errorCupNum = "";
	for (int cupIndex = 1; cupIndex <= m_selectedResult.size(); cupIndex++)
	{
		if (!m_selectedResult.contains(cupIndex))
		{
			continue;
		}

		ChCupTestResult cupTemp;
		cupTemp.strCupNum = std::to_string(cupIndex);

		// 记录异常反应杯号
		if (!m_selectedResult[cupIndex].isNormal)
		{
			errorCupNum += (cupTemp.strCupNum + " ");
		}

		auto values = isAdcType ? m_selectedResult[cupIndex].adcValue : m_selectedResult[cupIndex].absValue;
		if (values.size()!= (DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE-1))
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

	// 异常反应杯号
	info.strErrorCupNums = errorCupNum;
}

///
/// @brief 是否显示ADC值
///
///
/// @return true:ADC值，false:ABS值
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
///
bool QCupBlankTestResultDlg::IsShowADCValue()
{
	return ui->rBtn_ADC->isChecked();
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
void QCupBlankTestResultDlg::showEvent(QShowEvent *event)
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
