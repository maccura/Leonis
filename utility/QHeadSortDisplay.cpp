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
/// @file     QDisplayTableSet.cpp
/// @brief    显示设置页面
///
/// @author   5774/WuHongTao
/// @date     2023年2月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QHeadSortDisplay.h"
#include "ui_QHeadSortDisplay.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <QObject>
#include <QPainter>
#include <QStandardItemModel>
#include <QTableView>

#include "shared/tipdlg.h"
#include "shared/QComDelegate.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/msgiddef.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#define MAXNUM 11

QHeadSortDisplay::QHeadSortDisplay(QWidget *parent)
    : QWidget(parent)
    , m_prePareItemModel(Q_NULLPTR)
    , m_selectedModel(Q_NULLPTR)
    , m_selectAllButton(Q_NULLPTR)
    , m_mainTabelModel(Q_NULLPTR)
{
    ui = new Ui::QHeadSortDisplay();
    ui->setupUi(this);
    Init();
}

QHeadSortDisplay::~QHeadSortDisplay()
{
}

QCheckBox* QHeadSortDisplay::GetRowCheckBox(int row)
{
	if (m_prePareItemModel == Q_NULLPTR)
	{
		return Q_NULLPTR;
	}

	auto item = m_prePareItemModel->item(row, 0);
	if (item == Q_NULLPTR)
	{
		return Q_NULLPTR;
	}

	auto widget = qobject_cast<QWidget*>(ui->beSelect_View->indexWidget(item->index()));
	if (widget == Q_NULLPTR)
	{
		return Q_NULLPTR;
	}

	auto checkBoxList = widget->findChildren<QCheckBox*>();
	if (!checkBoxList.isEmpty())
	{
		return checkBoxList[0];
	}

	return Q_NULLPTR;
}

///
/// @brief 设置表类型
///
/// @param[in]  data  设置数据
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QHeadSortDisplay::SetTableData(DisplaySet data)
{
    if (m_prePareItemModel == Q_NULLPTR || m_selectedModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "new QStandardItemModel failed");
        return;
    }

    m_prePareItemModel->clear();
    m_selectedModel->clear();
	m_mainTabelModel->setHorizontalHeaderLabels({ tr("名称")});
	m_prePareItemModel->setHorizontalHeaderLabels({ tr(" "), tr("表头名称") });
    m_selectedModel->setHorizontalHeaderLabels({ "" });

	ui->beSelect_View->setColumnWidth(0, 85);
	ui->beSelect_View->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui->beSelect_View->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	// 保存数据
    m_data = data;
	// 备选数据的数目
    int beSelectNum = 0;
	// 已选数据的数目
    m_selectedNum = 0;

    auto spCom = CommonInformationManager::GetInstance();
    if (spCom == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "Get CommonInformationManager failed");
        return;
    }

	// 依次生成备选项和已选项
    for (auto& itemData : data.displayItems)
    {
		// 若未使能
		if (!itemData.enable)
		{
			continue;
		}

		auto checkbox = GenCheckBoxFromData(ui->beSelect_View, itemData, data.type);
		if (checkbox == Q_NULLPTR)
		{
			continue;
		}

		// 若不允许删除-则不允许操作
		if (!itemData.enDelete)
		{
			checkbox->setEnabled(false);
		}

		// 初始的时候是否选中
        if (itemData.enSelect)
        {
			// 1：选中则checkbox打勾
            checkbox->setChecked(true);
			// 2：将项目放到已选模型中
            auto item = new QStandardItem(GetNameFormType(itemData.type, data.type));
            m_selectedModel->setItem(itemData.postion, 0, item);
			// 3: 更新已选项目数量
            m_selectedNum++;
        }
        else
        {
            checkbox->setChecked(false);
        }

        beSelectNum++;

		// 备选的表头名称
        auto item = new QStandardItem(GetNameFormType(itemData.type, data.type));
        item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
		auto postion = ConvertTypeToPostion(itemData.type, data.type);
        m_prePareItemModel->setItem(postion, 1, item);
    }

	if (m_selectedNum < 0)
	{
		m_selectedNum = 0;
	}

	// 更新备选和已选的项目数量
    ui->selected_label->setText(QString(tr("已选（%1）")).arg(m_selectedNum));
    ui->beSelect_label->setText(QString(tr("备选（%1）")).arg(beSelectNum));

	// 全选(modify by wuht for bug3473)
	if (IsSelectAll(m_selectedNum))
	{
		m_selectAllButton->setChecked(true);
	}
	else
	{
		m_selectAllButton->setChecked(false);
	}

    UpdateBtnState();
}

void QHeadSortDisplay::showEvent(QShowEvent *event)
{
    std::vector<DisplaySet> data;
    if (!DictionaryQueryManager::GetUiDispalySet(data))
    {
        ULOG(LOG_ERROR, "Failed to DisplaySet.");
        return;
    }

    int postion = 0;
    for (auto& dataItem : data)
    {
        if (dataItem.type == PATIENTTYPE)
        {
            continue;
        }

        QString titleName = "";
        switch (dataItem.type)
        {
            // 数据浏览样本列表
            case SAMPLETYPE:
                titleName = tr("样本列表");
                break;
                // 数据浏览项目列表
            case TESTITEMTYPE:
                titleName = tr("结果列表");
                break;
                // 试剂信息列表
            case REAGENTTYPE:
                titleName = tr("试剂信息列表");
                break;
            default:
                break;                
        }

        m_dataMap[dataItem.type] = dataItem;
        auto item = new QStandardItem(titleName);
        item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        m_mainTabelModel->setItem(postion++, 0, item);
    }

	// 初始化第一个选中
	if (!m_dataMap.empty())
	{
		SetTableData(m_dataMap.begin()->second);
		ui->main_head_view->setCurrentIndex(m_mainTabelModel->index(0, 0));
	}
}

bool QHeadSortDisplay::IsModified()
{
	std::vector<DisplaySet> tmpSetVec;
	if (!DictionaryQueryManager::GetUiDispalySet(tmpSetVec))
	{
		ULOG(LOG_ERROR, "Failed to get pageset.");
		return false;
	}

	bool isFind = false;
	DisplaySet tmpData;
	for (auto& data : tmpSetVec)
	{
		if (data.type == m_data.type)
		{
			tmpData = data;

			if (tmpData.displayItems.size() == m_data.displayItems.size())
			{
				isFind = true;
			}
			break;
		}
	}

	if (!isFind)
	{
		return false;
	}

	int rows = m_selectedModel->rowCount();
	int index = 0;
	while (index < rows)
	{
		auto item = m_selectedModel->item(index, 0);
		if (item == Q_NULLPTR)
		{
			index++;
			continue;
		}

		AdjustItemPostion(item->text(), index);
		index++;
	}

	/*
	int j = 0;
	std::vector<DisplaySetItem> newSortItems;
	for (const auto& item : m_data.displayItems)
	{
		DisplaySetItem tmpItem = item;
		tmpItem.postion = j++;
		newSortItems.push_back(tmpItem);
	}

	j = 0;
	std::vector<DisplaySetItem> newSortItems1;
	for (const auto& item : tmpData.displayItems)
	{
		DisplaySetItem tmpItem = item;
		tmpItem.postion = j++;
		newSortItems1.push_back(tmpItem);
	}*/

	int i = 0;
	for (const auto& item : m_data.displayItems)
	{
		auto tmp = tmpData.displayItems[i++];
		// 若都被屏蔽了，那么无论其他项数据是否相等，都视为同一状态
		if (!item.enable && !tmp.enable)
		{
			continue;
		}

		if (item == tmp)
		{
			continue;
		}

		// 若都未被选中，那么无需考虑其他
		if (!item.enSelect && !tmp.enSelect)
		{
			continue;
		}

		return true;
	}

	return false;
}

///
/// @brief 复选框的槽函数（点击复选框）
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QHeadSortDisplay::OnCheckBoxClicked()
{
    // 如不能获取对应的控件，则返回
    auto sendSrc = qobject_cast<QCheckBox*>(sender());
    if (sendSrc == Q_NULLPTR)
    {
        return;
    }

    int type = sendSrc->property("type").toInt();
    // 查找对应的数据
    auto findIter = std::find_if(m_data.displayItems.begin(), m_data.displayItems.end(), [&type](const auto& element)
    {
        return element.type == type;
    });

    // 找不到数据，则返回
    if (findIter == m_data.displayItems.end())
    {
        return;
    }

	// 获取项目名称
	QString name = GetNameFormType(findIter->type, m_data.type);
	// 选中
    if (sendSrc->isChecked())
    {
        auto items = m_selectedModel->findItems(name);
		// 若未查到，说明是新增，若由则不必做任何动作
        if (items.empty())
        {
            m_selectedNum++;
            auto item = new QStandardItem(name);
            item->setData(Qt::AlignVCenter, Qt::TextAlignmentRole);
            m_selectedModel->appendRow(item);
            // 使能
            findIter->enSelect = true;
            // 放到最后一位
            findIter->postion = m_selectedModel->rowCount();
        }
    }
    else
    {
        auto items = m_selectedModel->findItems(name);
        for (auto item : items)
        {
            m_selectedNum--;
            m_selectedModel->takeRow(item->row());
        }

        // 取消选中
        findIter->enSelect = false;

        // 依次缩进-提高位置
        for (auto& dataItem : m_data.displayItems)
        {
            if (dataItem.postion > findIter->postion)
            {
                dataItem.postion--;
            }
        }
    }

    if (m_selectedNum >= MAXNUM)
    {
        ui->clearAll_Button->move(1470, 18);
    }
    else
    {
        ui->clearAll_Button->move(1470, 18);
    }

	if (m_selectedNum < 0)
	{
		m_selectedNum = 0;
	}
    ui->selected_label->setText(QString(tr("已选(%1)")).arg(m_selectedNum));

    // 全选(modify by wuht for bug3473)
    if (IsSelectAll(m_selectedNum))
    {
        m_selectAllButton->setChecked(true);
    }
	else
	{
		m_selectAllButton->setChecked(false);
	}
}

///
/// @brief 保存数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月6日，新建函数
///
void QHeadSortDisplay::OnSaveData()
{
    std::vector<DisplaySet> savedata;
    if (!DictionaryQueryManager::GetUiDispalySet(savedata))
    {
        ULOG(LOG_ERROR, "Failed to get DisplaySet.");
        return;
    }

    int rows = m_selectedModel->rowCount();
    int index = 0;
    while (index < rows)
    {
        auto item = m_selectedModel->item(index, 0);
        if (item == Q_NULLPTR)
        {
            index++;
            continue;
        }

        AdjustItemPostion(item->text(), index);
        index++;
    }

    // 更新数据
    for (auto& data : savedata)
    {
        if (data.type == m_data.type)
        {
            data = m_data;
        }
    }

    if (!DictionaryQueryManager::SaveUiDisplaySet(savedata))
    {
        ULOG(LOG_ERROR, "Failed to save displayset.");
    }
    else
    {
		// 更新数据
		for (auto& data : savedata)
		{
			auto iter = m_dataMap.find(data.type);
			if (iter == m_dataMap.end())
			{
				continue;
			}

			iter->second = data;
		}

        POST_MESSAGE(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE);
		TipDlg(tr("保存成功")).exec();
    }
}

void QHeadSortDisplay::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    bool bOperPermit = UserInfoManager::GetInstance()->IsPermisson(PSM_DISPLAYSET_TABLEHEADERORDER);
    bool bDisable = !bOperPermit;

    ui->beSelect_View->setDisabled(bDisable);
    ui->selected_View->setDisabled(bDisable);

    ui->clearAll_Button->setDisabled(bDisable);
    ui->top_Button->setDisabled(bDisable);
    ui->pre_Button->setDisabled(bDisable);
    ui->next_Button->setDisabled(bDisable);
    ui->bottom_Button->setDisabled(bDisable);
    ui->reset_btn->setDisabled(bDisable);
    ui->save_Button->setDisabled(bDisable);
}

///
/// @brief 初始化函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QHeadSortDisplay::Init()
{
    m_prePareItemModel = new QStandardItemModel(this);
    m_selectedModel = new QStandardItemModel(this);
    m_mainTabelModel = new QStandardItemModel(this);

    if (m_prePareItemModel == Q_NULLPTR || m_selectedModel == Q_NULLPTR
        || m_mainTabelModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "new QStandardItemModel failed");
        return;
    }

    ui->main_head_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->main_head_view->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->main_head_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->main_head_view->verticalHeader()->setVisible(false);

    ui->beSelect_View->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->beSelect_View->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->beSelect_View->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->beSelect_View->verticalHeader()->setVisible(false);

    ui->selected_View->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->selected_View->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->selected_View->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->selected_View->verticalHeader()->setVisible(false);

    ui->main_head_view->setModel(m_mainTabelModel);
    ui->beSelect_View->setModel(m_prePareItemModel);
    ui->selected_View->setModel(m_selectedModel);

    auto header = ui->beSelect_View->horizontalHeader();
    m_selectAllButton = new QCheckBox(header);
    m_selectAllButton->move(m_selectAllButton->x() + 10, m_selectAllButton->y());

    connect(m_selectAllButton, &QCheckBox::clicked, this, [&]() {
        auto spCheckBox = qobject_cast<QCheckBox*>(sender());
        if (spCheckBox == Q_NULLPTR)
        {
            return;
        }
        // 全选
        SetCheckBoxStatus(spCheckBox->isChecked());

		// modify bug 3469 by wuht
		UpdateBtnState();
    });

    // 保存
    connect(ui->save_Button, &QPushButton::clicked, this, [&](){
        OnSaveData();
        emit this->closeDialog();
    });

    // 清空全部
    connect(ui->clearAll_Button, &QPushButton::clicked, this, [&]() {
        int rowIndex = 0;
        int rowCount = m_prePareItemModel->rowCount();
        while (rowCount > 0)
        {
            auto item = m_prePareItemModel->item(rowIndex, 0);
			if (item == Q_NULLPTR)
			{
				rowIndex++;
				rowCount--;
				continue;
			}

			auto widget = qobject_cast<QWidget*>(ui->beSelect_View->indexWidget(item->index()));
			if (widget == Q_NULLPTR)
			{
				rowIndex++;
				rowCount--;
				continue;
			}

			auto checkBoxList = widget->findChildren<QCheckBox*>();
			for (auto& checkBox : checkBoxList)
			{
				bool enDelete = checkBox->property("enDelete").toBool();
				if (!enDelete)
				{
					continue;
				}

				if (checkBox != Q_NULLPTR)
				{
					checkBox->setChecked(false);
					emit checkBox->clicked(false);
				}
			}

            rowIndex++;
            rowCount--;
        }

		// modify bug 3469 by wuht
		UpdateBtnState();
    });

    // 向上
    connect(ui->pre_Button, &QPushButton::clicked, this, [&]() {
        auto currentIndex = ui->selected_View->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

		for (auto item : items)
		{
			if (item == Q_NULLPTR)
			{
				return;
			}
		}

        auto currentPos = currentIndex.row() - 1;
        m_selectedModel->insertRow(currentPos, items);
        ui->selected_View->setCurrentIndex(m_selectedModel->index(currentPos, 0));
        //AdjustItemPostion(items[0]->text(), currentPos);
        UpdateBtnState();
    });

    // 向下
    connect(ui->next_Button, &QPushButton::clicked, this, [&]() {
        auto currentIndex = ui->selected_View->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

		for (auto item : items)
		{
			if (item == Q_NULLPTR)
			{
				return;
			}
		}

        auto currentPos = currentIndex.row() + 1;
        m_selectedModel->insertRow(currentIndex.row() + 1, items);
        ui->selected_View->setCurrentIndex(m_selectedModel->index(currentIndex.row() + 1, 0));
        //AdjustItemPostion(items[0]->text(), currentPos);
        UpdateBtnState();
    });

    // 置顶
    connect(ui->top_Button, &QPushButton::clicked, this, [&]() {
        auto currentIndex = ui->selected_View->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

		for (auto item : items)
		{
			if (item == Q_NULLPTR)
			{
				return;
			}
		}

        m_selectedModel->insertRow(0, items);
        ui->selected_View->setCurrentIndex(m_selectedModel->index(0, 0));
        //AdjustItemPostion(items[0]->text(), 0);
        UpdateBtnState();
    });

    // 置底
    connect(ui->bottom_Button, &QPushButton::clicked, this, [&]() {
        auto currentIndex = ui->selected_View->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

		for (auto item : items)
		{
			if (item == Q_NULLPTR)
			{
				return;
			}
		}

        m_selectedModel->insertRow(m_selectedModel->rowCount(), items);
		auto postion = m_selectedModel->rowCount() - 1;
        ui->selected_View->setCurrentIndex(m_selectedModel->index(postion, 0));
        //AdjustItemPostion(items[0]->text(), postion);
        UpdateBtnState();
    });

	// 重置
	connect(ui->reset_btn, &QPushButton::clicked, this, [&]() {
		auto current = ui->main_head_view->currentIndex();
		if (!current.isValid())
		{
			return;
		}

		int row = current.row();
		auto iter = m_dataMap.find(DisplayType(row));
		if (iter == m_dataMap.end())
		{
			return;
		}

		// 重置数据
		ResetData();
		// 回写数据
		iter->second = m_data;
		// 显示数据
		SetTableData(iter->second);
	});

    // 点击更新当前按钮状态
    connect(ui->selected_View, &QTableView::clicked, this, [&](const QModelIndex& current) {
        UpdateBtnState();
    });

    auto deleGate = new QSelectNewDelegate(this);
    ui->selected_View->setItemDelegateForColumn(0, deleGate);
    connect(deleGate, &QSelectNewDelegate::closeDelegate, this, [&](QString text) {
		// 备选的item
		auto itemBeSelects = m_prePareItemModel->findItems(text, Qt::MatchExactly, 1);
		if (itemBeSelects.empty())
		{
			return;
		}

		// 正常情况下应该只有一个item,所以取第一个
		auto objectItem = itemBeSelects[0];
		auto checkBox = GetRowCheckBox(objectItem->row());
		// checkBox为空，直接返回
		if (checkBox == Q_NULLPTR )
		{
			return;
		}

		// 若不允许删除也直接返回
		if (!checkBox->property("enDelete").toBool())
		{
			return;
		}

        auto items = m_selectedModel->findItems(text);
        for (auto item : items)
        {
			if (m_selectedNum > 0)
			{
				m_selectedNum--;
			}

            m_selectedModel->takeRow(item->row());
        }

		if (m_selectedNum < 0)
		{
			m_selectedNum = 0;
		}
        ui->selected_label->setText(QString(tr("已选(%1)")).arg(m_selectedNum));
		if (IsSelectAll(m_selectedNum))
		{
			m_selectAllButton->setChecked(true);
		}
		else
		{
			m_selectAllButton->setChecked(false);
		}

        UpdateBtnState();

        for (auto& beItemSelect : itemBeSelects)
        {
            auto indexCheckBox = m_prePareItemModel->index(beItemSelect->index().row(), 0);
            auto widget = qobject_cast<QWidget*>(ui->beSelect_View->indexWidget(indexCheckBox));
            if (widget != Q_NULLPTR)
            {
                auto checkBoxList = widget->findChildren<QCheckBox*>();
                for (auto& checkBox : checkBoxList)
                {
                    if (checkBox != Q_NULLPTR)
                    {
						if (checkBox != Q_NULLPTR && !(checkBox->property("enDelete").toBool()))
						{
							continue;
						}

                        checkBox->setChecked(false);
						int type = checkBox->property("type").toInt();
						// 查找对应的数据
						auto findIter = std::find_if(m_data.displayItems.begin(), m_data.displayItems.end(), [&type](const auto& element)
						{
							return element.type == type;
						});

						// 找不到数据，则返回
						if (findIter == m_data.displayItems.end())
						{
							continue;
						}

						findIter->enSelect = false;
                    }
                }
            }
        }
    });

    connect(ui->main_head_view, &QTableView::clicked, this, [&](const QModelIndex& current) {
        if (!current.isValid())
        {
            return;
        }

        int row = current.row();
        auto iter = m_dataMap.find(DisplayType(row));
        if (iter == m_dataMap.end())
        {
            return;
        }

        SetTableData(iter->second);
    });

	// modify bug0012334 by wuht(需求完善)
	connect(ui->main_head_view->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex& current, const QModelIndex& pre)
	{
		if (!current.isValid())
		{
			return;
		}

		int row = current.row();
		auto iter = m_dataMap.find(DisplayType(row));
		if (iter == m_dataMap.end())
		{
			return;
		}

		SetTableData(iter->second);
	});

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
/// @brief 是否全部选中
///
/// @param[in]  selected  已经选中的条目数目
///
/// @return true:全部选中
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月16日，新建函数
///
bool QHeadSortDisplay::IsSelectAll(int selected)
{
	if (selected == 0)
	{
		return false;
	}

	// 跟使能的样本数目相同，则表明已经全选了
	int enableNumber = 0;
	for (const auto& data : m_data.displayItems)
	{
		if (data.enable)
		{
			enableNumber++;
		}
	}

	if (selected == enableNumber)
	{
		return true;
	}

	return false;
}


///
/// @brief 根据item类型和项目类型获取列表中的位置
///
/// @param[in]  type		项目类型
/// @param[in]  dataType	item类型
///
/// @return 位置
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月16日，新建函数
///
int QHeadSortDisplay::ConvertTypeToPostion(int type, DisplayType dataType)
{
	int offset = 0;
	for (const auto& dataItme : m_data.displayItems)
	{
		if (!dataItme.enable && dataItme.type < type)
		{
			offset++;
		}
	}

	return type - offset;
}

///
/// @brief 通过类型获取项目名称
///
/// @param[in]  type		项目类型
/// @param[in]  dataType    item类型
///
/// @return 项目名称
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月16日，新建函数
///
QString QHeadSortDisplay::GetNameFormType(int type, DisplayType dataType)
{
	QString name = "";
	auto spCom = CommonInformationManager::GetInstance();
	if (spCom == Q_NULLPTR)
	{
		ULOG(LOG_ERROR, "%s(Get name failed ,type: %d, dataTyPe: %d)", __FUNCTION__, type, dataType);
		return name;
	}

	if (dataType == REAGENTTYPE)
	{
		name = spCom->GetHeaderNameOfRgntTbl(type + 1);
	}
	else if(dataType == SAMPLETYPE)
	{
		name = spCom->GetNameOfSampleOrAssay(type);
	}
	else if (dataType == TESTITEMTYPE)
	{
		name = spCom->GetNameOfSampleOrAssay(type + 9);
	}

	return name;
}

///
/// @brief 根据数据生成对应的checkBox
///
/// @param[in]  tableView	目标tableView
/// @param[in]  dataItem    数据
/// @param[in]  dataType    item类型
///
/// @return 生成的checkBox
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月16日，新建函数
///
QCheckBox* QHeadSortDisplay::GenCheckBoxFromData(QTableView* tableView, DisplaySetItem dataItem, DisplayType dataType)
{
	if (tableView == Q_NULLPTR)
	{
		return Q_NULLPTR;
	}

	auto model = qobject_cast<QStandardItemModel*>(tableView->model());
	if (model == Q_NULLPTR)
	{
		return Q_NULLPTR;
	}

	auto checkBoxItem = new QStandardItem("");
	auto postion = ConvertTypeToPostion(dataItem.type, dataType);
	model->setItem(postion, 0, checkBoxItem);

	QWidget *widget = new QWidget(tableView);
	QCheckBox *checkbox = new QCheckBox(tableView);
	// 是否允许删除
	checkbox->setProperty("enDelete", dataItem.enDelete);
	// 用于查找
	checkbox->setProperty("type", dataItem.type);

	QHBoxLayout *hLayout = new QHBoxLayout();
	hLayout->addWidget(checkbox);
	hLayout->setAlignment(checkbox, Qt::AlignCenter);
	hLayout->setMargin(0);
	widget->setLayout(hLayout);
	tableView->setIndexWidget(model->index(postion, 0), widget);

	connect(checkbox, SIGNAL(clicked()), this, SLOT(OnCheckBoxClicked()));

	return checkbox;
}

///
/// @brief 更新按钮状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QHeadSortDisplay::UpdateBtnState()
{
    if (m_selectedNum >= MAXNUM)
    {
        ui->clearAll_Button->move(1470, 18);
    }
    else
    {
        ui->clearAll_Button->move(1470, 18);
    }

    ui->next_Button->setEnabled(false);
    ui->pre_Button->setEnabled(false);
    ui->bottom_Button->setEnabled(false);
    ui->top_Button->setEnabled(false);

    int rowCount = m_selectedModel->rowCount();
    auto currentIndex = ui->selected_View->currentIndex();
    if (!currentIndex.isValid() || rowCount <= 1)
    {
        return;
    }

    int currentRow = currentIndex.row();

    // 第一行
    if (currentRow == 0)
    {
        ui->bottom_Button->setEnabled(true);
        ui->next_Button->setEnabled(true);
    }
    // 最后一行
    else if (currentRow == (rowCount - 1))
    {
        ui->top_Button->setEnabled(true);
        ui->pre_Button->setEnabled(true);
    }
    else
    {
        ui->next_Button->setEnabled(true);
        ui->pre_Button->setEnabled(true);
        ui->bottom_Button->setEnabled(true);
        ui->top_Button->setEnabled(true);
    }
}

///
/// @brief 设置全选中与否
///
/// @param[in]  enable  true:全选 false:全不选
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月15日，新建函数
///
void QHeadSortDisplay::SetCheckBoxStatus(bool enable)
{
    auto spCom = CommonInformationManager::GetInstance();
    if (spCom == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "Get CommonInformationManager failed");
        return;
    }

    int rowIndex = 0;
    m_selectedModel->setHorizontalHeaderLabels({ "" });
    int rowCount = m_prePareItemModel->rowCount();
    while (rowCount > 0)
    {
        auto item = m_prePareItemModel->item(rowIndex, 0);
        if (item != Q_NULLPTR)
        {
            auto widget = qobject_cast<QWidget*>(ui->beSelect_View->indexWidget(item->index()));
            if (widget != Q_NULLPTR)
            {
                auto checkBoxList = widget->findChildren<QCheckBox*>();
                for (auto& checkbox : checkBoxList)
                {
					// 若不允许删除，则直接略过
					if (checkbox != Q_NULLPTR && !(checkbox->property("enDelete").toBool()))
					{
						continue;
					}

                    checkbox->setChecked(enable);
					emit checkbox->clicked();
                }
            }
        }

        rowIndex++;
        rowCount--;
    }
}

///
/// @brief 调整项目条目的位置信息
///
/// @param[in]  name        项目名称
/// @param[in]  postion     位置信息  
///
/// @return true表示调整成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月6日，新建函数
///
bool QHeadSortDisplay::AdjustItemPostion(QString name, int postion)
{
    auto spCom = CommonInformationManager::GetInstance();
    if (spCom == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "Get CommonInformationManager failed");
        return false;
    }

    for (auto & data : m_data.displayItems)
    {
		QString typeName = GetNameFormType(data.type, m_data.type);
		if (typeName == name)
        {
            data.postion = postion;
            return true;
        }
    }

    return false;
}

///
/// @brief 重置到最初状态（非上一次保存的状态）
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月23日，新建函数
///
void QHeadSortDisplay::ResetData()
{
	// 样本
	DisplaySetItem childData;
	if(m_data.type == SAMPLETYPE)
	{
		DisplaySet ds;
		int index = 0;
		int type = 0;

		//childData.name = u8"";(check box，没有字段名称)
		childData.postion = 0;
		childData.type = type++;
		childData.enDelete = false;
		childData.enSelect = true;
		childData.width = 45;
		childData.enable = false;
		ds.displayItems.push_back(childData);

		//childData.name = u8"状态";
		childData.postion = 1;
		childData.type = type++;
		childData.width = 63;
		childData.enDelete = false;
		childData.enSelect = true;
		childData.enable = false;
		ds.displayItems.push_back(childData);

		//childData.name = u8"样本号";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 110;
		childData.enDelete = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"样本条码";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 200;
		childData.enDelete = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"样本类型";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 110;
		childData.enDelete = true;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"架/位置";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 100;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"检测完成时间";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 185;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"复查完成时间";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 185;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"审核";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enSelect = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"打印";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enSelect = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		ds.type = SAMPLETYPE;
		m_data = ds;
	}
	else if (m_data.type == TESTITEMTYPE)
	{
		DisplaySet ds;
		int index = 0;
		int type = 0;

		//childData.name = u8"";(check box，没有字段名称)
		childData.postion = 0;
		childData.type = type++;
		childData.enDelete = false;
		childData.width = 45;
		childData.enSelect = true;
		childData.enable = false;
		ds.displayItems.push_back(childData);

		//childData.name = u8"项目名称";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 200;
		childData.enDelete = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"检测结果";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enDelete = true;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"结果状态";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 110;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"复查结果";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"结果状态";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 120;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"单位";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"模块";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"检测信号值";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enSelect = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"复查信号值";
		childData.postion = index++;
		childData.type = type++;
		childData.width = -1;
		childData.enSelect = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		ds.type = TESTITEMTYPE;
		m_data = ds;
	}
	else if (m_data.type == REAGENTTYPE)
	{
		DisplaySet ds;
		int index = 0;
		int type = 0;
		//childData.name = u8"位置";
		childData.postion = index++;
		childData.type = type++;
		childData.width = 100;
		childData.enDelete = false;
		childData.enSelect = true;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"项目名称";
		childData.postion = index++;
		childData.type = type++;
		childData.enDelete = false;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"可用测试数";
		childData.postion = index++;
		childData.type = type++;
		childData.enDelete = true;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"剩余测试数";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"使用状态";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"校准状态";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"试剂批号";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"试剂瓶号";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"当前曲线有效期";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"开瓶有效期";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"上机时间";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		//childData.name = u8"失效日期";
		childData.postion = index++;
		childData.type = type++;
		childData.enable = true;
		ds.displayItems.push_back(childData);

		ds.type = REAGENTTYPE;
		m_data = ds;
	}
}

QSelectNewDelegate::QSelectNewDelegate(QObject *parent)
{

}

bool QSelectNewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!index.isValid())
    {
        return false;
    }

    auto mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent == Q_NULLPTR)
    {
        return false;
    }

    if (event->type() == QEvent::MouseMove)
    {
        QString text = index.data(Qt::DisplayRole).toString();
        int iTextWidth = option.fontMetrics.width(text) + 20;
        int icellWidth = option.rect.width();
        if (iTextWidth > icellWidth)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (option.rect.contains(mouseEvent->pos()))
            {
                QToolTip::showText(mouseEvent->globalPos(), text);
            }
        }
    }

    QRect newRect = QRect(option.rect.right() - option.rect.width() / 8, option.rect.top() + 1, option.rect.width() / 5, option.rect.height() - 2);
    if (mouseEvent->type() == QMouseEvent::MouseButtonPress && newRect.contains(mouseEvent->pos()))
    {
        auto text = index.data(Qt::DisplayRole).toString();
        emit closeDelegate(text);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void QSelectNewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    QRect newRect = QRect(option.rect.right() - 38, option.rect.top() + 24, option.rect.width() / 5, option.rect.height() - 2);
    /*QRect newRect = QRect(option.rect.right() - option.rect.width() / 9, option.rect.top() + 16, option.rect.width() / 5, option.rect.height() - 2);*/

    QPixmap pixmap;
    pixmap.load(QString(":/Leonis/resource/image/icon-cancel.png"));
    painter->drawPixmap(newRect.left(), newRect.top(), pixmap);
    // 设置颜色
    QStyledItemDelegate::paint(painter, option, index);
}

///////////////////////////////////////////////////////////////////////////
/// @class     QShowTextDelegate
/// @brief 	   表格提示框代理
///////////////////////////////////////////////////////////////////////////
QShowTextDelegate::QShowTextDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
}

bool QShowTextDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    if (event->type() == QEvent::MouseMove)
    {
        QString text = index.data().toString();
        int iTextWidth = option.fontMetrics.width(text) + 20;
        int icellWidth = option.rect.width();
        if (iTextWidth > icellWidth)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (option.rect.contains(mouseEvent->pos()))
            {
                QToolTip::showText(mouseEvent->globalPos(), text);
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

