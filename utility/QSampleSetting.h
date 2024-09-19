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
/// @file     QSampleSetting.h
/// @brief    在应用-显示设置-样本信息界面
///
/// @author   
/// @date     2023年8月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月31日，添加注释
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>

namespace Ui {
    class QSampleSetting;
};

class QSampleSetting : public QWidget
{
    Q_OBJECT

public:
    QSampleSetting(QWidget *parent = Q_NULLPTR);
    ~QSampleSetting();

    bool isExistUnsaveData();

Q_SIGNALS:
    void closeDialog();

protected:
    ///
    /// @brief 界面初始化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 2023年8月31日，添加注释
    ///
    void Init();

	///
	/// @brief 更新样本管和样本类型的默认类型
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月20日，新建函数
	///
	void UpdateComBoxInfo();
	void showEvent(QShowEvent *event);

protected slots:
    ///
    /// @brief 保存用户设置
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 2023年8月31日，添加注释
    ///
    void OnSaveData();

private:
    Ui::QSampleSetting*              ui;
};
