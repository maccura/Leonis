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
/// @file     AssayCardsManage.h
/// @brief    试剂卡片管理页面
///
/// @author   5774/WuHongTao
/// @date     2021年12月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "AssayCardsManage.h"
#include "ui_AssayCardsManage.h"
#include "ReagentAssayCard.h"
#include "shared/ReagentCommon.h"
#include "src/common/Mlog/mlog.h"

///
/// @brief
///     构造函数
///
/// @param[in]  parent  窗口父指针
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
AssayCardsManage::AssayCardsManage(QWidget *parent)
    : QFrame(parent)
{
    ui = new Ui::AssayCardsManage();
    ui->setupUi(this);
    Init();
}

///
/// @brief
///     析构函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
AssayCardsManage::~AssayCardsManage()
{
}

///
/// @brief
///     刷新页面试剂卡片信息
///
/// @param[in]  card  卡片信息表
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月1日，新建函数
///
void AssayCardsManage::RefreshCards(std::vector<CardShowInfo>& card)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_CardInfos = card;

    // 确保不会越界
    if (m_CardInfos.size() > m_AssayCards.size())
    {
        ULOG(LOG_INFO, "The card data size %d is > card size %d", m_CardInfos.size(), m_AssayCards.size());
        return;
    }

    // 清除卡片信息
    ClearAllCrards();

    // 依次刷新每张卡片信息
    int cardIndex = 0;
    for (const auto& card : m_CardInfos)
    {
        m_AssayCards[cardIndex++]->SetCardInfo(card);
    }
}

///
/// @brief
///     初始化页面（生成空的试剂卡片）
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月30日，新建函数
///
void AssayCardsManage::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    for (int row = 0; row < RowColumnCount; row++)
    {
        for (int column = 0; column < RowColumnCount; column++)
        {
            // 添加控件
            ReagentAssayCard* card = new ReagentAssayCard(this);
            card->ClearCard();
            ui->gridLayout->addWidget(card,row,column);
            m_AssayCards.push_back(card);
        }
    }

}

///
/// @brief
///     清除所有的试剂卡片显示信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月1日，新建函数
///
void AssayCardsManage::ClearAllCrards()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 依次清空卡片信息
    for (auto card : m_AssayCards)
    {
        card->ClearCard();
    }
}
