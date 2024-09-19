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

#include <map>
#include <QPushButton>
#include <QHBoxLayout>
#include "QCaliBrationShellClass.h"
#include "QCalibrateSet.h"
#include "QCalibrateOverView.h"
#include "QCaliBrateHistory.h"

#include "shared/CommonInformationManager.h"
#include "shared/QSingleDeviceSwitch.h"
#include "../im/ImQCalibrateSet.h"
#include "../im/ImQCaliBrateHistory.h"
#include "../ise/QIseCaliBrateHistory.h"
#include "src/common/Mlog/mlog.h"

QCaliBrationShellClass::QCaliBrationShellClass(QWidget *parent, CALIBRATEPAGENAVIGATION pageType)
    : QPageShellClass(parent),
    m_hasChemistry(false),
    m_hasImmune(false),
    m_calibrateOverview(nullptr),
    m_caliBrateHistory(nullptr),
    m_caliBrateSet(nullptr),
    m_pageType(pageType)
{
    // 创建页面
    CreatSubPageByPageType(pageType);
}

QCaliBrationShellClass::~QCaliBrationShellClass()
{
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
std::vector<std::shared_ptr<const tf::DeviceInfo>> QCaliBrationShellClass::GetDeviceInfos()
{
	std::vector<std::shared_ptr<const tf::DeviceInfo>> deviceList = CommonInformationManager::GetInstance()->GetDeviceFromType(\
        std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_C1000,
        tf::DeviceType::DEVICE_TYPE_ISE1005, tf::DeviceType::DEVICE_TYPE_I6000 }));
    return deviceList;
}

void QCaliBrationShellClass::SetCornerButtonWidget(const std::vector<CornerButtonInfo>& pageInfos)
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

    bool isChecked = false;
    if (m_hasChemistry)
    {
        isChecked = true;
        QPushButton*functionChButton = new QPushButton(tr("   生化   "), m_cornerItem);
        functionChButton->setObjectName(QStringLiteral("btn_conner_chemistry"));
        functionChButton->setProperty("number", TAB_INDEX_CHEMISTRY);
        functionChButton->setCheckable(true);
        functionChButton->setChecked(isChecked);
        functionChButton->setAutoExclusive(true);

        m_deviceClassifyBtns.push_back(functionChButton);
        // 添加到布局
        hlayout->addWidget(functionChButton);
        // 根据按键类型--生化类型
        connect(functionChButton, SIGNAL(clicked()), this, SLOT(OnSelectButton()));
    }

    if (m_hasImmune)
    {
        QPushButton*functionImButton = new QPushButton(tr("   免疫   "), m_cornerItem);
        functionImButton->setObjectName(QStringLiteral("btn_conner_immunity"));
        functionImButton->setProperty("number", TAB_INDEX_IMMUNITY);
        functionImButton->setCheckable(true);
        functionImButton->setChecked(!isChecked);
        functionImButton->setAutoExclusive(true);

        m_deviceClassifyBtns.push_back(functionImButton);
        // 添加到布局
        hlayout->addWidget(functionImButton);
        // 根据按键类型--生化类型
        connect(functionImButton, SIGNAL(clicked()), this, SLOT(OnSelectButton()));
    }

    // 选中一个页面
    for (const auto& btn : m_deviceClassifyBtns)
    {
        if (btn == nullptr || !btn->isChecked())
            continue;

        emit btn->clicked();
        break;
    }

    GenDeviceButton();
    m_cornerItem->hide();
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
void QCaliBrationShellClass::CreatSubPageByPageType(int type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	auto devices = CommonInformationManager::GetInstance()->GetDeviceFromType(std::vector<tf::DeviceType::type>(\
	{ tf::DeviceType::DEVICE_TYPE_C1000, tf::DeviceType::DEVICE_TYPE_ISE1005
		, tf::DeviceType::DEVICE_TYPE_C200, tf::DeviceType::DEVICE_TYPE_I6000 }));
	for (const auto& device : devices)
	{
		if (device->deviceType == tf::DeviceType::DEVICE_TYPE_C1000
			|| device->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
		{
            m_hasChemistry = true;
		}
		else
		{
			m_hasImmune = true;
		}
	}

    switch (type)
    {
        // 校准申请
    case CALIBRATEOVERVIEW:
        m_calibrateOverview = new QCalibrateOverView(this);
        addWidget(m_calibrateOverview);
        m_Isall = true;
        m_IsIse = true;
        break;
        // 校准历史
    case CALIBRATEHIS:
        m_caliBrateHistory = new QSingleDeviceSwitch(this);
        {
            QCaliBrateHistory* chCaliHis = new QCaliBrateHistory;
            m_caliBrateHistory->SetClassifyFunc(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY, 
                std::bind(&QCaliBrateHistory::RefreshPage, chCaliHis, std::placeholders::_1));
        }
        {
            ImQCaliBrateHistory* imCaliHis = new ImQCaliBrateHistory;
            m_caliBrateHistory->SetClassifyFunc(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE,
                std::bind(&ImQCaliBrateHistory::RefreshPage, imCaliHis, std::placeholders::_1));
        }
        {
            QIseCaliBrateHistory* iseCaliHis = new QIseCaliBrateHistory;
            m_caliBrateHistory->SetClassifyFunc(tf::AssayClassify::ASSAY_CLASSIFY_ISE,
                std::bind(&QIseCaliBrateHistory::RefreshPage, iseCaliHis, std::placeholders::_1));
        }
        // 传递消息到导航层
        //connect(m_caliBrateHistory, SIGNAL(SendCondMessage(QString)), this, SIGNAL(ShowTipMessage(QString)));
        // 关闭查询消息的提示框(校准页面没有快捷筛选)
        // connect(this, SIGNAL(CloseFilterLable()), m_caliBrateHistory, SLOT(OnRefreshHistory()));
        addWidget(m_caliBrateHistory);
        m_Isall = false;
        m_IsIse = true;
        break;
        // 校准设置
    case CALIBRATESET:
        m_caliBrateSet = new QSingleDeviceSwitch(this);
        if (m_hasChemistry)
        {
            m_caliBrateSet->SetClassifyWidget(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY, new QCalibrateSet);
        }
        if (m_hasImmune)
        {
            m_caliBrateSet->SetClassifyWidget(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE, new ImQCalibrateSet);
        }
        m_IsNeedShow = false;
        m_deviceTypeShow = true;
        if (!(m_hasChemistry && m_hasImmune))
            m_showItem = false;
        break;
    default:
        break;
    }
}

///
/// @brief
///     子模块按钮被选中
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建函数
///
void QCaliBrationShellClass::OnSelectPage()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 根据句柄查找对应的设备信息
    std::vector<std::shared_ptr<const tf::DeviceInfo>> vecDevcices;
    // 获取失败，直接返回
    if (!GetDevicesFromButton(vecDevcices))
    {
        return;
    }

    switch (m_pageType)
    {
        // 校准概况
    case CALIBRATEOVERVIEW:
        if (m_calibrateOverview == nullptr)
        {
            return;
        }
        m_calibrateOverview->RefreshPage(vecDevcices);
        break;

        // 校准历史
    case CALIBRATEHIS:
        if (m_caliBrateHistory == nullptr)
        {
            return;
        }
        m_caliBrateHistory->RefreshPage(vecDevcices);
        break;

        // 校准设置
    case CALIBRATESET:
    {
        if (m_caliBrateSet != nullptr)
        {
            return;
        }

        int curPage = CurrentPageType();
        if (curPage == TAB_INDEX_CHEMISTRY)
        {
            m_caliBrateSet->RefreshPage(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
        }
        else if (curPage == TAB_INDEX_IMMUNITY)
        {
            m_caliBrateSet->RefreshPage(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE);
        }
        break;
    }

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
void QCaliBrationShellClass::OnSelectButton()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取点击的控件句柄
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    if (pButton == nullptr)
    {
        return;
    }
    int btnNum = pButton->property("number").toInt();

    if (m_caliBrateSet != Q_NULLPTR)
    {
        if (btnNum == TAB_INDEX_CHEMISTRY)
        {
            m_caliBrateSet->RefreshPage(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
        }
        else
        {
            m_caliBrateSet->RefreshPage(tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE);
        }
    }
}

int QCaliBrationShellClass::CurrentPageType()
{
    // 选中一个页面
    for (const auto& btn : m_deviceClassifyBtns)
    {
        if (btn == nullptr || !btn->isChecked())
            continue;

        return btn->property("number").toInt();
    }

    return -1;
}
