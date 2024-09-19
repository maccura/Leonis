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
/// @file     QAssaySet.h
/// @brief    应用/显示/项目显示顺序界面
///
/// @author   1556/Chenjianlin
/// @date     2023年10月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1556/Chenjianlin，2023年10月27日，添加注释
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "src/public/ConfigDefine.h"

namespace Ui {
    class QAssaySet;
};
class QStandardItemModel;


class QAssaySet : public QWidget
{
    Q_OBJECT

public:

    QAssaySet(QWidget *parent = Q_NULLPTR);
    ~QAssaySet() {};

    void showEvent(QShowEvent *event);
	bool IsModified();
protected:
    void Init();
    bool MoveItem(int row, bool direction);
    void UpdateBtnState();
	std::string GetAssayName(int assayCode);

    protected slots:
    void OnReset();
    void OnReFresh();
    void OnSaveData();
	void SetButtonStatus(bool isChecked);

Q_SIGNALS:
    void closeDialog();

private:
    OrderAssay                      m_showOrder;
    QStandardItemModel*				m_model;			///< 自定义模式
    Ui::QAssaySet*                  ui;
};
