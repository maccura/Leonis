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
/// @file     QcRules.h
/// @brief    质控规则配置加载与管理
///
/// @author   8276/huchunli
/// @date     2022年12月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2022年12月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _INCLUDE_QCRULES_H_
#define _INCLUDE_QCRULES_H_

#include <map>
#include <vector>
#include <QString>
#include "src/thrift/gen-cpp/defs_types.h"

// 一条规则描述
class QcRuleItem
{
public:
    QcRuleItem(::tf::QcRuleType::type id, const QString& name, const QString& mark, const QString& desc)
        :m_id(id), m_name(name), m_mark(mark), m_desc(desc)
    {}
    ::tf::QcRuleType::type m_id;           // 规则编号（与defs.thrift中定义的QcRuleType枚举对应）
    QString m_name;     // 规则名字
    QString m_mark;     // 规则默认状态标记
    QString m_desc;     // 规则描述
};

class QcRules
{
public:
    static QcRules* GetInstance();

    ///
    /// @bref
    ///		获取所有规则
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    std::vector<std::shared_ptr<QcRuleItem>> GetAllRules()
    {
        return m_allRules;
    }

    ///
    /// @bref
    ///		获取所有的规则状态
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    std::vector<QString>& GetQcMarkList()
    {
        return m_markes;
    }

    ///
    /// @bref
    ///		将分号分割的规则编号转换为规则名
    ///
    /// @param[in] ruleCodes 分号分割的规则编号串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月18日，新建函数
    ///
    QString TranseRuleCode(const std::string& ruleCodes);

private:
    QcRules();

    ///
    /// @bref
    ///		从json配置文件加载规则配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    void LoadOriginalQcRules();

    std::vector<QString>        m_markes; // 状态信息（告警、失控）

    std::map<int, std::shared_ptr<QcRuleItem>>  m_rules;
    std::vector<std::shared_ptr<QcRuleItem>>    m_allRules;
};

#endif
