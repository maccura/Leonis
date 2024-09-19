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

#include "TrkStepModuleWgt.h"
#include "ui_TrkStepModuleWgt.h"
#include <QTableWidget>
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"

Q_DECLARE_METATYPE(std::string)

TrkStepModuleWgt::TrkStepModuleWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TrkStepModuleWgt)
    , m_nodeProperty{}
    , m_selectFlow("")
	, m_lastStep(0)
{
    ui->setupUi(this);

    InitUi();
    InitData();
    InitConnect();
}

TrkStepModuleWgt::~TrkStepModuleWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

void TrkStepModuleWgt::OnAddClicked()
{
    //正向修改目标位置
    int targetValue = ui->spinBox_targetPos->value() + ui->spinBox_pos->value();
    ui->spinBox_targetPos->setValue(targetValue);
}

void TrkStepModuleWgt::OnDelClicked()
{
    //反向修改目标位置
    int targetValue = ui->spinBox_targetPos->value() - ui->spinBox_pos->value();
    ui->spinBox_targetPos->setValue(targetValue);
}

void TrkStepModuleWgt::OnSaveClicked()
{

}

void TrkStepModuleWgt::OnExecuteClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (nullptr == _bitDebugger)
	{
		ULOG(LOG_ERROR, "_bitDebugger is nullptr");
		return;
	}

    int step = ui->spinBox_targetPos->value();
	int rtn = _bitDebugger->adjust_devcie_params(m_nodeProperty.node_id, m_nodeProperty.node_speci, m_selectFlow, step);
    if(rtn == 0)
    {
        // 记录之前选中的单元名称
        QModelIndex selIdx = ui->tableWidget->currentIndex();
        if (!selIdx.isValid() || ui->tableWidget->item(selIdx.row(), 0) == Q_NULLPTR)
        {
            return;
        }
        if(ui->tableWidget->item(selIdx.row(), 2)->text() != "/")
        {
            ui->tableWidget->item(selIdx.row(), 2)->setText(QString::number(step));
			//更新修改数据回表格
			std::string flowName = m_selectFlow + "(" + std::to_string(m_lastStep) + ")";
			//_bitDebugger->update_motor_step(m_nodeProperty.node_speci, flowName, step);
			m_lastStep = step;
        }
    }
	else if(rtn == 1)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(QString::fromStdString(m_selectFlow) + tr(" 执行失败!")));
		pTipDlg->exec();
	}
	
}

void TrkStepModuleWgt::OnSelectListItem()
{
    // 清空内容
    ui->frame->setVisible(false);
//    ui->frame_2->setVisible(false);
//    ui->btn_execute->setEnabled(false);

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
        auto pos = m_selectFlow.find_first_of("(");
        if(pos != std::string::npos)
        {
            m_selectFlow = m_selectFlow.substr(0, pos);
            ui->frame->setVisible(true);
            ui->spinBox_pos->setValue(100);
            ui->spinBox_targetPos->setValue(ui->tableWidget->item(selIdx.row(), 2)->text().toInt());
        }
    }
}

void TrkStepModuleWgt::InitUi()
{
    // 初始化tableWidget
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("名称") << tr("设置位置") << tr("目标位置"));
    // 表头
    ui->tableWidget->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::ResizeToContents);    // 根据内容自适应
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);   // 最后一列占满
    ResizeTblColToContent(ui->tableWidget);

	ui->frame->setVisible(false);
}

void TrkStepModuleWgt::InitData()
{

}

void TrkStepModuleWgt::InitConnect()
{
    connect(ui->btn_add, &QPushButton::clicked, this, &TrkStepModuleWgt::OnAddClicked);
    connect(ui->btn_del, &QPushButton::clicked, this, &TrkStepModuleWgt::OnDelClicked);
    connect(ui->btn_save, &QPushButton::clicked, this, &TrkStepModuleWgt::OnSaveClicked);
    connect(ui->btn_execute, &QPushButton::clicked, this, &TrkStepModuleWgt::OnExecuteClicked);

    // 选中项改变
    QItemSelectionModel* pSelModel = ui->tableWidget->selectionModel();
    if (pSelModel != Q_NULLPTR)
    {
        connect(pSelModel, &QItemSelectionModel::selectionChanged, this, &TrkStepModuleWgt::OnSelectListItem);
    }
}

void TrkStepModuleWgt::UpdateUi(node_property& nodeProperty, std::vector<Motor>& motors)
{
    m_nodeProperty = nodeProperty;

    // 重置行数为0
    ui->tableWidget->setRowCount(0);
    // 将数据加载到表格
    int iRow = 0;
    for(const auto& motor : motors)
    {
        //流程名称
        for(const auto& flow : motor.flowNames)
        {
            m_flows.emplace(flow.first, flow.second);

            // 行数不足则自增
            if (iRow == ui->tableWidget->rowCount())
            {
                ui->tableWidget->insertRow(iRow);
            }

            auto unitNameItem = new QTableWidgetItem(QString::fromStdString(motor.name));
            unitNameItem->setData(Qt::UserRole, QVariant::fromValue(flow.first));
            ui->tableWidget->setItem(iRow, 0, unitNameItem);
            std::string debug = "";
            std::stringstream ss(flow.first);
            while(std::getline(ss, debug, '_'))
            {
                if(debug.substr(0, 5) == "DEBUG")
                {
                    auto startPos = debug.find("(");
                    auto endPos = debug.find(")");
                    if(startPos != std::string::npos && endPos != std::string::npos)
                    {
						std::string step = debug.substr(startPos + 1, endPos - startPos - 1);
                        auto unitStepItem = new QTableWidgetItem(QString::fromStdString(step));
						m_lastStep = std::stoi(step);
                        ui->tableWidget->setItem(iRow, 1, unitStepItem);
                        auto unitTargetItem = new QTableWidgetItem(QString::fromStdString(step));
                        ui->tableWidget->setItem(iRow, 2, unitTargetItem);
                    }
                }
                else
                {
                    auto unitStepItem = new QTableWidgetItem(QString::fromStdString("/"));
                    ui->tableWidget->setItem(iRow, 1, unitStepItem);
                    auto unitTargetItem = new QTableWidgetItem(QString::fromStdString("/"));
                    ui->tableWidget->setItem(iRow, 2, unitTargetItem);
                }
            }
            iRow++;
        }
    }
}

