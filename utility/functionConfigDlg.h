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
/// @file     functionConfigDlg.h
/// @brief    应用--仪器--功能屏蔽--功能配置
///
/// @author   7951/LuoXin
/// @date     2023年8月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年8月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"

struct FunctionManageItem;
namespace Ui {
    class functionConfigDlg;
};
class QStandardItemModel;


class functionConfigDlg : public BaseDlg
{
    Q_OBJECT

public:
    functionConfigDlg(QWidget *parent = Q_NULLPTR);
    ~functionConfigDlg();

    ///
    /// @brief  加载数据到界面
    ///     
    ///
    /// @param[in]  data  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月4日，新建函数
    ///
    void LoadDataToView(std::vector<FunctionManageItem> data);

    ///
    /// @brief  获取保存按钮的指针
    ///     
    /// @return 保存按钮的指针
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月4日，新建函数
    ///
    QPushButton* GetSaveBtnPtr();
   
    ///
    /// @brief  通过索引获取对应的文字
    ///     
    ///
    /// @param[in]  index  索引
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月4日，新建函数
    ///
    QString GetStringByIndex(int index);

    ///
    /// @brief  获取界面设置的数据
    ///     
    /// @return 界面设置的数据
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月4日，新建函数
    ///
    std::vector<FunctionManageItem> GetData();

    protected slots:
    ///
    /// @brief 复选框的槽函数（点击复选框）
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月2日，新建函数
    ///
    void OnCheckBoxClicked(bool checked);

    ///
    /// @brief 右侧表格的删除按钮
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月2日，新建函数
    ///
    void OnCloseDelegate(QString text);

    ///
    /// @brief 更新按钮状态
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月2日，新建函数
    ///
    void UpdateBtnState();

private:

    ///
    /// @brief 初始化
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月2日，新建函数
    ///
    void Init();

private:
    Ui::functionConfigDlg*          ui;
    QStandardItemModel*				m_leftModel;			///< 左侧列表模式
    QStandardItemModel*				m_rightModel;			///< 右侧列表模式
};
