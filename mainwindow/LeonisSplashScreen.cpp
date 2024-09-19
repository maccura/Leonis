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
/// @file     LeonisSplashScreen.h
/// @brief 	  启动动画
///
/// @author   7656/zhang.changjiang
/// @date      2022年11月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年11月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "LeonisSplashScreen.h"
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QMovie>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QHBoxLayout>

LeonisSplashScreen::LeonisSplashScreen(const QString path, IMAGES_FORMAT fmt, const QString version)
	: m_iFrameCount(0),
	m_strVersion(version)
{
	Init(path, fmt);
}
LeonisSplashScreen::~LeonisSplashScreen()
{

}

///
///  @brief 加载完成
///
///
///  @param[in]   iWaitTime  延时时间（ms）
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月1日，新建函数
///
void LeonisSplashScreen::Finish(int iWaitTime/* = 0*/)
{
	QTimer::singleShot(iWaitTime, [&]()
	{
		if (nullptr != m_pVideoWidget)
		{
			m_pVideoWidget->close();
			delete m_pVideoWidget;
		}
		close();
	});
}

///
///  @brief 初始化
///
///
///  @param[in]   path  文件路径
///  @param[in]   fmt   文件格式
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月1日，新建函数
///
void LeonisSplashScreen::Init(const QString path, IMAGES_FORMAT fmt)
{
	switch (fmt)
	{
	case LeonisSplashScreen::GIF:
		StartWithGIF(path);
		break;
	case LeonisSplashScreen::WMV:
		StartWithWMV(path);
		break;
	case LeonisSplashScreen::MP3:
		StartWithMP3(path);
		break;
	case LeonisSplashScreen::MP4:
		StartWithMP4(path);
		break;
	default:
		break;
	}
}

///
///  @brief 加载GIF
///
///
///  @param[in]   path  文件路径
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月1日，新建函数
///
void LeonisSplashScreen::StartWithGIF(const QString path)
{
	m_pMovie = new QMovie(path);
	m_iFrameCount = m_pMovie->frameCount();
	connect(m_pMovie, &QMovie::frameChanged, this, [=](int temp) {
		if (temp == (m_iFrameCount - 1))
		{
			m_pMovie->start();
		}
	});
	m_pMovie->start();
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &LeonisSplashScreen::Update);
	timer->start(40);
	QPixmap pixmap(path);
	this->setPixmap(pixmap);
}

///
///  @brief 加载WMV
///
///
///  @param[in]   path  文件路径
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月1日，新建函数
///
void LeonisSplashScreen::StartWithWMV(const QString path)
{
	m_pVideoWidget = new QVideoWidget();
	m_pMediaPlayer = new QMediaPlayer(m_pVideoWidget);
	m_pMediaPlayer->setVideoOutput(m_pVideoWidget);
	m_pMediaPlayer->setMedia(QUrl::fromLocalFile(path));
	m_pVideoWidget->showFullScreen();
	m_pMediaPlayer->play();
}

///
///  @brief 加载MP3
///
///
///  @param[in]   path  文件路径
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月1日，新建函数
///
void LeonisSplashScreen::StartWithMP3(const QString path)
{
	// TODO
}

///
///  @brief 加载MP4
///
///
///  @param[in]   path  文件路径
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月1日，新建函数
///
void LeonisSplashScreen::StartWithMP4(const QString path)
{
	// TODO
}

///
///  @brief 刷新Gif
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年11月30日，新建函数
///
void LeonisSplashScreen::Update()
{
	this->setPixmap(m_pMovie->currentPixmap());
	repaint();
}
