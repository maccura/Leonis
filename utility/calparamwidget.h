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
/// @file     calparamwidget.h
/// @brief    应用界面->项目设置界面->校准参数设置界面
///
/// @author   4170/TangChuXian
/// @date     2020年8月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
namespace Ui { class CalParamWidget; };

class CalParamWidget : public QWidget
{
    Q_OBJECT

public:
    CalParamWidget(QWidget *parent = Q_NULLPTR);
    ~CalParamWidget();

    ///
    /// @brief
    ///     加载指定项目的校准参数
    ///
    /// @param[in]  db_no  指定项目的数据库主键
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月26日，新建函数
    ///
    void LoadCalParam(long long db_no);

    ///
    /// @brief
    ///     保存指定项目的校准参数
    ///
    /// @param[in]  db_no  指定项目的数据库主键
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月26日，新建函数
    ///
    void SaveCalParam(long long db_no);

protected:
    ///
    /// @brief 界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月8日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月26日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月26日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月26日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月26日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     清空参数内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月26日，新建函数
    ///
    void ClearText();

private:
    Ui::CalParamWidget      *ui;                        // UI指针对象
    bool                     m_bInit;                   // 是否已经初始化

    // 设置友元类
    friend class             AssayConfigWidget;         // 将仪器设置窗口设置为友元类
};
