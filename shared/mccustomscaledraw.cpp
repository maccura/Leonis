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

#include "mccustomscaledraw.h"
#include <QPainter>
#include <cfloat>

#define LINE_TEXT_INTERVAL    2

McCustomScaleDraw::McCustomScaleDraw()
{
}

QwtText McCustomScaleDraw::label(double v) const
{
    // 初始化数值显示
    QString StrOffset;

    // 如果不为1则显示数值
    if ((qAbs(v) - 1.0) > DBL_EPSILON)
    {
        StrOffset.setNum(v);
    }
    else if (v < DBL_EPSILON)
    {
        StrOffset.append("-");
    }
    StrOffset.append("SD");

    // 大于0显示+号
    if (v > DBL_EPSILON)
    {
        StrOffset = QString("+").append(StrOffset);
    }

    // 判断v是否为0,为0则显示靶值X
    if (qAbs(v) <= DBL_EPSILON)
    {
        StrOffset = QString("Mean");
    }

    // 显示刻度值
    return StrOffset;
}

void McCustomScaleDraw::drawLabel(QPainter *painter, double val) const
{
    // 判断v是否为0，为0则重绘靶值
//     if (qAbs(val) <= DBL_EPSILON)
//     {
//         QwtText lbl = tickLabel( painter->font(), val );
//         if ( lbl.isEmpty() )
//             return;
// 
//         QPointF pos = labelPosition(val);
//         QSizeF labelSize = lbl.textSize( painter->font() );
//         QRect labelRect = QRect( QPoint( 0, 0 ), labelSize.toSize());
// 
//         const QTransform transform = labelTransformation( pos, labelSize );
// 
//         painter->save();
//         painter->setWorldTransform( transform, true );
//         painter->setPen(Qt::black);
//         painter->drawText(labelRect, Qt::AlignCenter, QString("X"));
//         int textLen = painter->fontMetrics().width(QString("X"));
//         int textHeight = painter->fontMetrics().height();
//         int intervalX = (labelRect.width() - textLen) / 2;
//         int intervalY = (labelRect.height() - textHeight) / 2;
//         painter->drawLine(QPoint(labelRect.left() + intervalX,
//                                  labelRect.top() + intervalY - LINE_TEXT_INTERVAL),
//                           QPoint(labelRect.right() - intervalX,
//                                  labelRect.top() + intervalY - LINE_TEXT_INTERVAL)
//                     );
// 
//         painter->restore();
//     }
//    else
    {
        QwtScaleDraw::drawLabel(painter, val);
    }
}


McCustomIntegerScaleDraw::McCustomIntegerScaleDraw()
{

}

QwtText McCustomIntegerScaleDraw::label(double v) const
{
    // 初始化数值显示
    QString StrOffset("");

    // 如果是整数，则显示整数，否则不显示
    if (qAbs(v - int(v)) <= DBL_EPSILON)
    {
        StrOffset.setNum(int(v));
    }

    // 构造返回值
    QwtText qwtText(StrOffset);
    if (m_color.isValid() && (int(v) == m_iSelVal))
    {
        qwtText.setColor(m_color);
    }

    // 显示刻度值
    return qwtText;
}

void McCustomIntegerScaleDraw::drawLabel(QPainter* painter, double val) const
{
    QwtScaleDraw::drawLabel(painter, val);
    if (m_color.isValid())
    {
        QwtScaleDraw::drawLabel(painter, m_iSelVal);
    }
}

void McCustomIntegerScaleDraw::SetSelLabelColor(int val, const QColor& labelColor)
{
    m_iSelVal = val;
    m_color = labelColor;
}

McCustomColorScaleDraw::McCustomColorScaleDraw()
{
}

QwtText McCustomColorScaleDraw::label(double v) const
{
    // 初始化数值显示
    int i = 0;
    double dTempV = qAbs(v);
    while (dTempV >= 10.0)
    {
        dTempV /= 10.0;
        ++i;
    }
    QString StrOffset = QString::number(v, 'g', i + 7);

    // 构造返回值
    QwtText qwtText(StrOffset);
    if (m_color.isValid())
    {
        qwtText.setColor(m_color);
    }

    // 显示刻度值
    return qwtText;
}

void McCustomColorScaleDraw::drawLabel(QPainter* painter, double val) const
{
    QwtScaleDraw::drawLabel(painter, val);
}

void McCustomColorScaleDraw::SetLabelColor(const QColor& labelColor)
{
    m_color = labelColor;
}
