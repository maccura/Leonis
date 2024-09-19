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
/// @file     qctabwidget.h
/// @brief    质控tab界面
///
/// @author   4170/TangChuXian
/// @date     2022年12月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QTabWidget>

class QcTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    QcTabWidget(QWidget *parent);
    ~QcTabWidget();

protected:
    ///
    /// @brief
    ///     显示之前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    void InitBeforeShow();
    
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月14日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月14日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:
    ///
    /// @brief
    ///     tab页面角落窗口更新
    ///
    /// @param[in]  strDevNameList  设备列表
    /// @param[in]  iSelDevIdx      当前选中设备索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    void OnTabCornerUpdate(QStringList strDevNameList, int iSelDevIdx);

    ///
    /// @brief
    ///     角落单元按钮被点击
    ///
    /// @param[in]  iBtnIdx 按钮索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    void OnCornerRBtnClicked(int iBtnIdx);

    ///
    /// @brief
    ///     tab索引改变（临时添加函数，后续删除）
    ///
    /// @param[in]  iTabIdx  tab索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月22日，新建函数
    ///
    void OnTabInexChanged(int iTabIdx);

    ///
    /// @brief
    ///     查询条件重置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月15日，新建函数
    ///
    void OnQryCondReset();

private:
    bool            m_bInit;                // 是否已经初始化
    QWidget*        m_pCornerWidget;        // 角落窗口
};
