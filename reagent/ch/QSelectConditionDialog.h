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

#include "shared/basedlg.h"
#include <memory>
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui{
    class QSelectConditionDialog;
};

struct SelectConditions
{
    // 使用状态
    bool m_inUse;
    bool m_backup;
    bool m_reagentShield;
    bool m_caliShield;
    bool m_notPlace;
    bool m_usageStatusEmpty;

    // 校准状态
    bool m_caliSuccess;
    bool m_caliFailed;
    bool m_notyetCali;
    bool m_caliing;
    bool m_caliStatusEmpty;
    bool m_caliEdit;

    // 试剂类型
    bool m_closeType;
    bool m_openType;
    bool m_scanFailed;

    SelectConditions()
        : m_inUse(false), m_backup(false), m_reagentShield(false), m_caliShield(false), m_notPlace(false), m_usageStatusEmpty(false)
        , m_caliSuccess(false), m_caliFailed(false), m_notyetCali(false), m_caliing(false), m_caliStatusEmpty(false), m_caliEdit(false)
        , m_closeType(false), m_openType(false), m_scanFailed(false)
    {}

    void CleanValues()
    {
        memset(this, 0, sizeof(SelectConditions));
    }

    int CountTrue()
    {
        int count = m_inUse + m_backup + m_reagentShield + m_caliShield + m_notPlace + m_usageStatusEmpty + \
            m_caliSuccess + m_caliFailed + m_notyetCali + m_caliing + m_caliStatusEmpty + m_caliEdit +\
            m_closeType + m_openType + m_scanFailed;

        return count;
    }

    // 转换为免疫查询结构
    void CovertTo(::im::tf::ReagTableUIQueryCond& queryCond);

    // 转换为生化查询结构
    void CovertTo(::ch::tf::ReagentGroupQueryCond& queryCond);

	// 是否选中使用状态的条件
	bool SelUseStatusIf() const;

	// 是否选中校准状态的条件
	bool SelCaliStatusIf() const;

	// 是否选中试剂类型的条件
	bool SelReagentTypeIf() const;

    // 转换为字符串
    QString toString();
};

class QSelectConditionDialog : public BaseDlg
{
    Q_OBJECT

public:
    QSelectConditionDialog(QWidget *parent = Q_NULLPTR);
    ~QSelectConditionDialog();

    void InitConditionCheck(const std::shared_ptr<SelectConditions>& cond, \
        const std::vector<std::shared_ptr<const tf::DeviceInfo>>& curDevi);

Q_SIGNALS:
    void SendSearchCondition(std::shared_ptr<SelectConditions>);

protected:
    void Init();

protected Q_SLOTS:
    void OnDoSearchReagent();
    void OnReset();

private:

    // 对“未放置”、“开试剂”、“闭试剂”做灰置显示
    void SetDisableForIm(bool disableFlag);

private:
    Ui::QSelectConditionDialog* ui;
};
