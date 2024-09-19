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
/// @file     mccustomscaledraw.h
/// @brief    自定义坐标轴
///
/// @author   4170/TangChuXian
/// @date     2018年06月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2018年06月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#ifndef MCCUSTOMSCALEDRAW_H
#define MCCUSTOMSCALEDRAW_H

#include <QColor>
#include <qwt_scale_draw.h>

// 自画坐标轴(用于质控Y轴)
class McCustomScaleDraw: public QwtScaleDraw
{
public:
    explicit McCustomScaleDraw();

    // 重写函数:设置坐标轴的刻度值
    virtual QwtText label(double v) const;

    // 重写函数:重绘坐标轴的刻度值
    virtual void drawLabel(QPainter* painter, double val) const;
};

// 自画坐标轴(用于只有整数显示)
class McCustomIntegerScaleDraw : public QwtScaleDraw
{
public:
    explicit McCustomIntegerScaleDraw();

    // 重写函数:设置坐标轴的刻度值
    virtual QwtText label(double v) const;

    // 重写函数:重绘坐标轴的刻度值
    virtual void drawLabel(QPainter* painter, double val) const;

    // 设置特定文本颜色
    void SetSelLabelColor(int val, const QColor& labelColor);

protected:
    QColor m_color;
    double m_iSelVal;
};

// 自画彩色值坐标轴
class McCustomColorScaleDraw : public QwtScaleDraw
{
public:
    explicit McCustomColorScaleDraw();

    // 重写函数:设置坐标轴的刻度值
    virtual QwtText label(double v) const;

    // 重写函数:重绘坐标轴的刻度值
    virtual void drawLabel(QPainter* painter, double val) const;

    // 设置文本颜色
    void SetLabelColor(const QColor& labelColor);

private:
    QColor m_color;
};

#endif // MCCUSTOMSCALEDRAW_H
