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
/// @file     qryconddisplaywgt.h
/// @brief    查询条件显示控件
///
/// @author   4170/TangChuXian
/// @date     2023年8月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
namespace Ui { class QryCondDisplayWgt; };

class QryCondDisplayWgt : public QWidget
{
    Q_OBJECT

public:
    QryCondDisplayWgt(QWidget *parent = Q_NULLPTR);
    ~QryCondDisplayWgt();

    ///
    /// @brief
    ///     查询文本
    ///
    /// @param[in]  strQryText  查询文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月15日，新建函数
    ///
    void SetText(const QString& strQryText);

    ///
    /// @brief
    ///     重置查询
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月15日，新建函数
    ///
    void ResetQry();

    ///
    /// @brief
    ///     设置文本提示
    ///
    /// @param[in]  strToolTip  文本提示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月5日，新建函数
    ///
    void SetToolTip(const QString& strToolTip);

    ///
    /// @brief
    ///     设置文本省略模式
    ///
    /// @param[in]  elideMode  省略模式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月5日，新建函数
    ///
    void SetElideMode(Qt::TextElideMode elideMode);

protected:
    ///
    /// @brief
    ///     事件过滤器
    ///
    /// @param[in]  obj    目标对象
    /// @param[in]  event  事件
    ///
    /// @return true表示已处理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月15日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *event);

protected Q_SLOTS:
    ///
    /// @brief
    ///     文本内容改变
    ///
    /// @param[in]  strText  改变后的文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月15日，新建函数
    ///
    void OnTextChanged(const QString& strText);

    ///
    /// @brief
    ///     更新可省略文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月16日，新建函数
    ///
    void UpdateElidedText();

Q_SIGNALS:
    ///
    /// @brief
    ///     重置查询
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月15日，新建函数
    ///
    void SigResetQry();

private:
    Ui::QryCondDisplayWgt  *ui;                     // UI对象指针
    QString                 m_strText;              // 全文本
    QString                 m_strToolTip;           // 文本提示
    Qt::TextElideMode       m_enElideMode;          // 省略模式
};
