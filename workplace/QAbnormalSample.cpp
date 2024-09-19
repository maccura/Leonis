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

#include "QAbnormalSample.h"
#include "ui_QAbnormalSample.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/QComDelegate.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QRadioButton>

QAbnormalSample::QAbnormalSample(QWidget *parent)
    : QWorkShellPage(parent)
    , m_sampleModel(Q_NULLPTR)
    , m_assayModel(Q_NULLPTR)
{
    ui = new Ui::QAbnormalSample();
    ui->setupUi(this);
    m_sampleModel = new QAbnormalSampleModel();
    m_assayModel = new QAbnormalAssayModel();
    ui->tableView->setItemDelegateForColumn(0, new CheckBoxDelegate(true, this));
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setModel(m_sampleModel);
    ui->result_detail_btn->hide();
    ui->flat_recheck->hide();

	ui->tableView->horizontalHeader()->setMinimumSectionSize(45);
	ui->tableView->setColumnWidth(0, 45);
	ui->tableView->setColumnWidth(1, 200);
	ui->tableView->setColumnWidth(2, 300);
	ui->tableView->setColumnWidth(3, 200);
	ui->tableView->setColumnWidth(4, 200);

	auto header = ui->tableView->horizontalHeader();
    auto selectAllButton = new QCheckBox(header);
    connect(selectAllButton, &QCheckBox::clicked, this,
        [&]()
    {
    });

    // 1：右上角部件的显示
    m_cornerItem = new QWidget(m_parent);
    QHBoxLayout* hlayout = new QHBoxLayout(m_cornerItem);
    m_cornerItem->setLayout(hlayout);

    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    // 设置互斥
    pButtonGroup->setExclusive(true);
    QRadioButton *pButton = new QRadioButton(tr("样本异常"));
    pButton->setObjectName(QStringLiteral("showBySample_btn"));
    pButton->setCheckable(true);
    connect(pButton, SIGNAL(clicked()), this, SLOT(ShowBySample()));
    pButton->setChecked(true);
    pButtonGroup->addButton(pButton);
    hlayout->addWidget(pButton);

    pButton = new QRadioButton(tr("结果异常"));
    pButton->setObjectName(QStringLiteral("showByAssay_btn"));
    connect(pButton, SIGNAL(clicked()), this, SLOT(ShowByResult()));
    pButtonGroup->addButton(pButton);
    hlayout->addWidget(pButton);
    m_parent->setCornerWidget(m_cornerItem);
    m_cornerItem->show();
}

QAbnormalSample::~QAbnormalSample()
{
}

void QAbnormalSample::RefreshCornerWidgets(int index)
{
    // 句柄检查（不能为空）
    if (m_parent == nullptr || m_cornerItem == nullptr)
    {
        return;
    }

    // 设置切换句柄
    m_parent->setCornerWidget(m_cornerItem);
    m_cornerItem->show();
}

void QAbnormalSample::ShowBySample()
{
    if (m_sampleModel != Q_NULLPTR)
    {
        ui->tableView->setModel(m_sampleModel);
    }

    ui->result_detail_btn->hide();
    ui->flat_recheck->hide();
}

void QAbnormalSample::ShowByResult()
{
    if (m_assayModel != Q_NULLPTR)
    {
        ui->tableView->setModel(m_assayModel);
    }

    ui->result_detail_btn->show();
    ui->flat_recheck->show();
}

QAbnormalSampleModel::QAbnormalSampleModel()
{
    m_headerNames << tr("") << tr("样本号") << tr("样本条码") << tr("托盘") << tr("位置") << tr("样本类型") << tr("异常原因");
}

int QAbnormalSampleModel::rowCount(const QModelIndex &parent) const
{
    return 16;
}

int QAbnormalSampleModel::columnCount(const QModelIndex &parent) const
{
    return m_headerNames.size();
}

QVariant QAbnormalSampleModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant QAbnormalSampleModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (section < 0 || section >= m_headerNames.size())
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_headerNames.at(section);
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

QAbnormalAssayModel::QAbnormalAssayModel()
{
    m_headerNames.clear();
    m_headerNames << tr("") << tr("样本号") << tr("样本条码") << tr("托盘") << tr("位置") << tr("项目名称") << tr("结果") << tr("结果状态");
}

QVariant QAbnormalAssayModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}
