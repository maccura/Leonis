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
/// @file     devmaintainstatusgrp.cpp
/// @brief    应用->维护保养界面
///
/// @author   4170/TangChuXian
/// @date     2023年2月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "devmaintainstatusgrp.h"
#include <QHBoxLayout>
#include <QEvent>
#include <QPushButton>
#include "maintaindatamng.h"
#include "uidcsadapter/uidcsadapter.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

#define MAX_DEV_CNT_OF_DISPLAY                          (5)                         // 最大显示设备数

#define PREV_PAGE_BTN_OBJ_NAME                          ("PrevBtnOfDevMtStatus")    // 设备维护状态向前按钮
#define NEXT_PAGE_BTN_OBJ_NAME                          ("NextBtnOfDevMtStatus")    // 设备维护状态向后按钮

DevMaintainStatusGrp::DevMaintainStatusGrp(QWidget *parent)
    : QWidget(parent),
      m_bInit(false),
      m_iCurPage(1),
      m_iTotalPage(1),
      m_pPrevPageBtn(new QPushButton()),
      m_pNextPageBtn(new QPushButton())
{
    // 显示之前初始化
    InitBeforeShow();
}

DevMaintainStatusGrp::~DevMaintainStatusGrp()
{
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void DevMaintainStatusGrp::InitBeforeShow()
{
    // 获取设备列表
    gUiAdapterPtr()->GetDevSnVectorWithTrack(m_vecDevSn);

    // 更新设备页数
    if (m_vecDevSn.isEmpty())
    {
        m_iTotalPage = 1;
    }
    else
    {
        m_iTotalPage = (m_vecDevSn.size() - 1) / MAX_DEV_CNT_OF_DISPLAY + 1;
    }

    // 只有1页，则隐藏翻页按钮
    if (m_iTotalPage <= 1)
    {
        m_pPrevPageBtn->setVisible(false);
        m_pNextPageBtn->setVisible(false);
    }
    else
    {
        m_pPrevPageBtn->setVisible(true);
        m_pNextPageBtn->setVisible(true);
    }

    // 翻页按钮设置固定宽度
    m_pPrevPageBtn->setObjectName(PREV_PAGE_BTN_OBJ_NAME);
    m_pNextPageBtn->setObjectName(NEXT_PAGE_BTN_OBJ_NAME);
    m_pPrevPageBtn->setFixedSize(62, 304);
    m_pNextPageBtn->setFixedSize(62, 304);

    // 使用水平布局
    QHBoxLayout* pHLayout = new QHBoxLayout(this);
    pHLayout->setMargin(0);
    pHLayout->setSpacing(10);

    // 添加向前翻页按钮
    pHLayout->addWidget(m_pPrevPageBtn);

    // 构造设备状态控件
    m_vDevWgt.clear();
	// 维护显示模式
	DevMaintainStatusWgt::DevMaintainInfoShowModel maintainShowModel = GetDevsMaintianShowModel();
    for (int i = 0; i < MAX_DEV_CNT_OF_DISPLAY; i++)
    {
        // 创建对象
        DevMaintainStatusWgt* pDevWgt = new DevMaintainStatusWgt();
		pDevWgt->SetMaintainInfoShowModel(maintainShowModel);

        // 加入布局
        pHLayout->addWidget(pDevWgt, 1);
        m_vDevWgt.push_back(pDevWgt);

        // 连接信号槽
        connect(pDevWgt, SIGNAL(SigDevDetailWgtClicked(QString)), this, SIGNAL(SigDevWgtClicked(QString)));
    }

    // 添加向后翻页按钮
    pHLayout->addWidget(m_pNextPageBtn);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void DevMaintainStatusGrp::InitAfterShow()
{
    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void DevMaintainStatusGrp::InitChildCtrl()
{
    // 默认显示第一页
    SetCurrentPage(1);
}

///
/// @brief
///     设置当前页
///
/// @param[in]  iPage  页码编号
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月25日，新建函数
///
void DevMaintainStatusGrp::SetCurrentPage(int iPage)
{
    // 参数检查
    if ((iPage <= 0) || (iPage > m_iTotalPage))
    {
        return;
    }

    // 更新当前页
    m_iCurPage = iPage;

    // 重置设备维护状态控件
    ResetAllDevStatusWgt();

    // 按照维护详情显示设备维护状态界面
    auto funcUpdateDetailToUi = [this](DevMaintainStatusWgt* pDevWgt, const QString& strDevSn, const tf::LatestMaintainInfo& stuDetailInfo)
    {
        // 如果结果为空，则返回
        if (stuDetailInfo.id <= 0)
        {
            pDevWgt->SetDeviceSn(strDevSn);
            pDevWgt->SetStatus(DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN);
            return;
        }

        // 设置设备维护信息
        pDevWgt->SetDeviceSn(strDevSn);

		// 获取进度信息信息
		QString strProgress = "";
		QString strItemName = "";
		QString strItemCount = "";
		GetMaintainProcessByLatestMaintainInfo(stuDetailInfo, strItemName, strProgress, strItemCount);
		
        // 构造状态
        DevMaintainStatusWgt::DevMaintainStatus enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
        if (stuDetailInfo.exeResult == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
        {
            enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
            strProgress.clear();
        }
        else if (stuDetailInfo.exeResult == tf::MaintainResult::MAINTAIN_RESULT_FAIL)
        {
            enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_FAILED;
        }
        else if (stuDetailInfo.exeResult == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
        {
            enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_MAINTAINING;
        }
        else if (stuDetailInfo.exeResult == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
        {
            enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_SUCC;
        }
        else
        {
            enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
            strProgress.clear();
        }

        // 更新设备维护控件的状态和进度
        pDevWgt->SetStatus(enStatus);
        pDevWgt->SetProgress(strProgress, strItemCount, strItemName);

		// 默认获取完成时间
		QString maintainTime = "";
		if (stuDetailInfo.__isset.finishTime && !stuDetailInfo.finishTime.empty())
		{
			maintainTime = QString::fromStdString(stuDetailInfo.finishTime);
		}
		else
		{
			maintainTime = QString::fromStdString(stuDetailInfo.exeTime);
		}

        pDevWgt->SetMaintainTime(maintainTime);
    };

    // 加载设备
    int iDevWgtIdx = 0;
    m_mapDevToWgt.clear();
    for (int i = (iPage - 1) * MAX_DEV_CNT_OF_DISPLAY; i < iPage * MAX_DEV_CNT_OF_DISPLAY; i++, iDevWgtIdx++)
    {
        // 如果索引已经超过设备数，则退出循环
        if ((i < 0) || (i >= m_vecDevSn.size()) || (iDevWgtIdx >= MAX_DEV_CNT_OF_DISPLAY))
        {
            break;
        }

        // 获取设备序列号
        const QString& strDevSn = m_vecDevSn.at(i);

        // 获取设备状态控件
        DevMaintainStatusWgt* pDevWgt = m_vDevWgt.at(iDevWgtIdx);

        // 建立映射
        m_mapDevToWgt.insert(strDevSn, pDevWgt);

        // 获取对应的维护详情
        tf::LatestMaintainInfo stuDetailInfo = MaintainDataMng::GetInstance()->GetMaintainDetailByDevSn(strDevSn);

        // 更新维护详情显示到界面
        funcUpdateDetailToUi(pDevWgt, strDevSn, stuDetailInfo);
    }

    // 翻页按钮使能更新
    m_pPrevPageBtn->setEnabled(false);
    m_pNextPageBtn->setEnabled(false);
    if (m_iCurPage > 1)
    {
        m_pPrevPageBtn->setEnabled(true);
    }

    if (m_iCurPage < m_iTotalPage)
    {
        m_pNextPageBtn->setEnabled(true);
    }
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void DevMaintainStatusGrp::InitConnect()
{
    // 连接信号槽
    connect(m_pPrevPageBtn, SIGNAL(clicked()), this, SLOT(OnPrevPageClicked()));

    // 连接信号槽
    connect(m_pNextPageBtn, SIGNAL(clicked()), this, SLOT(OnNextPageClicked()));

    // 监听维护组阶段更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, this, OnMaintainGrpPhaseUpdate);

    // 监听维护组进度
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PROGRESS_UPDATE, this, OnMaintainGrpProgressUpdate);

    // 监听维护组执行时间更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_TIME_UPDATE, this, OnMaintainGrpTimeUpdate);

    // 监听维护项状态更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE, this, OnMaintainItemStatusUpdate);

	// 监听维护显示信息设置更新
	REGISTER_HANDLER(MSG_ID_MAINTAIN_SHOW_SET_UPDATE, this, OnMaintainShowSetUpdate);
}

///
/// @brief
///     重置所有设备状态控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void DevMaintainStatusGrp::ResetAllDevStatusWgt()
{
    // 遍历设备维护状态控件
    for (DevMaintainStatusWgt* pDevWgt : m_vDevWgt)
    {
        // 为空则跳过
        if (pDevWgt == Q_NULLPTR)
        {
            continue;
        }

        // 界面重置
        pDevWgt->Reset();
    }
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void DevMaintainStatusGrp::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化, 显示后初始化
    if (!m_bInit)
    {
        // 显示之后初始化
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     上一页按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月25日，新建函数
///
void DevMaintainStatusGrp::OnPrevPageClicked()
{
    // 向前翻页
    SetCurrentPage(m_iCurPage - 1);
}

///
/// @brief
///     下一页按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月25日，新建函数
///
void DevMaintainStatusGrp::OnNextPageClicked()
{
    // 向后翻页
    SetCurrentPage(m_iCurPage + 1);
}

///
/// @brief
///     维护组阶段更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enPhaseType     阶段类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void DevMaintainStatusGrp::OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType)
{
    // 等待状态由特殊用途
    if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
    {
        return;
    }

    // 获取对应设备序列号的控件
    auto it = m_mapDevToWgt.find(strDevSn);
    if (it == m_mapDevToWgt.end())
    {
        return;
    }

    // 构造状态
    DevMaintainStatusWgt::DevMaintainStatus enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
    if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
    }
    else if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_FAIL)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_FAILED;
    }
    else if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_MAINTAINING;
    }
    else if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_SUCC;
    }
    else
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
    }

    // 更新阶段
	if (enPhaseType != enStatus)
	{
		it.value()->SetStatus(enStatus);
	}

    // 如果维护阶段不是更新为正在维护，则返回，否则进度更新为0（完成的最后再刷新一下，gzq）
   /* if (enStatus != DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_MAINTAINING)
    {
        return;
    }*/

    // 获取维护详情
    tf::LatestMaintainInfo stuMaintainDetail = MaintainDataMng::GetInstance()->GetMaintainDetailByDevSn(strDevSn);

	// 获取进度信息信息
	QString strProgress = "";
	QString strItemName = "";
	QString strItemCount = "";
	GetMaintainProcessByLatestMaintainInfo(stuMaintainDetail, strItemName, strProgress, strItemCount);

    // 更新进度
    it.value()->SetProgress(strProgress, strItemCount, strItemName);
}

///
/// @brief
///     维护组进度更新
///
/// @param[in]  lmi     正在执行的维护组信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void DevMaintainStatusGrp::OnMaintainGrpProgressUpdate(tf::LatestMaintainInfo lmi)
{
    // 根据设备序列号获取控件
    auto it = m_mapDevToWgt.find(QString::fromStdString(lmi.deviceSN));
    if (it == m_mapDevToWgt.end())
    {
        return;
    }

    // 获取维护详情
    tf::LatestMaintainInfo stuMaintainDetail = MaintainDataMng::GetInstance()->GetMaintainDetailByDevSn(QString::fromStdString(lmi.deviceSN));

	// 获取进度信息信息
	QString strProgress = "";
	QString strItemName = "";
	QString strItemCount = "";
	GetMaintainProcessByLatestMaintainInfo(stuMaintainDetail, strItemName, strProgress, strItemCount);

    // 更新进度
    it.value()->SetProgress(strProgress, strItemCount, strItemName);
}

///
/// @brief
///     维护组执行时间更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  strMaintainTime 维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void DevMaintainStatusGrp::OnMaintainGrpTimeUpdate(QString strDevSn, long long lGrpId, QString strMaintainTime)
{
	// 根据设备序列号获取控件
    auto it = m_mapDevToWgt.find(strDevSn);
    if (it == m_mapDevToWgt.end())
    {
        return;
    }

    // 更新时间
    it.value()->SetMaintainTime(strMaintainTime);
}

///
/// @brief
///     维护项状态更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
/// @param[in]  lGrpId			维护组ID
/// @param[in]  enItemType      维护项类型
/// @param[in]  enPhaseType     阶段类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月16日，新建函数
///
void DevMaintainStatusGrp::OnMaintainItemStatusUpdate(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type enItemType, tf::MaintainResult::type enPhaseType)
{
    // 不使用参数
    Q_UNUSED(devModuleIndex);
    Q_UNUSED(enItemType);
    Q_UNUSED(enPhaseType);

    // 获取设备名
    auto it = m_mapDevToWgt.find(strDevSn);
    if (it == m_mapDevToWgt.end())
    {
        return;
    }

    // 获取维护详情
    tf::LatestMaintainInfo stuMaintainDetail = MaintainDataMng::GetInstance()->GetMaintainDetailByDevSn(strDevSn);

    // 等待状态由特殊用途
    tf::MaintainResult::type enGrpPhaseType = stuMaintainDetail.exeResult;

    // 构造状态
    DevMaintainStatusWgt::DevMaintainStatus enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
    if (enGrpPhaseType == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
    }
    else if (enGrpPhaseType == tf::MaintainResult::MAINTAIN_RESULT_FAIL)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_FAILED;
    }
    else if (enGrpPhaseType == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_MAINTAINING;
    }
    else if (enGrpPhaseType == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_SUCC;
    }
    else
    {
        enStatus = DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN;
    }

    // 更新阶段
    it.value()->SetStatus(enStatus);

	// 获取进度信息信息
	QString strProgress = "";
	QString strItemName = "";
	QString strItemCount = "";
	GetMaintainProcessByLatestMaintainInfo(stuMaintainDetail, strItemName, strProgress, strItemCount);

    // 更新进度
    it.value()->SetProgress(strProgress, strItemCount, strItemName);
}

///
/// @brief
///     维护显示信息设置更新
///
/// @param[in]  isShowByItemCnt      是否通过项目数显示（true:项目数，false:百分比）
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月18日，新建函数
///
void DevMaintainStatusGrp::OnMaintainShowSetUpdate(bool isShowByItemCnt)
{
	// 遍历设置模式 
	DevMaintainStatusWgt::DevMaintainInfoShowModel showModel = (isShowByItemCnt ? DevMaintainStatusWgt::MODEL_ITEM_CNT : DevMaintainStatusWgt::MODEL_PERCENT);

	for (DevMaintainStatusWgt* pDevWgt : m_vDevWgt)
	{
		// 为空则跳过
		if (pDevWgt == Q_NULLPTR)
		{
			continue;
		}

		// 界面重置
		pDevWgt->SetMaintainInfoShowModel(showModel);
	}
}

///
/// @brief
///     获取维护显示模式
///
/// @return 维护显示模式
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月18日，新建函数
///
DevMaintainStatusWgt::DevMaintainInfoShowModel DevMaintainStatusGrp::GetDevsMaintianShowModel()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 默认百分数
	DevMaintainStatusWgt::DevMaintainInfoShowModel showModel = DevMaintainStatusWgt::MODEL_PERCENT;

    MaintainShowSet mss;
    if (!DictionaryQueryManager::GetMaintainShowConfig(mss))
    {
        ULOG(LOG_ERROR, "Failed to get maintain show config.");
        return showModel;
    }
	showModel = (mss.bEnableShowInfoProNum ? DevMaintainStatusWgt::MODEL_ITEM_CNT: DevMaintainStatusWgt::MODEL_PERCENT);

	return showModel;
}

///
/// @brief 通过最后一次维护信息获取维护进度
///
/// @param[in]  stuDetailInfo 维护细节 
/// @param[out]  strItemName   当前执行到的维护项名称
/// @param[out]  strProgress   整体维护进度
/// @param[out]  strItemCount  维护计数 
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月22日，新建函数
///
void DevMaintainStatusGrp::GetMaintainProcessByLatestMaintainInfo(const tf::LatestMaintainInfo& stuDetailInfo, QString& strItemName, QString& strProgress, QString& strItemCount)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 清空
	int executingItemIndex = 0;
	strProgress = "";
	strItemName = "";
	strItemCount = "";

	// 参数判断
	if (stuDetailInfo.resultDetail.size() == 0)
	{
		ULOG(LOG_ERROR, " the LatestMaintainInfo of MaintainGroup(%d) is empty.", stuDetailInfo.groupId);
		return;
	}

	// 遍历详情获取信息
	QSet<int> modules;
	int finishedItemCount = 0;
	::tf::MaintainItemExeResult exeingItem;
	for (int itemIndex = 0; itemIndex < stuDetailInfo.resultDetail.size(); itemIndex++)
	{
		auto mItem = stuDetailInfo.resultDetail[itemIndex];

		// 统计模块数量
		if (!modules.contains(mItem.moduleIndex))
		{
			modules.insert(mItem.moduleIndex);
		}

		// 统计完成的项数
		if (mItem.result == ::tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
		{
			finishedItemCount++;
		}

		// 没有设置过正在维护项,则取第一个未知，正在执行中或执行失败的维护项
		if (exeingItem.itemType == tf::MaintainItemType::MAINTAIN_ITEM_INVALID && 
			(
			mItem.result == ::tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN || 
			mItem.result == ::tf::MaintainResult::MAINTAIN_RESULT_EXECUTING || 
			mItem.result == ::tf::MaintainResult::MAINTAIN_RESULT_FAIL
			))
		{
			exeingItem = mItem;
		}
	}

	// 显示名称,多模块同时执行则显示维护组名称，单模块显示正在维护项
	if (modules.size() > 1)
	{
		// 构造维护更新参数
		tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpById(stuDetailInfo.groupId);
		strItemName = QString::fromStdString(stuMtGrp.groupName);
	}
	else
	{
		// 全部完成
		if (finishedItemCount == stuDetailInfo.resultDetail.size())
		{
			// 取最后一项
			exeingItem = stuDetailInfo.resultDetail.back();
		}
		
		// 显示维护项名称
		strItemName = MaintainDataMng::GetInstance()->GetMaintainItemNameByType(exeingItem.itemType);
	}

	// 进度
	strProgress = QString("(%1%)").arg(stuDetailInfo.progress);
	strItemCount = QString("(%1/%2)").arg(finishedItemCount).arg(stuDetailInfo.resultDetail.size());
}