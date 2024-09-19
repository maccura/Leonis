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
#include <vector>
#include <memory>
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui {
    class QReagentScanDialog;
};
class QCheckBox;


class QReagentScanDialog : public BaseDlg
{
    Q_OBJECT

public:
    QReagentScanDialog(QWidget *parent);
    ~QReagentScanDialog();

protected:
    ///
    /// @brief 初始化试剂扫描对话框
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月7日，新建函数
    ///
    void Init();

    protected slots:
    ///
    /// @brief 确定按钮槽函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月7日，新建函数
    ///
    void OnScanReagent();

    ///
    /// @bref
    ///		设备选择框勾选槽
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年1月16日，新建函数
    ///
    void OnDeviceCheckboxCheck(int stat);

private:

    ///
    /// @bref
    ///		获取勾选的设备SN，如果没有设备被勾选，则执行失败
    ///
    /// @param[out] devicesSn 勾选的设备SN
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月29日，新建函数
    ///
    bool GetSelectedDeviceSn(std::set<std::string>& devicesSn);

private:
    Ui::QReagentScanDialog* ui;
    std::vector<std::shared_ptr<const tf::DeviceInfo>>	m_devices;                          // 因为有多设备模块选择框，所以传入的当前设备可能时多个
    std::vector<QCheckBox*>                         m_deviceCheckboxs;                  // 设备勾选框
    const int                                       m_ciDevGridRowCnt;                  // 设备网格行数
    const int                                       m_ciDevGridColCnt;                  // 设备网格列数
    const int                                       m_ciStrechRowDis;                   // 行缩进距离
    const int                                       m_ciVSpace;                         // 网格间距
    const int                                       m_ciHSpace;                         // 网格间距
    const int                                       m_ciCheckBoxFixHeight;              // 复选框固定高度
};
