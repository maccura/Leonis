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
/// @file     ReagentAssayCard.h
/// @brief    试剂项目卡片信息
///
/// @author   5774/WuHongTao
/// @date     2021年12月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QFrame>
#include "ReagentCommon.h"

namespace Ui {
    class ReagentAssayCard;
};

#define MODULEMAX 6

class QMouseEvent;
struct CardShowInfo;

class ReagentAssayCard : public QFrame
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  窗口父指针
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    ReagentAssayCard(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    ~ReagentAssayCard();

    ///
    /// @brief
    ///     初始化框体的一些默认属性
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月30日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     设置项目名称
    ///
    /// @param[in]  assayName  项目名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月30日，新建函数
    ///
    void SetAssayName(const QString& assayName);

    ///
    /// @brief
    ///     设置模块对应的信息
    ///
    /// @param[in]  seq  模块序号（必须<=6 ，>=0）
    /// @param[in]  textForModule  (具体的信息)
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月30日，新建函数
    ///
    void SetModuleText(int seq, const QString& textForModule);

    ///
    /// @brief
    ///     设置测试余量
    ///
    /// @param[in]  testTimes  余量还能使用多少次
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月30日，新建函数
    ///
    void SetLastReagentVolumn(int testTimes);

    ///
    /// @brief
    ///     设置项目的状态
    ///
    /// @param[in]  state  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月30日，新建函数
    ///
    void SetReagentState(USESTATE state);

    ///
    /// @brief
    ///     清除卡片显示信息恢复到初始
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月1日，新建函数
    ///
    void ClearCard();

    ///
    /// @brief
    ///     设置卡片信息
    ///
    /// @param[in]  cardInfo  卡片信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月12日，新建函数
    ///
    void SetCardInfo(const CardShowInfo& cardInfo);
protected:
    ///
    /// @brief
    ///     重写鼠标按键处理事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    void mousePressEvent(QMouseEvent *event);

private:
    ///
    /// @brief
    ///     设置控件的背景色
    ///
    /// @param[in]  dstObj  目标控件
    /// @param[in]  dstColor  目标色彩
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    void SetBackGroudColor(QWidget* dstObj, QColor dstColor);

    ///
    /// @brief
    ///     测试项目卡片功能，仅供内部测试使用
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月30日，新建函数
    ///
    void TestCardParamter();

private:
    Ui::ReagentAssayCard*            ui;
};
