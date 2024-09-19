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
/// @file     DetectDebugWidget.h
/// @brief 	 检测系统调试
///
/// @author   7656/zhang.changjiang
/// @date      2022年7月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年7月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include "thrift/im/ImLogicControlProxy.h"

namespace Ui {
class DetectDebugWidget;
}

class DetectDebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DetectDebugWidget(QWidget *parent = nullptr);
    ~DetectDebugWidget();
public slots:
	
	///
	///  @brief:	显示信号值
	///
	///  @param[in]   mit  
	///  @param[in]   sigValue  
	///
	///  @return	:
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年7月27日，新建函数
	///
	void ShowDetectSignals(tf::MaintainItemType::type mit, double sigValue);
private slots:
    
    ///
    ///  @brief:	点击开始按钮
    ///
    ///
    ///  @return	:
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年7月27日，新建函数
    ///
    void on_pushButton_start_clicked();
	///
	///  @brief:	点击清除
	///
	///
	///  @return	:
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年7月27日，新建函数
	///
	void on_pushButton_clean_clicked();
	
private:
    Ui::DetectDebugWidget *ui;
	double			m_dMaxValue;	///< 当前最大值
	int				m_iRow;			///< 行数
};

