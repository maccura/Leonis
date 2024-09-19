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
/// @file     QSingleDeviceSwitch.cpp
/// @brief    校准历史界面
///
/// @author   8090/YeHuaNing
/// @date     2022年12月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QSingleDeviceSwitch.h"
#include "ui_QSingleDeviceSwitch.h"
#include "src/common/Mlog/mlog.h"

QSingleDeviceSwitch::QSingleDeviceSwitch(QWidget *parent)
    : QWidget(parent)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::QSingleDeviceSwitch();
    ui->setupUi(this);
    
}

QSingleDeviceSwitch::~QSingleDeviceSwitch()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief刷新设备试剂列表
///     
///
/// @param[in]  devices  设备列表
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月13日，新建函数
///
void QSingleDeviceSwitch::RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices)
{
    static QWidget nullPage;
    if (m_mapClassifyFunc.find(devices[0]->deviceClassify) != m_mapClassifyFunc.end())
    {
        QWidget* page = m_mapClassifyFunc[devices[0]->deviceClassify](devices);
        if (ui->caliHisStack->indexOf(page) == -1)
            ui->caliHisStack->addWidget(page);

        ui->caliHisStack->setCurrentWidget(page);
    }
    else
    {
        if (ui->caliHisStack->indexOf(&nullPage) == -1)
            ui->caliHisStack->addWidget(&nullPage);

        ui->caliHisStack->setCurrentWidget(&nullPage);
    }
}

void QSingleDeviceSwitch::RefreshPage(tf::AssayClassify::type classify)
{
    static QWidget nullPage;
    if (m_mapClassifyWidget.find(classify) != m_mapClassifyWidget.end())
    {
        QWidget* page = m_mapClassifyWidget[classify];
        if (ui->caliHisStack->indexOf(page) == -1)
            ui->caliHisStack->addWidget(page);

        ui->caliHisStack->setCurrentWidget(page);
    }
    else
    {
        if (ui->caliHisStack->indexOf(&nullPage) == -1)
            ui->caliHisStack->addWidget(&nullPage);

        ui->caliHisStack->setCurrentWidget(&nullPage);
    }
}

///
/// @brief 设置不同类型设备的设备刷新函数
///
/// @param[in]  classify   设备类型
/// @param[in]  func  调用函数
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月14日，新建函数
///
void QSingleDeviceSwitch::SetClassifyFunc(tf::AssayClassify::type classify, const std::function<QWidget*(std::vector<std::shared_ptr<const tf::DeviceInfo>>&)>& func)
{
    m_mapClassifyFunc[classify] = func;
}

void QSingleDeviceSwitch::SetClassifyWidget(tf::AssayClassify::type classify, QWidget * const page)
{
    m_mapClassifyWidget[classify] = page;
}

