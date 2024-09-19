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
/// @file     QcRules.cpp
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
#include "QcRules.h"
#include <memory>
#include "QObject"
#include "src/common/Mlog/mlog.h"

#define INIT_QCRULE(qid,name,stat,decrip) do {\
std::shared_ptr<QcRuleItem> pItem(new QcRuleItem(qid, name, stat, decrip));\
m_allRules.push_back(pItem);} while (0);

QcRules* QcRules::GetInstance()
{
    static QcRules qcRulesSgingel;
    return &qcRulesSgingel;
}

QString QcRules::TranseRuleCode(const std::string& ruleCodes)
{
    QStringList qlst = QString::fromStdString(ruleCodes).split(";");
    QStringList qlstNames;
    for (const QString& qItem : qlst)
    {
        auto it = m_rules.find(qItem.toInt());
        if (it != m_rules.end())
        {
            qlstNames.append(it->second->m_name);
        }
    }

    return qlstNames.join(";");
}

QcRules::QcRules()
{
    LoadOriginalQcRules();
}

///
/// @bref
///		从json配置文件加载规则配置
///
/// @par History:
/// @li 8276/huchunli, 2022年12月23日，新建函数
///
void QcRules::LoadOriginalQcRules()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化全状态信息
    m_markes.push_back(QObject::tr("警告"));
    m_markes.push_back(QObject::tr("失控"));

    // 初始化质控规则描述
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_1_2S, QString("1-2S"), m_markes[0], QObject::tr("1个质控结果超过X±2s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_1_2_5_S, "1-2.5S", m_markes[1], QObject::tr("1个质控结果超过X±2.5S，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_1_3S, "1-3S", m_markes[1], QObject::tr("1个质控结果超过X±3s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_2_2S, "2-2S", m_markes[1], QObject::tr("2个连续的质控结果同时超过X-2s或X+2s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_3_1S, "3-1S", m_markes[1], QObject::tr("3个连续的质控结果同时超过X-1s或X+1s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_4_1S, "4-1S", m_markes[1], QObject::tr("4个连续的质控结果同时超过X-1s或X+1s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_R_4S, "R-4S", m_markes[1], QObject::tr("连续2个质控结果，一个超过X-2s，一个超过X+2s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_7_X, "7-X", m_markes[1], QObject::tr("7个连续的质控结果落在平均数的一侧，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_8_X, "8-X", m_markes[1], QObject::tr("8个连续的质控结果落在平均数的一侧，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_9_X, "9-X", m_markes[1], QObject::tr("9个连续的质控结果落在平均数的一侧，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_10_X, "10-X", m_markes[1], QObject::tr("10个连续的质控结果落在平均数的一侧，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_2_OF_3_2S, "(2 of 3)2S", m_markes[1], QObject::tr("连续3个质控结果有2个超过X-2s或X+2s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_3_OF_6_2S, "(3 of 6)2S", m_markes[1], QObject::tr("连续6个质控结果有3个超过X-2s或X+2s，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_12_X, "12-X", m_markes[1], QObject::tr("12个连续的质控结果落在平均数的一侧，为违背此规则"));
    INIT_QCRULE(::tf::QcRuleType::QC_RULE_7T, "7-T", m_markes[1], QObject::tr("7个连续的质控结果呈现向上或向下的趋势，为违背此规则"));

    // 初始化m_rules结构
    for (int i = 0; i < m_allRules.size(); ++i)
    {
        m_rules.insert(std::pair<int, std::shared_ptr<QcRuleItem>>(m_allRules[i]->m_id, m_allRules[i]));
    }
}
