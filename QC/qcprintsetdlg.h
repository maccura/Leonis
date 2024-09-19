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

#include "shared/basedlg.h"
namespace Ui { class QcPrintSetDlg; };

class QcPrintSetDlg : public BaseDlg
{
    Q_OBJECT

public:
    QcPrintSetDlg(QWidget *parent = Q_NULLPTR);
    ~QcPrintSetDlg();
    void showEvent(QShowEvent *event);                  // 重载显示事件处理函数
    void InitConnect();
    void InitAfterShow();                               // 显示后初始化
    void InitStrResource();                             // 初始化界面显示字符资源
    void InitChildWidget();                             // 初始化子控件

    bool isSelectPrintLJ();
    QDate getQcDate();

protected Q_SLOTS:
    void ChoosePrintMode();

private:
    Ui::QcPrintSetDlg           *ui;
    bool                        m_bInit;
};
