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
#include "QSpecialProgessBar.h"

namespace Ui {
    class QUnderDeckSupply;
    class OutBoardSupply;
};


class QSupplyContainer : public QWidget
{
    Q_OBJECT

public:
    // 耗材类型
    enum SUPPLYTYPE
    {
        UNDERDECK,
        OUTBOARD,
        ISEUNDERDECK
    };

    QSupplyContainer(QWidget *parent = Q_NULLPTR);
    ~QSupplyContainer();

    ///
    /// @brief
    ///     设置耗材类型
    ///
    /// @param[in]  type  耗材类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void SetContainerType(SUPPLYTYPE type);

    ///
    /// @brief
    ///     显示控件状态
    ///
    /// @param[in]  statusList  状态信息列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void SetSupplyStatus(std::vector<QSpecialProgessBar::ProgressStatus>& statusList);

private:
    Ui::QUnderDeckSupply*           ui_underDeck;               ///< 仓内耗材
    Ui::OutBoardSupply*             ui_outBoardSupply;          ///< 仓外耗材
    std::vector<QSpecialProgessBar*>m_ProgressBarList;          ///< 控件列表
};
