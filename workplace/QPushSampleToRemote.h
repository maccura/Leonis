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
/// @file     QPushSampleToRemote.h
/// @brief    手工传输
///
/// @author   5774/WuHongTao
/// @date     2022年12月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年12月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/public/ConfigSerialize.h"

namespace Ui {
    class QPushSampleToRemote;
};


class QPushSampleToRemote : public BaseDlg
{
    Q_OBJECT
public:
    QPushSampleToRemote(QWidget *parent = Q_NULLPTR);
    ~QPushSampleToRemote();

    ///
    /// @brief 设置页面类型
    ///
    /// @param[in]  sampleKeys  单项目-多项目
    /// @param[in]  maxRecheckCnt  最大复查次数(初始化组合框使用)
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月2日，新建函数
    ///
    void SetPageAttribute(std::vector<int64_t> sampleKeys, int maxTestCnt = -1);

    ///
    /// @brief 设置当前手工传输设置（包含页面所有设置）
    ///
    /// @param[in]  workSet  工作页面的显示设置
    ///
    /// @return true:设置成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月20日，新建函数
    ///
    bool SetWorkSet(PageSet& workSet);

    ///
    /// @brief 获取选择的样本数据库主键List
    ///
    ///
    /// @return 数据库主键List
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月2日，新建函数
    ///
    std::vector<int64_t>& GetSelectSamples()
    {
        return m_sampleSelcts;
    }

signals:
    ///
    /// @brief 选择的结果
    ///
    /// @param[in]  reCheck  true: 复查  false: 初查
    /// @param[in]  reCheckCnt  复查的第几次(默认-1)
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月2日，新建函数
    ///
    void selectResultIndex(bool reCheck, int reCheckCnt = -1);

protected:
    ///
    /// @brief 保存选择结果
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月2日，新建函数
    ///
    void SaveResult();

    ///
    /// @brief 更新当前手工传输设置（包含页面所有设置）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月27日，新建函数
    ///
    void UpdateWorkSet();

private:
    Ui::QPushSampleToRemote*        ui;
    std::vector<int64_t>            m_sampleSelcts;
    PageSet                         m_workSet;
};
