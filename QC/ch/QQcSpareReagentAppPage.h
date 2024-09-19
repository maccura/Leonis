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
#include "src/thrift/gen-cpp/defs_types.h"

class QQcSpareAppModel;
namespace Ui {
    class QQcSpareReagentAppPage;
};


class QQcSpareReagentAppPage : public BaseDlg
{
    Q_OBJECT

public:
    using DeviceModules = std::vector<std::shared_ptr<const tf::DeviceInfo>>;
    QQcSpareReagentAppPage(QWidget *parent = Q_NULLPTR);
    ~QQcSpareReagentAppPage();

    /// @brief 启动申请页面的列表显示
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @return 成功，返回true
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月6日，新建函数
    ///
    bool StartQcAppPage(DeviceModules& devices);

Q_SIGNALS:
    void closeWindows();
private slots:
    void OnShowCurrentQc(int index);

private:
    Ui::QQcSpareReagentAppPage*     ui;
    DeviceModules                   m_devInfos;
    std::vector<::tf::QcDoc>        m_docs;
    QQcSpareAppModel*               m_module;
    int                             m_currentQcIndex;
};
