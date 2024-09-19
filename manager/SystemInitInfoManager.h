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
/// @file     SystemInitInfoManager.h
/// @brief    系统初始化信息管理器
///
/// @author   8580/GongZhiQiang
/// @date     2024年1月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年1月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <mutex>
#include <set>
#include <QObject>
#include <memory>
#include "src/db/AlarmInfo.hxx"
#include <boost/noncopyable.hpp>
using namespace std;

///
/// @brief   系统初始化信息管理器
///
class SystemInitInfoManager : public QObject, public boost::noncopyable
{
	Q_OBJECT

protected:

	///
	/// @brief  构造函数
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年1月31日，新建函数
	///
	SystemInitInfoManager();

public:
    ///
    /// @brief  获得单例对象
    ///
    /// @return 系统初始化信息管理单例对象
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年1月31日，新建函数
    ///
    static std::shared_ptr<SystemInitInfoManager> GetInstance();

	///
	/// @brief  系统信息初始化
	///
	/// @return true 表示初始化成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年1月31日，新建函数
	///
	bool SystemDefaultInfoInit();
	
protected Q_SLOTS:
	
private:
	///
	///  @brief 保存系统UUID
	///
	///
	///
	///  @return true:保存成功
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年12月6日，新建函数
	///
	bool SaveSystemUUID();

	///
	///  @brief 保存系统日志路径
	///
	///
	///
	///  @return true:保存成功
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年1月25日，新建函数
	///
	bool SaveSystemPaths();

	///
	/// @brief 设置周维护间隔维护起始时间为应用程序启动时间
	///
	///
	/// @return true:设置成功
	///
	/// @par History:
    /// @li 8580/GongZhiQiang，2024年1月31日，新建函数
    /// @li 8276/huchunli，2024年6月24日，调整为对所有设置了间隔时间的自动维护赋予初始值
	///
	bool SetMaintainIntervalSetTime();

    ///
    /// @brief 更新免疫仪器灯报警码
    ///
    ///
    /// @return true:更新成功
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2024年1月31日，新建函数
    ///
    bool UpdateI6000AffectLightAlarms();
	
    ///
    /// @brief 获取报警码
    ///
    ///
    /// @return AlarmCode 报警码封装
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2024年1月31日，新建函数
    ///
    AlarmCode GetAlarmCode(std::vector<int> AlarmCode);

private:
    static std::shared_ptr<SystemInitInfoManager> s_instance;			///< 单例对象
    static std::recursive_mutex					  s_mtx;				///< 保护单例对象的互斥量

};
