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
/// @file     QReagentShellClass.h
/// @brief    生化免疫试剂外壳页面
///
/// @author   5774/WuHongTao
/// @date     2022年1月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QQcShellClass.h"

#include <QPushButton>
#include "shared/CommonInformationManager.h"
#include "shared/QNavigationPage.h"
#include "QQcAppPage.h"
#include "QQcSettingPage.h"
#include "./LeveyJennings/QQcLeveyJennPage.h"

#include "src/common/Mlog/mlog.h"

QQcShellClass::QQcShellClass(QWidget *parent, int pageType)
    : QPageShellClass(parent)
    ,m_setShell(nullptr)
    , m_pageType(pageType)
    ,m_qcAppShell(nullptr)
{
    CreatSubPageByPageType(pageType);
}

QQcShellClass::~QQcShellClass()
{
}


///
/// @brief
///     根据页面类型，生成不同的页面
///
/// @param[in]  type  页面类型
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月21日，新建函数
///
void QQcShellClass::CreatSubPageByPageType(int type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    switch (type)
    {
    // 质控申请
    case 0:
        m_qcAppShell = new QQcAppPage(this);
        addWidget(m_qcAppShell);
		m_IsNeedShow = true;
        break;
    // Levey-Jennings图
    case 1:
        m_leveyShell = new QQcLeveyJennPage(this);
        addWidget(m_leveyShell);
        m_IsNeedShow = true;
        break;
    // Twin Plot图
    case 2:
        /*m_twinShell = new QQcTwinPlotPage(this);
        addWidget(m_twinShell);*/
        m_IsNeedShow = true;
        break;
    // 质控设置
    case 3:
		m_setShell = new QQcSettingPage(this);
        addWidget(m_setShell);
		m_IsNeedShow = false;
        m_showItem = false;
        break;
    default:
        break;
    }
}

///
/// @brief
///     获取设备信息列表
///
///
/// @return 设备信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月18日，新建函数
///
std::vector<std::shared_ptr<const tf::DeviceInfo>> QQcShellClass::GetDeviceInfos()
{
	// 首先刷新设备信息
	std::vector<std::shared_ptr<const tf::DeviceInfo>> deviceList = CommonInformationManager::GetInstance()->GetDeviceFromType(\
        std::vector<tf::DeviceType::type>({tf::DeviceType::DEVICE_TYPE_C1000, tf::DeviceType::DEVICE_TYPE_ISE1005}));
    return deviceList;
}

///
/// @brief
///     子模块按钮被选中
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建函数
///
void QQcShellClass::OnSelectPage()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 根据句柄查找对应的设备信息
    std::vector<std::shared_ptr<const tf::DeviceInfo>> vecDevcices;
    // 获取失败，直接返回
    if (!GetDevicesFromButton(vecDevcices) || vecDevcices.empty())
    {
        return;
    }

    switch (m_pageType)
    {
        // 质控申请
        case 0:
            if (m_qcAppShell != nullptr)
            {
                m_qcAppShell->StartQcAppPage(vecDevcices);
            }
            break;
        case 1:
            if (m_leveyShell != nullptr)
            {
                m_leveyShell->StartLeveyJennAppPage(vecDevcices);
            }
            break;
            // 质控设置
        case 3:
            break;
    default:
        break;
    }
}

///
/// @brief 选中生化和免疫的切换
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月22日，新建函数
///
void QQcShellClass::OnSelectButton()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取点击的控件句柄
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    if (pButton == nullptr)
    {
        return;
    }
    int btnNum = pButton->property("number").toInt();
    // 跳转到对应的页面并刷新界面
    emit ChangePage(btnNum);
}

int QQcShellClass::CurrentPageType()
{
    return TAB_INDEX_CHEMISTRY;
}
