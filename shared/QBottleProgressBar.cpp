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
#include "QBottleProgressBar.h"
#include "ui_QBottleProgressBar.h"
#include <QPainter>
#include "src/common/Mlog/mlog.h"

QBottleProgressBar::QBottleProgressBar(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QBottleProgressBar();
    ui->setupUi(this);
    // 默认显示条
    m_pixmap.load(":/Leonis/resource/image/water-blue.png");
    // 初始显示进度为0
    SetProgress(0, m_pixmap);
}

QBottleProgressBar::~QBottleProgressBar()
{
}

///
/// @brief
///     设置进度条的进度
///
/// @param[in]  precent  进度（0-100）
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月23日，新建函数
///
void QBottleProgressBar::SetProgress(int percent, QPixmap& pixmap)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, percent);
    // 越界判断
    if (percent < 0)
    {
        percent = 0;
    }

    if (percent > 100)
    {
        percent = 100;
    }

    // 计算掩盖多少
    m_pixmap = pixmap;
    int height = this->height() - this->height()*percent/100;
    QRegion maskArea(0, height,this->width(),this->height());
    setMask(maskArea);
}

///
/// @brief
///     进度条的绘制函数
///
/// @param[in]  p  绘制事件
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月24日，新建函数
///
void QBottleProgressBar::paintEvent(QPaintEvent* p)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, this->width(), this->height(), m_pixmap);
}
