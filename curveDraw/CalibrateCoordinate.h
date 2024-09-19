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
/// @file     ScatterPlotCoordinatesClass.h
/// @brief    坐标系的绘制(X,Y轴)
///
/// @author   5774/WuHongTao
/// @date     2020年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include "common.h"
#include "CoordinateBase.h"

class QColor;
class QPainter;

class CalibrateCoordinate :
    public CoordinateBase
{
    Q_OBJECT

public:
    ///
    /// @brief 构造函数
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月13日，新建函数
    ///
    CalibrateCoordinate(int xaAisLength, int yaAisLength);

    ///
    /// @brief 析构函数
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月13日，新建函数
    ///
    ~CalibrateCoordinate();

    ///
    /// @brief QGraphicsView调用，在本地坐标系统中绘制项
    /// @param[in]  painter 绘图句柄 
    /// @param[in]  option  绘图选项
    /// @param[in]  widget  父控件
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月13日，新建函数
    ///    
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

signals:
    void yAxisMaxWidthChanged(int width);

private:
    ///
    /// @brief 绘制坐标刻度
    ///     
    /// @param[in]  painter  画笔
    /// @param[in]  rect  框体坐标系
    /// @param[in]  rectColor  框体坐标系的颜色
    /// @param[in]  scaleText  文字显示
    /// @param[in]  type  竖线还是横线
    /// @param[in]  yAxisTextMaxWidth   Y轴文字最宽的刻度像素宽度（仅用于Y轴文字右对齐使用）
    /// @param[in]  textAlignment  文字在区域内的对齐方式
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月21日，新建函数
    ///
    void drawOneScaleLine(QPainter * painter, const QRectF rect, QColor rectColor, QString scaleText, int type, int yAxisTextMaxWidth = 0, Qt::Alignment textAlignment = Qt::AlignLeft);

    ///
    /// @brief 绘制一个刻度值
    ///     
    /// @param[in]  painter 画笔 
    /// @param[in]  xPos  X的位置
    /// @param[in]  yPos  Y的位置
    /// @param[in]  width  刻度宽
    /// @param[in]  height  刻度高
    /// @param[in]  rectColor  颜色
    /// @param[in]  scaleText  刻度显示的内容
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///
    void drawOneScale(QPainter * painter, int xPos, int yPos,int width, int height, QColor rectColor, QString scaleText);

    ///
    /// @brief 绘制一个刻度值
    ///     
    /// @param[in]  painter  画笔
    /// @param[in]  rect  浮点数精度的位置信息
    /// @param[in]  rectColor  颜色
    /// @param[in]  scaleText  刻度信息
    /// @par History:
    /// @li 5774/WuHongTao，2020年4月17日，新建函数
    ///
    void drawOneScale(QPainter * painter, const QRectF rect, QColor rectColor, QString scaleText);
};

