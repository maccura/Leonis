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
/// @file     mcusernamelabel.h
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
#ifndef MCUSERNAMELABEL_H
#define MCUSERNAMELABEL_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include <QString>
#include <QImage>
#include <QColor>

class QPainter;


class McUserNameLabel : public QWidget
{
    Q_OBJECT

public:
    McUserNameLabel(QWidget* parent = NULL);

    ///
    /// @brief
    ///     设置对齐方式
    ///
    /// @param[in]  alignment  对齐方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void SetAlignment(Qt::Alignment alignment);

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
    void SetText(const QString &text);


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
    void SetIcon(const QImage &img);

    ///
    /// @brief
    ///     设置图标绘制区域
    ///
    /// @param[in]  rect  图标绘制区域
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void SetIconRect(const QRect &rect);

    ///
    /// @brief
    ///     设置画笔
    ///
    /// @param[in]  pen  画笔
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void SetPen(const QPen& pen);

    ///
    /// @brief
    ///     设置画刷
    ///
    /// @param[in]  brush  画刷
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void SetBrush(const QBrush& brush);

    ///
    /// @brief
    ///     设置背景颜色
    ///
    /// @param[in]  color  背景颜色
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void SetBackGroundColor(const QColor &color);

    ///
    /// @brief
    ///     获取对齐方式
    ///
    /// @return 对齐方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    Qt::Alignment Alignment();

    ///
    /// @brief
    ///     获取文本
    ///
    /// @return 文本内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    const QString& Text();

    ///
    /// @brief
    ///     获取图标
    ///
    /// @return 图标
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    const QImage& Icon();

    ///
    /// @brief
    ///     获取图标绘制区域
    ///
    /// @return 图标绘制区域
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    const QRect& IconRect();

    ///
    /// @brief
    ///     获取画笔
    ///
    /// @return 画笔
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    QPen& Pen();

    ///
    /// @brief
    ///     获取画刷
    ///
    /// @return 画刷
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    QBrush& Brush();

protected:
    ///
    /// @brief
    ///     重绘函数
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void paintEvent(QPaintEvent* event) override;

private:
    ///
    /// @brief
    ///     绘制居中对齐方式
    ///
    /// @param[in]  painter  绘图对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void DrawAlignCenter(QPainter& painter);

    ///
    /// @brief
    ///     绘制左对齐方式
    ///
    /// @param[in]  painter  绘图对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void DrawAlignLeft(QPainter& painter);

    ///
    /// @brief
    ///     绘制右对齐方式
    ///
    /// @param[in]  painter  绘图对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月12日，新建函数
    ///
    void DrawAlignRight(QPainter& painter);

private:
    QString         m_strText;             // 文本内容
    QImage          m_Icon;                // 图标
    Qt::Alignment   m_Alignment;           // 对齐方式
    QRect           m_IconRect;            // 绘制图标矩形区域

    QPen            m_Pen;                 // 画笔对象
    QBrush          m_Brush;               // 画刷对象

    QColor          m_BackGroundColor;     // 背景色
};

#endif // MCUSERNAMELABEL_H
