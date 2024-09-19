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

#include "TrkScanModuleWgt.h"
#include "ui_TrkScanModuleWgt.h"
#include "src/common/Mlog/mlog.h"

Q_DECLARE_METATYPE(std::string)

TrkScanModuleWgt::TrkScanModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TrkScanModuleWgt)
    , m_nodeProperty{}
    , m_selectFlow("")
{
    ui->setupUi(this);

    InitUi();
    InitConnect();
}

TrkScanModuleWgt::~TrkScanModuleWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

void TrkScanModuleWgt::OnExecuteClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == _bitDebugger)
	{
		ULOG(LOG_ERROR, "_bitDebugger is nullptr");
		return;
	}

    if(_bitDebugger->adjust_devcie_params(m_nodeProperty.node_id, m_nodeProperty.node_speci, m_selectFlow, 0) == 0)
    {
        // 记录之前选中的单元名称
        QModelIndex selIdx = ui->tableWidget->currentIndex();
        if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
        {
            return;
        }
        std::string operate = "";
        std::stringstream ss(m_selectFlow);
        while(std::getline(ss, operate, '_'))
        {
            if(operate == "ON")
            {
                ui->tableWidget->item(selIdx.row(), 1)->setText("打开");

            }
            else if(operate == "RUN")
            {

            }
        }

    }
}

void TrkScanModuleWgt::OnSelectListItem()
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
    m_selectFlow = ui->tableWidget->item(selIdx.row(), 0)->data(Qt::UserRole).value<std::string>();
    if(!m_selectFlow.empty())
    {
        auto it = m_flows.find(m_selectFlow);
        if(it != m_flows.end())
        {
            ui->FlowName->setText(QString::fromStdString(it->second));
        }
    }
}

void TrkScanModuleWgt::InitUi()
{
    // 初始化tableWidget
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("名称") << tr("状态"));
    // 表头
    ui->tableWidget->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);    // 根据内容自适应
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);   // 最后一列占满
    //ResizeTblColToContent(ui->tableWidget);
}

void TrkScanModuleWgt::InitData()
{

}

void TrkScanModuleWgt::InitConnect()
{
    connect(ui->btn_execute, &QPushButton::clicked, this, &TrkScanModuleWgt::OnExecuteClicked);

    // 选中项改变
    QItemSelectionModel* pSelModel = ui->tableWidget->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, &QItemSelectionModel::selectionChanged, this, &TrkScanModuleWgt::OnSelectListItem);
    }
}

void TrkScanModuleWgt::UpdateUi(node_property& nodeProperty, std::vector<Scanner>& scanners)
{
    m_nodeProperty = nodeProperty;

    // 重置行数为0
    ui->tableWidget->setRowCount(0);
    // 将数据加载到表格
    int iRow = 0;
    for(const auto& scanner : scanners)
    {
        //流程名称
        for(const auto& flow : scanner.flowNames)
        {
            m_flows.emplace(flow.first, flow.second);

            // 行数不足则自增
            if (iRow == ui->tableWidget->rowCount())
            {
                ui->tableWidget->insertRow(iRow);
            }
            auto unitNameItem = new QTableWidgetItem(QString::fromStdString(scanner.name));
            unitNameItem->setData(Qt::UserRole, QVariant::fromValue(flow.first));
            ui->tableWidget->setItem(iRow, 0, unitNameItem);
            auto unitStatusItem = new QTableWidgetItem(QString("关闭"));
            ui->tableWidget->setItem(iRow, 1, unitStatusItem);
            iRow++;
        }
    }
}
