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
/// @file     QNavigationPage.h
/// @brief    导航界面
///
/// @author   5774/WuHongTao
/// @date     2022年2月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "ui_QNavigationPage.h"

struct CornerButtonInfo;
namespace Ui {
    class QNavigationPage;
};

#define TAB_INDEX_CHEMISTRY                              (0)                       // 生化索引
#define TAB_INDEX_IMMUNITY                               (1)                       // 免疫索引

class QNavigationPage : public QWidget
{
    Q_OBJECT

public:
    QNavigationPage(QWidget *parent = Q_NULLPTR);
    ~QNavigationPage();

    ///
    /// @brief
    ///     设置修改过滤条件
    ///
    /// @param[in]  cond  条件是否显示
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void SetShowFilterCond(bool cond);

    ///
    /// @brief
    ///     设置页面需要显示的按钮等，名称和对应的切换Index
    ///  
    /// @param[in]  pageInfos  按钮信息队列
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月12日，新建函数
    ///
    virtual void SetPageInfos(const std::vector<CornerButtonInfo>& pageInfos) = 0;

Q_SIGNALS:
    ///
    /// @brief
    ///     页面改变信号
    ///
    /// @param[in]  index  切换到的页面Index
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月12日，新建函数
    ///
    void ChangePage(int index);

protected slots:
    ///
    /// @brief
    ///     刷新右上角按钮
    ///
    /// @param[in]  index  刷新页面index
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月12日，新建函数
    ///
    virtual void RefreshCornerWidgets(int index);

    ///
    /// @brief
    ///     设置过滤条件数目
    ///
    /// @param[in]  condNumber  条件数目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void OnFilterCondNumber(QString message);

    ///
    /// @brief 隐藏快捷筛选标签
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2023年8月16日，添加注释
    ///
    void OnHideFilterLable();

protected:
    Ui::QNavigationPage*             ui;
    std::pair<int, bool>            m_status;
};
