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

#pragma once

#include <QWidget>
#include <map>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

class QDeviceSupplyStatus;

namespace Ui {
    class QDeviceSupplyShowList;
};


class QDeviceSupplyShowList : public QWidget
{
    Q_OBJECT

public:
    QDeviceSupplyShowList(QWidget *parent = Q_NULLPTR);
    ~QDeviceSupplyShowList();

    ///
    /// @brief
    ///     刷新耗材页面
    ///
    /// @param[in]  list  数据列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void RefreshSupplyList(std::map<std::shared_ptr<const tf::DeviceInfo>, std::vector<ch::tf::SuppliesOverview>>& list);

private:
    ///
    /// @brief
    ///     初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void Init();

private:
    Ui::QDeviceSupplyShowList* ui;
    std::vector<QDeviceSupplyStatus*> m_deviceSupplyList;       ///< 设备耗材信息列表
};
