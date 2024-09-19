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

#include "QDetectProgressBar.h"
#include "ui_QDetectProgressBar.h"
#include "src/common/Mlog/mlog.h"
#define  TIMEINTERVAL                                   (1000)      // 定时器刷新时间
#define  TIMEOUT                                        (10)		// 超时时间（一般下位机进行ise余量探测5秒左右返回）
#define  DETECTTIME                                     (6)         // 余量探测预估时长

QDetectProgressBar::QDetectProgressBar(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::QDetectProgressBar();
    ui->setupUi(this);

	SetTitleName(tr("余量探测"));

    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

	m_timer.setInterval(TIMEINTERVAL);
    connect(&m_timer, &QTimer::timeout, [&]() 
	{
        m_elapsedTime++;

		std::lock_guard<std::mutex> autoLock(m_mutex);
		// 下位机处理完成 或者超时 提前返回
		if (m_currentPostions.empty() || 
			m_elapsedTime >= TIMEOUT * m_posCount)
		{
			m_timer.stop();
			this->close();
			return;
		}

		// 当未超时，且下位机未返回结果时，大于了100显示99
		int value = (m_elapsedTime * 100) / (DETECTTIME * m_posCount);
        ui->progressBar->setValue(value >= 100 ? 99 : value);
    });
}

QDetectProgressBar::~QDetectProgressBar()
{
}

void QDetectProgressBar::Start(const map<string, vector<pair<int, int>>>& devSnPostions)
{
	if (devSnPostions.empty())
	{
		return;
	}

	m_posCount = 1;

	for (const auto& dev : devSnPostions)
	{
		if (dev.second.size() > 1)
		{
			m_posCount = 2;
		}

		for (const auto &pos : dev.second)
		{
			string devSnPos = dev.first + "-" + to_string(pos.first) + "-" + to_string(pos.second);
			m_currentPostions.insert(devSnPos);
		}
	}

    m_elapsedTime = 0;
    ui->progressBar->setValue(0);

    if (!m_timer.isActive())
    {
        m_timer.start();
    }

    this->show();
}

void QDetectProgressBar::Update(const string& devSn, const int pos, const int moduleIdx /*= 0*/)
{
	ULOG(LOG_INFO, "%s(devSn=%s, pos=%d, moduleIdx=%d)", __FUNCTION__, devSn, pos, moduleIdx);

	std::lock_guard<std::mutex> autoLock(m_mutex);
	string devSnPos = devSn + "-" + to_string(moduleIdx) + "-" + to_string(pos);

	if (m_currentPostions.count(devSnPos))
	{
		m_currentPostions.erase(devSnPos);
	}
}
