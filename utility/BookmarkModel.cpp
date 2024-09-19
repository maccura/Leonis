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
/// @file     BookmarkModel.cpp
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
#include "BookmarkModel.h"
#include "Bookmark.h"

BookmarkModel::BookmarkModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new Bookmark("Title");
}

BookmarkModel::~BookmarkModel()
{
    delete rootItem;
}

///
/// @brief
///     返回节点数据
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
QVariant BookmarkModel::data(const QModelIndex &index, int role) const 
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Bookmark *item = static_cast<Bookmark*>(index.internalPointer());

    return (item->Title());
}

///
/// @brief
///     返回节点显示设置
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
Qt::ItemFlags BookmarkModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

///
/// @brief
///     返回表头数据
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
QVariant BookmarkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return (rootItem->Title());

    return QVariant();
}

///
/// @brief
///     返回给定位置序号
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
QModelIndex BookmarkModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Bookmark* parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Bookmark*>(parent.internalPointer());

    Bookmark* childItem = parentItem->Child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

///
/// @brief
///     返回父节点
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
QModelIndex BookmarkModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Bookmark *childItem = static_cast<Bookmark*>(index.internalPointer());
    Bookmark *parentItem = childItem->Parent();

    if (parentItem == rootItem || parentItem == nullptr)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

///
/// @brief
///     行数
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
int BookmarkModel::rowCount(const QModelIndex &parent) const
{
    Bookmark *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Bookmark*>(parent.internalPointer());

    return parentItem->ChildCount();
}

///
/// @brief
///     列数
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
int BookmarkModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

///
/// @brief
///     创建节点
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
Bookmark* BookmarkModel::MakeBookmark(FPDF_BOOKMARK bookmark, Bookmark* parent)
{
    QString title = PdfPretreatment::GetInstance()->GetTitle(bookmark);
    Bookmark* Item = new Bookmark(title, parent);
    FPDF_BOOKMARK child = PdfPretreatment::GetInstance()->GetFirstChild(bookmark);
    while (child)
    {
        Bookmark* childItem = MakeBookmark(child, Item);
        childItem->SetBookmark(child);
        Item->AddChildItem(childItem);
        child = PdfPretreatment::GetInstance()->GetNextSibling(child);
    }

    return Item;
}

///
/// @brief
///     构建目录
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void BookmarkModel::BuildTree()
{
    rootItem->ClearChild();
    FPDF_BOOKMARK emptyBM = NULL;
    Bookmark* Item = MakeBookmark(emptyBM, nullptr);
    if (Item)
    {
        delete rootItem;
        rootItem = Item;
    }
}

int BookmarkModel::GetPageIndex(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return -1;
    }

    Bookmark *item = static_cast<Bookmark*>(index.internalPointer());
    int iPageIndex = PdfPretreatment::GetInstance()->GetDestPageIndex(item->GetBookmark());
    return iPageIndex;
}



