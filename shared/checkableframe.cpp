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
/// @file     checkableframe.cpp
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

#include "checkableframe.h"
#include <QVariant>
#include <QStyle>

#define PROPERTY_NAME_CHECKED                    ("checked")        // 属性名（选择）
#define PROPERTY_NAME_HOVER                      ("hover")          // 属性名（悬停）

CheckableFrame::CheckableFrame(QWidget *parent)
    : QFrame(parent)
{
    // 显示之前初始化
    InitBeforeShow();
}

CheckableFrame::~CheckableFrame()
{
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void CheckableFrame::InitBeforeShow()
{
    // 设置样式
    setStyleSheet("QFrame[checked=true]{border:2px solid #459fff;border-radius:10px;}\
                   QFrame[hover=true]{border:2px solid #459fff;border-radius:10px;}");

    // 默认不选中
    SetChecked(false);
}

///
/// @brief
///     是否选择
///
/// @return true表示已选择
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
bool CheckableFrame::IsChecked()
{
    return m_bChecked;
}

///
/// @brief
///     设置选择
///
/// @param[in]  bChecked  是否选择
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void CheckableFrame::SetChecked(bool bChecked)
{
    m_bChecked = bChecked;
    setProperty(PROPERTY_NAME_CHECKED, bChecked);
    UpdateStyleShow();
}

///
/// @brief
///     更新风格显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void CheckableFrame::UpdateStyleShow()
{
    // 更新样式显示(是否选择)
    this->style()->unpolish(this);
    this->style()->polish(this);
    update();
}

///
/// @brief
///     鼠标按下事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void CheckableFrame::mousePressEvent(QMouseEvent *event)
{
    // 选中效果取反
    SetChecked(!m_bChecked);
    emit SigMousePressed();
}

///
/// @brief
///     鼠标进入事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void CheckableFrame::enterEvent(QEvent *event)
{
    setProperty(PROPERTY_NAME_HOVER, true);
    UpdateStyleShow();
}

///
/// @brief
///     鼠标离开事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月5日，新建函数
///
void CheckableFrame::leaveEvent(QEvent *event)
{
    setProperty(PROPERTY_NAME_HOVER, false);
    UpdateStyleShow();
}
