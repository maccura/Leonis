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

#include "QWorkPageSet.h"
#include "ui_QWorkPageSet.h"
#include <QHBoxLayout>
#include <QStandardItem>

#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"


QWorkPageSet::QWorkPageSet(QWidget *parent)
    : QWidget(parent)
    , m_model(Q_NULLPTR)
{
    ui = new Ui::QWorkPageSet();
    ui->setupUi(this);
    Init();
}

QWorkPageSet::~QWorkPageSet()
{
}

bool QWorkPageSet::IsModified()
{
	PageSet tmpSet;
	if (!DictionaryQueryManager::GetPageset(tmpSet))
	{
		ULOG(LOG_ERROR, "Failed to get pageset.");
		return false;
	}

	if (tmpSet == m_data)
	{
		return false;
	}

	return true;
}

void QWorkPageSet::Init()
{
    // 与时间同步
    connect(ui->checkBox_call, &QCheckBox::toggled, this, [&]()
    {
        QCheckBox* check = qobject_cast<QCheckBox*>(sender());
        if (check == Q_NULLPTR)
        {
            return;
        }

        if (check->isChecked())
        {
            ui->lineEdit_time->setEnabled(true);
            ui->lineEdit_time->setFocus();
        }
        else
        {
            ui->lineEdit_time->setEnabled(false);
        }
    });

    // 限制时间为正整数
    ui->lineEdit_time->setValidator(new QIntValidator(0, 99999999, this));

    // 保存
    connect(ui->pushButton_save, &QPushButton::clicked, this, [&]()
    {
        // 设置数据
        if (CheckEditorIsZero(ui->lineEdit_time))
        {
            TipDlg(tr("监控页面呼出时间不能设置为0")).exec();
            return;
        }
        
        m_data.callTime.first = ui->checkBox_call->isChecked();
        m_data.callTime.second = ui->lineEdit_time->text().toInt();

        if (!DictionaryQueryManager::SavePageset(m_data))
        {
            ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
        }
        else
        {
            POST_MESSAGE(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE);
        }

		TipDlg(tr("保存成功")).exec();
    });

    // 向上按钮
    connect(ui->pushButton_up, &QPushButton::clicked, this, [&]()
    {
        auto index = ui->tableView->currentIndex();
        if (!index.isValid())
        {
            return;
        }

        // 如果已经是第一行则无法继续往上调整
        auto row = index.row();
        if (row == 0)
        {
            return;
        }

        // 超界了
        if (row >= m_data.sampleShows.size())
        {
            return;
        }

        ChangeViewRow(row, row - 1);
        Refresh();
        ui->tableView->selectRow(row - 1);

        if (0 == (row - 1))
        {
            ui->pushButton_up->setEnabled(false);
        }
        ui->pushButton_down->setEnabled(true);

    });

    // 向下按钮
    connect(ui->pushButton_down, &QPushButton::clicked, this, [&]()
    {
        auto index = ui->tableView->currentIndex();
        if (!index.isValid())
        {
            return;
        }

        // 如果已经是最后一行
        auto row = index.row();
        if (row == 2)
        {
            return;
        }

        // 超界了
        if (row >= m_data.sampleShows.size())
        {
            return;
        }

        ChangeViewRow(row, row + 1);
        Refresh();
        ui->tableView->selectRow(row + 1);

        if (2 == (row + 1))
        {
            ui->pushButton_down->setEnabled(false);
        }
        ui->pushButton_up->setEnabled(true);
    });

	// modify bug3421 by wuht
	ui->pushButton_down->setEnabled(false);
	ui->pushButton_up->setEnabled(false);
	connect(ui->tableView, &QTableView::clicked, this, [&](const QModelIndex& current)
	{
		if (!current.isValid())
		{
			ui->pushButton_down->setEnabled(false);
			ui->pushButton_up->setEnabled(false);
			return;
		}

		// 若是第一行，则屏蔽向上按钮
		int row = current.row();
		if (row == 0)
		{
			ui->pushButton_up->setEnabled(false);
		}
		else
		{
			ui->pushButton_up->setEnabled(true);
		}

		// 最后一行
		if (row == 2)
		{
			ui->pushButton_down->setEnabled(false);
		}
		else
		{
			ui->pushButton_down->setEnabled(true);
		}
	});

	auto f = [&](QCheckBox* checkBox, bool& flag)
	{
		connect(checkBox, &QCheckBox::clicked, this, [&]()
		{
			QCheckBox* check = qobject_cast<QCheckBox*>(sender());
			if (check == Q_NULLPTR)
			{
				return;
			}

			if (check->isChecked())
			{
				flag = true;
			}
			else
			{
				flag = false;
			}
		});
	};

	// 筛选
	f(ui->checkBox_select, m_data.filter);
	// 结果详情
	f(ui->checkBox_detail, m_data.detail);
	// 审核
	f(ui->checkBox_audit, m_data.audit);
	// 打印
	f(ui->checkBox_print, m_data.print);
	// 手工传输
	f(ui->checkBox_transfer, m_data.transfer);
	// 导出
	f(ui->checkBox_export, m_data.exported);
	// 患者信息
	f(ui->checkBox_patient, m_data.patient);
	// 复查
	f(ui->checkBox_recheck, m_data.recheck);
	// 删除
	f(ui->checkBox_delete, m_data.deleteData);
	// 重新计算
	f(ui->checkBox_recomput, m_data.reComput);
	// 弹窗设置
	f(ui->checkBox_transferOpen, m_data.transferManual.first);
	// Ai设置
	f(ui->checkBox_transferOpen_2, m_data.aiRecognition);
	// 监控页面定时呼出
	f(ui->checkBox_call, m_data.callTime.first);
	UpdateInfo();
}

void QWorkPageSet::Refresh()
{
    if (m_model == Q_NULLPTR)
    {
        return;
    }

    m_model->clear();
    m_model->setHorizontalHeaderLabels({tr("显示"), tr("订单类型") });

    int row = 0;
    for (auto& data : m_data.sampleShows)
    {
        QString bookTypeName = "";
        switch (data.second)
        {
            case 0:
				bookTypeName = tr("校准订单");
                break;
            case 1:
                bookTypeName = tr("质控订单");
                break;
            case 2:
				bookTypeName = tr("样本订单");
                break;
            default:
                break;
        }

        auto checkBox = fSetCheck(data, row);
        auto item = new QStandardItem(bookTypeName);
        item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        m_model->setItem(row++, 1, item);
		ui->tableView->setColumnWidth(0, 255);
		ui->tableView->horizontalHeader()->setStretchLastSection(true);
    }
}

bool QWorkPageSet::ChangeViewRow(int srcRow, int dstRow)
{
    auto model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (model == Q_NULLPTR)
    {
        return false;
    }

    if (srcRow < 0 || srcRow > model->rowCount())
    {
        return false;
    }

    if (dstRow < 0 || dstRow > model->rowCount())
    {
        return false;
    }

    // 交换全局数据
    auto srcData = m_data.sampleShows[srcRow];
    auto dstData = m_data.sampleShows[dstRow];
    m_data.sampleShows[srcRow] = dstData;
    m_data.sampleShows[dstRow] = srcData;

    return true;
}

QWidget* QWorkPageSet::fSetCheck(std::pair<bool, int>& data, int row)
{
    auto checkBoxItem = new QStandardItem("");
    m_model->setItem(row, 0, checkBoxItem);
    QWidget *widget = new QWidget(ui->tableView);
    QCheckBox *checkbox = new QCheckBox();
    checkbox->setProperty("type", data.second);
    QHBoxLayout *hLayout = new QHBoxLayout();
    if (data.first)
    {
        checkbox->setChecked(true);
    }
    else
    {
        checkbox->setChecked(false);
    }

    hLayout->addWidget(checkbox);
    hLayout->setAlignment(checkbox, Qt::AlignCenter);
    hLayout->setMargin(0);
    widget->setLayout(hLayout);
    ui->tableView->setIndexWidget(ui->tableView->model()->index(row, 0), widget);
    connect(checkbox, &QCheckBox::clicked, this,
        [&]()
    {
        auto checkBox = qobject_cast<QCheckBox*>(sender());
        if (checkBox == Q_NULLPTR)
        {
            return;
        }

        // 获取类型
        int type = checkBox->property("type").toInt();
        if (type >= m_data.sampleShows.size())
        {
            return;
        }

        bool flag = checkBox->isChecked() ? true : false;
        for (auto& aimData : m_data.sampleShows)
        {
            if (aimData.second == type)
            {
                aimData.first = flag;
            }
        }
    });

    return widget;
}

///
/// @brief 更新消息（bug3463）
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年10月20日，新建函数
///
void QWorkPageSet::UpdateInfo()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!DictionaryQueryManager::GetPageset(m_data))
    {
        ULOG(LOG_ERROR, "Failed to get pageset.");
        return;
    }

	auto f = [&](QCheckBox* checkBox, bool& flag)
	{
		if (flag)
		{
			checkBox->setChecked(true);
		}
		else
		{
			checkBox->setChecked(false);
		}
	};

	// 筛选
	f(ui->checkBox_select, m_data.filter);
	// 结果详情
	f(ui->checkBox_detail, m_data.detail);
	// 审核
	f(ui->checkBox_audit, m_data.audit);
	// 打印
	f(ui->checkBox_print, m_data.print);
	// 手工传输
	f(ui->checkBox_transfer, m_data.transfer);
	// 导出
	f(ui->checkBox_export, m_data.exported);
	// 患者信息
	f(ui->checkBox_patient, m_data.patient);
	// 复查
	f(ui->checkBox_recheck, m_data.recheck);
	// 删除
	f(ui->checkBox_delete, m_data.deleteData);
	// 重新计算
	f(ui->checkBox_recomput, m_data.reComput);
	// 弹窗设置
	f(ui->checkBox_transferOpen, m_data.transferManual.first);
	// Ai设置
	f(ui->checkBox_transferOpen_2, m_data.aiRecognition);
	// 监控页面定时呼出
	f(ui->checkBox_call, m_data.callTime.first);
    ui->lineEdit_time->setText(m_data.callTime.second == 0 ? "" : QString::number(m_data.callTime.second));

	auto softType = CommonInformationManager::GetInstance()->GetSoftWareType();
	if (softType == SOFTWARE_TYPE::CHEMISTRY)
	{
		ui->checkBox_recomput->hide();
	}
	else
	{
		ui->checkBox_recomput->show();
	}

	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView->verticalHeader()->setVisible(false);
	m_model = new QStandardItemModel(ui->tableView);
	ui->tableView->setModel(m_model);
	
	Refresh();
	ui->tableView->setColumnWidth(0, 255);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);

	bool isPipel = DictionaryQueryManager::GetInstance()->GetPipeLine();
	if (!isPipel)
	{
		ui->checkBox_transferOpen_2->hide();
	}
	else
	{
		ui->checkBox_transferOpen_2->show();
	}
}

void QWorkPageSet::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	UpdateInfo();
}
