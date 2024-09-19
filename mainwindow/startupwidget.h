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
/// @file     startupdlg.h
/// @brief    启动界面
///
/// @author   4170/TangChuXian
/// @date     2020年4月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#ifndef STARTUP_DLG_H
#define STARTUP_DLG_H

#include <QWidget>
#include <QtMultimedia/QMediaPlayer>

class QVideoWidget;

#define STARTMV_DURATION    6 * 1000 //6s

class StartupWidget : public QWidget
{
    Q_OBJECT

public:
    StartupWidget(QWidget *parent = 0);
    ~StartupWidget();

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月28日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月28日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief 窗口缩放事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月29日，新建函数
    ///
    void resizeEvent(QResizeEvent *event);

protected Q_SLOTS:
    ///
    /// @brief 播放器播放状态发生改变
    ///     
    /// @param[in]    state 播放器状态（正在播放、暂停、停止）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月28日，新建函数
    ///
    void OnMediaStateChanged(QMediaPlayer::State state);

private:
    bool          m_bInit;            // 是否已经初始化

    QVideoWidget *m_pVideoWidget;     // 播放窗口
    QMediaPlayer *m_pMediaPlayer;     // 播放器
};

#endif // DLG_STARTUP_H
