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
/// @file     mcpixmapitemdelegate.h
/// @brief    表格图片居中显示代理类
///
/// @author   6889/ChenWei
/// @date     2021年12月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2021年12月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "mcpixmapitemdelegate.h"
#include <QPainter>
#include <QIcon>

McPixmapItemDelegate::McPixmapItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_ciIconMaxSize(100)
{
}

McPixmapItemDelegate::~McPixmapItemDelegate()
{
}

///
/// @brief
///     重写paint函数，在表格item中居中绘制图片
///
/// @par History:
/// @li 6889/ChenWei，2021年12月10日，新建函数
///
void McPixmapItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data(Qt::DecorationRole).canConvert<QIcon>())
    {
        painter->setRenderHints(QPainter::Antialiasing);
        QIcon lIcon = index.data(Qt::DecorationRole).value<QIcon>();
        QSize iconSize = lIcon.actualSize(QSize(m_ciIconMaxSize, m_ciIconMaxSize));
        QImage image = lIcon.pixmap(iconSize).toImage();

        // 修正画图坐标，使图片显示在表格中央
        QRect _rect = option.rect;
        int x = _rect.x() + _rect.width() / 2 - image.width() / 2;
        int y = _rect.y() + _rect.height() / 2 - image.height() / 2;
        painter->drawImage(x, y, image);
    }

    // 基类不画图标
    QVariant backVarData = index.data(Qt::DecorationRole);
    QAbstractItemModel* pModel = (QAbstractItemModel*)index.model();
    if (pModel != Q_NULLPTR && backVarData.isValid())
    {
        pModel->blockSignals(true);
        pModel->setData(index, QVariant(), Qt::DecorationRole);
        QStyledItemDelegate::paint(painter, option, index);
        pModel->setData(index, backVarData, Qt::DecorationRole);
        pModel->blockSignals(false);
        return;
    }

    // 正常绘制
    QStyledItemDelegate::paint(painter, option, index);
};
