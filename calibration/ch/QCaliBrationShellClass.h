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
/// @file     QReagentShellClass.h
/// @brief    生化免疫试剂外壳页面
///
/// @author   5774/WuHongTao
/// @date     2022年1月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/QPageShellClass.h"
#include "QCaliBrationNavigation.h"

class ImQCalibrateSet;
class QCalibrateSet;
class QSingleDeviceSwitch;
class QCalibrateOverView;

class QCaliBrationShellClass : public QPageShellClass
{
    Q_OBJECT
   
public:
    QCaliBrationShellClass(QWidget *parent, CALIBRATEPAGENAVIGATION pageType);
    ~QCaliBrationShellClass();

    ///
    /// @brief
    ///     获取设备信息列表
    ///
    ///
    /// @return 设备信息列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月18日，新建函数
    ///
    virtual std::vector<std::shared_ptr<const tf::DeviceInfo>> GetDeviceInfos();

    ///
    /// @brief
    ///     生成右上角按钮控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void SetCornerButtonWidget(const std::vector<CornerButtonInfo>& pageInfos);

private slots:

    ///
    /// @brief
    ///     子模块按钮被选中
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    virtual void OnSelectPage();

    ///
    /// @brief 选中生化和免疫的切换
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月22日，新建函数
    ///
    virtual void OnSelectButton();

private:
    ///
    /// @brief
    ///     根据页面类型，生成不同的页面
    ///
    /// @param[in]  type  页面类型
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月21日，新建函数
    ///
    void CreatSubPageByPageType(int type);

    virtual int CurrentPageType() override;

private:
    bool                            m_hasChemistry;
    bool                            m_hasImmune;
    QCalibrateOverView*             m_calibrateOverview;        ///< 校准总览页面
    QSingleDeviceSwitch*            m_caliBrateHistory;         ///< 校准历史页面
    QSingleDeviceSwitch*            m_caliBrateSet;             ///< 校准设置(生化)
    //ImQCalibrateSet*                m_caliImBrateSet;           ///< 校准设置（免疫）
    CALIBRATEPAGENAVIGATION         m_pageType;
};
