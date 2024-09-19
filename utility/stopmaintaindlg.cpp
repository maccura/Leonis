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
/// @file     stopmaintaindlg.cpp
/// @brief    停止维护对话框
///
/// @author   4170/TangChuXian
/// @date     2023年7月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "stopmaintaindlg.h"
#include "ui_stopmaintaindlg.h"
#include "maintaingrpdetail.h"
#include "maintaindatamng.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "uidcsadapter/uidcsadapter.h"
#include "uidcsadapter/abstractdevice.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include <QHBoxLayout>
#include <QPushButton>

#define DEV_BTN_STATUS_PROPERTY_NAME                    ("status")              // 设备按钮状态属性名

#define DEV_BTN_STATUS_UNMAINTAIN                       ("unmaintain")          // 未维护
#define DEV_BTN_STATUS_MAINTAINING                      ("maintaining")         // 正在维护

StopMaintainDlg::StopMaintainDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    // 初始化Ui对象
    ui = new Ui::StopMaintainDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

StopMaintainDlg::~StopMaintainDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建函数
///
void StopMaintainDlg::InitBeforeShow()
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
		pDevBtn->setProperty(DEV_BTN_STATUS_PROPERTY_NAME, DEV_BTN_STATUS_UNMAINTAIN);
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

	// 如果只有一个按钮，则选中
	if (m_pDevBtnList.size() == 1)
	{
		// 更新维护组按钮状态
		m_pDevBtnList[0]->setChecked(true);
		UpdateMtGrpBtnStatus();
	}

	// 根据单机还是联机调整界面
	if (gUiAdapterPtr()->WhetherSingleDevMode())
	{
		ui->TrackFrame->setVisible(false);
		int moveHeight = ui->TrackFrame->height();

		ui->DevFrame->move(ui->DevFrame->x(), ui->DevFrame->y() - moveHeight);
		ui->optionBtnFrame->move(ui->optionBtnFrame->x(), ui->optionBtnFrame->y() - moveHeight);
		this->setFixedHeight(this->height() - moveHeight);
	}
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建函数
///
void StopMaintainDlg::InitAfterShow()
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
void StopMaintainDlg::InitStrResource()
{

}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建函数
///
void StopMaintainDlg::InitConnect()
{
    // 状态表选择列被点击
    connect(ui->StopMtBtn, SIGNAL(clicked()), this, SLOT(OnStopBtnClicked()));

    // 取消按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 监听维护组阶段更新
    REGISTER_HANDLER(MSG_ID_MAINTAIN_GROUP_PHASE_UPDATE, this, OnMaintainGrpPhaseUpdate);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月14日，新建函数
///
void StopMaintainDlg::InitChildCtrl()
{
    // 设置标题
    SetTitleName(tr("停止维护"));

    // 设置维护组按钮组
    QStringList strDevNameList;;
    for (auto it = m_mapBtnDevInfo.begin(); it != m_mapBtnDevInfo.end(); it++)
    {
        strDevNameList.push_back(it.key()->text());
    }

    // 更新设备维护组状态
    UpdateDevMaintainGrpStatus();

    // 选择正在维护的按钮
    SelectMaintainingBtn();

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
/// @li 4170/TangChuXian，2023年7月14日，新建函数
///
void StopMaintainDlg::showEvent(QShowEvent *event)
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
void StopMaintainDlg::TakeCheckedDevkeyInfo(std::vector<::tf::DevicekeyInfo>& checkedDev)
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

		// 存在SN相同，并且模块不同的，则设置为整机
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
///     选中正在维护按钮
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月25日，新建函数
///
void StopMaintainDlg::SelectMaintainingBtn()
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

        // 如果状态不为未维护则跳过
        if (pBtn->property(DEV_BTN_STATUS_PROPERTY_NAME).toString() != DEV_BTN_STATUS_MAINTAINING)
        {
            continue;
        }

        // 选中按钮
        pBtn->setChecked(true);
    }
}

///
/// @brief
///     停止按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月18日，新建函数
///
void StopMaintainDlg::OnStopBtnClicked()
{
    // 弹框询问：确定停止维护
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("停止维护"), tr("确定停止维护？"), TipDlgType::TWO_BUTTON));
    if (pTipDlg->exec() == QDialog::Rejected)
    {
        return;
    }

    // 获取勾选的按钮列表和设备SN
    std::vector<::tf::DevicekeyInfo> SnAndModelIndexs;
	TakeCheckedDevkeyInfo(SnAndModelIndexs);

    // 执行维护
    if (!DcsControlProxy::GetInstance()->StopMaintain(SnAndModelIndexs))
    {
        ULOG(LOG_ERROR, "%s(), Maintain() failed", __FUNCTION__);
        return;
    }
}

///
/// @brief
///     更新维护组按钮显示状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月23日，新建函数
///
void StopMaintainDlg::UpdateMtGrpBtnStatus()
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
    ui->StopMtBtn->setEnabled(bStopBtnEnable);

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
void StopMaintainDlg::OnDevBtnClicked()
{
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
void StopMaintainDlg::OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType)
{
    Q_UNUSED(strDevSn);
    Q_UNUSED(lGrpId);
    Q_UNUSED(enPhaseType);
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
            (*itDevBtn)->setProperty(DEV_BTN_STATUS_PROPERTY_NAME, DEV_BTN_STATUS_MAINTAINING);
        }
        else
        {
            // 将按钮状态设置为未在维护
            (*itDevBtn)->setProperty(DEV_BTN_STATUS_PROPERTY_NAME, DEV_BTN_STATUS_UNMAINTAIN);
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
/// @brief
///     更新设备维护组状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月30日，新建函数
///
void StopMaintainDlg::UpdateDevMaintainGrpStatus()
{
    // 更新设备按钮状态
    for (auto pBtn : m_pDevBtnList)
    {
        pBtn->setProperty(DEV_BTN_STATUS_PROPERTY_NAME, DEV_BTN_STATUS_UNMAINTAIN);
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
        (*itDevBtn)->setProperty(DEV_BTN_STATUS_PROPERTY_NAME, DEV_BTN_STATUS_MAINTAINING);
    };

    // 遍历设备最近维护记录，更新设备维护状态
    for (auto itMap = mapDevMtStatus.begin(); itMap != mapDevMtStatus.end(); itMap++)
    {
        // 如果维护状态不为正在维护，则跳过
        if (itMap.value().exeResult != tf::MaintainResult::MAINTAIN_RESULT_EXECUTING)
        {
            continue;
        }

        // 获取设备名
        QPair<QString, QString> strDevNamePair = gUiAdapterPtr()->GetDevNameBySn(QString::fromStdString(itMap.value().deviceSN));
        if (strDevNamePair.first.isEmpty())
        {
            continue;
        }

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
