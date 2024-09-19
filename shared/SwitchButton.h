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

#pragma once

#include <QWidget>
#include <QTimer>
#include "ui_SwitchButton.h"

namespace Ui {
class SwitchButton;
}

class SwitchButton : public QWidget
{
	Q_OBJECT

public:
	explicit SwitchButton(QWidget *parent = nullptr);
	~SwitchButton();
	/**
	* @brief SetSize 设置按钮的尺寸
	* @param nWidth 按钮的新宽度
	* @param nHeight 按钮的新高度
	*/
	void SetSize(int nWidth, int nHeight);

	/**
	* @brief SetActiveColor 设置按钮激活时候的颜色
	* @param color 激活颜色
	*/
	void SetActiveColor(QColor color);

	/**
	* @brief SetInactiveColor 设置按钮未激活时候的颜色
	* @param color 未激活颜色
	*/
	void SetInactiveColor(QColor color);

	/**
	* @brief SetSliderColor 设置滑块颜色
	* @param color 滑块的颜色
	*/
	void SetSliderColor(QColor color);

	/**
	* @brief SetStatus 设置按钮状态
	* @param bActive true: 激活，false: 未激活
	*/
	void SetStatus(bool bActive);

	/**
	* @brief GetStatus 获取按钮当前状态
	* @return  true: 激活，false: 未激活
	*/
	bool GetStatus();

signals:

	///
	/// @brief 被点击以后发出的消息
	///
	/// @param[in]  index  序号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void Clicked(int index);

protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void DrawSlider(QPainter* p);

private:
	Ui::SwitchButton *ui;
	bool m_bActive; // 是否激活
	float m_fStep; // 每次重绘时候的滑块的步进值
	float m_fCurrent; // 当前滑块的位置（left位置）
	int m_nArcRadius; // 圆弧的半径
	int m_nRectWidth; // 矩形的宽度
	QColor m_colorActive; // 激活时的颜色
	QColor m_colorInactive;
	QColor m_colorSlider;
	QTimer m_timer;
private slots:
	void SwitchAnimation(); // 切换动画
};
