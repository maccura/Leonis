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
/// @file     QBottleProgressBar.h
/// @brief    进度条的条目
///
/// @author   5774/WuHongTao
/// @date     2022年1月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QPixmap>

namespace Ui {
    class QBottleProgressBar;
};

class QBottleProgressBar : public QWidget
{
    Q_OBJECT

public:
    QBottleProgressBar(QWidget *parent = Q_NULLPTR);
    ~QBottleProgressBar();

    ///
    /// @brief
    ///     设置进度条的进度
    ///
    /// @param[in]  precent  进度（0-100）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月23日，新建函数
    ///
    void SetProgress(int percent, QPixmap& pixmap);

    ///
    /// @brief
    ///     进度条的绘制函数
    ///
    /// @param[in]  p  绘制事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月24日，新建函数
    ///
    virtual void paintEvent(QPaintEvent* p);

private:
    Ui::QBottleProgressBar* ui;
    QPixmap                         m_pixmap;
};
