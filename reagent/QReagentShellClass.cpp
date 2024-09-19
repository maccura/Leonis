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
#include "QReagentShellClass.h"
#include <QPushButton>
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/QNavigationPage.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"

#include "./im/imrgntplatewidget.h"
#include "./ReagentShowList.h"
#include "./QSupplyList.h"
#include "./ch/ReagentOverview.h"
#include "./ch/ReagentChemistryWidgetPlate.h"
#include "./ch/QPageIseSupply.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"

QReagentShellClass::QReagentShellClass(QWidget *parent, PAGENAVIGATION pageType)
    : QPageShellClass(parent),
    m_chemistryPage(nullptr),
    m_iseSupplyPage(Q_NULLPTR),
    m_pageType(pageType),
    m_overViewPage(nullptr),
    m_reagentChemistryList(nullptr),
    m_pRgntPlateWgt(nullptr),
	m_supplyOuter(nullptr)
{
    CreatSubPageByPageType(pageType);

	// 注册设备状态改变处理槽函数
	REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDeviceStateChange);
}

QReagentShellClass::~QReagentShellClass()
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
void QReagentShellClass::CreatSubPageByPageType(int type)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto deviceInfo = GetDeviceInfos();
    switch (type)
    {
    case REAGENTOVERVIEW:
        m_overViewPage = new ReagentOverview(this);
        addWidget(m_overViewPage);
        m_IsNeedShow = false;
        break;
    case REAGENTPLATE: // 试剂盘页面
    {
        m_pRgntPlateWgt = new ImRgntPlateWidget(this);
        m_chemistryPage = new ReagentChemistryWidgetPlate(this);
        m_iseSupplyPage = new QPageIseSupply(this);

        if (deviceInfo.size() != 0)
        {
            if (deviceInfo[0]->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
            {
                // 添加免疫页面
                 addWidget(m_pRgntPlateWgt);
                 m_chemistryPage->hide();
            }
            else
            {
                // 添加生化页面，试剂盘
                addWidget(m_chemistryPage);
                m_pRgntPlateWgt->hide();
            }
        }
        m_IsIse = true;
        break;
    }
    case REAGENTLIST:
        // 试剂信息页面
        m_reagentChemistryList = new ReagentShowList(this);
        addWidget(m_reagentChemistryList);
		m_Isall = true;
        break;
    case SUPPLIESLIST:
        // 耗材管理页面
        m_supplyOuter = new QSupplyList(this);
        m_Isall = true;
        m_IsIse = true;
        addWidget(m_supplyOuter);
        break;
    default:
        break;
    }
}

std::vector<std::shared_ptr<const tf::DeviceInfo>> QReagentShellClass::GetDeviceInfos()
{
    // 首先刷新设备信息
    auto deviceList = CommonInformationManager::GetInstance()->GetDeviceFromType(
        std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_C1000, 
            tf::DeviceType::DEVICE_TYPE_ISE1005, tf::DeviceType::DEVICE_TYPE_I6000 }));
    return std::move(deviceList);
}

///
/// @bref
///		切换到当前页面
///
/// @param[in] targetWidget 目标页面
///
/// @par History:
/// @li 8276/huchunli, 2022年12月2日，新建函数
///
void QReagentShellClass::SwitchWidgetToTarget(QWidget* targetWidget)
{
    if (targetWidget == nullptr)
    {
        return;
    }
    QWidget *currWidget = currentWidget();
    if (currWidget == targetWidget)
    {
        currWidget->update();
        return;
    }

    // 暂时只保留一个页面，减少多页面的管理复杂性（setCurrentIndex）
    removeWidget(currWidget);
    addWidget(targetWidget);
}

void QReagentShellClass::OnSelectPage()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 根据句柄查找对应的设备信息
    std::vector<std::shared_ptr<const tf::DeviceInfo>> vecDevcices;
    // 获取失败，直接返回
    if (!GetDevicesFromButton(vecDevcices) || vecDevcices.empty())
    {
        return;
    }

    // 依次查询设备状态  获取的是智能指针不能修改其状态
	/*
	for (auto& device : vecDevcices)
	{
		::tf::DeviceInfoQueryResp _return;
		::tf::DeviceInfoQueryCond diqc;
		diqc.__set_deviceSN(device->deviceSN);
		DcsControlProxy::GetInstance()->QueryDeviceInfo(_return, diqc);
		if (_return.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS
			&& !_return.lstDeviceInfos.empty())
		{
			device->__set_status(_return.lstDeviceInfos[0].status);
			device->__set_enableWasterContainer(_return.lstDeviceInfos[0].enableWasterContainer);
		}
	}
	*/
	
    // 页面切换
    switch (m_pageType)
    {
        // 试剂总览
        case REAGENTOVERVIEW:
            if (m_overViewPage == nullptr)
            {
                return;
            }

            // 试剂页面
            m_overViewPage->UpdateReagentPages();
            // 耗材信息
            m_overViewPage->UpdateSupplyPage(vecDevcices);
            break;

            // 试剂盘
        case REAGENTPLATE:
            if (vecDevcices[0]->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
            {
                // 切换页面
                SwitchWidgetToTarget(m_pRgntPlateWgt);

                m_pRgntPlateWgt->UpdateCurDev(vecDevcices[0]);
            }
            else if (vecDevcices[0]->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
            {
                SwitchWidgetToTarget(m_iseSupplyPage);
                m_iseSupplyPage->UpdateIseSupplies(vecDevcices[0]->deviceSN);
            }
            else
            {
                // 切换页面
                SwitchWidgetToTarget(m_chemistryPage);
                // 刷新试剂盘页面
                m_chemistryPage->UpdateGroupDevice(vecDevcices);
            }
            break;

            // 试剂列表
        case REAGENTLIST:
            // 传递消息到导航层
            connect(m_reagentChemistryList, SIGNAL(SendCondMessage(QString)), this, SIGNAL(ShowTipMessage(QString)));
            connect(m_reagentChemistryList, SIGNAL(SentHideFilterLable()), this, SIGNAL(HideFilterLable()));
            // 关闭查询消息的提示框
            connect(this, SIGNAL(CloseFilterLable()), m_reagentChemistryList, SLOT(OnFilterLableClosed()));
            m_reagentChemistryList->UpdateModeDevices(vecDevcices);
            break;

            // 耗材列表
        case SUPPLIESLIST:
            m_supplyOuter->RefreshSupplyList(vecDevcices);
            break;
        default:
            break;
    }
}

void QReagentShellClass::OnSelectButton()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取点击的控件句柄
    QPushButton* pButton = qobject_cast<QPushButton*>(sender());
    if (pButton == nullptr)
    {
        return;
    }

    // 跳转到对应的页面并刷新界面
    int btnNum = pButton->property("number").toInt();
    emit ChangePage(btnNum);
}

void QReagentShellClass::OnDeviceStateChange(tf::DeviceInfo deviceInfo)
{
	ULOG(LOG_INFO, "%s(): %s", __FUNCTION__, ToString(deviceInfo));
	// 设备状态更新，通知界面试剂扫描按钮更新 add by chenjianlin 20230802	
	if (nullptr != m_pRgntPlateWgt)
	{
		m_pRgntPlateWgt->DevStateChange();
	}
	if (nullptr != m_reagentChemistryList)
	{
		m_reagentChemistryList->DevStateChange();
	}

	// 更新液路灌注按钮使能 只针对ise设备 add by xuxiaolong 20231116	
	if (deviceInfo.deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
	{
		if (nullptr != m_iseSupplyPage)
		{
			m_iseSupplyPage->updatePerfusionBtnEnable(deviceInfo.deviceSN, deviceInfo.status);
		}
		if (nullptr != m_supplyOuter)
		{
			m_supplyOuter->updatePerfusionBtnEnable();
		}
	}
}

int QReagentShellClass::CurrentPageType()
{
    return TAB_INDEX_CHEMISTRY;
}
