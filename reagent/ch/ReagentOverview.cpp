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
/// @file     ReagentOverview.h
/// @brief    试剂总览页面
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
#include "ReagentOverview.h"
#include "ui_ReagentOverview.h"

#include <map>
#include "QDeviceSupplyShowList.h"
#include "QComputNeedDialog.h"
#include "AssayCardsManage.h"

#include "shared/ReagentCommon.h"
#include "shared/CommonInformationManager.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"

///
/// @brief
///     构造函数
///
/// @param[in]  parent  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
ReagentOverview::ReagentOverview(QWidget *parent)
    : QFrame(parent),
    m_ComputDialog(nullptr)
{
    ui = new Ui::ReagentOverview();
    ui->setupUi(this);
    // 下一个页面
    connect(ui->Next_Btn, SIGNAL(clicked()), this, SLOT(OnNextBtnClicked()));
    // 上一个页面
    connect(ui->Pre_Btn, SIGNAL(clicked()), this, SLOT(OnPreBtnClicked()));
    // 需求页面的对话框
    connect(ui->Comput_Button, SIGNAL(clicked()), this, SLOT(OnComputDialogClicked()));
}

///
/// @brief
///     析构函数
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
ReagentOverview::~ReagentOverview()
{
}

///
/// @brief
///     显示生化试剂总览
///
/// @param[in]  Reagents  生化试剂列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月10日，新建函数
///
void ReagentOverview::ShowChemistryReagentOverview(std::vector<::ch::tf::ReagentOverview>& ReagentOverViews)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    //项目名称--项目详细信息
    std::multimap<std::string, CardShowInfo> cardMap;

    for (const auto& Overview : ReagentOverViews)
    {
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(Overview.assayCode);
        if (Q_NULLPTR == spAssayInfo)
        {
            ULOG(LOG_WARN, "Invalid assay code");
            continue;
        }

        CardShowInfo card;
        card.assayCode = Overview.assayCode;
        card.assayName = spAssayInfo->assayName;

        // 设置为告警状态
        if (Overview.alarm)
        {
            card.state = WARING;
        }

        // 依次填写设备的剩余量
        for (const auto& Residuals : Overview.deviceResiduals)
        {
            card.deviceVolumn.insert(std::pair<std::string, int>(Residuals.first, static_cast<int>(Residuals.second)));
        }
        cardMap.insert(std::pair<std::string, CardShowInfo>(card.assayName, card));
    }

    // 1:形成模组信息显示
    // 2:转化模组名称等
    int iCount = 0;
    int ipageCount = 1;

    std::vector<std::vector<CardShowInfo>> CardPages;
    std::vector<CardShowInfo> cards;
    for (auto card : cardMap)
    {
        iCount++;
        // 生成模组试剂余量信息
        card.second.ToShowContent();
        // 生成状态信息
        ToImplementCardStatus(card.second);
        cards.push_back(card.second);

        // 满一页则换下一页
        if (iCount >= PAGENUMBER)
        {
            // 添加到页面
            AssayCardsManage* assayPage = new AssayCardsManage(this);
            // 合成页面名称
            assayPage->setObjectName(QString("assayPage"+QString::number(ipageCount)));
            ui->stackedWidget->addWidget(assayPage);
            assayPage->RefreshCards(cards);
            m_managePages.push_back(assayPage);
            cards.clear();
            iCount = 0;
            ipageCount++;
        }
    }

    // 刚好整数页时候返回
    if (cards.size() <= 0)
    {
        return;
    }

    // 添加最后一页
    AssayCardsManage* assayPage = new AssayCardsManage(this);
    // 合成页面名称
    assayPage->setObjectName(QString("assayPage" + QString::number(ipageCount)));
    ui->stackedWidget->addWidget(assayPage);
    assayPage->RefreshCards(cards);
    m_managePages.push_back(assayPage);
}

///
/// @brief
///     判断和设置上下翻页键
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月10日，新建函数
///
void ReagentOverview::JudgeStatus()
{
    // 获取当前页的index
    int pageIndex = ui->stackedWidget->currentIndex();

    // 判断是否移动到最前
    if (pageIndex <= 0)
    {
        ui->Pre_Btn->setDisabled(true);
    }
    else
    {
        ui->Pre_Btn->setDisabled(false);
    }

    // 移动到了最后
    if(pageIndex >= (m_managePages.size()-1))
    {
        ui->Next_Btn->setDisabled(true);
    }
    else
    {
        ui->Next_Btn->setDisabled(false);
    }
}

///
/// @brief
///     生成试剂卡片的状态信息
///
/// @param[in]  card  试剂卡片信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月11日，新建函数
///
void ReagentOverview::ToImplementCardStatus(CardShowInfo& card)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 等于0就是错误状态
    if (card.lastVolumn <= 0)
    {
        card.state = ERRORSTATE;
    }
}

///
/// @brief
///     清除之前的page页面的数据，避免内存泄漏
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月11日，新建函数
///
void ReagentOverview::ClearPages()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 依次删除页面
    for (auto page : m_managePages)
    {
        ui->stackedWidget->removeWidget(page);
        delete page;        
    }

    // 清空容器
    m_managePages.clear();
}

///
/// @brief
///     前一页按钮被选中
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
void ReagentOverview::OnPreBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取当前页的index
    int pageIndex = ui->stackedWidget->currentIndex();
    // 向前移动
    ui->stackedWidget->setCurrentIndex(--pageIndex);

    // 重新设置按键状态
    JudgeStatus();

    // 设置页面的导向
    ui->label->setText(QString("%1/%2").arg(pageIndex+1).arg(m_managePages.size()));
}

///
/// @brief
///     后一页按钮被选中
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建函数
///
void ReagentOverview::OnNextBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取当前页的index
    int pageIndex = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(++pageIndex);

    // 重新设置按键状态
    JudgeStatus();

    // 设置页面的导向
    if ((pageIndex + 1) <= m_managePages.size())
    {
        ui->label->setText(QString("%1/%2").arg(pageIndex + 1).arg(m_managePages.size()));
    }
}

void ReagentOverview::OnComputDialogClicked()
{
    if (m_ComputDialog == nullptr)
    {
        m_ComputDialog = new QComputNeedDialog(this);
    }

    m_ComputDialog->show();
}

///
/// @brief
///     更新设备信息
///
/// @param[in]  deviceType  
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月10日，新建函数
///
void ReagentOverview::UpdateReagentPages()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 首先清空历史数据
    ClearPages();
    // 查询试剂总览
    m_reagentOverViews.clear();
    ch::LogicControlProxy::QueryReagentOverviews(m_reagentOverViews);

    if (m_reagentOverViews.empty())
    {
        return;
    }

    // 刷新页面
    ShowChemistryReagentOverview(m_reagentOverViews);

    // 设置状态栏
    JudgeStatus();
    ui->label->setText(QString("%1/%2").arg(1).arg(m_managePages.size()));
}

///
/// @brief
///     更新设备耗材信息
///
/// @param[in]  devList  设备信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void ReagentOverview::UpdateSupplyPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devList)
{
    std::map<std::shared_ptr<const tf::DeviceInfo>, std::vector<ch::tf::SuppliesOverview>> list;

    int iCount = 0;
    // 依次查询设备的耗材信息
    for (auto device : devList)
    {
        std::vector< ::ch::tf::SuppliesOverview> _return;
        ch::LogicControlProxy::QuerySuppliesOverviews(_return, device->deviceSN);
        if (_return.empty())
        {
            continue;
        }

        // 插入数据
        list.insert(std::pair<std::shared_ptr<const tf::DeviceInfo>, std::vector<ch::tf::SuppliesOverview>>(device, _return));
        iCount++;

        if (iCount >= 3)
        {
            QDeviceSupplyShowList* page = new QDeviceSupplyShowList(this);
            page->RefreshSupplyList(list);
            ui->stackedWidget_2->addWidget(page);
            list.clear();
            iCount = 0;
        }
    }

    // 添加最后一页
    if (!list.empty())
    {
        QDeviceSupplyShowList* page = new QDeviceSupplyShowList(this);
        page->RefreshSupplyList(list);
        ui->stackedWidget_2->addWidget(page);
        list.clear();
    }

    ui->stackedWidget_2->setCurrentIndex(0);
}

///
/// @brief
///     获取数据总览
///
/// @param[out]  ReagentOverviews  数据总览
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月16日，新建函数
///
void ReagentOverview::GetOverviewData(std::vector<::ch::tf::ReagentOverview>& ReagentOverviews)
{
    ReagentOverviews = m_reagentOverViews;
}
