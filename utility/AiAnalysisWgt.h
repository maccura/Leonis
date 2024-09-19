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
/// @file     AiAnalysisWgt.h
/// @brief    应用->AI识别设置界面
///
/// @author   4170/TangChuXian
/// @date     2023年11月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <QWidget>

namespace Ui {
    class AiAnalysisWgt;
};

class QFaultLogWidget;

class AiAnalysisWgt : public QWidget
{
    Q_OBJECT

public:
    AiAnalysisWgt(QWidget *parent = Q_NULLPTR);
    ~AiAnalysisWgt();

private:
    ///
    /// @brief 初始化日志展示模块
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年7月5日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     清空数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月26日，新建函数
    ///
    void ClearData();

    ///
    /// @brief
    ///     加载数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月3日，新建函数
    ///
    void LoadData();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月15日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月17日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:
    ///
    /// @bref
    ///        权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月25日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @brief
    ///     保存按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月15日，新建函数
    ///
    void OnSaveBtnClicked();

private:
    Ui::AiAnalysisWgt*              ui;
    bool                            m_bInit;        // 是否初始化
};
