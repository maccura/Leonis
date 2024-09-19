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
/// @file     mctubewidget.cpp
/// @brief    自定义试管界面
///
/// @author   4170/TangChuXian
/// @date     2022年8月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "immctubewidget.h"
#include <QPainter>
#include <QPen>

ImMcTubeWidget::ImMcTubeWidget(QWidget *parent /*= Q_NULLPTR*/, const QColor& tubeColor /*= QColor(0x45, 0x9f, 0xff)*/, 
    const QColor& liquidColor /*= QColor(0xc7, 0xe6, 0xff)*/, double dRemainRatio /*= 1.0*/)
    : QWidget(parent)
{
    // 初始化成员变量
    m_tubeColor    = tubeColor;
    m_liquidColor  = liquidColor;
    m_dRemainRatio = dRemainRatio;
}

ImMcTubeWidget::~ImMcTubeWidget()
{
}

///
/// @brief
///     获取试管颜色
///
/// @return 试管颜色
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
const QColor& ImMcTubeWidget::GetTubeColor()
{
    return m_tubeColor;
}

///
/// @brief
///     设置试管颜色
///
/// @param[in]  tubeColor  试管颜色
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
void ImMcTubeWidget::SetTubeColor(const QColor& tubeColor)
{
    m_tubeColor = tubeColor;
}

///
/// @brief
///     获取试管液颜色
///
/// @return 试管液颜色
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
const QColor& ImMcTubeWidget::GetLiquidColor()
{
    return m_liquidColor;
}

///
/// @brief
///     设置试管液颜色
///
/// @param[in]  liquidColor  试管液颜色
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
void ImMcTubeWidget::SetLiquidColor(const QColor& liquidColor)
{
    m_liquidColor = liquidColor;
}

///
/// @brief
///     获取剩余试管液比例
///
/// @return 1表示满 0表示空
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
double ImMcTubeWidget::GetRemainRatio()
{
    return m_dRemainRatio;
}

///
/// @brief
///     设置剩余试管液比例
///
/// @param[in]  dRemainRatio  剩余试管液比例 1表示满 0表示空
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
void ImMcTubeWidget::SetRemainRatio(const double& dRemainRatio)
{
    m_dRemainRatio = dRemainRatio;
}

///
/// @brief
///     重写重绘事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月25日，新建函数
///
void ImMcTubeWidget::paintEvent(QPaintEvent *event)
{
    // 基类处理
    QWidget::paintEvent(event);

    // 获取绘制区域和画家
    QPainter painter(this);
    QRect tubeRect = this->rect();

    // 画试管液
    painter.setPen(QPen(Qt::transparent, 1.0));
    painter.setBrush(m_liquidColor);
    painter.drawRoundRect(tubeRect, 37, 5);
    int liquidHeight = tubeRect.height() * m_dRemainRatio;
    QRect liquidMaskRect(0, 0, tubeRect.width(), tubeRect.height() - liquidHeight);
    painter.setBrush(Qt::white);
    painter.drawRect(liquidMaskRect);

    // 画试管
    QPen tubePen(m_tubeColor, 1.0);
    painter.setPen(tubePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundRect(tubeRect, 37, 5);
}
