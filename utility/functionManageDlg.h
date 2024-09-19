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
/// @file     functionManageDlg.h
/// @brief    应用--仪器--功能管理
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
#include <shared/basedlg.h>

namespace Ui {
    class functionManageDlg;
};

namespace tf {
    class DeviceInfo;
};

class QColorHeaderModel;
class functionConfigDlg;
class functionSetDlg;
class QItemSelection;
struct FunctionManageItem;

class functionManageDlg : public BaseDlg
{
    Q_OBJECT

public:
    functionManageDlg(QWidget *parent = Q_NULLPTR);
    ~functionManageDlg();

    ///
    /// @brief	加载数据到控件
    ///     
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void LoadDataToCtrls();

protected:
    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

private:
    ///
    /// @brief	初始化控件
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void InitCtrls();

    ///
    /// @brief	初始化表格
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void InitTabView();

    ///
    /// @brief	更新表格的行
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    void UpdateRowDataToView();

    ///
    /// @brief	通过设备名字获取所在列
    ///
    /// @param[in]  model  数据模型
    /// @param[in]  devName  设备的名字
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    int GetColunmByDevName(QColorHeaderModel* model, QString devName);

    ///
    /// @brief	通过设备名字获取所在列
    ///
    /// @param[in]  dev 设备信息
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    void SetViewItemSelect(const ::tf::DeviceInfo& dev);

    ///
    /// @brief	通过索引获取对应的状态
    ///
    /// @param[in]  dev        设备信息
    /// @param[in]  index      索引
    /// @param[in]  isExist    模块是否存在
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    bool GetStatusByIndex(const ::tf::DeviceInfo& dev, int index, bool& isExist);

    ///
    /// @brief	更新设备的额外信息
    ///
    /// @param[in]  model   表格的数据模型
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    bool UpdateDeviceOtherInfo(QColorHeaderModel* model);

    ///
    /// @brief	更新设备的额外信息
    ///
    /// @param[in]  dev   设备信息
    /// @param[in]  col   表格的列
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    bool UpdateChDeviceOtherInfo(const ::tf::DeviceInfo& dev, int col);

    ///
    /// @brief	更新设备的额外信息
    ///
    /// @param[in]  dev   设备信息
    /// @param[in]  col   表格的列
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    bool UpdateIseDeviceOtherInfo(const ::tf::DeviceInfo& dev, int col);

    ///
    /// @brief	更新设备的额外信息
    ///
    /// @param[in]  dev   设备信息
    /// @param[in]  col   表格的列
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月3日，新建函数
    ///
    bool UpdateImDeviceOtherInfo(const ::tf::DeviceInfo& dev, int col);

    protected Q_SLOTS:
    ///
    /// @brief	确定按钮被点击
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月4日，新建函数
    ///
    void OnSaveBtnClicked();

    ///
    /// @brief	保存功能管理界面信息
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月4日，新建函数
    ///
    void OnFunctionConfigSave();

    ///
    /// @brief	功能设置按钮被点击
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年1月3日，新建函数
    ///
    void OnFunctionSetBtnClicked();

    ///
    /// @brief
    ///     表格当前选中行改变的槽函数
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月4日，新建函数
    ///
    void OnCurrentRowChanged(const QItemSelection &selected, const QItemSelection &deselected);

    ///
    /// @brief 权限变更
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年3月29日，新建函数
    ///
    void OnPermisionChanged();

private:
    Ui::functionManageDlg*						ui;
    functionConfigDlg*                          m_pFunctionConfigDlg;   // 功能管理弹窗
    functionSetDlg*                             m_pFunctionSetDlg;      // 功能设置弹窗
    QColorHeaderModel*                          m_pChTabViewModel;      // 生化列表的model
    QColorHeaderModel*                          m_pImTabViewModel;      // 免疫列表的model
    std::map<std::string, std::vector<FunctionManageItem>> m_functionManageData;    // 功能管理数据
};
