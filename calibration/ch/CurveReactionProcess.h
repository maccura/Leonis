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
/// @file     CurveReactionProcess.cpp
/// @brief    反应过程的文件
///
/// @author   5774/WuHongTao
/// @date     2021年11月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "ui_CurveReactionProcess.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "CaliBrationStatusPage.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/leonis/thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/thrift/ch/c1005/gen-cpp/C1005LogicControl.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

#define  defaultRow 17

// 前置声明
class CaliBrationStatusPage;

class CurveReactionProcess : public BaseDlg
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  父指针
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    CurveReactionProcess(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    ~CurveReactionProcess();

    ///
    /// @brief
    ///     展示项目校准的反应过程
    ///
    /// @param[in]  db  试剂组的主键
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    void StartShowAssayCaliBrateAction(int db);

    ///
    /// @brief
    ///     初始化表格
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月24日，新建函数
    ///
    void Init();

private:
    ///
    /// @brief
    ///     显示项目状态信息
    ///
    /// @param[in]  group  试剂组信息
    /// @param[in]  curve  校准曲线的信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    void ShowAssayStatusInfo(ch::tf::ReagentGroup& group, ch::tf::CaliCurve& curve);

private slots:
    ///
    /// @brief
    ///     水平变化
    ///
    /// @param[in]  index  变化后对应的索引
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    void OnLevelChanged(int index);

    ///
    /// @brief
    ///     点击关闭按钮的槽函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    void OnCloseBtn();

private:
    Ui::CurveReactionProcess        ui;                         ///< 反应过程界面句柄
    CaliBrationStatusPage*          m_parent;                   ///< 保存父指针以备查询
};
