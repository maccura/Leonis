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
/// @file     CalibrateCoordinate.h
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

#include "CalibrateCoordinate.h"
#include <QColor>
#include <QPainter>
#include "shared/uidef.h"

///
/// @brief 构造函数
/// @par History:
/// @li 5774/WuHongTao，2020年4月13日，新建函数
///
CalibrateCoordinate::CalibrateCoordinate(int xaAisLength, int yaAisLength)
    : CoordinateBase(xaAisLength, yaAisLength)
{

}

///
/// @brief 析构函数
/// @par History:
/// @li 5774/WuHongTao，2020年4月13日，新建函数
///
CalibrateCoordinate::~CalibrateCoordinate()
{

}

///
/// @brief QGraphicsView调用，在本地坐标系统中绘制项
/// @param[in]  painter 绘图句柄 
/// @param[in]  option  绘图选项
/// @param[in]  widget  父控件
/// @par History:
/// @li 5774/WuHongTao，2020年4月13日，新建函数
///
void CalibrateCoordinate::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{

    QPen pen;
    QFont font;
    font.setPointSize(8);
    font.setPixelSize(10);
    font.setFamily(FONT_FAMILY);
    pen.setColor(Qt::black);
    pen.setWidth(0.5);
    painter->save();
    painter->setFont(font);
    painter->setPen(pen);

    //如果没有刻度，就没必要绘制了
    if (XaxisPostionGroup.empty() || YaxisPostionGroup.empty())
    {
        return;
    }

    QRectF RectRange;
    RectRange.setRect(ScaleCoordinateSystem.startxDistance + ScaleCoordinateSystem.width, ScaleCoordinateSystem.startyDistance,
        Xaxis - ScaleCoordinateSystem.startxDistance - ScaleCoordinateSystem.width,
        Yaxis - ScaleCoordinateSystem.startyDistance - ScaleCoordinateSystem.height);

    //绘制曲线区域的矩形框
    painter->drawRect(RectRange);
    painter->fillRect(RectRange, QColor("#f5f7fb"));

    // 初始化字体，用于计算是否会有重叠显示
    QFont calcFont(FONT_FAMILY, 12);
    QFontMetrics fm(calcFont);

    
    QMap<qreal, DataPosAndValue>::iterator iter = XaxisPostionGroup.begin();
    
    // 获取X轴显示的最大值
    qreal maxXValue = -MAXNUMBER;
    while (iter != XaxisPostionGroup.end())
    {
        if (iter->valueText != MAXNUMBER && maxXValue < iter->valueText)
            maxXValue = iter->valueText;

        ++iter;
    }
    
    //绘制x坐标系的刻度
    iter = XaxisPostionGroup.begin();
    qreal xAxisMaxPos = -MAXNUMBER;
    Qt::Alignment textAlign = Qt::AlignLeft;
    
    while (iter != XaxisPostionGroup.end())
    {
        QRectF rect;
        rect.setX(iter->xPos);
        rect.setY(iter->yPos);
        rect.setWidth(iter->width);
        rect.setHeight(iter->hieght);
        QString scaleContent = QString::number(iter->valueText,'f', ScaleCoordinateSystem.retainedNumber);

        // 如果重叠了，则此刻度不显示
        if (iter->xPos < xAxisMaxPos)
        {
            scaleContent.clear();
        }
        else
            xAxisMaxPos = iter->xPos + fm.width(scaleContent);

        // 最大的坐标位置要右对齐显示
        if (maxXValue == iter->valueText)
            textAlign = Qt::AlignRight;

        //选择分支控制隔离带的显示
        if (ScaleCoordinateSystem.scaleType == 0)
        {
            if (iter->specialPoint == true)
            {
                drawOneScale(painter, rect, Qt::lightGray, QString(" "));
            }
            else
            {
                drawOneScale(painter, rect, Qt::lightGray, scaleContent);
            }
        }
        else
        {
            if (iter->specialPoint == true)
            {
                drawOneScaleLine(painter, rect, Qt::lightGray, QString(" "), 1, 0, textAlign);
            }
            else
            {
                drawOneScaleLine(painter, rect, Qt::lightGray, scaleContent, 1, 0, textAlign);
            }

        }
        iter++;
    }
    
    // 获取Y轴最大的刻度值文字像素宽度
    int yTextMaxWidth = 0;
    
    //绘制y轴坐标系的刻度
    {
        QMap<qreal, DataPosAndValue>::iterator iter = YaxisPostionGroup.begin();
        while (iter != YaxisPostionGroup.end())
        {
            QString scaleContent = QString::number(iter->valueText, 'f', 0);
            if (iter->valueText != MAXNUMBER)
            {
                int w = fm.width(scaleContent);
                if (w > yTextMaxWidth)
                {
                    yTextMaxWidth = w;
                }
            }

            ++iter;
        }

        iter = YaxisPostionGroup.begin();
        while (iter != YaxisPostionGroup.end())
        {
            QRectF rect;
            rect.setX(iter->xPos);
            rect.setY(iter->yPos);
            rect.setWidth(iter->width);
            rect.setHeight(iter->hieght);
            QString scaleContent = QString::number(iter->valueText, 'f', 0);

            if (ScaleCoordinateSystem.scaleType == 0)
            {
                //选择分支控制隔离带的显示
                if (iter->specialPoint == true)
                {
                    drawOneScale(painter, rect, Qt::lightGray, QString(" "));
                }
                else
                {
                    drawOneScale(painter, rect, Qt::lightGray, scaleContent/*QString::number(iter->valueText, 'f', 2)*/);
                }
            }
            else
            {
                if (iter->specialPoint == true)
                {
                    drawOneScaleLine(painter, rect, Qt::lightGray, QString(" "), 2, yTextMaxWidth);
                }
                else
                {
                    drawOneScaleLine(painter, rect, Qt::lightGray, scaleContent/*QString::number(iter->valueText, 'f', 2)*/, 2, yTextMaxWidth);
                }
            }
            iter++;
        }
        
        // 通知更新Y轴最大刻度，用于文字缩放显示
        emit yAxisMaxWidthChanged(yTextMaxWidth);
    }
    painter->restore();
}

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
void CalibrateCoordinate::drawOneScale(QPainter * painter, const QRectF rect, QColor rectColor, QString scaleText)
{
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(ScaleCoordinateSystem.linewidth);
    QFont font;
/*    font.setPointSize(8);*/
    font.setPixelSize(12);
    font.setFamily(FONT_FAMILY);
    painter->save();
    painter->setFont(font);
    painter->setPen(pen);
    painter->fillRect(rect, QBrush(QColor(220, 220, 220)));
    painter->drawRect(rect);
    painter->drawText(QPoint(rect.x() + 1, rect.y() + 10), scaleText);
    painter->restore();
}

///
/// @brief 绘制坐标刻度
///     
/// @param[in]  painter  画笔
/// @param[in]  rect  框体坐标系
/// @param[in]  rectColor  框体坐标系的颜色
/// @param[in]  scaleText  文字显示
/// @param[in]  type  竖线还是横线
/// @par History:
/// @li 5774/WuHongTao，2020年4月21日，新建函数
///
void CalibrateCoordinate::drawOneScaleLine(QPainter * painter, const QRectF rect, QColor rectColor, QString scaleText, int type, int yAxisTextMaxWidth, Qt::Alignment textAlignment)
{
    //设置画笔
    QPen pen;
    pen.setColor(Qt::gray);
    pen.setWidth(ScaleCoordinateSystem.linewidth);
    pen.setStyle(Qt::DotLine);

    //设置字体
    QFont font;
/*    font.setPointSize(8);*/
    font.setPixelSize(12);
    font.setFamily(FONT_FAMILY);
    painter->save();
    painter->setFont(font);
    painter->setPen(pen);
    QFontMetrics fm(font);
    QRectF& newRect = const_cast<QRectF&>(rect);
    //根据坐标系类型选择不同的绘制方式
    if (ScaleCoordinateSystem.scaleType == 0)
    {
        painter->drawRect(rect);
        painter->fillRect(rect, QBrush(rectColor));
        painter->drawText(QPoint(rect.x() - 5, rect.y() + 10), scaleText);
    }
    else
    {
        if (type == 1)
        {
            painter->drawLine(QPointF(rect.x(), ScaleCoordinateSystem.startyDistance), QPointF(rect.x(), Yaxis - ScaleCoordinateSystem.height));
            if (textAlignment == Qt::AlignRight)
                newRect.setX(rect.x() - fm.width(scaleText) + 12);
                
            painter->drawText(QPoint(newRect.x() - 5, Yaxis - 8), scaleText);
        }
        //绘制y轴
        else
        {
            painter->drawLine(QPointF(ScaleCoordinateSystem.startxDistance + ScaleCoordinateSystem.width, rect.y()), QPointF(Xaxis, rect.y()));
            if (scaleText != " ")
            {
                //绘制空的位置时候
                if (scaleText == "0")
                {
                    painter->drawText(QPoint(1, rect.y() + 5), "x");
                }
                else
                {
                    // Y轴文字右对齐
                    if (yAxisTextMaxWidth > 0/* && textAlignment == Qt::AlignRight*/)
                    {
                        int offset = yAxisTextMaxWidth - fm.width(scaleText + ScaleCoordinateSystem.unit);
                        painter->drawText(QPoint(1 + (offset) - 5, rect.y() + 5), scaleText + ScaleCoordinateSystem.unit);
                    }
                    // 从左侧开始显示文字
                    else
                        painter->drawText(QPoint(1, rect.y() + 5), scaleText + ScaleCoordinateSystem.unit);
                }
            }
        }
    }
    painter->restore();
}

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
void CalibrateCoordinate::drawOneScale(QPainter * painter, int xPos, int yPos, int width, int height, QColor rectColor, QString scaleText)
{
    //设置画笔的属性
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(1);
    painter->save();
    painter->setPen(pen);

    //绘制长方形
    painter->drawRect(xPos, yPos, width, height);
    painter->fillRect(xPos, yPos, width, height, QBrush(rectColor));
    painter->drawText(QPoint(xPos + 1, yPos + 10), scaleText);
    painter->restore();
}
