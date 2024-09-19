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

#include "QSupplyContainer.h"
#include "ui_QUnderDeckSupply.h"
#include "ui_OutBoardSupply.h"

QSupplyContainer::QSupplyContainer(QWidget *parent)
    : QWidget(parent),
    ui_underDeck(nullptr),
    ui_outBoardSupply(nullptr)
{
    ui_underDeck = new Ui::QUnderDeckSupply();
    ui_outBoardSupply = new Ui::OutBoardSupply();
}

QSupplyContainer::~QSupplyContainer()
{
}

///
/// @brief
///     设置耗材类型
///
/// @param[in]  type  耗材类型
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QSupplyContainer::SetContainerType(SUPPLYTYPE type)
{
    m_ProgressBarList.clear();
    switch (type)
    {
        // 仓内耗材
    case QSupplyContainer::UNDERDECK:
        ui_underDeck = new Ui::QUnderDeckSupply();
        ui_underDeck->setupUi(this);
        m_ProgressBarList.push_back(ui_underDeck->Diluent_Sample);
        m_ProgressBarList.push_back(ui_underDeck->Hitergent_water);
        m_ProgressBarList.push_back(ui_underDeck->Reagent_Wash);
        m_ProgressBarList.push_back(ui_underDeck->Reagent_Wash_2);
        break;
        // 仓外耗材
    case QSupplyContainer::OUTBOARD:
        ui_outBoardSupply = new Ui::OutBoardSupply();
        ui_outBoardSupply->setupUi(this);
        m_ProgressBarList.push_back(ui_outBoardSupply->Acidity1_Widget);
        m_ProgressBarList.push_back(ui_outBoardSupply->Acidity2_Widget);
        m_ProgressBarList.push_back(ui_outBoardSupply->Alkalinity1_Widget);
        m_ProgressBarList.push_back(ui_outBoardSupply->Alkalinity2_Widget);
        m_ProgressBarList.push_back(ui_outBoardSupply->WashWater_Widget);
        break;
    case QSupplyContainer::ISEUNDERDECK:
        break;
    default:
        break;
    }
}

///
/// @brief
///     显示控件状态
///
/// @param[in]  statusList  状态信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QSupplyContainer::SetSupplyStatus(std::vector<QSpecialProgessBar::ProgressStatus>& statusList)
{
    // 判断类型必须一致
    if (statusList.size() != m_ProgressBarList.size())
    {
        return;
    }

    // 进度条状态依次填充
    int i = 0;
    for (const auto& progressStatus : statusList)
    {
        m_ProgressBarList[i]->SetProgressBar(progressStatus);
        i++;
    }
}
