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
/// @file     tipdlg.h
/// @brief    提示对话框
///
/// @author   4170/TangChuXian
/// @date     2020年6月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "basedlg.h"
namespace Ui { class TipDlg; };

namespace TipDlgType
{
    // 按钮模式
    enum ButtonModel
    {
        SINGLE_BUTTON = 0,   // 单个按钮
        TWO_BUTTON           // 双按钮
    };
}

class TipDlg : public BaseDlg
{
    Q_OBJECT

public:
    TipDlg(QWidget *parent = Q_NULLPTR);
    ~TipDlg();

    ///
    /// @brief
    ///     构造时初始化提示文本和按钮模式
    ///
    /// @param[in]  strText     提示文本
    /// @param[in]  enBtnModel  按钮模式
    /// @param[in]  parent      父窗口指针
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月8日，新建函数
    ///
    TipDlg(const QString& strText, TipDlgType::ButtonModel enBtnModel = TipDlgType::SINGLE_BUTTON, QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     构造时初始化提示文本和按钮模式
    ///
    /// @param[in]  strTitle    标题文本
    /// @param[in]  strText     提示文本
    /// @param[in]  enBtnModel  按钮模式
    /// @param[in]  parent      父窗口指针
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月11日，新建函数
    ///
    TipDlg(const QString& strTitle, const QString& strText, TipDlgType::ButtonModel enBtnModel = TipDlgType::SINGLE_BUTTON, QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     设置提示对话框文本
    ///
    /// @param[in]  strText  提示对话框
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月8日，新建函数
    ///
    void SetText(const QString& strText);

    ///
    /// @brief
    ///     获取当前显示文本
    ///
    /// @return 当前显示文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月29日，新建函数
    ///
    const QString& Text();

    ///
    /// @brief
    ///     设置按钮模式
    ///
    /// @param[in]  enBtnModel  按钮模式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月8日，新建函数
    ///
    void SetButtonModel(TipDlgType::ButtonModel enBtnModel);

    inline void SetButtonText(const QString& fristBtnText, const QString& secondBtnText) 
    {
        m_fristBtnText = fristBtnText;
        m_secondBtnText = secondBtnText;
    }

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月8日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月8日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     根据文本调整长款自适应
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月20日，新建函数
    ///
    void ResizeByText();

private:
    Ui::TipDlg                 *ui;                      // UI对象指针
    bool                        m_bInit;                 // 是否已经初始化

    QString                     m_strTipText;            // 提示文本
    TipDlgType::ButtonModel     m_enBtnModel;            // 按钮模式
    QString                     m_fristBtnText;          // 第一个按钮的文本
    QString                     m_secondBtnText;         // 第二个按钮的文本
};
