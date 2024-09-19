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
/// @file     QStatisBaseWgt.h
/// @brief    统计分析ui基类
///
/// @author   7656/zhang.changjiang
/// @date     2023年11月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QStatisBaseWgt.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"

#define ALL_COMBOBOX_OFFSET 1				// 下拉列表“全部”复选框索引偏移量
#define NOR_COMBOBOX_OFFSET 2				// 下拉列表常规复选框索引偏移量
#define POSIX_START_TIME    "1970/01/01"      // POSIX时间起点

QStatisBaseWgt::QStatisBaseWgt(QWidget *parent)
	: QWidget(parent)
	, m_pLowerDateEit(nullptr)
	, m_pUpperDateEit(nullptr)
	, m_pDevModel(nullptr)
	, m_pDevView(nullptr)
	, m_pDevComboBox(nullptr)
	, m_pAssayModel(nullptr)
	, m_pAssayView(nullptr)
	, m_pAssayComboBox(nullptr)
	, m_pReagRadioBtn(nullptr)
	, m_pReagModel(nullptr)
	, m_pReagView(nullptr)
	, m_pReagComboBox(nullptr)
	, m_pQueryBtn(nullptr)
	, m_pResetBtn(nullptr)
	, m_pStatisType(::tf::StatisType::type::STATIS_SAMPLE)
{
	
}

QStatisBaseWgt::~QStatisBaseWgt()
{
}

///
/// @brief  获取选中的设备序列号
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
std::vector<std::string> QStatisBaseWgt::GetVecDeviceSN()
{
	std::vector<std::string> vecDeviceSN;

	if (m_pDevModel == nullptr || m_pDevView == nullptr)
	{
		return vecDeviceSN;
	}

	if (m_pDevModel->rowCount() <= NOR_COMBOBOX_OFFSET)
	{
		return std::move(vecDeviceSN);
	}

	for (int i = 0; i < m_pDevModel->rowCount(); ++i)
	{
		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pDevView->indexWidget(m_pDevModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
		if (checkBox == nullptr)
		{
			continue;
		}

		if (checkBox->isChecked())
		{
			vecDeviceSN.push_back(checkBox->property("deviceSN").toString().toStdString());
		}
	}

	return std::move(vecDeviceSN);
}

///
/// @brief  获取选中的项目名称
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
std::vector<int> QStatisBaseWgt::GetVecAssayCode()
{
	std::vector<int> vecAssayCode;

	if (m_pAssayModel == nullptr || m_pAssayView == nullptr)
	{
		return vecAssayCode;
	}

	if (m_pAssayModel->rowCount() <= NOR_COMBOBOX_OFFSET)
	{
		return std::move(vecAssayCode);
	}

	for (int i = 0; i < m_pAssayModel->rowCount(); ++i)
	{
		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pAssayView->indexWidget(m_pAssayModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
		if (checkBox == nullptr)
		{
			continue;
		}

		if (checkBox->isChecked())
		{
			vecAssayCode.push_back((m_pReagRadioBtn == nullptr || m_pReagRadioBtn->isChecked()) ? checkBox->property("assayCode").toInt() : checkBox->property("subType").toInt());
		}
	}

	return std::move(vecAssayCode);
}

///
/// @brief  获取选中的试剂批号
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
std::vector<std::string> QStatisBaseWgt::GetVecReagentLot()
{
	std::vector<std::string> vecReagentLot;

	if (m_pReagModel == nullptr || m_pReagView == nullptr)
	{
		return vecReagentLot;
	}

	if (m_pReagModel->rowCount() <= NOR_COMBOBOX_OFFSET)
	{
		return std::move(vecReagentLot);
	}

	for (int i = 0; i < m_pReagModel->rowCount(); ++i)
	{
		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pReagView->indexWidget(m_pReagModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
		if (checkBox == nullptr)
		{
			continue;
		}

		if (checkBox->isChecked())
		{
			vecReagentLot.push_back(checkBox->property("reagentLot").toString().toStdString());
		}
	}

	return std::move(vecReagentLot);
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
void QStatisBaseWgt::ConnectSlots()
{
	if (m_pQueryBtn != nullptr)
	{
		connect(m_pQueryBtn, SIGNAL(clicked()), this, SLOT(OnQueryBtnClicked()));
	}

	if (m_pResetBtn != nullptr)
	{
		connect(m_pResetBtn, SIGNAL(clicked()), this, SLOT(OnResetBtnClicked()));
	}
	
	if (m_pLowerDateEit != nullptr)
	{
		connect(m_pLowerDateEit, &QDateEdit::dateChanged, this, &QStatisBaseWgt::DateEditChanged);
	}

	if (m_pUpperDateEit != nullptr)
	{
		connect(m_pUpperDateEit, &QDateEdit::dateChanged, this, &QStatisBaseWgt::DateEditChanged);
	}

	if (m_pDevComboBox != nullptr)
	{
		connect(m_pDevComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated), [=](const QString &text) {
			ComboBoxActivated(m_pDevComboBox);
		});
	}

	if (m_pAssayComboBox != nullptr)
	{
		connect(m_pAssayComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated), [=](const QString &text) {
			ComboBoxActivated(m_pAssayComboBox);
		});
	}

	if (m_pReagComboBox != nullptr)
	{
		connect(m_pReagComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated), [=](const QString &text) {
			ComboBoxActivated(m_pReagComboBox);
		});
	}
}

///
/// @brief  更新模块下拉列表
///
/// @param[in]  bIsDefault  是否是默认模式（true ：默认全部查询并显示，fasle ：更具用户选择的日期查询并显示）
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QStatisBaseWgt::UpdateDevList(const bool bIsDefault/* = false*/)
{
	if (m_pDevModel == nullptr || m_pDevView == nullptr || m_pDevComboBox == nullptr || m_pLowerDateEit == nullptr || m_pUpperDateEit == nullptr)
	{
		return;
	}

	m_pDevComboBox->clear();
	m_pDevModel->clear();
	
	::tf::DeviceSNQueryResp ret;
	::tf::StatisComboBoxQueryCond queryCond;
	queryCond.__set_startTime(bIsDefault ? POSIX_START_TIME : m_pLowerDateEit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00");
	queryCond.__set_endTime(m_pUpperDateEit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59");
	queryCond.__set_statisType(static_cast<::tf::StatisType::type> (m_pStatisType));
	if (!DcsControlProxy::GetInstance()->QueryDeviceSN(ret, queryCond))
	{
		ULOG(LOG_ERROR, "QueryDeviceSN Failed");
		return;
	}

	if (ret.lstDeviceSN.empty())
	{
		ULOG(LOG_INFO, "QueryDeviceSN lstDeviceSN is null");
		return;
	}

	for (int i = 0; i < m_pDevModel->rowCount(); ++i)
	{
		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pDevView->indexWidget(m_pDevModel->index(0, 0)));
		if (checkBox != nullptr)
		{
			disconnect(checkBox, &QCheckBox::stateChanged, this, &QStatisBaseWgt::CheckDevStateChanged);
		}
	}

	for (int i = 0; i < ret.lstDeviceSN.size() + NOR_COMBOBOX_OFFSET; ++i)
	{
		m_pDevModel->insertRow(i, new QStandardItem(""));
	}

	EllipsisCheckBox *checkAllBox;
	checkAllBox = new EllipsisCheckBox(tr("全部"), m_pDevView);
	checkAllBox->setToolTip(tr("全部"));
	checkAllBox->setMouseTracking(true);
	checkAllBox->setProperty("deviceSN", "all_dev");
	m_pDevView->setIndexWidget(m_pDevModel->index(ALL_COMBOBOX_OFFSET, 0), checkAllBox);
	connect(checkAllBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckDevStateChanged);

	for (int i = 0; i < ret.lstDeviceSN.size(); ++i)
	{
		QString deviceName = QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(ret.lstDeviceSN[i].deviceSN));
		EllipsisCheckBox *checkBox = new EllipsisCheckBox(tr("%1").arg(deviceName), m_pDevView);
		checkBox->setToolTip(deviceName);
		checkBox->setMouseTracking(true);
		checkBox->setProperty("deviceSN", QString::fromStdString(ret.lstDeviceSN[i].deviceSN));
		m_pDevView->setIndexWidget(m_pDevModel->index(i + NOR_COMBOBOX_OFFSET, 0), checkBox);
		connect(checkBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckDevStateChanged);
	}

	m_pDevView->setRowHidden(0, true);
	checkAllBox->setChecked(true);
}

///
/// @brief  更新项目名称下拉列表
///
/// @param[in]  bIsDefault  是否是默认模式（true ：默认全部查询并显示，fasle ：更具用户选择的日期查询并显示）
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
///
void QStatisBaseWgt::UpdateAssayItemList(const bool bIsDefault/* = false*/)
{
	if (m_pAssayModel == nullptr || m_pAssayView == nullptr || m_pAssayComboBox == nullptr || m_pLowerDateEit == nullptr || m_pUpperDateEit == nullptr)
	{
		return;
	}

	EllipsisCheckBox *checkAllBox;
	if (m_pReagRadioBtn == nullptr || m_pReagRadioBtn->isChecked())
	{
		m_pAssayComboBox->clear();
		m_pAssayModel->clear();

		::tf::AssayCodeQueryResp ret;
		::tf::StatisComboBoxQueryCond queryCond;
		queryCond.__set_startTime(bIsDefault ? POSIX_START_TIME : m_pLowerDateEit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00");
		queryCond.__set_endTime(m_pUpperDateEit->date().toString("yyyy/MM/dd").toStdString() +" 23:59:59");
		queryCond.__set_statisType(static_cast<::tf::StatisType::type> (m_pStatisType));
		if (!DcsControlProxy::GetInstance()->QueryAssayCode(ret, queryCond))
		{
			ULOG(LOG_ERROR, "QueryAssayCode Failed");
			return;
		}

		if (ret.lstAssayCode.empty())
		{
			ULOG(LOG_INFO, "QueryAssayCode lstAssayCode is null");
			return;
		}

		for (int i = 0; i < m_pAssayModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pAssayView->indexWidget(m_pAssayModel->index(i, 0)));
			if (checkBox != nullptr)
			{
				disconnect(checkBox, &QCheckBox::stateChanged, this, &QStatisBaseWgt::CheckAssayStateChanged);
			}
		}

		for (int i = 0; i < ret.lstAssayCode.size() + NOR_COMBOBOX_OFFSET; ++i)
		{
			m_pAssayModel->insertRow(i, new QStandardItem(""));
		}

		checkAllBox = new EllipsisCheckBox(tr("全部"), m_pAssayView);
		checkAllBox->setProperty("assayCode", -1);
		checkAllBox->setToolTip(tr("全部"));
		checkAllBox->setMouseTracking(true);
		m_pAssayView->setIndexWidget(m_pAssayModel->index(ALL_COMBOBOX_OFFSET, 0), checkAllBox);
		connect(checkAllBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckAssayStateChanged);

		for (int i = 0; i < ret.lstAssayCode.size(); ++i)
		{
			QString itemName = CommonInformationManager::GetInstance()->GetAssayNameByCodeEx(ret.lstAssayCode[i].assayCode);
			EllipsisCheckBox *checkBox = new EllipsisCheckBox(tr("%1").arg(itemName), m_pAssayView);
			checkBox->setProperty("assayCode", ret.lstAssayCode[i].assayCode);
			checkBox->setToolTip(itemName);
			checkBox->setMouseTracking(true);
			m_pAssayView->setIndexWidget(m_pAssayModel->index(i + NOR_COMBOBOX_OFFSET, 0), checkBox);
			connect(checkBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckAssayStateChanged);
		}
	}
	else
	{
		m_pAssayComboBox->clear();
		m_pAssayModel->clear();

		::tf::ConsumablesQueryResp ret;
		::tf::StatisComboBoxQueryCond queryCond;
		queryCond.__set_startTime(bIsDefault ? POSIX_START_TIME : m_pLowerDateEit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00");
		queryCond.__set_endTime(m_pUpperDateEit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59");
		queryCond.__set_statisType(static_cast<::tf::StatisType::type> (m_pStatisType));
		if (!DcsControlProxy::GetInstance()->QueryConsumables(ret, queryCond))
		{
			ULOG(LOG_ERROR, "QueryConsumables Failed");
			return;
		}

		if (ret.lstConsumables.empty())
		{
			ULOG(LOG_ERROR, "QueryConsumables lstConsumables is null");
			return;
		}

		for (int i = 0; i < m_pAssayModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pAssayView->indexWidget(m_pAssayModel->index(i, 0)));
			if (checkBox != nullptr)
			{
				disconnect(checkBox, &QCheckBox::stateChanged, this, &QStatisBaseWgt::CheckAssayStateChanged);
			}
		}
	
		for (int i = 0; i < ret.lstConsumables.size() + NOR_COMBOBOX_OFFSET; ++i)
		{
			m_pAssayModel->insertRow(i, new QStandardItem(""));
		}

		checkAllBox = new EllipsisCheckBox(tr("全部"), m_pAssayView);
		checkAllBox->setProperty("subType", -1);
		checkAllBox->setToolTip(tr("全部"));
		checkAllBox->setMouseTracking(true);
		m_pAssayView->setIndexWidget(m_pAssayModel->index(ALL_COMBOBOX_OFFSET, 0), checkAllBox);
		connect(checkAllBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckAssayStateChanged);

		for (int i = 0; i < ret.lstConsumables.size(); ++i)
		{
			QString itemName = ConvertTfEnumToQString(static_cast<::tf::ConsumablesType::type> (ret.lstConsumables[i].subType));
			EllipsisCheckBox *checkBox = new EllipsisCheckBox(tr("%1").arg(itemName), m_pAssayView);
			checkBox->setToolTip(itemName);
			checkBox->setMouseTracking(true);
			checkBox->setProperty("subType", ret.lstConsumables[i].subType);
			m_pAssayView->setIndexWidget(m_pAssayModel->index(i + NOR_COMBOBOX_OFFSET, 0), checkBox);
			connect(checkBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckAssayStateChanged);
		}
	}

	m_pAssayView->setRowHidden(0, true);
	checkAllBox->setChecked(true);
}

///
/// @brief  更新试剂批号下拉列表
///
/// @param[in]  bIsDefault  是否是默认模式（true ：默认全部查询并显示，fasle ：更具用户选择的日期查询并显示）
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QStatisBaseWgt::UpdateReagentLotList(const bool bIsDefault/* = false*/)
{
	if (m_pReagModel == nullptr || m_pReagView == nullptr || m_pReagComboBox == nullptr || m_pLowerDateEit == nullptr || m_pUpperDateEit == nullptr)
	{
		return;
	}

	m_pReagComboBox->clear();
	m_pReagModel->clear();

	::tf::ReagentLotQueryResp ret;
	::tf::StatisComboBoxQueryCond queryCond;
	queryCond.__set_startTime(bIsDefault ? POSIX_START_TIME : m_pLowerDateEit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00");
	queryCond.__set_endTime(m_pUpperDateEit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59");
	queryCond.__set_statisType(static_cast<::tf::StatisType::type> (m_pStatisType));
	if (!DcsControlProxy::GetInstance()->QueryReagentLot(ret, queryCond))
	{
		ULOG(LOG_ERROR, "QueryReagentLot Failed");
		return;
	}

	if (ret.lstReagentLot.empty())
	{
		ULOG(LOG_INFO, "QueryReagentLot lstReagentLot is null");
		return;
	}

	for (int i = 0; i < m_pReagModel->rowCount(); ++i)
	{
		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pReagView->indexWidget(m_pReagModel->index(i, 0)));
		if (checkBox != nullptr)
		{
			disconnect(checkBox, &QCheckBox::stateChanged, this, &QStatisBaseWgt::CheckLotStateChanged);
		}
	}

	for (int i = 0; i < ret.lstReagentLot.size() + NOR_COMBOBOX_OFFSET; ++i)
	{
		m_pReagModel->insertRow(i, new QStandardItem(""));
	}

	EllipsisCheckBox *checkAllBox;
	checkAllBox = new EllipsisCheckBox(tr("全部"), m_pReagView);
	checkAllBox->setProperty("reagentLot", "all_lot");
	checkAllBox->setToolTip(tr("全部"));
	checkAllBox->setMouseTracking(true);
	m_pReagView->setIndexWidget(m_pReagModel->index(ALL_COMBOBOX_OFFSET, 0), checkAllBox);
	connect(checkAllBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckLotStateChanged);

	for (int i = 0; i < ret.lstReagentLot.size(); ++i)
	{
		EllipsisCheckBox *checkBox = new EllipsisCheckBox(tr("%1").arg(QString::fromStdString(ret.lstReagentLot[i].reagentLot)), m_pReagView);
		checkBox->setToolTip(QString::fromStdString(ret.lstReagentLot[i].reagentLot));
		checkBox->setMouseTracking(true);
		checkBox->setProperty("reagentLot", QString::fromStdString(ret.lstReagentLot[i].reagentLot));
		m_pReagView->setIndexWidget(m_pReagModel->index(i + NOR_COMBOBOX_OFFSET, 0), checkBox);
		connect(checkBox, &EllipsisCheckBox::stateChanged, this, &QStatisBaseWgt::CheckLotStateChanged);
	}

	m_pReagView->setRowHidden(0, true);
	checkAllBox->setChecked(true);
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
void QStatisBaseWgt::OnQueryBtnClicked()
{
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
void QStatisBaseWgt::OnResetBtnClicked()
{
	if (m_pUpperDateEit == nullptr || m_pLowerDateEit == nullptr)
	{
		return;
	}
	QDate date = QDate::currentDate();

	// 先断开时间改变信号连接
	disconnect(m_pLowerDateEit, &QDateEdit::dateChanged, this, &QStatisBaseWgt::DateEditChanged);
	disconnect(m_pUpperDateEit, &QDateEdit::dateChanged, this, &QStatisBaseWgt::DateEditChanged);

	// 设置日期
	m_pUpperDateEit->setDate(date);
	m_pLowerDateEit->setDate(date);

	// 重新连接时间改变的信号
	connect(m_pLowerDateEit, &QDateEdit::dateChanged, this, &QStatisBaseWgt::DateEditChanged);
	connect(m_pUpperDateEit, &QDateEdit::dateChanged, this, &QStatisBaseWgt::DateEditChanged);
}

///
/// @brief  模块复选框
///
/// @param[in]  state  复选框状态
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QStatisBaseWgt::CheckDevStateChanged(int state)
{
	if (m_pDevModel == nullptr || m_pDevView == nullptr || m_pDevComboBox == nullptr)
	{
		ULOG(LOG_ERROR, "m_pDevModel or m_pDevView or m_pDevComboBox is nulptr!");
		return;
	}

	QCheckBox *senderCheckBox = qobject_cast<QCheckBox *> (sender());
	if (senderCheckBox == nullptr)
	{
		ULOG(LOG_ERROR, "Get senderCheckDevBox is nullptr!");
		return;
	}

	if (senderCheckBox->property("deviceSN").toString() == "all_dev")
	{
		for (int i = 0; i < m_pDevModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pDevView->indexWidget(m_pDevModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
			if (checkBox != nullptr)
			{
				checkBox->blockSignals(true);
				checkBox->setChecked(state == Qt::Checked);
				checkBox->blockSignals(false);
			}
		}
		m_pDevModel->item(0, 0)->setText(state == Qt::Checked ? tr("全部") : "");
	}
	else
	{
		senderCheckBox->blockSignals(true);
		senderCheckBox->setChecked(state == Qt::Checked);
		senderCheckBox->blockSignals(false);

		QVector<QString> vecStr;
		bool isCheckedAll = true;
		for (int i = 0; i < m_pDevModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pDevView->indexWidget(m_pDevModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
			if (checkBox == nullptr)
			{
				continue;
			}

			if (!checkBox->isChecked())
			{
				isCheckedAll = false;
				continue;
			}

			vecStr.append(checkBox->text());
		}

		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pDevView->indexWidget(m_pDevModel->index(ALL_COMBOBOX_OFFSET, 0)));
		if (checkBox == nullptr)
		{
			ULOG(LOG_ERROR, "Get checkAllDevBox is nullptr!");
			return;
		}
		checkBox->blockSignals(true);
		checkBox->setChecked(isCheckedAll);
		checkBox->blockSignals(false);

		m_pDevModel->item(0, 0)->setText(isCheckedAll ? tr("全部") : CreateSelectedItemShowInfo(vecStr));
	}
	m_pDevComboBox->setCurrentIndex(0);
}

///
/// @brief  项目名称复选框
///
/// @param[in]  state  复选框状态
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QStatisBaseWgt::CheckAssayStateChanged(int state)
{
	if (m_pAssayModel == nullptr || m_pAssayView == nullptr || m_pAssayComboBox == nullptr)
	{
		ULOG(LOG_ERROR, "m_pAssayModel or m_pAssayView or m_pAssayComboBox is nulptr!");
		return;
	}

	QCheckBox *senderCheckBox = qobject_cast<QCheckBox *> (sender());
	if (senderCheckBox == nullptr)
	{
		ULOG(LOG_ERROR, "Get senderCheckAssayBox is nullptr!");
		return;
	}

	if (senderCheckBox->property("assayCode").toInt() == -1 || senderCheckBox->property("subType").toInt() == -1)
	{
		for (int i = 0; i < m_pAssayModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pAssayView->indexWidget(m_pAssayModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
			if (checkBox != nullptr)
			{
				checkBox->blockSignals(true);
				checkBox->setChecked(state == Qt::Checked);
				checkBox->blockSignals(false);
			}
		}
		m_pAssayModel->item(0, 0)->setText(state == Qt::Checked ? tr("全部") : "");
	}
	else
	{
		senderCheckBox->blockSignals(true);
		senderCheckBox->setChecked(state == Qt::Checked);
		senderCheckBox->blockSignals(false);

		QVector<QString> vecStr;
		bool isCheckedAll = true;
		for (int i = 0; i < m_pAssayModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pAssayView->indexWidget(m_pAssayModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
			if (checkBox == nullptr)
			{
				continue;
			}

			if (!checkBox->isChecked())
			{
				isCheckedAll = false;
				continue;
			}

			vecStr.append(checkBox->text());
		}

		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pAssayView->indexWidget(m_pAssayModel->index(ALL_COMBOBOX_OFFSET, 0)));
		if (checkBox == nullptr)
		{
			ULOG(LOG_ERROR, "Get checkAllAssayBox is nullptr!");
			return;
		}
		checkBox->blockSignals(true);
		checkBox->setChecked(isCheckedAll);
		checkBox->blockSignals(false);

		m_pAssayModel->item(0, 0)->setText(isCheckedAll ? tr("全部") : CreateSelectedItemShowInfo(vecStr));
	}
	m_pAssayComboBox->setCurrentIndex(0);
}

///
/// @brief  试剂批号复选框 
///
/// @param[in]  state  复选框状态
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
///
void QStatisBaseWgt::CheckLotStateChanged(int state)
{
	if (m_pReagModel == nullptr || m_pReagView == nullptr || m_pReagComboBox == nullptr)
	{
		ULOG(LOG_ERROR, "m_pReagModel or m_pReagView or m_pReagComboBox is nulptr!");
		return;
	}

	QCheckBox *senderCheckBox = qobject_cast<QCheckBox *> (sender());
	if (senderCheckBox == nullptr)
	{
		ULOG(LOG_ERROR, "Get senderCheckReagBox is nullptr!");
		return;
	}
	if (senderCheckBox->property("reagentLot").toString() == "all_lot")
	{
		for (int i = 0; i < m_pReagModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pReagView->indexWidget(m_pReagModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
			if (checkBox != nullptr)
			{
				checkBox->blockSignals(true);
				checkBox->setChecked(state == Qt::Checked);
				checkBox->blockSignals(false);
			}
		}
		m_pReagModel->item(0, 0)->setText(state == Qt::Checked ? tr("全部") : "");
	}
	else
	{
		senderCheckBox->blockSignals(true);
		senderCheckBox->setChecked(state == Qt::Checked);
		senderCheckBox->blockSignals(false);

		QVector<QString> vecStr;
		bool isCheckedAll = true;
		for (int i = 0; i < m_pReagModel->rowCount(); ++i)
		{
			QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pReagView->indexWidget(m_pReagModel->index(i + NOR_COMBOBOX_OFFSET, 0)));
			if (checkBox == nullptr)
			{
				continue;
			}

			if (!checkBox->isChecked())
			{
				isCheckedAll = false;
				continue;
			}

			vecStr.append(checkBox->text());
		}

		QCheckBox *checkBox = dynamic_cast<QCheckBox *>(m_pReagView->indexWidget(m_pReagModel->index(ALL_COMBOBOX_OFFSET, 0)));
		if (checkBox == nullptr)
		{
			ULOG(LOG_ERROR, "Get checkAllReagBox is nullptr!");
			return;
		}
		checkBox->blockSignals(true);
		checkBox->setChecked(isCheckedAll);
		checkBox->blockSignals(false);

		m_pReagModel->item(0, 0)->setText(isCheckedAll ? tr("全部") : CreateSelectedItemShowInfo(vecStr));
	}
	m_pReagComboBox->setCurrentIndex(0);
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
void QStatisBaseWgt::DateEditChanged()
{
    //删除模块下拉菜单更新bug32606
	if (m_pReagModel != nullptr && m_pReagComboBox != nullptr)
	{
		m_pReagComboBox->clear();
		m_pReagModel->clear();
	}

	if (m_pAssayModel != nullptr && m_pAssayComboBox != nullptr)
	{
		m_pAssayComboBox->clear();
		m_pAssayModel->clear();
	}

	// 检查控件是否存在
	if (m_pLowerDateEit == nullptr || m_pUpperDateEit == nullptr)
	{
		ULOG(LOG_ERROR, "m_pLowerDateEit or m_pUpperDateEit is nullptr");
		return;
	}

	// 检查起止日期是否合法，起始日期应该小于等于终止日期
	if (m_pLowerDateEit->date() > m_pUpperDateEit->date())
	{
		ULOG(LOG_ERROR, "LowerDateEit > UpperDateEit");
		return;
	}

	// 检查日期间隔是否超过一年
	if (m_pLowerDateEit->date().addYears(1) < m_pUpperDateEit->date())
	{
		ULOG(LOG_ERROR, "Date more than one year!");
		return;
	}
	// 更新项目名称（试剂/耗材名称）
	UpdateAssayItemList();

	// 更新试剂批号
	UpdateReagentLotList();
}

///
/// @brief  下拉列表选中响应槽
///
/// @param[in]  pComboBox  下拉列表
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
///
void QStatisBaseWgt::ComboBoxActivated(QComboBox * pComboBox)
{
	pComboBox->blockSignals(true);
	pComboBox->setCurrentIndex(0);
	pComboBox->blockSignals(false);
}

///
/// @brief  生成复选框选选中的项目显示字符串
///
/// @param[in]  vecStr  选中的项目名称列表
///
/// @return 选中的项目显示字符串
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月24日，新建函数
///
QString QStatisBaseWgt::CreateSelectedItemShowInfo(const QVector<QString>& vecStr)
{
	QString str;
	for (int i = 0; i < vecStr.size(); ++i)
	{
		if (i < vecStr.size() - 1)
		{
			str += vecStr[i] + "、";
		}
		else
		{
			str += vecStr[i];
		}

	}
	return str;
}

///
/// @brief  绘制函数
///
/// @param[in]  event  绘制事件
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
///
void CustomComboBox::paintEvent(QPaintEvent * event)
{
	// 初始化下拉列表绘制风格
	QStyleOptionComboBox opt;
	initStyleOption(&opt);

	// 绘制下拉列表主体
	opt.currentText = fontMetrics().elidedText(currentText(), Qt::ElideLeft, width() - 20);
	QStylePainter painters(this);
	painters.drawComplexControl(QStyle::CC_ComboBox, opt);

	// 绘制选中的文本
	QRect rect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
	QFontMetrics metrics(font());
	QString elidedText = metrics.elidedText(currentText(), Qt::ElideLeft, rect.width());
	QPainter painter(this);
	painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
}

///
/// @brief  鼠标进入事件
///
/// @param[in]  event  事件
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
///
void CustomComboBox::enterEvent(QEvent * event)
{
	QComboBox::enterEvent(event);
	QToolTip::showText(mapToGlobal(rect().center()), currentText());
}

///
/// @brief  鼠标离开事件
///
/// @param[in]  event  事件
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
///
void CustomComboBox::leaveEvent(QEvent * event)
{
	QComboBox::leaveEvent(event);
	QToolTip::hideText();
}

///
/// @brief  绘制函数
///
/// @param[in]  event  绘制事件
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
///
void EllipsisCheckBox::paintEvent(QPaintEvent * event)
{
	// 初始化复选框绘制风格
	QStyleOptionButton option;
	initStyleOption(&option);

	// 绘制复选框主体
	QRect checkBoxRect = style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, this);
	option.rect.setWidth(checkBoxRect.width());
	QPainter painter(this);
	style()->drawControl(QStyle::CE_CheckBox, &option, &painter, this);

	// 绘制文本
	QRect textRect = style()->subElementRect(QStyle::SE_CheckBoxContents, &option, this);
	textRect.setWidth(105);
	textRect.setY(16);
	QFontMetrics metrics(font());
	QString elidedText = metrics.elidedText(text(), Qt::ElideLeft, textRect.width());
	painter.drawText(textRect, Qt::TextWordWrap | Qt::AlignLeft, elidedText);
}
