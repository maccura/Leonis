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
/// @file     QLogModule.cpp
/// @brief    日志功能展示
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

#include "QLogModule.h"
#include "ui_QLogModule.h"
#include <QGridLayout>
#include <QSignalMapper>
#include <QFileDialog>
#include <QDateTime>
#include "QOperationLogWgt.h"
#include "QConsumableChangeLogWgt.h"
#include "QFaultLogWidget.h"

#include "model/OperationLogModel.h"
#include "model/ConsumableChangeLogModel.h"

#include "shared/FileExporter.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#include "Serialize.h"
#include "printcom.hpp"

#define TAB_INDEX_OPERATE									(0)                       // 操作日志页面索引
#define TAB_INDEX_SUPPLIES									(1)                       // 试剂更换日志页面索引
#define TAB_INDEX_FAULT										(2)                       // 故障日志页面索引

QLogModule::QLogModule(QWidget *parent)
	: QWidget(parent),
      m_pFaultLogWidget(nullptr),
	  ui(new Ui::QLogModule)
{
	ui->setupUi(this);
	Init();
}

QLogModule::~QLogModule()
{

}

void QLogModule::StopQueryAlarmLog()
{
    m_pFaultLogWidget->StopQueryAlarmLog();
};

void QLogModule::Init()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 插入项目设置tab页面
	ui->tabWidget->insertTab(TAB_INDEX_OPERATE, new QOperationLogWgt(this), tr("操作日志"));
	ui->tabWidget->insertTab(TAB_INDEX_SUPPLIES, new QConsumableChangeLogWgt(this), tr("试剂/耗材更换日志"));

    m_pFaultLogWidget = new QFaultLogWidget(this);
	ui->tabWidget->insertTab(TAB_INDEX_FAULT, m_pFaultLogWidget, tr("报警日志"));

    QTabBar *tabBar = ui->tabWidget->findChild<QTabBar *>();
    if (tabBar)
    {
        QFontMetrics fm = tabBar->fontMetrics();
        int Width = 0;
        for (int i = 0; i < tabBar->count(); ++i)
        {
            int tabWidth = fm.width(tabBar->tabText(i));
            tabWidth > Width ? Width = tabWidth : Width = Width;
        }
        for (int i = 0; i < tabBar->count(); ++i)
        {
            QString tabText = tabBar->tabText(i);
            QString elidedText = fm.elidedText(tabText, Qt::ElideRight, 150);
            tabBar->setTabToolTip(i, tabText);
            tabBar->setTabText(i, elidedText);
        }
    }

    connect(ui->print_btn, &QPushButton::clicked, this, &QLogModule::OnPrintBtnClicked);
    connect(ui->export_btn, &QPushButton::clicked, this, &QLogModule::OnExportBtnClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &QLogModule::OnTabWidgetChangePage);
    connect(ui->flat_return, &QPushButton::clicked, this, [&]() {m_pFaultLogWidget->StopQueryAlarmLog(); });
    connect(this, SIGNAL(hint(QString)), this, SLOT(OnHint(QString)));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void QLogModule::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 打开界面默认显示操作日志
    ui->tabWidget->setCurrentIndex(0);
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (ui->tabWidget->currentIndex() == TAB_INDEX_OPERATE)
    {
        // 耗时操作放在线程里完成
        std::thread th(std::bind(&QLogModule::PrintOperationLog, this));
        th.detach();
    }
    else if (ui->tabWidget->currentIndex() == TAB_INDEX_SUPPLIES)
    {
        // 耗时操作放在线程里完成
        std::thread th(std::bind(&QLogModule::PrintChangeLog, this));
        th.detach();
    }
    else if (ui->tabWidget->currentIndex() == TAB_INDEX_FAULT)
    {
        // 耗时操作放在线程里完成
        std::thread th(std::bind(&QLogModule::PrintAlarmInfo, this));
        th.detach();
    }
}

///
/// @brief 打印报警信息
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::PrintAlarmInfo()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
    AlarmInfoLog Log;
    GetPrintAlarmInfo(Log);

    // 判断是否有数据需要打印
    if (Log.vecRecord.empty())
    {
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(tr("没有可以打印的数据!"));
        return;
    }

    QString strPrintTime = QDateTime::currentDateTime().toString(DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss");
    Log.strInstrumentModel = "i 6000";
    Log.strPrintTime = strPrintTime.toStdString();
    std::string strInfo = GetJsonString(Log);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
    emit hint(tr("数据已发送到打印机!"));
}

///
/// @brief 打印操作日志
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::PrintOperationLog()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
    OperationLog Log;
    GetPrintOperationLog(Log);

    // 判断是否有数据需要打印
    if (Log.vecRecord.empty())
    {
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(tr("没有可以打印的数据!"));
        return;
    }

    QString strPrintTime = QDateTime::currentDateTime().toString(DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss");
    Log.strInstrumentModel = "i 6000";
    Log.strPrintTime = strPrintTime.toStdString();
    std::string strInfo = GetJsonString(Log);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                                 // 关闭进度条
    emit hint(tr("数据已发送到打印机!"));
}

///
/// @brief 打印更换日志
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::PrintChangeLog()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
    ReagentChangeLog Log;
    GetPrintReagentSupplyChangeLog(Log);

    // 判断是否有数据需要打印
    if (Log.vecRecord.empty())
    {
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(tr("没有可以打印的数据!"));
        return;
    }

    QString strPrintTime = QDateTime::currentDateTime().toString(DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss");
    Log.strInstrumentModel = "i 6000";
    Log.strPrintTime = strPrintTime.toStdString();
    std::string strInfo = GetJsonString(Log);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                                 // 关闭进度条
    emit hint(tr("数据已发送到打印机!"));
}

///
/// @brief 响应导出按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 检查数据是否为空
	bool bDataIsEmpty = false;
	switch (ui->tabWidget->currentIndex())
	{
	case TAB_INDEX_OPERATE:
		bDataIsEmpty = OperationLogModel::Instance().GetData().empty();
		break;
	case TAB_INDEX_SUPPLIES:
		bDataIsEmpty = ConsumableChangeLogModel::Instance().GetData().empty();
		break;
	case TAB_INDEX_FAULT:
		bDataIsEmpty = FaultLogTableModel::Instance().GetData().empty();
		break;
	default:
		return;
	}

	if (bDataIsEmpty)
	{
		emit hint(tr("没有可以导出的数据!"));
		return;
	}

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        ULOG(LOG_INFO, "File path is empty!");
        return;
    }

    if (ui->tabWidget->currentIndex() == TAB_INDEX_OPERATE)
    {
        // 耗时操作放在线程里完成
        std::thread th(std::bind(&QLogModule::ExportOperationLog, this, strFilepath));
        th.detach();
    }
    else if (ui->tabWidget->currentIndex() == TAB_INDEX_SUPPLIES)
    {
        // 耗时操作放在线程里完成
        std::thread th(std::bind(&QLogModule::ExportChangeLog, this, strFilepath));
        th.detach();
    }
    else if (ui->tabWidget->currentIndex() == TAB_INDEX_FAULT)
    {
        // 耗时操作放在线程里完成
        std::thread th(std::bind(&QLogModule::ExportAlarmInfo, this, strFilepath));
        th.detach();
    }
}

///
/// @brief 导出报警信息
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::ExportAlarmInfo(QString strFilepath)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    AlarmInfoLog vecInfo;
    GetPrintAlarmInfo(vecInfo);

    // 判断是否有数据需要导出
    if (vecInfo.vecRecord.empty())
    {
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(tr("没有可以导出的数据!"));
        return;
    }

    bool bRect = false;
    QFileInfo FileInfo(strFilepath);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString(DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss");
        vecInfo.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(vecInfo);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportAlarmInfoLog.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportAlarmInfo(vecInfo, strFilepath);
    }
    
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
    emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
}

///
/// @brief 导出操作日志
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::ExportOperationLog(QString strFilepath)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    OperationLog OperationLogVec;
    GetPrintOperationLog(OperationLogVec);

    // 判断是否有数据需要导出
    if (OperationLogVec.vecRecord.empty())
    {
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(tr("没有可以导出的数据!"));
        return;
    }

    bool bRect = false;
    QFileInfo FileInfo(strFilepath);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString(DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss");
        OperationLogVec.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(OperationLogVec);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportOperationLog.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportOperationLog(OperationLogVec, strFilepath);
    }

    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
    emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
}

///
/// @brief 导出更换日志
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::ExportChangeLog(QString strFilepath)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 打开进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
    ReagentChangeLog ChangeLogVec;
    GetPrintReagentSupplyChangeLog(ChangeLogVec);

    // 判断是否有数据需要导出
    if (ChangeLogVec.vecRecord.empty())
    {
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(tr("没有可以导出的数据!"));
        return;
    }

    bool bRect = false;
    QFileInfo FileInfo(strFilepath);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString(DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss");
        ChangeLogVec.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(ChangeLogVec);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportReagentChangeLog.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportChangeLog(ChangeLogVec, strFilepath);
    }

    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
    emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年9月25日，新建函数
///
void QLogModule::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    // 导出
    ui->export_btn->setVisible(userPms->IsPermisson(PSM_IM_EXPORT_OPERATORLOG));
}

void QLogModule::OnTabWidgetChangePage()
{
    if (ui->tabWidget->currentIndex() == TAB_INDEX_FAULT)
    {
        m_pFaultLogWidget->ResetWidget();
    }
    else
    {
        m_pFaultLogWidget->StopQueryAlarmLog();
    }
}

///
/// @brief 打印报警信息
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::GetPrintAlarmInfo(AlarmInfoLog& Log)
{
    QString format = DictionaryQueryManager::GetUIDateFormat();

	auto& faultLogLogs = FaultLogTableModel::Instance().GetData();
	for (int iIndex = 0; iIndex<faultLogLogs.size(); iIndex++)
    {
        AlarmInfo info;
        const FaultLogTableModel::StFaultLogItem& it = faultLogLogs.at(iIndex);
        info.strIndex = QString::number(iIndex + 1).toStdString();     // 序号
        info.strModule = it.model.toStdString();                     // 模块
        info.strAlarmCode = it.code.toStdString();                   // 报警码
        info.strAlarmLevel = it.level.toStdString();                 // 报警级别
        info.strAlarmName = it.name.toStdString();                   // 告警简述
        info.strAlarmTime = QDateTime::fromString(it.time, 
            "yyyy-MM-dd HH:mm:ss").toString(format).toStdString();   // 告警时间

        Log.vecRecord.push_back(info);
    }
}

///
/// @brief 打印操作日志
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::GetPrintOperationLog(OperationLog& Log)
{
    // 查询条件设置一次即可
	auto cmPtr = CommonInformationManager::GetInstance();
	if (cmPtr == nullptr)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("创建打印数据失败！")));
		pTipDlg->exec();
		return;
	}
	const auto& devInfos = cmPtr->GetDeviceMaps();

	const ::tf::OperationLogQueryCond &queryCond = OperationLogModel::Instance().GetCurQueryCond();

    // 填写操作记录
	const std::vector<::tf::OperationLog>& operationLogs = OperationLogModel::Instance().GetData();
    int iIndex = 0;
	for (auto &log : operationLogs)
    {
        OperationLogItem Record;
        Record.strIndex = QString::number(++iIndex).toStdString();
        Record.strUserName = log.user;																				// 操作者
        Record.strOperationType = OperationLogModel::Instance().GetOperationType(log.operationType).toStdString();	// 操作类型
        Record.strTime = log.operationTime;																			// 时间
        Record.strRecord = log.operationRecord;																		// 操作记录

        Log.vecRecord.push_back(Record);
    }
}

///
/// @brief 打印试剂/耗材更换日志
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QLogModule::GetPrintReagentSupplyChangeLog(ReagentChangeLog& Log)
{
	auto cmPtr = CommonInformationManager::GetInstance();
	if (cmPtr == nullptr)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("创建打印数据失败！")));
		pTipDlg->exec();
		return;
	}
    const auto& devInfos = cmPtr->GetDeviceMaps();
	const std::vector<::tf::ConsumableChangeLog>& consumableChangeLogs = ConsumableChangeLogModel::Instance().GetData();
    int iIndex = 0;
    for (auto &log : consumableChangeLogs)
    {
        ChangeRecord Record;
        Record.strIndex = QString::number(++iIndex).toStdString();
        Record.strModule = devInfos.count(log.deviceSN) > 0 ? devInfos.at(log.deviceSN)->name : "";// 模块
        Record.strName = log.consumablName;																		// 名称
        Record.strOperationType = ConsumableChangeLogModel::Instance().GetChangeAction(log.changeAction).toStdString();// 动作
        Record.strState = ConsumableChangeLogModel::Instance().GetChangeStatus(log.changeStatus).toStdString(); // 状态
        Record.strUserName = log.user;																			// 操作者
        Record.strLot = log.batchNum;					                                                        // 批号
        Record.strReagentSN = log.bottleNum;                                                                    // 瓶号
        Record.strTime = log.changeTime;																		// 时间
        Log.vecRecord.push_back(Record);
    }
}

///
/// @bref
///		响应提示信息信号
///
/// @par History:
/// @li 6889/ChenWei，2024年1月29日，新建函数
///
void QLogModule::OnHint(QString strInfo)
{
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(strInfo));
    pTipDlg->exec();
}

