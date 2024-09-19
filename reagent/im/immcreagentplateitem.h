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
/// @file     immcreagentplateitem.h
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

#pragma once

#include <QGraphicsItem>
#include <QSet>
#include <QFont>

// 试剂盘item
class ImMcReagentPlateItem : public QGraphicsItem
{
public:
    ImMcReagentPlateItem(QGraphicsItem *parent = Q_NULLPTR);

    ~ImMcReagentPlateItem();

    ///
    /// @brief
    ///     边界矩形
    ///
    /// @param
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月15日，新建函数
    ///
    QRectF boundingRect() const;

    ///
    /// @brief
    ///     重绘函数
    ///
    /// @param
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月15日，新建函数
    ///
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

    ///
    /// @brief
    ///     设置试剂盘绘制区域
    ///
    /// @param[in] rect  矩形区域
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    void SetReagentPlateRect(const QRect& rect);

    ///
    /// @brief
    ///     设置试剂文本绘制区域
    ///
    /// @param[in] rect  矩形区域
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    void SetReagentPlateTextRect(const QRect& rect);

    ///
    /// @brief
    ///     设置字体
    ///
    /// @param[in] font  字体
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    void SetFont(const QFont& font);

    ///
    /// @brief
    ///     设置文本内容
    ///
    /// @param[in] text  文本内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    void SetText(const QString& text);

    ///
    /// @brief
    ///     设置文本颜色
    ///
    /// @param[in] textColor  文本颜色
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月16日，新建函数
    ///
    void SetTextColor(const QColor& textColor);

    ///
    /// @brief
    ///     获取槽位个数
    ///
    /// @return 槽位个数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月12日，新建函数
    ///
    int GetSlotCnt();

    ///
    /// @brief
    ///     设置槽位个数
    ///
    /// @param[in]  iCnt  槽位个数

    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月12日，新建函数
    ///
    void SetSlotCnt(int iCnt);

    ///
    /// @brief
    ///     添加指示器索引
    ///
    /// @param[in]  iIndex  指示索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月12日，新建函数
    ///
    void AddIndicatorIndex(int iIndex);

    ///
    /// @brief
    ///     移除指示索引
    ///
    /// @param[in]  iIndex  指示索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月12日，新建函数
    ///
    void RemoveIndicatorIndex(int iIndex);
    
    ///
    /// @brief
    ///     清空指示器索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月12日，新建函数
    ///
    void ClearIndicatorIndex();

    ///
    /// @brief
    ///     设置指示器集合
    ///
    /// @param[in]  setIndicator  指示器集合
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年8月12日，新建函数
    ///
    void SetIndicatorSet(const QSet<int>& setIndicator);

private:
    int         m_iSlotCnt;               // 槽位个数
    QRect       m_ReagentPlateRect;       // 试剂盘绘制区域
    QRect       m_ReagentTextRect;        // 试剂详情绘制区域
    QFont       m_font;                   // 字体
    QString     m_strText;                // 文本内容
    QColor      m_TextColor;              // 文本颜色
    QSet<int>   m_indicatorSet;           // 指示器集合
};
