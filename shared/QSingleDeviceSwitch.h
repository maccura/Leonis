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
/// @file     QSingleDeviceSwitch.h
/// @brief    单个设备切换的基础（按照设备类型切换，如有需要可以考虑增加按照型号等其它条件切换，需要再细粒度控制）
///
/// @author   8090/YeHuaNing
/// @date     2022年12月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
#include "ui_QSingleDeviceSwitch.h"
#include "src/thrift/gen-cpp/defs_types.h"

typedef std::map<tf::AssayClassify::type,
    std::function<QWidget*(std::vector<std::shared_ptr<const tf::DeviceInfo>>&)>> ClassifyFuncMap;
typedef std::map<tf::AssayClassify::type, QWidget*> ClassifyWidgetMap;

namespace Ui { class QSingleDeviceSwitch; };

class QSingleDeviceSwitch : public QWidget
{
    Q_OBJECT

public:
    QSingleDeviceSwitch(QWidget *parent = Q_NULLPTR);
    ~QSingleDeviceSwitch();

    ///
    /// @brief刷新设备列表
    ///     
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月13日，新建函数
    ///
    void RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices);

    ///
    /// @brief 按照类型刷新页面
    ///     
    ///
    /// @param[in]  classify  分类
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月13日，新建函数
    ///
    void RefreshPage(tf::AssayClassify::type classify);

    ///
    /// @brief 设置不同类型设备的设备刷新函数
    ///
    /// @param[in]  classify   设备类型
    /// @param[in]  func  调用函数
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月14日，新建函数
    ///
    void SetClassifyFunc(tf::AssayClassify::type classify, const std::function<QWidget*(std::vector<std::shared_ptr<const tf::DeviceInfo>>&)>& func);

    ///
    /// @brief 按产品线切换时，直接设置页面
    ///
    /// @param[in]  classify  分类
    /// @param[in]  page  页面
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年7月4日，新建函数
    ///
    void SetClassifyWidget(tf::AssayClassify::type classify, QWidget* const page);

private:
    Ui::QSingleDeviceSwitch*        ui;
    ClassifyFuncMap                 m_mapClassifyFunc;          ///< 设备类型切换表
    ClassifyWidgetMap               m_mapClassifyWidget;        ///< 设备类型设备页面切换
};
