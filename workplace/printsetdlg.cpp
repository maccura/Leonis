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
/// @file     printsetdlg.cpp
/// @brief    工作界面->数据浏览->打印设置
///
/// @author   6889/ChenWei
/// @date     2022年11月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2022年11月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "printsetdlg.h"
#include "ui_printsetdlg.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "messagebus.h"
#include "msgiddef.h"
#include "src/common/Mlog/mlog.h"
#include "printcom.hpp"
#include "Serialize.h"


std::mutex PrintSetDlg::m_mtx;

Q_DECLARE_METATYPE(int32_t)
PrintSetDlg::PrintSetDlg(bool bIsHisData, QWidget *parent)
    : BaseDlg(parent)
    , m_bInit(false)
	, m_bIsPrint(false)
    , m_bIsHisData(bIsHisData)
{
    ui = new Ui::PrintSetDlg();
    ui->setupUi(this);
    SetTitleName(tr("打印"));
    InitStrResource();
}

void PrintSetDlg::SetModel(PrintMode printMode)
{
    m_PrintMode = printMode;
    ChoosePrintMode();
}

PrintSetDlg::~PrintSetDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    m_CV.notify_all();
    delete ui;
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
void PrintSetDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief 显示后初始化
///     
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
void PrintSetDlg::InitAfterShow()
{
    InitConnect();
    InitChildWidget();
}

void PrintSetDlg::InitChildWidget()
{

}

void PrintSetDlg::InitStrResource()
{
    ui->FirstCheckCBox->setText(tr("结果"));
    ui->ReviewCBox->setText(tr("复查结果"));
    ui->SinglePrintCBox->setText(tr("单页打印"));
    ui->CombinePrintCBox->setText(tr("合并打印"));
    ui->OkBtn->setText(tr("打印"));
    ui->CancelBtn->setText(tr("取消"));
    ui->PreviewBtn->setText(tr("预览"));
}

void PrintSetDlg::InitConnect()
{
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnOKBtnClicked()));
	connect(ui->CancelBtn, &QPushButton::clicked, this, [&]() {m_bIsPrint = false; reject(); });
    connect(ui->PreviewBtn, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
    connect(this, SIGNAL(PagesCompleted()), this, SLOT(OpenPreviewWidget()));
}

void PrintSetDlg::ChoosePrintMode()
{
    if (m_PrintMode == PrintMode::PRINTBYSAMPLE)
        ui->stackedWidget->setCurrentIndex(0);
    else if (m_PrintMode == PrintMode::PRINTBYITEM)
        ui->stackedWidget->setCurrentIndex(1);
}

void PrintSetDlg::OnOKBtnClicked()
{
	m_bIsPrint = true;
    Print();
    accept();
}

void PrintSetDlg::OnPreviewBtnClicked()
{
    if (m_PrintMode == PrintMode::PRINTBYSAMPLE)
    {
        if (ui->SinglePrintCBox->isChecked())                 // 单页打印
        {
            SampleInfoVector vecSampleDatas;
            std::vector<int64_t> sampleIds;
            ULOG(LOG_INFO, "Get print datas!");
            if (!m_GetSampleDate(vecSampleDatas, sampleIds))
                return;

            if (vecSampleDatas.empty())
                ULOG(LOG_INFO, "Print datas is empty!");

            std::vector<std::string> json_strs;
            for (int i = 0; i < vecSampleDatas.size(); i++)
            {
                std::string info = GetJsonString(vecSampleDatas[i]);
                ULOG(LOG_INFO, "Print datas : %s", info);
                json_strs.push_back(info);
            }

            std::string unique_id;
            int irect = printcom::printcom_open_overall_print_window(json_strs, unique_id);
            if (irect == 0)
            {
                m_bIsPrint = true;
                if (m_SetAlreadyPrinted)
                {
                    m_SetAlreadyPrinted(sampleIds);
                }

                accept();
            }

            ULOG(LOG_INFO, "Print preview SampleInfo retrun value: %d; size: %d; ID: %s", irect, json_strs.size(), unique_id);
        }
        else if (ui->CombinePrintCBox->isChecked())           // 合并打印（按样本打印）
        {
            SimpleSampleInfoVector vecSampleDatas;
            std::vector<int64_t> sampleIds;
            ULOG(LOG_INFO, "Get print datas!");
            if (!m_GetSimpleSampleDate(vecSampleDatas, sampleIds))
                return;

            if (vecSampleDatas.empty())
                ULOG(LOG_INFO, "Print datas is empty!");
            
            std::vector<std::string> json_strs;
            for (int i = 0; i < vecSampleDatas.size(); i++)
            {
                std::string info = GetJsonString(vecSampleDatas[i]);
                ULOG(LOG_INFO, "Print datas : %s", info);
                json_strs.push_back(info);
            }

            std::string unique_id;
            int irect = printcom::printcom_open_overall_print_window(json_strs, unique_id);
            if (irect == 0)
            {
                m_bIsPrint = true;
                if (m_SetAlreadyPrinted)
                {
                    m_SetAlreadyPrinted(sampleIds);
                }

                accept();
            }

            ULOG(LOG_INFO, "Print preview SimpleSampleInfo retrun value: %d size: %d", irect, json_strs.size());
        }
    }
    else if (m_PrintMode == PrintMode::PRINTBYITEM)// 按项目打印
    {
        if (GetResultType() == 0)
        {
            TipDlg(tr("请选择要打印的结果类型！")).exec();
            return ;
        }

        ItemSampleInfoVector vecItemDatas;
        ULOG(LOG_INFO, "Get print datas!");
        if (!m_GetItemData(vecItemDatas))
            return;

        if (vecItemDatas.empty())
            ULOG(LOG_INFO, "Print datas is empty!");

        std::vector<std::string> json_strs;
        for (int i = 0; i < vecItemDatas.size(); i++)
        {
            std::string info = GetJsonString(vecItemDatas[i]);
            ULOG(LOG_INFO, "Print datas : %s", info);
            json_strs.push_back(info);
        }

        std::string unique_id;
        int irect = printcom::printcom_open_overall_print_window(json_strs, unique_id);
        if (irect == 0)
        {
            accept();
        }

        ULOG(LOG_INFO, "Print preview SimpleSampleInfo retrun value: %d size: %d", irect, json_strs.size());
    }
}

void PrintSetDlg::OpenPreviewWidget()
{
    
}

void PrintSetDlg::PrintPreviewPages()
{
    Print();
}

void PrintSetDlg::SetSampleGetFun(std::function<bool(SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)> fun)
{
    m_GetSampleDate = fun;
}

void PrintSetDlg::SetSimpleSampleGetFun(std::function<bool(SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)> fun)
{
    m_GetSimpleSampleDate = fun;
}

void PrintSetDlg::SetItemDataGetFun(std::function<bool(ItemSampleInfoVector& vecItemDatas)> fun)
{
    m_GetItemData = fun;
}

void PrintSetDlg::SetFuncPrinted(std::function<void(const std::vector<int64_t>& sampIds)> fun)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_SetAlreadyPrinted = fun;
};

int PrintSetDlg::GetResultType()
{
    if (ui->FirstCheckCBox->isChecked() && ui->ReviewCBox->isChecked())
        return 3;
    else if (ui->FirstCheckCBox->isChecked())
        return 1;
    else if (ui->ReviewCBox->isChecked())
        return 2;
    else
        return 0;
}

void PrintSetDlg::Print()
{
    if (m_PrintMode == PrintMode::PRINTBYSAMPLE)
    {
        // 单页打印
        if (ui->SinglePrintCBox->isChecked())
        {
            std::thread t([this]() {

                // 打开进度条
                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
                std::vector<int64_t> alreadyPrintedSamples;                                     // 已打印的样本ID
                SampleInfoVector vecSampleDatas;
                std::vector<int64_t> sampleIds;
                ULOG(LOG_INFO, "Get print datas!");
                if (!m_GetSampleDate(vecSampleDatas, sampleIds))
                    return;

                if (vecSampleDatas.empty())
                    ULOG(LOG_INFO, "Print datas is empty!");

                std::vector<std::string> json_strs;
                for (int i = 0; i < vecSampleDatas.size(); i++)
                {
                    std::string info = GetJsonString(vecSampleDatas[i]);
                    ULOG(LOG_INFO, "Print datas : %s", info);
                    json_strs.push_back(info);
                    if (i < sampleIds.size())
                    {
                        alreadyPrintedSamples.push_back(sampleIds[i]);
                    }
                }

                std::string unique_id;
                int irect = printcom::printcom_overall_async_print(json_strs, unique_id);
                ULOG(LOG_INFO, "Print PRINTBYSAMPLE datas retrun value: %d ID: %s", irect, unique_id);

                // 回调设置已打印标记
                std::lock_guard<std::mutex> lock(m_mtx);
                if (m_SetAlreadyPrinted)
                {
                    m_SetAlreadyPrinted(alreadyPrintedSamples);
                }

                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);
            });

            t.detach();
        }
        // 合并打印（按样本打印）
        else if (ui->CombinePrintCBox->isChecked())
        {
            std::thread t([this]() {
                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
                SimpleSampleInfoVector vecSampleDatas;
                std::vector<int64_t> sampleIds;
                ULOG(LOG_INFO, "Get print datas!");
                if (!m_GetSimpleSampleDate(vecSampleDatas, sampleIds))
                    return;

                if (vecSampleDatas.empty())
                    ULOG(LOG_INFO, "Print datas is empty!");

                for (int i = 0; i < vecSampleDatas.size(); i++)
                {
                    std::string info = GetJsonString(vecSampleDatas[i]);
                    ULOG(LOG_INFO, "Print datas : %s", info);
                    std::string unique_id;
                    int irect = printcom::printcom_async_print(info, unique_id);
                    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
                }

                // 回调设置已打印标记
                std::lock_guard<std::mutex> lock(m_mtx);
                if (m_SetAlreadyPrinted)
                {
                    m_SetAlreadyPrinted(sampleIds);
                }

                POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);
            });
            t.detach();
        }
    }
    // 按项目打印
    else if (m_PrintMode == PrintMode::PRINTBYITEM)
    {
        if (GetResultType() == 0)
        {
            TipDlg(tr("请选择要打印的结果类型！")).exec();
            return;
        }

        std::thread t([this]() {
            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在处理数据，请稍候..."), true);
            ItemSampleInfoVector vecItemDatas;
            ULOG(LOG_INFO, "Get print datas!");
            if (!m_GetItemData(vecItemDatas))
                return;

            if (vecItemDatas.empty())
                ULOG(LOG_INFO, "Print datas is empty!");

            for (int i = 0; i < vecItemDatas.size(); i++)
            {
                std::string info = GetJsonString(vecItemDatas[i]);
                ULOG(LOG_INFO, "Print datas : %s", info);
                std::string unique_id;
                int irect = printcom::printcom_async_print(info, unique_id);
                ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            }

            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);
        });
        t.detach();
    }
}

