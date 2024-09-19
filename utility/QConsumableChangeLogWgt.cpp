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
/// @file     QConsumableChangeLogWgt.h
/// @brief 	  试剂/耗材更换日志窗口（应用->日志->试剂/耗材更换日志）
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
#include "QConsumableChangeLogWgt.h"
#include "ui_QConsumableChangeLogWgt.h"
#include "model/ConsumableChangeLogModel.h"
#include "shared/uicommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"

QConsumableChangeLogWgt::QConsumableChangeLogWgt(QWidget *parent)
	: QWidget(parent),
	m_bInit(false),
	ui(new Ui::QConsumableChangeLogWgt)
{
	ui->setupUi(this);
	Init();
}

QConsumableChangeLogWgt::~QConsumableChangeLogWgt()
{
	UNREGISTER_HANDLER(MSG_ID_CONSUMABLE_CHANGE_LOG_UPDATE, this, OnNotifyConsumableChangeLog);
	delete ui;
}

///
///  @brief 初始化
///
void QConsumableChangeLogWgt::Init()
{
	// 绑定起止日期编辑框(间隔时间不超过2个月)
	BindDateEdit(ui->lower_date_edit, ui->upper_date_edit, 2);

	// 表格初始化
	m_pModel = new QUtilitySortFilterProxyModel(this);
	auto srcModel = &ConsumableChangeLogModel::Instance();
	srcModel->SetSortModel(m_pModel);
	m_pModel->setSourceModel(srcModel);
	ui->tableView->setModel(m_pModel);
	QVector<int> sortColumns
	({ 
		//(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::SYSTEM,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::MODULE,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::NAME,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::ACTION,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::STATUS,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::USER,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::BATCHNUM,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::BOTTLENUM,
		(int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::TIME
	});
	m_pModel->SetTableViewSort(ui->tableView, sortColumns);

	ui->tableView->setModel(m_pModel);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);
	ui->tableView->setColumnHidden((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::ID, true);
	ui->tableView->sortByColumn((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::TIME, Qt::DescendingOrder);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	// 隐藏系统列（后面可能删除）
	ui->tableView->hideColumn((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::SYSTEM);

	// 试剂耗材更换日志
	REGISTER_HANDLER(MSG_ID_CONSUMABLE_CHANGE_LOG_UPDATE, this, OnNotifyConsumableChangeLog);

	// 查询数据库获取名称
	std::vector<std::string> vecNames;
	DcsControlProxy::GetInstance()->QueryConsumableChangeLogNames(vecNames);
	// 把名称添加到set中
	m_mapNames.clear();
	for (const auto& atName : vecNames)
	{
		m_mapNames.insert(QString(atName.c_str()));
	}
	// 初始化耗材名称下拉列表
	InitNameCombox();

	// 连接信号槽
	connect(ui->query_btn, SIGNAL(clicked()), this, SLOT(OnQueryBtnClicked()));
	connect(ui->reset_btn, SIGNAL(clicked()), this, SLOT(OnReset()));

	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::INDEX, 120);
	//ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::SYSTEM, 150);
	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::MODULE, 170);
	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::NAME, 210);
	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::ACTION, 200);
	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::STATUS, 210);
	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::USER, 180);
	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::BATCHNUM, 231);
	ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::BOTTLENUM, 200);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(100);
	//ui->tableView->setColumnWidth((int)ConsumableChangeLogModel::CONSUMABLE_CHANGE_LOG_LIST_COL::TIME, 350);
    // 设置代理
    ui->tableView->setItemDelegateForColumn(7, new CReadOnlyDelegate(this));

	OnReset();

    // 将初始化状态置为已完成
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
void QConsumableChangeLogWgt::showEvent(QShowEvent * event)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 基类先处理
	QWidget::showEvent(event);

	OnReset();
}

///
/// @brief 重置名称组合框
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年1月29日，新建函数
///
void QConsumableChangeLogWgt::InitNameCombox()
{
	// 上锁
	std::unique_lock<std::mutex> locker(m_mtxNames);
	// 先清空在添加
	ui->name_combox->clear();	
	// 添加一个“全部”的选项
	ui->name_combox->addItem(tr("全部"));
	// 把名称添加到组合框中
	for (const auto& atName : m_mapNames)
	{
		ui->name_combox->addItem(atName);
	}
}

///
/// @brief 通知UI试剂/耗材更换日志更新
///
/// @param[in]  ccl  日志信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年2月1日，新建函数
///
void QConsumableChangeLogWgt::OnNotifyConsumableChangeLog(const tf::ConsumableChangeLog& ccl)
{
	QString strName(ccl.consumablName.c_str());
	if (!strName.isEmpty())
	{
		// 如果没有找到
		if (m_mapNames.find(strName) == m_mapNames.end())
		{
			{
				std::unique_lock<std::mutex> locker(m_mtxNames);
				m_mapNames.insert(strName);
			}
			// 重置名称组合框
			InitNameCombox();
		}
	}
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
void QConsumableChangeLogWgt::OnReset()
{
	// 查询时间默认为一月
	QDate date = QDate::currentDate();
    //ui->lower_date_edit->setDate(date.addMonths(-1));
	//ui->upper_date_edit->setDate(date);
    //查询时间更改 bug 0021838 by ldx 20230906
	ui->upper_date_edit->setDate(date);
	ui->lower_date_edit->setDate(date);
	// 清空表格数据
	ConsumableChangeLogModel::Instance().ClearData();
	// 更新用户名
	ui->user_edit->clear();
	// 更新批号
	ui->batch_num_edit->clear();
	// 名称默认不选择
	ui->name_combox->setCurrentIndex(-1);

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
void QConsumableChangeLogWgt::OnQueryBtnClicked()
{
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

	// 清空表格
	ConsumableChangeLogModel::Instance().ClearData();

    // 查询
    ConsumableChangeLogQuery();
}

///
/// @brief	查询指定的操作日志
/// @par History:
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
void QConsumableChangeLogWgt::ConsumableChangeLogQuery()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    try
    {
        ::tf::ConsumableChangeLogQueryCond qryCond;
        ::tf::ConsumableChangeLogQueryResp qryResp;

        if (m_bInit)
        {
            // 时间
            ::tf::TimeRangeCond temp;
			temp.startTime = ui->lower_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 00:00:00";
			temp.endTime = ui->upper_date_edit->date().toString("yyyy/MM/dd").toStdString() + " 23:59:59";
            qryCond.__set_changeTime(temp);
            // 用户名称
            if (!ui->user_edit->text().isEmpty())
            {
                qryCond.__set_user(ui->user_edit->text().toStdString());
            }
            // 名称
            if (ui->name_combox->currentIndex() > 0)
            {
                qryCond.__set_consumablName(ui->name_combox->currentText().toStdString());
            }
            // 批号
            if (ui->batch_num_edit->text() != "")
            {
                qryCond.__set_batchNum(ui->batch_num_edit->text().toStdString());
            }
        }
        else
        {
            // 默认查询一个月的日志
            ::tf::TimeRangeCond temp;
            auto time = QDateTime::currentDateTime();
            temp.startTime = time.addMonths(-1).toString("yyyy/MM/dd HH:mm:ss").toStdString();
            temp.endTime = time.toString("yyyy/MM/dd HH:mm:ss").toStdString();
            qryCond.__set_changeTime(temp);
        }

        // 执行查询条件
        if (!DcsControlProxy::GetInstance()->QueryConsumableChangeLog(qryResp, qryCond) || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            // 查询失败
            ULOG(LOG_ERROR, "ConsumableChangeLogQuery() failed");
        }

        // 查询完成
        if (qryResp.lstConsumableChangeLog.empty())
        {
            ULOG(LOG_INFO, "ConsumableChangeLogQuery() Finished");
        }
        // 设置数据到ui渲染
        ConsumableChangeLogModel::Instance().SetData(qryResp.lstConsumableChangeLog);
        // 保存当前的查询条件
        ConsumableChangeLogModel::Instance().SetCurQueryCond(qryCond);
    }
    catch (...)
    {
        // 退出线程
        ULOG(LOG_INFO, "%s() execute failed!", __FUNCTION__);
        return;
    }
}
