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
/// @file     basedlg.h
/// @brief    通用基础对话框
///
/// @author   4170/TangChuXian
/// @date     2020年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QDialog>

class QLabel;
class QFrame;

class BaseDlg : public QDialog
{
    Q_OBJECT

public:
    BaseDlg(QWidget *parent = Q_NULLPTR);
    ~BaseDlg();

    ///
    /// @brief 设置标题名
    ///     
    /// @param[in]  strTitlename  标题名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月30日，新建函数
    ///
    void SetTitleName(const QString& strTitlename);

    ///
    /// @brief 获取标题名
    ///     
    /// @return 标题名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月30日，新建函数
    ///
    QString GetTitleName();

    ///
    /// @brief
    ///     设置中心点
    ///
    /// @param[in]  ptCenter  中心点
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月9日，新建函数
    ///
    static void SetCenterPoint(const QPoint& ptCenter);

    ///
    /// @brief
    ///     获取中心点
    ///
    /// @return 显示中心点坐标
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月30日，新建函数
    ///
    static QPoint GetCenterPoint();

protected:
    ///
    /// @brief 初始化标题栏
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月24日，新建函数
    ///
    void InitTitleLab();

    ///
    /// @brief 初始化内容栏
    ///     
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月24日，新建函数
    ///
    void InitContentLab();

    ///
    /// @brief
    ///     事件过滤器（主要用于监听标题栏拖拽）
    ///
    /// @param[in] obj    事件的目标对象
    /// @param[in] event  事件对象
    ///
    /// @return true表示事件已处理，false表示事件未处理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月24日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *event);

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月9日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected:
    // 子控件
    QFrame* m_backgroundFrame;          // 背景
    QLabel* m_pTitleLabel;              // 标题栏
    QFrame* m_pContentFrame;            // 内容栏

private:
    // 窗口拖拽
    bool    m_bIsMousePress;            // 鼠标是否按下
    QPoint  m_mousePressPoint;          // 上一次鼠标按下的坐标
    QPoint  m_mousePressDlgPoint;       // 上一次鼠标按下时对话框的坐标

    static QPoint sm_ptCenter;          // 中心点
};

class QCustomDialog : public QDialog
{
    Q_OBJECT
public:
    QCustomDialog(QWidget* parent = Q_NULLPTR, int times = 1000, QString showMsg = tr("新增成功"), bool isSuccess = true);
    ~QCustomDialog() {};

    void SetCenter();

protected:
    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月30日，新建函数
    ///
    void showEvent(QShowEvent *event) override;

private:
    int                             m_times;                    ///< 显示时间
    QString                         m_showMsg;                  ///< 显示内容
	bool							m_success;                  ///< 是否成功
};
