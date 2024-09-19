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
/// @file     QMaintainLog.h
/// @brief 	 应用->日志->维护日志界面
///
/// @author   7656/zhang.changjiang
/// @date      2022年8月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年8月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QMaintainLog.h"
#include "ui_QMaintainLog.h"

#include <thread>
#include <QDate>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include "../thrift/DcsControlProxy.h"
#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/FileExporter.h"
#include "shared/datetimefmttool.h"

#include "model/MaintainGroupModel.h"
#include "model/MaintainLogModel.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#include "Serialize.h"
#include "printcom.hpp"

#define YEAR_OFFSET 5 // 年份偏移量（从当前时间往前五年）

QMaintainLog::QMaintainLog(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::QMaintainLog();
	ui->setupUi(this);
	Init();
}

QMaintainLog::~QMaintainLog()
{
}

QPushButton* QMaintainLog::GetCloseBtnPtr()
{
    return ui->close_btn;
}

///
///  @brief
///		初始化
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月2日，新建函数
///
void QMaintainLog::Init()
{
	SetTitleName(tr("维护日志"));
	// 监听维护组阶段更新
	REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, this, OnMaintainGrpPhaseUpdate);

	// 获取DCS客户端对象
	std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
	if (spCilent == nullptr)
	{
		return;
	}
	// 初始化维护日志模型
	m_pMaintainLogModel = &MaintainLogModel::Instance();
	ui->tableView_maintain_date->setModel(m_pMaintainLogModel);
	ui->tableView_maintain_date->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableView_maintain_date->verticalHeader()->setMinimumWidth(206);
	ui->tableView_maintain_date->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	ui->tableView_maintain_date->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
	ui->tableView_maintain_date->verticalHeader()->setMinimumHeight(43);
	ui->tableView_maintain_date->verticalHeader()->setTextElideMode(Qt::ElideRight);
	ui->tableView_maintain_date->verticalHeader()->setMouseTracking(true);
	ui->tableView_maintain_date->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView_maintain_date->setItemDelegate(new CenteredDelegate());
	ui->tableView_maintain_date->setShowGrid(true);
	connect(ui->tableView_maintain_date->verticalHeader(), &QHeaderView::sectionPressed, [&](int logicalIndex) {
		QHeaderView *header = ui->tableView_maintain_date->verticalHeader();
		if (header == nullptr)
		{
			return;
		}

		QString fullText = header->model()->headerData(logicalIndex, Qt::Vertical, Qt::DisplayRole).toString();
		QToolTip::showText(QCursor::pos(), fullText);
	});

	// 查询所有设备信息(初始化设备下拉列表)
	::tf::DeviceInfoQueryResp retDevs;
	::tf::DeviceInfoQueryCond dIqcDevs;
	if (!spCilent->QueryDeviceInfo(retDevs, dIqcDevs))
	{
		return;
	}
	m_pMaintainLogModel->SetDeviceList(retDevs.lstDeviceInfos);

	// 根据设备列表创建单选框
	QHBoxLayout* pHlayout = new QHBoxLayout(ui->devListFrame);
	pHlayout->setMargin(0);
	pHlayout->setSpacing(30);
	pHlayout->addStretch(1);
	m_curDeviceSN = retDevs.lstDeviceInfos.front().deviceSN;
	QVector<QRadioButton*> vecRBtn;

	bool isPipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();
	// 依次添加具体设备
	for (const auto& dev : retDevs.lstDeviceInfos)
	{
		if (dev.deviceType == ::tf::DeviceType::DEVICE_TYPE_INVALID)
		{
			continue;
		}

		if (dev.deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK && !isPipeLine)
		{
			continue;
		}

		QRadioButton* pRBtn;
		std::string devName = dev.groupName.empty() ? dev.name : dev.groupName + dev.name;
		pRBtn = new QRadioButton(QString::fromStdString(devName));
		connect(pRBtn, SIGNAL(clicked()), this, SLOT(OnDevRBtnClicked()));
		pHlayout->addWidget(pRBtn);
		m_devNameSNMap[devName] = dev.deviceSN;
		vecRBtn.push_back(pRBtn);
	}

	// 连接信号槽
	ConnectSigAndSlots();

	// 初始化UI数据
	InitUIData();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	// 默认查询当前日期
	if (!vecRBtn.isEmpty())
	{
		vecRBtn.first()->clicked();
		vecRBtn.first()->setChecked(true);
	}

	// 如果只有一个设备，则隐藏设备列表
	if (vecRBtn.size() == 1)
	{
		ui->devListFrame->hide();
	}
}

///
///  @brief 连接信号槽
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainLog::ConnectSigAndSlots()
{
	// 年下拉列表
	connect(ui->comboBox_year, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), [=](const QString &text) { OnComboBoxYearCurrentIndexChanged(text); });
	// 月下拉列表
	connect(ui->comboBox_mon, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) { OnComboBoxMonCurrentIndexChanged(index); });
	// 上一月按钮
	connect(ui->last_mon_btn, &QPushButton::clicked, this, &QMaintainLog::OnLastMonBtnClicked);
	// 下一月按钮
	connect(ui->next_mon_btn, &QPushButton::clicked, this, &QMaintainLog::OnNextMonBtnClicked);
	// 打印按钮
	connect(ui->print_maintain_btn, &QPushButton::clicked, this, &QMaintainLog::OnPrintBtnClicked);
	// 输出按钮
	connect(ui->output_btn, &QPushButton::clicked, this, &QMaintainLog::OnOutputBtnClicked);
	// 关闭按钮
	connect(ui->close_btn, &QPushButton::clicked, this, &QMaintainLog::OnCloseBtnClicked);
    // 维护日期改变
	connect(ui->tableView_maintain_date, &QTableView::clicked, this, [&](const QModelIndex & index) 
    {
        if (!index.isValid())
        {
            return;
        }
		::tf::MaintainGroup group;
		if (!m_pMaintainLogModel->GetGroupByRow(group, index.row()))
		{
			return;
		}
		

		QueryMaintainLog(ui->comboBox_year->currentText().toInt(), ui->comboBox_mon->currentIndex() + 1, index.column() + 1, m_curDeviceSN, group);
    });

    connect(this, SIGNAL(hint(QString)), this, SLOT(OnHint(QString)));
}

///
///  @brief	查询维护一月之内日志
///
///
///  @param[in]   year		年
///  @param[in]   mon		月
///  @param[in]   deviceSN  设备编号
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void QMaintainLog::QueryMaintainLog(int year, int mon, std::string deviceSN)
{
	// 获取DCS客户端对象
	std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
	if (spCilent == nullptr)
	{
		return;
	}

	// 创建维护日志查询条件（查询一个月内的维护记录）
	::tf::MaintainLogQueryResp retLog;
	::tf::MaintainLogQueryCond mlqcLog;
	mlqcLog.__set_deviceSN(deviceSN);
	QDateTime dateTime = QDateTime::fromString(QString("%1-%2-01 00:00:00").arg(year).arg(mon, 2, 10, QChar('0')), "yyyy-MM-dd HH:mm:ss");
	::tf::TimeRangeCond exeTime;
	exeTime.__set_startTime(std::string(QString("%1-%2-01 00:00:00").arg(year).arg(mon, 2, 10, QChar('0')).toLocal8Bit()));
	exeTime.__set_endTime(std::string(QString("%1-%2-%3 23:59:59").arg(year).arg(mon, 2, 10, QChar('0')).arg(dateTime.date().daysInMonth()).toLocal8Bit()));
	mlqcLog.__set_exeTime(exeTime);
	if (!spCilent->QueryMaintainLog(retLog, mlqcLog))
	{
		return;
	}

	// 清除表格和文本框中的数据
	ui->textEdit_maintain_result->clear();
	m_pMaintainLogModel->ClearData();
	m_pMaintainLogModel->SetData(retLog.lstMaintainLogs);
}

///
///  @brief	查询维护一天之内的日志
///
///
///  @param[in]   year		年
///  @param[in]   mon		月
///  @param[in]   day		日
///  @param[in]   deviceSN  设备编号
///  @param[in]   group     维护组
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void QMaintainLog::QueryMaintainLog(int year, int mon, int day, std::string deviceSN, ::tf::MaintainGroup& group)
{
	// 检查参数是否合法
	if (mon < 0 || year < 0 || day < 0 || deviceSN.empty())
	{
		return;
	}

    // 获取DCS客户端对象
    std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
    if (spCilent == nullptr)
    {
        return;
    }

    // 创建维护日志查询条件（查询一天之内的维护记录）
    ::tf::MaintainLogQueryResp retLog;
    ::tf::MaintainLogQueryCond mlqcLog;
    mlqcLog.__set_deviceSN(deviceSN);

	// 如果是其他维护组
	if (group.__isset.id && group.groupType != ::tf::MaintainGroupType::MAINTAIN_GROUP_OTHER)
	{
		mlqcLog.__set_groupId(group.id);
	}
    ::tf::TimeRangeCond exeTime;
    exeTime.__set_startTime(std::string(QString("%1-%2-%3 00:00:00").arg(year).arg(mon, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0')).toLocal8Bit()));
    exeTime.__set_endTime(std::string(QString("%1-%2-%3 23:59:59").arg(year).arg(mon, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0')).toLocal8Bit()));
    mlqcLog.__set_exeTime(exeTime);
    if (!spCilent->QueryMaintainLog(retLog, mlqcLog))
    {
        return;
    }

	// 如果维护组是其他，删除系统维护组日志
	if (group.groupType == ::tf::MaintainGroupType::MAINTAIN_GROUP_OTHER)
	{
		RemoveSystemLog(retLog.lstMaintainLogs);
	}

	// 显示维护详情
	ShowMaintainDetail(retLog.lstMaintainLogs);
}

///
///  @brief 显示维护详情
///
///
///  @param[in]   lstMaintainLogs  维护组日志列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月23日，新建函数
///
void QMaintainLog::ShowMaintainDetail(const std::vector<::tf::MaintainLog>& lstMaintainLogs)
{
	// 清除表格和文本框中的数据
	ui->textEdit_maintain_result->clear();

	// 遍历查询结果
	for (auto & log : lstMaintainLogs)
	{
		// 将维护详细信息写入详细信息文本框
		QString strResult;
		strResult += QString::fromStdString(CommonInformationManager::GetDeviceName(log.deviceSN)) + ": ";

		::tf::MaintainGroup group;
		if (MaintainGroupModel::Instance().GetGroupById(group, log.groupId))
		{
			strResult += group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE ? "" : MaintainGroupModel::GetGroupName(group) + ":";
		}

		for (int i = 0; i < log.resultDetail.size(); ++i)
		{
			QString str;
			if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE || log.resultDetail.size() == 1)
			{
				str = QString("%1;").arg(ConvertTfEnumToQString(log.resultDetail[i].itemType));
			}
			else
			{
				str = QString("(%1)%2;").arg(i + 1).arg(ConvertTfEnumToQString(log.resultDetail[i].itemType));

				if (log.resultDetail[i].result == ::tf::MaintainResult::type::MAINTAIN_RESULT_SUCCESS)
				{
					str += tr("成功;");
				}

				if (log.resultDetail[i].result == ::tf::MaintainResult::type::MAINTAIN_RESULT_FAIL)
				{
					str += tr("失败;");
					str = QString("<font color = 'red'>%1</font>").arg(str);
				}

				if (log.resultDetail[i].result == ::tf::MaintainResult::type::MAINTAIN_RESULT_UNKNOWN)
				{
					str += tr("等待;");
					str = QString("<font color = 'red'>%1</font>").arg(str);
				}
			}

			strResult.append(str);
		}

		strResult.append(log.exeResult != ::tf::MaintainResult::type::MAINTAIN_RESULT_SUCCESS ? tr("执行失败;") : tr("执行成功;"));
		strResult.append(QString(tr("%1;")).arg(ToCfgFmtDateTime(QString::fromStdString(log.exeTime))));
		strResult.append(QString(tr("<font color = 'red'>%1</font>"))
			.arg(log.exeResult != ::tf::MaintainResult::type::MAINTAIN_RESULT_SUCCESS 
				? (log.failCause == ::tf::MaintainFailCause::CASE_PROCESS_NORMAL ? "" : UiCommon::Instance()->ConvertMaintainFailCauseToString(log.failCause))
				: ""));
		ui->textEdit_maintain_result->append(((group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE 
			|| log.resultDetail.size() == 1) && (log.exeResult != ::tf::MaintainResult::type::MAINTAIN_RESULT_SUCCESS)) 
			? QString("<font color = 'red'>%1</font>").arg(strResult) 
			: strResult);
	}
}

///
///  @brief	设置垂直表头
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainLog::SetVerHeader()
{
	// 获取DCS客户端对象
	std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
	if (spCilent == nullptr)
	{
		return;
	}

	// 查询所有的维护组
	::tf::MaintainGroupQueryResp retGroup;
	::tf::MaintainGroupQueryCond miqcGroup;
	if (!spCilent->QueryMaintainGroup(retGroup, miqcGroup))
	{
		return;
	}

	m_pMaintainLogModel->SetVorList(retGroup.lstMaintainGroups);
}

///
///  @brief	设置水平表头
///
///
///  @param[in]   year  年
///  @param[in]   mon	月
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainLog::SetHorHeader(int year, int mon)
{
	QDate date = QDate::fromString(QString("%1-%2-01").arg(year).arg(mon, 2, 10, QChar('0')), "yyyy-MM-dd");
	QVector<int> horList;
	for (int i = 0; i < date.daysInMonth(); ++i)
	{
		horList.append(i + 1);
	}
	m_pMaintainLogModel->SetHorList(horList);
}

///
/// @brief
///     维护组阶段更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enPhaseType     阶段类型
///
/// @par History:
///  @li 7656/zhang.changjiang，2024年2月23日，新建函数
///
void QMaintainLog::OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType)
{
	// 如果有维护组执行完成，无论成功或者失败，刷新一下界面，防止数据不同步
	if ((enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS) || (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_FAIL))
	{
		QModelIndexList selectedIndexs = ui->tableView_maintain_date->selectionModel()->selectedIndexes();

		int year = ui->comboBox_year->currentText().toInt();
		int mon = ui->comboBox_mon->currentIndex() + 1;
		SetHorHeader(year, mon);
		QueryMaintainLog(year, mon, m_curDeviceSN);

		if (selectedIndexs.isEmpty())
		{
			return;
		}

		auto index = selectedIndexs.first();
		if (!index.isValid())
		{
			return;
		}

		::tf::MaintainGroup group;
		if (!m_pMaintainLogModel->GetGroupByRow(group, index.row()))
		{
			return;
		}

		int day = index.column() + 1;
		QueryMaintainLog(year, mon, day, m_curDeviceSN, group);

		ui->tableView_maintain_date->selectionModel()->select(index, QItemSelectionModel::Select);
	}
}

///
///  @brief
///
///
///  @param[in]   ml  更新维护日志
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月18日，新建函数
///
void QMaintainLog::UpdateMaintianLog(tf::MaintainLog ml)
{
	// TODO
}

///
///     设备单选框被点击
///
void QMaintainLog::OnDevRBtnClicked()
{
	// 获取信号发送者
	QRadioButton* pRBtn = qobject_cast<QRadioButton*>(sender());
	if (pRBtn == Q_NULLPTR)
	{
		return;
	}

	m_curDeviceSN = m_devNameSNMap[std::string(pRBtn->text().toLocal8Bit())];
	int year = ui->comboBox_year->currentText().toInt();
	int mon = ui->comboBox_mon->currentIndex() + 1;
	QueryMaintainLog(year, mon, m_curDeviceSN);
}

///
///  @brief	选择年份下拉列表
///
///
///  @param[in]   value  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void QMaintainLog::OnComboBoxYearCurrentIndexChanged(QString value)
{
	int year = value.toInt();
	int selectMonth = ui->comboBox_mon->currentIndex() + 1;
	// 需要更新当前月份选择框
	UpdateMonthCombxByYear(year, selectMonth);
	
	// 更新界面
	UpdateUIByDate();
}

///
///  @brief	选择月份下拉列表
///
///
///  @param[in]   value  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void QMaintainLog::OnComboBoxMonCurrentIndexChanged(int value)
{
	// 更新界面
	UpdateUIByDate();
}

///
///  @brief	上一月按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void QMaintainLog::OnLastMonBtnClicked()
{
	// 需要修改年份
	if (ui->comboBox_mon->currentIndex() - 1 < 0)
	{
		if (ui->comboBox_year->currentIndex() - 1 >= 0 )
		{
			//更新年份
			ui->comboBox_year->blockSignals(true);
			ui->comboBox_year->setCurrentIndex(ui->comboBox_year->currentIndex() - 1);
			ui->comboBox_year->blockSignals(false);

			// 更新月份选择框
			int year = ui->comboBox_year->currentText().toInt();
			UpdateMonthCombxByYear(year, 12);
		}
	}
	else
	{
		// 正常跳转
		ui->comboBox_mon->blockSignals(true);
		ui->comboBox_mon->setCurrentIndex(ui->comboBox_mon->currentIndex() - 1);
		ui->comboBox_mon->blockSignals(false);
	}

	// 更新界面
	UpdateUIByDate();
}

///
///  @brief 下一月按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void QMaintainLog::OnNextMonBtnClicked()
{
	// 需要修改年份
	if (ui->comboBox_mon->currentIndex() + 1 >= 12)
	{
		if (ui->comboBox_year->currentIndex() + 1 < YEAR_OFFSET)
		{
			//更新年份
			ui->comboBox_year->blockSignals(true);
			ui->comboBox_year->setCurrentIndex(ui->comboBox_year->currentIndex() + 1);
			ui->comboBox_year->blockSignals(false);

			// 更新月份选择框
			int year = ui->comboBox_year->currentText().toInt();
			UpdateMonthCombxByYear(year, 1);
		}
	}
	else
	{
		// 正常跳转
		ui->comboBox_mon->blockSignals(true);
		ui->comboBox_mon->setCurrentIndex(ui->comboBox_mon->currentIndex() + 1);
		ui->comboBox_mon->blockSignals(false);
	}

	// 更新界面
	UpdateUIByDate();
}

///
///  @brief 打印按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///  @li 6889/ChenWei，2023年3月31日，修改函数
///
void QMaintainLog::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 耗时操作放线程处理
    std::thread t([this]() {
        ULOG(LOG_INFO, "%s()", __FUNCTION__);

        // 打开进度条
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
        MaintenanceLog Info;
        CreatePrintData(Info);
        if (Info.vecRecord.empty())
        {
            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
            emit hint(tr("没有可打印的数据！"));
            return;
        }

        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strInstrumentModel = "i 6000";
        Info.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        int irect = printcom::printcom_async_print(strInfo, unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

        // 弹框提示打印结果
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(tr("数据已发送到打印机！"));
    });
    t.detach();
}

///
///  @brief	导出按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///  @li 6889/ChenWei，2023年3月31日，修改函数
///
void QMaintainLog::OnOutputBtnClicked()
{
	// 先检查是否有数据
    if (m_pMaintainLogModel->GetCurShowMaintainLog().empty())
    {
		emit hint(tr("没有可导出的数据！"));
		return;
    }

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        //ALOGI << "Export canceled!";
        return;
    }

    // 耗时操作放线程处理
    std::thread t([this, strFilepath]() {
        ULOG(LOG_INFO, "%s()", __FUNCTION__);

        // 打开进度条
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
        MaintenanceLog vecInfo;
        CreatePrintData(vecInfo);
        bool bRect = false;
        QFileInfo FileInfo(strFilepath);
        QString strSuffix = FileInfo.suffix();
        if (strSuffix == "pdf")
        {
            QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
            vecInfo.strPrintTime = strPrintTime.toStdString();
            std::string strInfo = GetJsonString(vecInfo);
            ULOG(LOG_INFO, "Print datas : %s", strInfo);
            std::string unique_id;
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportMaintenanceLog.lrxml";
            int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            bRect = true;
        }
        else
        {
            std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
            bRect = pFileEpt->ExportMaintainLog(vecInfo, strFilepath);
        }

        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
    });
    t.detach();
    
}

///
///  @brief 关闭按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月6日，新建函数
///
void QMaintainLog::OnCloseBtnClicked()
{
	this->close();
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年8月7日，新建函数
///
void QMaintainLog::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    userPms->IsPermisson(PSM_IM_EXPORT_MANT_LOG) ? ui->output_btn->show() : ui->output_btn->hide();
    userPms->IsPermisson(PSM_IM_PRINTT_MAIN_LOG) ? ui->print_maintain_btn->show() : ui->print_maintain_btn->hide();
}

///
///  @brief 创建打印导出数据
///
///
///  @param[in]   vecInfo  数据容器
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
void QMaintainLog::CreatePrintData(MaintenanceLog& Info)
{
    std::string strModuleName;
    for (std::map<std::string, std::string>::iterator iter = m_devNameSNMap.begin(); iter!= m_devNameSNMap.end(); iter++)
    {
        if (iter->second == m_curDeviceSN)
        {
            strModuleName = iter->first;
        }
    }

    int iIndex = 0;
	for (auto& log : m_pMaintainLogModel->GetCurShowMaintainLog())
	{
        for (auto detail : log.resultDetail)
		{
            MaintenanceItem item;
            item.strIndex = QString::number(++iIndex).toStdString();                                                           // 序号
			item.strGroupMaint = log.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE ? "/" : m_pMaintainLogModel->GetGroupNameById(log.groupId);// 维护组名称
			item.strSingleMaint = ConvertTfEnumToQString(detail.itemType).toStdString();				                       // 维护项目名称
			item.strMaintenanceStatus = ConvertTfEnumToQString(detail.result).toStdString(); 		                           // 维护状态
			item.strMaintenaceDate = detail.result == ::tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN ? "" : detail.exeTime;	   // 维护时间
            item.strModule = strModuleName;                                                                                    // 模块
			item.strUserName = log.user;																                       // 用户名

			Info.vecRecord.push_back(item);
		}
	}
}

///
///  @brief 删除系统维护日志
///
///
///  @param[in]   vecLogs  日志列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
void QMaintainLog::RemoveSystemLog(std::vector<tf::MaintainLog>& vecLogs)
{
	for (auto it = vecLogs.begin(); it != vecLogs.end();)
	{
		if (it->groupType != ::tf::MaintainGroupType::MAINTAIN_GROUP_OTHER &&
			/*it->groupType != ::tf::MaintainGroupType::MAINTAIN_GROUP_CUSTOM &&*/
			it->groupType != ::tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
		{
			it = vecLogs.erase(it);
		}
		else
		{
			++it;
		}
	}
}

///
/// @bref
///		响应提示信息信号
///
/// @par History:
/// @li 6889/ChenWei，2024年1月29日，新建函数
///
void QMaintainLog::OnHint(QString strInfo)
{
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(strInfo));
    pTipDlg->exec();
}

///
/// @brief 初始化界面数据
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
///
void QMaintainLog::InitUIData()
{
	// 初始化可选年份（默认日期是当前日期，年份下拉列表可选年份为当前年份上下5年）
	QDate date = QDate::currentDate();
	// 阻塞信号
	ui->comboBox_year->blockSignals(true);
	ui->comboBox_year->clear();
	for (int i = date.year() - YEAR_OFFSET; i < date.year(); ++i)
	{
		ui->comboBox_year->addItem(QString::number(i + 1));
	}
	ui->comboBox_year->setCurrentIndex(YEAR_OFFSET - 1);
	ui->comboBox_year->blockSignals(false);

	// 需要更新当前月份
	UpdateMonthCombxByYear(date.year(), date.month());
	
	// 设置维护日志表垂直表头（垂直表头是维护组名称）
	SetVerHeader();

	// 更新数据
	UpdateUIByDate();
}

///
/// @brief 通过年份更新日期
///
/// @param[in]  year  年份
/// @param[in]  oldSelectedMon  之前选择的日期
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
///
void QMaintainLog::UpdateMonthCombxByYear(const int& year, const int& oldSelectedMon /*= 1*/)
{
	auto currentDate = QDate::currentDate();

	// 获取月数
	int monCount = (year == currentDate.year()) ? currentDate.month() : 12;

	// 阻塞信号
	ui->comboBox_mon->blockSignals(true);
	// 清空
	ui->comboBox_mon->clear();
	for (int monIndex = 1; monIndex <= monCount; monIndex++)
	{
		ui->comboBox_mon->addItem(QString::number(monIndex) + tr("月"));
	}

	// 设置显示月份
	if (oldSelectedMon <= monCount)
	{
		ui->comboBox_mon->setCurrentIndex(oldSelectedMon - 1);
	}
	else
	{
		ui->comboBox_mon->setCurrentIndex(monCount - 1);
	}

	// 解除阻塞
	ui->comboBox_mon->blockSignals(false);

}

///
/// @brief 通过日期更新按钮状态
///
/// @param[in]  year  年份
/// @param[in]  month  日期
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
///
void QMaintainLog::UpdateBtnStatusByDate(const int& year, const int& month)
{
	// 下一个月按钮
	if (QDate::currentDate().year() == year &&
		QDate::currentDate().month() == month)
	{
		ui->next_mon_btn->setDisabled(true);
	}
	else
	{
		ui->next_mon_btn->setDisabled(false);
	}

	// 上一个月按钮
	if ((QDate::currentDate().year() - YEAR_OFFSET + 1) == year && 
		month == 1)
	{
		ui->last_mon_btn->setDisabled(true);
	}
	else
	{
		ui->last_mon_btn->setDisabled(false);
	}
	
}

///
/// @brief 通过界面日期更新UI
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月30日，新建函数
///
void QMaintainLog::UpdateUIByDate()
{
	int year = ui->comboBox_year->currentText().toInt();
	int month = ui->comboBox_mon->currentIndex() + 1;

	// 更新选择按钮状态
	UpdateBtnStatusByDate(year, month);

	// 更新表头
	SetHorHeader(year, month);
	// 查询并更新日志
	QueryMaintainLog(year, month, m_curDeviceSN);
}

