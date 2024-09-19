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
/// @file     datetimefmttool.cpp
/// @brief    通用基础对话框
///
/// @author   4170/TangChuXian
/// @date     2020年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "datetimefmttool.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

#include <QDateTime>
#include <QRegularExpression>

// 定义24小时制和12小时制时间格式
#define         TIME_FORMAT_24_HOUR             ("hh:mm:ss")            // 24小时制时间格式
#define         TIME_FORMAT_12_HOUR             ("hh:mm:ss AP")         // 12小时制时间格式

#define         INDEX_TIME_FORMAT_24_HOUR       (0)                     // 24小时制时间格式索引
#define         INDEX_TIME_FORMAT_12_HOUR       (1)                     // 12小时制时间格式索引

DateTimeFmtTool::DateTimeFmtTool(QObject *parent)
    : QObject(parent)
{
    // 注册时间格式改变槽函数
    REGISTER_HANDLER(MSG_ID_DISPLAY_SET_UPDATE, this, OnDateTimeFormatUpdate);

    BaseSet ds;
    if (!DictionaryQueryManager::GetUiBaseSet(ds))
    {
        ULOG(LOG_ERROR, "Failed to get BaseSet from DB.");
        return;
    }

    // 日期时间格式
    m_strDateFmt = QString::fromStdString(ds.strDateForm);
    if (ds.iTimeForm == INDEX_TIME_FORMAT_24_HOUR)
    {
        m_strTimeFmt = TIME_FORMAT_24_HOUR;
    }
    else
    {
        m_strTimeFmt = TIME_FORMAT_12_HOUR;
    }
}

DateTimeFmtTool::~DateTimeFmtTool()
{
    // 清空父对象关系
    setParent(Q_NULLPTR);
}

///
/// @brief
///     获取单例
///
/// @return 单例对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月19日，新建函数
///
DateTimeFmtTool* DateTimeFmtTool::GetInstance()
{
    static DateTimeFmtTool s_obj;
    return &s_obj;
}

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
QString DateTimeFmtTool::GetCfgFmtDateTimeStr(const QString& strFixFmtDateTime)
{
    // 先尝试转换为日期时间
    QDateTime fmtDateTime = QDateTime::fromString(strFixFmtDateTime, UI_DATE_TIME_FORMAT);
    if (fmtDateTime.isValid())
    {
        return fmtDateTime.toString(m_strDateFmt + " " + m_strTimeFmt);
    }

    // 再尝试转换为日期
    QDate fmtDate = QDate::fromString(strFixFmtDateTime, UI_DATE_FORMAT);
    if (fmtDate.isValid())
    {
        return fmtDate.toString(m_strDateFmt);
    }

    // 最后参数转换为时间
    QTime fmtTime = QTime::fromString(strFixFmtDateTime, UI_TIME_FORMAT);
    if (fmtTime.isValid())
    {
        return fmtTime.toString(m_strTimeFmt);
    }

    return strFixFmtDateTime;
}

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
QString DateTimeFmtTool::GetCfgFmtDateStr(const QString& strFixFmtDateTime)
{
	QDateTime fmtDateTime = QDateTime::fromString(strFixFmtDateTime, UI_DATE_TIME_FORMAT);
	if (fmtDateTime.isValid())
	{
		return fmtDateTime.toString(m_strDateFmt);
	}

	return strFixFmtDateTime;
}

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
QString DateTimeFmtTool::GetCfgFmtTimeStr(const QString& strFixFmtDateTime)
{
	QDateTime fmtDateTime = QDateTime::fromString(strFixFmtDateTime, UI_DATE_TIME_FORMAT);
	if (fmtDateTime.isValid())
	{
		return fmtDateTime.toString(m_strTimeFmt);
	}

	return strFixFmtDateTime;
}


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
QString DateTimeFmtTool::GetMonthDayFormatStr(const QDate& date)
{
	// 不合法
	if (!date.isValid())
	{
		return "";
	}

	QString formateStr = date.toString(m_strDateFmt);

	return formateStr.remove(QRegularExpression("[/]?\\d{4}[/]?"));
}

///
/// @brief
///     获取日期时间格式
///
/// @return 日期时间格式
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月21日，新建函数
///
QString DateTimeFmtTool::GetDateTimeFormatStr()
{
    return m_strDateFmt + " " + m_strTimeFmt;
}

///
/// @brief
///     获取日期格式
///
/// @par History:
/// @li 4170/TangChuXian，2023年4月21日，新建函数
///
QString DateTimeFmtTool::GetDateFormatStr()
{
    return m_strDateFmt;
}

///
/// @brief
///     更新时间显示格式
///
/// @param[in]  ds  显示设置信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
void DateTimeFmtTool::OnDateTimeFormatUpdate(const BaseSet& ds)
{
    // 日期时间格式
    m_strDateFmt = QString::fromStdString(ds.strDateForm);
    if (ds.iTimeForm == INDEX_TIME_FORMAT_24_HOUR)
    {
        m_strTimeFmt = TIME_FORMAT_24_HOUR;
    }
    else
    {
        m_strTimeFmt = TIME_FORMAT_12_HOUR;
    }
}
