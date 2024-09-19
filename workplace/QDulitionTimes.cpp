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

#include "QDulitionTimes.h"
#include "ui_QDulitionTimes.h"
#include <QStandardItemModel>
#include <QStringList>

#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"

QDulitionTimes::QDulitionTimes(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::QDulitionTimes();
	ui->setupUi(this);
	Init();
}

QDulitionTimes::~QDulitionTimes()
{
}

///
/// @brief 设置项目列表
///
/// @param[in]  testItems  项目列表信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QDulitionTimes::SetAssayParamter(std::vector<::tf::TestItem>& testItems)
{
	if (m_itemConfigModel == nullptr)
	{
		return;
	}

	m_dataRow = 0;
	m_itemConfigModel->clear();
	QStringList headString;
	headString << tr("项目") << tr("加样量") << tr("稀释倍数") << tr("次数");
	m_itemConfigModel->setHorizontalHeaderLabels(headString);
	m_itemConfigModel->setRowCount(20);

	std::map<int, QStandardItem*> assayCodeList;
	for (const auto& item : testItems)
	{
		int column = 0;
		// 显示项目名称

		// 获取项目名
		QString strAssayName("");
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(item.assayCode);
		if (Q_NULLPTR == spAssayInfo)
		{
			continue;
		}

		// 名称
		strAssayName = QString::fromStdString(spAssayInfo->assayName);
		m_itemConfigModel->setItem(m_dataRow, column++, new QStandardItem(strAssayName));

		QStandardItem* itemStand = nullptr;
		if (item.dilutionFactor == 1)
		{

			itemStand = new QStandardItem(QString::number(item.suckVolType));

		}
		else
		{
			// 稀释选择项
			itemStand = new QStandardItem(QString::number(3));
		}

		// 稀释选择项
		m_itemConfigModel->setItem(m_dataRow, column++, itemStand);
		ui->assay_paramter_list->openPersistentEditor(itemStand->index());

		// 稀释倍数
		if (item.dilutionFactor == 1)
		{
			itemStand = new QStandardItem(QString::number(0));
		}
		else
		{
			int level = 0;
			boost::bimap<int, QString>::right_const_iterator iter;
			iter = m_dulition.right.find(QString::number(item.dilutionFactor));
			if (iter != m_dulition.right.end())
			{
				level = iter->second;
			}

			itemStand = new QStandardItem(QString::number(level));
		}

		m_itemConfigModel->setItem(m_dataRow, column++, itemStand);
		ui->assay_paramter_list->openPersistentEditor(itemStand->index());

		// 若没有找到对应的项目则，加入--[assaycode--row]
		if (assayCodeList.find(item.assayCode) == assayCodeList.end())
		{
			itemStand = new QStandardItem("1");
			itemStand->setData("1", Qt::EditRole);
			// 测试次数
			m_itemConfigModel->setItem(m_dataRow, column++, itemStand);
			assayCodeList.insert(std::make_pair(item.assayCode, itemStand));
			m_dataRow++;
		}
		else
		{
			// 获取测试次数
			itemStand = assayCodeList[item.assayCode];
			int times = itemStand->data(Qt::EditRole).toString().toInt();
			itemStand->setData(QString::number(times + 1), Qt::EditRole);
		}
		ui->assay_paramter_list->openPersistentEditor(itemStand->index());
	}
}

///
/// @brief 获取参数信息
///
/// @param[out]  testItems  参数信息
///
/// @return true：成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
bool QDulitionTimes::GetAssayparameterInfo(std::vector<::tf::TestItem>& testItems, std::map<int, int>& assayRepeatTimes)
{
	if (m_itemConfigModel == nullptr)
	{
		return false;
	}

	::tf::TestItem itemData;
	// 依次获取项目的值
	for (int i = 0; i< m_dataRow; i++)
	{
		int column = 0;

		// 获取项目编号
		QModelIndex index = m_itemConfigModel->index(i, column++);
		std::string assayName = m_itemConfigModel->data(index).toString().toStdString();
		std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayName);
		if (Q_NULLPTR == spAssayInfo)
		{
			continue;
		}
		// 设置项目编号
		itemData.__set_assayCode(spAssayInfo->assayCode);

		// 样本量
		index = m_itemConfigModel->index(i, column++);
		int volumn = m_itemConfigModel->data(index).toString().toInt();
		if (volumn != 3)
		{
			itemData.__set_suckVolType(static_cast<tf::SuckVolType::type>(volumn));
			itemData.__set_dilutionFactor(1);
			column++;
		}
		else
		{
			// 稀释倍数
			index = m_itemConfigModel->index(i, column++);
			int levelIndex = m_itemConfigModel->data(index, Qt::EditRole).toString().toInt();

			itemData.__set_suckVolType(static_cast<tf::SuckVolType::type>(0));

			QString dilutionFactor = "1";
			boost::bimap<int, QString>::left_const_iterator iter;
			iter = m_dulition.left.find(levelIndex);
			if (iter != m_dulition.left.end())
			{
				dilutionFactor = iter->second;
			}

			itemData.__set_dilutionFactor(dilutionFactor.toInt());
		}

		// 测试次数代表需要测试项目多少次
		index = m_itemConfigModel->index(i, column++);
		int times = m_itemConfigModel->data(index, Qt::EditRole).toString().toInt();
		// 当大于1的时候才设置
		if (times > 1)
		{
			assayRepeatTimes.insert(std::make_pair(spAssayInfo->assayCode, times));
		}

		testItems.push_back(itemData);
	}

	return true;
}

///
/// @brief 确定按钮槽函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QDulitionTimes::OnOkBtnClicked()
{
	emit dulitionTimesSelected();
	close();
}

///
/// @brief 初始化稀释页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
void QDulitionTimes::Init()
{
	m_itemConfigModel = new QStandardItemModel(ui->assay_paramter_list);
	ui->assay_paramter_list->setModel(m_itemConfigModel);
	ui->assay_paramter_list->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->assay_paramter_list->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->assay_paramter_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->assay_paramter_list->verticalHeader()->hide();

	QStringList headString;
	headString << tr("项目") << tr("加样量") << tr("稀释倍数") << tr("次数");
	m_itemConfigModel->setHorizontalHeaderLabels(headString);
	m_itemConfigModel->setRowCount(20);

	// 吸取量
	QStringList valueList;
	valueList << tr("std") << tr("dec") << tr("inc") << tr("dulition");
	ui->assay_paramter_list->setItemDelegateForColumn(1, new ComBoxDelegate(this, valueList));

	// 稀释倍数
	valueList.clear();
	valueList << tr("--")<<tr("3") << tr("10") << tr("20") << tr("50");
	int i = 0;
	for (const auto& value : valueList)
	{
		m_dulition.left.insert(std::make_pair(i++, value));
	}
	ui->assay_paramter_list->setItemDelegateForColumn(2, new ComBoxDelegate(this, valueList));

	// 设置测试次数
	ui->assay_paramter_list->setItemDelegateForColumn(3, new LineEditDelegate(this));

	connect(ui->Cancel_button, &QPushButton::clicked, this, [&]() {this->close(); });
	connect(ui->ok_btn, SIGNAL(clicked()), this, SLOT(OnOkBtnClicked()));
}
