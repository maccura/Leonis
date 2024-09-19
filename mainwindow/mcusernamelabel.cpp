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
/// @file     mcusernamelabel.cpp
/// @brief    自定义用户名标签
///
/// @author   4170/TangChuXian
/// @date     2019年7月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2019年7月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "mcusernamelabel.h"
#include <QFontMetrics>
#include <QUrl>
#include <QPainter>

#define INTERVAL_OF_IMG_AND_WORD     6                                     // 图标和文本的间距

McUserNameLabel::McUserNameLabel(QWidget *parent) :
    QWidget(parent)
{
    // 初始化图标和文本
    resize(120, 21);
    m_Icon.load(QString(":/Resources/image/mainUser.png"));
    m_Alignment = Qt::AlignLeft;
    m_strText = QString("admin");
    m_IconRect.setRect(0, 0, 11, 12);

    // 设置文本颜色和背景色
    m_Pen.setWidth(1);
    m_Pen.setColor(Qt::white);
    m_Brush.setColor(Qt::transparent);
    m_BackGroundColor = QColor(Qt::transparent);

    // 设置背景透明
    setAutoFillBackground(true);
    QPalette p = palette();
    p.setBrush(QPalette::Window,Qt::transparent);
    setPalette(p);
}

///
/// @brief
///     设置对齐方式
///
/// @param[in]  alignment  对齐方式
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::SetAlignment(Qt::Alignment alignment)
{
    m_Alignment = alignment;
}

///
/// @brief
///     设置文本
///
/// @param[in]  text  文本内容
///
/// @return 
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::SetText(const QString &text)
{
    m_strText = text;
}

///
/// @brief
///     设置图标
///
/// @param[in]  img  图标
///
/// @return 
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::SetIcon(const QImage &img)
{
    m_Icon = img;
}

///
/// @brief
///     设置图标绘制区域
///
/// @param[in]  rect  图标绘制区域
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::SetIconRect(const QRect &rect)
{
    m_IconRect = rect;
}

///
/// @brief
///     设置画笔
///
/// @param[in]  pen  画笔
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::SetPen(const QPen &pen)
{
    m_Pen = pen;
}

///
/// @brief
///     设置画刷
///
/// @param[in]  brush  画刷
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::SetBrush(const QBrush &brush)
{
    m_Brush = brush;
}

///
/// @brief
///     设置背景颜色
///
/// @param[in]  color  背景颜色
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::SetBackGroundColor(const QColor &color)
{
    m_BackGroundColor = color;
}

///
/// @brief
///     获取对齐方式
///
/// @return 对齐方式
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
Qt::Alignment McUserNameLabel::Alignment()
{
    return m_Alignment;
}

///
/// @brief
///     获取文本
///
/// @return 文本内容
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
const QString &McUserNameLabel::Text()
{
    return m_strText;
}

///
/// @brief
///     获取图标
///
/// @return 图标
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
const QImage &McUserNameLabel::Icon()
{
    return m_Icon;
}

///
/// @brief
///     获取图标绘制区域
///
/// @return 图标绘制区域
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
const QRect &McUserNameLabel::IconRect()
{
    return m_IconRect;
}

///
/// @brief
///     获取画笔
///
/// @return 画笔
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
QPen &McUserNameLabel::Pen()
{
    return m_Pen;
}

///
/// @brief
///     获取画刷
///
/// @return 画刷
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
QBrush &McUserNameLabel::Brush()
{
    return m_Brush;
}

///
/// @brief
///     重绘函数
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::paintEvent(QPaintEvent *)
{
    // 构造绘图对象
    QPainter painter(this);
    painter.setPen(m_Pen);
    painter.setBrush(m_Brush);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 绘制背景
    painter.save();
    painter.setPen(QPen(Qt::transparent));
    painter.setBrush(QBrush(m_BackGroundColor));
    painter.drawRect(QRect(0,0, size().width(), size().height()));
    painter.restore();

    // 根据对齐方式绘制图表和文本
    if(m_Alignment == Qt::AlignCenter)
    {
        DrawAlignCenter(painter);
    }
    else if(m_Alignment == Qt::AlignLeft)
    {
        DrawAlignLeft(painter);
    }
    else if(m_Alignment == Qt::AlignRight)
    {
        DrawAlignRight(painter);
    }
}

///
/// @brief
///     绘制居中对齐方式
///
/// @param[in]  painter  绘图对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::DrawAlignCenter(QPainter& painter)
{
    // 获取窗口绘制区域
    QRect wgtRect = rect();

    // 判断是否设置图标
    if(m_Icon.isNull())
    {
        // 无图标则绘制文本
        painter.drawText(wgtRect,Qt::AlignCenter | Qt::AlignVCenter,
                         m_strText);
    }
    else
    {
        // 有图标绘制图表和文本
        int fontLen = painter.fontMetrics().width(m_strText);
        QRect imgRect = m_IconRect;
        int x = ((wgtRect.width() - fontLen -
                      INTERVAL_OF_IMG_AND_WORD -
                      imgRect.width()) / 2);
        int y = ((wgtRect.height() - imgRect.height()) / 2);
        imgRect.moveTo(x, y);
        painter.drawImage(imgRect, m_Icon);

        wgtRect.setLeft(imgRect.x() + imgRect.width() +
                        INTERVAL_OF_IMG_AND_WORD);
        painter.drawText(wgtRect,Qt::AlignLeft  | Qt::AlignVCenter,
                         m_strText);
    }
}

///
/// @brief
///     绘制左对齐方式
///
/// @param[in]  painter  绘图对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::DrawAlignLeft(QPainter& painter)
{
    // 获取窗口绘制区域
    QRect wgtRect = rect();

    // 判断是否设置图标
    if(m_Icon.isNull())
    {
        // 无图标则绘制文本
        painter.drawText(wgtRect,Qt::AlignLeft  | Qt::AlignVCenter,
                         m_strText);
    }
    else
    {
        // 有图标绘制图表和文本
        QRect imgRect = m_IconRect;
        int x = (wgtRect.x());
        int y = ((wgtRect.height() - imgRect.height()) / 2);
        imgRect.moveTo(x, y);
        painter.drawImage(imgRect, m_Icon);

        wgtRect.setLeft(imgRect.x() + imgRect.width() +
                        INTERVAL_OF_IMG_AND_WORD);
        painter.drawText(wgtRect,Qt::AlignLeft | Qt::AlignVCenter,
                         m_strText);
    }
}

///
/// @brief
///     绘制右对齐方式
///
/// @param[in]  painter  绘图对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年6月12日，新建函数
///
void McUserNameLabel::DrawAlignRight(QPainter& painter)
{
    // 获取窗口绘制区域
    QRect wgtRect = rect();

    // 绘制文本
    painter.drawText(wgtRect, Qt::AlignRight | Qt::AlignVCenter,
                     m_strText);

    // 判断是否设置图标，有图标则绘制图标
    if(!m_Icon.isNull())
    {
        int fontLen = painter.fontMetrics().width(m_strText);
        QRect imgRect = m_IconRect;
        int x = (wgtRect.width() - fontLen -
                 INTERVAL_OF_IMG_AND_WORD - imgRect.width());
        int y = ((wgtRect.height() - imgRect.height()) / 2);
        imgRect.moveTo(x, y);
        painter.drawImage(imgRect, m_Icon);
    }
}
