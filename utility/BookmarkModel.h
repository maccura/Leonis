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
/// @file     BookmarkModel.h
/// @brief    pdf目录模型
///
/// @author   6889/ChenWei
/// @date     2024年1月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QAbstractItemModel>
#include <QModelIndex>
#include <memory>
#include "PdfPretreatment.h"

class Bookmark;
class BookmarkModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    BookmarkModel(QObject *parent = Q_NULLPTR);
    ~BookmarkModel();

    // 返回节点数据
    QVariant data(const QModelIndex &index, int role) const override;

    // 返回节点显示设置
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // 返回表头数据
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 返回给定行列的序号
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    // 返回父节点
    QModelIndex parent(const QModelIndex &index) const override;

    // 行数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // 列数
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // 构建树
    void BuildTree();

    // 获取指定节点的页面序号
    int GetPageIndex(const QModelIndex &index);
private:

    // 创建节点
    Bookmark* MakeBookmark(FPDF_BOOKMARK bookmark, Bookmark* parent);
private:
    Bookmark* rootItem;
};
