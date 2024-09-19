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
/// @file     printsetdlg.h
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
#pragma once

#include "shared/basedlg.h"
#include "PrintExportDefine.h"
#include <thread>
#include <mutex>
#include <condition_variable>
namespace Ui { class PrintSetDlg; };

class PrintSetDlg : public BaseDlg
{
    Q_OBJECT

public:
    enum PrintMode
    {
        UNKNOMN = -1,
        PRINTBYSAMPLE = 0,  // 按样本打印
        PRINTBYITEM = 1     // 按项目打印
    };

public:
    PrintSetDlg(bool bIsHisData, QWidget *parent = Q_NULLPTR);

    // 设置打印模式（按样本/按项目
    void SetModel(PrintMode printMode);
    ~PrintSetDlg();

    void showEvent(QShowEvent *event);                  // 重载显示事件处理函数
    void InitConnect();
    void InitAfterShow();                               // 显示后初始化
    void InitStrResource();                             // 初始化界面显示字符资源
    void InitChildWidget();                             // 初始化子控件
    int GetResultType();
    void SetSampleGetFun(std::function<bool(SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)> fun);
    void SetSimpleSampleGetFun(std::function<bool(SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)> fun);
    void SetItemDataGetFun(std::function<bool(ItemSampleInfoVector& vecItemDatas)> fun);
	bool IsPrintFinished() { return m_bIsPrint; };

    // 设置告知已打印样本的回调
    void SetFuncPrinted(std::function<void(const std::vector<int64_t>& sampIds)> fun);

private:

    void Print();

protected Q_SLOTS:
    void ChoosePrintMode();
    void OnOKBtnClicked();
    void OnPreviewBtnClicked();
    void OpenPreviewWidget();
    void PrintPreviewPages();

Q_SIGNALS:
    void PagesCompleted();

private:
    Ui::PrintSetDlg             *ui;
    bool                        m_bInit;
    bool                        m_bIsHisData;
	bool						m_bIsPrint;
    PrintMode                   m_PrintMode;

    static std::mutex           m_mtx;
    std::condition_variable     m_CV;           // 条件变量，用于控制预览页面内存的释放

    std::function<bool(SampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)> m_GetSampleDate;
    std::function<bool(SimpleSampleInfoVector& vecSampleDatas, std::vector<int64_t>& sampIds)> m_GetSimpleSampleDate;
    std::function<bool(ItemSampleInfoVector& vecItemDatas)> m_GetItemData;

    std::function<void(const std::vector<int64_t>& sampIds)> m_SetAlreadyPrinted; // 回调，用于告知这些样本已经被打印
};
