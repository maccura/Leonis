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

#include "QDeviceSupplyShowList.h"
#include "ui_QDeviceSupplyShowList.h"

QDeviceSupplyShowList::QDeviceSupplyShowList(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QDeviceSupplyShowList();
    ui->setupUi(this);
    Init();
}

QDeviceSupplyShowList::~QDeviceSupplyShowList()
{
}

///
/// @brief
///     刷新耗材页面
///
/// @param[in]  list  数据列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QDeviceSupplyShowList::RefreshSupplyList(std::map<std::shared_ptr<const tf::DeviceInfo>, std::vector<ch::tf::SuppliesOverview>>& list)
{
    // 数目不对，返回
    if (list.size() > 3 || list.empty())
    {
        return;
    }

    // 依次遍历耗材
    int seq = 0;
    for (const auto& supply : list)
    {
        // 生化设备
        if (supply.first->deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
        {
            m_deviceSupplyList[seq]->SetSupplyStatus(QDeviceSupplyStatus::DEVICETYPE::CHE, supply.second);
            m_deviceSupplyList[seq]->SetDeviceName(QString::fromStdString(supply.first->deviceSN));
            m_deviceSupplyList[seq]->show();
        }
        // ISE设备
        else if (supply.first->deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            m_deviceSupplyList[seq]->SetSupplyStatus(QDeviceSupplyStatus::DEVICETYPE::ISE, supply.second);
            m_deviceSupplyList[seq]->SetDeviceName(QString::fromStdString(supply.first->deviceSN));
            m_deviceSupplyList[seq]->show();
        }

        seq++;
    }
}

///
/// @brief
///     初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月15日，新建函数
///
void QDeviceSupplyShowList::Init()
{
    m_deviceSupplyList.push_back(ui->Device_Widgets1);
    m_deviceSupplyList.push_back(ui->Device_Widgets2);
    m_deviceSupplyList.push_back(ui->Device_Widgets3);
    ui->Device_Widgets1->hide();
    ui->Device_Widgets2->hide();
    ui->Device_Widgets3->hide();
}
