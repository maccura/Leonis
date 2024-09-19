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
/// @file     checkableframe.h
/// @brief    可选Frame
///
/// @author   4170/TangChuXian
/// @date     2023年6月5日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QFrame>

class CheckableFrame : public QFrame
{
    Q_OBJECT

public:
    CheckableFrame(QWidget *parent);
    ~CheckableFrame();

    ///
    /// @brief
    ///     是否选择
    ///
    /// @return true表示已选择
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    bool IsChecked();

    ///
    /// @brief
    ///     设置选择
    ///
    /// @param[in]  bChecked  是否选择
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void SetChecked(bool bChecked);

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     更新风格显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void UpdateStyleShow();

    ///
    /// @brief
    ///     鼠标按下事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void mousePressEvent(QMouseEvent *event);

    ///
    /// @brief
    ///     鼠标进入事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void enterEvent(QEvent *event);

    ///
    /// @brief
    ///     鼠标离开事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void leaveEvent(QEvent *event);

Q_SIGNALS:
    ///
    /// @brief
    ///     鼠标按下
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void SigMousePressed();

private:
    bool m_bChecked;                        // 是否选择
};
