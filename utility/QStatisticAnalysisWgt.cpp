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
/// @file     QStatisticAnalysisWgt.h
/// @brief 	  统计分析界面
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <thread>
#include "QStatisticAnalysisWgt.h"
#include "ui_QStatisticAnalysisWgt.h"
#include <QFileDialog>

#include "src/common/Mlog/mlog.h"
#include "manager/UserInfoManager.h"
#include "shared/FileExporter.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"

#include "QSampleSizeStatistics.h"
#include "QResultStatistics.h"
#include "QConsumableStatistics.h"
#include "QCalibrationStatistics.h"
#include "Serialize.h"
#include "printcom.hpp"

// 页面索引
enum TAB_INDEX
{
	TAB_INDEX_SAMLPLE_SIZE_STATISTICS,						// 样本量统计
	TAB_INDEX_CONSUMABLE_STATISTICS,						// 试剂统计
	TAB_INDEX_CALIBRATION_STATISTICS,						// 校准统计
};
QStatisticAnalysisWgt::QStatisticAnalysisWgt(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::QStatisticAnalysisWgt();
	ui->setupUi(this);
    m_strFileName = "";
	Init();
}

QStatisticAnalysisWgt::~QStatisticAnalysisWgt()
{
	delete ui;
}

///
///  @brief 初始化
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月11日，新建函数
///
void QStatisticAnalysisWgt::Init()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_tabs.clear();

	// 插入项目设置tab页面
    TabInfo tbiSampledu(TAB_INDEX_SAMLPLE_SIZE_STATISTICS, new QSampleSizeStatistics(this), tr("样本量统计"));
    ui->tabWidget->insertTab(tbiSampledu.m_tbId, tbiSampledu.m_widget, tbiSampledu.m_Name);
    m_tabs.push_back(tbiSampledu);

    TabInfo tbiConsumb(TAB_INDEX_CONSUMABLE_STATISTICS, new QConsumableStatistics(this), tr("试剂统计"));
    ui->tabWidget->insertTab(tbiConsumb.m_tbId, tbiConsumb.m_widget, tbiConsumb.m_Name);
    m_tabs.push_back(tbiConsumb);

    TabInfo tbiCaliStat(TAB_INDEX_CALIBRATION_STATISTICS, new QCalibrationStatistics(this), tr("校准统计"));
    ui->tabWidget->insertTab(tbiCaliStat.m_tbId, tbiCaliStat.m_widget, tbiCaliStat.m_Name);
    m_tabs.push_back(tbiCaliStat);

	connect(ui->print_btn, &QPushButton::clicked, this, &QStatisticAnalysisWgt::OnPrintBtnClicked);
	connect(ui->export_btn, &QPushButton::clicked, this, &QStatisticAnalysisWgt::OnExportBtnClicked);
    connect(this, &QStatisticAnalysisWgt::hint, this, &QStatisticAnalysisWgt::OnHint);
    connect(this, &QStatisticAnalysisWgt::importFileName, this, &QStatisticAnalysisWgt::OnImportFileName);


    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
///  @brief 响应打印按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月11日，新建函数
///
void QStatisticAnalysisWgt::OnPrintBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	switch (ui->tabWidget->currentIndex())
	{
	case TAB_INDEX_SAMLPLE_SIZE_STATISTICS:
    {
        std::thread PrintThread([this]() 
        {
            // 打开进度条
            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
            QSampleSizeStatistics* pSampleSizeWdg = qobject_cast<QSampleSizeStatistics *>(ui->tabWidget->currentWidget());
            if (pSampleSizeWdg != Q_NULLPTR)
            {
                SampleStatisticsInfo Info;
                if (!pSampleSizeWdg->GetStatisticsInfo(Info))
                {
                    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
                    emit hint(tr("没有可打印的数据!"));
                    return;
                }

                std::string strInfo = GetJsonString(Info);
                ULOG(LOG_INFO, "Print datas : %s", strInfo);
                std::string unique_id;
                int irect = printcom::printcom_async_print(strInfo, unique_id);
                ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            }

            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                                 // 关闭进度条
            emit hint(tr("数据已发送到打印机!"));
        });
        PrintThread.detach();
    }
		break;
	case TAB_INDEX_CONSUMABLE_STATISTICS:
    {
        QConsumableStatistics* pConsumableWdg = qobject_cast<QConsumableStatistics *>(ui->tabWidget->currentWidget());
        if (pConsumableWdg != Q_NULLPTR)
        {
            ReagentStatisticsInfo Info;
            if (!pConsumableWdg->GetStatisticsInfo(Info))
            {
                // 弹框提示导出完成
                emit hint(tr("没有可打印的数据!"));
                return;
            }

            std::string strInfo = GetJsonString(Info);
            ULOG(LOG_INFO, "Print datas : %s", strInfo);
            std::string unique_id;
            int irect = printcom::printcom_async_print(strInfo, unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            emit hint(tr("数据已发送到打印机!"));
        }

    }
		break;
	case TAB_INDEX_CALIBRATION_STATISTICS:
    {
        std::thread PrintThread([this]() 
        {
            // 打开进度条
            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
            QCalibrationStatistics* pCalibrationWdg = qobject_cast<QCalibrationStatistics *>(ui->tabWidget->currentWidget());
            if (pCalibrationWdg != Q_NULLPTR)
            {
                CaliStatisticsInfo Info;
                if (!pCalibrationWdg->GetStatisticsInfo(Info))
                {
                    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
                    emit hint(tr("没有可打印的数据!"));
                    return;
                }

                std::string strInfo = GetJsonString(Info);
                ULOG(LOG_INFO, "Print datas : %s", strInfo);
                std::string unique_id;
                int irect = printcom::printcom_async_print(strInfo, unique_id);
                ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            }

            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                                 // 关闭进度条
            emit hint(tr("数据已发送到打印机!"));
        });
        PrintThread.detach();
    }
		break;
	default:
		break;
	}
}

///
///  @brief 响应导出按钮
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月11日，新建函数
///
void QStatisticAnalysisWgt::OnExportBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
    bool bRect = false;

	switch (ui->tabWidget->currentIndex())
	{
	case TAB_INDEX_SAMLPLE_SIZE_STATISTICS:
    {
        std::thread ExportThread([this]() 
        {
            QSampleSizeStatistics* pSampleSizeWdg = qobject_cast<QSampleSizeStatistics *>(ui->tabWidget->currentWidget());
            if (pSampleSizeWdg != Q_NULLPTR)
            {
                // 打开进度条
                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
                SampleStatisticsInfo Info;
                if (!pSampleSizeWdg->GetStatisticsInfo(Info))
                {
                    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
                    emit hint(tr("没有可导出的数据！"));
                    return;
                }

                emit importFileName();
                std::unique_lock<std::mutex> lck(m_mtx);
                m_CV.wait(lck);
                if (m_strFileName.isEmpty())
                {
                    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
                    return;
                }

                bool bRect = false;
                QFileInfo FileInfo(m_strFileName);
                QString strSuffix = FileInfo.suffix();
                if (strSuffix == "pdf")
                {
                    std::string strInfo = GetJsonString(Info);
                    ULOG(LOG_INFO, "Print datas : %s", strInfo);
                    std::string unique_id;
                    QString strDirPath = QCoreApplication::applicationDirPath();
                    QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportSampleStatisticsInfo.lrxml";
                    int irect = printcom::printcom_async_assign_export(strInfo, m_strFileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
                    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
                    bRect = true;
                }
                else
                {
                    std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
                    bRect = pFileEpt->ExportSampleStatisticsInfo(Info, m_strFileName);
                }

                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                                 // 关闭进度条
                m_strFileName = "";
                emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
            }
        });
        ExportThread.detach();
    }
		break;
	case TAB_INDEX_CONSUMABLE_STATISTICS:
    {
        QConsumableStatistics* pConsumableWdg = qobject_cast<QConsumableStatistics *>(ui->tabWidget->currentWidget());
        if (pConsumableWdg != Q_NULLPTR)
        {
            ReagentStatisticsInfo Info;
            if (!pConsumableWdg->GetStatisticsInfo(Info))
            {
                // 弹框提示导出完成
                emit hint(tr("没有可导出的数据！"));
                return;
            }

            m_strFileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
            if (m_strFileName.isEmpty())
            {
                //POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
                return;
            }

            bool bRect = false;
            QFileInfo FileInfo(m_strFileName);
            QString strSuffix = FileInfo.suffix();
            if (strSuffix == "pdf")
            {
                std::string strInfo = GetJsonString(Info);
                ULOG(LOG_INFO, "Print datas : %s", strInfo);
                std::string unique_id;
                QString strDirPath = QCoreApplication::applicationDirPath();
                QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportReagentStatisticsInfo.lrxml";
                int irect = printcom::printcom_async_assign_export(strInfo, m_strFileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
                ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
                bRect = true;
            }
            else
            {
                std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
                bRect = pFileEpt->ExportReagentStatisticsInfo(Info, m_strFileName);
            }

            m_strFileName = "";
            emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
        }
    }
		break;
	case TAB_INDEX_CALIBRATION_STATISTICS:
    {
        std::thread ExportThread([this]() 
        {
            QCalibrationStatistics* pCalibrationWdg = qobject_cast<QCalibrationStatistics *>(ui->tabWidget->currentWidget());
            if (pCalibrationWdg != Q_NULLPTR)
            {
                // 打开进度条
                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
                CaliStatisticsInfo Info;
                if (!pCalibrationWdg->GetStatisticsInfo(Info))
                {
                    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
                    emit hint(tr("没有可导出的数据！"));
                    return;
                }

                emit importFileName();
                std::unique_lock<std::mutex> lck(m_mtx);
                m_CV.wait(lck);
                if (m_strFileName.isEmpty())
                {
                    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
                    return;
                }

                bool bRect = false;
                QFileInfo FileInfo(m_strFileName);
                QString strSuffix = FileInfo.suffix();
                if (strSuffix == "pdf")
                {
                    std::string strInfo = GetJsonString(Info);
                    ULOG(LOG_INFO, "Print datas : %s", strInfo);
                    std::string unique_id;
                    QString strDirPath = QCoreApplication::applicationDirPath();
                    QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliStatisticsInfo.lrxml";
                    int irect = printcom::printcom_async_assign_export(strInfo, m_strFileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
                    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
                    bRect = true;
                }
                else
                {
                    std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
                    bRect = pFileEpt->ExportCaliStatisticsInfo(Info, m_strFileName);
                }

                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                                 // 关闭进度条
                emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
            }
        });
        ExportThread.detach();
    }
		break;
	default:
		break;
	}

}

///
/// @bref
///		权限变更响应
///
/// @par History:
/// @li 8276/huchunli, 2023年7月26日，新建函数
///
void QStatisticAnalysisWgt::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    // 全部tab都remove一遍
    while (ui->tabWidget->count() > 0)
    {
        ui->tabWidget->removeTab(0);
    }

    // 逐个判断添加
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    std::vector<TabInfo>::iterator it = m_tabs.begin();
    for (; it != m_tabs.end(); ++it)
    {
        switch (it->m_tbId)
        {
            // 样本量统计
        case TAB_INDEX_SAMLPLE_SIZE_STATISTICS:
            if (userPms->IsPermisson(PSM_IM_STATISTICS_SAMPLEVOL))
            {
                ui->tabWidget->addTab(it->m_widget, it->m_Name);
            }
            break;
            // 试剂统计
        case TAB_INDEX_CONSUMABLE_STATISTICS:
            if (userPms->IsPermisson(PSM_IM_STATISTICS_REAGENT))
            {
                ui->tabWidget->addTab(it->m_widget, it->m_Name);
            }
            break;
            // 校准统计
        case TAB_INDEX_CALIBRATION_STATISTICS:
            if (userPms->IsPermisson(PSM_IM_STATISTICS_CALI))
            {
                ui->tabWidget->addTab(it->m_widget, it->m_Name);
            }
            break;
        default:
            ULOG(LOG_WARN, "Unknown table type.");
            break;
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
void QStatisticAnalysisWgt::OnHint(QString strInfo)
{
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(strInfo));
    pTipDlg->exec();
}

///
/// @bref
///		响应提示信息信号
///
/// @par History:
/// @li 6889/ChenWei，2024年1月29日，新建函数
///
void QStatisticAnalysisWgt::OnImportFileName()
{
    // 弹出保存文件对话框
    m_strFileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));

    // 通知导出线程导出文件
    m_CV.notify_all();
}
