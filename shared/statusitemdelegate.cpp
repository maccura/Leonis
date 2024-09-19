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
/// @file     statusitemdelegate.cpp
/// @brief    状态代理
///
/// @author   4170/TangChuXian
/// @date     2021年8月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年8月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "statusitemdelegate.h"
#include "shared/uidef.h"
#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QStyle>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QApplication>

#define BACK_GROUND_COLOR_NORMAL                 (QColor(0xff, 0xff, 0xff)) // 背景色正常
#define BACK_GROUND_COLOR_WARNING                (QColor(0xff, 0xe6, 0xba)) // 背景色警告
#define BACK_GROUND_COLOR_ERROR                  (QColor(0xff, 0xda, 0xd6)) // 背景色错误
#define BACK_GROUND_COLOR_INVALID                (QColor(0xed, 0xee, 0xf0)) // 背景色不可用

#define BACK_GROUND_COLOR_SEL                    (QColor(2, 56, 255, 25))   // 背景色选中

#define BACK_GROUND_COLOR_WORKING                (QColor(0xe1, 0xfa, 0xe3)) // 背景色运行状态

#define TEXT_LEFT_RIGHT_PADDING                  (5)                        // 文本左右外距

StatusItemDelegate::StatusItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

StatusItemDelegate::~StatusItemDelegate()
{
}

///
/// @brief
///     绘制displaymode的时候的函数
///
/// @param[in]  painter  图形绘制句柄
/// @param[in]  option  选项
/// @param[in]  index  索引
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月9日，新建函数
///
void StatusItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
    // 如果选中画选中效果
    // 获取自定义值
    BackGroundState enState = (BackGroundState)index.data(BK_STATE_ROLE).toInt();

    // 根据状态设置填充色
    QColor fillColor;
    if (enState == STATE_NORMAL)
    {
        fillColor = BACK_GROUND_COLOR_NORMAL;
    }
    else if (enState == STATE_WARNING)
    {
        fillColor = BACK_GROUND_COLOR_WARNING;
        QAbstractItemModel* pModel = (QAbstractItemModel*)(index.model());
        pModel->blockSignals(true);
        pModel->setData(index, QColor(0xed, 0xb2, 0x00), Qt::TextColorRole);
        pModel->blockSignals(false);
    }
    else if (enState == STATE_ERROR)
    {
        fillColor = BACK_GROUND_COLOR_ERROR;
        QAbstractItemModel* pModel = (QAbstractItemModel*)(index.model());
        pModel->blockSignals(true);
        pModel->setData(index, QColor(0xff, 0x56, 0x5f), Qt::TextColorRole);
        pModel->blockSignals(false);
    }
    else if (enState == STATE_WORK)
    {
        fillColor = BACK_GROUND_COLOR_WORKING;
    }
    else if (enState == STATE_INVALID)
    {
        fillColor = BACK_GROUND_COLOR_INVALID;
    }

    // 画填充色
    if (enState != STATE_NORMAL)
    {
        // del_tcx 不再画填充色
        //painter->fillRect(option.rect, fillColor);
    }

    // 画表头背景色
    QVariant varHeader = index.data(BK_HEADER_ROLE);
    if (varHeader.isValid())
    {
        QColor bkColor = varHeader.value<QColor>();
        painter->fillRect(option.rect, bkColor);
    }

    // 获取填充区域
    QRect fillRect = option.rect;
    fillRect.setWidth(fillRect.width() - 1);
    fillRect.setHeight(fillRect.height() - 1);
    QPoint ptTopLeft(fillRect.left() + 1, fillRect.top());
    fillRect.moveTopLeft(ptTopLeft);

    // 绘制文字
    auto text = index.data(Qt::DisplayRole).toString();

    // 绘制文字右上角标识
    auto flag = index.data(BK_FLAG_ROLE);
    if (flag.isValid())
    {
        // 绘制标志
        if (flag.canConvert<QPixmap>())
        {
            // 获取图片
            auto flagPix = flag.value<QPixmap>();

            // 字体宽度
            int fontWidth = option.fontMetrics.width(text);

            // 绘制位置
            QPoint flagPoint = fillRect.center();
            flagPoint.setX(flagPoint.x() + fontWidth / 2);
            flagPoint.setY(flagPoint.y() - flagPix.height());

            painter->drawPixmap(flagPoint, flagPix);
        }
    }

    // 省略号在左
    QStyleOptionViewItem optionNew = option;
    optionNew.textElideMode = Qt::ElideLeft;

    // 绘制内容和背景
    QColor textColor = index.data(Qt::TextColorRole).value<QColor>();

    // 绘制特殊背景
    bool isSelected = option.state & QStyle::State_Selected;
    if (!isSelected || text.isEmpty() || !textColor.isValid())
    {
        QStyledItemDelegate::paint(painter, optionNew, index);
        return;
    }

    // 选中特殊处理
    QAbstractItemModel* pModel = (QAbstractItemModel*)index.model();
    if (pModel == Q_NULLPTR)
    {
        return;
    }

    pModel->blockSignals(true);
    pModel->setData(index, "", Qt::DisplayRole);
    QStyledItemDelegate::paint(painter, optionNew, index);
    painter->save();
    QPen curPen = painter->pen();
    curPen.setColor(textColor);
    painter->setPen(curPen);
    QApplication::style()->drawItemText(painter, option.rect, Qt::AlignCenter, textColor, true, text);
    painter->restore();
    pModel->setData(index, text, Qt::DisplayRole);
    pModel->blockSignals(false);
}
