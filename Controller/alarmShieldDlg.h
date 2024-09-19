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
/// @file     alarmShieldDlg.h
/// @brief    主界面->告警对话框->报警屏蔽列表
///
/// @author   7951/LuoXin
/// @date     2023年6月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年6月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "shared/basedlg.h"
#include <set>

class QStandardItemModel;
namespace Ui {
    class alarmShieldDlg;
}
struct AlarmShieldInfo;


class alarmShieldDlg : public BaseDlg
{
    Q_OBJECT

public:
    alarmShieldDlg(QWidget *parent = Q_NULLPTR);
    ~alarmShieldDlg();

    ///
    /// @brief  获取解除报警屏蔽按钮的指针
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月13日，新建函数
    ///
    QPushButton* GetDisAlarmShieldBtnPtr();

    ///
    /// @brief  加载报警码到表格
    ///     
    ///
    /// @param[in]  mapShieldCodeAndSn  报警屏蔽列表
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月13日，新建函数
    ///
    void LoadCodeToview(const AlarmShieldInfo& asi);

    ///
    /// @brief  获取解除报警屏蔽的设备和报警码列表
    ///     
    /// @return 解除报警屏蔽的设备和报警码列表
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月29日，新建函数
    ///
    AlarmShieldInfo GetDisAlarmShieldCodeMap();


protected Q_SLOTS:

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月4日，新建函数
    ///
    void OnPermisionChanged();

private:

    ///
    /// @brief  初始化
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月13日，新建函数
    ///
    void Init();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月16日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief 时间过滤器
    ///
    /// @param[in]  target  目标对象
    /// @param[in]  event   事件对象
    ///
    /// @return 处理则返回true，否则返回false
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月30日，新建函数
    ///
    virtual bool eventFilter(QObject* target, QEvent* event) override;

signals:
    void AlarmLevelChanged(int level);

private:
    Ui::alarmShieldDlg*							ui;
    QStandardItemModel*							m_tabViewMode;				///< 列表的模式
};
