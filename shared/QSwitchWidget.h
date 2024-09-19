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
/// @file     QSwitchWidget.h
/// @brief    页面切换选择
///
/// @author   5774/WuHongTao
/// @date     2022年2月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "ui_QSwitchWidget.h"

class QNavigationPage;
struct CornerButtonInfo;

class QSwitchWidget : public QWidget
{
    Q_OBJECT

public:
    QSwitchWidget(QWidget *parent = Q_NULLPTR);
    ~QSwitchWidget();

protected:
    ///
    /// @brief
    ///     注册切换后的模块主页面
    ///
    /// @param[in]  index  模块序号
    /// @param[in]  page   页面句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月18日，新建函数
    ///
    void RegisterPage(int index, const QString& name, const QString& cornerbtnObjectName, QNavigationPage* page);

    ///
    /// @brief
    ///     页面初始化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月12日，新建函数
    ///
    virtual void Init() = 0;

private slots:
    ///
    /// @brief
    ///     页面切换槽函数
    ///
    /// @param[in]  index  切换页面的Index
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月12日，新建函数
    ///
    void ChangePage(int index);

protected:
    Ui::QSwitchWidget               ui;
    std::vector<QNavigationPage*>   m_subPages;                 ///< 切换页面的集合
    std::vector<CornerButtonInfo>   m_pageInfos;                ///< 页面的信息合集
};
