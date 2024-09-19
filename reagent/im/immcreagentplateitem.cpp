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
/// @file     mcreagentplateitem.cpp
/// @brief    试剂盘item
///
/// @author   4170/TangChuXian
/// @date     2020年4月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "immcreagentplateitem.h"
#include <QPainter>
#include <QGraphicsScene>

#define REAGENT_PLATE_INNER_CIRCLE_RECT     QRect(198, 196, 309, 309)               // 试剂盘内圆绘制区域
#define REAGENT_PLATE_SLOT_INDEX_TEXT_RECT  QRect(-9, -206, 21, 12)                 // 试剂盘槽位索引文本文本绘制区域
#define REAGENT_PLATE_INDICATOR_RECT        QRect(-5, -187, 13, 10)                 // 试剂盘槽位索引文本文本绘制区域
#define REAGENT_PLATE_TEXT_COLOR            QColor(0x56, 0x56, 0x56)                // 试剂盘文本颜色
#define REAGENT_PLATE_INDICATOR_COLOR       QColor(0x6a, 0xa8, 0xf2)                // 试剂盘指示器颜色

ImMcReagentPlateItem::ImMcReagentPlateItem(QGraphicsItem *parent)
          :QGraphicsItem(parent),
           m_iSlotCnt(0),
           m_strText("C2"),
           m_TextColor(REAGENT_PLATE_TEXT_COLOR)
{
}

ImMcReagentPlateItem::~ImMcReagentPlateItem()
{
}

///
/// @brief
///     边界矩形
///
/// @param
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月15日，新建函数
///
QRectF ImMcReagentPlateItem::boundingRect() const
{
    return m_ReagentPlateRect;
}

///
/// @brief
///     边界矩形
///
/// @param
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月15日，新建函数
///
void ImMcReagentPlateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /*= Q_NULLPTR*/)
{
    // 保存绘制上下文
    painter->save();

    // 反走样
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    // 画试剂盘
    // 画外圆
    QPainterPath pathOutCircle;
    {
        QRegion innerRegion(6, 6, m_ReagentPlateRect.width() - 12, m_ReagentPlateRect.height() - 12, QRegion::Ellipse);
        QRegion outRegion(m_ReagentPlateRect, QRegion::Ellipse);
        pathOutCircle.addRegion(outRegion.subtracted(innerRegion));
    }
    painter->fillPath(pathOutCircle, QColor(0xe6, 0xea, 0xf4));

    // 画内圆
    QPainterPath pathInnerCircle;
    {
        QRegion innerRegion(REAGENT_PLATE_INNER_CIRCLE_RECT.x() + 20, REAGENT_PLATE_INNER_CIRCLE_RECT.y() + 20, REAGENT_PLATE_INNER_CIRCLE_RECT.width() - 40, REAGENT_PLATE_INNER_CIRCLE_RECT.height() - 40, QRegion::Ellipse);
        QRegion outRegion(REAGENT_PLATE_INNER_CIRCLE_RECT, QRegion::Ellipse);
        pathInnerCircle.addRegion(outRegion.subtracted(innerRegion));
    }
    painter->fillPath(pathInnerCircle, QColor(0xf2, 0xf4, 0xf9));

    // 画文本
    QPen myPen = painter->pen();
    myPen.setCapStyle(Qt::RoundCap);
    myPen.setJoinStyle(Qt::RoundJoin);
    myPen.setWidth(1);
    myPen.setColor(QColor(0xe6, 0xea, 0xf4));

    // 设置画笔字体
    m_font.setPixelSize(20);
    m_font.setWeight(QFont::Bold);
    myPen.setColor(m_TextColor);
    painter->setFont(m_font);
    painter->setPen(myPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(REAGENT_PLATE_INNER_CIRCLE_RECT, Qt::AlignHCenter | Qt::AlignVCenter, m_strText);

    // 画试剂盒索引（数字）
    if (m_iSlotCnt > 0)
    {
        m_font.setPixelSize(12);
        m_font.setWeight(QFont::Normal);
        myPen.setColor(m_TextColor);
        painter->setPen(myPen);
        painter->setFont(m_font);
        painter->translate(350.0, 350.0);

        double dAngleStep = -360.0 / m_iSlotCnt;
        for (int i = 1; i <= m_iSlotCnt; i++)
        {
            if (i > 1)
            {
                painter->translate(REAGENT_PLATE_SLOT_INDEX_TEXT_RECT.center());
                painter->rotate((i - 1) * (-dAngleStep));
                painter->translate(-REAGENT_PLATE_SLOT_INDEX_TEXT_RECT.center().x(), -REAGENT_PLATE_SLOT_INDEX_TEXT_RECT.center().y());
                painter->drawText(REAGENT_PLATE_SLOT_INDEX_TEXT_RECT, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(i));
                painter->translate(REAGENT_PLATE_SLOT_INDEX_TEXT_RECT.center());
                painter->rotate((i - 1) * (dAngleStep));
                painter->translate(-REAGENT_PLATE_SLOT_INDEX_TEXT_RECT.center().x(), -REAGENT_PLATE_SLOT_INDEX_TEXT_RECT.center().y());
            }
            else
            {
                painter->drawText(REAGENT_PLATE_SLOT_INDEX_TEXT_RECT, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(i));
            }

            // 画指示器
            if (m_indicatorSet.find(i) != m_indicatorSet.end())
            {
                // 画三角形
                QPolygon triAngle;
                triAngle.setPoints(3, REAGENT_PLATE_INDICATOR_RECT.left(), REAGENT_PLATE_INDICATOR_RECT.bottom(),
                    REAGENT_PLATE_INDICATOR_RECT.center().x(), REAGENT_PLATE_INDICATOR_RECT.top(),
                    REAGENT_PLATE_INDICATOR_RECT.right(), REAGENT_PLATE_INDICATOR_RECT.bottom());

                myPen = painter->pen();
                painter->setPen(Qt::NoPen);
                painter->setBrush(REAGENT_PLATE_INDICATOR_COLOR);
                painter->drawPolygon(triAngle);

                // 画家对象还原
                painter->setPen(myPen);
                painter->setBrush(Qt::NoBrush);
            }

            // 旋转到下一刻度
            painter->rotate(dAngleStep);
        }
    }

    // 恢复
    painter->restore();
}

///
/// @brief
///     设置试剂盘绘制区域
///
/// @param[in] rect  矩形区域
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月16日，新建函数
///
void ImMcReagentPlateItem::SetReagentPlateRect(const QRect& rect)
{
    m_ReagentPlateRect = rect;
}

///
/// @brief
///     设置试剂文本绘制区域
///
/// @param[in] rect  矩形区域
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月16日，新建函数
///
void ImMcReagentPlateItem::SetReagentPlateTextRect(const QRect& rect)
{
    m_ReagentTextRect = rect;
}

///
/// @brief
///     设置字体
///
/// @param[in] font  字体
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月16日，新建函数
///
void ImMcReagentPlateItem::SetFont(const QFont& font)
{
    m_font = font;
}

///
/// @brief
///     设置文本内容
///
/// @param[in] text  文本内容
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月16日，新建函数
///
void ImMcReagentPlateItem::SetText(const QString& text)
{
    m_strText = text;
}

///
/// @brief
///     设置文本颜色
///
/// @param[in] font  文本颜色
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月16日，新建函数
///
void ImMcReagentPlateItem::SetTextColor(const QColor& textColor)
{
    m_TextColor = textColor;
}

///
/// @brief
///     获取槽位个数
///
/// @return 槽位个数
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月12日，新建函数
///
int ImMcReagentPlateItem::GetSlotCnt()
{
    return m_iSlotCnt;
}

///
/// @brief
///     设置槽位个数
///
/// @param[in]  iCnt  槽位个数

///
/// @par History:
/// @li 4170/TangChuXian，2022年8月12日，新建函数
///
void ImMcReagentPlateItem::SetSlotCnt(int iCnt)
{
    if (iCnt < 0)
    {
        return;
    }

    m_iSlotCnt = iCnt;
}

///
/// @brief
///     添加指示器索引
///
/// @param[in]  iIndex  指示索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月12日，新建函数
///
void ImMcReagentPlateItem::AddIndicatorIndex(int iIndex)
{
    if (iIndex <= 0 || iIndex > m_iSlotCnt)
    {
        return;
    }

    m_indicatorSet.insert(iIndex);
}

///
/// @brief
///     移除指示索引
///
/// @param[in]  iIndex  指示索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月12日，新建函数
///
void ImMcReagentPlateItem::RemoveIndicatorIndex(int iIndex)
{
    if (iIndex <= 0 || iIndex > m_iSlotCnt)
    {
        return;
    }

    m_indicatorSet.remove(iIndex);
}

///
/// @brief
///     清空指示器索引
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月12日，新建函数
///
void ImMcReagentPlateItem::ClearIndicatorIndex()
{
    m_indicatorSet.clear();
}

///
/// @brief
///     设置指示器集合
///
/// @param[in]  setIndicator  指示器集合
///
/// @par History:
/// @li 4170/TangChuXian，2022年8月12日，新建函数
///
void ImMcReagentPlateItem::SetIndicatorSet(const QSet<int>& setIndicator)
{
    m_indicatorSet = std::move(setIndicator);
}
