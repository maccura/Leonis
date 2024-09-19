﻿/***************************************************************************
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
/// @file     TrkMagnetModuleWgt.h
/// @brief    应用-节点调试-轨道运输模块
///
/// @author   8110/wutong
/// @date     2024年5月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8110/wutong，2024年5月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#ifndef TRKROUTERDEBUGWGT_H
#define TRKROUTERDEBUGWGT_H

//#pragma execution_character_set("utf-8")

#include <QWidget>
#include "iBITDebugger.h"

class TrkStepModuleWgt;              // 运动模块界面
class TrkMagnetModuleWgt;            // 电磁铁模块界面

namespace Ui {
class TrkRouterDebugWgt;
}

namespace Router{

enum StackPageIdx
{
    STACK_PAGE_IDX_STEP = 0,
    STACK_PAGE_IDX_ELECTROMAGNET
};

}

class TrkRouterDebugWgt : public QWidget
{
    Q_OBJECT

public:
    explicit TrkRouterDebugWgt(QWidget *parent = nullptr);
    ~TrkRouterDebugWgt();

    ///
    /// @brief  更新界面信息
    ///
    /// @param[in]
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月10日，新建函数
    ///
    void UpdateUi();

    ///
    /// @brief 更新模块列表
    ///
    /// @param[in] info 模块元件信息
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月10日，新建函数
    ///
    void UpdateModuleList(ModuleInfo& info);

protected:
    ///
    /// @brief 窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月10日，新建函数
    ///
    void showEvent(QShowEvent *event);

private:
    ///
    /// @brief	初始化UI
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月10日，新建函数
    ///
    void InitUi();

    ///
    /// @brief	初始化数据
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月10日，新建函数
    ///
    void InitData();

    ///
    /// @brief	初始化连接
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月10日，新建函数
    ///
    void InitConnect();

protected Q_SLOTS:
    ///
    /// @brief  选中项改变
    ///
    /// @par History:
    /// @li 8110/wutong，2024年5月9日，新建函数
    ///
    void OnSelectListItem();

private:
    Ui::TrkRouterDebugWgt *ui;
    bool m_isInit;
    TrkStepModuleWgt* m_stepModuleWgt = nullptr;
    TrkMagnetModuleWgt* m_magnetModuleWgt = nullptr;
};

#endif // TRKROUTERDEBUGWGT_H