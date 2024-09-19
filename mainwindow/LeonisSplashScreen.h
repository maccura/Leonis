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
#pragma once
#include <QSplashScreen>

class QMovie;
class QMediaPlayer;
class QVideoWidget;

class LeonisSplashScreen : public QSplashScreen
{
	Q_OBJECT
	
public:
	enum IMAGES_FORMAT
	{
		GIF,
		WMV,
		MP3,
		MP4
	};
	LeonisSplashScreen(const QString path, IMAGES_FORMAT fmt, const QString version = QString());
	virtual ~LeonisSplashScreen();
public:

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
	void Finish(int iWaitTime);
public slots:
	///
	///  @brief 刷新
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年11月30日，新建函数
	///
	void Update();

private:
	
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
	void Init(const QString path, IMAGES_FORMAT fmt);

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
	void StartWithGIF(const QString path);

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
	void StartWithWMV(const QString path);

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
	void StartWithMP3(const QString path);

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
	void StartWithMP4(const QString path);

private:
	int						m_iFrameCount;			// 剩余帧数
	QMovie*					m_pMovie;				// QMovie指针
	QMediaPlayer*           m_pMediaPlayer;         // QMediaPlayer指针
	QVideoWidget*           m_pVideoWidget;         // QVideoWidget指针，wmv格式播放窗口
	QString					m_strVersion;			// 版本信息
};
