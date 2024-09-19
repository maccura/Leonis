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
/// @file     StartupSetDlg.cpp
/// @brief    应用--系统--自动开机设置
///
/// @author   7951/LuoXin
/// @date     2022年10月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年10月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "StartupSetDlg.h"
#include "ui_StartupSetDlg.h" 
#include <QCheckBox>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QTime>
#include <QTimeEdit>

#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"


#define POWERON_TIME_FORMART "HH:mm"   // 开机计划的时间格式
#define WEEK_DAY_COUNT 7 // 一周7天

// StartupSetDg
StartupSetDlg::StartupSetDlg(QWidget *parent)
    : BaseDlg(parent)
    , ui(new Ui::StartupSetDlg)
{
    ui->setupUi(this);

    InitCtrls();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

StartupSetDlg::~StartupSetDlg()
{
}

void StartupSetDlg::InitCtrls()
{
    SetTitleName(tr("自动开机"));

    QStringList headNames = { tr("仪器"), tr("星期一"), tr("星期二"), \
        tr("星期三"), tr("星期四"), tr("星期五"), tr("星期六"), tr("星期日")};

    // 初始化列头
    m_model = new QStandardItemModel();
    m_model->setHorizontalHeaderLabels(headNames);
    ui->tableViewStartupSet->setModel(m_model);
    ui->tableViewStartupSet->horizontalHeader()->setHighlightSections(false);
    ui->tableViewStartupSet->verticalHeader()->hide();

    // 初始化模块行 
    auto devMaps = CommonInformationManager::GetInstance()->GetDeviceMaps();
    int rIdx = 0;
    for (auto it = devMaps.begin(); it != devMaps.end(); ++it)
    {
        auto& curDevice = it->second;
        if (curDevice == nullptr)
        {
            continue;
        }

        // 轨道设备不支持自动开机。ISE与比色设备是一体的不支持单独开机
        if (curDevice->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK
            || curDevice->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            continue;
        }

        // 填充空项
        QList<QStandardItem*> rowEmpty;
        for (int i = 0; i < headNames.size(); ++i)
        {
            rowEmpty.append(new QStandardItem(""));
        }
        m_model->appendRow(rowEmpty);
        QString strDevSn = QString::fromStdString(curDevice->deviceSN);

        // 仪器单元格
        QCheckBox* instractorBox = new QCheckBox(QString::fromStdString(curDevice->groupName + curDevice->name));
        instractorBox->setObjectName(strDevSn);
        connect(instractorBox, SIGNAL(stateChanged(int)), this, SLOT(OnDeviceChecked(int)));
        ui->tableViewStartupSet->setIndexWidget(m_model->index(rIdx, 0), instractorBox);
        instractorBox->setChecked(curDevice->powerOnSchedule.times.size() > 0);

        // 时间设置单元格
        std::vector<CellItem> deviceCellItems;
        for (int i = 1; i <= WEEK_DAY_COUNT; ++i) // 一周7天
        {
            QWidget *cellWidget = new QWidget(this);
            QHBoxLayout* hLayout = new QHBoxLayout();
            QCheckBox *ck = new QCheckBox(cellWidget);
            ck->setObjectName(strDevSn + "_" + QString::number(i));
            connect(ck, SIGNAL(stateChanged(int)), this, SLOT(OnDeviceChecked(int)));
            hLayout->addWidget(ck);

            QTimeEdit *ctrTime = new QTimeEdit(cellWidget);
            ctrTime->setDisplayFormat(POWERON_TIME_FORMART);
            hLayout->addWidget(ctrTime);
            cellWidget->setLayout(hLayout);

            // 对控件赋值
            auto schIter = curDevice->powerOnSchedule.times.find(i);
            if (schIter != curDevice->powerOnSchedule.times.end())
            {
                ck->setChecked(true);
                ctrTime->setTime(QTime::fromString(QString::fromStdString(schIter->second), POWERON_TIME_FORMART));
            }

            ui->tableViewStartupSet->setIndexWidget(m_model->index(rIdx, i), cellWidget);
            deviceCellItems.push_back(CellItem(ck, ctrTime));
        }

        m_rowItems[curDevice->deviceSN] = deviceCellItems;
        rIdx++;
    }

    // 设置列宽
    ui->tableViewStartupSet->setColumnWidth(0, 101);
    ui->tableViewStartupSet->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewStartupSet->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);

    for (int i = 1; i < headNames.size(); ++i)
    {
        ui->tableViewStartupSet->setColumnWidth(i, 179);
    }
    // 设置行高
    for (int i = 0; i < m_rowItems.size(); ++i)
    {
        ui->tableViewStartupSet->setRowHeight(i, 77);
    }

    // 确认按钮
    connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
    // 时间修改
    connect(ui->timeEdit, SIGNAL(timeChanged(const QTime&)), this, SLOT(OnTimeChanged(const QTime&)));
}

void StartupSetDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 保存自动开机设置
    std::map<std::string, tf::PowerOnSchedule> powerSchedule;
    std::map<std::string, std::vector<CellItem>>::iterator it = m_rowItems.begin();
    for (; it != m_rowItems.end(); ++it)
    {
        // 获取该设备勾选时间的计划
        tf::PowerOnSchedule weekSchedule;
        for (int i = 0; i < it->second.size(); ++i)
        {
            if (it->second[i].itemCheck->isChecked())
            {
                weekSchedule.times[i + 1] = it->second[i].itemTimeE->time().toString(POWERON_TIME_FORMART).toStdString();
            }
        }

        // 未勾选任何一天的计划也要更新，下位机需要更新计划
        powerSchedule[it->first] = weekSchedule;
    }

    if (!CommonInformationManager::GetInstance()->UpdateDevicePoweronSchedul(powerSchedule))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
        TipDlg(tr("保存失败"), "保存失败").exec();
    }

    this->accept();
}

void StartupSetDlg::OnTimeChanged(const QTime& time)
{
    std::map<std::string, std::vector<CellItem>>::iterator it = m_rowItems.begin();
    for (; it != m_rowItems.end(); ++it)
    {
        for (int i = 0; i < it->second.size(); ++i)
        {
            // 只对勾选的时间框进行统一时间设置
            if (it->second[i].itemCheck->isChecked())
            {
                it->second[i].itemTimeE->setTime(time);
            }
        }
    }
}

void StartupSetDlg::OnDeviceChecked(int stat)
{
    QCheckBox* devCheck = qobject_cast<QCheckBox*>(sender());
    if (devCheck == Q_NULLPTR)
    {
        return;
    }
    bool isSubCheck = false; // 是否是子勾选框
    QString strDevSn = devCheck->objectName();
    QStringList objParts = strDevSn.split('_');
    if (objParts.size() > 1)
    {
        strDevSn = objParts[0];
        isSubCheck = true;
    }

    std::map<std::string, std::vector<CellItem>>::iterator it = m_rowItems.find(strDevSn.toStdString());
    if (it == m_rowItems.end())
    {
        return;
    }

    // 如果是子勾选框
    if (isSubCheck)
    {      
        int checkedCnt = 0; // 统计是否全选
        for (int i = 0; i < it->second.size(); ++i)
        {
            checkedCnt += it->second[i].itemCheck->isChecked();
        }

        QCheckBox* pDevCheck = ui->tableViewStartupSet->findChild<QCheckBox*>(strDevSn);
        if (pDevCheck != Q_NULLPTR)
        {
            pDevCheck->blockSignals(true);
            pDevCheck->setChecked(checkedCnt > 0); // 如果存在勾选，则自动勾选设备
            pDevCheck->blockSignals(false);
        }
    }
    // 是设备勾选框
    else
    {
        bool isChecked = devCheck->isChecked();
        for (int i = 0; i < it->second.size(); ++i)
        {
            QCheckBox* pCur = it->second[i].itemCheck;

            pCur->blockSignals(true);
            pCur->setChecked(isChecked);
            pCur->blockSignals(false);
        }
    }
}

void StartupSetDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    bool isPmt = userPms->IsPermisson(PSM_IM_AUTO_POWERON);
    ui->timeEdit->setEnabled(isPmt);
    ui->tableViewStartupSet->setEnabled(isPmt);
    ui->save_btn->setEnabled(isPmt);
}
