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

#include "QAssaySet.h"
#include "ui_QAssaySet.h"
#include <QStandardItemModel>
#include "src/common/Mlog/mlog.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"

QAssaySet::QAssaySet(QWidget *parent /*= Q_NULLPTR*/)
    : QWidget(parent)
    , m_model(Q_NULLPTR)
{
    ui = new Ui::QAssaySet();
    ui->setupUi(this);
    Init();
}

void QAssaySet::showEvent(QShowEvent *event)
{
	ui->assayView->setCurrentIndex(QModelIndex());
    OnReFresh();
}

bool QAssaySet::IsModified()
{
	OrderAssay saveOrder;
	for (int row = 0; row < m_model->rowCount(); row++)
	{
		auto item = m_model->item(row, 0);
		if (item == Q_NULLPTR)
		{
			continue;
		}

		saveOrder.orderVec.emplace_back(item->data(Qt::UserRole + 1).toInt());
	}

	OrderAssay newOrder;
	newOrder.enableCustom = m_showOrder.enableCustom;
	newOrder.enableContinuousSample = m_showOrder.enableContinuousSample;
	saveOrder.enableContinuousSample = m_showOrder.enableContinuousSample;
	saveOrder.enableCustom = m_showOrder.enableCustom;
	for (const auto& item : m_showOrder.orderVec)
	{
		auto assayName = GetAssayName(item);
		if (assayName.empty())
		{
			continue;
		}

		newOrder.orderVec.emplace_back(item);
	}

	if (!ui->user_checkBox->isChecked())
	{
		newOrder.enableCustom = false;
	}
	else
	{
		newOrder.enableCustom = true;
	}

	if (saveOrder == newOrder)
	{
		return false;
	}

	return true;
}

void QAssaySet::SetButtonStatus(bool isChecked)
{
	// 初始化
	ui->top_Button->setEnabled(false);
	ui->assayView->setEnabled(false);
	ui->assayView->clearSelection();
	ui->pre_Button->setEnabled(false);
	ui->next_Button->setEnabled(false);
	ui->bottom_Button->setEnabled(false);
	ui->reset_Button->setEnabled(false);

	// 自定义打开
	if (isChecked)
	{
		ui->assayView->setEnabled(true);
		ui->reset_Button->setEnabled(true);
	}
}

void QAssaySet::Init()
{
    if (m_model == Q_NULLPTR)
    {
        m_model = new QStandardItemModel(ui->assayView);
    }

    // 更新数据
    OnReFresh();

    // 生化
    connect(ui->chButton, &QPushButton::clicked, this, [&]()
    {
        this->OnReFresh();
    });

    // 免疫
    connect(ui->imButton, &QPushButton::clicked, this, [&]()
    {
        this->OnReFresh();
    });

    // 重置
    connect(ui->reset_Button, &QPushButton::clicked, this, [&]()
    {
        this->OnReset();
    });

    // 保存
    connect(ui->save_Button, &QPushButton::clicked, this, [&]()
    {
        OnSaveData();
        emit this->closeDialog();
    });

	// modify bug0011160 by wuht
	SetButtonStatus(ui->user_checkBox->isChecked());
    // 自定义按钮是否打开
    connect(ui->user_checkBox, &QCheckBox::clicked, this, [&]()
    {
		SetButtonStatus(ui->user_checkBox->isChecked());
    });

	// modify bug0012050 by wuht
	connect(ui->assayView->selectionModel(), &QItemSelectionModel::currentRowChanged, [=](const QModelIndex& index, const QModelIndex& preIndex)
	{
		ui->assayView->setCurrentIndex(index);
		UpdateBtnState();
	});

    // 向上
    connect(ui->pre_Button, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assayView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        if (MoveItem(currentIndex.row(), true))
        {
            ui->assayView->setCurrentIndex(m_model->index(currentIndex.row() - 1, 0));
        }

        UpdateBtnState();
    });

    // 向下
    connect(ui->next_Button, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assayView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        if (MoveItem(currentIndex.row(), false))
        {
            ui->assayView->setCurrentIndex(m_model->index(currentIndex.row() + 1, 0));
        }

        UpdateBtnState();
    });

    // 置顶
    connect(ui->top_Button, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assayView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        // 多次移动
        for (int row = currentIndex.row(); row > 0; row--)
        {
            if (!MoveItem(row, true))
            {
                return;
            }
        }

        ui->assayView->setCurrentIndex(m_model->index(0, 0));
		ui->assayView->setFocus();
        UpdateBtnState();
    });

    // 置底
    connect(ui->bottom_Button, &QPushButton::clicked, this, [&]()
    {
        auto currentIndex = ui->assayView->currentIndex();
        if (!currentIndex.isValid())
        {
            return;
        }

        // 多次移动
        for (int row = currentIndex.row(); row < m_model->rowCount() - 1; row++)
        {
            if (!MoveItem(row, false))
            {
                return;
            }
        }

        ui->assayView->setCurrentIndex(m_model->index(m_model->rowCount() - 1, 0));
		ui->assayView->setFocus();
        UpdateBtnState();
    });

    // 点击更新当前按钮状态
    connect(ui->assayView, &QTableView::clicked, this, [&](const QModelIndex& current)
    {
        UpdateBtnState();
    });
}

///
/// @brief 移动项目往上或者往下
///
/// @param[in]  row         当前位置
/// @param[in]  direction   true:往上，false:往下
///
/// @return true，移动成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年3月2日，新建函数
///
bool QAssaySet::MoveItem(int row, bool direction)
{
    if (m_model == Q_NULLPTR || m_model->rowCount() <= row || row < 0)
    {
        return false;
    }

    auto currentIndex = m_model->index(row, 0);

    QModelIndex destIndex = QModelIndex();
    // 往上
    if (direction)
    {
        // 第一行不能再往上移动了
        if (row == 0)
        {
            return false;
        }

        destIndex = m_model->index(row - 1, 0);
    }
    // 往下
    else
    {
        // 最后一行不能再往下了
        if (m_model->rowCount() == (row + 1))
        {
            return false;
        }

        destIndex = m_model->index(row + 1, 0);
    }

    // 获取位置
    auto currentItem = m_model->takeItem(currentIndex.row(), 0);
    auto destItem = m_model->takeItem(destIndex.row(), 0);

    // 交换位置
    m_model->setItem(destIndex.row(), 0, currentItem);
    m_model->setItem(currentIndex.row(), 0, destItem);
    ui->assayView->update();
    return true;
}

void QAssaySet::OnReset()
{
    m_model->clear();
    QStringList assayHeadList = { tr("名称") };
    m_model->setHorizontalHeaderLabels(assayHeadList);
    ui->assayView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->assayView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->assayView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->assayView->verticalHeader()->setVisible(false);
    ui->assayView->setModel(m_model);
    ui->assayView->setColumnWidth(0, 200);

	std::set<int> assayMaps;
	// 1:普通项目编号排序
    auto assayCodeMaps = CommonInformationManager::GetInstance()->GetAssayInfo();
	for (const auto& item : assayCodeMaps)
	{
		if (item.first == ch::tf::g_ch_constants.ASSAY_CODE_SIND)
		{
			continue;
		}

		if (item.first == ise::tf::g_ise_constants.ASSAY_CODE_ISE)
		{
			continue;
		}

		assayMaps.insert(item.first);
	}

	// 计算项目的编号排序
	auto calcMap = CommonInformationManager::GetInstance()->GetCalcAssayInfoMap();
	for (const auto& item : calcMap)
	{
		assayMaps.insert(item.first);
	}

    int row = 0;
    for (const auto assayCode : assayMaps)
    {
		// bug0010961-获取项目名称
		std::string name = GetAssayName(assayCode);
		if (name.empty())
		{
			continue;
		}

        auto nameitem = new QStandardItem(QString::fromStdString(name));
        nameitem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        nameitem->setData(assayCode);
        m_model->setItem(row, 0, nameitem);
        row++;
    }
}

void QAssaySet::OnReFresh()
{
    if (m_model == Q_NULLPTR)
    {
        return;
    }

    m_model->clear();
    QStringList assayHeadList = { tr("序号")/*, tr("项目名称") */};
    m_model->setHorizontalHeaderLabels(assayHeadList);
    ui->assayView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->assayView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->assayView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->assayView->verticalHeader()->setVisible(false);
    ui->assayView->setModel(m_model);

    //ui->assayView->setColumnWidth(0, 100);
    ui->assayView->setColumnWidth(0, 200);

    if (!DictionaryQueryManager::GetAssayShowOrder(m_showOrder))
    {
        ULOG(LOG_ERROR, "Failed to get assayshoworder config.");
        return;
    }

    ui->groupBox->hide();
    ui->chButton->hide();
    ui->imButton->hide();

    if (!m_showOrder.enableCustom)
    {
        ui->user_checkBox->setChecked(false);
        ui->assayView->setEnabled(false);
        ui->pre_Button->setEnabled(false);
        ui->next_Button->setEnabled(false);
        ui->bottom_Button->setEnabled(false);
        ui->reset_Button->setEnabled(false);
		// modify bug0011161
		ui->top_Button->setEnabled(false);
    }
    else
    {
		ui->user_checkBox->setChecked(true);
		ui->assayView->setEnabled(true);
		// bug0011160
		auto index = ui->assayView->currentIndex();
		if (index.isValid())
		{
			ui->assayView->setEnabled(true);
			ui->pre_Button->setEnabled(true);
			ui->next_Button->setEnabled(true);
			ui->bottom_Button->setEnabled(true);
			// modify bug0011161
			ui->top_Button->setEnabled(true);
		}
		ui->reset_Button->setEnabled(true);
    }

    int row = 0;
    for (const auto& item : m_showOrder.orderVec)
    {
        auto assayName = GetAssayName(item);
        if (assayName.empty())
        {
            continue;
        }

        //m_model->setItem(row, 0, new QStandardItem(QString::number(row + 1)));

        auto nameitem = new QStandardItem(QString::fromStdString(assayName));
        nameitem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        nameitem->setData(item);
        m_model->setItem(row, 0, nameitem);
        row++;
    }
}

void QAssaySet::OnSaveData()
{
    if (m_model == Q_NULLPTR)
    {
        return;
    }

    const auto& CIM = CommonInformationManager::GetInstance();
    for (auto& dev : CIM->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_I6000,::tf::DeviceType::DEVICE_TYPE_ISE1005,
        ::tf::DeviceType::DEVICE_TYPE_C1000, ::tf::DeviceType::DEVICE_TYPE_I6000 }))
    {
        if (devIsRun(*dev))
        {
            TipDlg(tr("保存失败"), tr("仪器运行中不能修改项目显示顺序！")).exec();
            return;
        }
    }

    OrderAssay saveOrder;
    for (int row = 0; row < m_model->rowCount(); row++)
    {
        auto item = m_model->item(row, 0);
        if (item == Q_NULLPTR)
        {
            continue;
        }

        saveOrder.orderVec.push_back(item->data(Qt::UserRole + 1).toInt());
    }

    if (!ui->user_checkBox->isChecked())
    {
        saveOrder.enableCustom = false;
    }
    else
    {
        saveOrder.enableCustom = true;
    }

    if (!DictionaryQueryManager::SaveAssayShowOrder(saveOrder))
    {
        return;
    }

	m_showOrder = saveOrder;
	// 修改显示顺序记录操作日志 add by chenjianlin 20231215
	AddOptLog(tf::OperationType::type::MOD, tr("修改显示顺序"));
    // 更新界面
    POST_MESSAGE(MSG_ID_SAMPLE_AND_ITEM_COLUMN_DISPLAY_UPDATE);
	TipDlg(tr("保存成功")).exec();
}

void QAssaySet::UpdateBtnState()
{
    ui->next_Button->setEnabled(false);
    ui->pre_Button->setEnabled(false);
    ui->bottom_Button->setEnabled(false);
    ui->top_Button->setEnabled(false);

    int rowCount = m_model->rowCount();
    auto currentIndex = ui->assayView->currentIndex();
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

std::string QAssaySet::GetAssayName(int assayCode)
{
	// 首先判断是否血清指数
// 	if (ch::tf::g_ch_constants.ASSAY_CODE_L == assayCode)
// 	{
// 		return "L";
// 	}
// 	else if (ch::tf::g_ch_constants.ASSAY_CODE_H == assayCode)
// 	{
// 		return "H";
// 	}
// 	else if (ch::tf::g_ch_constants.ASSAY_CODE_I == assayCode)
// 	{
// 		return "I";
// 	}
// 	else
	{
		// 普通项目
		auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
		if (spAssay != Q_NULLPTR)
		{
			return spAssay->assayName;
		}

		// 计算项目
		auto spCalcAssay = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
		if (spCalcAssay != Q_NULLPTR)
		{
			return spCalcAssay->name;
		}
	}

	return "";
}
