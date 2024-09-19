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
/// @file     devmaintainstatuswgt.cpp
/// @brief    设备维护状态控件
///
/// @author   4170/TangChuXian
/// @date     2023年2月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "devmaintainstatuswgt.h"
#include "ui_devmaintainstatuswgt.h"
#include "shared/datetimefmttool.h"
#include "uidcsadapter/abstractdevice.h"
#include "uidcsadapter/uidcsadapter.h"
#include "src/common/Mlog/mlog.h"
#include <QVariant>
#include <QMovie>

#define MAINTAIN_STATUS_PROPERTY_NAME                   ("status")                  // 维护状态属性名

#define MAINTAIN_STATUS_EMPTY                           ("empty")                   // 空
#define MAINTAIN_STATUS_NOT_MAINTAIN                    ("not_maintain")            // 未维护
#define MAINTAIN_STATUS_MAINTAINING                     ("maintaining")             // 正在维护
#define MAINTAIN_STATUS_MAINTAIN_FAILED                 ("maintain_failed")         // 失败
#define MAINTAIN_STATUS_MAINTAIN_SUCC                   ("maintain_succ")           // 维护成功

#define PROPERTY_DEVICE_TYPE_I6000                      ("i6000")                   // i6000
#define PROPERTY_DEVICE_TYPE_C1000                      ("c1000")                   // c1000
#define PROPERTY_DEVICE_TYPE_C2000_A                    ("c2000a")                  // c2000a
#define PROPERTY_DEVICE_TYPE_C2000_B                    ("c2000b")                  // c2000b
#define PROPERTY_DEVICE_TYPE_ISE                        ("ise")                     // ise
#define PROPERTY_DEVICE_TYPE_TRACK                      ("iom")                     // iom

DevMaintainStatusWgt::DevMaintainStatusWgt(QWidget *parent)
    : QWidget(parent),
      m_enStatus(DEV_MAINTAIN_STATUS_EMPTY),
	  m_enShowModel(MODEL_EMPTY),
	  m_enDisplayModel(MODEL_NORMAL)
{
    ui = new Ui::DevMaintainStatusWgt();
    ui->setupUi(this);

    // 初始化动画
    m_pMovie = new QMovie(":/Leonis/resource/image/gif-maintaining.gif");

    // 重置
    Reset();

    // 连接信号槽
    connect(ui->DetailBtn, SIGNAL(clicked()), this, SLOT(OnDetailBtnClicked()));
}

DevMaintainStatusWgt::~DevMaintainStatusWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    m_pMovie->deleteLater();
    delete ui;
}

///
/// @brief
///     获取控件的设备名
///
/// @return 设备名
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
QPair<QString, QString> DevMaintainStatusWgt::GetDeviceName()
{
	return m_pairDevName;
}

///
/// @brief
///     设置设备名
///
/// @param[in]  strDevName  设备名
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void DevMaintainStatusWgt::SetDeviceName(const QString& strDevName)
{
    ui->DevNameLab->setText(strDevName);
    ui->TopFrame->setProperty("devType", QVariant());

    // 更新设备图片
    ui->TopFrame->style()->unpolish(ui->TopFrame);
    ui->TopFrame->style()->polish(ui->TopFrame);
    ui->TopFrame->update();

    // 设备组特殊处理
    tf::DeviceType::type enDevType = tf::DeviceType::DEVICE_TYPE_INVALID;
    if (!gUiAdapterPtr()->WhetherContainOtherSubDev(strDevName))
    {
        // 不是设备组
        // 获取对应设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strDevName);
        if (spIDev == Q_NULLPTR)
        {
            return;
        }

        // 获取设备类型
        enDevType = (tf::DeviceType::type)spIDev->DeviceType();
    }
    else
    {
        // 设备组
        // 获取子设备
        QStringList strSubDevNameList;
        if (!gUiAdapterPtr()->GetSubDevNameList(strDevName, strSubDevNameList) || strSubDevNameList.isEmpty())
        {
            return;
        }

        // 获取对应子设备
        std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(strSubDevNameList.first(), strDevName);
        if (spIDev == Q_NULLPTR)
        {
            return;
        }

        // 获取设备类型
        enDevType = (tf::DeviceType::type)spIDev->DeviceType();
    }

    // 判断设备类型
    if (enDevType == tf::DeviceType::DEVICE_TYPE_I6000)
    {
        ui->TopFrame->setProperty("devType", PROPERTY_DEVICE_TYPE_I6000);
    }
    else if (enDevType == tf::DeviceType::DEVICE_TYPE_C1000 || enDevType == tf::DeviceType::DEVICE_TYPE_C200)
    {
        ui->TopFrame->setProperty("devType", PROPERTY_DEVICE_TYPE_C1000);
    }
    else if (enDevType == tf::DeviceType::DEVICE_TYPE_ISE1005)
    {
        ui->TopFrame->setProperty("devType", PROPERTY_DEVICE_TYPE_ISE);
    }
    else if (enDevType == tf::DeviceType::DEVICE_TYPE_TRACK)
    {
        ui->TopFrame->setProperty("devType", PROPERTY_DEVICE_TYPE_TRACK);
    }

    // 更新设备图片
    ui->TopFrame->style()->unpolish(ui->TopFrame);
    ui->TopFrame->style()->polish(ui->TopFrame);
    ui->TopFrame->update();
}

///
/// @brief 获取设备序列号
///
///
/// @return 设备序列号
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年8月28日，新建函数
///
QString DevMaintainStatusWgt::GetDeviceSn()
{
	return m_strDevSn;
}

///
/// @brief 设置设备序列号
///
/// @param[in]  strDevSn  设备序列号
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年8月28日，新建函数
///
void DevMaintainStatusWgt::SetDeviceSn(const QString& strDevSn)
{
	// 缓存设备序列号
	m_strDevSn = strDevSn;

	// 初始化显示
	ui->DevNameLab->setText("");
	ui->TopFrame->setProperty("devType", QVariant());

	// 更新设备图片
	ui->TopFrame->style()->unpolish(ui->TopFrame);
	ui->TopFrame->style()->polish(ui->TopFrame);
	ui->TopFrame->update();

	// 获取（设备名，组名）
	m_pairDevName = gUiAdapterPtr()->GetDevNameBySn(strDevSn);
	std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(m_pairDevName.first, m_pairDevName.second);
	if (spIDev == Q_NULLPTR)
	{
		return;
	}
	
	// 获取设备类型
	tf::DeviceType::type enDevType = (tf::DeviceType::type)spIDev->DeviceType();
	QString devTypeString;
	switch (enDevType)
	{
	// I6000
	case tf::DeviceType::DEVICE_TYPE_I6000:
	{
		devTypeString = PROPERTY_DEVICE_TYPE_I6000;
	}break;

	// C200
	case tf::DeviceType::DEVICE_TYPE_C200:
	{
		devTypeString = PROPERTY_DEVICE_TYPE_C1000;
	}break;

	// C1000和C2000
	case tf::DeviceType::DEVICE_TYPE_C1000:
	{
		// 需要对组特殊处理
		if (!m_pairDevName.second.isEmpty())
		{
			devTypeString = m_pairDevName.first.contains("A") ? PROPERTY_DEVICE_TYPE_C2000_A : PROPERTY_DEVICE_TYPE_C2000_B;
		}
		else
		{
			// 没有组名称，则为C1000
			devTypeString = PROPERTY_DEVICE_TYPE_C1000;
		}
		
	}break;

	// ISE
	case tf::DeviceType::DEVICE_TYPE_ISE1005:
	{
		devTypeString = PROPERTY_DEVICE_TYPE_ISE;
	}break;

	// 轨道
	case tf::DeviceType::DEVICE_TYPE_TRACK:
	{
		devTypeString = PROPERTY_DEVICE_TYPE_TRACK;
	}break;

	default:
	{
		return;
	}break;
	}

	// 更新设备名称
	ui->DevNameLab->setText(m_pairDevName.second + m_pairDevName.first);

	// 更新设备图片
	ui->TopFrame->setProperty("devType", devTypeString);
	ui->TopFrame->style()->unpolish(ui->TopFrame);
	ui->TopFrame->style()->polish(ui->TopFrame);
	ui->TopFrame->update();
}

///
/// @brief
///     获取进度
///
/// @return 进度百分百
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
QString DevMaintainStatusWgt::GetProgress()
{
    return m_strProgress;
}

///
/// @brief
///     设置进度
///
/// @param[in]  strProgress  进度百分百
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void DevMaintainStatusWgt::SetProgress(const QString& strProgress, const QString& strItemCount, const QString& strItemName)
{
    m_strProgress = strProgress;
	m_strItemCount = strItemCount;
	m_strItemName = strItemName;

	// 按照维护显示设置更新状态界面
	auto UpdateMaintainDetailToUi = [this](const QString& maintainStatus)
	{
		if (m_enShowModel == MODEL_PERCENT)
		{
			ui->MaintainStatusLab->setText(maintainStatus + m_strProgress);
			ui->ProgressLab->setText(m_strItemName);
		}
		else if(m_enShowModel == MODEL_ITEM_CNT)
		{
			ui->MaintainStatusLab->setText(maintainStatus + m_strItemCount);
			ui->ProgressLab->setText(m_strItemName);
		}

		return;
		
	};

	// 根据维护状态更新界面
    if (m_enStatus == DEV_MAINTAIN_STATUS_MAINTAINING)
    {
		UpdateMaintainDetailToUi(tr("正在维护"));
    }
	else if (m_enStatus == DEV_MAINTAIN_STATUS_FAILED)
	{
		UpdateMaintainDetailToUi(tr("维护失败"));
	}
}

///
/// @brief
///     获取维护时间
///
/// @return 维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月25日，新建函数
///
QString DevMaintainStatusWgt::GetMaintainTime()
{
    return m_strMaintainTime;
}

///
/// @brief
///     设置维护时间
///
/// @param[in]  strMaintainTime  维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月25日，新建函数
///
void DevMaintainStatusWgt::SetMaintainTime(const QString& strMaintainTime)
{
    m_strMaintainTime = strMaintainTime;
    if ((m_enStatus == DEV_MAINTAIN_STATUS_SUCC) || (m_enStatus == DEV_MAINTAIN_STATUS_FAILED))
    {
        QStringList strTimeList = m_strMaintainTime.split(" ");
        if (strTimeList.size() < 2)
        {
            return;
        }

        QString strDate = ToCfgFmtDateTime(strTimeList.front());
        QString strTime = ToCfgFmtDateTime(strTimeList.back());
        ui->MaintainStatusLab->setText(strTime);
        ui->ProgressLab->setText(strDate);
    }
}

///
/// @brief
///     获取设备维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
DevMaintainStatusWgt::DevMaintainStatus DevMaintainStatusWgt::GetStatus()
{
    return m_enStatus;
}

///
/// @brief
///     设置设备维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void DevMaintainStatusWgt::SetStatus(DevMaintainStatus enStatus)
{
    // 设置状态
    m_enStatus = enStatus;

    // 默认详情按钮使能
    ui->DetailBtn->setEnabled(true);

    // 获取维护时间
    QString strDate("");
    QString strTime("");
    QStringList strTimeList = m_strMaintainTime.split(" ");
    if (strTimeList.size() >= 2)
    {
        strDate = ToCfgFmtDateTime(strTimeList.front());
        strTime = ToCfgFmtDateTime(strTimeList.back());
    }

    // 根据状态构造对应字符串（用于qss动态属性
    QString strStatus("");
    switch (enStatus)
    {
    case DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_EMPTY:
        strStatus = MAINTAIN_STATUS_EMPTY;
        ui->DevNameLab->clear();
        ui->MaintainStatusLab->clear();
        ui->ProgressLab->clear();
        ui->DetailBtn->setVisible(false);
        ui->GifLab->setMovie(Q_NULLPTR);
        ui->GifLab->setVisible(false);
        m_pMovie->stop();
        break;
    case DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_NOT_MAINTAIN:
        strStatus = MAINTAIN_STATUS_NOT_MAINTAIN;
        ui->DetailBtn->setEnabled(false);
        ui->MaintainStatusLab->setText(tr("未维护"));
        ui->ProgressLab->setVisible(false);
        ui->DetailBtn->setVisible(true);
        ui->GifLab->setMovie(Q_NULLPTR);
        ui->GifLab->setVisible(false);
        m_pMovie->stop();
        break;
    case DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_MAINTAINING:
        strStatus = MAINTAIN_STATUS_MAINTAINING;
		UpdateMaintainDetailToUi();
        ui->ProgressLab->setVisible(true);
        ui->DetailBtn->setVisible(true);
        ui->GifLab->setMovie(m_pMovie);
        ui->GifLab->setVisible(true);
        if (m_pMovie->state() != QMovie::Running)
        {
            m_pMovie->start();
        }
        break;
    case DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_FAILED:
        strStatus = MAINTAIN_STATUS_MAINTAIN_FAILED;
        //ui->MaintainStatusLab->setText(strTime);
        //->ProgressLab->setText(strDate);
		UpdateMaintainDetailToUi();
        ui->ProgressLab->setVisible(true);
        ui->DetailBtn->setVisible(true);
        ui->GifLab->setMovie(Q_NULLPTR);
        ui->GifLab->setVisible(false);
        m_pMovie->stop();
        break;
    case DevMaintainStatusWgt::DEV_MAINTAIN_STATUS_SUCC:
        strStatus = MAINTAIN_STATUS_MAINTAIN_SUCC;
        ui->MaintainStatusLab->setText(strTime);
        ui->ProgressLab->setText(strDate);
        ui->ProgressLab->setVisible(true);
        ui->DetailBtn->setVisible(true);
        ui->GifLab->setMovie(Q_NULLPTR);
        ui->GifLab->setVisible(false);
        m_pMovie->stop();
        break;
    default:
        break;
    }

    // 更新动态属性
    ui->TopFrame->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    ui->BottomFrame->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    ui->DevNameLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    ui->MaintainStatusLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    ui->ProgressLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    ui->DetailBtn->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);

    // 刷新界面显示
    UpdateStatus();
}

///
/// @brief
///     更新状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月13日，新建函数
///
void DevMaintainStatusWgt::UpdateStatus()
{
    ui->TopFrame->style()->unpolish(ui->TopFrame);
    ui->TopFrame->style()->polish(ui->TopFrame);
    ui->TopFrame->update();

    ui->BottomFrame->style()->unpolish(ui->BottomFrame);
    ui->BottomFrame->style()->polish(ui->BottomFrame);
    ui->BottomFrame->update();

    ui->DevNameLab->style()->unpolish(ui->DevNameLab);
    ui->DevNameLab->style()->polish(ui->DevNameLab);
    ui->DevNameLab->update();

    ui->MaintainStatusLab->style()->unpolish(ui->MaintainStatusLab);
    ui->MaintainStatusLab->style()->polish(ui->MaintainStatusLab);
    ui->MaintainStatusLab->update();

    ui->ProgressLab->style()->unpolish(ui->ProgressLab);
    ui->ProgressLab->style()->polish(ui->ProgressLab);
    ui->ProgressLab->update();

    ui->DetailBtn->style()->unpolish(ui->DetailBtn);
    ui->DetailBtn->style()->polish(ui->DetailBtn);
    ui->DetailBtn->update();
}

///
/// @brief
///     重置
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月25日，新建函数
///
void DevMaintainStatusWgt::Reset()
{
	m_strDevSn = "";
	m_pairDevName = QPair<QString, QString>();;
    SetStatus(DEV_MAINTAIN_STATUS_EMPTY);
}

///
/// @brief
///     设置维护信息显示模式
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月19日，新建函数
///
void DevMaintainStatusWgt::SetMaintainInfoShowModel(const DevMaintainInfoShowModel& showModel)
{
	if (showModel == m_enShowModel)
	{
		return;
	}

	m_enShowModel = showModel;

	// 更新界面
	UpdateMaintainDetailToUi();
}

///
/// @brief
///     设置维护信息显示模式
///
/// @param[in]  maintainStatus  维护状态（仅支持正在维护和维护失败）
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月19日，新建函数
///
void DevMaintainStatusWgt::UpdateMaintainDetailToUi()
{
	// 按照维护显示设置更新状态界面
	auto UpdateToUi = [this](const QString& maintainStatus)
	{
		if (m_enShowModel == MODEL_PERCENT)
		{
			ui->MaintainStatusLab->setText(maintainStatus + m_strProgress);
			ui->ProgressLab->setText(m_strItemName);
		}
		else if (m_enShowModel == MODEL_ITEM_CNT)
		{
			ui->MaintainStatusLab->setText(maintainStatus + m_strItemCount);
			ui->ProgressLab->setText(m_strItemName);
		}

		return;

	};

	// 根据维护状态更新界面
	if (m_enStatus == DEV_MAINTAIN_STATUS_MAINTAINING)
	{
		UpdateToUi(tr("正在维护"));
	}
	else if (m_enStatus == DEV_MAINTAIN_STATUS_FAILED)
	{
		UpdateToUi(tr("维护失败"));
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
void DevMaintainStatusWgt::showEvent(QShowEvent* event)
{
    if ((m_enStatus == DEV_MAINTAIN_STATUS_SUCC) || (m_enStatus == DEV_MAINTAIN_STATUS_FAILED))
    {
        QStringList strTimeList = m_strMaintainTime.split(" ");
        if (strTimeList.size() >= 2)
        {
            QString strDate = ToCfgFmtDateTime(strTimeList.front());
            QString strTime = ToCfgFmtDateTime(strTimeList.back());
            ui->MaintainStatusLab->setText(strTime);
            ui->ProgressLab->setText(strDate);
        }
    }
    return QWidget::showEvent(event);
}

///
/// @brief
///     详情按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月25日，新建函数
///
void DevMaintainStatusWgt::OnDetailBtnClicked()
{
    // 没有设备序列号直接返回
    if (m_strDevSn.isEmpty())
    {
        return;
    }

    // 发送信号
    emit SigDevDetailWgtClicked(m_strDevSn);
}
