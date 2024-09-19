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
/// @file     imloaderresetdlg.cpp
/// @brief    免疫磁珠混匀对话框
///
/// @author   4170/TangChuXian
/// @date     2024年7月4日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "imloaderresetdlg.h"
#include "QCheckBox"
#include "ui_imloaderresetdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"

ImLoaderResetDlg::ImLoaderResetDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    ui = new Ui::ImLoaderResetDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

ImLoaderResetDlg::~ImLoaderResetDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     显示之前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
void ImLoaderResetDlg::InitBeforeShow()
{
    SetTitleName(tr("装载装置复位"));

    // 监听设备状态改变，只有待机设备可勾选
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDeviceStatusUpdate);

    // 初始化设备选项
    m_vCB.push_back(ui->Module1CB);
    m_vCB.push_back(ui->Module2CB);
    m_vCB.push_back(ui->Module3CB);
    m_vCB.push_back(ui->Module4CB);

    // 隐藏所有复选框
    for (auto pCB : m_vCB)
    {
        pCB->setVisible(false);
        pCB->setEnabled(false);
    }

    // 首先刷新设备信息
    auto devInfoList = CommonInformationManager::GetInstance()->GetDeviceFromType(std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_I6000 }));

    // 遍历所有设备
    int iCB = 0;
    for (auto devInfo : devInfoList)
    {
        // 越界判断
        if (iCB >= m_vCB.count())
        {
            break;
        }

        // 显示对应复选框为设备名
        m_vCB[iCB]->setVisible(true);
        if (devInfo->status != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
        {
            m_vCB[iCB]->setEnabled(true);
        }
        else
        {
            m_vCB[iCB]->setEnabled(false);
            m_vCB[iCB]->setChecked(false);
        }
        m_vCB[iCB]->setText(QString::fromStdString(devInfo->name));
        m_mapNameToSn.insert(std::pair<std::string, std::string>(devInfo->name, devInfo->deviceSN));

        // 索引自增
        ++iCB;
    }

    // 填满网格布局
    QGridLayout* pGridLyt = qobject_cast<QGridLayout*>(ui->ContentFrame->layout());
    if (pGridLyt == Q_NULLPTR)
    {
        return;
    }

    // 计算行号列号
    QList<QCheckBox*> delCBList;
    while (iCB < m_vCB.size())
    {
        int iRow = iCB / 2 + 1;
        int iCol = iCB % 2 + 1;
        pGridLyt->removeWidget(m_vCB[iCB]);
        delCBList.push_back(m_vCB[iCB]);
        pGridLyt->addItem(new QSpacerItem(m_vCB[iCB]->width(), m_vCB[iCB]->height()), iRow, iCol);
        ++iCB;
    }

    // 移除
    for (auto pCB : delCBList)
    {
        m_vCB.removeOne(pCB);
    }

    // 如果只有一个设备，则不显示设备选择
    if (m_vCB.size() == 1)
    {
        m_vCB[0]->setChecked(true);
        m_vCB[0]->setEnabled(true);
        ui->AllCB->setChecked(true);
        ui->ContentFrame->setVisible(false);
        ui->OkBtn->move(ui->OkBtn->x(), ui->OkBtn->y() - 150);
        ui->CancelBtn->move(ui->CancelBtn->x(), ui->CancelBtn->y() - 150);
        this->resize(this->width(), this->height() - 150);
    }
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
void ImLoaderResetDlg::InitAfterShow()
{
    // 连接信号槽
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->AllCB, SIGNAL(toggled(bool)), this, SLOT(OnAllCBCheckChanged()));
    for (auto pCB : m_vCB)
    {
        connect(ui->Module1CB, SIGNAL(toggled(bool)), this, SLOT(OnModuleCBCheckChanged()));
    }
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
void ImLoaderResetDlg::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
        m_bInit = true;
    }
}

///
/// @brief
///     【全部】复选框选中状态改变
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImLoaderResetDlg::OnAllCBCheckChanged()
{
    for (auto pCB : m_vCB)
    {
        // 置灰的则跳过
        if (!pCB->isEnabled())
        {
            continue;
        }

        disconnect(pCB, SIGNAL(toggled(bool)), this, SLOT(OnModuleCBCheckChanged()));
        pCB->setChecked(ui->AllCB->isChecked());
        connect(pCB, SIGNAL(toggled(bool)), this, SLOT(OnModuleCBCheckChanged()));
    }
}

///
/// @brief
///     模块复选框选中状态改变
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImLoaderResetDlg::OnModuleCBCheckChanged()
{
    disconnect(ui->AllCB, SIGNAL(toggled(bool)), this, SLOT(OnAllCBCheckChanged()));

    bool bAllCheck = true;
    for (auto pCB : m_vCB)
    {
        if (!pCB->isChecked())
        {
            bAllCheck = false;
            break;
        }
    }
    ui->AllCB->setChecked(bAllCheck);

    connect(ui->AllCB, SIGNAL(toggled(bool)), this, SLOT(OnAllCBCheckChanged()));
}

///
/// @brief
///     设备状态更新
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月19日，新建函数
///
void ImLoaderResetDlg::OnDeviceStatusUpdate(tf::DeviceInfo deviceInfo)
{
    // 如果只有一个设备时，不允许用户选择，默认必然选择
    if (m_vCB.size() == 1)
    {
        return;
    }

    // 根据选中设备执行装载装置复位
    for (auto pCB : m_vCB)
    {
        // 未显示
        if (!pCB->isVisible())
        {
            continue;
        }

        // 设备名不匹配则跳过
        if (pCB->text() != QString::fromStdString(deviceInfo.name))
        {
            continue;
        }

        // 根据状态更新对应复选框
        if ((deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY) ||
            (deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP))
        {
            pCB->setEnabled(true);
        }
        else
        {
            pCB->setEnabled(false);
            pCB->setChecked(false);
        }
    }
}

void ImLoaderResetDlg::GetSelectDeviceSn(std::vector<std::string>& deviceSnVec)
{
    // 根据选中设备执行磁珠混匀
    for (auto pCB : m_vCB)
    {
        // 未显示或未选中
        if (!pCB->isEnabled() || !pCB->isChecked())
        {
            continue;
        }

        // 获取对应的设备序列号
        auto it = m_mapNameToSn.find(pCB->text().toStdString());
        if (it == m_mapNameToSn.end())
        {
            continue;
        }
        deviceSnVec.push_back(it->second);
    }
}
