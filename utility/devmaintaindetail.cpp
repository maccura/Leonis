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
/// @file     devmaintaindetail.cpp
/// @brief    设备维护详情对话框
///
/// @author   4170/TangChuXian
/// @date     2023年2月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "devmaintaindetail.h"
#include "ui_devmaintaindetail.h"
#include "maintaindatamng.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "uidcsadapter/uidcsadapter.h"
#include "uidcsadapter/abstractdevice.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include <QHBoxLayout>
#include <QRadioButton>

DevMaintainDetail::DevMaintainDetail(const QString strDevSn, QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_strCurDevSn(strDevSn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化Ui对象
    ui = new Ui::DevMaintainDetail();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

DevMaintainDetail::~DevMaintainDetail()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void DevMaintainDetail::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化信号槽连接
    InitConnect();

    // 更新结果详情信息
    UpdateMtDetailInfo(true);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void DevMaintainDetail::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void DevMaintainDetail::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 取消按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 监听重新维护信号
    connect(ui->MaintainDetailWgt, SIGNAL(SigReMaintain(QString)), this, SLOT(OnReMaintain(QString)));

    // 监听维护项时间更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_TIME_UPDATE, this, OnMaintainItemTimeUpdate);

    // 监听维护项状态更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_PHASE_UPDATE, this, OnMaintainItemStatusUpdate);

    // 监听维护组阶段更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, this, OnMaintainGrpPhaseUpdate);

	// 监听维护组进度
	REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PROGRESS_UPDATE, this, OnMaintainGrpProgressUpdate);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void DevMaintainDetail::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("维护详情"));
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void DevMaintainDetail::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     更新维护详情信息
///
/// @param[in]  bInit   是否是初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月9日，新建函数
///
void DevMaintainDetail::UpdateMtDetailInfo(bool bInit)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初次管理器更新维护详情
	if (bInit)
	{
		MaintainDataMng::GetInstance()->UpdateMaintainDetail(m_strCurDevSn);
	}

    // 添加维护详情到map容器
    QMap<QString, QStringList>                                  mapMaintainItemLst;
    QMap<QString, QVector<tf::MaintainItemExeResult>>           mapItemDetail;
    auto AddDetailItemToMap = [&mapMaintainItemLst, &mapItemDetail](const QString& strCurDevSn, QMap<QString, int>& mapIndexName)
    {
        // 获取当前设备维护详情
        tf::LatestMaintainInfo stuDetailInfo = MaintainDataMng::GetInstance()->GetMaintainDetailByDevSn(strCurDevSn);

        // 如果结果为空，则返回
        if (stuDetailInfo.id <= 0)
        {
            ULOG(LOG_INFO, "%s(), GetMaintainDetailByDevSn() failed", __FUNCTION__);
            return;
        }

		// 获取当前（设备名，组名）
		auto curPairDevName = gUiAdapterPtr()->GetDevNameBySn(strCurDevSn);
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(curPairDevName.first, curPairDevName.second);
        if (spIDev == Q_NULLPTR)
        {
            return;
        }

        // 构造新的显示设备名
        QString strShowDevName = curPairDevName.second + curPairDevName.first;

        // 记录维护项列表,清空模块索引名称映射，用于重新维护
		mapIndexName.clear();
		// 记录该模块是否已经设置为运行
		QMap<QString, bool> mapSetRuning;
        for (auto& stuItemDetail : stuDetailInfo.resultDetail)
        {
            // 如果模块数大于1，设备名特殊显示
            QString strModuleDevName = strShowDevName;
            if ((spIDev->ModuleCount() > 1) && (stuItemDetail.moduleIndex > 0))
            {
                strModuleDevName = strShowDevName + QChar(QChar('A').unicode() + stuItemDetail.moduleIndex - 1);
				if (!mapIndexName.contains(strModuleDevName))
				{
					mapIndexName.insert(strModuleDevName, stuItemDetail.moduleIndex);
				}
            }

            // 获取维护名
            QString strItemName = MaintainDataMng::GetInstance()->GetMaintainItemNameByType(stuItemDetail.itemType);

            // 项目名为空则跳过
            if (strItemName.isEmpty())
            {
                continue;
            }

            // 构造维护项列表
            auto it = mapMaintainItemLst.find(strModuleDevName);
            if (it == mapMaintainItemLst.end())
            {
                QStringList strItemList;
                strItemList.push_back(strItemName);
                mapMaintainItemLst.insert(strModuleDevName, strItemList);
				mapSetRuning.insert(strModuleDevName, false);
            }
            else
            {
                it.value().push_back(strItemName);
            }

			// 设置正在维护项，如果该模块被设置过，则跳过
			if (stuDetailInfo.exeResult == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING && 
				!mapSetRuning[strModuleDevName])
			{
				// 第一个未知设为正在维护
				if (stuItemDetail.result == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
				{
					stuItemDetail.__set_result(tf::MaintainResult::MAINTAIN_RESULT_EXECUTING);
					mapSetRuning[strModuleDevName] = true;
				}
				// 遇到正在执行中或失败则不往后面设置
				else if (stuItemDetail.result == tf::MaintainResult::MAINTAIN_RESULT_FAIL || 
					stuItemDetail.result == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
				{
					mapSetRuning[strModuleDevName] = true;
				}
			}

            // 构造维护单项列表
			if (!mapItemDetail.contains(strModuleDevName))
			{
				mapItemDetail.insert(strModuleDevName, {});
			}
			mapItemDetail[strModuleDevName].push_back(stuItemDetail);
        }
    };

	// 添加维护细节到缓存
	AddDetailItemToMap(m_strCurDevSn, m_mapIndexName);

    // 第一次更新时设置维护项
    if (bInit)
    {
        ui->MaintainDetailWgt->SetMaintainNameList(mapMaintainItemLst);
    }

	// 记录个状态维护项数量
	m_iWaitCnt = 0;
	m_iSuccCnt = 0;
	m_iFailedCnt = 0;
	int itemIndex = 0;
	for (auto it = mapItemDetail.begin(); it != mapItemDetail.end(); it++)
	{
		// 遍历对应模块维护详情列表
		for (const auto& item : it.value())
		{
			// 构造状态
			DevMaintainDetailItemWgt::DevMaintainItemStatus enStatus;
			if (item.result == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
			{
				enStatus = DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_WAIT;
				++m_iWaitCnt;
			}
			else if (item.result == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
			{
				enStatus = DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_EXCUTE;
			}
			else if (item.result == tf::MaintainResult::MAINTAIN_RESULT_FAIL)
			{
				enStatus = DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_FAILED;
				++m_iFailedCnt;
			}
			else if (item.result == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
			{
				enStatus = DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_SUCC;
				++m_iSuccCnt;
			}

			// 更新状态和维护时间
			ui->MaintainDetailWgt->SetMaintainStatus(itemIndex, enStatus);
			ui->MaintainDetailWgt->SetMaintainTime(itemIndex, QString::fromStdString(item.exeTime));

			itemIndex++;
		}
	}
	
    // 设置统计文本
    ui->WaitLab->setText(tr("等待：") + QString::number(m_iWaitCnt));
    ui->SuccLab->setText(tr("成功：") + QString::number(m_iSuccCnt));
    ui->FailedLab->setText(tr("失败：") + QString::number(m_iFailedCnt));
}

///
/// @brief
///     重新维护
///
/// @param[in]  strDevName   设备名
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月21日，新建函数
///
void DevMaintainDetail::OnReMaintain(QString strDevName)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 判断该维护是否是排空
    auto IsDrainMaintain = [](const tf::MaintainGroup& stuMtGrp)
    {
        // 遍历维护项
        for (auto MtItems : stuMtGrp.items)
        {
            // 如果为整机管理排空或在线配液排空，则返回true
            if (MtItems.itemType == tf::MaintainItemType::MAINTAIN_ITEM_DRAIN_PIP || MtItems.itemType == tf::MaintainItemType::MAINTAIN_ITEM_CONFECT_LIQUID_DRAIN)
            {
                return true;
            }
        }

        return false;
    };

	// 维护组变更判断
	auto MaintainGroupIsChanged = [](tf::DeviceType::type devType, const tf::MaintainGroup& stuMtGrp, const tf::LatestMaintainInfo& stuLastInfo)->bool {
		
		std::vector<tf::MaintainItemType::type> maintainItemGroup;
		// 过滤不支持的维护项
		for (auto& groupItem : stuMtGrp.items)
		{
			auto stuItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(groupItem.itemType);
			auto typeIt = std::find(stuItem.deviceTypes.begin(), stuItem.deviceTypes.end(), devType);
			if (typeIt != stuItem.deviceTypes.end())
			{
				maintainItemGroup.push_back(groupItem.itemType);
			}
		}

		std::vector<tf::MaintainItemType::type> maintainItemLast;
		// 过滤多模块，只留一个模块
		int checkIndex = -1;
		for (auto& lastItem : stuLastInfo.resultDetail)
		{
			// 设置检查模块索引
			if (checkIndex == -1)
			{
				checkIndex = lastItem.moduleIndex;
			}

			if (lastItem.moduleIndex != checkIndex)
			{
				continue;
			}

			maintainItemLast.push_back(lastItem.itemType);
		}

		// 检查两个容器是否相等
		return maintainItemGroup != maintainItemLast;
	};

	// 设备序列号为空则返回
	if (m_strCurDevSn.isEmpty())
	{
		ULOG(LOG_ERROR, "m_strCurDevSn.isEmpty().");
		return;
	}

    // 构造执行维护的设备信息
    ::tf::DevicekeyInfo stuTfDevKeyInfo;
    stuTfDevKeyInfo.__set_sn(m_strCurDevSn.toStdString());

	// 设置模块索引
	if (m_mapIndexName.contains(strDevName))
	{
		stuTfDevKeyInfo.__set_modelIndex(m_mapIndexName[strDevName]);
	}
	
	// 设备状态判断
	const auto& devStatus = CommonInformationManager::GetInstance()->GetDevStateByKeyInfo(stuTfDevKeyInfo);
	if (devStatus == tf::DeviceWorkState::DEVICE_STATUS_RUNNING || 
		devStatus == tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
	{
		TipDlg(tr("仪器处于运行或维护状态，不能进行重新维护。")).exec();
		return;
	}

    // 获取所有维护详情
    tf::LatestMaintainInfo stuDetailInfo = MaintainDataMng::GetInstance()->GetMaintainDetailByDevSn(m_strCurDevSn);

	// 获取设备
	auto spStuTfDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strCurDevSn.toStdString());
	if (spStuTfDevInfo == Q_NULLPTR)
	{
		ULOG(LOG_ERROR, "GetDeviceInfo(%s) Failed.", m_strCurDevSn.toStdString());
		return;
	}

	// 获取维护组
	auto reGroup = MaintainDataMng::GetInstance()->GetMaintainGrpById(stuDetailInfo.groupId);
	// 维护组变更判断
	if (MaintainGroupIsChanged(spStuTfDevInfo->deviceType, reGroup, stuDetailInfo))
	{
		TipDlg(tr("维护组对应此设备的内容已经变更，不能进行重新维护。")).exec();
		return;
	}

    // 准备启动重新维护
    MaintainDataMng::GetInstance()->PrepareStartRemaintain();

    // 如果是免疫排空，则需弹框提示
    // 免疫执行排空弹框提示
    if (spStuTfDevInfo->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE && IsDrainMaintain(reGroup))
    {
        QString noticeInfo = tr("执行此维护后，无论成功与否，必须清空纯水桶、缓存桶、配液桶中残余液体；若需使用仪器，需重新开关电源后，重启上位机软件。");;
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), noticeInfo, TipDlgType::SINGLE_BUTTON));
        pTipDlg->exec();
    }
	
	// 组装维护参数
	tf::MaintainExeParams exeParams;

	// 设置单项维护参数，组合维护用默认参数
	if (reGroup.groupType == tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE &&
		reGroup.items.size() == 1)
	{
		// 执行查询
		tf::MaintainGroupQueryCond grpQryCond;
		grpQryCond.__set_ids({ reGroup.id });
		tf::MaintainGroupQueryResp grpQryResp;
		if (!DcsControlProxy::GetInstance()->QueryMaintainGroup(grpQryResp, grpQryCond) || 
			(grpQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS) ||
			(grpQryResp.lstMaintainGroups.size() != 1) ||
			(grpQryResp.lstMaintainGroups[0].groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
			)
		{
			ULOG(LOG_ERROR, "%s(), QueryMaintainGroup() failed", __FUNCTION__);
			return;
		}

		auto exeItem = grpQryResp.lstMaintainGroups[0].items[0];
		if (!exeItem.param.empty())
		{
			exeParams.__set_itemParams({ { exeItem.itemType, exeItem.param } });
		}
	}

	exeParams.__set_groupId(stuDetailInfo.groupId);
	exeParams.__set_lstDev({ stuTfDevKeyInfo });
	exeParams.__set_mode(::tf::MaintainExeMode::INDIRECT_REMAINTENANCE);
    if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
    {
        ULOG(LOG_ERROR, "%s(), Maintain() failed", __FUNCTION__);
        return;
    }
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
void DevMaintainDetail::OnMaintainItemStatusUpdate(QString strDevSn, int devModuleIndex, long long lGrpId, tf::MaintainItemType::type enItemType, tf::MaintainResult::type enPhaseType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (m_strCurDevSn == strDevSn)
	{
		UpdateMtDetailInfo();
	}
}

///
/// @brief
///     维护项时间更新
///
/// @param[in]  strDevSn        设备序列号
/// @param[in]  lGrpId          维护组ID
/// @param[in]  enItemType      维护项类型
/// @param[in]  strTime         时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月16日，新建函数
///
void DevMaintainDetail::OnMaintainItemTimeUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemType::type enItemType, QString strTime)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (m_strCurDevSn == strDevSn)
	{
		UpdateMtDetailInfo();
	}
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
/// @li 4170/TangChuXian，2023年10月20日，新建函数
///
void DevMaintainDetail::OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 如果维护组开始执行，重新加载维护子项(失败也重新刷一下)
    if (m_strCurDevSn == strDevSn)
    {
        // 维护组开始执行需要重新初始化
        bool bInit = (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING);
        UpdateMtDetailInfo(bInit);
    }
}

///
/// @brief
///     维护组阶段更新
///
/// @param[in]  lmi     正在执行的维护组信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月24日，新建函数
///
void DevMaintainDetail::OnMaintainGrpProgressUpdate(tf::LatestMaintainInfo lmi)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (m_strCurDevSn == QString::fromStdString(lmi.deviceSN)
		&& lmi.progress == 0)
	{
		UpdateMtDetailInfo(true);
	}
}
