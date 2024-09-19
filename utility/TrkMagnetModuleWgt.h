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
/// @file     TrkMagnetModuleWgt.h
/// @brief    应用-节点调试-轨道电磁铁模块
///
/// @author   8110/wutong
/// @date     2024年5月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8110/wutong，2024年5月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#ifndef TRKMAGNETMODULEWGT_H
#define TRKMAGNETMODULEWGT_H

//#pragma execution_character_set("utf-8")

#include <QWidget>
#include <sstream>
#include "iBITDebugger.h"

namespace Ui {
class TrkMagnetModuleWgt;
}

class TrkMagnetModuleWgt : public QWidget
{
    Q_OBJECT

public:
    explicit TrkMagnetModuleWgt(QWidget *parent = nullptr);
    ~TrkMagnetModuleWgt();

    ///
    /// @brief  更新界面信息
    ///
    /// @param[in]  nodeID  模块节点号
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月13日，新建函数
    ///
    void UpdateUi(node_property& nodeProperty, std::vector<Electromagnet>& magnets);

protected Q_SLOTS:
    ///
    /// @brief  点击打开按钮
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月13日，新建函数
    ///
    void OnOpenClicked();

    ///
    /// @brief  点击关闭按钮
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月13日，新建函数
    ///
    void OnCloseClicked();

    ///
    /// @brief  选中项改变
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月13日，新建函数
    ///
    void OnSelectListItem();

private:
    ///
    /// @brief	初始化UI
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月13日，新建函数
    ///
    void InitUi();

    ///
    /// @brief	初始化数据
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月13日，新建函数
    ///
    void InitData();

    ///
    /// @brief	初始化连接
    ///
    /// @par History:
    ///
    /// @li 8110/wutong，2024年5月13日，新建函数
    ///
    void InitConnect();


private:
    Ui::TrkMagnetModuleWgt *ui;
    node_property m_nodeProperty;										 //当前选中模块
    std::map<int, std::pair<std::string, std::string>> m_rowFlow;        //列对应电磁铁流程
    std::string m_openFlow;                                              //当前打开流程
    std::string m_closeFlow;                                             //当前关闭流程
};

#endif // TRKMAGNETMODULEWGT_H
