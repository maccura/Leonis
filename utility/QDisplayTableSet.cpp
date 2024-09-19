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
#include "QDisplayTableSet.h"
#include "ui_QDisplayTableSet.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <QObject>
#include <QPainter>
#include <QStandardItemModel>

#include "shared/tipdlg.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "manager/DictionaryQueryManager.h"

#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#define MAXNUM 15

QDisplayTableSet::QDisplayTableSet(QWidget *parent)
    : QWidget(parent)
    , m_beSelectModel(Q_NULLPTR)
    , m_selectedModel(Q_NULLPTR)
    , m_checkBoxAll(Q_NULLPTR)
{
    ui = new Ui::QDisplayTableSet();
    ui->setupUi(this);
    Init();
}

QDisplayTableSet::~QDisplayTableSet()
{
}

bool QDisplayTableSet::IsModified()
{
	if (m_oriData.displayItems.size() != m_data.displayItems.size())
	{
		return true;
	}

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
	for (const auto& item : m_oriData.displayItems)
	{
		DisplaySetItem tmpItem = item;
		tmpItem.postion = j++;
		newSortItems1.push_back(tmpItem);
	}

	int i = 0;
	for (const auto& item : newSortItems)
	{
		auto tmp = newSortItems1[i++];
		// 若都被屏蔽了，那么无论其他项数据是否相等，都视为同一状态
		if (!item.enable && !tmp.enable)
		{
			continue;
		}

		// 若都未被选中，那么无需考虑其他
		if (!item.enSelect && !tmp.enSelect)
		{
			continue;
		}

		if (item == tmp)
		{
			continue;
		}

		return true;
	}

	return false;
}

///
/// @brief 设置表类型
///
/// @param[in]  data  设置数据
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QDisplayTableSet::SetTableData(const DisplaySet& data)
{
    // 保存最原始的数据
    m_oriData = data;
    if (m_beSelectModel == Q_NULLPTR || m_selectedModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "new QStandardItemModel failed");
        return;
    }

    //m_beSelectModel->clear();
    m_selectedModel->clear();

	m_beSelectModel->setHorizontalHeaderLabels({ tr(""), tr("名称") });
    m_selectedModel->setHorizontalHeaderLabels({""});
        

    m_data = data;
    int beSelectNum = 0;
    m_selectedNum = 0;
    int row = 0;
    for (auto& itemData : data.displayItems)
    {
        auto setPostion = itemData.type;
        auto checkBoxItem = m_beSelectModel->item(row, 0);
        if (checkBoxItem == nullptr)
        {
            checkBoxItem = new QStandardItem("");
            // 设置行位置
            m_beSelectModel->setItem(row, 0, checkBoxItem);
        }

        QWidget *widget = new QWidget(ui->beSelect_View);
        QCheckBox *checkbox = new QCheckBox(ui->beSelect_View);
        checkbox->setProperty("type", itemData.type);
        QHBoxLayout *hLayout = new QHBoxLayout();

        // 获取名称（自定义的不需要转为内置的，内置的需要在界面实现文字转换，便于翻译）
        QString name;
        if ((::tf::PatientFields::type)itemData.type < tf::PatientFields::PATIENT_CUSTOM_BASE)
            name = std::move(ConvertTfEnumToQString((::tf::PatientFields::type)itemData.type));

        if (name.isEmpty())
            name = std::move(QString::fromStdString(itemData.name));

        if (itemData.enable)
        {
            checkbox->setChecked(true);
            if ((itemData.type == tf::PatientFields::PATIENT_SEQ_NO) ||
                (itemData.type == tf::PatientFields::PATIENT_BARCODE) ||
                (itemData.type == tf::PatientFields::PATIENT_NAME))
                checkbox->setEnabled(false);
            
            auto item = new QStandardItem(name);

            //item->setData(Qt::AlignHCenter, Qt::TextAlignmentRole);
            m_selectedModel->setItem(itemData.postion, 0, item);
            m_selectedNum++;
        }
        else
        {
            checkbox->setChecked(false);
        }

        beSelectNum++;
        hLayout->addWidget(checkbox);
        hLayout->setAlignment(checkbox, Qt::AlignCenter);
        hLayout->setMargin(0);
        widget->setLayout(hLayout);
        ui->beSelect_View->setIndexWidget(ui->beSelect_View->model()->index(row, 0), widget);
        connect(checkbox, &QCheckBox::stateChanged, this, &QDisplayTableSet::OnCheckBoxClicked);
        // 设置名称
        auto item = m_beSelectModel->item(row, 1);
        if (item == nullptr)
        {
            item = new QStandardItem();
            m_beSelectModel->setItem(row++, 1, item);
        }
        else
        {
            row++;
        }
        item->setText(name);
        item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    }

    int moreRows = m_beSelectModel->rowCount() - row;

    // 清除多余行
    if (moreRows > 0)
    {
        m_beSelectModel->removeRows(row, moreRows);
    }

    // 设置全选行
    const auto& firstColIndex = ui->beSelect_View->horizontalHeader()->model()->index(0, 0);
    if (firstColIndex.isValid())
    {
        QWidget *widget = new QWidget(ui->beSelect_View->horizontalHeader());
        widget->setMinimumSize(40, 40);
        QHBoxLayout *hLayout = new QHBoxLayout(widget);
        m_checkBoxAll = new QCheckBox();

        hLayout->addWidget(m_checkBoxAll);
        hLayout->setAlignment(m_checkBoxAll, Qt::AlignCenter);
        hLayout->setMargin(0);
        widget->setLayout(hLayout);
        ui->beSelect_View->horizontalHeader()->setIndexWidget(firstColIndex, widget);
        widget->resize(ui->beSelect_View->horizontalHeader()->sectionSize(0), ui->beSelect_View->horizontalHeader()->height());
        ui->beSelect_View->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
		ui->beSelect_View->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        connect(m_checkBoxAll, &QPushButton::clicked, this, &QDisplayTableSet::OnSelecAllClicked);
        // 调整列宽后，重新设置第一列widget的宽度
        /*connect(ui->beSelect_View->horizontalHeader(), &QHeaderView::sectionResized, this, [this, widget](int logicalIndex, int oldSize, int newSize) {
            if (0 == logicalIndex)
                widget->resize(newSize, ui->beSelect_View->horizontalHeader()->height());
        });*/
    }

    ui->selected_label->setText(QString(tr("已选（%1）")).arg(m_selectedNum));
    ui->beSelect_label->setText(QString(tr("备选（%1）")).arg(beSelectNum));
    UpdateBtnState();
    UpdateSelectAllState();

    ui->beSelect_View->horizontalHeader()->setStretchLastSection(true);
}

void QDisplayTableSet::showEvent(QShowEvent *event)
{
    //if (ui->selected_View->verticalScrollBar()->isVisible())
    //{
    //    ui->clearAll_Button->move(1340/* - 15*/, 12);
    //}
    //else
    //{
    //    ui->clearAll_Button->move(1340, 12);
    //}
}

///
/// @brief 复选框的槽函数（点击复选框）
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QDisplayTableSet::OnCheckBoxClicked(int state)
{
    Q_UNUSED(state);
    // 如不能获取对应的控件，则返回
    auto sendSrc = qobject_cast<QCheckBox*>(sender());
    if (sendSrc == Q_NULLPTR)
    {
        return;
    }

    int type = sendSrc->property("type").toInt();
    // 查找对应的数据
   auto findIter =  std::find_if(m_data.displayItems.begin(), m_data.displayItems.end(), [&type](const auto& element) 
    {
        return element.type == type;
    });

   // 找不到数据，则返回
   if (findIter == m_data.displayItems.end())
   {
       return;
   }

   QString name = QString::fromStdString(findIter->name);
   if (findIter->type < tf::PatientFields::PATIENT_CUSTOM_BASE)
       name = ConvertTfEnumToQString((::tf::PatientFields::type)findIter->type);

   if (sendSrc->isChecked())
   {
      auto items = m_selectedModel->findItems(name);
      if (items.empty())
      {
          m_selectedNum++;
          m_selectedModel->appendRow(new QStandardItem(name));
          // 使能
          //findIter->enable = true;
          // 放到最后一位
          //findIter->postion = m_selectedModel->rowCount();
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
       //findIter->enable = false;

       // 依次缩进-提高位置
       /*for (auto& dataItem : m_data.displayItems)
       {
           if (dataItem.postion > findIter->postion)
           {
               dataItem.postion--;
           }
       }*/
   }

   if (m_selectedNum >= MAXNUM)
   {
       ui->clearAll_Button->move(1340/* - 15*/, 12);
   }
   else
   {
       ui->clearAll_Button->move(1340, 12);
   }

   ui->selected_label->setText(QString(tr("已选(%1)")).arg(m_selectedNum));
   UpdateBtnState();
   UpdateSelectAllState();

}

///
/// @brief 保存数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月6日，新建函数
///
void QDisplayTableSet::OnSaveData()
{
    FUNC_ENTRY_LOG();

    std::vector<DisplaySet> savedata;
    if (!DictionaryQueryManager::GetUiDispalySet(savedata))
    {
        ULOG(LOG_ERROR, "Failed to DisplaySet.");
        TipDlg(tr("提醒"), "保存失败！").exec();
        return;
    }

    for (auto& itemData : m_data.displayItems)
    {
        // 获取名称（自定义的不需要转为内置的，内置的需要在界面实现文字转换，便于翻译）
        QString name;
        if ((::tf::PatientFields::type)itemData.type < tf::PatientFields::PATIENT_CUSTOM_BASE)
            name = std::move(ConvertTfEnumToQString((::tf::PatientFields::type)itemData.type));

        if (name.isEmpty())
            name = std::move(QString::fromStdString(itemData.name));

        auto items = m_selectedModel->findItems(name);
        if (items.empty())
        {
            itemData.postion = -1; 
            itemData.enable = false;
        }
        else
        {
            itemData.postion = items[0]->row();
            itemData.enable = true;
        }
    }

    // 更新数据
    for (auto& data : savedata)
    {
        if (data.type == m_data.type)
        {
            data = m_data;
        }
    }

    if (DictionaryQueryManager::SaveUiDisplaySet(savedata))
    {
        POST_MESSAGE(MSG_ID_PATINET_INFO_POSITION_UPDATE, m_data);
        m_oriData = m_data;
        TipDlg(tr("提示"), tr("保存成功")).exec();
    }
    else
    {
        TipDlg(tr("提示"), tr("保存失败")).exec();
    }
}

void QDisplayTableSet::OnDataDictUpdated(DisplaySet dsSet)
{
    m_oriData = dsSet;
    SetTableData(dsSet);
}
///
/// @brief 初始化函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QDisplayTableSet::Init()
{
    m_beSelectModel = new QStandardItemModel(this);
    m_selectedModel = new QStandardItemModel(this);

    if (m_beSelectModel == Q_NULLPTR || m_selectedModel == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "new QStandardItemModel failed");
        return;
    }

    ui->beSelect_View->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->beSelect_View->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->beSelect_View->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->beSelect_View->verticalHeader()->setVisible(false);
    ui->beSelect_View->setItemDelegateForColumn(0, new CReadOnlyDelegate(this));
    ui->beSelect_View->setItemDelegateForColumn(1, new CReadOnlyDelegate(this));

    ui->selected_View->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->selected_View->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->selected_View->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->selected_View->verticalHeader()->setVisible(false);

    ui->beSelect_View->setModel(m_beSelectModel);
    ui->selected_View->setModel(m_selectedModel);

    // 保存
    connect(ui->save_Button, &QPushButton::clicked, this, [&]()
    {
        OnSaveData();
    });

    // 清空全部
    connect(ui->clearAll_Button, &QPushButton::clicked, this, [&]()
    {
        int rowIndex = 0;
        int rowCount = m_beSelectModel->rowCount();
        while (rowCount > 0)
        {
            auto checkbox = GetRowCheckBox(rowIndex);

            if (checkbox != Q_NULLPTR)
            {
                int type = checkbox->property("type").toInt();
                if (type == tf::PatientFields::PATIENT_SEQ_NO ||
                    type == tf::PatientFields::PATIENT_BARCODE ||
                    type == tf::PatientFields::PATIENT_NAME)
                {
                }
                else
                {
                    checkbox->setChecked(false);
                }
            }

            rowIndex++;
            rowCount--;
        }
    });

    // 向上
    connect(ui->pre_Button, &QPushButton::clicked, this, [&]()
    {
        ULOG(LOG_INFO, "Up");
        auto currentIndex = GetSelectViewCurrentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        ULOG(LOG_INFO, "text is %s", currentIndex.data().toString().toStdString());

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        auto currentPos = currentIndex.row() - 1;
        m_selectedModel->insertRow(currentPos, items);
        ui->selected_View->setCurrentIndex(m_selectedModel->index(currentPos, 0));
    });

    // 向下
    connect(ui->next_Button, &QPushButton::clicked, this, [&]()
    {
        ULOG(LOG_INFO, "Down");
        auto currentIndex = GetSelectViewCurrentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        ULOG(LOG_INFO, "text is %s", currentIndex.data().toString().toStdString());

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        auto currentPos = currentIndex.row() + 1;
        m_selectedModel->insertRow(currentPos, items);
        ui->selected_View->setCurrentIndex(m_selectedModel->index(currentPos, 0));
    });

    // 置顶
    connect(ui->top_Button, &QPushButton::clicked, this, [&]()
    {
        ULOG(LOG_INFO, "Top");
        auto currentIndex = GetSelectViewCurrentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        ULOG(LOG_INFO, "text is %s", currentIndex.data().toString().toStdString());

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        m_selectedModel->insertRow(0, items);
        ui->selected_View->setCurrentIndex(m_selectedModel->index(0, 0));
    });

    // 置底
    connect(ui->bottom_Button, &QPushButton::clicked, this, [&]()
    {
        ULOG(LOG_INFO, "Buttom");
        auto currentIndex = GetSelectViewCurrentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        ULOG(LOG_INFO, "text is %s", currentIndex.data().toString().toStdString());

        auto items = m_selectedModel->takeRow(currentIndex.row());
        if (items.empty())
        {
            return;
        }

        m_selectedModel->insertRow(m_selectedModel->rowCount(), items);
        auto postion = m_selectedModel->rowCount() - 1;
        ui->selected_View->setCurrentIndex(m_selectedModel->index(postion, 0));
    });

    // 重置
    connect(ui->reset_Button, &QPushButton::clicked, this, [&] {
        ULOG(LOG_INFO, "Reset");
        const auto& defaultData = GetDefaultData();
        // 没有自定义项目
        if (m_data.displayItems.size() == defaultData.displayItems.size())
        {
            SetTableData(defaultData);
        }
        else
        {
            for (auto& data : m_data.displayItems)
            {
                // 查找对应的数据
                auto findIter = std::find_if(defaultData.displayItems.begin(), defaultData.displayItems.end(), [&data](const auto& element)
                {
                    return element.type == data.type;
                });

                if (findIter != defaultData.displayItems.end())
                {
                    data = *findIter;
                }
                // 自定义项目
                else
                {
                    data.postion = -1;
                    data.enable = false;
                }
            }
            SetTableData(m_data);
        }
    });

    // 点击更新当前按钮状态
    connect(ui->selected_View->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&](const QItemSelection &selected, const QItemSelection &deselected)
    {
        Q_UNUSED(selected);
        Q_UNUSED(deselected);
        UpdateBtnState();
    });

    auto deleGate = new QSelectDelegate(this);
    ui->selected_View->setItemDelegateForColumn(0, deleGate);
    connect(deleGate, &QSelectDelegate::closeDelegate, this, [&](QString text)
    {
        ULOG(LOG_INFO, "closeDelegate, text: %s", text.toStdString());
        auto itemBeSelects = m_beSelectModel->findItems(text, Qt::MatchExactly, 1);
        if (itemBeSelects.empty())
            return;

        for (auto& beItemSelect : itemBeSelects)
        {
            auto checkBox = GetRowCheckBox(beItemSelect->index().row());

            if (checkBox != Q_NULLPTR)
            {
                // 样本号和样本条码不能删除
                int type = checkBox->property("type").toInt();
                ULOG(LOG_INFO, "type: %d", type);
                if (type == tf::PatientFields::PATIENT_SEQ_NO ||
                    type == tf::PatientFields::PATIENT_BARCODE ||
                    type == tf::PatientFields::PATIENT_NAME)
                    continue;

                auto items = m_selectedModel->findItems(text, Qt::MatchExactly);

                /*for (auto item : items)
                {
                    m_selectedNum--;
                    m_selectedModel->removeRow(item->row());
                }*/

                checkBox->setChecked(false);
                /*if (beItemSelect->index().row() < m_data.displayItems.size())
                {
                    m_data.displayItems[beItemSelect->index().row()].enable = false;
                }*/
            }
        }

        //ui->selected_label->setText(QString(tr("已选(%1)")).arg(m_selectedNum));
        //UpdateBtnState();
        //UpdateSelectAllState();
    });

    // 注册数据字典更新消息
    REGISTER_HANDLER(MSG_ID_PATINET_DATA_DICT_ITEM_UPDATE, this, OnDataDictUpdated);
}

///
/// @brief 更新按钮状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void QDisplayTableSet::UpdateBtnState()
{
    if (m_selectedNum >= MAXNUM)
    {
        ui->clearAll_Button->move(1340/* - 15*/,12);
    }
    else
    {
        ui->clearAll_Button->move(1340, 12);
    }

    ui->next_Button->setEnabled(false);
    ui->pre_Button->setEnabled(false);
    ui->bottom_Button->setEnabled(false);
    ui->top_Button->setEnabled(false);

    int rowCount = m_selectedModel->rowCount();
    auto currentIndex = GetSelectViewCurrentIndex();
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
bool QDisplayTableSet::AdjustItemPostion(QString name, int postion)
{
    for (auto & data : m_data.displayItems)
    {
        QString typeName = std::move(QString::fromStdString(data.name));
        if (data.type < tf::PatientFields::PATIENT_CUSTOM_BASE)
            typeName = ConvertTfEnumToQString((tf::PatientFields::type)data.type);

        if ( typeName == name)
        {
            data.postion = postion;
            return true;
        }
    }

    return false;
}

QCheckBox* QDisplayTableSet::GetRowCheckBox(int row)
{
    auto item = m_beSelectModel->item(row, 0);
    if (item != Q_NULLPTR)
    {
        auto widget = qobject_cast<QWidget*>(ui->beSelect_View->indexWidget(item->index()));
        if (widget != Q_NULLPTR)
        {
            auto checkBoxList = widget->findChildren<QCheckBox*>();
            if (!checkBoxList.isEmpty())
                return checkBoxList[0];
        }
    }

    return nullptr;
}

void QDisplayTableSet::OnSelecAllClicked()
{
    // 设置标记，避免后续信号造成重复处理
    m_bIsUpdateSelectAll = true;

    if (m_checkBoxAll == Q_NULLPTR )
        return;

    for (int i = 0; i < m_beSelectModel->rowCount(); ++i)
    {
        auto checkBox = GetRowCheckBox(i);
        int type = checkBox->property("type").toInt();
        if (type == tf::PatientFields::PATIENT_BARCODE ||
            type == tf::PatientFields::PATIENT_SEQ_NO ||
            type == tf::PatientFields::PATIENT_NAME)
            continue;

        if (checkBox)
        {
            if (checkBox->isChecked() != m_checkBoxAll->isChecked())
            {
                checkBox->setChecked(m_checkBoxAll->isChecked());
            }
        }
    }

    // 重置标记
    m_bIsUpdateSelectAll = false;
}

void QDisplayTableSet::hideEvent(QHideEvent * event)
{
    // 界面切换时，清除未保存的数据，恢复至原始数据
    SetTableData(m_oriData);

    QWidget::hideEvent(event);
}

void QDisplayTableSet::UpdateSelectAllState()
{
    // 对话框为空，或者正在执行操作，直接返回
    if (m_checkBoxAll == Q_NULLPTR || m_bIsUpdateSelectAll)
        return;

    // 设置全选的勾选状态
    if (m_selectedNum == m_beSelectModel->rowCount())
        m_checkBoxAll->setChecked(true);
    else
        m_checkBoxAll->setChecked(false);
}

DisplaySet QDisplayTableSet::GetDefaultData()
{
    DisplaySet ds;
    DisplaySetItem childData;
    int index = 0;
    //int type = 1;
    //childData.name = u8"样本号";
    childData.name = "";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_SEQ_NO;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"样本条码";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_BARCODE;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"患者ID";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_RECORD_NO;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"患者类型";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_TYPE;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"临床诊断";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_DIAGNOSIS;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"姓名";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_NAME;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"年龄";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_AGE;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"性别";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_GENDER;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"送检时间";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_INSPECT_TIME;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"送检科室";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_INSPECT_DEPARTMENT;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"送检医师";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_INSPECT_DOCTOR;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"采样时间";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_SAMPLING_TIME;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"检验医师";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_INSPECTOR;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"住院号";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_ADMISSION_NO;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"收费类型";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_COST_TYPE;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"审核医师";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_AUDITOR;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"病区";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_INPATIENT_AREA;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"医保账号";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_ACCOUNT_NO;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"主治医生";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_ATTENDING_DOCTOR;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"床号";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_BED_NO;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"出生日期";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_BIRTHDAY;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    //childData.name = u8"备注";
    childData.postion = index++;
    childData.type = ::tf::PatientFields::PATIENT_COMMENT;
    childData.enable = true;
    ds.displayItems.push_back(childData);

    //childData.name = u8"电话";
    childData.postion = -1;
    childData.type = ::tf::PatientFields::PATIENT_PHONE_NO;
    childData.enable = false;
    ds.displayItems.push_back(childData);

    ds.type = PATIENTTYPE;

    return ds;
}

QModelIndex QDisplayTableSet::GetSelectViewCurrentIndex()
{
    auto selIndexes = ui->selected_View->selectionModel()->selectedIndexes();
    if (!selIndexes.isEmpty())
    {
        return selIndexes[0];
    }

    return QModelIndex();
}

QSelectDelegate::QSelectDelegate(QObject *parent)
{

}

bool QSelectDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
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

    QRect newRect = QRect(option.rect.right() - option.rect.width() / 8, option.rect.top() + 1, option.rect.width() / 5, option.rect.height() - 2);
    if (mouseEvent->type() == QMouseEvent::MouseButtonPress && newRect.contains(mouseEvent->pos()))
    {
        auto text = index.data(Qt::DisplayRole).toString();
        emit closeDelegate(text);
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void QSelectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	QRect newRect = QRect(option.rect.right() - 38, option.rect.top() + 24, option.rect.width() / 5, option.rect.height() - 2);
	/*QRect newRect = QRect(option.rect.right() - option.rect.width() / 9, option.rect.top() + 16, option.rect.width() / 5, option.rect.height() - 2);*/

    QPixmap pixmap;
    pixmap.load(QString(":/Leonis/resource/image/icon-cancel.png"));
    painter->drawPixmap(newRect.left(), newRect.top(), pixmap);
    // 设置颜色
    QStyledItemDelegate::paint(painter, option, index);
}
