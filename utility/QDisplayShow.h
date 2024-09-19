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

#pragma once

#include <QWidget>

namespace Ui {
    class QDisplayShow;
};


class QDisplayShow : public QWidget
{
    Q_OBJECT

public:
    QDisplayShow(QWidget *parent = Q_NULLPTR);
    ~QDisplayShow();

    ///
    /// @brief	当前界面是否有未保存的数据
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年03月14日，新建函数
    ///
    bool isExistChangedData();

private:
    void Init();

    ///
    /// @brief	初始化语言下拉框
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年11月14日，新建函数
    ///
    void InitLanguageCombox();

    void showEvent(QShowEvent *event);

    ///
    /// @brief	是否存在未保存的语言设置或时间格式设置
    ///
    /// @param[in]  isLanuage 是否为语言设置
    ///     
    /// @par History:
    /// @li 7951/LuoXin，202年3月14日，新建函数
    ///
    bool isExistUnSaveLanuageOrTimeFormat(bool isLanuage);

    ///
    /// @brief 事件过滤器（处理tabbar的点击事件，当前页面存在未保存的数据，切换时提示用户）
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年3月13日，新建函数
    /// 
    bool eventFilter(QObject *obj, QEvent *event) override;

    protected Q_SLOTS:

    ///
    /// @brief	更新日期格式和语言
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年7月3日，新建函数
    ///
    void UpdateDateAndLanuage();

    ///
    /// @brief	保存日期格式和语言
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年7月3日，新建函数
    ///
    void OnSaveDateAndLanuage();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月24日，新建函数
    ///
    void OnPermisionChanged();

private:
    Ui::QDisplayShow* ui;
    bool             isInit;
};
