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
/// @file     maintainopdlg.cpp
/// @brief    维护执行对话框
///
/// @author   4170/TangChuXian
/// @date     2021年6月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "maintainopdlg.h"
#include "ui_maintainopdlg.h"
#include <algorithm>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextLayout>
#include <QLineEdit>
#include "maintaingrpdetail.h"
#include "maintaindatamng.h"
#include "manager/UserInfoManager.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "uidcsadapter/uidcsadapter.h"
#include "uidcsadapter/abstractdevice.h"
#include "thrift/DcsControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "src/public/ConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ise/IseConfigSerialize.h"
#include "QCupBlankTestResultDlg.h"
#include "QPhotoCheckResultDlg.h"

#define DEFAULT_ROW_CNT_OF_DEFAULT_QC_TABLE             (20)                    // 默认质控表默认行数
#define DEFAULT_COL_CNT_OF_DEFAULT_QC_TABLE             (11)                    // 默认质控表默认列数

#define DEV_BTN_STATUS_PROPERTY_NAME                    ("status")              // 设备按钮状态属性名

#define DEV_BTN_STATUS_UNMAINTAIN                       ("unmaintain")          // 未维护
#define DEV_BTN_STATUS_MAINTAINING                      ("maintaining")         // 正在维护
#define DEV_BTN_STATUS_DISABLED							("maintain_disabled")	// 失能不可选

#define OPTION_BOTTOM_MARGIN_HIGHT						(104)					// 操作按钮距离对话框底部高度
#define CH_UNIT_CUP_COUNT								(13)					// 生化每一联杯子数

MaintainOpDlg::MaintainOpDlg(const QString strMaintainName, QWidget *parent)
    : BaseDlg(parent),
      m_strMaintainName(strMaintainName),
      m_bInit(false),
	  m_DevBtnsExclusiveFlag(false)
{
    // 初始化Ui对象
    ui = new Ui::MaintainOpDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();

	// 限制输入(没有输入限制)
	//SetCtrlsRegExp();
}

MaintainOpDlg::~MaintainOpDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void MaintainOpDlg::InitBeforeShow()
{
    // 获取所有设备
    if (!gUiAdapterPtr()->GetGroupDevNameListWithTrack(m_strDevNameList, true))
    {
        return;
    }

    // 根据设备列表创建单选框
    QHBoxLayout* pHlayout = new QHBoxLayout(ui->DevFrame);
    pHlayout->setMargin(0);
    pHlayout->setSpacing(8);

    // 通过设备名创建按钮并添加到布局中
    auto funcCreateDevBtnToLayout = [this](const QString& strDevName, QBoxLayout* pLayout, int iFixHeight,
                                           const tf::DevicekeyInfo& stuTfDevInfo)
    {
        QPushButton* pDevBtn = new QPushButton(strDevName);
        pDevBtn->setCheckable(true);
        pDevBtn->setFocusPolicy(Qt::NoFocus);
        pDevBtn->setFixedHeight(iFixHeight);
		SwitchDevBtnStatus(pDevBtn, DEV_BTN_STATUS_UNMAINTAIN);
        pLayout->addWidget(pDevBtn, 1);
        m_pDevBtnList.push_back(pDevBtn);
        m_mapBtnDevInfo.insert(pDevBtn, stuTfDevInfo);
        connect(pDevBtn, SIGNAL(clicked()), this, SLOT(OnDevBtnClicked()));
    };

    // 依次添加具体设备
    for (const auto& strDevName : m_strDevNameList)
    {
		// 轨道
		if (gUiAdapterPtr()->GetDeviceType(strDevName) == tf::DeviceType::DEVICE_TYPE_TRACK)
		{
			// 构造关键设备信息
			tf::DevicekeyInfo stuTfDevInfo;
			stuTfDevInfo.__set_sn(gUiAdapterPtr()->GetDevSnByName(strDevName).toStdString());
			// 轨道横向布局
			QHBoxLayout* pTrackHlayout = new QHBoxLayout(ui->TrackFrame);
			pTrackHlayout->setMargin(0);
			pTrackHlayout->setSpacing(8);
			funcCreateDevBtnToLayout(tr("轨道"), pTrackHlayout, ui->TrackFrame->height(), stuTfDevInfo);
			continue;
		}

        // 判断是否为设备组
        if (!gUiAdapterPtr()->WhetherContainOtherSubDev(strDevName) &&
            gUiAdapterPtr()->GetDeviceClasssify(strDevName) != DEVICE_CLASSIFY_ISE)
        {
            // 构造关键设备信息
            tf::DevicekeyInfo stuTfDevInfo;
            stuTfDevInfo.__set_sn(gUiAdapterPtr()->GetDevSnByName(strDevName).toStdString());

            // 不为设备组，不为ISE，直接水平添加设备按钮
            funcCreateDevBtnToLayout(strDevName, pHlayout, ui->DevFrame->height(), stuTfDevInfo);
            continue;
        }

        // 判断是否为设备组
        if (!gUiAdapterPtr()->WhetherContainOtherSubDev(strDevName) &&
            gUiAdapterPtr()->GetDeviceClasssify(strDevName) == DEVICE_CLASSIFY_ISE)
        {
            // 不为设备组，为ISE，直接水平添加设备按钮
            // 获取对应设备
            std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName);
            if (spIDev == Q_NULLPTR)
            {
                continue;
            }

            // 构造关键设备信息
            tf::DevicekeyInfo stuTfDevInfo;
            stuTfDevInfo.__set_sn(gUiAdapterPtr()->GetDevSnByName(strDevName).toStdString());

            // 如果模块数小于等于1
            if (spIDev->ModuleCount() <= 1)
            {
                funcCreateDevBtnToLayout(strDevName, pHlayout, ui->DevFrame->height(), stuTfDevInfo);
                continue;
            }

            // 模块数大于1，则每个模块并列一列
            QVBoxLayout* pVlayout = new QVBoxLayout();
            pVlayout->setMargin(0);
            pVlayout->setSpacing(8);
            pHlayout->addLayout(pVlayout, 1);
            for (int i = 0; i < spIDev->ModuleCount(); i++)
            {
                QString strNewSubDevName = strDevName + QChar(QChar('A').unicode() + i);
                stuTfDevInfo.__set_modelIndex(i + 1);
                funcCreateDevBtnToLayout(strNewSubDevName, pVlayout, (ui->DevFrame->height() + 8) / spIDev->ModuleCount() - 8, stuTfDevInfo);
            }
            continue;
        }

        // 获取子设备
        QStringList strSubDevList;
        if (!gUiAdapterPtr()->GetSubDevNameList(strDevName, strSubDevList) || strSubDevList.isEmpty())
        {
            continue;
        }

        // 根据子设备列表创建单选框
        QVBoxLayout* pVlayout = new QVBoxLayout();
        pVlayout->setMargin(0);
        pVlayout->setSpacing(8);
        pHlayout->addLayout(pVlayout, 1);
        for (const QString& strSubDevName : strSubDevList)
        {
            // 构造关键设备信息
            tf::DevicekeyInfo stuTfDevInfo;
            stuTfDevInfo.__set_sn(gUiAdapterPtr()->GetDevSnByName(strSubDevName, strDevName).toStdString());

            QString strNewSubDevName = strDevName + strSubDevName;
            funcCreateDevBtnToLayout(strNewSubDevName, pVlayout, (ui->DevFrame->height() + 8) / strSubDevList.size() - 8, stuTfDevInfo);
        }
    }

	// 根据单机还是联机调整界面
	AdjustUIBySingleOrPipeLine();

	// 根据维护名称初始化界面
	UpdateUIByMaintainName();

	// 调整设备按钮
	AdjustDevBtnByMaintainItem();
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void MaintainOpDlg::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void MaintainOpDlg::InitStrResource()
{

}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void MaintainOpDlg::InitConnect()
{
    // 状态表选择列被点击
    connect(ui->ExcuteBtn, SIGNAL(clicked()), this, SLOT(OnExcuteBtnClicked()));

	// 结束确认按钮
	connect(ui->EndComfirmBtn, SIGNAL(clicked()), this, SLOT(OnEndComfirmBtnClicked()));
	connect(ui->btn_IPC_PlaceOver, SIGNAL(clicked()), this, SLOT(OnEndComfirmBtnClicked()));

    // 取消按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

	// 杯空白测定结果被点击
	connect(ui->cuvette_blank_btn, SIGNAL(clicked()), this, SLOT(OnCupBlankTestResultClicked()));

	// 光度计检查结果被点击
	connect(ui->photometer_check_btn, SIGNAL(clicked()), this, SLOT(OnPhotoCheckResultClicked()));

    // 监听维护组阶段更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, this, OnMaintainGrpPhaseUpdate);

	// 监听维护项阶段更新
	REGISTER_HANDLER(MSG_ID_MAINTAIN_ITEM_STAGE_UPDATE, this, OnMaintainItemStageUpdate);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void MaintainOpDlg::InitChildCtrl()
{
    // 设置标题
    SetTitleName(m_strMaintainName);

    // 更新设备维护组状态
    UpdateDevMaintainGrpStatus();

    // 如果为待机状态，则选中则选中
    SelectUnmaintainBtn();

    // 更新维护组按钮状态
    UpdateMtGrpBtnStatus();
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void MaintainOpDlg::showEvent(QShowEvent *event)
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
/// @brief 获取被选中的设备信息
///		如果同一个sn的多个模块同时被选中，说明是整机，则modelIndex为0
///
/// @param[in]  checkedDev  被选中的设备信息
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年9月9日，新建函数
///
void MaintainOpDlg::TakeCheckedDevkeyInfo(std::vector<::tf::DevicekeyInfo>& checkedDev)
{
    for (QPushButton* pBtn : m_pDevBtnList)
    {
        if (!pBtn->isChecked())
        {
            continue;
        }

        // 获取对应设备
        auto it = m_mapBtnDevInfo.find(pBtn);
        if (it == m_mapBtnDevInfo.end())
        {
            continue;
        }

        // 加入设备信息
		auto devKeyInfo = it.value();

		// 查找是否已经设置了该设备但是为不同模块
		auto& iterDev = std::find_if(checkedDev.begin(), checkedDev.end(), [&devKeyInfo](auto& iter)
		{
			return (devKeyInfo.sn == iter.sn && devKeyInfo.modelIndex != iter.modelIndex);
		});

		// 存在SN相同，并且模块不同的，则设置为整机（模块索引设置为0）
		if (iterDev != checkedDev.end())
		{
			iterDev->__set_modelIndex(0);
		}
		else
		{
			checkedDev.push_back(devKeyInfo);
		}
    }
}

///
/// @brief
///     执行按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void MaintainOpDlg::OnExcuteBtnClicked()
{
	ULOG(LOG_INFO, "%s()::%s", __FUNCTION__, m_strMaintainName.toStdString());

    // 获取勾选的按钮列表和设备SN
    std::vector<::tf::DevicekeyInfo> devKeyInfoList;
	TakeCheckedDevkeyInfo(devKeyInfoList);

	// 判空
	if (devKeyInfoList.empty())
	{
		TipDlg(tr("提示"), tr("未选择设备！")).exec();
		return;
	}
	
	// 获取维护组
	tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpByName(m_strMaintainName);

	// 检查
	std::vector<::tf::DevicekeyInfo> excDevice;
	QSet<QString> noticeSet;
	for (const auto& dev : devKeyInfoList)
	{
		QString noticeInfo;
		auto checkFlag = MaintainDataMng::GetInstance()->CheckDevIsReadyForMaintain(noticeInfo, dev, stuMtGrp);

		// 有提示则压入
		if (!noticeInfo.isEmpty())
		{
			// 自动去重
			noticeSet.insert(noticeInfo);
		}

		// 没有检查通过，不执行该设备
		if (!checkFlag)
		{
			continue;
		}

		excDevice.push_back(dev);
	}

	// 遍历提示(最大显示18行，由于每个仪器只会有一条，所以不会超出显示)
	QString showInfo;
	for (auto notice : noticeSet)
	{
		showInfo += (notice + "\n");
	}

	// 不为空则提示
	if (!showInfo.isEmpty())
	{
		TipDlg(tr("提示"), showInfo).exec();
	}

	// 没有需要执行维护的设备
	if (excDevice.empty())
	{
		return;
	}

	// 维护执行参数
	tf::MaintainExeParams exeParams;
	// 设置参数
	if (!SetMaintainSingleParamFromUi(stuMtGrp, exeParams))
	{
		ULOG(LOG_ERROR, "SetMaintainSingleParamFromUi Failed!");
		return;
	}
	// 设置维护组id
	exeParams.__set_groupId(stuMtGrp.id);
	// 设置执行设备
	exeParams.__set_lstDev(excDevice);
    if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
    {
        ULOG(LOG_ERROR, "%s(), Maintain() failed", __FUNCTION__);
        return;
    }

	// 初始执行后显示界面
	InitMaintainExecStatus(stuMtGrp);
}

///
/// @brief 执行后初始化显示（半自动维护）
///
/// @param[in]  currentMaintainGroup  当前维护组
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年6月19日，新建函数
///
void MaintainOpDlg::InitMaintainExecStatus(const tf::MaintainGroup& currentMaintainGroup)
{
	// 判断是否是单项维护,组合维护直接退出
	if (currentMaintainGroup.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE ||
		currentMaintainGroup.items.size() != 1)
	{
		this->accept();
		return;
	}

	// 获取单项维护（多阶段的）
	auto maintainItem = currentMaintainGroup.items[0];
	switch (maintainItem.itemType)
	{
	// 反应杯更换
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP:
	{

		
	}break;

	// 排空供水桶
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_DRAIN_BUCKET:
	{
		ui->label_DB_notice->setText(tr("供水桶排水中。"));
	}break;

	// 清洗反应槽
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_WASH_REACTION_PARTS:
	{
		ui->label_RTC_notice->setText(tr("反应槽排水中。"));
	}break;

	// 电极更换
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE:
	{

	}break;

	// ISE管路清洗
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CLEAN_FLUID_PATH:
	{

	}break;

	default:
	{
		// 没有多阶段的单项维护，直接退出
		this->accept();
	}break;
	}

}

///
/// @brief 半自动维护结束确认
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月12日，新建函数
///
void MaintainOpDlg::OnEndComfirmBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 发送下一阶段维护
	auto SendNextStageMaintain = [&]()
	{
		// 发送下一阶段维护
		if (!NextStageMaintain())
		{
			return;
		}

		// 按钮失能
		ui->EndComfirmBtn->setEnabled(false);
	};

	// 根据当前维护类型进行处理
	tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpByName(m_strMaintainName);
	if (stuMtGrp.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE ||
		stuMtGrp.items.size() != 1)
	{
		return;
	}

	// 获取单项维护
	auto maintainItem = stuMtGrp.items[0];
	switch (maintainItem.itemType)
	{
	// 排空供水桶, 清洗反应槽
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_DRAIN_BUCKET:
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_WASH_REACTION_PARTS:
	{
		this->accept();
	}break;

	// 反应杯更换
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP:
	{
		// 获取勾选记录
		std::vector<int> unitNum = GetCuvetteReplaceCupUnits();
		if (!unitNum.empty())
		{
			//获取选中设备
			std::vector<::tf::DevicekeyInfo> SnAndModelIndexs;
			TakeCheckedDevkeyInfo(SnAndModelIndexs);

			// 有且只有一个
			if (SnAndModelIndexs.size() == 1)
			{
				// 构建需要清空的杯子
				std::vector<int> cupNums;
				for (const auto& unit : unitNum)
				{
					for (int cup = 1; cup <= CH_UNIT_CUP_COUNT; cup++)
					{
						cupNums.push_back(cup + (unit - 1) * CH_UNIT_CUP_COUNT);
					}
				}

				// 清空计数
				if (!ch::LogicControlProxy::ResetReactionCupHistoryInfo(SnAndModelIndexs.front().sn, cupNums))
				{
					ULOG(LOG_ERROR, "%s(), Failed to execute ResetReactionCupHistoryInfo", __FUNCTION__);
				}
			}
		}
		this->accept();
	}break;

	// ISE管路清洗
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CLEAN_FLUID_PATH:
	{
		SendNextStageMaintain();
		auto spBtn = qobject_cast<QPushButton*>(sender());
		if (spBtn == ui->btn_IPC_PlaceOver)
		{
			ui->label_IPC_notice->setText(tr("管路清洗中。"));
			spBtn->setVisible(false);
		}
		else if (spBtn == ui->EndComfirmBtn)
		{
			this->accept();
		}
	} break;

	// 电极更换
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE:
	{
		SendNextStageMaintain();
		this->accept();
	} break;

	default:
		this->accept();
		break;
	}
}

///
/// @brief
///     更新维护组按钮显示状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月23日，新建函数
///
void MaintainOpDlg::UpdateMtGrpBtnStatus()
{
    // 更新维护组按钮状态，默认为未维护
    bool bMtGrpMaintaining = false;

    // 选中按钮列表
    QList<QPushButton*> pCheckedBtnList;
    for (auto pBtn : m_pDevBtnList)
    {
        // 参数检查
        if (pBtn == Q_NULLPTR)
        {
            continue;
        }

        // 如果存在按钮状态为正在维护，则维护组按钮也为正在维护
        if (pBtn->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() == DEV_BTN_STATUS_MAINTAINING)
        {
            bMtGrpMaintaining = true;
        }

        // 未选中则跳过
        if (!pBtn->isChecked())
        {
            continue;
        }

        pCheckedBtnList.push_back(pBtn);
    }

    // 如果选中按钮为空，则开始按钮和停止按钮都禁能
    // 更新开始按钮和停止按钮使能状态，默认都使能
    bool bStartBtnEnable = true;
    bool bStopBtnEnable = true;
    if (pCheckedBtnList.isEmpty())
    {
        bStartBtnEnable = false;
        bStopBtnEnable = false;
    }
    else
    {
        for (auto pBtn : pCheckedBtnList)
        {
            // 参数检查
            if (pBtn == Q_NULLPTR)
            {
                continue;
            }

            // 如果选中按钮状态为正在维护，则开始按钮禁能
            if (pBtn->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() == DEV_BTN_STATUS_MAINTAINING)
            {
                bStartBtnEnable = false;
            }

            // 如果选中按钮状态为未维护，则停止按钮禁能
            if (pBtn->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() == DEV_BTN_STATUS_UNMAINTAIN)
            {
                bStopBtnEnable = false;
            }
        }
    }

    // 更新按钮使能状态
    ui->ExcuteBtn->setEnabled(bStartBtnEnable);

    // 更新设备按钮状态
    for (auto pBtn : m_pDevBtnList)
    {
        pBtn->style()->unpolish(pBtn);
        pBtn->style()->polish(pBtn);
        pBtn->update();
    }
}

///
/// @brief
///     设备按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月23日，新建函数
///
void MaintainOpDlg::OnDevBtnClicked()
{
	// 按钮互斥
	QObject* senderObject = sender();
	if (senderObject != nullptr)
	{
		QPushButton* devBtn = qobject_cast<QPushButton*>(senderObject);

		if (devBtn != nullptr   &&
			devBtn->isChecked() &&
			m_pDevBtnList.contains(devBtn))
		{
			for (auto pBtn : m_pDevBtnList)
			{
				// 互斥标识
				if (m_DevBtnsExclusiveFlag)
				{
					if (devBtn == pBtn) continue;
					pBtn->setChecked(false);
					continue;
				}
				
				//// 多模块互斥
				//if (m_mapBtnDevInfo.contains(devBtn) && m_mapBtnDevInfo.contains(pBtn) &&
				//	m_mapBtnDevInfo[devBtn].sn == m_mapBtnDevInfo[pBtn].sn &&
				//	m_mapBtnDevInfo[devBtn].modelIndex != m_mapBtnDevInfo[pBtn].modelIndex)
				//{
				//	pBtn->setChecked(false);
				//}
			}
		}
	}

    // 更新维护组按钮状态
    UpdateMtGrpBtnStatus();
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
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void MaintainOpDlg::OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType)
{
    Q_UNUSED(lGrpId);
    // 等待状态由特殊用途
    if (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_UNKNOWN)
    {
        return;
    }

    // 获取设备名
    QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(strDevSn);
    if (strDevNamePair.first.isEmpty())
    {
        return;
    }

    // 将设备按钮状态置为正在维护
    auto funcSetDevBtnMaintaining = [this](const QString& strDevName, bool bMaintaining)
    {
        // 根据设备名找到对应设备按钮
        auto itDevBtn = std::find_if(m_pDevBtnList.begin(), m_pDevBtnList.end(),
            [&strDevName](QPushButton* pDevBtn) { return (pDevBtn->text() == strDevName); });
        if (itDevBtn == m_pDevBtnList.end())
        {
            return;
        }

        // 如果按钮状态已为正在维护，则跳过
        if (bMaintaining)
        {
            // 将按钮状态设置为正在维护
			SwitchDevBtnStatus((*itDevBtn), DEV_BTN_STATUS_MAINTAINING);
        }
        else
        {
            // 将按钮状态设置为未在维护
			SwitchDevBtnStatus((*itDevBtn), DEV_BTN_STATUS_UNMAINTAIN);
        }
    };

    // 判断是否有多个模块
    // 获取对应设备
    std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevNamePair.first, strDevNamePair.second);
    if (spIDev == Q_NULLPTR)
    {
        return;
    }

    // 如果模块数小于等于1
    if (spIDev->ModuleCount() <= 1)
    {
        // 获取设备名
        QString strDevName = strDevNamePair.second.isEmpty() ? strDevNamePair.first : (strDevNamePair.second + strDevNamePair.first);

        // 轨道
        if (gUiAdapterPtr()->GetDeviceType(strDevName) == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            strDevName = tr("轨道");
        }

        // 将按钮状态置为正在维护
        funcSetDevBtnMaintaining(strDevName, (enPhaseType == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING));
    }
	else
	{
		// 遍历所有模块
		for (int i = 0; i < spIDev->ModuleCount(); i++)
		{
			// 获取设备模块名
			QString strDevName = strDevNamePair.first + QChar(QChar('A').unicode() + i);

			// 判断模块是否正在维护
			bool bMaintaining = MaintainDataMng::GetInstance()->IsDevModuleMaintaining(strDevSn, i + 1);

			// 将按钮状态更新
			funcSetDevBtnMaintaining(strDevName, bMaintaining);
		}
	}

    // 更新维护组按钮状态
    UpdateMtGrpBtnStatus();
}

///
/// @brief 维护项阶段更新
///
/// @param[in]  strDevSn  设备序列号
/// @param[in]  lGrpId  维护组ID
/// @param[in]  mier  维护结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月12日，新建函数
///
void MaintainOpDlg::OnMaintainItemStageUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemExeResult mier)
{
	ULOG(LOG_INFO, "Sn:%s, GroupId:%ld, ", strDevSn, lGrpId);

	// 不是当前维护组不更新
	tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpByName(m_strMaintainName);
	if (stuMtGrp.id != lGrpId)
	{
		return;
	}

	// 设置提示信息
	UpdateNoticeInfo(strDevSn, mier);
}

///
/// @brief
///     更新设备维护组状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void MaintainOpDlg::UpdateDevMaintainGrpStatus()
{
    // 更新设备按钮状态
    for (auto pBtn : m_pDevBtnList)
    {
		SwitchDevBtnStatus(pBtn, DEV_BTN_STATUS_UNMAINTAIN);
    }

    // 获取所有维护详情
    QList<tf::LatestMaintainInfo> detailLst;
    if (!MaintainDataMng::GetInstance()->GetAllMaintainDetail(detailLst))
    {
        ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetAllMaintainDetail failed.", __FUNCTION__);
        return;
    }

    // 记录设备维护状态
    QMap<QString, tf::LatestMaintainInfo> mapDevMtStatus;
    for (const auto& stuLatestMtInfo : detailLst)
    {
        // 获取设备名
        QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(QString::fromStdString(stuLatestMtInfo.deviceSN));
        if (strDevNamePair.first.isEmpty())
        {
            return;
        }

        // 构造设备名
        QString strDevName = strDevNamePair.second.isEmpty() ? strDevNamePair.first : (strDevNamePair.second + strDevNamePair.first);

        // 查找设备对应的最近维护信息
        auto it = mapDevMtStatus.find(strDevName);
        if (it == mapDevMtStatus.end())
        {
            mapDevMtStatus.insert(strDevName, stuLatestMtInfo);
            continue;
        }

        // 如果设备维护信息已存在，则使用最新的
        if (QString::fromStdString(it.value().exeTime) > QString::fromStdString(stuLatestMtInfo.exeTime))
        {
            continue;
        }

        // 使用最新的
        mapDevMtStatus.insert(strDevName, stuLatestMtInfo);
    }

    // 将设备按钮状态置为正在维护
    auto funcSetDevBtnMaintaining = [this](const QString& strDevName)
    {
        // 根据设备名找到对应设备按钮
        auto itDevBtn = std::find_if(m_pDevBtnList.begin(), m_pDevBtnList.end(),
            [&strDevName](QPushButton* pDevBtn) { return (pDevBtn->text() == strDevName); });
        if (itDevBtn == m_pDevBtnList.end())
        {
            return;
        }

        // 如果按钮状态已为正在维护，则跳过
        if ((*itDevBtn)->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() == DEV_BTN_STATUS_MAINTAINING)
        {
            return;
        }

        // 将按钮状态设置为正在维护
		SwitchDevBtnStatus(*itDevBtn, DEV_BTN_STATUS_MAINTAINING);
    };

    // 遍历设备最近维护记录，更新设备维护状态
    for (auto itMap = mapDevMtStatus.begin(); itMap != mapDevMtStatus.end(); itMap++)
    {
        // 如果维护状态不为正在维护，则跳过
        if (itMap.value().exeResult != tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
        {
            continue;
        }

        // 如果维护组不匹配，则跳过
//         tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpById(itMap.value().groupId);
//         if (m_strMaintainName != QString::fromStdString(stuMtGrp.groupName))
//         {
//             continue;
//         }

        // 获取设备名
        QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(QString::fromStdString(itMap.value().deviceSN));
        if (strDevNamePair.first.isEmpty())
        {
            continue;
        }

		// 有设备正在维护，需要判断是否设置参数
		InitParamByLatestMaintain(itMap.value());

        // 判断是否有多个模块
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevNamePair.first, strDevNamePair.second);
        if (spIDev == Q_NULLPTR)
        {
            continue;
        }

        // 如果模块数小于等于1
        if (spIDev->ModuleCount() <= 1)
        {
            // 获取设备名
            QString strDevName = strDevNamePair.second.isEmpty() ? strDevNamePair.first : (strDevNamePair.second + strDevNamePair.first);

            // 轨道
            if (gUiAdapterPtr()->GetDeviceType(strDevName) == tf::DeviceType::DEVICE_TYPE_TRACK)
            {
                strDevName = tr("轨道");
            }

            // 将按钮状态置为正在维护
            funcSetDevBtnMaintaining(strDevName);
        }
		else
		{
			// 遍历所有模块
			for (int i = 0; i < spIDev->ModuleCount(); i++)
			{
				// 判断模块是否正在维护
				if (MaintainDataMng::GetInstance()->IsDevModuleMaintaining(QString::fromStdString(itMap.value().deviceSN), i + 1))
				{
					// 获取设备模块名
					QString strDevName = strDevNamePair.first + QChar(QChar('A').unicode() + i);

					// 将按钮状态置为正在维护
					funcSetDevBtnMaintaining(strDevName);
				}
			}
		}
    }
}

///
/// @brief
///     杯空白测定结果被点击
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void MaintainOpDlg::OnCupBlankTestResultClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 弹出杯空白测定结果对话框
	std::shared_ptr<QCupBlankTestResultDlg> spQCupBlankTestResultDlg(new QCupBlankTestResultDlg(this));
	spQCupBlankTestResultDlg->exec();

}

///
/// @brief
///     光度计检查结果被点击
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
///
void MaintainOpDlg::OnPhotoCheckResultClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 弹出光度计检查结果对话框
	std::shared_ptr<QPhotoCheckResultDlg> spQPhotoCheckResultDlg(new QPhotoCheckResultDlg(this));
	spQPhotoCheckResultDlg->exec();

}

///
/// @brief
///     根据维护名称调整界面
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月20日，新建函数
///
void MaintainOpDlg::UpdateUIByMaintainName()
{
	// 默认不互斥，半自动维护项互斥
	m_DevBtnsExclusiveFlag = false;

	// 结束确认按钮默认不可见
	ui->EndComfirmBtn->setDisabled(true);
	ui->EndComfirmBtn->setVisible(false);

	tf::MaintainItemType::type currentItemType = MaintainDataMng::GetInstance()->GetMaintainItemByName(m_strMaintainName).itemType;

	switch (currentItemType)
	{
	// 比色管路填充
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_ChPipeFill);
		VerticalMoveOptionBtnFrame(true, 50);
	}break;

	// 针清洗
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_ProbeClean);
		//VerticalMoveOptionBtnFrame(true, 50);
		// 针清洗不选择洗哪种针，下位机自行判断(2024.8.2)
		ui->sample_probe_ckbox->setVisible(false);
		ui->reagent_probe_ckbox->setVisible(false);
	}break;

	// 反应槽水更换
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_INCUBATION_WATER_EXCHANGE:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_ReacTankWaterReplace);
		VerticalMoveOptionBtnFrame(true, 50);
	}break;

	// 反应杯更换
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP:
	{
		m_DevBtnsExclusiveFlag = true;
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_CuvetteReplace);
		VerticalMoveOptionBtnFrame(true, 140);

		// 初始化
		ui->label_CR_notice->clear();
		ui->EndComfirmBtn->setDisabled(true);
		ui->EndComfirmBtn->setVisible(true);
	}break;

	// 杯空白测定
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_WATER_BLANK:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_CuvetteBlank);
	}break;

	// 光度计检查
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_PHOTOMETER_CHECK:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_PhotometerCheck);
		VerticalMoveOptionBtnFrame(true, 20);
	}break;

	// 清洗反应槽
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_WASH_REACTION_PARTS:
	{
		m_DevBtnsExclusiveFlag = true;
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_ReacTankClean);
		VerticalMoveOptionBtnFrame(true, 80);

		// 初始化
		ui->label_RTC_notice->clear();
		ui->EndComfirmBtn->setDisabled(true);
		ui->EndComfirmBtn->setVisible(true);
	}break;

	// ISE管路填充
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_IsePipeFill);
		VerticalMoveOptionBtnFrame(true, 50);
	}break;

	// ISE管路清洗
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CLEAN_FLUID_PATH:
	{
		m_DevBtnsExclusiveFlag = true;
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_IsePipeClean);
		VerticalMoveOptionBtnFrame(true, 70);

		// 初始化
		ui->label_IPC_notice->clear();
		ui->btn_IPC_PlaceOver->setVisible(false);
		ui->EndComfirmBtn->setDisabled(true);
		ui->EndComfirmBtn->setVisible(true);
	}break;

	// ISE电极更换
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE:
	{
		m_DevBtnsExclusiveFlag = true;
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_IseElecReplace);
		VerticalMoveOptionBtnFrame(true, 80);

		// 初始化
		ui->label_IER_notice->clear();
		ui->EndComfirmBtn->setDisabled(true);
		ui->EndComfirmBtn->setVisible(true);
	}break;

	// 排空供水桶
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_DRAIN_BUCKET:
	{
		m_DevBtnsExclusiveFlag = true;
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_DrainBucket);
		VerticalMoveOptionBtnFrame(true, 80);

		// 初始化
		ui->label_DB_notice->clear();
		ui->EndComfirmBtn->setDisabled(true);
		ui->EndComfirmBtn->setVisible(true);
	}break;

	// 默认窗口
	default:
	{
		ui->paramStackedWidget->setCurrentIndex(ParamPageNum::page_Normal);
	}break;
	}

}

void MaintainOpDlg::AdjustUIBySingleOrPipeLine()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 调整是否联机调整布局
	if (gUiAdapterPtr()->WhetherSingleDevMode())
	{
		ui->TrackFrame->setVisible(false);
		int moveHeight = ui->TrackFrame->height();

		ui->DevFrame->move(ui->DevFrame->x(), ui->DevFrame->y() - moveHeight);
		ui->optionBtnFrame->move(ui->optionBtnFrame->x(), ui->optionBtnFrame->y() - moveHeight);
		ui->paramStackedWidget->move(ui->paramStackedWidget->x(), ui->paramStackedWidget->y() - moveHeight);
		this->setFixedHeight(this->height() - moveHeight);
	}
}

void MaintainOpDlg::VerticalMoveOptionBtnFrame(bool direction, int height)
{
	// 参数检查
	if (height <= 0)
	{
		return;
	}

	// true向下
	if (direction)
	{
		this->setFixedHeight(this->height() + height);
	}
	else
	{
		this->setFixedHeight(this->height() - height);
	}
	
	// 操作按钮框始终在底部
	ui->optionBtnFrame->move(ui->optionBtnFrame->x(), this->height() - OPTION_BOTTOM_MARGIN_HIGHT);
}

void MaintainOpDlg::InitParamByLatestMaintain(const tf::LatestMaintainInfo& latestMaintainInfo)
{
	// 只针对单项维护
	if (latestMaintainInfo.resultDetail.size()!=1)
	{
		return;
	}

	// 结果信息
	auto singleMaintainGroup = MaintainDataMng::GetInstance()->GetSingleMaintainGrpByType(latestMaintainInfo.resultDetail[0].itemType);
	if (singleMaintainGroup.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
	{
		return;
	}

	// 设置上次设置的参数
	InitMaintainGroupParam(singleMaintainGroup);

	// 设置提示信息
	// 去数据库中拿结果，因为数据库是实时更新的，界面缓存的没有实时更新
	tf::LatestMaintainInfoQueryCond detailQryCond;
	tf::LatestMaintainInfoQueryResp detailQryResp;
	detailQryCond.__set_deviceSN(latestMaintainInfo.deviceSN);
	detailQryCond.__set_groupId(latestMaintainInfo.groupId);
	if (!DcsControlProxy::GetInstance()->QueryLatestMaintainInfo(detailQryResp, detailQryCond) ||
		(detailQryResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS) ||
		detailQryResp.lstLatestMaintainInfo.size() != 1)
	{
		ULOG(LOG_ERROR, "%s(), QueryLatestMaintainInfo() failed", __FUNCTION__);
		return;
	}

	// 单项维护
	if (detailQryResp.lstLatestMaintainInfo[0].resultDetail.size() != 1)
	{
		ULOG(LOG_ERROR, "%s(), QueryLatestMaintainInfo() resultDetail size error!", __FUNCTION__);
		return;
	}
	auto realTimeMier = detailQryResp.lstLatestMaintainInfo[0].resultDetail[0];

	OnMaintainItemStageUpdate(QString::fromStdString(latestMaintainInfo.deviceSN), latestMaintainInfo.groupId, realTimeMier);
}

///
/// @brief
///     根据维护名称调整界面
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月20日，新建函数
/// @li 4170/TangChuXian，2024年5月7日，增加对执行维护组时按钮置灰的判断
///
void MaintainOpDlg::AdjustDevBtnByMaintainItem()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取维护组
    tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpByName(m_strMaintainName);

    // 如果不是是单项维护，则返回——mod_tcx:维护组也应该考虑不支持设备执行需要置灰按钮的情况
    // 	if (stuMtGrp.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE ||
    // 		stuMtGrp.items.size() != 1)
    // 	{
    // 		return;
    // 	}

    // 判断设备是否支持维护项
    auto WhetherDevSurportMaintain = [](const std::string& strDevSn, const tf::MaintainGroup& stuMtGrp)
    {
        // 遍历维护组中所有维护项
        tf::DeviceType::type devType = gUiAdapterPtr()->GetDevTypeBySn(QString::fromStdString(strDevSn));
        for (const auto& stuTfMaintainItem : stuMtGrp.items)
        {
            auto stuItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(stuTfMaintainItem.itemType);
            auto it = std::find(stuItem.deviceTypes.begin(), stuItem.deviceTypes.end(), devType);

            // 支持则跳过
            if (it == stuItem.deviceTypes.end())
            {
                continue;
            }

            // 有一个支持则直接返回true
            return true;
        }

        // 没找到则返回false
        return false;
    };

    // 不支持执行维护组的模块置灰
    for (auto it = m_mapBtnDevInfo.begin(); it != m_mapBtnDevInfo.end(); it++)
    {
        // 拿到按钮
        QPushButton* devBtn = it.key();
        if (devBtn == Q_NULLPTR)
        {
            continue;
        }

        // 如果设备不支持执行该维护组，则禁能
        if (!WhetherDevSurportMaintain(it.value().sn, stuMtGrp))
        {
            // 默认先全部置灰
            devBtn->setDisabled(true);
            SwitchDevBtnStatus(devBtn, DEV_BTN_STATUS_DISABLED);
            devBtn->style()->unpolish(devBtn);
            devBtn->style()->polish(devBtn);
            devBtn->update();
        }
    }

    // 遍历
    // 	tf::MaintainItem  maintainItem = MaintainDataMng::GetInstance()->GetMaintainItemByType(stuMtGrp.items[0].itemType);
    // 
    // 	// 遍历设备按钮
    // 	QMapIterator<QPushButton*, ::tf::DevicekeyInfo> devIter(m_mapBtnDevInfo);
    // 	while (devIter.hasNext()) {
    // 		devIter.next();
    // 
    // 		tf::DeviceType::type devType = gUiAdapterPtr()->GetDevTypeBySn(QString::fromStdString(devIter.value().sn));
    // 
    // 		// 未找到则失能该设备按钮
    // 		QPushButton* devBtn = devIter.key();
    // 		if (devBtn == nullptr)
    // 		{
    // 			continue;
    // 		}
    // 
    // 		if (std::find(maintainItem.deviceTypes.begin(), maintainItem.deviceTypes.end(), devType) == maintainItem.deviceTypes.end())
    // 		{
    // 			devBtn->setDisabled(true);
    // 			SwitchDevBtnStatus(devBtn, DEV_BTN_STATUS_DISABLED);
    // 			devBtn->style()->unpolish(devBtn);
    // 			devBtn->style()->polish(devBtn);
    // 			devBtn->update();
    // 		}
    // 	}

    // 如果不是是单项维护，则返回——add_tcx:只有生化的维护单项才需要调整界面
	InitMaintainGroupParam(stuMtGrp);
}

///
/// @brief 设置维护组参数
///
/// @param[in]  group  维护组
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月30日，新建函数
///
void MaintainOpDlg::InitMaintainGroupParam(const tf::MaintainGroup& group)
{
	// 单项维护才设置参数
	if (group.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE ||
		group.items.size() != 1)
	{
		return;
	}

	// 执行查询
	tf::MaintainGroupQueryCond grpQryCond;
	grpQryCond.__set_ids({ group.id });
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
		SetMaintainItemDefaultParam(exeItem);
	}
}

///
/// @brief 设置维护单项参数默认值
///
/// @param[in]  maintainItem  维护单项
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月30日，新建函数
///
void MaintainOpDlg::SetMaintainItemDefaultParam(const tf::MaintainItem&  maintainItem)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 判空
	if (maintainItem.param.empty())
	{
		return;
	}
	// 根据维护类型更新界面参数
	switch (maintainItem.itemType)
	{
	// 比色管路填充
	case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION:
	{
		MiChFluidPerfusionParams mcfpp;
		if (!DecodeJson(mcfpp, maintainItem.param))
		{
			ULOG(LOG_ERROR, "DecodeJson for MiChFluidPerfusionParams failed!");
			return;
		}

		// 设置参数
		ui->acid_clean_solu_ckbox->setChecked(mcfpp.bEnableAcidCleanSolution);
		ui->alkalinity_clean_solu_ckbox->setChecked(mcfpp.bEnableAlkalinityCleanSolution);

	}break;

	// 光度计检查
	case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_PHOTOMETER_CHECK:
	{
		MiChPhotometerCheckParams mcpcp;
		if (!DecodeJson(mcpcp, maintainItem.param))
		{
			ULOG(LOG_ERROR, "DecodeJson for MiChPhotometerCheckParams failed!");
			return;
		}

		// 设置参数
		ui->correct_check_ckbox->setChecked(mcpcp.bEnableCorrectCheck);

	}break;

	// 针清洗
	case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN:
	{
		MiProbeCleanParams mpcp;
		if (!DecodeJson(mpcp, maintainItem.param))
		{
			ULOG(LOG_ERROR, "DecodeJson for MiProbeCleanParams failed!");
			return;
		}

		// 设置参数
		ui->sample_probe_ckbox->setChecked(mpcp.bEnableSampleProbeClean);
		ui->reagent_probe_ckbox->setChecked(mpcp.bEnableReagentProbeClean);

	}break;

	// 反应槽水更换
	case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_INCUBATION_WATER_EXCHANGE:
	{
		MiIncubWaterExchParams miwep;
		if (!DecodeJson(miwep, maintainItem.param))
		{
			ULOG(LOG_ERROR, "DecodeJson for MiIncubWaterExchParams failed!");
			return;
		}

		// 设置参数
		ui->normal_rBtn->setChecked((miwep.iExchangeWaterMode == 0));
		ui->quick_rBtn->setChecked((miwep.iExchangeWaterMode == 1));

	}break;

	// 反应杯更换
	case ::tf::MaintainItemType::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP:
	{
		// 默认全部勾选（2024.8.6）
		for (auto box : ui->paramStackedWidget->currentWidget()->findChildren<QCheckBox*>())
		{
			box->setChecked(true);
		}
	}break;

	// ISE管路填充
	case ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION:
	{
		MiIseFluidPerfusionParams mifpp;
		if (!DecodeJson(mifpp, maintainItem.param))
		{
			ULOG(LOG_ERROR, "DecodeJson for MiIseFluidPerfusionParams failed!");
			return;
		}

		// 设置参数
		ui->Ise_stan_solu_ckbox->setChecked(mifpp.bEnableStandardSolution);
		ui->Ise_buffer_ckbox->setChecked(mifpp.bEnableBuffer);

	}break;

	// ISE电极更换
	case ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE:
	{
		// 设置参数（默认全部勾选，2024.8.6）
		ui->Ise_Na_Elec->setChecked(true);
		ui->Ise_K_Elec->setChecked(true);
		ui->Ise_Cl_Elec->setChecked(true);
		ui->Ise_Ref_Elec->setChecked(true);

	}break;

	default:
	{}break;
	}

}

///
/// @brief
///     获取维护参数
///
/// @param[in]  params  编码后的维护参数
/// @param[in]  info   提示信息
///
/// @return true:获取成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月20日，新建函数
///
bool MaintainOpDlg::GetMaintainSetParam(std::string& params, QString& info)
{
	std::string encodeParam = "";

	tf::MaintainItemType::type currentItemType = MaintainDataMng::GetInstance()->GetMaintainItemByName(m_strMaintainName).itemType;
	
	// 比色管路填充
	if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION
		&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_ChPipeFill)
	{
		bool bEACS = ui->acid_clean_solu_ckbox->isChecked();
		bool bEALCS = ui->alkalinity_clean_solu_ckbox->isChecked();

		// 两者都不选，比色管路填充会用水填充，还是可以执行该项维护

		// 转换数据
		encodeParam = MaintainDataMng::GetInstance()->GetChFluidPerfusionParam(bEACS, bEALCS);
			
		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetChFluidPerfusionParam failed.", __FUNCTION__);
			return false;
		}
	}
	// 光度计检查
	else if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_PHOTOMETER_CHECK
		&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_PhotometerCheck)
	{
		bool bECC = ui->correct_check_ckbox->isChecked();

		// 转换数据
		encodeParam = MaintainDataMng::GetInstance()->GetChPhotometerCheckParam(bECC);

		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetChPhotometerCheckParam failed.", __FUNCTION__);
			return false;
		}
	}
	// 针清洗
	else if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN
			&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_ProbeClean)
	{
		bool bESPC = ui->sample_probe_ckbox->isChecked();
		bool bERPC = ui->reagent_probe_ckbox->isChecked();


		// 针清洗特殊检查
		if (!bESPC && !bERPC)
		{
			info = tr("请至少选择一种针进行清洗!");
			return false;
		}

		// 转换数据
		encodeParam = MaintainDataMng::GetInstance()->GetProbeCleanParam(bESPC, bERPC);

		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetProbeCleanParam failed.", __FUNCTION__);
			return false;
		}
	}
	// 反应杯更换
	else if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP
			&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_CuvetteReplace)
	{
		// 勾选的联块编号列表
		std::vector<int> unitNum = GetCuvetteReplaceCupUnits();
		// 参数检查
		if (unitNum.empty())
		{
			info = tr("请至少选择一联进行更换!");
			return false;
		}

		// 转换数据
		encodeParam = MaintainDataMng::GetInstance()->GetChChangeReactionCupParam(unitNum);

		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetChChangeReactionCupParam failed.", __FUNCTION__);
			return false;
		}
	}
	// 反应槽水更换
	else if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_INCUBATION_WATER_EXCHANGE
		&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_ReacTankWaterReplace)
	{
		// 0:常规换水 1:快速换水
		int iMode = ui->normal_rBtn->isChecked() ? 0 : 1;

		// 转换数据
		encodeParam = MaintainDataMng::GetInstance()->GetReacTankWaterReplaceParam(iMode);

		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetReacTankWaterReplaceParam failed.", __FUNCTION__);
			return false;
		}

	}
	// ISE管路填充
	else if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION
			&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_IsePipeFill)
	{
		bool bES = ui->Ise_stan_solu_ckbox->isChecked();
		bool bEB = ui->Ise_buffer_ckbox->isChecked();

		// ISE管路填充特殊检查
		if (!bES && !bEB)
		{
			info = tr("请至少选择一种填充液进行管路填充!");
			return false;
		}

		// 转换数据
		encodeParam = MaintainDataMng::GetInstance()->GetIseFluidPerfusionParam(bES, bEB);
		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetIseFluidPerfusionParam failed.", __FUNCTION__);
			return false;
		}
	}
	// ISE电极更换
	else if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE
		&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_IseElecReplace)
	{
		bool bNa = ui->Ise_Na_Elec->isChecked();
		bool bK = ui->Ise_K_Elec->isChecked();
		bool bCl = ui->Ise_Cl_Elec->isChecked();
		bool bRef = ui->Ise_Ref_Elec->isChecked();
		

		// 参数检查
		if (!bNa && !bK && !bCl && !bRef)
		{
			info = tr("请至少选择一个电极进行更换!");
			return false;
		}

		// 转换数据
		encodeParam = MaintainDataMng::GetInstance()->GetIseElecReplaceParam(bNa, bK, bCl, bRef);
		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), MaintainDataMng::GetInstance()->GetIseElecReplaceParam failed.", __FUNCTION__);
			return false;
		}
	}
	// 生化试剂扫描（默认参数）
	else if (currentItemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_REAGENT_SCAN
		&& ui->paramStackedWidget->currentIndex() == ParamPageNum::page_Normal)
	{
		// 转换数据
		encodeParam =  MaintainDataMng::GetInstance()->GetMaintainItemByType(currentItemType).param;
		
		// 转换失败
		if (encodeParam == "")
		{
			ULOG(LOG_ERROR, "%s(), Get MAINTAIN_ITEM_CH_REAGENT_SCAN default param failed.", __FUNCTION__);
			return false;
		}
	}

	else{}

	// 返回参数
	params = encodeParam;
	
	return true;
}

///
/// @brief 从界面设置单项维护的参数
///
/// @param[in]  stuMtGrp  执行的维护组
/// @param[in]  exeParams  维护执行参数
///
/// @return true:设置成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年9月10日，新建函数
///
bool MaintainOpDlg::SetMaintainSingleParamFromUi(tf::MaintainGroup& stuMtGrp, tf::MaintainExeParams& exeParams)
{
	// 只有单项维护的参数从界面获取，组合维护使用默认参数
	if (stuMtGrp.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
	{
		return true;
	}

	// 设置维护参数
	std::string param = "";
	QString info = "";
	if (!GetMaintainSetParam(param, info))
	{
		// 获取失败，如果有提示信息要进行提示
		if (!info.isEmpty())
		{
			TipDlg(tr("提示"), info).exec();
		}
		return false;
	}

	// 参数存储
	if (!param.empty() && stuMtGrp.items.size() == 1)
	{
		// 单项维护中只有一个维护项
		stuMtGrp.items[0].param = param;
		if (!DcsControlProxy::GetInstance()->ModifyMaintainGroups({ stuMtGrp }))
		{
			ULOG(LOG_ERROR, "%s(),ModifyMaintainGroup Param Failed!", __FUNCTION__);
			TipDlg(tr("提示"), tr("参数缓存错误！")).exec();
			return false;
		}

		// 设置下发参数
		exeParams.__set_itemParams({ { stuMtGrp.items[0].itemType, param } });
	}

	return true;
}

///
/// @brief	为输入框设置正则表达式
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月21日，新建函数
///
void MaintainOpDlg::SetCtrlsRegExp()
{
	for (auto edit : findChildren<QLineEdit*>())
	{
		edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));
	}
}

///
/// @brief 执行下一阶段维护
///
///
/// @return true : 成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
///
bool MaintainOpDlg::NextStageMaintain()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取维护组
	tf::MaintainGroup stuMtGrp = MaintainDataMng::GetInstance()->GetMaintainGrpByName(m_strMaintainName);
	// 只支持单项维护
	if (stuMtGrp.groupType != tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
	{
		ULOG(LOG_ERROR, "Not Single maintain:%s", m_strMaintainName.toStdString());
		return false;
	}

	//获取选中设备
	std::vector<::tf::DevicekeyInfo> SnAndModelIndexs;
	TakeCheckedDevkeyInfo(SnAndModelIndexs);

	// 只支持单个模块进行
	if (SnAndModelIndexs.size() != 1)
	{
		ULOG(LOG_ERROR, "SnAndModelIndexs size error!");
		return false;
	}

	// 执行下一阶段维护
	tf::MaintainExeParams exeParams;
	exeParams.__set_groupId(stuMtGrp.id);
	exeParams.__set_lstDev(SnAndModelIndexs);
	exeParams.__set_mode(tf::MaintainExeMode::NEXT_STAGE_MAINTENANCE);
	if (!DcsControlProxy::GetInstance()->Maintain(exeParams))
	{
		ULOG(LOG_ERROR, "Next Stage Maintain Failed!");
		return false;
	}

	return true;
}

///
/// @brief 更新提示信息
///
/// @param[in]  mier  维护单项执行结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月9日，新建函数
///
void MaintainOpDlg::UpdateNoticeInfo(const QString& sn, const tf::MaintainItemExeResult& mier)
{
	// 根据维护项进行分发
	switch (mier.itemType)
	{
	// 反应杯更换
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP:
	{
		UpdateCuvetteReplacePageNotice(sn, mier);
	}break;

	// 反应槽清洗
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_WASH_REACTION_PARTS:
	{
		UpdateReacTankCleanPageNotice(mier);
	}break;

	// ISE管路清洗
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CLEAN_FLUID_PATH:
	{
		UpdateIsePipeCleanPageNotice(mier);
	}break;

	// 电极更换
	case tf::MaintainItemType::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE:
	{
		UpdateIseElecReplacePageNotice(mier);
	}break;

	// 排空供水桶
	case tf::MaintainItemType::MAINTAIN_ITEM_CH_DRAIN_BUCKET:
	{
		UpdateDrainBucketPageNotice(mier);
	}break;

	// 默认
	default: {}
			 break;
	}

}

///
/// @brief 反应杯更换阶段更新
///
/// @param[in]  sn  设备序列号
/// @param[in]  mier  维护单项阶段执行结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
///
void MaintainOpDlg::UpdateCuvetteReplacePageNotice(const QString& sn, const tf::MaintainItemExeResult& mier)
{
	// 初始化
	ui->label_CR_notice->clear();
	ui->EndComfirmBtn->setDisabled(true);

	// 执行成功
	if (mier.result == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
	{
		ui->label_CR_notice->setText(tr("仪器已关机，请点击【结束确认】关闭对话框。"));
		ui->EndComfirmBtn->setDisabled(false);

		ui->ExcuteBtn->setVisible(false);
		ui->CloseBtn->setDisabled(true);
	}
}

///
/// @brief 清洗反应槽阶段更新
///
/// @param[in]  mier  维护单项阶段执行结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
///
void MaintainOpDlg::UpdateReacTankCleanPageNotice(const tf::MaintainItemExeResult& mier)
{
	// 初始化
	ui->label_RTC_notice->clear();
	ui->EndComfirmBtn->setDisabled(true);

	if (mier.result == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
	{
		ui->label_RTC_notice->setText(tr("反应槽排水中。"));
		ui->EndComfirmBtn->setDisabled(true);
	}
	else if (mier.result == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
	{
		ui->label_RTC_notice->setText(tr("排水完成，仪器已关机，请点击【结束确认】关闭对话框。"));
		ui->EndComfirmBtn->setDisabled(false);

		ui->ExcuteBtn->setVisible(false);
		ui->CloseBtn->setDisabled(true);
	}
}

///
/// @brief ISE管路清洗阶段更新
///
/// @param[in]  mier  维护单项阶段执行结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
///
void MaintainOpDlg::UpdateIsePipeCleanPageNotice(const tf::MaintainItemExeResult& mier)
{
	// 初始化
	ui->label_IPC_notice->clear();
	ui->btn_IPC_PlaceOver->setVisible(false);
	ui->EndComfirmBtn->setDisabled(true);
	ui->EndComfirmBtn->setVisible(true);

	// 不是执行中直接返回
	if (mier.result != tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
	{
		return;
	}

	// 根据阶段设置提示信息
	switch (mier.currentStage)
	{
	case 2://（人工操作阶段）
	{
		ui->label_IPC_notice->setText(tr("请拆卸电极盖和ISE盖子，拆卸Na、K、Cl、Ref电极，更换成管路清洗用电极，\
打开专用清洗液盖子，通过吸管向清洗用电极中注入约5mL生化分析仪电解质模块用清洗液，盖上电极盖和ISE盖子，然后点击【放置完成】。"));
		ui->btn_IPC_PlaceOver->setVisible(true);
	}break;

	case 3:
	{
		ui->label_IPC_notice->setText(tr("管路清洗中。"));
	}break;

	case 4://（人工操作阶段）
	{
		ui->label_IPC_notice->setText(tr("管路清洗结束。请拆卸电极盖和ISE盖，拆卸管道清洗用电极，安装Na、K、Cl、Ref电极。\
安装电极盖和ISE盖。安装结束后点击【结束确认】，结束维护进程。"));
		ui->EndComfirmBtn->setDisabled(false);
	}break;

	default:
		break;
	}
}

///
/// @brief 电极更换阶段更新
///
/// @param[in]  mier  维护单项阶段执行结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
///
void MaintainOpDlg::UpdateIseElecReplacePageNotice(const tf::MaintainItemExeResult& mier)
{
	// 初始化
	ui->label_IER_notice->clear();
	ui->EndComfirmBtn->setDisabled(true);

	// 不是执行中直接返回
	if (mier.result != tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
	{
		return;
	}

	// 根据阶段设置提示信息
	switch (mier.currentStage)
	{
	case 2:
	{
		ui->label_IER_notice->setText(tr("仪器已关机，请更换电极。更换完毕后，请点击【结束确认】仪器将自动开机。"));
		ui->EndComfirmBtn->setDisabled(false);

		ui->ExcuteBtn->setVisible(false);
		ui->CloseBtn->setDisabled(true);
	}break;

	default:
		break;
	}

	// 执行成功
	if (mier.result == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
	{
		ui->label_IER_notice->setText(tr("仪器已关机，请更换电极。更换完毕后，请点击【结束确认】仪器将自动开机。"));
		ui->EndComfirmBtn->setDisabled(false);

		ui->ExcuteBtn->setVisible(false);
		ui->CloseBtn->setDisabled(true);
	}
}

///
/// @brief 排空供水桶阶段更新
///
/// @param[in]  mier  维护单项阶段执行结果
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
///
void MaintainOpDlg::UpdateDrainBucketPageNotice(const tf::MaintainItemExeResult& mier)
{
	// 初始化
	ui->label_DB_notice->clear();
	ui->EndComfirmBtn->setDisabled(true);

	if (mier.result == tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
	{
		ui->label_DB_notice->setText(tr("供水桶排水中。"));
		ui->EndComfirmBtn->setDisabled(true);
	}
	else if (mier.result == tf::MaintainResult::MAINTAIN_RESULT_SUCCESS)
	{
		ui->label_DB_notice->setText(tr("供水桶排水完成，仪器已关机，请点击【结束确认】关闭对话框。"));
		ui->EndComfirmBtn->setDisabled(false);

		ui->ExcuteBtn->setVisible(false);
		ui->CloseBtn->setDisabled(true);
	}
}

///
/// @brief	切换设备按钮状态
///
/// @param[in]  devBtn		需要切换的设备按钮
/// @param[in]  newStatus   新的状态（status）
///
/// @return  true:切换成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月9日，新建函数
///
bool MaintainOpDlg::SwitchDevBtnStatus( QPushButton* const &devBtn, const QString &newStatus)
{
	if (devBtn == nullptr)
	{
		return false;
	}

	// 失去能的按钮不可切换
	if (devBtn->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() == DEV_BTN_STATUS_DISABLED)
	{
		return false;
	}

	return devBtn->setProperty(DEV_BTN_STATUS_PROPERTY_NAME, newStatus);
}

///
/// @brief
///     选中未维护按钮
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月25日，新建函数
///
void MaintainOpDlg::SelectUnmaintainBtn()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 遍历所有设备
    for (auto pBtn : m_pDevBtnList)
    {
        // 参数检查
        if (pBtn == Q_NULLPTR)
        {
            continue;
        }

		// 如果是互斥维护项，只需要找到正在维护的设备
		if (m_DevBtnsExclusiveFlag)
		{
			// 选中正在执行的按钮
			if(pBtn->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() == DEV_BTN_STATUS_MAINTAINING)
			{
				pBtn->setChecked(true);
			}
		}
		else
		{
			// 可多选维护项，则选中所有未维护按钮
			if (pBtn->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() == DEV_BTN_STATUS_UNMAINTAIN)
			{
				pBtn->setChecked(true);
			}
		}
    }
}

///
/// @brief 获取反应杯更换选择的联排编号
///
///
/// @return 选中的联排编号，没有选中的则为空
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年8月7日，新建函数
///
std::vector<int> MaintainOpDlg::GetCuvetteReplaceCupUnits()
{
	std::vector<int> unitNums;
	if (ui->paramStackedWidget->currentIndex() == ParamPageNum::page_CuvetteReplace)
	{
		for (auto box : ui->paramStackedWidget->currentWidget()->findChildren<QCheckBox*>())
		{
			if (box->isChecked())
			{
				unitNums.push_back(box->objectName().split('_').last().toInt());
			}
		}
	}

	return unitNums;
}
