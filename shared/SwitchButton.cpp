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

#include "SwitchButton.h"
#include <QPainter>
#include <QPainterPath>

SwitchButton::SwitchButton(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SwitchButton)
{
	ui->setupUi(this);
	m_bActive = false; // 默认未激活
	m_fStep = width() / 15.0;
	m_fCurrent = 2.0f;
	m_nArcRadius = std::min(width(), height()); // 默认半径
	m_nRectWidth = width() - m_nArcRadius;
	m_colorActive = Qt::green;
	m_colorInactive = Qt::red;
	m_colorSlider = Qt::white;
	setCursor(QCursor(Qt::PointingHandCursor));
	connect(&m_timer, &QTimer::timeout, this, &SwitchButton::SwitchAnimation);
}

SwitchButton::~SwitchButton()
{
	delete ui;
}

void SwitchButton::SetSize(int nWidth, int nHeight)
{
	resize(nWidth, nHeight);
	m_nArcRadius = std::min(width(), height());
	m_nRectWidth = width() - m_nArcRadius;
	m_fStep = width() / 15.0;
}

void SwitchButton::SetActiveColor(QColor color)
{
	m_colorActive = color;
}

void SwitchButton::SetInactiveColor(QColor color)
{
	m_colorInactive = color;
}

void SwitchButton::SetSliderColor(QColor color)
{
	m_colorSlider = color;
}

void SwitchButton::SetStatus(bool bActive)
{
	if (m_bActive == bActive) return;
	m_bActive = bActive;
	if (m_bActive) m_fCurrent = width() - m_nArcRadius;
	else m_fCurrent = 2.0f;
	update();
}

bool SwitchButton::GetStatus()
{
	return m_bActive;
}

void SwitchButton::paintEvent(QPaintEvent *)
{
	QPainter p;
	p.begin(this);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setPen(Qt::NoPen);
	if (m_bActive) p.setBrush(QBrush(m_colorActive));
	else p.setBrush(QBrush(m_colorInactive));

	QPainterPath leftPath;
	leftPath.addEllipse(0, 0, m_nArcRadius, m_nArcRadius);

	QPainterPath middlePath;
	middlePath.addRect(m_nArcRadius / 2, 0, m_nRectWidth, m_nArcRadius);

	QPainterPath rightPath;
	rightPath.addEllipse(m_nRectWidth, 0, m_nArcRadius, m_nArcRadius);

	QPainterPath path = leftPath + middlePath + rightPath;

	p.drawPath(path);

	DrawSlider(&p);

	p.end();
}

void SwitchButton::mousePressEvent(QMouseEvent *event)
{
	QWidget::mousePressEvent(event);
	m_bActive = !m_bActive;
	m_timer.start(15);
	// 发出消息
	emit(Clicked(-1));
}

void SwitchButton::DrawSlider(QPainter *p)
{
	p->setBrush(QBrush(m_colorSlider));
	p->drawEllipse(m_fCurrent, 1, m_nArcRadius - 2, m_nArcRadius - 2);
}

void SwitchButton::SwitchAnimation()
{
	if (!m_bActive)
	{
		m_fCurrent -= m_fStep;
		if (m_fCurrent <= 2.0f)
		{
			m_fCurrent = 2.0f;
			m_timer.stop();
		}
	}
	else
	{
		m_fCurrent += m_fStep;
		if (m_fCurrent >= width() - m_nArcRadius)
		{
			m_fCurrent = width() - m_nArcRadius;
			m_timer.stop();
		}
	}
	update();
}
