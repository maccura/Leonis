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
/// @file     ImNodeDebugWgt.h
/// @brief    生化节点调试界面
///
/// @author   4170/TangChuXian
/// @date     2024年3月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "thrift/im/ImLogicControlProxy.h"
#include <QWidget>
#include <QMap>

using namespace std;

class QPushButton;                  // 按钮
class ImStepModuleWgt;              // 运动模块界面
class ImPumpModuleWgt;              // 泵界面
class ImValveModuleWgt;             // 阀界面
class ImDevFacadeModuleWgt;         // 外观模块界面
class ImCustomNodeWgt;              // 自定义参数
class QListWidgetItem;              // 列表单元项

namespace Ui {
    class ImNodeDebugWgt;
};

class ImNodeDebugWgt : public QWidget
{
    Q_OBJECT

public:
    enum StackPageIdx
    {
        STACK_PAGE_IDX_STEP = 0,        // 运动模块
        STACK_PAGE_IDX_PUMP,            // 泵模块
        STACK_PAGE_IDX_VALVE,           // 阀模块
        STACK_PAGE_IDX_FACADE,          // 外观模块
        STACK_PAGE_IDX_CUSTOM,          // 自定义参数
    };

public:
    ImNodeDebugWgt(QWidget *parent = Q_NULLPTR);
    ~ImNodeDebugWgt();

	///
	/// @brief  更新界面信息
	///
	/// @param[in]  devSN  设备序列号
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月26日，新建函数
	///
	void UpdateUi(const string& devSN);

    ///
    /// @brief
    ///     导出文件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月10日，新建函数
    ///
    void ExportFIle();

    ///
    /// @brief
    ///     导入文件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月10日，新建函数
    ///
    void ImportFile();

protected:
    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月9日，新建函数
    ///
    void showEvent(QShowEvent *event);

private:
    ///
    /// @brief	初始化UI
    ///     
    /// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
    ///
    void InitUi();

	///
	/// @brief	初始化数据
	///     
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
	void InitData();

	///
	/// @brief	初始化连接
	///     
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
	void InitConnect();

protected Q_SLOTS:
    ///
	/// @brief
	///     选中项改变
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年4月7日，新建函数
	///
	void OnSelectListItem();

    ///
    /// @brief
    ///     更新模块列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月2日，新建函数
    ///
    void UpdateModuleList();

    ///
    /// @brief
    ///     调试模块更新
    ///
    /// @param[in]  stuTfDbgMd  调试模块
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月26日，新建函数
    ///
    void OnDbgModuleUpdate(const im::tf::DebugModule& stuTfDbgMd);

    ///
    /// @brief
    ///     WhatsThis按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月8日，新建函数
    ///
    void OnWhatsThisBtnClicked();

private:
    Ui::ImNodeDebugWgt*  ui;

    bool m_bInit;

	ImStepModuleWgt *m_stepModuleWgt = nullptr;
	ImPumpModuleWgt *m_pumpModuleWgt = nullptr;
	ImValveModuleWgt *m_valveModuleWgt = nullptr;
    ImDevFacadeModuleWgt *m_pLedModuleWgt = nullptr;
    ImCustomNodeWgt *m_pImCustomNodeWgt = nullptr;
	string m_devSN;

    QPushButton*      m_pWhatsThisBtn;
};
