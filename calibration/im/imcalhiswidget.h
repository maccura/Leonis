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
/// @file     imcalhiswidget.h
/// @brief    免疫校准历史界面
///
/// @author   4170/TangChuXian
/// @date     2022年9月5日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include <memory>
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui { class ImCalHisWidget; };

class ImCalHisWidget : public QWidget
{
    Q_OBJECT

public:
    ImCalHisWidget(QWidget *parent = Q_NULLPTR);
    ~ImCalHisWidget();

    ///
    /// @brief
    ///     切换设备刷新界面
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月08日，新建函数
    ///
    void RefreshPage(std::vector<std::shared_ptr<tf::DeviceInfo>>& devices);

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月17日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月17日，新建函数
    ///
    void InitConnect();

private:
    Ui::ImCalHisWidget   *ui;                   // ui对象指针
    bool                  m_bInit;              // 是否初始化

    // 设备相关
    QString               m_strDevSn;           // 设备序列号
};
