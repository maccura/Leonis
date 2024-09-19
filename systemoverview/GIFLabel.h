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
/// @file     GIFLabel.h
/// @brief    同时显示gif和文字的Label
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QLabel>
#include <QMovie>
#include <QPainter>

class GIFLabel : public QLabel 
{
public:

	GIFLabel(QWidget* parent = nullptr) : QLabel(parent)
	{
		m_movie = new QMovie;
		this->setMovie(m_movie);
		m_movie->start();

		m_txetLabel = new QLabel(this);
		// 设置标签鼠标事件穿透
		m_txetLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
		m_txetLabel->setAlignment(Qt::AlignCenter);
		m_txetLabel->raise();
		m_txetLabel->setStyleSheet("background: transparent; font-size: 24px;");
	}

	///
	/// @brief  设置绘制文字
	///
	/// @param[in]  txt  文字内容
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月20日，新建函数
	///
	void setDrawText(const QString &txt)
	{
		m_txetLabel->setText(txt);
	}

	///
	/// @brief  改变gif
	///
	/// @param[in]  filename  gif文件路径
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月20日，新建函数
	///
	void changeGIF(const QString& filename)
	{
		if (m_movie == nullptr)
		{
			return;
		}
		m_movie->stop();
		m_movie->setFileName(filename);
		m_movie->start();
	}

	///
	/// @brief  改变文字颜色
	///
	/// @param[in]  color  文字颜色
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月20日，新建函数
	///
	void changeTextColor(const QColor &color)
	{
		QString stls = "background: transparent; font-size: 24px; color: " + color.name() + ";";
		m_txetLabel->setStyleSheet(stls);
	}

protected:
	///
	/// @brief label显示事件
	///     
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月20日，新建函数
	///
	void showEvent(QShowEvent *event) override
	{
		m_txetLabel->setGeometry(rect().x(), rect().y() + TOP_H, rect().width(), TEXT_H);
		QLabel::showEvent(event);
	}

private:

	QMovie* m_movie = nullptr;				// 动画
	QLabel* m_txetLabel = nullptr;			// 绘制文字的label

	const int TOP_H = 104;					// 距离顶部的高度
	const int TEXT_H = 41;					// 文字高度 
};
