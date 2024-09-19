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
/// @file     ReagentAssayCard.h
/// @brief    试剂项目卡片信息
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
#include "ReagentAssayCard.h"
#include "ui_ReagentAssayCard.h"
#include <QBrush>
#include <QPalette>
#include <QMouseEvent>
#include "shared/ReagentCommon.h"
#include "src/common/Mlog/mlog.h"

///
/// @brief
///     构造函数
///
/// @param[in]  parent  窗口父指针
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
ReagentAssayCard::ReagentAssayCard(QWidget *parent)
    : QFrame(parent)
{
    ui = new Ui::ReagentAssayCard();
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
ReagentAssayCard::~ReagentAssayCard()
{
}

///
/// @brief
///     初始化框体的一些默认属性
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月30日，新建函数
///
void ReagentAssayCard::Init()
{
    // 设置默认背景色
    SetReagentState(INIT);
}

///
/// @brief
///     设置项目名称
///
/// @param[in]  assayName  项目名称
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月30日，新建函数
///
void ReagentAssayCard::SetAssayName(const QString& assayName)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->assayName->setText(assayName);
}

///
/// @brief
///     设置模块对应的信息
///
/// @param[in]  seq  模块序号（必须<=6 ，>=0）
/// @param[in]  textForModule  (具体的信息)
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月30日，新建函数
///
void ReagentAssayCard::SetModuleText(int seq, const QString& textForModule)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 判断设置是否超界
    if (seq < 0 || seq > MODULEMAX)
    {
        ULOG(LOG_ERROR, "The seq is out of the range");
        return;
    }

    // 根据序号选择不同的控件设置信息
    switch (seq)
    {
    case 1 :
        ui->M1->setText(textForModule);
        break;
    case 2:
        ui->M2->setText(textForModule);
        break;
    case 3:
        ui->M3->setText(textForModule);
        break;
    case 4:
        ui->M4->setText(textForModule);
        break;
    case 5:
        ui->M5->setText(textForModule);
        break;
    case 6:
        ui->M6->setText(textForModule);
        break;
    default:
        break;
    }
}

///
/// @brief
///     设置测试余量
///
/// @param[in]  testTimes  余量还能使用多少次
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月30日，新建函数
///
void ReagentAssayCard::SetLastReagentVolumn(int testTimes)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui->test->setText(QString("test"));
    ui->last->setText(QString::number(testTimes));
}

///
/// @brief
///     设置项目的状态
///
/// @param[in]  state  状态
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月30日，新建函数
///
void ReagentAssayCard::SetReagentState(USESTATE state)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QPalette palette;

    switch (state)
    {
    case USESTATE::INIT:
    {
        setProperty("cards", "card_normal");
    }
    break;

    case USESTATE::NORMAL:
    {
        setProperty("cards", "card_normal");
    }
    break;

    case USESTATE::WARING:
    {
        setProperty("cards", "card_waring");

    }
    break;

    case USESTATE::ERRORSTATE:
    {
        setProperty("cards", "card_errorstate");
    }
    break;

    default:
    break;
    }
}

///
/// @brief
///     清除卡片显示信息恢复到初始
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月1日，新建函数
///
void ReagentAssayCard::ClearCard()
{
    // 设置默认背景色
    SetReagentState(INIT);

    // 设置项目名称
    SetAssayName(QString(""));
    // 设置模块1的信息
    SetModuleText(1, QString(""));
    // 设置模块2的信息
    SetModuleText(2, QString(""));
    // 设置模块2的信息
    SetModuleText(3, QString(""));
    // 设置模块2的信息
    SetModuleText(4, QString(""));
    SetModuleText(5, QString(""));
    // 设置模块2的信息
    SetModuleText(6, QString(""));
    // 设置余量
    ui->last->setText("");
    ui->test->setText("");
}

///
/// @brief
///     设置卡片信息
///
/// @param[in]  cardInfo  卡片信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月12日，新建函数
///
void ReagentAssayCard::SetCardInfo(const CardShowInfo& cardInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置项目名称
    SetAssayName(QString::fromStdString(cardInfo.assayName));

    // 显示模块信息
    int moduleIndex = 1;
    for (const auto& moduleMsg : cardInfo.showMsg)
    {
        // 越界控制
        if (moduleIndex > MODULEMAX)
        {
            break;
        }

        SetModuleText(moduleIndex++, QString::fromStdString(moduleMsg));
    }

    // 设置余量
    SetLastReagentVolumn(cardInfo.lastVolumn);
    // 设置状态
    SetReagentState(cardInfo.state);
}

///
/// @brief
///     重写鼠标按键处理事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
void ReagentAssayCard::mousePressEvent(QMouseEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 处理左键
    if (event->button() == Qt::LeftButton)
    {
/*        setStyleSheet(QString("QWidget#ReagentAssayCard{border:1px solid blue}"));*/
    }

}

///
/// @brief
///     设置控件的背景色
///
/// @param[in]  dstObj  目标控件
/// @param[in]  dstColor  目标色彩
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
void ReagentAssayCard::SetBackGroudColor(QWidget* dstObj, QColor dstColor)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (dstObj == nullptr)
    {
        return;
    }

    // 使用stylesheet来设置颜色
    dstObj->setStyleSheet(QString("background-color:%1;").arg(dstColor.name()));
}

///
/// @brief
///     测试项目卡片功能，仅供内部测试使用
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月30日，新建函数
///
void ReagentAssayCard::TestCardParamter()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    static int i = 0;
    if (i > 2)
    {
        i = 0;
        return;
    }

    // 设置项目名称
    SetAssayName(QString("ALT"));
    // 设置模块1的信息
    SetModuleText(1, QString("M1:001"));
    // 设置模块2的信息
    SetModuleText(2, QString("M2:001"));
    // 设置模块2的信息
    SetModuleText(3, QString("M3:008"));
    // 设置模块2的信息
    SetModuleText(4, QString("M4:010"));
    SetModuleText(5, QString("M5:200"));
    // 设置模块2的信息
    SetModuleText(6, QString("M6:100"));
    // 设置余量
    SetLastReagentVolumn(100);

    SetReagentState(static_cast<USESTATE>(i));
    i++;
}
