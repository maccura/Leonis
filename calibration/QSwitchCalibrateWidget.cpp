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
/// @file     QSwitchCalibrateWidget.h
/// @brief    页面切换选择
///
/// @author   5774/WuHongTao
/// @date     2022年2月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSwitchCalibrateWidget.h"
#include "shared/CommonInformationManager.h"
#include "calibration/ch/QCaliBrationNavigation.h"
#include "src/common/Mlog/mlog.h"

QSwitchCalibrateWidget::QSwitchCalibrateWidget(QWidget *parent)
    : QSwitchWidget(parent)
{
    Init();
}

QSwitchCalibrateWidget::~QSwitchCalibrateWidget()
{
}


///
/// @brief
///     页面初始化
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月12日，新建函数
///
void QSwitchCalibrateWidget::Init()
{
    auto devices = CommonInformationManager::GetInstance()->GetDeviceFromType(std::vector<tf::DeviceType::type>(\
    { tf::DeviceType::DEVICE_TYPE_C1000, tf::DeviceType::DEVICE_TYPE_ISE1005
        , tf::DeviceType::DEVICE_TYPE_C200, tf::DeviceType::DEVICE_TYPE_I6000 }));

    bool hasChemistry = false;
    bool hasImmune = false;
    for (const auto& device : devices)
    {
        if (device->deviceType == tf::DeviceType::DEVICE_TYPE_C1000
            || device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            hasChemistry = true;
        }
        else
        {
            hasImmune = true;
        }
    }

    // 生化页面的句柄
    //if (hasChemistry)
    //{
        RegisterPage(TAB_INDEX_CHEMISTRY, tr("   生化   "), QStringLiteral("btn_conner_chemistry"), new QCaliBrationNavigation(this));
    //}

    // 免疫页面句柄
    /*if (hasImmune)
    {
        RegisterPage(TAB_INDEX_IMMUNITY, tr("   免疫   "), QStringLiteral("btn_conner_immunity"), new QCaliBrationNavigation(this));
    }*/

    // 依次设置页面
    for (auto page : m_subPages)
    {
        page->SetPageInfos(m_pageInfos);
    }

    // 设置默认的显示句柄
	/*if (hasImmune)
	{
		ui.stackedWidget->setCurrentIndex(TAB_INDEX_IMMUNITY);
	}
	else if(hasChemistry)
	{
		ui.stackedWidget->setCurrentIndex(TAB_INDEX_CHEMISTRY);
	}*/
}
