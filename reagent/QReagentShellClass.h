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
#include "src/thrift/gen-cpp/defs_types.h"

class ImRgntPlateWidget;
class ReagentShowList;
class QSupplyList;
class ReagentOverview;
class ReagentChemistryWidgetPlate;
class QPageIseSupply;


// 页面导航
enum PAGENAVIGATION
{
	// 由于屏蔽了REAGENTOVERVIEW，第一页应该是REAGENTPLATE
    REAGENTPLATE = 0,
    REAGENTLIST,
    SUPPLIESLIST,
	REAGENTOVERVIEW
};

class QReagentShellClass : public QPageShellClass
{
    Q_OBJECT
   
public:
    QReagentShellClass(QWidget *parent, PAGENAVIGATION pageType);
    ~QReagentShellClass();

protected:
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
    virtual std::vector<std::shared_ptr<const tf::DeviceInfo>> GetDeviceInfos();

    virtual int CurrentPageType() override;

protected slots:
    virtual void OnSelectPage();
    virtual void OnSelectButton();

	// 设备状态改变的槽函数
	void OnDeviceStateChange(tf::DeviceInfo deviceInfo);

private:

    ///
    /// @bref
    ///		切换到当前页面
    ///
    /// @param[in] targetWidget 目标页面
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月2日，新建函数
    ///
    void SwitchWidgetToTarget(QWidget* targetWidget);

private:
    ReagentOverview*                m_overViewPage;             ///< 生化总览
    ReagentChemistryWidgetPlate*    m_chemistryPage;            ///< 生化页面
    QPageIseSupply*                 m_iseSupplyPage;            ///< ISE的耗材页面
    ReagentShowList*                m_reagentChemistryList;     ///< 试剂列表（生化）
    QSupplyList*                    m_supplyOuter;              ///< 仓外耗材
    PAGENAVIGATION                  m_pageType;

    ImRgntPlateWidget*              m_pRgntPlateWgt;            // 试剂盘界面（免疫）
};
