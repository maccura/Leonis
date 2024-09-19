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
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "QSpecialProgessBar.h"

namespace Ui {
    class QDeviceSupplyStatus;
};

class QDeviceSupplyStatus : public QWidget
{
    Q_OBJECT

public:

    // 定义设备类型
    enum DEVICETYPE
    {
        CHE,
        ISE
    };

    QDeviceSupplyStatus(QWidget *parent = Q_NULLPTR);
    ~QDeviceSupplyStatus();

    ///
    /// @brief
    ///     设置设备类型
    ///
    /// @param[in]  type  设备类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void SetDeviceType(DEVICETYPE& type);

    ///
    /// @brief
    ///     设置设备名称
    ///
    /// @param[in]  name  设备名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    void SetDeviceName(QString name);

    ///
    /// @brief
    ///     设置设备耗材数据，显示
    ///
    /// @param[in]  supplyOverView  耗材数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void SetSupplyStatus(DEVICETYPE type, const std::vector<ch::tf::SuppliesOverview>& supplyOverView);

private:
    ///
    /// @brief
    ///     初始化仓内仓外以及ISE初始内容
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     根据耗材总览信息获取显示信息
    ///
    /// @param[in]  overview  耗材总览信息
    ///
    /// @return 显示信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    QSpecialProgessBar::ProgressStatus GetStatusFromSupplyInfo(const ch::tf::SuppliesOverview& overview);

    ///
    /// @brief
    ///     设置仓内清洗液
    ///
    /// @param[in]  overview  耗材信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void SetUnderDeckSupplyStatus(const ch::tf::SuppliesOverview& overview);

    ///
    /// @brief
    ///     设置仓外清洗液
    ///
    /// @param[in]  overview  耗材信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void SetOutBoardSupplyStatus(const ch::tf::SuppliesOverview& overview);

    ///
    /// @brief
    ///     以某值初始化，队列多少次
    ///
    /// @param[in]  vector  需要初始化的队列
    /// @param[in]  value   初始化值
    /// @param[in]  times   次数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    template<typename T>
    void InitVector(std::vector<T>& vector, T value, int times)
    {
        for (int i = 0 ; i < times; i++)
        {
            vector.push_back(value);
        }
    }

private:
    Ui::QDeviceSupplyStatus*         ui;
    std::vector<QSpecialProgessBar::ProgressStatus> m_UnderDeckSupplyList; ///< 仓内耗材状态列表
    std::vector<QSpecialProgessBar::ProgressStatus> m_OutBoardSupplyList;  ///< 仓外耗材状态列表
    std::vector<QSpecialProgessBar::ProgressStatus> m_IseSupplyList;       ///< ISE耗材状态列表
};
