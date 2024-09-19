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
/// @file     startupdlg.cpp
/// @brief    开机动画界面
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

#include "startupwidget.h"

#include <QStandardItem>
#include <QTimer>
#include <QUrl>
#include <QtMultimediaWidgets/QVideoWidget>

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/common/Mlog/mlog.h"

#define DELAY_MSEC_ON_CLOSE                         (10)                // 延迟关闭毫秒数

StartupWidget::StartupWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false)
{
	setWindowIcon(QIcon(":/Leonis/resource/image/Mc64.ico"));
    // 初始化播放器和播放窗口
    m_pVideoWidget = new QVideoWidget(this);
    m_pMediaPlayer = new QMediaPlayer(this);

    // 设置播放器和播放窗口相关参数
    m_pMediaPlayer->setVideoOutput(m_pVideoWidget);
    m_pMediaPlayer->setMedia(QUrl::fromLocalFile("./resource/video/Startup.wmv"));

    // 播放窗口背景默认为黑色
    QPalette pal(m_pVideoWidget->palette());
    pal.setBrush(QPalette::Window, QBrush(Qt::black));
    m_pVideoWidget->setAutoFillBackground(true);
    m_pVideoWidget->setPalette(pal);

    // 监听动画是否播放结束
    connect(m_pMediaPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(OnMediaStateChanged(QMediaPlayer::State)));
}

StartupWidget::~StartupWidget()
{
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月28日，新建函数
///
void StartupWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 调试阶段不播放动画
    OnMediaStateChanged(QMediaPlayer::StoppedState);

    // 播放动画
//     m_pVideoWidget->show();
//     m_pMediaPlayer->play();
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月28日，新建函数
///
void StartupWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 基类处理事件
    QWidget::showEvent(event);

    // 如果没有初始化，则初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

void StartupWidget::resizeEvent(QResizeEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 播放窗口自适应大小
    m_pVideoWidget->setFixedSize(width(), height());
}

///
/// @brief 播放器播放状态发生改变
///     
/// @param[in]    state 播放器状态（正在播放、暂停、停止）
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月28日，新建函数
///
void StartupWidget::OnMediaStateChanged(QMediaPlayer::State state)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, int(state));

    // 如果已经播放完成，触发播放完成事件
    if (state == QMediaPlayer::StoppedState)
    {
        // 通知开机动画播放完成
        POST_MESSAGE(MSG_ID_STARTUP_VIDEO_FINISHED);

        // 延迟10毫秒后关闭窗口
        QTimer::singleShot(DELAY_MSEC_ON_CLOSE, this, SLOT(close()));
    }
}
