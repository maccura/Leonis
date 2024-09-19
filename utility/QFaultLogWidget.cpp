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
/// @file     QFaultLogWidget.cpp
/// @brief    故障日志功能展示
///
/// @author   7951/LuoXin
/// @date     2022年7月5日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年7月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QFaultLogWidget.h"
#include "SortHeaderView.h"
#include "ui_QFaultLogWidget.h"
#include <QTimer>
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

#include "thrift/DcsControlProxy.h"
#include "src/public/ConfigSerialize.h"
#include "src/common/defs.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"

#define QUERY_ID_RANG										(1000)					  // 分页查询id范围
#define QUERY_BUFF_SIZE										(1000)					  // 异步查询缓存大小（数据条数）
#define GET_BUFF_INTERVAL									(10)					  // 获取缓存的时间间隔（毫秒）


QFaultLogWidget::QFaultLogWidget(QWidget *parent)
	: QWidget(parent),
    m_isManager(false),
	m_pGetBufferTimer(new QTimer(this)),
	ui(new Ui::QFaultLogWidget)
{
	ui->setupUi(this);

	// 设置读取缓存间隔
	m_pGetBufferTimer->setInterval(GET_BUFF_INTERVAL);

	Init();
}

QFaultLogWidget::~QFaultLogWidget()
{

}

void QFaultLogWidget::ResetWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 停止查询线程
    if (m_pGetBufferTimer->isActive())
    {
        m_ambNeedStop = true;
    }

    // 清空表格
    FaultLogTableModel::Instance().RemoveAllRows();

    // 清空详细信息
    ui->detail_label->clear();

    // 清空解决防范
    ui->solution_label->clear();

    // 查询时间默认为一天
    QDate date = QDate::currentDate();
	ui->upper_date_edit->setDate(date);
	ui->lower_date_edit->setDate(date);

    // 更新模块下拉框
    ui->model_cobox->clear();
    ui->model_cobox->addItem(tr("全部"));
    ui->model_cobox->addItem(tr("控制单元"), "ControlUnit");
    auto cmPtr = CommonInformationManager::GetInstance();
    bool isSingle = !DictionaryQueryManager::GetInstance()->GetPipeLine();
    if (cmPtr != nullptr)
    {
        const auto& devInfos = cmPtr->GetDeviceMaps();

        //若为免疫单机版仅显示M1
        if (cmPtr->GetImisSingle())
        {
            for (auto dev : devInfos)
            {
                if (dev.second->deviceType == ::tf::DeviceType::DEVICE_TYPE_I6000)
                {
                    ui->model_cobox->addItem(QString::fromStdString(dev.second->groupName + dev.second->name)
                        , QString::fromStdString(dev.second->deviceSN));

                    break;
                }
            }
        }
        else
        {
            for (auto dev : devInfos)
            {
                // 单机版排除轨道
                if (isSingle && dev.second->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
                {
                    continue;
                }

                ui->model_cobox->addItem(QString::fromStdString(dev.second->groupName + dev.second->name)
                    , QString::fromStdString(dev.second->deviceSN));
            }
        }
    }

    ui->model_cobox->setCurrentIndex(0);
    ui->level_combox->setCurrentIndex(0);
    ui->alarm_code_edit->clear();

    // 还原默认顺序
    ui->tableView->sortByColumn(-1, Qt::DescendingOrder);
    m_sortModel->sort(-1, Qt::DescendingOrder);

    update();
}

void QFaultLogWidget::Init()
{
    // Sort Header
    // 升序 降序 原序
    m_sortModel = new QUtilitySortFilterProxyModel(this);
    auto srcModel = &FaultLogTableModel::Instance();
    srcModel->SetSortModel(m_sortModel);
    m_sortModel->setSourceModel(srcModel);
    ui->tableView->setModel(m_sortModel);

    QVector<int> sortColumns({ FaultLogTableModel::FAULT_LOG_LIST_COL::MODULE,
        FaultLogTableModel::FAULT_LOG_LIST_COL::CODE,
        FaultLogTableModel::FAULT_LOG_LIST_COL::LEVEL,
        FaultLogTableModel::FAULT_LOG_LIST_COL::NAME,
        FaultLogTableModel::FAULT_LOG_LIST_COL::TIME
    });
    m_sortModel->SetTableViewSort(ui->tableView, sortColumns);

    ui->tableView->hideColumn(FaultLogTableModel::FAULT_LOG_LIST_COL::SYSTEM);

	ui->tableView->setColumnWidth(FaultLogTableModel::FAULT_LOG_LIST_COL::INDEX, 120);
	ui->tableView->setColumnWidth(FaultLogTableModel::FAULT_LOG_LIST_COL::MODULE, 200);
	ui->tableView->setColumnWidth(FaultLogTableModel::FAULT_LOG_LIST_COL::CODE, 200);
    ui->tableView->setColumnWidth(FaultLogTableModel::FAULT_LOG_LIST_COL::LEVEL, 200);
	ui->tableView->setColumnWidth(FaultLogTableModel::NAME, 500);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(80);


    // 设置时间格式代理
    ui->tableView->setItemDelegateForColumn(FaultLogTableModel::FAULT_LOG_LIST_COL::TIME, new CReadOnlyDelegate(this));
    // 设置报警等级颜色代理
    ui->tableView->setItemDelegateForColumn(FaultLogTableModel::FAULT_LOG_LIST_COL::LEVEL, new CReadOnlyDelegate(this));

    // 伸展按钮
    ui->expand_btn->setProperty("isHide", false);
    ui->expand_btn->style()->unpolish(ui->expand_btn);
    ui->expand_btn->style()->polish(ui->expand_btn);

	// 初始化告警级别下拉框	
	for (auto each : ::tf::_AlarmLevel_VALUES_TO_NAMES)
	{
		auto type = (::tf::AlarmLevel::type)each.first;
        if (type == tf::AlarmLevel::ALARM_LEVEL_TYPE_INVALID)
        {
            ui->level_combox->addItem(tr("全部"), type);
            continue;
        }

		AddTfEnumItemToComBoBox(ui->level_combox, type);
	}

	// 连接信号槽
	connect(ui->query_btn, &QPushButton::clicked, this, &QFaultLogWidget::OnFaultLogQueryBtn);
    connect(ui->reset_btn, &QPushButton::clicked, this, &QFaultLogWidget::ResetWidget);
	connect(this, SIGNAL(QueryFinished()), this, SLOT(OnQueryFinished()));
	connect(m_pGetBufferTimer, SIGNAL(timeout()), this, SLOT(GetBuffer()));	
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QFaultLogWidget::OnCurrentRowChanged);
    connect(ui->expand_btn, &QPushButton::clicked, this, [&]
    {
        bool status = ui->expand_btn->property("isHide").toBool();
        SetCtrlPropertyAndUpdateQss(ui->expand_btn, "isHide", !status);
		int iWidth = ui->widget_3->width();
        ui->widget_3->setVisible(status);
		// 调整报警名称栏宽度 add by chenjianlin 20230815
		ui->tableView->setColumnWidth(FaultLogTableModel::FAULT_LOG_LIST_COL::NAME,
			status ? ui->tableView->columnWidth(FaultLogTableModel::FAULT_LOG_LIST_COL::NAME) - iWidth : ui->tableView->columnWidth(FaultLogTableModel::FAULT_LOG_LIST_COL::NAME) + iWidth);
    });

	// 绑定起止日期编辑框(间隔时间不超过2个月)
	BindDateEdit(ui->lower_date_edit, ui->upper_date_edit, 2);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

FaultLogTableModel::StFaultLogItem QFaultLogWidget::AlarmDescDecode(::tf::AlarmDesc alarmDesc)
{
	FaultLogTableModel::StFaultLogItem item;
	
	// 报警码
	item.code = UiCommon::AlarmCodeToString(alarmDesc.mainCode, alarmDesc.middleCode, alarmDesc.subCode);
	
	// 报警等级
	item.level = ConvertTfEnumToQString(alarmDesc.alarmLevel);

	// 设备信息
	if (alarmDesc.deviceSN == CONTROL_UNIT_NAME)
	{
		// 系统
		item.system = tr("控制单元");
		// 模块
		item.model = tr("控制单元");
	}
	else
	{
        auto devInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(alarmDesc.deviceSN);
        if (devInfo != nullptr)
        {
            // 系统
            item.system = ConvertTfEnumToQString(devInfo->deviceClassify);
            // 模块
            item.model = QString::fromStdString(devInfo->groupName + devInfo->name);           

            if (!alarmDesc.subModuleName.empty())
            {
                item.model += QString::fromStdString("-" + alarmDesc.subModuleName);
            }
        }
    }

	// 时间
	item.time = QString::fromStdString(PosixTimeToTimeString(boost::posix_time::from_iso_string(alarmDesc.alarmTime)));
	// 描述
	item.name = QString::fromStdString(alarmDesc.alarmName);
	// 报警详情
	item.detail = QString::fromStdString(alarmDesc.alarmDetail);
	// 解决方案
	item.solution = QString::fromStdString(alarmDesc.customSolution);
    item.managerSolution = QString::fromStdString(alarmDesc.managerSolution);

	return item;
}

void QFaultLogWidget::OnFaultLogQueryBtn()
{
    //清空详情
    ui->detail_label->clear();
    ui->solution_label->clear();

	if (!ui->alarm_code_edit->text().isEmpty()
		&& ui->alarm_code_edit->text().split("-").size() != 3)
	{
		TipDlg(tr("报警代码格式错误")).exec();
		return;
	}

	QDateTime startTime(ui->lower_date_edit->date(), QTime(0,0,0));
	QDateTime endTime(ui->upper_date_edit->date(), QTime(0,0,0));

	if (startTime > endTime)
	{
		TipDlg(tr("值错误"), tr("起始日期不能大于截止日期！")).exec();
		return;
	}

	if (startTime.addMonths(2) < endTime)
	{
		TipDlg(tr("日志查找最大时间跨度不得超过两个月")).exec();
		return;
	}

    if (startTime > endTime)
    {
        TipDlg(tr("报警的开始时间不能小于结束时间")).exec();
        return;
    }

    FaultLogTableModel::Instance().RemoveAllRows();
    update();

	ui->query_btn->setEnabled(false);
    m_vecAlarmDescs.clear();

    // 设置线程停止标志
    m_ambNeedStop = false;

	// 启动查询线程
	m_pQryTrd = std::make_shared<std::thread>(std::bind(&QFaultLogWidget::FaultLogQuery, this));
	m_pQryTrd->detach();
	
	// 启动读取缓存定时器
	m_pGetBufferTimer->start();
}

void QFaultLogWidget::FaultLogQuery()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	try
	{
		::tf::AlarmInfoQueryCond qryCond;
		::tf::AlarmInfoQueryResp qryResp;

        // 时间
        ::tf::TimeRangeCond temp;
        temp.startTime = ui->lower_date_edit->date().toString("yyyy-MM-dd").toStdString() + " 00:00:00";
        temp.endTime = ui->upper_date_edit->date().toString("yyyy-MM-dd").toStdString() + " 23:59:59";
        qryCond.__set_alarmTime(temp);

        // 报警码
        QStringList tempList = ui->alarm_code_edit->text().split("-");
        if (tempList.size() == 3)
        {
            qryCond.__set_mainCode(tempList[0].toInt());
            qryCond.__set_middleCode(tempList[1].toInt());
            qryCond.__set_subCode(tempList[2].toInt());
        }

        // 报警级别
        if (ui->level_combox->currentIndex() > 0)
        {
            qryCond.__set_alarmLevel((tf::AlarmLevel::type)ui->level_combox->currentIndex());
        }

        // 模块
        if (ui->model_cobox->currentIndex() != 0)
        {
            qryCond.__set_devSn(ui->model_cobox->currentData().toString().toStdString());
        }

        const auto& dcp = DcsControlProxy::GetInstance();

        // 获取数据库的最大主键
        int64_t maxDbId = 0;
        {
            ::tf::AlarmInfoQueryCond qc(qryCond);
            ::tf::AlarmInfoQueryResp qr;

            // id降序
            ::tf::OrderByCond orderCnd;
            orderCnd.__set_asc(false);
            orderCnd.__set_fieldName("id");
            qc.orderByConds.push_back(orderCnd);

            // 只查一条
            ::tf::LimitCond lc;
            lc.__set_offset(0);
            lc.__set_count(1);
            qc.__set_limit(std::move(lc));

            // 执行查询条件
            if (!dcp->QueryAlarmInfo(qryResp, qc) 
                || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
                || qryResp.lstAlarmDescs.empty())
            {
                // 查询失败
                ULOG(LOG_ERROR, "Query AlarmInfo Max Id Failed");

                // 发送异步查询完成信号
                emit QueryFinished();

                return;
            }

            maxDbId = qryResp.lstAlarmDescs[0].id;
        }

        int allResultCount = dcp->QueryAlarmInfoCount(qryCond);
		FaultLogTableModel::Instance().ReserveDataSize(allResultCount);

		// id范围,解决深度分页问题
		int resultCount = 0;
		tf::IdRangeCond idRangeCond;
		idRangeCond.startId = maxDbId - QUERY_ID_RANG;
		idRangeCond.endId = maxDbId;
		qryCond.__set_idRangeCond(idRangeCond);

		do
		{
			// 执行查询条件
            qryResp.lstAlarmDescs.clear();
            qryResp.result = ::tf::ThriftResult::THRIFT_RESULT_UNKNOWN_ERROR;
			if (!dcp->QueryAlarmInfo(qryResp, qryCond) || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
			{
				// 查询失败
				ULOG(LOG_ERROR, "FaultLogQuery QueryAlarmInfo() failed");
				break;
			}

			// 统计结果数量
			resultCount += qryResp.lstAlarmDescs.size();

			// id范围
            idRangeCond.endId = idRangeCond.startId - 1;
			idRangeCond.startId = idRangeCond.endId - QUERY_ID_RANG;


			qryCond.__set_idRangeCond(idRangeCond);

			// 查询完成
			if (qryResp.lstAlarmDescs.empty() && (resultCount >= allResultCount))
			{
				ULOG(LOG_INFO, "FaultLogQuery QueryAlarmInfo() Finished");
				break;
			}

			// 往缓存里写数据
			{
				std::unique_lock<std::mutex> buffLock(s_mtxAlarmData);

				// 如果缓存已满，则等待
				if (m_vecAlarmDescs.size() >= QUERY_BUFF_SIZE)
				{
					m_condPutBuffer.wait(buffLock);
				}

				m_vecAlarmDescs.insert(m_vecAlarmDescs.end(), qryResp.lstAlarmDescs.begin(), qryResp.lstAlarmDescs.end());
			}

		} while (!m_ambNeedStop);
	}
	catch (...)
	{
		// 退出线程
		ULOG(LOG_INFO, "FaultLogQuery exec this thread %s()", __FUNCTION__);
	}

	// 发送异步查询完成信号
	emit QueryFinished();
}

void QFaultLogWidget::OnQueryFinished()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 最后一次获取缓存中数据
	GetBuffer();

	ui->query_btn->setEnabled(true);

    // 停止读取缓存定时器
    m_pGetBufferTimer->stop();
    m_ambNeedStop = false;
}

void QFaultLogWidget::GetBuffer()
{
	ULOG(LOG_INFO, "%s() ", __FUNCTION__);

    // 如果是手动停止查询线程，则不处理结果
    if (m_ambNeedStop)
    {
        m_vecAlarmDescs.clear();
        ui->query_btn->setEnabled(true);
        return;
    }

	// 从缓存里取数据
	std::vector<::tf::AlarmDesc> vAlarmDescBuffer;
	{
		std::unique_lock<std::mutex> buffLock(s_mtxAlarmData);
		// 如果缓存已空，则返回
		if (m_vecAlarmDescs.empty())
		{
			return;
		}

		vAlarmDescBuffer.swap(m_vecAlarmDescs);
	}

	// 通知生产者往缓存中放数据
	m_condPutBuffer.notify_one();

	for (auto data : vAlarmDescBuffer)
	{
		// 加载告警日志到表格
		FaultLogTableModel::Instance().AppendData(AlarmDescDecode(data));
	}
}

void QFaultLogWidget::OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    FaultLogTableModel::StFaultLogItem data;
    if (FaultLogTableModel::Instance().GetRowData(m_sortModel->mapToSource(current).row(), data))
    {
        ui->detail_label->setText(data.detail);
        ui->solution_label->setText(m_isManager ? data.managerSolution : data.solution);
    }
}

void QFaultLogWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    const auto spUim = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if (spUim == nullptr)
    {
        ULOG(LOG_ERROR, "Get login user info failed !");
    }

    m_isManager = spUim->type >= ::tf::UserType::USER_TYPE_ENGINEER;

    OnCurrentRowChanged(ui->tableView->currentIndex(), ui->tableView->currentIndex());
}
