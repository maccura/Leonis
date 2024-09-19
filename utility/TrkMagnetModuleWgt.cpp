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

#include "TrkMagnetModuleWgt.h"
#include "ui_TrkMagnetModuleWgt.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"

Q_DECLARE_METATYPE(std::string)

TrkMagnetModuleWgt::TrkMagnetModuleWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TrkMagnetModuleWgt)
{
    ui->setupUi(this);

    InitUi();
    InitConnect();
}

TrkMagnetModuleWgt::~TrkMagnetModuleWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

void TrkMagnetModuleWgt::OnOpenClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == _bitDebugger)
	{
		ULOG(LOG_ERROR, "_bitDebugger is nullptr");
		return;
	}

	int rtn = _bitDebugger->adjust_devcie_params(m_nodeProperty.node_id, m_nodeProperty.node_speci, m_openFlow, 1);
    if(rtn == 0)
    {
        // 记录之前选中的单元名称
        QModelIndex selIdx = ui->tableWidget->currentIndex();
        if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
        {
            return;
        }
        ui->tableWidget->item(selIdx.row(), 2)->setText("打开");
    }
	else if (rtn == 1)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(QString::fromStdString(m_openFlow) + tr(" 执行失败!")));
		pTipDlg->exec();
	}
}

void TrkMagnetModuleWgt::OnCloseClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == _bitDebugger)
	{
		ULOG(LOG_ERROR, "_bitDebugger is nullptr");
		return;
	}

	int rtn = _bitDebugger->adjust_devcie_params(m_nodeProperty.node_id, m_nodeProperty.node_speci, m_closeFlow, 0);
    if(rtn == 0)
    {
        // 记录之前选中的单元名称
        QModelIndex selIdx = ui->tableWidget->currentIndex();
        if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
        {
            return;
        }
        ui->tableWidget->item(selIdx.row(), 2)->setText("关闭");
    }
	else if (rtn == 1)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(QString::fromStdString(m_closeFlow) + tr(" 执行失败!")));
		pTipDlg->exec();
	}
}

void TrkMagnetModuleWgt::OnSelectListItem()
{
    // 记录之前选中的单元名称
    QModelIndex selIdx = ui->tableWidget->currentIndex();
    if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
    {
        return;
    }

    // 选中单项
    QItemSelectionModel* pSelModel = ui->tableWidget->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        disconnect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnSelectListItem()));
        ui->tableWidget->selectRow(selIdx.row());
        connect(pSelModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnSelectListItem()));
    }

    // 获取流程
    auto it = m_rowFlow.find(selIdx.row());
    if(it != m_rowFlow.end())
    {
        m_openFlow = it->second.first;
        m_closeFlow = it->second.second;
        ui->FlowName->setText(QString::fromStdString(m_openFlow + "   " + m_closeFlow));
    }
}

void TrkMagnetModuleWgt::InitUi()
{
    // 初始化tableWidget
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("名称") << tr("类型") << tr("是否打开"));
    // 表头
    ui->tableWidget->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);    // 根据内容自适应
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);   // 最后一列占满
    ResizeTblColToContent(ui->tableWidget);
}

void TrkMagnetModuleWgt::InitData()
{

}

void TrkMagnetModuleWgt::InitConnect()
{
    connect(ui->btn_open, &QPushButton::clicked, this, &TrkMagnetModuleWgt::OnOpenClicked);
    connect(ui->btn_close, &QPushButton::clicked, this, &TrkMagnetModuleWgt::OnCloseClicked);

    // 选中项改变
    QItemSelectionModel* pSelModel = ui->tableWidget->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, &QItemSelectionModel::selectionChanged, this, &TrkMagnetModuleWgt::OnSelectListItem);
    }
}

void TrkMagnetModuleWgt::UpdateUi(node_property& nodeProperty, std::vector<Electromagnet>& magnets)
{
    m_nodeProperty = nodeProperty;

    // 重置行数为0
    ui->tableWidget->setRowCount(0);
    // 将数据加载到表格
    int iRow = 0;
    for(const auto& magnet : magnets)
    {
        // 行数不足则自增
        if (iRow >= ui->tableWidget->rowCount())
        {
            ui->tableWidget->insertRow(iRow);
        }

        std::string onFlow = "";
        std::string offFlow = "";
        for(const auto& flow : magnet.flowNames)
        {
            std::string act = "";
            std::stringstream ss(flow.first);
            while(std::getline(ss, act, '_'))
            {
                if(act == "ON")
                {
                    onFlow = flow.first;
                }
                else if(act == "OFF")
                {
                    offFlow = flow.first;
                }
            }
        }
        m_rowFlow.emplace(iRow, std::make_pair(onFlow, offFlow));

        // 电磁铁名称
        auto unitNameItem = new QTableWidgetItem(QString::fromStdString(magnet.name));
        ui->tableWidget->setItem(iRow, 0, unitNameItem);
        if(magnet.type)
        {
            auto unitTypeItem = new QTableWidgetItem("常开型");
            ui->tableWidget->setItem(iRow, 1, unitTypeItem);
            auto unitStatusItem = new QTableWidgetItem("打开");
            ui->tableWidget->setItem(iRow, 2, unitStatusItem);
        }
        else
        {
            auto unitTypeItem = new QTableWidgetItem("常闭型");
            ui->tableWidget->setItem(iRow, 1, unitTypeItem);
            auto unitStatusItem = new QTableWidgetItem("关闭");
            ui->tableWidget->setItem(iRow, 2, unitStatusItem);
        }
        iRow++;
    }
}


