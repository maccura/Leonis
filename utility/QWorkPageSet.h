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
/// @file     QWorkPageSet.h
/// @brief    工作页面的设置
///
/// @author   5774/WuHongTao
/// @date     2023年6月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2023年6月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "src/public/ConfigSerialize.h"

namespace Ui {
    class QWorkPageSet;
};
class QStandardItemModel;


class QWorkPageSet : public QWidget
{
    Q_OBJECT

public:
    QWorkPageSet(QWidget *parent = Q_NULLPTR);
    ~QWorkPageSet();

	///
	/// @brief 配置是否被修改过未保存
	///
	///
	/// @return true:被修改过
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月29日，新建函数
	///
	bool IsModified();

protected:
    ///
    /// @brief 初始化
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月14日，新建函数
    ///
    void Init();

    ///
    /// @brief 刷新样本列表
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月14日，新建函数
    ///
    void Refresh();

    ///
    /// @brief 切换样本列表中的两行
    ///
    /// @param[in]  srcRow  源
    /// @param[in]  dstRow  目的
    ///
    /// @return true:成功切换，false:失败切换
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月14日，新建函数
    ///
    bool ChangeViewRow(int srcRow, int dstRow);

    ///
    /// @brief 设置样本列表的checkbox
    ///
    /// @param[in]  data  first::是否显示，second:类型（0：样本订单，1:质控订单 2:校准订单）
    /// @param[in]  row  第几行
    ///
    /// @return 返回checkBox的句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月14日，新建函数
    ///
    QWidget* fSetCheck(std::pair<bool, int>& data, int row);

	///
	/// @brief 更新消息
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月20日，新建函数
	///
	void UpdateInfo();

private:
    void showEvent(QShowEvent *event);

private:
    Ui::QWorkPageSet*                ui;
    PageSet                         m_data;                     ///< 保存数据
    QStandardItemModel*             m_model;                    ///< 模式
};
