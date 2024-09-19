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
/// @file     QOperationLogWgt.h
/// @brief 	  操作日志窗口（应用->日志->操作日志）
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QOperationLogWgt.h"
#include "ui_QOperationLogWgt.h"
#include "model/OperationLogModel.h"
#include "shared/uicommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "src/common/Mlog/mlog.h"
#include "shared/tipdlg.h"
#include "thrift/DcsControlProxy.h"

///
/// @brief 操作日志列表栏名称索引
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月15日
/// 
enum OperLogHorHeaderCol
{
	OperLogHorHeaderCol_Order =	0,	// 序号
	OperLogHorHeaderCol_User,		// 用户名
	OperLogHorHeaderCol_Type,		// 操作类型
	OperLogHorHeaderCol_Record,		// 操作记录
	OperLogHorHeaderCol_Time,		// 操作时间
};


QOperationLogWgt::QOperationLogWgt(QWidget *parent)
	: QWidget(parent),
	m_bInit(false),
	ui(new Ui::QOperationLogWgt)
{
	ui->setupUi(this);
	Init();
}

QOperationLogWgt::~QOperationLogWgt()
{
	delete ui;
}

///
///  @brief 初始化
///
void QOperationLogWgt::Init()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 绑定起止日期编辑框(间隔时间不超过2个月)
	BindDateEdit(ui->lower_date_edit, ui->upper_date_edit, 2);

	// 表格初始化
	m_pModel = new QUtilitySortFilterProxyModel(this);
	auto srcModel = &OperationLogModel::Instance();
	srcModel->SetSortModel(m_pModel);
	m_pModel->setSourceModel(srcModel);
	ui->tableView->setModel(m_pModel);

	QVector<int> sortColumns({ 
		(int)OperationLogModel::OPERATION_LOG_LIST_COL::USER,
		(int)OperationLogModel::OPERATION_LOG_LIST_COL::TYPE,
		(int)OperationLogModel::OPERATION_LOG_LIST_COL::RECORD,
		(int)OperationLogModel::OPERATION_LOG_LIST_COL::TIME
	});
	m_pModel->SetTableViewSort(ui->tableView, sortColumns);
	
	ui->tableView->setModel(m_pModel);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);
	ui->tableView->setColumnHidden((int)OperationLogModel::OPERATION_LOG_LIST_COL::ID, true);
	ui->tableView->sortByColumn((int)OperationLogModel::OPERATION_LOG_LIST_COL::ID, Qt::DescendingOrder);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	// 初始化操作类型下拉列表
	m_operationTypeMap[::tf::OperationType::UNKNOWN] = tr("");
	m_operationTypeMap[::tf::OperationType::ADD] = tr("新增");
	m_operationTypeMap[::tf::OperationType::DEL] = tr("删除");
	m_operationTypeMap[::tf::OperationType::MOD] = tr("修改");
	QStringList items;
	for (auto each : m_operationTypeMap)
	{
		items.append(each.second);
	}
	ui->operationType_cobox->addItems(items);

	// 伸展按钮
	ui->expand_btn->setProperty("isHide", false);
	ui->expand_btn->style()->unpolish(ui->expand_btn);
	ui->expand_btn->style()->polish(ui->expand_btn);

	// 连接信号槽
	connect(ui->query_btn, SIGNAL(clicked()), this, SLOT(OnQueryBtnClicked()));
	connect(ui->reset_btn, SIGNAL(clicked()), this, SLOT(OnReset()));
	connect(ui->expand_btn, &QPushButton::clicked, this, [&]
	{
		bool status = ui->expand_btn->property("isHide").toBool();
		ui->expand_btn->setProperty("isHide", !status);
		ui->expand_btn->style()->unpolish(ui->expand_btn);
		ui->expand_btn->style()->polish(ui->expand_btn);
		int iWidth = ui->widget_3->width();
		ui->widget_3->setVisible(status);
		// 调整操作日志栏宽度 add by chenjianlin 20230815
		ui->tableView->setColumnWidth(OperLogHorHeaderCol_Record, 
			status ? ui->tableView->columnWidth(OperLogHorHeaderCol_Record)-iWidth : ui->tableView->columnWidth(OperLogHorHeaderCol_Record) + iWidth);
	});

    // 键盘上下键切换选择行未刷新详细信息，由原来的点击信号改为当前行改变的信号。生化bug：13197
    // connect(ui->tableView, &QTableView::clicked, this, [=](const QModelIndex &index)
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](const QModelIndex &current, const QModelIndex &previous)
	{
        if (!current.isValid())
        {
            return;
        }
        auto idx = m_pModel->index(current.row(), (int)OperationLogModel::OPERATION_LOG_LIST_COL::RECORD);
        if (!idx.isValid())
        {
            return;
        }

        ui->record_label->setText(m_pModel->data(idx, Qt::DisplayRole).toString());
	});
	// 设置列表栏宽度
	ui->tableView->setColumnWidth(OperLogHorHeaderCol_Order, 50);
	ui->tableView->setColumnWidth(OperLogHorHeaderCol_User, 300);
	ui->tableView->setColumnWidth(OperLogHorHeaderCol_Type, 200);
	ui->tableView->setColumnWidth(OperLogHorHeaderCol_Record, 500);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(80);

    // 设置代理
    ui->tableView->setItemDelegateForColumn(OperLogHorHeaderCol_Record, new CReadOnlyDelegate(this));

	// 重置ui
	OnReset();

    m_bInit = true;
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
void QOperationLogWgt::showEvent(QShowEvent * event)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 基类先处理
	QWidget::showEvent(event);

	OnReset();
}

///
///  @brief 重置界面
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
void QOperationLogWgt::OnReset()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询时间默认为一月
	QDate date = QDate::currentDate();
	//ui->lower_date_edit->setDate(date.addMonths(-1));
	//ui->upper_date_edit->setDate(date);
    //查询时间更改 bug 0021838 by ldx 20230906
	ui->upper_date_edit->setDate(date);
    ui->lower_date_edit->setDate(date);
    ui->record_label->clear();
	// 清空表格数据
	OperationLogModel::Instance().ClearData();

	ui->user_edit->clear();
	ui->operationType_cobox->setCurrentIndex(0);

	// 还原默认顺序
	ui->tableView->sortByColumn(-1, Qt::DescendingOrder);
	m_pModel->sort(-1, Qt::DescendingOrder);
}

///
/// @brief
///     查询按钮槽函数
///
/// @par History:
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
void QOperationLogWgt::OnQueryBtnClicked()
{
	// 清空界面右侧的操作记录详细信息，生化bug13188
    ui->record_label->clear();

	QDateTime startTime(ui->lower_date_edit->date(), QTime(0, 0, 0));
	QDateTime endTime(ui->upper_date_edit->date(), QTime(0, 0, 0));

	if (startTime > endTime)
	{
		TipDlg(tr("值错误"), tr("起始日期不能大于截止日期！")).exec();
		return;
	}

	if (startTime.addMonths(2) < endTime)
	{
		TipDlg(tr("值错误"), tr("日志查找最大时间跨度不得超过两个月")).exec();
		return;
	}

	OperationLogModel::Instance().ClearData();
    OperationLogQuery();
}

///
/// @brief	查询指定的操作日志
/// @par History:
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
void QOperationLogWgt::OperationLogQuery()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    try
    {
        ::tf::OperationLogQueryCond qryCond;
        ::tf::OperationLogQueryResp qryResp;

        if (m_bInit)
        {
            // 时间
            ::tf::TimeRangeCond temp;
			temp.startTime = ui->lower_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00";
			temp.endTime = ui->upper_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59";
            qryCond.__set_operationRange(temp);

            // 用户名称
            if (!ui->user_edit->text().isEmpty())
            {
                qryCond.__set_user(ui->user_edit->text().toStdString());
            }

            // 操作类型
            if (ui->operationType_cobox->currentIndex() != 0)
            {
                qryCond.__set_operationType((::tf::OperationType::type)ui->operationType_cobox->currentIndex());
            }
        }
        else
        {
            // 默认查询一个月的日志
            ::tf::TimeRangeCond temp;
            auto time = QDateTime::currentDateTime();
            temp.startTime = time.addMonths(-1).toString("yyyy/MM/dd HH:mm:ss").toStdString();
            temp.endTime = time.toString("yyyy/MM/dd HH:mm:ss").toStdString();
            qryCond.__set_operationRange(temp);
        }
        // 执行查询条件
        if (!DcsControlProxy::GetInstance()->QueryOperationLog(qryResp, qryCond) || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            // 查询失败
            ULOG(LOG_ERROR, "QueryOperationLog() failed");
        }

        // 查询完成
        if (qryResp.lstOperationLog.empty())
        {
            ULOG(LOG_INFO, "qryResp.lstOperationLog.empty()");
        }

        // 设置数据到ui渲染
        OperationLogModel::Instance().SetData(qryResp.lstOperationLog);

        // 保存当前的查询条件
        OperationLogModel::Instance().SetCurQueryCond(qryCond);
    }
    catch (...)
    {
        // 退出线程
        ULOG(LOG_INFO, "%s() execute failed!", __FUNCTION__);
        return;
    }
}
