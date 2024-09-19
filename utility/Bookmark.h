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
/// @file     BookMark.h
/// @brief    pdf目录标签
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
#include <QString>
#include <QList>
#include <memory>
#include "fpdfview.h"

class Bookmark
{
public:
    Bookmark(const QString &data, Bookmark* parentItem = nullptr)
        : m_strTitle(data)
        , m_Bookmark(nullptr)
        , m_pParent(parentItem)
    {
    };

    ~Bookmark()
    {
        qDeleteAll(m_Childs);
    };

    // 获取标题
    inline QString Title() {
        return m_strTitle;
    }

    // 获取父节点
    inline Bookmark* Parent()
    {
        return m_pParent;
    }

    // 获取子节点
    inline Bookmark* Child(int row){
        if (row >= m_Childs.count())
            return nullptr;

        return m_Childs.at(row);
    }

    // 子节点数量
    inline int ChildCount(){
        return m_Childs.count();
    }

    // 设置pdf书签
    inline void SetBookmark(FPDF_BOOKMARK bm){
        m_Bookmark = bm;
    }

    // 获取pdf书签
    inline FPDF_BOOKMARK GetBookmark()
    {
        return m_Bookmark;
    }

    // 添加子节点
    inline void AddChildItem(Bookmark* Item) {
        m_Childs.append(Item);
    }

    // 清除子节点
    inline void ClearChild() {
        qDeleteAll(m_Childs);
    }

    // 获取自身行数
    int row() const
    {
        if (m_pParent != nullptr)
            return m_pParent->m_Childs.indexOf(const_cast<Bookmark*>(this));

        return 0;
    }

private:
    FPDF_BOOKMARK m_Bookmark;
    QString m_strTitle;
    QList<Bookmark*> m_Childs;
    Bookmark* m_pParent;
};
