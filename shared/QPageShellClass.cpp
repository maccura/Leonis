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
/// @file     QPageShellClass.h
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
#include "QPageShellClass.h"
#include <QRadioButton>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QButtonGroup>
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "QNavigationPage.h"
#include "src/common/Mlog/mlog.h"

QPageShellClass::QPageShellClass(QWidget *parent)
    : QStackedWidget(parent),
    m_cornerItem(nullptr),
    m_IsNeedShow(true),
    m_Isall(false),
	m_IsIse(false),
    m_showItem(true),
    m_deviceTypeShow(false),
    m_parent(static_cast<QTabWidget*>(parent))
{
    m_buttons.clear();	
	m_deviceClassifyBtns.clear();
}

QPageShellClass::~QPageShellClass()
{
}

void QPageShellClass::RefreshCornerWidget(int index)
{

}

void QPageShellClass::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_parent->setCornerWidget(0);
    // 基类先处理
    QWidget::showEvent(event);
    // 句柄检查（不能为空）
    if (m_parent == nullptr || m_cornerItem == nullptr)
    {
        ULOG(LOG_ERROR, "%s", "The parent is null or corner is null");
        return;
    }

    if (m_showItem)
    {
        // 设置切换句柄
        m_parent->setCornerWidget(m_cornerItem);
        m_cornerItem->show();
    }

    // 设置当前页面右上角切换按钮的状态
    int curPageType = CurrentPageType();
    for (auto& btn : m_deviceClassifyBtns)
    {
        if (btn->property("number").toInt() == curPageType)
        {
            btn->setChecked(true);
            break;
        }
    }
}

///
/// @brief
///     生成右上角按钮控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建函数
///
void QPageShellClass::SetCornerButtonWidget(const std::vector<CornerButtonInfo>& pageInfos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置水平布局器
    m_cornerItem = new QWidget(m_parent);
    QHBoxLayout* hlayout = new QHBoxLayout(m_cornerItem);
	if (m_deviceTypeShow)
	{
		hlayout->setContentsMargins(0, 0, 0, 0);
		hlayout->setSpacing(0);
		m_cornerItem->setObjectName("bgCorner");
	}
	else
	{
		hlayout->setContentsMargins(0, 0, 31, 17);
		hlayout->setSpacing(20);
	}
	m_cornerItem->setLayout(hlayout);
	
    if (pageInfos.size() >= 2)
    {
        // 按照规则依次生成按钮
        for (auto pageinfo : pageInfos)
        {
            QPushButton* functionButton = new QPushButton(pageinfo.m_buttonText, m_cornerItem);
            functionButton->setObjectName(pageinfo.m_objectName);
            functionButton->setProperty("number", pageinfo.m_propertyNumber);

            functionButton->setCheckable(true);
            functionButton->setAutoExclusive(true);

            m_deviceClassifyBtns.push_back(functionButton);
            // 添加到布局
            hlayout->addWidget(functionButton);
            // 根据按键类型--生化类型
            connect(functionButton, SIGNAL(clicked()), this, SLOT(OnSelectButton()));
        }
    }

    GenDeviceButton();
    m_cornerItem->hide();
}

///
/// @brief
///     设置选择按钮的状态
///
/// @param[in]  status  状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月23日，新建函数
///
void QPageShellClass::HideSelectButton(bool status)
{
    m_IsNeedShow = status;
}

void QPageShellClass::SetSelectByDevSn(const std::string &devSn)
{
	ULOG(LOG_INFO, "%s(): devSn: %s", __FUNCTION__, devSn);

	for (auto & btn : m_DeviceButton)
	{
		for(auto &dev : btn.second)
		{
			if (dev->deviceSN == devSn && btn.first)
			{
				btn.first->setChecked(true);
				emit btn.first->clicked();
				return;
			}
		}
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
std::vector<std::shared_ptr<const tf::DeviceInfo>> QPageShellClass::GetDeviceInfos()
{
    // 首先刷新设备信息
    std::vector<std::shared_ptr<const tf::DeviceInfo>> list = CommonInformationManager::GetInstance()->GetDeviceFromType(\
        std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_C1000, tf::DeviceType::DEVICE_TYPE_ISE1005}));
    return std::move(list);
}

///
/// @brief 生成设备按钮
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月22日，新建函数
///
void QPageShellClass::GenDeviceButton()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_cornerItem == nullptr)
    {
        return;
    }
    // 查询设备列表
    auto deviceList = GetDeviceInfos();
    if (deviceList.empty())
    {
        ULOG(LOG_WARN, "The device is empty");
        // 跳转到对应的页面并刷新界面
        return;
    }
    std::sort(deviceList.begin(), deviceList.end(), 
        [](std::shared_ptr<const tf::DeviceInfo> spDevInfo1, std::shared_ptr<const tf::DeviceInfo> spDevInfo2){
		if (spDevInfo1 == nullptr || spDevInfo2 == nullptr)
		{
			return false;
		}
		return ((spDevInfo1->groupName + spDevInfo1->name) < (spDevInfo2->groupName + spDevInfo2->name));
	});

    QHBoxLayout* layout = static_cast<QHBoxLayout*>(m_cornerItem->layout());
    if (layout == nullptr)
    {
        return;
    }

    // bool ret = true;
    // 只有一种设备的时候，不显示生化和免疫
    if (!(m_deviceTypeShow))
    {
        for (const auto& deviceBtn : m_deviceClassifyBtns)
        {
            deviceBtn->hide();
        }
    }

    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    // 设置互斥
    pButtonGroup->setExclusive(true);
    // 依次添加设备按钮
    m_DeviceButton.clear();

    std::map<QString, QRadioButton*> butMap;
    bool once = false;
    QRadioButton *firstButton = nullptr;
	// 是否包含ise设备
	bool hasIseDev = false;
    // 添加其他的按钮
    for (const auto& device : deviceList)
    {
        QString deviceName;
        // 存在设备组信息
        if (device->__isset.groupName && !device->groupName.empty())
        {
            deviceName = QString::fromStdString(device->groupName);
        }
        else
        {
            deviceName = QString::fromStdString(device->name);
        }

        // 说明存在同一个组的设备(只会存在同一个设备按钮)
        if (butMap.count(deviceName) > 0)
        {
            auto btn = butMap[deviceName];
            if (m_DeviceButton.count(btn) > 0)
            {
                m_DeviceButton[btn].push_back(device);
            }
            continue;
        }

        QRadioButton *pButton = new QRadioButton(deviceName, m_cornerItem);
        pButton->setObjectName(deviceName);
        butMap[deviceName] = pButton;
        connect(pButton, SIGNAL(clicked()), this, SLOT(OnSelectPage()));
        m_DeviceButton[pButton].push_back(device);
        layout->addWidget(pButton);
        pButtonGroup->addButton(pButton);
        m_buttons.push_back(pButton);

		if (device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
		{
			hasIseDev = true;
		}

        if ((!m_IsIse && device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
            || !m_IsNeedShow)
        {
            pButton->hide();
            if (m_IsNeedShow)
            {
                continue;
            }
        }

        // 获取第一个按钮，该方法获取第一个按钮有缺陷，如果生化单机版数据库将ise放置在第一个，
		// 那么获取的是ise选中，此时的试剂列表界面会有问题，后面有时间再改，解决：单机时试剂列表界面不new pButton为ise的
        if (!once)
        {
            firstButton = pButton;
            once = true;
        }
    }

    // 点击第一个设备的函数：
    // 只有一个设备，或者没有“全部”时调用
    auto FuncClickFisrtBtn = [&] {
        // 默认展示第一个设备内容
        if (once && firstButton != nullptr)
        {
            firstButton->setChecked(true);
            emit firstButton->clicked();
        }
    };

    // 单机版的情况下，并且只有一个设备的情况下不显示任何
    if (!DictionaryQueryManager::GetInstance()->GetPipeLine() 
        && m_DeviceButton.size() == 1)
    {
        FuncClickFisrtBtn();
        auto deviceButton = m_DeviceButton.begin()->first;
        deviceButton->hide();
        return;
    }

	// 用于试剂列表页面只有一个生化设备时，隐藏全部按钮和对应设备按钮 
	if (!m_IsIse)
	{
		// 设备为2
		if (m_DeviceButton.size() == 2 && hasIseDev)
		{
			// 包含ise设备,那么隐藏全部按钮和对应设备按钮
			m_DeviceButton.begin()->first->hide();
			m_DeviceButton.rbegin()->first->hide();
			m_Isall = false;
		}
		// 只有一个设备时直接隐藏全部按钮和对应设备按钮
		else if(m_DeviceButton.size() == 1)
		{
			m_DeviceButton.begin()->first->hide();
			m_Isall = false;
		}
	}

    // 是否显示全部
    if (m_Isall)
    {
        QRadioButton *pButton = new QRadioButton(tr("全部"));
        connect(pButton, SIGNAL(clicked()), this, SLOT(OnSelectPage()));
        layout->insertWidget(0, pButton);
        pButtonGroup->addButton(pButton);
        pButton->setChecked(true);
        emit pButton->clicked();
        m_buttons.push_back(pButton);
    }
    else
    {
        FuncClickFisrtBtn();
    }
}

/////
///// @brief 选中生化和免疫的切换
/////
///// @par History:
///// @li 5774/WuHongTao，2022年9月22日，新建函数
/////
//void QPageShellClass::OnSelectButton()
//{
//}
//
/////
///// @brief 选中不同的功能页
/////
/////
///// @par History:
///// @li 5774/WuHongTao，2022年9月22日，新建函数
/////
//void QPageShellClass::OnSelectPage()
//{
//
//}

///
/// @brief
///     根据按钮信息获取设备信息
///
/// @param[out]  devices  设备信息
///
/// @return true表示成功，反之失败
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数
///
bool QPageShellClass::GetDevicesFromButton(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取点击的控件句柄
    QRadioButton* pButton = qobject_cast<QRadioButton*>(sender());
    if (pButton == nullptr)
    {
        return false;
    }

    // 根据句柄查找对应的设备信息
    if (m_DeviceButton.find(pButton) == m_DeviceButton.end())
    {
        for (auto button : m_DeviceButton)
        {
            devices.insert(devices.end(),button.second.begin(), button.second.end());
        }
    }
    else
    {
        devices.insert(devices.end(), m_DeviceButton[pButton].begin(), m_DeviceButton[pButton].end());
    }

    return true;
}

QPushButton* QPageShellClass::GetCurrentPageButton(int buttonNum)
{
    for (auto& btn : m_deviceClassifyBtns)
    {
        if (btn != nullptr && btn->property("number").toInt() == buttonNum)
        {
            return btn;
        }
    }
    
    return nullptr;
}
