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
#include <mutex>
#include <set>
#include <boost/optional/optional.hpp>
#include "shared/basedlg.h"

using namespace std;

namespace Ui {
    class QDetectProgressBar;
};

class QDetectProgressBar : public BaseDlg
{
    Q_OBJECT

public:
    QDetectProgressBar(QWidget *parent = Q_NULLPTR);
    ~QDetectProgressBar();

    ///
    /// @brief  开始余量探测
    ///
    /// @param[in]  devSnPostions  <设备序列号，vector<模块号，酸碱清洗液位置>>
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年12月14日，新建函数
    ///
    void Start(const map<string, vector<pair<int,int>>>& devSnPostions);

    ///
    /// @brief  更新余量探测进度
    ///
    /// @param[in]  devSn  设备序列号
	/// @param[in]  pos    位置
	/// @param[in]  moduleIdx 模块号
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年12月14日，新建函数
    ///
    void Update(const string& devSn, const int pos, const int moduleIdx = 0);

private:
    Ui::QDetectProgressBar*         ui;
    QTimer                          m_timer;					// 定时器刷新滚动条
    int                             m_elapsedTime = 0;			// 统计超时时长
    set<string>						m_currentPostions;			// 当前进行余量探测的位置列表(设备序列号+模块号+位置号)
	mutex							m_mutex;					// m_currentPostions的锁
	int								m_posCount = 0;				// 位置总数（样本针酸碱同时包含为2，只包含其中1个为1）
};
