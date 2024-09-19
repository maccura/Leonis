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
/// @file     datetimefmttool.h
/// @brief    日期格式工具
///
/// @author   4170/TangChuXian
/// @date     2023年4月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "src/public/ConfigDefine.h"
#include <QObject>
#include <QString>

// 转化为配置格式的日期时间
#define ToCfgFmtDateTime(strFixFmtDateTime)             DateTimeFmtTool::GetInstance()->GetCfgFmtDateTimeStr(strFixFmtDateTime)
#define ToCfgFmtDate(strFixFmtDateTime)					DateTimeFmtTool::GetInstance()->GetCfgFmtDateStr(strFixFmtDateTime)
#define ToCfgFmtTime(strFixFmtDateTime)					DateTimeFmtTool::GetInstance()->GetCfgFmtTimeStr(strFixFmtDateTime)

class DateTimeFmtTool : public QObject
{
    Q_OBJECT

public:
    ~DateTimeFmtTool();

    ///
    /// @brief
    ///     获取单例
    ///
    /// @return 单例对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月19日，新建函数
    ///
    static DateTimeFmtTool* GetInstance();

    ///
    /// @brief
    ///     通过固定日期时间格式（2023-04-19 15:10:30）获得满足配置的日期时间格式字符串
    ///
    /// @param[in]  strFixFmtDateTime  固定格式日期时间字符串
    ///
    /// @return 满足配置的日期时间格式字符串
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月19日，新建函数
    ///
    QString GetCfgFmtDateTimeStr(const QString& strFixFmtDateTime);

	///
	/// @brief
	///     通过固定日期时间格式（2023-04-19 15:10:30）获得满足配置的日期格式字符串
	///
	/// @param[in]  strFixFmtDateTime  固定格式日期时间字符串
	///
	/// @return 满足配置的日期格式字符串
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月7日，新建函数
	///
	QString GetCfgFmtDateStr(const QString& strFixFmtDateTime);

	///
	/// @brief
	///     通过固定日期时间格式（2023-04-19 15:10:30）获得满足配置的时间格式字符串
	///
	/// @param[in]  strFixFmtDateTime  固定格式日期时间字符串
	///
	/// @return 满足配置的时间格式字符串
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月7日，新建函数
	///
	QString GetCfgFmtTimeStr(const QString& strFixFmtDateTime);

	///
	/// @brief	通过QDate获得满足配置的月日格式字符串
	///	
	///
	/// @param[in]  date  日期（例如12/26, 26/12）
	///
	/// @return 满足配置的月日格式字符串
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月26日，新建函数
	///
	QString GetMonthDayFormatStr(const QDate& date);

    ///
    /// @brief
    ///     获取日期时间格式
    ///
    /// @return 日期时间格式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月21日，新建函数
    ///
    QString GetDateTimeFormatStr();

    ///
    /// @brief
    ///     获取日期格式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月21日，新建函数
    ///
    QString GetDateFormatStr();

protected Q_SLOTS:
    ///
    /// @brief
    ///     更新时间显示格式
    ///
    /// @param[in]  ds  显示设置信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月29日，新建函数
    ///
    void OnDateTimeFormatUpdate(const BaseSet& ds);

protected:
    DateTimeFmtTool(QObject *parent = Q_NULLPTR);

private:
    // 保存日期格式和时间格式
    QString      m_strDateFmt;       // 日期格式
    QString      m_strTimeFmt;       // 时间格式
};
